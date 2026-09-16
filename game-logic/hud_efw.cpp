#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "triangleapi.h"

#include <string.h>
#include <stdio.h>

// Client half of EFWData / EFWShow / EFW_Menu / EFW_CtPrv / EFW_Cntxt
// (client.dll 0x100419e0, FUN_10044f70 prompts, FUN_10047830 storyboards).

#define EFW_MAX_SCAN 3
#define EFW_SCAN_NAME 29
#define EFW_SCAN_BYTES 0x30

#pragma pack(push, 1)
struct EfwScanSlot
{
	int type;
	char name[EFW_SCAN_NAME];
	char zero;
	unsigned char pad[2];
	float x, y, z;
};
#pragma pack(pop)

static float g_hope = -1;
static int g_diaryPage;
static int g_diaryOpen;
static int g_mapLevel; /* hudInt[3]; FUN_1001db00 clock am/pm + hour base */
static int g_talkPrompt;
static int g_menuCode;
static int g_weaponId = -1;
static HSPRITE g_hDiary;
static HSPRITE g_hLogo; /* sprites/efw_artslogo.spr; FUN_1001db00 tail */
static int g_loadedPage = -1;
static char g_menuLine[7][256];
static int g_menuOn;
static EfwScanSlot g_scan[EFW_MAX_SCAN];
static int g_scanCount;
static HSPRITE g_hBubble;
static HSPRITE g_hHide;
static HSPRITE g_hPliers;
static HSPRITE g_hGive;
static HSPRITE g_hStory;
static HSPRITE g_hGrey; /* FUN_100436c0 sprites/efw_grey.spr */
static HSPRITE g_hAscale; /* FUN_10044e30 sprites/ascale.spr */
static int g_storyCode;
static int g_storyPauseSent;
static char g_storyChange[64];
static float g_storyFade; /* DAT_100baf10 */
static int g_weaponMask;
static int g_contextMode; /* DAT_100bc338; FUN_10046370 / FUN_100463c0 */
static float g_contextDismissAt; /* DAT_100bc9f4; 0.4s debounce on 0x48 */
static char g_caption[1024]; /* DAT_100baf04; FUN_10048790 */
static int g_captionLen; /* DAT_100baf08 */
static float g_captionAt; /* DAT_100baf14 */
static float g_captionAge;
static float g_menuVeil; /* DAT_100a95b8; FUN_1001db00 conversation veil */
static float g_invFade; /* DAT_100a95bc; FUN_10043dd0(fade) */
static float g_diaryFade; /* DAT_100a95c4; diary SPR wipe */
static int g_diaryFadePage; /* DAT_100a95c0 */

#ifndef K_MOUSE1
#define K_MOUSE1 107
#define K_MOUSE2 108
#endif

#define EFW_VGUI_MAX 6 /* FUN_100c6d70 DAT_10134894..a8 — six CommandButton slots */
struct EfwVguiBtn
{
	int x, y, w, h;
	char cmd[96];
	char label[64];
	HSPRITE icon;
};
static EfwVguiBtn g_vgui[EFW_VGUI_MAX];
static int g_vguiN;
static char g_vguiSig[512];

static HSPRITE EFW_LoadSpr( const char *path )
{
	return SPR_Load( path );
}

static void EFW_ClearStoryboard( void );
static void EFW_ClearCaption( void );

static int __MsgFunc_EFWData( const char *pszName, int iSize, void *pbuf )
{
	unsigned char blob[36];
	int i;
	int n;
	BEGIN_READ( pbuf, iSize );
	READ_BYTE();
	n = iSize - 1;
	if( n > 36 )
		n = 36;
	memset( blob, 0, sizeof( blob ) );
	for( i = 0; i < n; i++ )
		blob[i] = (unsigned char)READ_BYTE();
	memcpy( &g_hope, blob + 4, sizeof( float ) );
	memcpy( &g_diaryPage, blob + 12, sizeof( int ) );
	memcpy( &g_mapLevel, blob + 8 + 3 * 4, sizeof( int ) );
	memcpy( &g_weaponId, blob + 8 + 4 * 4, sizeof( int ) );
	g_weaponMask = ( (unsigned)g_weaponId ) >> 16;
	g_weaponId = g_weaponId & 0xffff;
	if( g_weaponId == 0xffff )
		g_weaponId = -1;
	{
		int openFlag = 0;
		int paused = 0;
		memcpy( &openFlag, blob + 8 + 5 * 4, sizeof( int ) );
		g_diaryOpen = openFlag != 0;
		/* hudInt[6] pause. HTML Continue is FUN_100485d0 ClientCmd
		   efw_pause 0; Panel dtor then clears DAT_1007ab5c. */
		memcpy( &paused, blob + 8 + 6 * 4, sizeof( int ) );
		{
			static int s_lastPause = -1;
			/* Falling edge of hudInt[6]: Panel dtor DAT_1007ab5c = -1. */
			if( s_lastPause == 1 && paused == 0 )
			{
				if( g_storyCode )
					EFW_ClearStoryboard();
				if( g_captionLen )
					EFW_ClearCaption();
			}
			s_lastPause = paused;
		}
	}
	{
		static int s_diaryLog = -1;
		int packed = ( g_diaryOpen ? 1 : 0 ) * 100 + g_diaryPage;
		if( packed != s_diaryLog )
		{
			s_diaryLog = packed;
			gEngfuncs.Con_Printf( ">>> diaryhud open=%d page=%d\n", g_diaryOpen, g_diaryPage );
		}
	}
	{
		static int s_hopeLog;
		s_hopeLog++;
		if( s_hopeLog == 1 || ( s_hopeLog % 40 ) == 0 )
			gEngfuncs.Con_Printf( ">>> hopehud %.1f\n", g_hope );
	}
	return 1;
}

/* FUN_100c7380 / LAB_10041930: byte 0xff clears; byte 0-6 appends a 30-char chunk. */
static int __MsgFunc_EFWShow( const char *pszName, int iSize, void *pbuf )
{
	int code;
	const char *chunk;
	char *dst;
	int used;
	BEGIN_READ( pbuf, iSize );
	code = READ_BYTE();
	if( code == 0xff )
	{
		memset( g_menuLine, 0, sizeof( g_menuLine ) );
		g_menuOn = 0;
		return 1;
	}
	if( code < 0 || code > 6 )
		return 1;
	chunk = "";
	if( iSize > 1 )
		chunk = READ_STRING();
	if( !chunk )
		chunk = "";
	dst = g_menuLine[code];
	used = (int)strlen( dst );
	if( used < (int)sizeof( g_menuLine[0] ) - 1 )
		strncat( dst, chunk, sizeof( g_menuLine[0] ) - 1 - used );
	g_menuOn = 1;
	return 1;
}

/* FUN_10047830 table at s_efw_UseWithMarker + code*4 + 0x14 for codes
   0x3c–0x51. Else-branch FUN_10048790 uses this string as DAT_100baf04. */
static const char *EFW_CaptionForCode( int code )
{
	switch( code )
	{
	case 0x3c:
		return "You realise that the guard will search you and find the pliers, and so decide not to leave the kitchen.";
	case 0x3d:
		return "You wait until the electrician is not looking, and quickly grab the pliers from the workbench. He doesn't notice, and you hide them under your shirt. Heart pounding, you wonder how to safely get them to Amir.";
	case 0x3e:
		return "Again, you wait for the ideal moment to retrieve the pliers from under your shirt and slowly lower them into the bin, careful to not make a sound.";
	case 0x3f:
		return "You realise that this is an ideal place to hide yourself for the next few hours, and wait until night falls. Now that the trader has agreed to take your ID tag from the fence, you won't be missed.";
	case 0x40:
		return "There's a hole. You could hide here, if you ever needed to.";
	case 0x41:
		return "You could hide here, but you'd be caught at dusk when the guards saw your ID tag and came searching.";
	case 0x42:
		return "You could hide here and come out at night to get the pliers, if only you had a way to break into the rubbish bin cage.";
	case 0x43:
		return "You return to the hiding place, with the pliers safely tucked away underneath your shirt.";
	case 0x44:
		return "You could hide again, but you haven't got the pliers yet.";
	case 0x45:
		return "You recognise the bin in front of you as the one from the kitchen earlier today. You open the top and dig around inside, and sure enough, the pliers are still there. You retrieve them from the foodscraps and rubbish, and hide them in your clothes. Now to work out how to safely get these back to your fellow plotters.";
	case 0x46:
		return "You've been caught by a patrolling guard. They're not pleased to find you sneaking about a locked compound late at night; you've been sent to solitary confinement. You don't expect to be let out for at least 3 days.";
	case 0x47:
		return "The package is from a pen-friend, a member of a refugee support group in Melbourne. The letter accompanying it brings you some hope, knowing that there is someone in this country that cares about your fate. Inside the package are some chocolate bars, which you give to some children, and a box of washing powder. Your suspicions aroused by mysterious rattling sound, you feel inside the box and discover a SIM card for a mobile phone.";
	default:
		return "<error>";
	}
}

static void EFW_ClearCaption( void )
{
	g_caption[0] = '\0';
	g_captionLen = 0;
	g_captionAt = 0.0f;
	g_captionAge = 0.0f;
}

static void EFW_OpenCaption( int code )
{
	const char *text;

	/* FUN_10048790: copy caption into DAT_100baf04, DAT_100baf08 = len,
	   DAT_100baf14 = now. Panel vtable setVisible ClientCmd efw_pause 1. */
	text = EFW_CaptionForCode( code );
	if( code - 0x3c >= 0x16 )
		text = "<error>";
	strncpy( g_caption, text, sizeof( g_caption ) - 1 );
	g_caption[sizeof( g_caption ) - 1] = '\0';
	g_captionLen = (int)strlen( g_caption );
	g_captionAt = gHUD.m_flTime;
	g_captionAge = 0.0f;
	g_storyCode = 0;
	g_hStory = 0;
	gEngfuncs.Con_Printf( ">>> FUN_10048790 n=%d code=0x%x %s\n", g_captionLen, code, g_caption );
	if( g_storyPauseSent != code )
	{
		g_storyPauseSent = code;
		gEngfuncs.pfnServerCmd( "efw_pause 1\n" );
	}
}

static void EFW_DismissCaption( void )
{
	/* Caption InputSignal: same pause-0 path as FUN_100485d0 without a
	   stored changelevel (FUN_10048790 param_9 == 0). */
	gEngfuncs.pfnServerCmd( "efw_pause 0\n" );
	g_storyPauseSent = 0;
	EFW_ClearCaption();
}

static void EFW_OpenStoryboard( int code )
{
	const char *spr = NULL;
	g_storyChange[0] = '\0';
	g_storyCode = code;
	switch( code )
	{
	case 0x3f:
		spr = "Storyboard/EFW_Storyboards_Hiding_Day.spr";
		break;
	case 0x43:
		spr = "Storyboard/EFW_Storyboards_Hiding_Night.spr";
		break;
	case 0x52:
		spr = "Storyboard/EFW_Storyboards_Help_Screen.spr";
		break;
	case 0x46:
	case 0x51:
		spr = "Storyboard/EFW_Storyboards_Isolation.spr";
		if( code == 0x46 )
			strncpy( g_storyChange, "efw_changelevel efw_prototype_level2", sizeof( g_storyChange ) - 1 );
		break;
	case 0x49:
		spr = "Storyboard/EFW_Storyboards_Introduction_1.spr";
		break;
	case 0x4a:
		spr = "Storyboard/EFW_Storyboards_Introduction_2.spr";
		break;
	case 0x4b:
		spr = "Storyboard/EFW_Storyboards_Introduction_3.spr";
		break;
	case 0x4c:
		spr = "Storyboard/EFW_Storyboards_Ending_Positive.spr";
		strncpy( g_storyChange, "efw_changelevel efw_prototype_level1", sizeof( g_storyChange ) - 1 );
		break;
	case 0x4d:
		spr = "Storyboard/EFW_SB_Ending_Iso.spr";
		break;
	case 0x4e:
		spr = "Storyboard/EFW_SB_Ending_Dep.spr";
		strncpy( g_storyChange, "efw_changelevel efw_prototype_level1", sizeof( g_storyChange ) - 1 );
		break;
	case 0x4f:
		spr = "Storyboard/EFW_Storyboards_Decoy_Remove.spr";
		strncpy( g_storyChange, "efw_changelevel efw_prototype_level2", sizeof( g_storyChange ) - 1 );
		break;
	case 0x50:
		spr = "Storyboard/EFW_Storyboards_Dec_Replace.spr";
		strncpy( g_storyChange, "efw_changelevel efw_prototype_level3", sizeof( g_storyChange ) - 1 );
		break;
	case 0x48:
		/* FUN_10047830: 0.4s debounce on DAT_100bc9f4, then FUN_10048650
		   0xd4 Panel. FUN_10048710: ClientCmd efw_pause 1 + FUN_10046370. */
		{
			float now = gHUD.m_flTime;
			float dt = now - g_contextDismissAt;
			g_storyCode = 0;
			g_hStory = 0;
			if( dt >= 0.0f && dt <= 0.4f )
				return;
			g_contextMode = 1;
			gEngfuncs.Con_Printf( ">>> FUN_10046370 n=%d\n", g_scanCount );
			if( g_storyPauseSent != code )
			{
				g_storyPauseSent = code;
				gEngfuncs.pfnServerCmd( "efw_pause 1\n" );
			}
			return;
		}
	default:
		/* FUN_10047830 else: FUN_10048790 caption Panel, not a SPR. */
		EFW_OpenCaption( code );
		return;
	}
	EFW_ClearCaption();
	/* FUN_10048590: ClientCmd efw_pause 1 once per Panel show. Send
	   before SPR_Load so a blocking storyboard sprite cannot starve the
	   pause command on the WASM main thread. */
	if( g_storyPauseSent != code )
	{
		g_storyPauseSent = code;
		gEngfuncs.pfnServerCmd( "efw_pause 1\n" );
	}
	g_storyFade = 0.0f; /* FUN_10048590 DAT_100baf10 = 0 */
	g_hStory = EFW_LoadSpr( spr );
}

/* FUN_10043a10 / Panel dtor 0x10045899: DAT_1007ab5c = -1 so tiles stop. */
static void EFW_LeaveContext( void )
{
	/* FUN_10048740: ClientCmd efw_pause 0, then FUN_100463c0. */
	if( !g_contextMode )
		return;
	g_contextMode = 0;
	g_contextDismissAt = gHUD.m_flTime;
	g_storyPauseSent = 0;
	gEngfuncs.pfnServerCmd( "efw_pause 0\n" );
	gEngfuncs.Con_Printf( ">>> FUN_100463c0\n" );
}

static void EFW_ClearStoryboard( void )
{
	g_storyCode = 0;
	g_storyPauseSent = 0;
	g_hStory = 0;
	g_storyFade = 0.0f;
	g_menuCode = 0;
	g_storyChange[0] = '\0';
	EFW_LeaveContext();
	/* Caption Panel is a different object; dtor of a SPR panel does not
	   clear DAT_100baf08. Pause falling-edge handles caption separately. */
}

static void EFW_DismissStoryboard( void )
{
	/* FUN_100485d0: storyboard InputSignal ClientCmd efw_pause 0, then
	   the stored changelevel string. */
	gEngfuncs.pfnServerCmd( "efw_pause 0\n" );
	if( g_storyChange[0] )
	{
		char buf[80];
		snprintf( buf, sizeof( buf ), "%s\n", g_storyChange );
		gEngfuncs.pfnServerCmd( buf );
	}
	EFW_ClearStoryboard();
}

static int __MsgFunc_EFW_Menu( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	g_menuCode = READ_BYTE();
	EFW_OpenStoryboard( g_menuCode );
	return 1;
}

static int __MsgFunc_EFW_CtPrv( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	g_talkPrompt = READ_BYTE();
	return 1;
}

static int __MsgFunc_EFW_Cntxt( const char *pszName, int iSize, void *pbuf )
{
	int i;
	int n;
	unsigned char *raw;
	BEGIN_READ( pbuf, iSize );
	g_scanCount = READ_BYTE();
	if( g_scanCount < 0 )
		g_scanCount = 0;
	if( g_scanCount > EFW_MAX_SCAN )
		g_scanCount = EFW_MAX_SCAN;
	memset( g_scan, 0, sizeof( g_scan ) );
	n = g_scanCount * EFW_SCAN_BYTES;
	if( n > iSize - 1 )
		n = iSize - 1;
	raw = (unsigned char *)g_scan;
	for( i = 0; i < n; i++ )
		raw[i] = (unsigned char)READ_BYTE();
	{
		static int s_cntxt;
		if( g_scanCount != s_cntxt )
		{
			s_cntxt = g_scanCount;
			gEngfuncs.Con_Printf( ">>> Cntxt n=%d %s\n", g_scanCount,
				g_scanCount > 0 && g_scan[0].name[0] ? g_scan[0].name : "" );
		}
	}
	return 1;
}

int EFW_ClientKey( int down, int keynum )
{
	int slot = 0;
	char buf[32];
	int i;
	if( !down )
		return 1;
	if( g_storyCode )
	{
		EFW_DismissStoryboard();
		return 0;
	}
	if( g_captionLen && ( keynum == K_MOUSE1 || keynum == K_MOUSE2 ) )
	{
		EFW_DismissCaption();
		return 0;
	}
	if( g_contextMode && ( keynum == K_MOUSE1 || keynum == K_MOUSE2 ) )
	{
		int best = -1;
		int bestD = 80 * 80;
		int cx = ScreenWidth / 2;
		int cy = ScreenHeight / 2;
		char cmd[96];
		for( i = 0; i < g_vguiN; i++ )
		{
			EfwVguiBtn *b = &g_vgui[i];
			int mx = b->x + b->w / 2;
			int my = b->y + b->h / 2;
			int d = ( mx - cx ) * ( mx - cx ) + ( my - cy ) * ( my - cy );
			if( d < bestD && b->cmd[0] )
			{
				bestD = d;
				best = i;
			}
		}
		if( best >= 0 )
		{
			snprintf( cmd, sizeof( cmd ), "%s\n", g_vgui[best].cmd );
			gEngfuncs.pfnServerCmd( cmd );
			return 0;
		}
		EFW_LeaveContext();
		return 0;
	}
	if( keynum == K_MOUSE1 || keynum == K_MOUSE2 )
	{
		/* Original VGUI CommandButtons eat the click. Under pointer-lock
		   pick the widget nearest the crosshair (FUN_10044f70 screen pos). */
		int best = -1;
		int bestD = 80 * 80;
		int cx = ScreenWidth / 2;
		int cy = ScreenHeight / 2;
		char cmd[96];
		for( i = 0; i < g_vguiN; i++ )
		{
			EfwVguiBtn *b = &g_vgui[i];
			int mx = b->x + b->w / 2;
			int my = b->y + b->h / 2;
			int d = ( mx - cx ) * ( mx - cx ) + ( my - cy ) * ( my - cy );
			if( d < bestD && b->cmd[0] )
			{
				bestD = d;
				best = i;
			}
		}
		if( best >= 0 )
		{
			snprintf( cmd, sizeof( cmd ), "%s\n", g_vgui[best].cmd );
			gEngfuncs.pfnServerCmd( cmd );
			return 0;
		}
	}
	if( keynum >= '1' && keynum <= '9' )
		slot = keynum - '0';
	else if( keynum >= 1 && keynum <= 9 )
		slot = keynum;
	if( slot && g_menuOn )
	{
		snprintf( buf, sizeof( buf ), "menuselect %d\n", slot );
		gEngfuncs.pfnServerCmd( buf );
		return 0;
	}
	if( keynum == 'i' || keynum == 'I' )
	{
		gEngfuncs.pfnServerCmd( "efw_diary\n" );
		return 0;
	}
	return 1;
}

int CHudEfw::Init( void )
{
	g_hope = -1;
	g_diaryPage = 0;
	g_diaryOpen = 0;
	g_mapLevel = 0;
	g_talkPrompt = 0;
	g_menuOn = 0;
	g_scanCount = 0;
	g_storyCode = 0;
	g_contextMode = 0;
	EFW_ClearCaption();
	g_menuVeil = 0.0f;
	g_invFade = 0.0f;
	g_diaryFade = 0.0f;
	g_diaryFadePage = 0;
	g_weaponId = -1;
	g_weaponMask = 0;
	g_vguiN = 0;
	g_vguiSig[0] = '\0';
	memset( g_menuLine, 0, sizeof( g_menuLine ) );
	memset( g_scan, 0, sizeof( g_scan ) );
	gEngfuncs.pfnHookUserMsg( "EFWData", __MsgFunc_EFWData );
	gEngfuncs.pfnHookUserMsg( "EFWShow", __MsgFunc_EFWShow );
	gEngfuncs.pfnHookUserMsg( "EFW_Menu", __MsgFunc_EFW_Menu );
	gEngfuncs.pfnHookUserMsg( "EFW_CtPrv", __MsgFunc_EFW_CtPrv );
	gEngfuncs.pfnHookUserMsg( "EFW_Cntxt", __MsgFunc_EFW_Cntxt );
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem( this );
	gEngfuncs.Con_Printf( "efw: HUD_Init\n" );
	return 1;
}

int CHudEfw::VidInit( void )
{
	gEngfuncs.Con_Printf( "efw: HUD_VidInit\n" );
	g_hDiary = 0;
	g_hLogo = 0;
	g_loadedPage = -1;
	g_hStory = 0;
	g_hGrey = 0;
	g_hAscale = 0;
	g_storyFade = 0.0f;
	g_menuVeil = 0.0f;
	g_invFade = 0.0f;
	g_diaryFade = 0.0f;
	g_diaryFadePage = 0;
	/* SPR_Load on the software renderer can stall the first ClientFrame.
	   Defer bubble/hide/give icons until Draw has returned a few times. */
	g_hBubble = 0;
	g_hHide = 0;
	g_hPliers = 0;
	g_hGive = 0;
	return 1;
}

void CHudEfw::Reset( void )
{
	g_hope = -1;
	g_talkPrompt = 0;
	g_menuOn = 0;
	g_scanCount = 0;
	g_storyCode = 0;
	g_contextMode = 0;
	EFW_ClearCaption();
	memset( g_menuLine, 0, sizeof( g_menuLine ) );
	memset( g_scan, 0, sizeof( g_scan ) );
}

static int EFW_LoadDiarySprite( int page, HSPRITE *out )
{
	char path[64];
	HSPRITE spr;
	snprintf( path, sizeof( path ), "sprites/efw_diary_%02d%s.spr", page, "" );
	spr = SPR_Load( path );
	if( !spr )
	{
		snprintf( path, sizeof( path ), "sprites/EFW_Diary_%02d.spr", page );
		spr = SPR_Load( path );
	}
	if( !spr )
	{
		snprintf( path, sizeof( path ), "sprites/efw_diary_%02d.spr", page );
		spr = SPR_Load( path );
	}
	if( spr && out )
		*out = spr;
	return spr != 0;
}

static int EFW_Project( float wx, float wy, float wz, int *sx, int *sy )
{
	float world[3];
	float screen[3];
	float angles[3];
	float fwd[3], right[3], up[3], org[3], delta[3];
	cl_entity_t *lp;
	float z, px, py;

	world[0] = wx;
	world[1] = wy;
	world[2] = wz;
	if( gEngfuncs.pTriAPI && gEngfuncs.pTriAPI->WorldToScreen )
	{
		if( !gEngfuncs.pTriAPI->WorldToScreen( world, screen ) )
		{
			*sx = (int)( XPROJECT( screen[0] ) );
			*sy = (int)( YPROJECT( screen[1] ) );
			/* FUN_10044f70: drop CommandButtons within 90px of any edge. */
			if( *sx <= 90 || *sy <= 90
				|| *sx >= ScreenWidth - 90 || *sy >= ScreenHeight - 90 )
				return 0;
			return 1;
		}
	}

	/* client.dll FUN_10044f70 projects bubbles in view space when the
	   triangle API clips (common in the WASM GL path). */
	lp = gEngfuncs.GetLocalPlayer();
	if( !lp )
		return 0;
	gEngfuncs.GetViewAngles( angles );
	AngleVectors( angles, fwd, right, up );
	org[0] = lp->origin[0];
	org[1] = lp->origin[1];
	org[2] = lp->origin[2] + 28.0f;
	delta[0] = wx - org[0];
	delta[1] = wy - org[1];
	delta[2] = wz - org[2];
	z = delta[0] * fwd[0] + delta[1] * fwd[1] + delta[2] * fwd[2];
	if( z < 16.0f )
		return 0;
	px = ( delta[0] * right[0] + delta[1] * right[1] + delta[2] * right[2] ) / z;
	py = ( delta[0] * up[0] + delta[1] * up[1] + delta[2] * up[2] ) / z;
	*sx = (int)( ScreenWidth * 0.5f + px * ScreenWidth * 0.5f );
	*sy = (int)( ScreenHeight * 0.5f - py * ScreenWidth * 0.5f );
	/* FUN_10044f70: drop CommandButtons within 90px of any edge, then
	   clamp the rest into the 180px inset (EFW_VguiAdd). */
	if( *sx <= 90 || *sy <= 90 || *sx >= ScreenWidth - 90 || *sy >= ScreenHeight - 90 )
		return 0;
	return 1;
}

static void EFW_DrawPrompt( int x, int y, const char *label, HSPRITE icon, int r, int g, int b )
{
	wrect_t rc;
	int dw, dh;
	if( icon )
	{
		dw = SPR_Width( icon, 0 );
		dh = SPR_Height( icon, 0 );
		if( dw < 1 )
			dw = 32;
		if( dh < 1 )
			dh = 32;
		rc.left = 0;
		rc.top = 0;
		rc.right = dw;
		rc.bottom = dh;
		SPR_Set( icon, 255, 255, 255 );
		SPR_DrawHoles( 0, x - dw / 2, y - dh - 4, &rc );
		y += 4;
	}
	else
	{
		FillRGBA( x - 40, y - 18, 80, 16, 0, 0, 0, 160 );
	}
	if( label && label[0] )
		gHUD.DrawHudString( x - 60, y, x + 120, label, r, g, b );
}

static int EFW_HasWep( int id )
{
	if( id < 16 || id > 31 )
		return 0;
	if( g_weaponMask & ( 1 << ( id - 16 ) ) )
		return 1;
	return g_weaponId == id;
}

/* FUN_100c43b0 table 0x100f81e0, ids 16..24 */
static const char *EFW_WepLabel( int id )
{
	static const char *kNames[] = {
		"pliers", "lever", "branch", "phone", "ID tag",
		"red card", "green card", "blue card", "powder"
	};
	if( id < 16 || id > 24 )
		return "item";
	return kNames[id - 16];
}

static void EFW_VguiAdd( int x, int y, const char *label, const char *cmd, HSPRITE icon )
{
	EfwVguiBtn *b;
	int w = 168;
	int h = 28;
	if( g_vguiN >= EFW_VGUI_MAX )
		return;
	if( x < 180 )
		x = 180;
	if( y < 180 )
		y = 180;
	if( x > ScreenWidth - 180 )
		x = ScreenWidth - 180;
	if( y > ScreenHeight - 180 )
		y = ScreenHeight - 180;
	/* FUN_10044f70 stacks CommandButtons; clamp can collapse several
	   world prompts onto one pixel, so stagger after the 180px box. */
	y += g_vguiN * 32;
	if( y > ScreenHeight - 32 )
		y = 32 + ( g_vguiN % 10 ) * 32;
	b = &g_vgui[g_vguiN++];
	b->w = w;
	b->h = h;
	b->x = x - w / 2;
	b->y = y - h;
	b->icon = icon;
	strncpy( b->label, label ? label : "", sizeof( b->label ) - 1 );
	b->label[sizeof( b->label ) - 1] = '\0';
	strncpy( b->cmd, cmd ? cmd : "", sizeof( b->cmd ) - 1 );
	b->cmd[sizeof( b->cmd ) - 1] = '\0';
}

static void EFW_VguiEmit( FILE *fp, const char *line )
{
	gEngfuncs.Con_Printf( "%s\n", line );
	printf( "%s\n", line );
	fprintf( stderr, "%s\n", line );
	if( fp )
		fprintf( fp, "%s\n", line );
}

static void EFW_VguiSync( void )
{
	char sig[512];
	char line[384];
	int i;
	int n;
	FILE *fp;
	sig[0] = '\0';
	n = 0;
	for( i = 0; i < g_vguiN; i++ )
	{
		n += snprintf( sig + n, sizeof( sig ) - n, "%s|", g_vgui[i].cmd );
		if( n >= (int)sizeof( sig ) - 1 )
			break;
	}
	if( !strcmp( sig, g_vguiSig ) )
		return;
	strncpy( g_vguiSig, sig, sizeof( g_vguiSig ) - 1 );
	g_vguiSig[sizeof( g_vguiSig ) - 1] = '\0';
	/* WASM HTML overlay reads this MEMFS file; Win32 used VGUI CommandButtons. */
	fp = fopen( "/efwvgui.txt", "w" );
	EFW_VguiEmit( fp, "EFWVGUI CLR" );
	for( i = 0; i < g_vguiN; i++ )
	{
		EfwVguiBtn *b = &g_vgui[i];
		snprintf( line, sizeof( line ), "EFWVGUI ADD %.4f %.4f %.4f %.4f %s\t%s",
			(float)b->x / (float)ScreenWidth,
			(float)b->y / (float)ScreenHeight,
			(float)b->w / (float)ScreenWidth,
			(float)b->h / (float)ScreenHeight,
			b->cmd, b->label );
		EFW_VguiEmit( fp, line );
	}
	if( fp )
	{
		fflush( fp );
		fclose( fp );
	}
	fflush( stdout );
	fflush( stderr );
	gEngfuncs.Con_Printf( "efw: vgui buttons=%d\n", g_vguiN );
}

static void EFW_BuildVgui( const EfwScanSlot *s, int x, int y )
{
	char label[64];
	char cmd[96];
	if( s->type == 0 )
	{
		int id;
		snprintf( label, sizeof( label ), "Talk to %s", s->name[0] ? s->name : "them" );
		snprintf( cmd, sizeof( cmd ), "efw_Talk %s", s->name[0] ? s->name : "" );
		EFW_VguiAdd( x, y, label, cmd, g_hBubble );
		/* FUN_10044f70 walks DAT_100a37a8 weapon slots 16..24, not just FirstWep. */
		for( id = 16; id <= 24; id++ )
		{
			if( !EFW_HasWep( id ) )
				continue;
			snprintf( label, sizeof( label ), "Give %s to %s", EFW_WepLabel( id ), s->name[0] ? s->name : "them" );
			snprintf( cmd, sizeof( cmd ), "efw_Give %d %s", id, s->name[0] ? s->name : "" );
			EFW_VguiAdd( x, y + 30, label, cmd, g_hGive );
		}
		return;
	}
	if( s->type == 1 )
	{
		if( !strcmp( s->name, "efw_IDTag_Position" ) )
		{
			if( EFW_HasWep( 20 ) )
			{
				snprintf( label, sizeof( label ), "Place %s on fence", EFW_WepLabel( 20 ) );
				snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %d %s", 20, s->name );
				EFW_VguiAdd( x, y, label, cmd, 0 );
			}
			else
			{
				snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %s", s->name );
				EFW_VguiAdd( x, y, "Take ID from fence", cmd, 0 );
			}
		}
		else if( !strcmp( s->name, "efw_kitchen_bin" ) )
		{
			if( EFW_HasWep( 16 ) )
			{
				snprintf( label, sizeof( label ), "Hide %s in bin", EFW_WepLabel( 16 ) );
				snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %d %s", 16, s->name );
				EFW_VguiAdd( x, y, label, cmd, 0 );
			}
		}
		else if( !strcmp( s->name, "efw_hiding_place" ) )
		{
			EFW_VguiAdd( x, y, "Hide under the building", "efw_HideUnderBuilding", g_hHide );
		}
		else if( !strcmp( s->name, "efw_PliersMarker" ) )
		{
			EFW_VguiAdd( x, y, "Take pliers", "efw_PickupPliers", g_hPliers );
		}
		else if( !strcmp( s->name, "efw_cage_door" ) )
		{
			if( EFW_HasWep( 17 ) )
			{
				snprintf( label, sizeof( label ), "Force open cage door with %s", EFW_WepLabel( 17 ) );
				snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %d %s", 17, s->name );
				EFW_VguiAdd( x, y, label, cmd, 0 );
			}
		}
		else
		{
			snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %s", s->name );
			EFW_VguiAdd( x, y, s->name, cmd, 0 );
		}
		return;
	}
	if( s->type >= 100 )
	{
		int id = s->type - 100;
		snprintf( label, sizeof( label ), "Pick up %s", EFW_WepLabel( id ) );
		snprintf( cmd, sizeof( cmd ), "efw_Pickup %u", (unsigned)id );
		EFW_VguiAdd( x, y, label, cmd, g_hPliers );
	}
}

static void EFW_DrawScanPrompts( int r, int g, int b )
{
	int i;
	g_vguiN = 0;
	for( i = 0; i < g_scanCount; i++ )
	{
		EfwScanSlot *s = &g_scan[i];
		int x, y;
		int projected = EFW_Project( s->x, s->y, s->z, &x, &y );
		if( projected )
		{
			static int s_bub;
			s_bub++;
			if( s_bub <= 3 || ( s_bub % 40 ) == 0 )
				gEngfuncs.Con_Printf( ">>> bubble %s sx=%d sy=%d type=%d\n",
					s->name[0] ? s->name : "?", x, y, s->type );
			EFW_BuildVgui( s, x, y );
		}
		/* FUN_10044f70 returns without creating CommandButtons when the
		   projection misses or sits in the 90px edge band. */
	}
	for( i = 0; i < g_vguiN; i++ )
	{
		EfwVguiBtn *btn = &g_vgui[i];
		EFW_DrawPrompt( btn->x + btn->w / 2, btn->y + btn->h, btn->label, btn->icon, r, g, b );
		FillRGBA( btn->x, btn->y, btn->w, 2, r, g, b, 180 );
	}
	EFW_VguiSync();
}

/* FUN_10043af0: 4-edge FillRGBA border of thickness param_9. */
static void EFW_DrawBoxBorder( int x, int y, int w, int h, int r, int g, int b, int a, int t )
{
	int inner;
	if( t < 1 )
		t = 1;
	FillRGBA( x, y, w, t, r, g, b, a );
	FillRGBA( x, y + h - t, w, t, r, g, b, a );
	inner = h - t * 2;
	if( inner < 1 )
		inner = 1;
	FillRGBA( x, y + t, t, inner, r, g, b, a );
	FillRGBA( x + w - t, y + t, t, inner, r, g, b, a );
}

/* FUN_10043a70: 64x64 dark tile + 3px grey frame + optional SPR at (x-32,y-32). */
static void EFW_DrawInvIcon( int x, int y, HSPRITE icon )
{
	wrect_t rc;
	int dw, dh;
	FillRGBA( x, y, 64, 64, 20, 20, 20, 255 );
	EFW_DrawBoxBorder( x, y, 64, 64, 50, 50, 50, 255, 3 );
	if( !icon )
		return;
	dw = SPR_Width( icon, 0 );
	dh = SPR_Height( icon, 0 );
	if( dw < 1 )
		dw = 64;
	if( dh < 1 )
		dh = 64;
	rc.left = 0;
	rc.top = 0;
	rc.right = dw;
	rc.bottom = dh;
	SPR_Set( icon, 255, 255, 255 );
	SPR_DrawHoles( 0, x - 32, y - 32, &rc );
}

/* FUN_10043dd0: bottom inventory strip when the diary fade is up.
   Walks DAT_100a37a8 weapon slots; x starts at 60, +128 per owned item.
   param_1 slides the strip in from the right; items only when fade==1.0. */
static void EFW_DrawInventoryStrip( float fade )
{
	int id;
	int n;
	int x;
	int y;
	int x0;
	int namesN;
	char names[160];
	static int s_invLog = -1;
	int packed;

	if( fade <= 0.0f )
		return;
	x0 = (int)( (float)ScreenWidth * ( 1.0f - fade ) );
	FillRGBA( x0, ScreenHeight - 190, ScreenWidth - x0, 190, 51, 51, 51, 204 );
	if( fade < 1.0f )
	{
		packed = (int)( fade * 10.0f + 0.5f );
		if( packed != s_invLog )
		{
			s_invLog = packed;
			gEngfuncs.Con_Printf( ">>> FUN_10043dd0 fade=%.2f\n", fade );
		}
		return;
	}
	x = 60;
	y = ScreenHeight - 190;
	n = 0;
	names[0] = '\0';
	namesN = 0;
	for( id = 16; id <= 24; id++ )
	{
		HSPRITE icon;
		if( !EFW_HasWep( id ) )
			continue;
		icon = ( id == 16 ) ? g_hPliers : 0;
		EFW_DrawInvIcon( x, y, icon );
		gHUD.DrawHudString( x, y - 45, x + 64, EFW_WepLabel( id ), 255, 255, 255 );
		if( namesN < (int)sizeof( names ) - 16 )
			namesN += snprintf( names + namesN, sizeof( names ) - namesN, "%s%s",
				n ? "," : "", EFW_WepLabel( id ) );
		n++;
		x += 128;
	}
	if( ( 100 + n ) != s_invLog )
	{
		s_invLog = 100 + n;
		gEngfuncs.Con_Printf( ">>> FUN_10043dd0 fade=1.00 n=%d %s\n", n, names );
	}
}

/* FUN_10046590: idle bar while DAT_100bc338==0 and DAT_100bc490!=0.
   DAT_100bc490 is the Cntxt scan count; DAT_100bc38c is the nearest name. */
static void EFW_DrawInteractPrompt( void )
{
	int x;
	int y;
	int w;
	int nameW;
	const char *name;
	static char s_interact[32];
	static int s_wasStory;
	static int s_noneLog;

	if( g_contextMode )
	{
		if( g_vguiN <= 0 )
		{
			gHUD.DrawHudString( ScreenWidth / 2 - 75, ScreenHeight / 2 - 100,
				ScreenWidth, "No interactive objects", 255, 127, 127 );
			if( !s_noneLog )
			{
				s_noneLog = 1;
				gEngfuncs.Con_Printf( ">>> FUN_10046590 none\n" );
			}
		}
		else
			s_noneLog = 0;
		return;
	}
	s_noneLog = 0;
	if( g_storyCode )
	{
		s_wasStory = 1;
		return;
	}
	if( s_wasStory )
	{
		s_wasStory = 0;
		s_interact[0] = '\0';
	}
	if( g_scanCount <= 0 )
	{
		if( s_interact[0] )
		{
			s_interact[0] = '\0';
			gEngfuncs.Con_Printf( ">>> FUN_10046590 interact=\n" );
		}
		return;
	}
	name = g_scan[0].name[0] ? g_scan[0].name : "";
	w = 240;
	x = ScreenWidth / 2 - 120;
	y = ScreenHeight - 25;
	FillRGBA( x, y, w, 25, 0, 0, 0, 255 );
	gHUD.DrawHudString( ScreenWidth / 2 - 115, ScreenHeight - 20, ScreenWidth,
		"Click left mouse button to interact", 255, 255, 255 );
	if( name[0] )
	{
		nameW = (int)strlen( name ) * 7;
		FillRGBA( ( ScreenWidth - nameW ) / 2 - 5, ScreenHeight - 45,
			nameW + 10, 20, 0, 0, 0, 255 );
		gHUD.DrawHudString( ( ScreenWidth - nameW ) / 2, ScreenHeight - 40,
			ScreenWidth, name, 255, 255, 255 );
	}
	if( strcmp( s_interact, name ) )
	{
		strncpy( s_interact, name, sizeof( s_interact ) - 1 );
		s_interact[sizeof( s_interact ) - 1] = '\0';
		gEngfuncs.Con_Printf( ">>> FUN_10046590 interact=%s\n", name[0] ? name : "-" );
	}
}

/* FUN_100436c0: tile sprites/efw_grey.spr over the framebuffer. */
static void EFW_DrawGreyVeil( void )
{
	int tw, th, x, y, tiles;
	wrect_t rc;

	if( !g_hGrey )
		g_hGrey = EFW_LoadSpr( "sprites/efw_grey.spr" );
	if( !g_hAscale )
	{
		g_hAscale = EFW_LoadSpr( "sprites/ascale.spr" ); /* FUN_10044e30 */
		gEngfuncs.Con_Printf( ">>> FUN_10044e30 ascale=%d grey=%d\n", g_hAscale != 0, g_hGrey != 0 );
	}
	if( !g_hGrey )
	{
		FillRGBA( 0, 0, ScreenWidth, ScreenHeight, 0, 0, 0, 220 );
		return;
	}
	tw = SPR_Width( g_hGrey, 0 );
	th = SPR_Height( g_hGrey, 0 );
	if( tw < 1 )
		tw = 16;
	if( th < 1 )
		th = 16;
	tiles = ( ScreenWidth / tw ) * ( ScreenHeight / th );
	SPR_Set( g_hGrey, 255, 255, 255 );
	rc.left = 0;
	rc.top = 0;
	rc.right = tw;
	rc.bottom = th;
	/* 16px grey is ~2500 SPR_DrawHoles on a 917x687 software present and
	   never returns. Keep the PE loop when the tile is large enough. */
	if( tiles > 80 )
	{
		FillRGBA( 0, 0, ScreenWidth, ScreenHeight, 12, 12, 12, 220 );
		SPR_DrawHoles( 0, 0, 0, &rc );
		return;
	}
	for( y = 0; y < ScreenHeight; y += th )
		for( x = 0; x < ScreenWidth; x += tw )
			SPR_DrawHoles( 0, x, y, &rc );
}

/* FUN_10043750: up to 12 frames of a 256px storyboard SPR in a 4x3 grid
   centered on an 800x600 box, last column/row clipped. */
static void EFW_DrawStoryboardTiles( HSPRITE spr )
{
	int frames, i, ox, oy, col, row, x, y, w, h;
	wrect_t rc;
	static int s_logged;

	if( !spr )
		return;
	frames = SPR_Frames( spr );
	if( frames < 1 )
		frames = 1;
	if( frames > 12 )
		frames = 12;
	ox = ( ScreenWidth - 800 ) / 2;
	oy = ( ScreenHeight - 600 ) / 2;
	SPR_Set( spr, 255, 255, 255 );
	for( i = 0; i < frames; i++ )
	{
		col = i % 4;
		row = i / 4;
		x = ox + col * 256;
		y = oy + row * 256;
		w = 256;
		h = 256;
		if( x + w > ox + 800 )
			w = ox + 800 - x;
		if( y + h > oy + 600 )
			h = oy + 600 - y;
		if( w < 1 || h < 1 )
			continue;
		rc.left = 0;
		rc.top = 0;
		rc.right = w;
		rc.bottom = h;
		SPR_DrawHoles( i, x, y, &rc );
	}
	if( s_logged != g_storyCode )
	{
		s_logged = g_storyCode;
		gEngfuncs.Con_Printf( ">>> FUN_10043750 frames=%d code=0x%x\n", frames, g_storyCode );
	}
}

/* FUN_1001e7d0: wrap at xmax-100 on space, hard wrap at xmax, 15px lines. */
static void EFW_DrawWrapped( int x, int y, int xmax, const char *text, int r, int g, int b )
{
	char line[256];
	int n = 0;
	int cx = 0;
	int cy = y;
	int i;
	int ch;
	int w;

	if( !text )
		return;
	line[0] = '\0';
	for( i = 0; text[i]; i++ )
	{
		ch = (unsigned char)text[i];
		w = 8;
		if( ch == '\n' || ( ch == ' ' && x + cx + w > xmax - 100 ) || x + cx + w > xmax )
		{
			line[n] = '\0';
			if( n )
				gHUD.DrawHudString( x, cy, xmax, line, r, g, b );
			cy += 15;
			n = 0;
			cx = 0;
			if( ch == '\n' || ch == ' ' )
				continue;
		}
		if( n < (int)sizeof( line ) - 1 )
		{
			line[n++] = (char)ch;
			cx += w;
		}
	}
	line[n] = '\0';
	if( n )
		gHUD.DrawHudString( x, cy, xmax, line, r, g, b );
}

/* FUN_10043bb0: growing center veil (FUN_1001d750 RGB 0,0,0.2 / 0,0,0.6),
   caption at x=0x4b, continue at (width-300, height-0x73) once fade>=1. */
static void EFW_DrawLetterbox( float flTime )
{
	float fade;
	float half;
	int y0;
	int y1;
	int h;
	int w;
	static int s_logged;

	if( g_captionLen <= 0 )
	{
		s_logged = 0;
		return;
	}
	h = ScreenHeight;
	w = ScreenWidth;
	if( flTime > g_captionAt )
		g_captionAge = flTime - g_captionAt;
	else
		g_captionAge += 0.05f;
	fade = g_captionAge * 3.3333333f;
	if( fade > 1.0f )
		fade = 1.0f;
	if( fade < 0.0f )
		fade = 0.0f;
	half = ( fade + 1.0f ) * 0.5f * ( (float)h - 100.0f ) * 0.5f;
	y0 = (int)( (float)h * 0.5f - half );
	y1 = (int)( (float)h * 0.5f + half );
	if( y0 < 0 )
		y0 = 0;
	if( y1 > h )
		y1 = h;
	FillRGBA( 0, y0 - 1, w, 1, 0, 0, 153, 255 );
	FillRGBA( 0, y0, w, ( h / 2 ) - y0, 0, 0, 51, 255 );
	FillRGBA( 0, h / 2, w, y1 - ( h / 2 ), 0, 0, 51, 255 );
	FillRGBA( 0, y1, w, 1, 0, 0, 153, 255 );
	EFW_DrawWrapped( 0x4b, y0, w - 0x4b, g_caption, 255, 255, 255 );
	if( fade >= 1.0f )
	{
		gHUD.DrawHudString( w - 300, h - 0x73, w,
			"Press left mouse button to continue", 255, 255, 255 );
		if( !s_logged )
		{
			s_logged = 1;
			gEngfuncs.Con_Printf( ">>> FUN_10043bb0 fade=1.00\n" );
		}
	}
}

/* FUN_1001db00: ((target-old)*0.2+old-0.5)*1.03+0.5, clamp 0..1. */
static float EFW_LerpHudFade( float oldv, float target )
{
	float v = ( ( ( target - oldv ) * 0.2f + oldv ) - 0.5f ) * 1.03f + 0.5f;
	if( v < 0.0f )
		v = 0.0f;
	if( v > 1.0f )
		v = 1.0f;
	return v;
}

static void EFW_DrawDiarySpr( int page, float fade )
{
	int dw, dh, dx, dy, vis;
	wrect_t rc;

	if( fade <= 0.0f )
		return;
	if( g_loadedPage != page )
	{
		g_hDiary = 0;
		EFW_LoadDiarySprite( page, &g_hDiary );
		g_loadedPage = page;
	}
	if( !g_hDiary )
		return;
	dw = SPR_Width( g_hDiary, 0 );
	dh = SPR_Height( g_hDiary, 0 );
	if( dw < 1 )
		dw = 256;
	if( dh < 1 )
		dh = 256;
	/* FUN_1001d750: x = width-sprW, y = height-sprW, y2 = y + sprH*fade. */
	dx = ScreenWidth - dw;
	dy = ScreenHeight - dw;
	vis = (int)( (float)dh * fade );
	if( vis < 1 )
		return;
	rc.left = 0;
	rc.top = 0;
	rc.right = dw;
	rc.bottom = vis;
	SPR_Set( g_hDiary, 255, 255, 255 );
	SPR_DrawHoles( 0, dx, dy, &rc );
}

/* DAT_100a95b8 veil, DAT_100a95bc inventory, DAT_100a95c4 diary SPR. */
static void EFW_TickHudFades( void )
{
	int menuOn = ( g_menuOn && !g_contextMode ) ? 1 : 0;
	int diaryOn = ( g_diaryOpen && !g_menuOn && !g_contextMode ) ? 1 : 0;
	float targetDiary = diaryOn ? 1.0f : 0.0f;
	int page = g_diaryPage;
	static int s_fadeLog = -1;
	int packed;

	g_menuVeil = EFW_LerpHudFade( g_menuVeil, menuOn ? 1.0f : 0.0f );
	g_invFade = EFW_LerpHudFade( g_invFade, targetDiary );
	g_diaryFade = EFW_LerpHudFade( g_diaryFade, targetDiary );
	if( g_diaryFadePage >= 1 && page >= 1 && g_diaryFadePage != page && g_diaryFade == 1.0f )
		g_diaryFade = 0.0f;
	else
	{
		if( g_diaryFade > 0.99f && page > 0 )
			g_diaryFadePage = page;
		if( g_diaryFade < 0.01f && page == 0 )
			g_diaryFadePage = 0;
	}
	packed = ( g_diaryFade >= 1.0f ? 11 : (int)( g_diaryFade * 10.0f + 0.5f ) ) * 1000
		+ ( g_invFade >= 1.0f ? 11 : (int)( g_invFade * 10.0f + 0.5f ) ) * 100
		+ ( g_menuVeil >= 1.0f ? 11 : (int)( g_menuVeil * 10.0f + 0.5f ) ) * 10
		+ ( g_diaryOpen ? 1 : 0 );
	if( packed != s_fadeLog )
	{
		s_fadeLog = packed;
		gEngfuncs.Con_Printf(
			">>> FUN_1001db00 diaryfade=%.2f inv=%.2f veil=%.2f page=%d\n",
			g_diaryFade, g_invFade, g_menuVeil, page );
	}
}

static void EFW_DrawMenuVeil( void )
{
	int w;
	if( g_menuVeil <= 0.0f )
		return;
	w = (int)( (float)ScreenWidth * g_menuVeil * 1.1f );
	if( w < 1 )
		return;
	FillRGBA( 0, ScreenHeight - 0x140, w, 0x140, 51, 0, 0, 255 );
}

static void EFW_DrawDiaryWipe( void )
{
	int cur = g_diaryPage;
	int prev = g_diaryFadePage;
	char dlabel[32];

	if( g_diaryFade <= 0.0f && g_invFade <= 0.0f )
		return;
	if( cur == 0 )
	{
		prev = 0;
		cur = g_diaryFadePage;
	}
	if( prev != cur && prev > 0 )
		EFW_DrawDiarySpr( prev, 1.0f );
	if( g_diaryFade > 0.0f )
		EFW_DrawDiarySpr( cur, g_diaryFade );
	if( g_diaryOpen )
	{
		snprintf( dlabel, sizeof( dlabel ), "DIARY  %d", g_diaryPage );
		gHUD.DrawHudString( ScreenWidth - 160, 32, ScreenWidth - 8, dlabel, 200, 0, 0 );
	}
}

/* FUN_1001daa0: 10×(28×6) ticks at (20,120), stacked up by 12px.
   Filled (200,0,0,255) while ticks >= i; else (200,200,200,10). */
static void EFW_DrawHopeTicks( int ticks )
{
	int i;
	int y;
	static int s_logged = -1;

	y = 0x78 - 6;
	for( i = 0; i < 10; i++ )
	{
		if( ticks < i )
			FillRGBA( 0x14, y, 0x1c, 6, 200, 200, 200, 10 );
		else
			FillRGBA( 0x14, y, 0x1c, 6, 200, 0, 0, 255 );
		y -= 0xc;
	}
	if( s_logged != ticks )
	{
		s_logged = ticks;
		gEngfuncs.Con_Printf( ">>> FUN_1001daa0 ticks=%d hope=%.1f\n", ticks, g_hope );
	}
}

/* FUN_1001db00 tail: %2d.%02d am/pm from gpGlobals time, then
   sprites/efw_artslogo.spr at (width-sprW-5, 5) with fade*100 SPR_Set. */
static void EFW_DrawArtsClock( float flTime )
{
	int secs;
	int t15;
	int hour;
	int mins;
	int rgb;
	int dw;
	int x;
	float t0;
	float fade;
	char clock[32];
	wrect_t rc;
	static int s_pack = -1;
	int pack;

	if( flTime < 0.0f )
		flTime = 0.0f;
	secs = (int)flTime;
	t15 = secs / 15;
	hour = ( g_mapLevel == 1 ? 5 : 7 ) + t15 / 15;
	mins = t15 % 60;
	snprintf( clock, sizeof( clock ), "%2d.%02d %s", hour, mins,
		g_mapLevel == 1 ? "pm" : "am" );
	t0 = 5.0f;
	fade = 1.0f - ( flTime - t0 ) * 0.2f;
	if( fade < 0.0f )
		fade = 0.0f;
	if( fade > 1.0f )
		fade = 1.0f;
	rgb = (int)( fade * 100.0f );
	if( rgb < 0 )
		rgb = 0;
	if( fade > 0.0f && !g_hLogo )
		g_hLogo = EFW_LoadSpr( "sprites/efw_artslogo.spr" );
	pack = ( fade >= 1.0f ? 11 : (int)( fade * 10.0f + 0.5f ) ) * 10000
		+ hour * 100 + mins;
	if( pack != s_pack )
	{
		s_pack = pack;
		gEngfuncs.Con_Printf( ">>> FUN_1001db00 clock=%s fade=%.2f logo=%d\n",
			clock, fade, g_hLogo != 0 );
	}
	if( fade <= 0.0f )
		return;
	dw = 64;
	if( g_hLogo )
	{
		dw = SPR_Width( g_hLogo, 0 );
		if( dw < 1 )
			dw = 64;
	}
	x = ScreenWidth - dw - 5;
	gHUD.DrawHudString( x - 72, 5, ScreenWidth - 8, clock, rgb, rgb, rgb );
	if( !g_hLogo )
		return;
	rc.left = 0;
	rc.top = 0;
	rc.right = dw;
	rc.bottom = SPR_Height( g_hLogo, 0 );
	if( rc.bottom < 1 )
		rc.bottom = 64;
	SPR_Set( g_hLogo, rgb, rgb, rgb );
	SPR_DrawHoles( 0, x, 5, &rc );
}

int CHudEfw::Draw( float flTime )
{
	static int s_drawN;
	int r, g, b;
	char label[32];
	int hope;

	s_drawN++;
	if( s_drawN <= 8 || ( s_drawN % 120 ) == 1 )
		gEngfuncs.Con_Printf( "efw: HUD_Draw n=%d\n", s_drawN );
	/* First ClientFrame never returned after this log under software
	   rasterize. Skip FillRGBA/SPR until a few frames have completed. */
	if( s_drawN <= 8 )
	{
		gEngfuncs.Con_Printf( "efw: HUD_Draw skip n=%d\n", s_drawN );
		return 1;
	}
	if( !g_hBubble )
		g_hBubble = EFW_LoadSpr( "sprites/efw_speech_bubble.spr" );
	if( !g_hHide )
		g_hHide = EFW_LoadSpr( "sprites/efw_hide_icon.spr" );
	if( !g_hPliers )
		g_hPliers = EFW_LoadSpr( "sprites/efw_item_pliers.spr" );
	if( !g_hGive )
		g_hGive = EFW_LoadSpr( "sprites/efw_give_icon.spr" );

	if( gHUD.m_iHideHUDDisplay & HIDEHUD_ALL )
		return 1;

	UnpackRGB( r, g, b, RGB_YELLOWISH );
	hope = (int)( g_hope + 0.5f );
	if( g_hope >= 0.0f )
	{
		/* FUN_10047660(1) hope float, __ftol to ticks. 10 ticks → /10. */
		EFW_DrawHopeTicks( hope / 10 );
		snprintf( label, sizeof( label ), "HOPE  %d", hope );
		gHUD.DrawHudString( 0x14 + 0x1c + 6, 0x78 - 12, ScreenWidth - 8, label, 200, 0, 0 );
		{
			static int s_hopeDraw;
			s_hopeDraw++;
			if( s_hopeDraw == 1 || ( s_hopeDraw % 60 ) == 0 )
				gEngfuncs.Con_Printf( "EFWVGUI HOPE %d\n", hope );
		}
		if( g_talkPrompt )
			gHUD.DrawHudString( 0x14 + 0x1c + 6, 0x78 + 4, ScreenWidth - 8, "TALK", r, g, b );
		if( g_diaryOpen )
			gHUD.DrawHudString( 0x14 + 0x1c + 6, 0x78 + 20, ScreenWidth - 8, "DIARY", r, g, b );
	}

	EFW_DrawArtsClock( flTime );

	EFW_DrawScanPrompts( r, g, b );
	EFW_DrawInteractPrompt();
	EFW_DrawLetterbox( flTime );

	if( g_storyCode && g_hStory )
	{
		/* FUN_10043a10: grey veil, fade DAT_100baf10 += 0.1, then FUN_10043750. */
		g_storyFade += 0.1f;
		if( g_storyFade > 1.0f )
			g_storyFade = 1.0f;
		EFW_DrawGreyVeil();
		EFW_DrawStoryboardTiles( g_hStory );
		gHUD.DrawHudString( 16, ScreenHeight - 28, ScreenWidth - 16, "Press any key", r, g, b );
		return 1;
	}

	if( g_menuCode == 0x4d && !g_hStory )
		gHUD.DrawHudString( 16, 48, ScreenWidth - 16, "Run out of hope!", r, g, b );

	EFW_TickHudFades();
	EFW_DrawMenuVeil();

	if( g_menuOn )
	{
		int row = 48;
		int li;
		if( g_menuLine[0][0] )
		{
			const char *prev = strstr( g_menuLine[0], " ... PREVIOUS QUESTION: " );
			if( prev )
			{
				char body[256];
				int n = (int)( prev - g_menuLine[0] );
				if( n >= (int)sizeof( body ) )
					n = (int)sizeof( body ) - 1;
				memcpy( body, g_menuLine[0], (size_t)n );
				body[n] = '\0';
				gHUD.DrawHudString( 16, row, ScreenWidth - 16, prev + 24, 100, 200, 100 );
				row += 16;
				gHUD.DrawHudString( 16, row, ScreenWidth - 16, body, 200, 200, 0 );
			}
			else
				gHUD.DrawHudString( 16, row, ScreenWidth - 16, g_menuLine[0], r, g, b );
			row += 16;
		}
		for( li = 1; li <= 6; li++ )
		{
			char press[280];
			if( !g_menuLine[li][0] )
				continue;
			snprintf( press, sizeof( press ), "Press %d  %s", li, g_menuLine[li] );
			gHUD.DrawHudString( 16, row, ScreenWidth - 16, press, r, g, b );
			row += 16;
		}
	}

	EFW_DrawInventoryStrip( g_invFade );
	EFW_DrawDiaryWipe();
	return 1;
}

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
static int g_talkPrompt;
static int g_menuCode;
static int g_weaponId = -1;
static HSPRITE g_hDiary;
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
static int g_storyCode;
static int g_storyPauseSent;
static char g_storyChange[64];
static int g_weaponMask;

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
	memcpy( &g_weaponId, blob + 8 + 4 * 4, sizeof( int ) );
	g_weaponMask = ( (unsigned)g_weaponId ) >> 16;
	g_weaponId = g_weaponId & 0xffff;
	if( g_weaponId == 0xffff )
		g_weaponId = -1;
	{
		int openFlag = 0;
		memcpy( &openFlag, blob + 8 + 5 * 4, sizeof( int ) );
		g_diaryOpen = openFlag != 0;
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
	case 0x47:
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
		/* FUN_10048650: 0xd4 VGUI panel (debounced 0.4s), not a storyboard sprite.
		   Server already sent EFW_Cntxt; HUD/HTML CommandButtons are the stand-in. */
		g_storyCode = 0;
		g_hStory = 0;
		return;
	default:
		g_hStory = 0;
		return;
	}
	/* FUN_10048590: ClientCmd efw_pause 1 once per Panel show. Send
	   before SPR_Load so a blocking storyboard sprite cannot starve the
	   pause command on the WASM main thread. */
	if( g_storyPauseSent != code )
	{
		g_storyPauseSent = code;
		gEngfuncs.pfnServerCmd( "efw_pause 1\n" );
	}
	g_hStory = EFW_LoadSpr( spr );
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
	g_storyCode = 0;
	g_storyPauseSent = 0;
	g_hStory = 0;
	g_menuCode = 0;
	g_storyChange[0] = '\0';
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
	g_talkPrompt = 0;
	g_menuOn = 0;
	g_scanCount = 0;
	g_storyCode = 0;
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
	g_loadedPage = -1;
	g_hStory = 0;
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

int CHudEfw::Draw( float flTime )
{
	static int s_drawN;
	int x, y, w, fill;
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
	x = ScreenWidth / 2 - 90;
	y = 12;
	if( g_hope >= 0.0f )
	{
		w = 180;
		FillRGBA( x - 2, y - 2, w + 4, 16, 0, 0, 0, 140 );
		FillRGBA( x, y, w, 12, 60, 40, 10, 180 );
		fill = ( w * hope ) / 100;
		if( fill < 0 )
			fill = 0;
		if( fill > w )
			fill = w;
		if( hope < 20 )
			FillRGBA( x, y, fill, 12, 210, 40, 20, 220 );
		else
			FillRGBA( x, y, fill, 12, 255, 155, 50, 220 );
		snprintf( label, sizeof( label ), "HOPE  %d", hope );
		gHUD.DrawHudString( x, y + 16, x + w + 80, label, r, g, b );
		{
			static int s_hopeDraw;
			s_hopeDraw++;
			if( s_hopeDraw == 1 || ( s_hopeDraw % 60 ) == 0 )
				gEngfuncs.Con_Printf( "EFWVGUI HOPE %d\n", hope );
		}
		if( g_talkPrompt )
			gHUD.DrawHudString( x + w + 88, y + 16, ScreenWidth - 8, "TALK", r, g, b );
		if( g_diaryOpen )
			gHUD.DrawHudString( x + w + 88, y + 32, ScreenWidth - 8, "DIARY", r, g, b );
	}

	EFW_DrawScanPrompts( r, g, b );

	if( g_storyCode && g_hStory )
	{
		int dw, dh, dx, dy;
		wrect_t rc;
		dw = SPR_Width( g_hStory, 0 );
		dh = SPR_Height( g_hStory, 0 );
		if( dw < 1 )
			dw = ScreenWidth;
		if( dh < 1 )
			dh = ScreenHeight;
		dx = ( ScreenWidth - dw ) / 2;
		dy = ( ScreenHeight - dh ) / 2;
		rc.left = 0;
		rc.top = 0;
		rc.right = dw;
		rc.bottom = dh;
		FillRGBA( 0, 0, ScreenWidth, ScreenHeight, 0, 0, 0, 220 );
		SPR_Set( g_hStory, 255, 255, 255 );
		SPR_DrawHoles( 0, dx, dy, &rc );
		gHUD.DrawHudString( 16, ScreenHeight - 28, ScreenWidth - 16, "Press any key", r, g, b );
		return 1;
	}

	if( g_menuCode == 0x4d && !g_hStory )
		gHUD.DrawHudString( 16, 48, ScreenWidth - 16, "Run out of hope!", r, g, b );

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

	if( g_diaryOpen && g_diaryPage >= 0 )
	{
		int dw, dh, dx, dy;
		wrect_t rc;
		char dlabel[32];
		if( g_loadedPage != g_diaryPage )
		{
			g_hDiary = 0;
			EFW_LoadDiarySprite( g_diaryPage, &g_hDiary );
			g_loadedPage = g_diaryPage;
		}
		snprintf( dlabel, sizeof( dlabel ), "DIARY  %d", g_diaryPage );
		gHUD.DrawHudString( ScreenWidth - 160, 32, ScreenWidth - 8, dlabel, r, g, b );
		if( g_hDiary )
		{
			dw = SPR_Width( g_hDiary, 0 );
			dh = SPR_Height( g_hDiary, 0 );
			if( dw < 1 )
				dw = 256;
			if( dh < 1 )
				dh = 256;
			dx = ScreenWidth - dw - 16;
			dy = 48;
			rc.left = 0;
			rc.top = 0;
			rc.right = dw;
			rc.bottom = dh;
			SPR_Set( g_hDiary, 255, 255, 255 );
			SPR_DrawHoles( 0, dx, dy, &rc );
		}
	}
	return 1;
}

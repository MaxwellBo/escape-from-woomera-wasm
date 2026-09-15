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
static HSPRITE g_hStory;
static int g_storyCode;
static char g_storyChange[64];

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
	{
		int openFlag = 0;
		memcpy( &openFlag, blob + 8 + 5 * 4, sizeof( int ) );
		g_diaryOpen = openFlag != 0;
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
		spr = "Storyboard/EFW_Sb_Ending_Iso.spr";
		break;
	case 0x4e:
		spr = "Storyboard/EFW_Sb_Ending_Dep.spr";
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
	default:
		g_hStory = 0;
		return;
	}
	g_hStory = EFW_LoadSpr( spr );
}

static void EFW_DismissStoryboard( void )
{
	if( g_storyChange[0] )
	{
		char buf[80];
		snprintf( buf, sizeof( buf ), "%s\n", g_storyChange );
		gEngfuncs.pfnServerCmd( buf );
	}
	g_storyCode = 0;
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
	return 1;
}

int EFW_ClientKey( int down, int keynum )
{
	int slot = 0;
	char buf[32];
	if( !down )
		return 1;
	if( g_storyCode )
	{
		EFW_DismissStoryboard();
		return 0;
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
	memset( g_menuLine, 0, sizeof( g_menuLine ) );
	memset( g_scan, 0, sizeof( g_scan ) );
	gEngfuncs.pfnHookUserMsg( "EFWData", __MsgFunc_EFWData );
	gEngfuncs.pfnHookUserMsg( "EFWShow", __MsgFunc_EFWShow );
	gEngfuncs.pfnHookUserMsg( "EFW_Menu", __MsgFunc_EFW_Menu );
	gEngfuncs.pfnHookUserMsg( "EFW_CtPrv", __MsgFunc_EFW_CtPrv );
	gEngfuncs.pfnHookUserMsg( "EFW_Cntxt", __MsgFunc_EFW_Cntxt );
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem( this );
	return 1;
}

int CHudEfw::VidInit( void )
{
	g_hDiary = 0;
	g_loadedPage = -1;
	g_hBubble = EFW_LoadSpr( "sprites/efw_speech_bubble.spr" );
	g_hHide = EFW_LoadSpr( "sprites/efw_hide_icon.spr" );
	g_hPliers = EFW_LoadSpr( "sprites/efw_item_pliers.spr" );
	g_hStory = 0;
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
	if( !gEngfuncs.pTriAPI || !gEngfuncs.pTriAPI->WorldToScreen )
		return 0;
	world[0] = wx;
	world[1] = wy;
	world[2] = wz;
	if( gEngfuncs.pTriAPI->WorldToScreen( world, screen ) )
		return 0;
	*sx = (int)( XPROJECT( screen[0] ) );
	*sy = (int)( YPROJECT( screen[1] ) );
	if( *sx < 8 || *sy < 8 || *sx > ScreenWidth - 8 || *sy > ScreenHeight - 8 )
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

static void EFW_DrawScanPrompts( int r, int g, int b )
{
	int i;
	for( i = 0; i < g_scanCount; i++ )
	{
		EfwScanSlot *s = &g_scan[i];
		int x, y;
		char label[64];
		HSPRITE icon = 0;
		label[0] = '\0';
		if( !EFW_Project( s->x, s->y, s->z, &x, &y ) )
			continue;
		if( s->type == 0 )
		{
			snprintf( label, sizeof( label ), "Talk to %s", s->name[0] ? s->name : "them" );
			icon = g_hBubble;
			EFW_DrawPrompt( x, y, label, icon, r, g, b );
			if( g_weaponId >= 16 )
			{
				snprintf( label, sizeof( label ), "Give to %s", s->name );
				EFW_DrawPrompt( x, y + 18, label, 0, r, g, b );
			}
		}
		else if( s->type == 1 )
		{
			if( !strcmp( s->name, "efw_IDTag_Position" ) )
				snprintf( label, sizeof( label ), "Place ID on fence" );
			else if( !strcmp( s->name, "efw_kitchen_bin" ) )
				snprintf( label, sizeof( label ), "Hide in bin" );
			else if( !strcmp( s->name, "efw_hiding_place" ) )
			{
				snprintf( label, sizeof( label ), "Hide under the building" );
				icon = g_hHide;
			}
			else if( !strcmp( s->name, "efw_PliersMarker" ) )
			{
				snprintf( label, sizeof( label ), "Take pliers" );
				icon = g_hPliers;
			}
			else if( !strcmp( s->name, "efw_cage_door" ) )
				snprintf( label, sizeof( label ), "Use with marker" );
			else
				snprintf( label, sizeof( label ), "%s", s->name );
			EFW_DrawPrompt( x, y, label, icon, r, g, b );
		}
		else if( s->type >= 100 )
		{
			snprintf( label, sizeof( label ), "Pickup" );
			EFW_DrawPrompt( x, y, label, g_hPliers, r, g, b );
		}
	}
}

int CHudEfw::Draw( float flTime )
{
	int x, y, w, fill;
	int r, g, b;
	char label[32];
	int hope;

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

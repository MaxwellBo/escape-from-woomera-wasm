#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

// Client half of EFWData / EFWShow / EFW_Menu / EFW_CtPrv (client.dll 0x100419e0).

static float g_hope = -1;
static int g_diaryPage;
static int g_diaryOpen;
static int g_talkPrompt;
static int g_menuCode;
static HSPRITE g_hDiary;
static int g_loadedPage = -1;
static char g_menuLine[7][256];
static int g_menuOn;

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

static int __MsgFunc_EFW_Menu( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	g_menuCode = READ_BYTE();
	return 1;
}

static int __MsgFunc_EFW_CtPrv( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	g_talkPrompt = READ_BYTE();
	return 1;
}

int EFW_ClientKey( int down, int keynum )
{
	int slot = 0;
	char buf[32];
	if( !down )
		return 1;
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
	memset( g_menuLine, 0, sizeof( g_menuLine ) );
	gEngfuncs.pfnHookUserMsg( "EFWData", __MsgFunc_EFWData );
	gEngfuncs.pfnHookUserMsg( "EFWShow", __MsgFunc_EFWShow );
	gEngfuncs.pfnHookUserMsg( "EFW_Menu", __MsgFunc_EFW_Menu );
	gEngfuncs.pfnHookUserMsg( "EFW_CtPrv", __MsgFunc_EFW_CtPrv );
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem( this );
	return 1;
}

int CHudEfw::VidInit( void )
{
	g_hDiary = 0;
	g_loadedPage = -1;
	return 1;
}

void CHudEfw::Reset( void )
{
	g_hope = -1;
	g_talkPrompt = 0;
	g_menuOn = 0;
	memset( g_menuLine, 0, sizeof( g_menuLine ) );
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
	}

	if( g_menuCode == 0x4d )
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
		if( g_loadedPage != g_diaryPage )
		{
			g_hDiary = 0;
			EFW_LoadDiarySprite( g_diaryPage, &g_hDiary );
			g_loadedPage = g_diaryPage;
		}
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
		else
		{
			char dlabel[32];
			snprintf( dlabel, sizeof( dlabel ), "DIARY  %d", g_diaryPage );
			gHUD.DrawHudString( ScreenWidth - 160, 48, ScreenWidth - 8, dlabel, r, g, b );
		}
	}
	return 1;
}

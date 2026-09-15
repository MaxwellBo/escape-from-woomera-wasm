#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE( m_Hope, Hope )

static char g_efwHint[192];

static int EFW_ReadShare( const char *name, char *dst, int dstSize )
{
	const char *roots[] = { "/woomera/", "/rwdir/woomera/", "", NULL };
	int i;
	int len = 0;
	byte *p;
	if( !name || !dst || dstSize < 2 )
		return 0;
	dst[0] = '\0';
	for( i = 0; roots[i]; i++ )
	{
		char path[160];
		FILE *f;
		snprintf( path, sizeof( path ), "%s%s", roots[i], name );
		f = fopen( path, "r" );
		if( !f )
			continue;
		if( fgets( dst, dstSize, f ) )
		{
			fclose( f );
			return 1;
		}
		fclose( f );
	}
	p = gEngfuncs.COM_LoadFile( name, 5, &len );
	if( p && len > 0 )
	{
		if( len >= dstSize )
			len = dstSize - 1;
		memcpy( dst, p, (size_t)len );
		dst[len] = '\0';
		gEngfuncs.COM_FreeFile( p );
		return 1;
	}
	return 0;
}

static int __MsgFunc_EfwHint( const char *pszName, int iSize, void *pbuf )
{
	const char *s;
	BEGIN_READ( pbuf, iSize );
	s = READ_STRING();
	strncpy( g_efwHint, s ? s : "", sizeof( g_efwHint ) - 1 );
	g_efwHint[sizeof( g_efwHint ) - 1] = '\0';
	return 1;
}

static void EFW_FwdServer( void )
{
	char buf[160];
	const char *cmd = gEngfuncs.Cmd_Argv( 0 );
	if( !cmd || !cmd[0] )
		return;
	if( gEngfuncs.Cmd_Argc() > 1 )
		snprintf( buf, sizeof( buf ), "%s %s\n", cmd, gEngfuncs.Cmd_Argv( 1 ) );
	else
		snprintf( buf, sizeof( buf ), "%s\n", cmd );
	gEngfuncs.pfnServerCmd( buf );
}

int CHudHope::Init( void )
{
	m_iHope = -1;
	g_efwHint[0] = '\0';
	HOOK_MESSAGE( Hope );
	gEngfuncs.pfnHookUserMsg( "EfwHint", __MsgFunc_EfwHint );
	gEngfuncs.pfnAddCommand( "efw_Talk", EFW_FwdServer );
	gEngfuncs.pfnAddCommand( "efw_choose", EFW_FwdServer );
	gEngfuncs.pfnAddCommand( "efw_diary", EFW_FwdServer );
	gEngfuncs.pfnAddCommand( "efw_diary_next", EFW_FwdServer );
	gEngfuncs.pfnAddCommand( "efw_diary_prev", EFW_FwdServer );
	gEngfuncs.pfnAddCommand( "efw_spider", EFW_FwdServer );
	gEngfuncs.pfnAddCommand( "efw_HelpScreen", EFW_FwdServer );
	gEngfuncs.pfnAddCommand( "efw_Give", EFW_FwdServer );
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem( this );
	return 1;
}

int CHudHope::VidInit( void )
{
	return 1;
}

void CHudHope::Reset( void )
{
	m_iHope = -1;
	g_efwHint[0] = '\0';
}

int CHudHope::MsgFunc_Hope( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iHope = READ_BYTE();
	m_iFlags |= HUD_ACTIVE;
	return 1;
}

int CHudHope::Draw( float flTime )
{
	int x, y, w, fill;
	int r, g, b;
	char label[32];
	const char *p;
	int hy;

	if( gHUD.m_iHideHUDDisplay & HIDEHUD_ALL )
		return 1;

	UnpackRGB( r, g, b, RGB_YELLOWISH );
	gHUD.DrawHudString( 8, 8, 80, "EFW", r, g, b );

	if( m_iHope < 0 )
	{
		const char *hs = gEngfuncs.pfnGetCvarString( "efw_hope" );
		if( hs && hs[0] )
			m_iHope = atoi( hs );
	}

	x = ScreenWidth / 2 - 90;
	y = 12;
	if( m_iHope >= 0 )
	{
		w = 180;
		FillRGBA( x - 2, y - 2, w + 4, 16, 0, 0, 0, 140 );
		FillRGBA( x, y, w, 12, 60, 40, 10, 180 );
		fill = ( w * m_iHope ) / 100;
		if( fill < 0 )
			fill = 0;
		if( fill > w )
			fill = w;
		if( m_iHope < 20 )
			FillRGBA( x, y, fill, 12, 210, 40, 20, 220 );
		else
			FillRGBA( x, y, fill, 12, 255, 155, 50, 220 );

		snprintf( label, sizeof( label ), "HOPE  %d", m_iHope );
		gHUD.DrawHudString( x, y + 16, x + w + 80, label, r, g, b );
		hy = y + 34;
	}
	else
		hy = y + 16;

	p = g_efwHint;
	while( *p )
	{
		char line[84];
		int n = 0;
		while( p[n] && p[n] != '\n' && n < 80 )
			n++;
		memcpy( line, p, (size_t)n );
		line[n] = '\0';
		gHUD.DrawHudString( 12, hy, ScreenWidth - 12, line, r, g, b );
		hy += 14;
		p += n;
		if( *p == '\n' )
			p++;
	}

	{
		char fileHud[192];
		char fileDiary[48];
		const char *hud = NULL;
		if( EFW_ReadShare( "efw_diary.txt", fileDiary, sizeof( fileDiary ) ) && fileDiary[0] )
		{
			int open = 0;
			int page = 1;
			unsigned mask = 0;
			char pages[80];
			int off;
			int i;
			sscanf( fileDiary, "%d %d %u", &open, &page, &mask );
			off = snprintf( pages, sizeof( pages ), "Diary%s  pages", open ? " OPEN" : "" );
			for( i = 1; i <= 12 && off < (int)sizeof( pages ) - 4; i++ )
			{
				if( mask & ( 1u << i ) )
					off += snprintf( pages + off, sizeof( pages ) - off, " %d", i );
			}
			gHUD.DrawHudString( 12, hy, ScreenWidth - 12, pages, r, g, b );
			hy += 14;
		}
		if( EFW_ReadShare( "efw_pick.txt", fileHud, sizeof( fileHud ) ) && fileHud[0] )
		{
			gHUD.DrawHudString( 12, hy, ScreenWidth - 12, fileHud, r, g, b );
			hy += 14;
		}
		if( !g_efwHint[0] && EFW_ReadShare( "efw_hud.txt", fileHud, sizeof( fileHud ) ) && fileHud[0] && strncmp( fileHud, "t=", 2 ) )
			hud = fileHud;
		if( !hud || !hud[0] )
			hud = gEngfuncs.pfnGetCvarString( "efw_hud" );
		if( hud && hud[0] )
		{
			while( *hud && hy < ScreenHeight - 20 )
			{
				char line[84];
				int n = 0;
				while( hud[n] && hud[n] != '\n' && hud[n] != '|' && n < 80 )
					n++;
				memcpy( line, hud, (size_t)n );
				line[n] = '\0';
				if( line[0] && strncmp( line, "t=", 2 ) )
				{
					gHUD.DrawHudString( 12, hy, ScreenWidth - 12, line, r, g, b );
					hy += 14;
				}
				hud += n;
				if( *hud == '\n' || *hud == '|' )
					hud++;
			}
		}
	}
	return 1;
}

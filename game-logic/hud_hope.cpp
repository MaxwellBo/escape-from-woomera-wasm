#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE( m_Hope, Hope )

static char g_efwHint[192];

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
		gHUD.DrawHudString( x, y + 16, x + w, label, r, g, b );
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
		const char *hud = gEngfuncs.pfnGetCvarString( "efw_hud" );
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
				if( line[0] )
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

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE( m_Hope, Hope )

int CHudHope::Init( void )
{
	m_iHope = 72;
	HOOK_MESSAGE( Hope );
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
	m_iHope = 72;
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

	if( gHUD.m_iHideHUDDisplay & HIDEHUD_ALL )
		return 1;

	UnpackRGB( r, g, b, RGB_YELLOWISH );
	x = ScreenWidth / 2 - 90;
	y = 12;
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
	return 1;
}

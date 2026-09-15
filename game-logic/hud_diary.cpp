#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE( m_Diary, EfwDiary )

int CHudDiary::Init( void )
{
	m_iOpen = 0;
	m_iPage = 1;
	m_iUnlocked = 1 << 1;
	m_hSprite = 0;
	m_iLoadedPage = -1;
	HOOK_MESSAGE( EfwDiary );
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem( this );
	return 1;
}

int CHudDiary::VidInit( void )
{
	m_hSprite = 0;
	m_iLoadedPage = -1;
	return 1;
}

void CHudDiary::Reset( void )
{
	m_iOpen = 0;
}

static int EFW_LoadDiarySprite( int page, HSPRITE *out )
{
	char path[64];
	HSPRITE spr;

	snprintf( path, sizeof( path ), "sprites/EFW_Diary_%02d.spr", page );
	spr = SPR_Load( path );
	if( !spr )
	{
		snprintf( path, sizeof( path ), "sprites/efw_diary_%02d.spr", page );
		spr = SPR_Load( path );
	}
	if( spr && out )
		*out = spr;
	return spr != 0;
}

int CHudDiary::MsgFunc_EfwDiary( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iOpen = READ_BYTE();
	m_iPage = READ_BYTE();
	m_iUnlocked = READ_LONG();
	if( m_iOpen )
		m_iFlags |= HUD_ACTIVE;
	else
		m_iFlags &= ~HUD_ACTIVE;
	m_iLoadedPage = -1;
	return 1;
}

int CHudDiary::Draw( float flTime )
{
	int w, h, x, y;
	wrect_t rc;
	char caption[48];

	{
		char fileSt[48];
		const char *st = NULL;
		FILE *f = fopen( "/woomera/efw_diary.txt", "r" );
		if( !f )
			f = fopen( "/rwdir/woomera/efw_diary.txt", "r" );
		if( f )
		{
			if( fgets( fileSt, sizeof( fileSt ), f ) )
				st = fileSt;
			fclose( f );
		}
		if( !st || !st[0] )
			st = gEngfuncs.pfnGetCvarString( "efw_diary_state" );
		if( st && st[0] )
		{
			int open = 0;
			int page = m_iPage;
			sscanf( st, "%d %d", &open, &page );
			m_iOpen = open;
			if( page > 0 )
				m_iPage = page;
		}
	}

	if( !m_iOpen )
		return 1;

	if( m_iLoadedPage != m_iPage )
	{
		m_hSprite = 0;
		EFW_LoadDiarySprite( m_iPage, &m_hSprite );
		m_iLoadedPage = m_iPage;
	}

	FillRGBA( 0, 0, ScreenWidth, ScreenHeight, 0, 0, 0, 140 );

	if( m_hSprite )
	{
		w = SPR_Width( m_hSprite, 0 );
		h = SPR_Height( m_hSprite, 0 );
		if( w < 8 )
			w = 256;
		if( h < 8 )
			h = 256;
		x = ( ScreenWidth - w ) / 2;
		y = ( ScreenHeight - h ) / 2 - 8;
		rc.left = 0;
		rc.top = 0;
		rc.right = w;
		rc.bottom = h;
		SPR_Set( m_hSprite, 255, 255, 255 );
		SPR_DrawHoles( 0, x, y, &rc );
	}

	snprintf( caption, sizeof( caption ), "Diary  page %d   [ / ] to turn    I close", m_iPage );
	gHUD.DrawHudString( 24, ScreenHeight - 28, ScreenWidth - 24, caption, 255, 155, 50 );
	return 1;
}

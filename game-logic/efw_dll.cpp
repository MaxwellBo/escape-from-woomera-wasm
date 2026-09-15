#ifndef CLIENT_DLL

// Port of EscapeFromWoomera.dll overlay cluster (image base 0x10000000).
// Engine imports DAT_10121e* become g_engfuncs. FindFirstFileA is a shim.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "client.h"
#include "efw_dll.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

extern int gmsgTextMsg;

int gmsgEFWShow = 0;
int gmsgEFWData = 0;
int gmsgEFWMenu = 0;
int gmsgEFWCntxt = 0;
int gmsgEFWCtPrv = 0;

static EfwDllState g_efw;

EfwDllState *EFW_Dll( void )
{
	return &g_efw;
}

CBasePlayer *EFW_Player( void )
{
	return g_efw.player;
}

void EFW_SetPlayer( CBasePlayer *pPlayer )
{
	g_efw.player = pPlayer;
}

void EFW_DebugPrint( const char *fmt, ... )
{
	char buf[256];
	va_list args;
	va_start( args, fmt );
	vsnprintf( buf, sizeof( buf ), fmt, args );
	va_end( args );
	ALERT( at_console, "%s\n", buf );
}

int EFW_FStrEq( const char *a, const char *b )
{
	if( !a || !b )
		return 0;
	return !strcmp( a, b );
}

int EFW_MapLevel( void )
{
	const char *map = STRING( gpGlobals->mapname );
	if( map && !strcmp( map, "efw_prototype_level3" ) )
		return 2;
	if( map && !strcmp( map, "efw_prototype_level2" ) )
		return 1;
	return 0;
}

void EFW_SetHudFloat( int slot, float value )
{
	if( slot < 0 || slot > 1 )
		return;
	g_efw.hudFloat[slot] = value;
}

float EFW_GetHudFloat( int slot )
{
	if( slot < 0 || slot > 1 )
		return 0;
	return g_efw.hudFloat[slot];
}

void EFW_SetHudInt( int slot, int value )
{
	if( slot < 0 || slot > 6 )
		return;
	g_efw.hudInt[slot] = value;
}

int EFW_GetHudInt( int slot )
{
	if( slot < 0 || slot > 6 )
		return 0;
	return g_efw.hudInt[slot];
}

static void EFW_PackBlob( void )
{
	memset( g_efw.blob, 0, EFW_HUD_BLOB );
	memcpy( g_efw.blob, g_efw.hudFloat, sizeof( g_efw.hudFloat ) );
	memcpy( g_efw.blob + sizeof( g_efw.hudFloat ), g_efw.hudInt, sizeof( g_efw.hudInt ) );
}

void EFW_SendEfwData( void )
{
	int i;
	if( !gmsgEFWData )
		return;
	EFW_PackBlob();
	MESSAGE_BEGIN( MSG_ALL, gmsgEFWData );
		WRITE_BYTE( 1 );
		for( i = 0; i < EFW_HUD_BLOB; i++ )
			WRITE_BYTE( g_efw.blob[i] );
	MESSAGE_END();
}

void EFW_ThinkDt( void )
{
	float now = gpGlobals->time;
	float dt = gpGlobals->frametime;
	if( dt <= 0.0f )
		dt = now - g_efw.lastTime;
	if( dt < 0.0f )
		dt = 0.0f;
	if( dt > 0.2f )
		dt = 0.2f;
	g_efw.dt = dt;
	g_efw.lastTime = now;
}

void EFW_ThinkHope( void )
{
	float hope;
	float now = gpGlobals->time;
	float elapsed;

	if( g_efw.hopeClock <= 0.0f )
		g_efw.hopeClock = now;
	elapsed = now - g_efw.hopeClock;
	if( elapsed <= 0.0f )
		return;
	g_efw.hopeClock = now;
	if( elapsed > 0.2f )
		elapsed = 0.2f;
	hope = EFW_GetHudFloat( 1 );
	hope -= elapsed * ( 1.0f / 12.0f );
	if( hope < 0.0f )
		hope = 0.0f;
	if( hope > 100.0f )
		hope = 100.0f;
	EFW_SetHudFloat( 1, hope );
	if( hope <= 0.0f && g_efw.player )
	{
		EFW_FailOrNarrate( g_efw.player, 0x4d );
		EFW_DebugPrint( "Run out of hope!" );
	}
}

void EFW_SendHudState( void )
{
	int cursor;
	int page;
	if( !g_efw.player )
		return;

	EFW_ThinkDt();
	EFW_ThinkHope();
	cursor = EFW_GetHudInt( 0 );
	if( cursor < 0 || cursor >= g_efw.diaryCount )
		cursor = 0;
	page = g_efw.diaryCount ? g_efw.diaryPages[cursor] : 0;
	EFW_SetHudInt( 1, page );
	EFW_SetHudInt( 2, ( cursor < EFW_MAX_DIARY ) ? g_efw.diaryFlags[cursor] : 0 );
	EFW_SetHudInt( 3, EFW_MapLevel() );
	if( g_efw.player && g_efw.player->m_pActiveItem )
		EFW_SetHudInt( 4, g_efw.player->m_pActiveItem->m_iId );
	else
		EFW_SetHudInt( 4, -1 );
	if( gpGlobals->time - g_efw.hudRetry >= 0.1f )
	{
		g_efw.hudRetry = gpGlobals->time;
		EFW_SendEfwData();
		EFW_TalkScan();
	}
	EFW_ThinkConversation();
}

void EFW_FailOrNarrate( CBasePlayer *pPlayer, int code )
{
	if( !pPlayer || !gmsgEFWMenu )
		return;
	MESSAGE_BEGIN( MSG_ONE, gmsgEFWMenu, NULL, pPlayer->pev );
		WRITE_BYTE( code );
	MESSAGE_END();
}

int EFW_DiaryCount( void )
{
	return g_efw.diaryCount;
}

void EFW_AddDiary( int page, int mode )
{
	int i;
	for( i = 0; i < g_efw.diaryCount; i++ )
	{
		if( g_efw.diaryPages[i] == page )
			return;
	}
	if( g_efw.diaryCount >= EFW_MAX_DIARY )
		return;
	g_efw.diaryPages[g_efw.diaryCount] = page;
	if( mode == 0 )
		EFW_SetHudInt( 0, g_efw.diaryCount );
	g_efw.diaryCount++;
	EFW_DebugPrint( "Diary active item added    %d", page );
}

void EFW_AddKeyword( const char *word, int unlocked )
{
	if( !word || !word[0] || !unlocked )
		return;
	if( EFW_HasKeyword( word ) )
		return;
	if( g_efw.keywordCount >= EFW_MAX_KEYWORDS )
		return;
	strncpy( g_efw.keywords[g_efw.keywordCount], word, EFW_TOPIC_LEN - 1 );
	g_efw.keywords[g_efw.keywordCount][EFW_TOPIC_LEN - 1] = '\0';
	g_efw.keywordCount++;
}

int EFW_HasKeyword( const char *word )
{
	int i;
	if( !word )
		return 0;
	for( i = 0; i < g_efw.keywordCount; i++ )
	{
		if( !strcmp( g_efw.keywords[i], word ) )
			return 1;
	}
	return 0;
}

int EFW_HasSeen( const char *npc, const char *topic )
{
	char key[64];
	int i;
	snprintf( key, sizeof( key ), "%s:%s", npc ? npc : "", topic ? topic : "" );
	for( i = 0; i < g_efw.seenCount; i++ )
	{
		if( !strcmp( g_efw.seen[i], key ) )
			return 1;
	}
	return 0;
}

void EFW_MarkSeen( const char *npc, const char *topic )
{
	char key[64];
	if( EFW_HasSeen( npc, topic ) )
		return;
	if( g_efw.seenCount >= EFW_MAX_SEEN )
		return;
	snprintf( key, sizeof( key ), "%s:%s", npc ? npc : "", topic ? topic : "" );
	strncpy( g_efw.seen[g_efw.seenCount], key, 63 );
	g_efw.seen[g_efw.seenCount][63] = '\0';
	g_efw.seenCount++;
}

/* FUN_100c7380: EFWShow WRITE_BYTE(line) + 30-char chunk. 0xff clears. */
static void EFW_SendEfwShow( CBasePlayer *pPlayer, int code, const char *chunk )
{
	if( !pPlayer || !gmsgEFWShow )
		return;
	MESSAGE_BEGIN( MSG_ONE, gmsgEFWShow, NULL, pPlayer->pev );
		WRITE_BYTE( code & 0xff );
		if( chunk )
			WRITE_STRING( chunk );
	MESSAGE_END();
}

static void EFW_SendEfwShowChunks( CBasePlayer *pPlayer, int line, const char *text )
{
	const char *p;
	char chunk[31];
	if( !text )
		text = "";
	p = text;
	if( !*p )
	{
		EFW_SendEfwShow( pPlayer, line, "" );
		return;
	}
	while( *p )
	{
		int n = 0;
		while( p[n] && n < 30 )
		{
			chunk[n] = ( p[n] == (char)0x92 ) ? '\'' : p[n];
			n++;
		}
		chunk[n] = '\0';
		EFW_SendEfwShow( pPlayer, line, chunk );
		p += n;
	}
}

void EFW_ShowDllMenu( CBasePlayer *pPlayer, const char *title, const char **lines, int nLines )
{
	int i;
	if( !pPlayer )
		return;
	EFW_SendEfwShow( pPlayer, 0xff, NULL );
	if( ( !title || !title[0] ) && nLines <= 0 )
		return;
	EFW_SendEfwShowChunks( pPlayer, 0, title ? title : "" );
	if( nLines < 0 )
		nLines = 0;
	if( nLines > 6 )
		nLines = 6;
	for( i = 0; i < nLines; i++ )
		EFW_SendEfwShowChunks( pPlayer, i + 1, lines[i] ? lines[i] : "" );
}

void EFW_ShowGoldMenu( CBasePlayer *pPlayer, int bits, int seconds, const char *text )
{
	(void)bits;
	(void)seconds;
	EFW_ShowDllMenu( pPlayer, text, NULL, 0 );
}

void EFW_CloseMenu( CBasePlayer *pPlayer )
{
	if( !pPlayer )
		return;
	EFW_SendEfwShow( pPlayer, 0xff, NULL );
}

void EFW_Print( CBasePlayer *pPlayer, const char *text )
{
	if( !pPlayer || !text )
		return;
	CLIENT_PRINTF( pPlayer->edict(), print_console, text );
	CLIENT_PRINTF( pPlayer->edict(), print_console, "\n" );
	if( gmsgTextMsg )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgTextMsg, NULL, pPlayer->pev );
			WRITE_BYTE( HUD_PRINTTALK );
			WRITE_STRING( text );
		MESSAGE_END();
	}
}

void EFW_GiveItem( CBasePlayer *pPlayer, int itemBit, const char *weaponName )
{
	g_efw.items |= itemBit;
	if( pPlayer && weaponName )
		pPlayer->GiveNamedItem( weaponName );
}

void EFW_RunScriptAction( CBasePlayer *pPlayer, const char *action )
{
	char name[48];
	char arg[48];
	const char *open;
	const char *close;

	if( !action || !action[0] )
		return;
	if( !strcmp( action, "Goodbye" ) )
	{
		EFW_CloseTalk();
		return;
	}
	open = strchr( action, '(' );
	if( !open )
		return;
	close = strchr( open, ')' );
	if( !close )
		return;
	strncpy( name, action, (size_t)( open - action ) );
	name[open - action] = '\0';
	strncpy( arg, open + 1, (size_t)( close - open - 1 ) );
	arg[close - open - 1] = '\0';
	if( !strcmp( name, "AddTopic" ) )
		EFW_AddKeyword( arg, 1 );
	else if( !strcmp( name, "DeleteTopic" ) )
	{
		int i;
		for( i = 0; i < g_efw.keywordCount; i++ )
		{
			if( !strcmp( g_efw.keywords[i], arg ) )
			{
				g_efw.keywords[i][0] = '\0';
				break;
			}
		}
	}
	else if( !strcmp( name, "AddDiary" ) )
		EFW_AddDiary( atoi( arg ), 2 );
	else if( !strcmp( name, "ServerCommand" ) && pPlayer )
		CLIENT_COMMAND( pPlayer->edict(), arg );
}

void EFW_InitFromSpawn( CBasePlayer *pPlayer )
{
	int level;
	memset( &g_efw, 0, sizeof( g_efw ) );
	EFW_SetPlayer( pPlayer );
	EFW_SetHudFloat( 0, 100.0f ); /* FUN_100c8180(0, 0x42c80000) */
	EFW_SetHudFloat( 1, 80.0f );  /* FUN_100c8180(1, 0x42a00000) */
	g_efw.lastTime = gpGlobals->time;
	g_efw.hideDist = EFW_HIDE_DIST;
	g_efw.diaryPending = -1;
	EFW_LoadAllConversations();
	EFW_AddDiary( 0, 2 );
	EFW_AddDiary( 1, 2 );
	level = EFW_MapLevel();
	if( level == 1 || level == 2 )
	{
		int p;
		for( p = 2; p <= 10; p++ )
			EFW_AddDiary( p, 2 );
	}
	EFW_SetHudInt( 0, 1 );
	g_efw.inited = 1;
	EFW_SendHudState();
}

static void EFW_HostFwd( void )
{
	edict_t *e = INDEXENT( 1 );
	if( e && e->pvPrivateData )
		EFW_ClientCommand( e );
}

static void EFW_RegisterHostCmds( void )
{
	static int done;
	const char *cmds[] = {
		"efw_Talk", "efw_Give", "efw_spider", "efw_Pickup", "efw_UseWithMarker",
		"efw_diary", "efw_diary_next", "efw_diary_prev", "efw_ShowMenu",
		"efw_HelpScreen", "efw_HideUnderBuilding", "efw_PickupPliers",
		"efw_pause", "efw_set_state", "efw_changelevel", "menuselect", NULL
	};
	int i;
	if( done )
		return;
	done = 1;
	for( i = 0; cmds[i]; i++ )
		g_engfuncs.pfnAddServerCommand( cmds[i], EFW_HostFwd );
}

void EFW_LinkUserMessages( void )
{
	EFW_RegisterHostCmds();
	if( !gmsgEFWShow )
		gmsgEFWShow = REG_USER_MSG( "EFWShow", -1 );
	if( !gmsgEFWData )
		gmsgEFWData = REG_USER_MSG( "EFWData", -1 );
	if( !gmsgEFWMenu )
		gmsgEFWMenu = REG_USER_MSG( "EFW_Menu", 1 );
	if( !gmsgEFWCntxt )
		gmsgEFWCntxt = REG_USER_MSG( "EFW_Cntxt", -1 );
	if( !gmsgEFWCtPrv )
		gmsgEFWCtPrv = REG_USER_MSG( "EFW_CtPrv", 1 );
}

void EFW_Precache( void )
{
	PRECACHE_MODEL( "models/w_pliers.mdl" );
	PRECACHE_MODEL( "models/v_pliers.mdl" );
	PRECACHE_MODEL( "models/p_pliers.mdl" );
	PRECACHE_MODEL( "models/w_Pliers.mdl" );
	PRECACHE_MODEL( "models/v_Pliers.mdl" );
	PRECACHE_MODEL( "models/p_Pliers.mdl" );
}

void EFW_PlayerSpawn( CBasePlayer *pPlayer )
{
	EFW_LinkUserMessages();
	EFW_Precache();
	EFW_InitFromSpawn( pPlayer );
	CLIENT_COMMAND( pPlayer->edict(), "bind i efw_diary\n" );
	CLIENT_COMMAND( pPlayer->edict(), "bind [ efw_diary_prev\n" );
	CLIENT_COMMAND( pPlayer->edict(), "bind ] efw_diary_next\n" );
}

void EFW_PlayerPreThink( CBasePlayer *pPlayer )
{
	if( !pPlayer )
		return;
	if( g_efw.player != pPlayer )
		EFW_SetPlayer( pPlayer );
	if( !g_efw.inited )
		EFW_InitFromSpawn( pPlayer );
	if( g_efw.lastTime > 1.0f && gpGlobals->time + 0.5f < g_efw.lastTime )
		EFW_InitFromSpawn( pPlayer );
	if( g_efw.talkActive )
	{
		int slot = pPlayer->pev->impulse;
		if( slot >= 1 && slot <= 9 )
		{
			EFW_ChooseTalk( pPlayer, slot );
			pPlayer->pev->impulse = 0;
		}
	}
	if( g_efw.lastTime != gpGlobals->time || !g_efw.inited )
		EFW_SendHudState();
}

#endif

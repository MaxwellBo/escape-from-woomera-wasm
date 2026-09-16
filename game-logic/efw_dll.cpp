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

typedef char EFW_SCAN_SIZE_CHECK[( sizeof( EfwScanSlot ) == EFW_SCAN_BYTES ) ? 1 : -1];

EfwDllState *EFW_Dll( void )
{
	return &g_efw;
}

CBasePlayer *EFW_Player( void )
{
	edict_t *e;

	if( g_efw.player )
	{
		e = g_efw.player->edict();
		if( e && !e->free && e->pvPrivateData )
			return g_efw.player;
		g_efw.player = NULL;
	}
	e = INDEXENT( 1 );
	if( e && !e->free && e->pvPrivateData )
		return (CBasePlayer *)GET_PRIVATE( e );
	return NULL;
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
	ALERT( at_error, "%s\n", buf );
	if( g_engfuncs.pfnServerPrint )
	{
		char line[260];
		snprintf( line, sizeof( line ), "%s\n", buf );
		g_engfuncs.pfnServerPrint( line );
	}
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

void EFW_AdjustHope( float delta )
{
	float hope = EFW_GetHudFloat( 1 ) + delta;
	if( hope < 0.0f )
		hope = 0.0f;
	if( hope > 100.0f )
		hope = 100.0f;
	EFW_SetHudFloat( 1, hope );
}

void EFW_ThinkHope( void )
{
	static int s_hopeN;
	float hope;
	float now = gpGlobals->time;
	float elapsed;

	if( EFW_GetHudInt( 6 ) )
		return;
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
	s_hopeN++;
	if( s_hopeN == 1 || ( s_hopeN % 40 ) == 0 )
		EFW_DebugPrint( ">>> hope %.1f time=%.2f", hope, now );
	if( hope <= 0.0f && g_efw.player && !g_efw.hopeFailed )
	{
		g_efw.hopeFailed = 1;
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
	/* FUN_100c6b60: hudInt[2] = diaryFlags[page number], not cursor. */
	EFW_SetHudInt( 2, ( page >= 0 && page < EFW_MAX_DIARY ) ? g_efw.diaryFlags[page] : 0 );
	EFW_SetHudInt( 3, EFW_MapLevel() );
	{
		int active = 0;
		int packed;
		if( g_efw.player && g_efw.player->m_pActiveItem )
			active = g_efw.player->m_pActiveItem->m_iId;
		if( active < 0 )
			active = 0;
		packed = ( EFW_WeaponMask( g_efw.player ) << 16 ) | ( active & 0xffff );
		EFW_SetHudInt( 4, packed );
	}
	/* FUN_100c6b60 calls FUN_100c6a70 then FUN_100c6a60 (talk-menu hotkeys). */
	EFW_PollMenuKeys();
	if( gpGlobals->time - g_efw.hudRetry >= 0.1f )
	{
		g_efw.hudRetry = gpGlobals->time;
		EFW_SendEfwData();
		EFW_TalkScan();
	}
	EFW_ThinkConversation();
}

/* FUN_100c81d0: 0x3f/0x43 hope+15 then EFW_Menu; 0x3c-0x45 (not those) ShowMenu string; else EFW_Menu. */
void EFW_FailOrNarrate( CBasePlayer *pPlayer, int code )
{
	static const char *kNarrate[] = {
		/* Pointers at 0x1011d618 + code*4 + 0x24. 0x3f/0x43 still EFW_Menu. */
		/* 0x3c */ "You realise that the guard will search you and find the pliers, and so decide not to leave the kitchen.",
		/* 0x3d */ "You wait until the electrician is not looking, and quickly grab the pliers from the workbench. He doesn't notice, and you hide them under your shirt. Heart pounding, you wonder how to safely get them to Amir.",
		/* 0x3e */ "Again, you wait for the ideal moment to retrieve the pliers from under your shirt and slowly lower them into the bin, careful to not make a sound.",
		/* 0x3f */ NULL, /* Hiding_Day storyboard; table text is the ID-tag night wait */
		/* 0x40 */ "There's a hole. You could hide here, if you ever needed to.",
		/* 0x41 */ "You could hide here, but you'd be caught at dusk when the guards saw your ID tag and came searching.",
		/* 0x42 */ "You could hide here and come out at night to get the pliers, if only you had a way to break into the rubbish bin cage.",
		/* 0x43 */ NULL, /* Hiding_Night storyboard */
		/* 0x44 */ "You could hide again, but you haven't got the pliers yet.",
		/* 0x45 */ "You recognise the bin in front of you as the one from the kitchen earlier today. You open the top and dig around inside, and sure enough, the pliers are still there. You retrieve them from the foodscraps and rubbish, and hide them in your clothes. Now to work out how to safely get these back to your fellow plotters."
	};
	int idx;

	if( !pPlayer )
		return;
	EFW_DebugPrint( ">>> FailOrNarrate 0x%x", code );
	if( code == 0x3f || code == 0x43 )
		EFW_AdjustHope( 15.0f );
	idx = code - 0x3c;
	if( idx >= 0 && idx < (int)( sizeof( kNarrate ) / sizeof( kNarrate[0] ) ) && kNarrate[idx] )
	{
		EFW_ShowDllMenu( pPlayer, kNarrate[idx], NULL, 0 );
		return;
	}
	if( code != 0x47 )
		EFW_CloseTalk();
	if( !gmsgEFWMenu )
		return;
	MESSAGE_BEGIN( MSG_ONE, gmsgEFWMenu, NULL, pPlayer->pev );
		WRITE_BYTE( code & 0xff );
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
	EFW_PlayCue( "Dingaling.wav" );
	if( mode == 0 )
		EFW_SetHudInt( 0, g_efw.diaryCount );
	else if( mode == 1 )
	{
		if( g_efw.talkActive )
			g_efw.diaryPending = g_efw.diaryCount;
		else
		{
			EFW_SetHudInt( 0, g_efw.diaryCount );
			g_efw.diaryPending = -1;
		}
	}
	g_efw.diaryCount++;
	EFW_DebugPrint( "Diary active item added    %d", page );
}

void EFW_FlagDiary( int page )
{
	if( page >= 0 && page < EFW_MAX_DIARY )
		g_efw.diaryFlags[page] = 1;
}

void EFW_AddKeyword( const char *word, int unlocked )
{
	int i;
	if( !word || !word[0] )
		return;
	for( i = 0; i < g_efw.keywordCount; i++ )
	{
		if( !strcmp( g_efw.keywords[i], word ) )
		{
			g_efw.keywordUnlocked[i] = unlocked ? 1 : 0;
			return;
		}
	}
	if( g_efw.keywordCount >= EFW_MAX_KEYWORDS )
		return;
	strncpy( g_efw.keywords[g_efw.keywordCount], word, EFW_TOPIC_LEN - 1 );
	g_efw.keywords[g_efw.keywordCount][EFW_TOPIC_LEN - 1] = '\0';
	g_efw.keywordUnlocked[g_efw.keywordCount] = unlocked ? 1 : 0;
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
			return g_efw.keywordUnlocked[i];
	}
	return 0;
}

int EFW_HasWeapon( CBasePlayer *pPlayer, const char *classname )
{
	if( !pPlayer || !classname )
		return 0;
	if( pPlayer->HasNamedPlayerItem( classname ) )
		return 1;
	if( !strcmp( classname, "weapon_efw_Pliers" ) && ( g_efw.items & EFW_ITEM_PLIERS ) )
		return 1;
	if( !strcmp( classname, "weapon_efw_IDTag" ) && ( g_efw.items & EFW_ITEM_IDTAG ) )
		return 1;
	if( !strcmp( classname, "weapon_efw_Lever" ) && ( g_efw.items & EFW_ITEM_LEVER ) )
		return 1;
	if( !strcmp( classname, "weapon_efw_Branch" ) && ( g_efw.items & EFW_ITEM_BRANCH ) )
		return 1;
	if( !strcmp( classname, "weapon_efw_MobilePhone" ) && ( g_efw.items & EFW_ITEM_PHONE ) )
		return 1;
	if( !strcmp( classname, "weapon_efw_WashingPowder" ) && ( g_efw.items & EFW_ITEM_POWDER ) )
		return 1;
	return 0;
}

int EFW_WeaponTypeId( const char *classname )
{
	/* FUN_100c43b0 walks PTR 0x100f81e0..0x100f8204, id = index + 0x10. */
	static const char *kNames[] = {
		"weapon_efw_Pliers", "weapon_efw_Lever", "weapon_efw_Branch",
		"weapon_efw_MobilePhone", "weapon_efw_IDTag", "weapon_efw_RedPhoneCard",
		"weapon_efw_GreenPhoneCard", "weapon_efw_BluePhoneCard", "weapon_efw_WashingPowder"
	};
	unsigned i;
	if( !classname )
		return -1;
	if( !strcmp( classname, "weapon_efw_Pilers" ) )
		return WEAPON_EFW_PLIERS;
	for( i = 0; i < sizeof( kNames ) / sizeof( kNames[0] ); i++ )
	{
		if( !strcmp( classname, kNames[i] ) )
			return (int)i + 0x10;
	}
	return -1;
}

const char *EFW_WeaponClassname( int id )
{
	static const char *kNames[] = {
		"weapon_efw_Pliers", "weapon_efw_Lever", "weapon_efw_Branch",
		"weapon_efw_MobilePhone", "weapon_efw_IDTag", "weapon_efw_RedPhoneCard",
		"weapon_efw_GreenPhoneCard", "weapon_efw_BluePhoneCard", "weapon_efw_WashingPowder"
	};
	if( id < 0x10 || id > 0x18 )
		return NULL;
	return kNames[id - 0x10];
}

int EFW_WeaponMask( CBasePlayer *pPlayer )
{
	int mask = 0;
	int slot;
	CBasePlayerItem *pItem;

	if( g_efw.items & EFW_ITEM_PLIERS )
		mask |= 1 << ( WEAPON_EFW_PLIERS - 16 );
	if( g_efw.items & EFW_ITEM_LEVER )
		mask |= 1 << ( WEAPON_EFW_LEVER - 16 );
	if( g_efw.items & EFW_ITEM_BRANCH )
		mask |= 1 << ( WEAPON_EFW_BRANCH - 16 );
	if( g_efw.items & EFW_ITEM_PHONE )
		mask |= 1 << ( WEAPON_EFW_MOBILEPHONE - 16 );
	if( g_efw.items & EFW_ITEM_IDTAG )
		mask |= 1 << ( WEAPON_EFW_IDTAG - 16 );
	if( g_efw.items & EFW_ITEM_REDCARD )
		mask |= 1 << ( WEAPON_EFW_REDPHONECARD - 16 );
	if( g_efw.items & EFW_ITEM_GREENCARD )
		mask |= 1 << ( WEAPON_EFW_GREENPHONECARD - 16 );
	if( g_efw.items & EFW_ITEM_BLUECARD )
		mask |= 1 << ( WEAPON_EFW_BLUEPHONECARD - 16 );
	if( g_efw.items & EFW_ITEM_POWDER )
		mask |= 1 << ( WEAPON_EFW_WASHINGPOWDER - 16 );
	if( !pPlayer )
		return mask;
	for( slot = 0; slot < MAX_ITEM_TYPES; slot++ )
	{
		for( pItem = pPlayer->m_rgpPlayerItems[slot]; pItem; pItem = pItem->m_pNext )
		{
			int id = pItem->m_iId;
			if( id >= 16 && id < 32 )
				mask |= 1 << ( id - 16 );
		}
	}
	return mask;
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
	EfwDllState *st = EFW_Dll();
	if( !pPlayer )
		return;
	EFW_SendEfwShow( pPlayer, 0xff, NULL );
	if( ( !title || !title[0] ) && nLines <= 0 )
		return;
	/* efw_ShowMenu 0x100c6e60 writes DAT_1013487c = now on each show. */
	st->talkStart = gpGlobals->time;
	st->menuTitle[0] = '\0';
	if( title )
		strncpy( st->menuTitle, title, sizeof( st->menuTitle ) - 1 );
	st->menuTitle[sizeof( st->menuTitle ) - 1] = '\0';
	EFW_SendEfwShowChunks( pPlayer, 0, title ? title : "" );
	if( nLines < 0 )
		nLines = 0;
	if( nLines > 6 )
		nLines = 6;
	for( i = 0; i < EFW_MENU_LINES; i++ )
		st->menuText[i][0] = '\0';
	for( i = 0; i < nLines; i++ )
	{
		if( lines[i] )
			strncpy( st->menuText[i], lines[i], sizeof( st->menuText[i] ) - 1 );
		st->menuText[i][sizeof( st->menuText[i] ) - 1] = '\0';
		EFW_SendEfwShowChunks( pPlayer, i + 1, lines[i] ? lines[i] : "" );
	}
	EFW_DebugPrint( ">>> EFWShow lines=%d %s", nLines, st->menuTitle[0] ? st->menuTitle : "" );
	EFW_HtmlVguiSync();
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
	if( !pPlayer || !weaponName || !weaponName[0] )
		return;
	/* After ServerActivate, s_mapLive used to reject every DispatchSpawn, so
	   GiveNamedItem never created the edict. FUN_100c2f70 is HasNamedPlayerItem. */
	if( !pPlayer->HasNamedPlayerItem( weaponName ) )
		pPlayer->GiveNamedItem( weaponName );
	if( pPlayer->HasNamedPlayerItem( weaponName ) )
		pPlayer->SelectItem( weaponName );
}

void EFW_StripWeapon( CBasePlayer *pPlayer, const char *classname, int itemBit )
{
	int slot;
	CBasePlayerItem *pItem;

	g_efw.items &= ~itemBit;
	if( !pPlayer || !classname )
		return;
	for( slot = 0; slot < MAX_ITEM_TYPES; slot++ )
	{
		for( pItem = pPlayer->m_rgpPlayerItems[slot]; pItem; )
		{
			CBasePlayerItem *pNext = pItem->m_pNext;
			if( !strcmp( STRING( pItem->pev->classname ), classname ) )
				pPlayer->RemovePlayerItem( pItem, true );
			pItem = pNext;
		}
	}
}

CBaseEntity *EFW_PlaceIdTag( CBaseEntity *pTag, CBaseEntity *pMark )
{
	Vector pos;
	CBasePlayerItem *pItem;

	/* FUN_100c2a20 after FStrEq(targetname, "efw_IDTag_Position"). */
	if( !pTag || !pMark )
		return NULL;
	pos = ( pMark->pev->absmin + pMark->pev->absmax ) * 0.5f;
	if( ( pMark->pev->absmax - pMark->pev->absmin ).Length() < 1.0f )
		pos = pMark->pev->origin;
	pTag->pev->origin = pos;
	pTag->pev->angles = g_vecZero;
	pTag->pev->velocity = g_vecZero;
	pTag->pev->movetype = MOVETYPE_NONE;
	pTag->pev->effects &= ~EF_NODRAW;
	SET_MODEL( ENT( pTag->pev ), "models/w_idtag.mdl" );
	UTIL_SetOrigin( pTag->pev, pos );
	pItem = (CBasePlayerItem *)pTag;
	pItem->SetThink( NULL );
	pTag->pev->nextthink = 0;
	pItem->Materialize();
	pTag->pev->solid = SOLID_NOT;
	pTag->pev->effects |= EF_NODRAW;
	pTag->pev->dmgtime = gpGlobals->time + 0.5f; /* GetTickCount + 0x1f4 at this+0x12c */
	EFW_AddKeyword( "Player'sIDTagOnFence", 1 );
	EFW_Print( EFW_Player(), "ID Tag has been placed on the wall" );
	EFW_Squark( "efw_compound_gate_guard", "Okay RAR-124, you can pass.", 4 );
	return pTag;
}

CBaseEntity *EFW_MaterializeIdTag( CBaseEntity *pMark )
{
	edict_t *pent;
	CBaseEntity *pTag;
	Vector pos;

	/* FUN_100c27f0: CREATE weapon_efw_IDTag, then vtable+0x1a8(marker) = FUN_100c2a20. */
	if( !pMark )
		return NULL;
	pos = ( pMark->pev->absmin + pMark->pev->absmax ) * 0.5f;
	if( ( pMark->pev->absmax - pMark->pev->absmin ).Length() < 1.0f )
		pos = pMark->pev->origin;
	pent = CREATE_NAMED_ENTITY( MAKE_STRING( "weapon_efw_IDTag" ) );
	if( FNullEnt( pent ) )
		return NULL;
	pent->v.origin = pos;
	DispatchSpawn( pent );
	pTag = CBaseEntity::Instance( pent );
	if( !pTag )
		return NULL;
	return EFW_PlaceIdTag( pTag, pMark );
}

CBaseEntity *EFW_PlacePlayerIdTag( CBasePlayer *pPlayer, CBaseEntity *pMark )
{
	int slot;
	CBasePlayerItem *pItem;

	if( !pPlayer || !pMark )
		return NULL;
	for( slot = 0; slot < MAX_ITEM_TYPES; slot++ )
	{
		for( pItem = pPlayer->m_rgpPlayerItems[slot]; pItem; pItem = pItem->m_pNext )
		{
			if( strcmp( STRING( pItem->pev->classname ), "weapon_efw_IDTag" ) )
				continue;
			pPlayer->RemovePlayerItem( pItem, true );
			pItem->m_pPlayer = NULL;
			g_efw.items &= ~EFW_ITEM_IDTAG;
			return EFW_PlaceIdTag( pItem, pMark );
		}
	}
	return NULL;
}

static void EFW_SpawnFenceTag( void )
{
	CBaseEntity *pMark;

	/* FUN_100c27f0: on maplevel 2, materialize weapon_efw_IDTag at efw_IDTag_Position. */
	if( EFW_MapLevel() != 2 )
		return;
	pMark = UTIL_FindEntityByTargetname( NULL, "efw_IDTag_Position" );
	if( pMark )
		EFW_MaterializeIdTag( pMark );
}

static void EFW_TrimInPlace( char *s )
{
	char *a;
	char *b;
	if( !s )
		return;
	a = s;
	while( *a == ' ' || *a == '\t' )
		a++;
	if( a != s )
		memmove( s, a, strlen( a ) + 1 );
	b = s + strlen( s );
	while( b > s && ( b[-1] == ' ' || b[-1] == '\t' ) )
	{
		b--;
		*b = '\0';
	}
}

static int EFW_CmdIs( const char *cmd, const char *want )
{
	size_t n;
	if( !cmd || !want )
		return 0;
	n = strlen( want );
	if( strncmp( cmd, want, n ) != 0 )
		return 0;
	return cmd[n] == '\0' || cmd[n] == ' ' || cmd[n] == '\t';
}

/* FUN_100bfbf0: conversation ServerCommand(efw_GetPackage / efw_EndMailPickupMessage).
   efw_TriggerMailPickupMessage is in Conversations txt files but not in the DLL
   strings; reconstruct as FUN_100c77c0 + AddKeyword(OFFICE) so Mail_Officer
   can reach the package topic. */
void EFW_ServerCommand( CBasePlayer *pPlayer, const char *cmd )
{
	char buf[64];
	if( !cmd || !cmd[0] )
		return;
	strncpy( buf, cmd, sizeof( buf ) - 1 );
	buf[sizeof( buf ) - 1] = '\0';
	EFW_TrimInPlace( buf );
	if( EFW_CmdIs( buf, "efw_GetPackage" ) )
	{
		if( !pPlayer )
			pPlayer = EFW_Player();
		if( pPlayer )
		{
			EFW_GiveItem( pPlayer, EFW_ITEM_POWDER, "weapon_efw_WashingPowder" );
			EFW_GiveItem( pPlayer, EFW_ITEM_PHONE, "weapon_efw_MobilePhone" );
			EFW_FailOrNarrate( pPlayer, 0x47 );
		}
		EFW_AdjustHope( 10.0f );
		return;
	}
	if( EFW_CmdIs( buf, "efw_EndMailPickupMessage" ) )
	{
		EFW_PAUnlock();
		return;
	}
	if( EFW_CmdIs( buf, "efw_TriggerMailPickupMessage" ) )
	{
		EFW_PALockRAR();
		EFW_AddKeyword( "OFFICE", 1 );
		return;
	}
}

void EFW_RunScriptAction( CBasePlayer *pPlayer, const char *action )
{
	char name[48];
	char arg[48];
	const char *open;
	const char *close;
	size_t nlen;
	size_t alen;

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
	nlen = (size_t)( open - action );
	if( nlen >= sizeof( name ) )
		nlen = sizeof( name ) - 1;
	memcpy( name, action, nlen );
	name[nlen] = '\0';
	alen = (size_t)( close - open - 1 );
	if( alen >= sizeof( arg ) )
		alen = sizeof( arg ) - 1;
	memcpy( arg, open + 1, alen );
	arg[alen] = '\0';
	EFW_TrimInPlace( name );
	EFW_TrimInPlace( arg );
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
				g_efw.keywordUnlocked[i] = 0;
				break;
			}
		}
	}
	else if( !strcmp( name, "AddDiary" ) )
		EFW_AddDiary( atoi( arg ), 2 );
	else if( !strcmp( name, "ServerCommand" ) )
		EFW_ServerCommand( pPlayer, arg );
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
	/* FUN_100c6780: LoadAll, seed diary 0-1 (level0) or 0-10 (level1/2). */
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
	/* FUN_100c3020 starting loadout by maplevel. */
	if( pPlayer )
	{
		if( level == 0 )
		{
			EFW_GiveItem( pPlayer, EFW_ITEM_IDTAG, "weapon_efw_IDTag" );
			EFW_GiveItem( pPlayer, EFW_ITEM_REDCARD, "weapon_efw_RedPhoneCard" );
		}
		else if( level == 1 )
			EFW_GiveItem( pPlayer, EFW_ITEM_LEVER, "weapon_efw_Lever" );
		else if( level == 2 )
		{
			EFW_GiveItem( pPlayer, EFW_ITEM_PLIERS, "weapon_efw_Pliers" );
			EFW_SpawnFenceTag();
		}
	}
	g_efw.inited = 1;
	EFW_SendHudState();
}

static void EFW_HostFwd( void )
{
	CBasePlayer *pPlayer;
	edict_t *e;
	char line[96];
	const char *pcmd = CMD_ARGV( 0 );

	pPlayer = EFW_Player();
	e = pPlayer ? pPlayer->edict() : NULL;
	snprintf( line, sizeof( line ), "efw: hostfwd %s pawn=%d\n",
		pcmd ? pcmd : "?", ( e && !e->free && e->pvPrivateData ) ? 1 : 0 );
	ALERT( at_error, "%s", line );
	if( g_engfuncs.pfnServerPrint )
		g_engfuncs.pfnServerPrint( line );
	if( pcmd && !strcmp( pcmd, "efw_inuse" ) )
	{
		int hit = 0;
		if( e && !e->free && e->pvPrivateData )
		{
			hit = EFW_LookUse( pPlayer );
			EFW_DebugPrint( ">>> IN_USE look-use hit=%d", hit );
		}
		else
			EFW_LatchInUse();
		return;
	}
	if( e && !e->free && e->pvPrivateData )
		EFW_ClientCommand( e );
}

static void EFW_HostPump( void )
{
	static int n;
	CBasePlayer *pPlayer;
	char line[80];

	n++;
	EFW_StartFrame();
	EFW_RunQueuedChangeLevel();
	pPlayer = EFW_Player();
	if( pPlayer )
		EFW_PlayerPreThink( pPlayer );
	if( n <= 8 || ( n % 30 ) == 1 )
	{
		snprintf( line, sizeof( line ), "efw: hostpump n=%d\n", n );
		ALERT( at_error, "%s", line );
		if( g_engfuncs.pfnServerPrint )
			g_engfuncs.pfnServerPrint( line );
	}
}

static void EFW_RegisterHostCmds( void )
{
	static int done;
	const char *cmds[] = {
		"efw_Talk", "efw_Give", "efw_spider", "efw_Pickup", "efw_UseWithMarker",
		"efw_diary", "efw_diary_next", "efw_diary_prev", "efw_ShowMenu",
		"efw_HelpScreen", "efw_HideUnderBuilding", "efw_PickupPliers",
		"efw_GetPackage", "efw_EndMailPickupMessage", "efw_TriggerMailPickupMessage",
		"efw_pause", "efw_set_state", "efw_changelevel", "efw_setpos", "setpos",
		"efw_lookuse", "menuselect", "give", "drop", "use", "efw_inuse", NULL
	};
	int i;
	if( done )
		return;
	done = 1;
	for( i = 0; cmds[i]; i++ )
		g_engfuncs.pfnAddServerCommand( cmds[i], EFW_HostFwd );
	g_engfuncs.pfnAddServerCommand( "efw_pump", EFW_HostPump );
}

void EFW_LinkUserMessages( void )
{
	EFW_RegisterHostCmds();
	ALERT( at_error, "efw: GameDLLInit ents=%d max=%d\n",
		NUMBER_OF_ENTITIES(), gpGlobals->maxEntities );
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

static int s_worldPrecache;
static int s_worldPrecacheDone;
static int s_worldPasses;
static int s_dropped;
static int s_mapLive;
static int s_skipThis;
static char s_precacheMap[32];
static char s_precacheSeen[96][40];
static int s_precacheSeenN;

/* WASM Host_Frame can re-enter ED_LoadFromFile mid-lump (first studio NPC).
   Returning -1 from pfnSpawn on worldspawn frees edict 0 and the local
   client never signs on. Remember the first copy of each map entity and
   only ED_Free duplicates. */
#define EFW_SPAWN_TRACK 512
static struct
{
	char cn[32];
	char model[32];
	char tn[32];
	int x, y, z;
} s_seen[EFW_SPAWN_TRACK];
static int s_seenN;
static int s_markers;
static int s_refugees;

static void EFW_SpawnResetSeen( void )
{
	s_seenN = 0;
	s_markers = 0;
	s_refugees = 0;
	memset( s_seen, 0, sizeof( s_seen ) );
}

static int EFW_SpawnAlready( edict_t *pent, const char *cn )
{
	const char *model;
	const char *tn;
	int x, y, z, i;

	if( !pent || !cn )
		return 0;
	model = pent->v.model ? STRING( pent->v.model ) : "";
	tn = pent->v.targetname ? STRING( pent->v.targetname ) : "";
	if( !model )
		model = "";
	if( !tn )
		tn = "";
	x = (int)pent->v.origin.x;
	y = (int)pent->v.origin.y;
	z = (int)pent->v.origin.z;
	for( i = 0; i < s_seenN; i++ )
	{
		if( s_seen[i].x != x || s_seen[i].y != y || s_seen[i].z != z )
			continue;
		if( strcmp( s_seen[i].cn, cn ) )
			continue;
		if( strcmp( s_seen[i].model, model ) )
			continue;
		if( strcmp( s_seen[i].tn, tn ) )
			continue;
		return 1;
	}
	return 0;
}

static void EFW_SpawnRemember( edict_t *pent, const char *cn )
{
	const char *model;
	const char *tn;

	if( !pent || !cn || s_seenN >= EFW_SPAWN_TRACK )
		return;
	model = pent->v.model ? STRING( pent->v.model ) : "";
	tn = pent->v.targetname ? STRING( pent->v.targetname ) : "";
	if( !model )
		model = "";
	if( !tn )
		tn = "";
	strncpy( s_seen[s_seenN].cn, cn, sizeof( s_seen[0].cn ) - 1 );
	s_seen[s_seenN].cn[sizeof( s_seen[0].cn ) - 1] = 0;
	strncpy( s_seen[s_seenN].model, model, sizeof( s_seen[0].model ) - 1 );
	s_seen[s_seenN].model[sizeof( s_seen[0].model ) - 1] = 0;
	strncpy( s_seen[s_seenN].tn, tn, sizeof( s_seen[0].tn ) - 1 );
	s_seen[s_seenN].tn[sizeof( s_seen[0].tn ) - 1] = 0;
	s_seen[s_seenN].x = (int)pent->v.origin.x;
	s_seen[s_seenN].y = (int)pent->v.origin.y;
	s_seen[s_seenN].z = (int)pent->v.origin.z;
	s_seenN++;
	if( !strcmp( cn, "efw_Marker" ) )
		s_markers++;
	if( !strcmp( cn, "monster_refugee" ) )
		s_refugees++;
}

static int s_deferStudio;
static int s_waitPawn;
static int s_thinkRestored;
static int s_studioDelay;
static int s_liveTicks;
static int s_inUseLatch; /* HostFwd efw_inuse → PreThink IN_USE */
static int s_menuKeyLatch; /* FUN_100c6a50 GetAsyncKeyState stand-in */

void EFW_LatchInUse( void )
{
	s_inUseLatch = 1;
}

void EFW_LatchMenuKey( int slot )
{
	if( slot >= 1 && slot <= 9 )
		s_menuKeyLatch = slot;
}

/* FUN_100c6a60: if FUN_100c7450 (talkActive), FUN_100c69a0 polls
   GetAsyncKeyState(DAT_1011d134[i]) for slots 1..6 and fires the
   matching ShowMenu CommandButton. WASM: impulse + HostFwd latch. */
void EFW_PollMenuKeys( void )
{
	CBasePlayer *pPlayer;
	int slot;

	if( !g_efw.talkActive )
		return;
	pPlayer = EFW_Player();
	if( !pPlayer )
		return;
	slot = s_menuKeyLatch;
	if( slot < 1 || slot > 9 )
	{
		slot = pPlayer->pev->impulse;
		if( slot < 1 || slot > 9 )
			return;
		pPlayer->pev->impulse = 0;
	}
	s_menuKeyLatch = 0;
	EFW_DebugPrint( ">>> FUN_100c69a0 slot=%d", slot );
	EFW_ChooseTalk( pPlayer, slot );
}

static void EFW_LogLine( const char *line )
{
	ALERT( at_error, "%s", line );
	if( g_engfuncs.pfnServerPrint )
		g_engfuncs.pfnServerPrint( line );
}

void EFW_EnginePrint( const char *line )
{
	EFW_LogLine( line );
}

int EFW_DeferStudio( void )
{
	return s_deferStudio;
}

static int EFW_MaxEnts( void )
{
	int maxEnts = gpGlobals->maxEntities;
	if( maxEnts > 1200 )
		maxEnts = 1200;
	return maxEnts;
}

/* IdleThink / PatrolThink / WALK_MOVE without a studio stall Host_ServerFrame
   so ClientFrame never runs. Freeze every monster_* until the pawn exists. */
static void EFW_FreezeNpcPhysics( void )
{
	int i;
	int n = 0;

	for( i = 1; i < EFW_MaxEnts(); i++ )
	{
		edict_t *pent;
		CBaseEntity *pEnt;
		const char *cn;

		pent = INDEXENT( i );
		if( !pent || pent->free )
			continue;
		cn = pent->v.classname ? STRING( pent->v.classname ) : "";
		if( !cn[0] || strncmp( cn, "monster_", 8 ) )
			continue;
		pent->v.nextthink = 0;
		pent->v.movetype = MOVETYPE_NONE;
		pent->v.solid = SOLID_NOT;
		/* Keep stock HL think pointers; only drop EFW IdleThink/PatrolThink
		   (those WALK_MOVE without a studio and never return). */
		if( !strcmp( cn, "monster_refugee" )
			|| !strcmp( cn, "monster_patrol_guard" )
			|| !strcmp( cn, "monster_efw_guard" ) )
		{
			pEnt = CBaseEntity::Instance( pent );
			if( pEnt )
				pEnt->SetThink( NULL );
		}
		n++;
	}
	if( s_waitPawn == 1 )
	{
		char line[80];
		snprintf( line, sizeof( line ), "efw: froze %d monster thinks until pawn\n", n );
		EFW_LogLine( line );
	}
}

void EFW_StartFrame( void )
{
	int i;

	if( !s_mapLive )
		return;
	/* SET_MODEL of detainee studios stalls the WASM loop. Wait until the
	   listen-server pawn exists so signon frames can run first. */
	if( !EFW_Player() )
	{
		s_waitPawn++;
		EFW_FreezeNpcPhysics();
		if( s_waitPawn <= 8 || ( s_waitPawn % 120 ) == 1 )
		{
			char line[96];
			snprintf( line, sizeof( line ), "efw: StartFrame waiting for pawn ticks=%d\n",
				s_waitPawn );
			EFW_LogLine( line );
		}
		return;
	}
	if( !s_thinkRestored )
	{
		s_thinkRestored = 1;
		EFW_LogLine( "efw: StartFrame pawn live - defer NPC think until ClientFrame loops\n" );
		EFW_FreezeNpcPhysics();
		/* Return so ClientFrame/CheckForResend and cmd forwarding run
		   before any IdleThink/SET_MODEL. */
		return;
	}
	s_liveTicks++;
	if( s_liveTicks <= 8 || ( s_liveTicks % 120 ) == 1 )
	{
		char line[80];
		snprintf( line, sizeof( line ), "efw: StartFrame live ticks=%d\n", s_liveTicks );
		EFW_LogLine( line );
	}
	/* FUN_100c6a60 also runs from StartFrame so HostPump can poll
	   talk hotkeys while ClientFrame (and therefore HUD_Key_Event) is stuck. */
	if( s_liveTicks >= 8 )
		EFW_PollMenuKeys();
	/* First live frames: do not SET_MODEL or unfreeze anyone. The 3D
	   ClientFrame plus leftover STEP physics is what stalls Host_Frame. */
	if( s_liveTicks < 45 )
	{
		int j;
		CBasePlayer *pPlayer = EFW_Player();
		if( pPlayer )
			pPlayer->pev->movetype = MOVETYPE_NOCLIP;
		EFW_FreezeNpcPhysics();
		for( j = 1; j < EFW_MaxEnts(); j++ )
		{
			edict_t *e = INDEXENT( j );
			if( !e || e->free )
				continue;
			if( pPlayer && e == pPlayer->edict() )
				continue;
			if( e->v.flags & FL_CLIENT )
				continue;
			e->v.nextthink = 0;
			if( e->v.movetype == MOVETYPE_STEP || e->v.movetype == MOVETYPE_FLY
				|| e->v.movetype == MOVETYPE_TOSS || e->v.movetype == MOVETYPE_WALK )
				e->v.movetype = MOVETYPE_NONE;
		}
		{
			char line[80];
			snprintf( line, sizeof( line ), "efw: StartFrame done live=%d\n", s_liveTicks );
			EFW_LogLine( line );
		}
		return;
	}
	if( s_studioDelay < 10 )
	{
		s_studioDelay++;
		return;
	}
	for( i = 1; i < EFW_MaxEnts(); i++ )
	{
		edict_t *pent;
		const char *model;
		const char *apply;
		char lower[80];
		int n;
		int c;

		pent = INDEXENT( i );
		if( !pent || pent->free )
			continue;
		if( pent->v.modelindex > 0 )
			continue;
		if( !pent->v.model )
			continue;
		model = STRING( pent->v.model );
		if( !model || !model[0] || model[0] == '*' )
			continue;
		if( !strstr( model, ".mdl" ) )
			continue;
		/* Detainee SET_MODEL (and player.mdl stand-in) never returns in
		   WASM. Bind the precached MODEL_INDEX and keep the PE name. */
		apply = model;
		n = 0;
		for( c = 0; model[c] && n < (int)sizeof( lower ) - 1; c++ )
		{
			char ch = model[c];
			if( ch >= 'A' && ch <= 'Z' )
				ch = (char)( ch + 32 );
			lower[n++] = ch;
		}
		lower[n] = 0;
		{
			char line[192];
			snprintf( line, sizeof( line ), "efw: studio begin edict=%d %s %s -> %s\n",
				i, pent->v.classname ? STRING( pent->v.classname ) : "?", model, apply );
			EFW_LogLine( line );
		}
		if( strstr( lower, "detainee" ) )
		{
			int idx = MODEL_INDEX( (char *)model );
			if( idx <= 0 )
				idx = MODEL_INDEX( "models/Security.mdl" );
			pent->v.modelindex = idx;
			pent->v.solid = SOLID_NOT;
			pent->v.flags |= FL_MONSTER;
			pent->v.movetype = MOVETYPE_NONE;
			{
				char line[160];
				snprintf( line, sizeof( line ), "efw: studio apply edict=%d %s idx=%d\n",
					i, pent->v.classname ? STRING( pent->v.classname ) : "?", idx );
				EFW_LogLine( line );
			}
			EFW_EnableNpcThink( pent );
			return;
		}
		SET_MODEL( pent, apply );
		pent->v.solid = SOLID_BBOX;
		pent->v.flags |= FL_MONSTER;
		pent->v.movetype = MOVETYPE_STEP;
		{
			char line[160];
			snprintf( line, sizeof( line ), "efw: studio apply edict=%d %s %s\n",
				i, pent->v.classname ? STRING( pent->v.classname ) : "?", apply );
			EFW_LogLine( line );
		}
		EFW_EnableNpcThink( pent );
		return;
	}
}

int EFW_PrecacheOnce( const char *szClassname )
{
	int i;
	if( !szClassname || !szClassname[0] )
		return 0;
	for( i = 0; i < s_precacheSeenN; i++ )
	{
		if( !strcmp( s_precacheSeen[i], szClassname ) )
			return 0;
	}
	if( s_precacheSeenN < (int)( sizeof( s_precacheSeen ) / sizeof( s_precacheSeen[0] ) ) )
	{
		strncpy( s_precacheSeen[s_precacheSeenN], szClassname, sizeof( s_precacheSeen[0] ) - 1 );
		s_precacheSeen[s_precacheSeenN][sizeof( s_precacheSeen[0] ) - 1] = 0;
		s_precacheSeenN++;
	}
	return 1;
}

int EFW_BeginWorldPrecache( void )
{
	char line[160];
	const char *map;

	map = ( gpGlobals && gpGlobals->mapname ) ? STRING( gpGlobals->mapname ) : "";
	if( s_worldPrecache )
	{
		snprintf( line, sizeof( line ), "efw: skip nested CWorld::Precache ents=%d\n",
			NUMBER_OF_ENTITIES() );
		EFW_LogLine( line );
		return 0;
	}
	if( s_worldPrecacheDone && s_precacheMap[0] && map[0] && !strcmp( s_precacheMap, map ) )
	{
		snprintf( line, sizeof( line ), "efw: skip repeat CWorld::Precache %s ents=%d\n",
			map, NUMBER_OF_ENTITIES() );
		EFW_LogLine( line );
		return 0;
	}
	strncpy( s_precacheMap, map, sizeof( s_precacheMap ) - 1 );
	s_precacheMap[sizeof( s_precacheMap ) - 1] = 0;
	s_worldPrecache = 1;
	s_precacheSeenN = 0;
	memset( s_precacheSeen, 0, sizeof( s_precacheSeen ) );
	snprintf( line, sizeof( line ), "efw: CWorld::Precache begin ents=%d max=%d map=%s\n",
		NUMBER_OF_ENTITIES(), gpGlobals->maxEntities, s_precacheMap[0] ? s_precacheMap : "?" );
	EFW_LogLine( line );
	return 1;
}

void EFW_EndWorldPrecache( void )
{
	char line[160];
	snprintf( line, sizeof( line ), "efw: CWorld::Precache end ents=%d\n",
		NUMBER_OF_ENTITIES() );
	EFW_LogLine( line );
	s_worldPrecache = 0;
	s_worldPrecacheDone = 1;
	s_deferStudio = 1;
}

int EFW_ShouldSpawn( edict_t *pent )
{
	const char *cn;

	s_skipThis = 0;
	cn = ( pent && pent->v.classname ) ? STRING( pent->v.classname ) : "";
	if( !cn )
		cn = "";
	if( !strcmp( cn, "player" ) )
		return 1;
	if( !strcmp( cn, "worldspawn" ) )
	{
		if( s_worldPrecache )
		{
			s_skipThis = 1;
			return 0;
		}
		s_worldPasses++;
		if( s_worldPasses > 1 || s_mapLive )
		{
			char line[160];
			snprintf( line, sizeof( line ),
				"efw: skip duplicate worldspawn pass=%d ents=%d seen=%d\n",
				s_worldPasses, NUMBER_OF_ENTITIES(), s_seenN );
			EFW_LogLine( line );
			s_skipThis = 1;
			return 0;
		}
		EFW_SpawnRemember( pent, cn );
		return 1;
	}
	if( EFW_SpawnAlready( pent, cn ) )
	{
		s_skipThis = 1;
		return 0;
	}
	if( !strncmp( cn, "monster_", 8 ) )
	{
		char line[160];
		snprintf( line, sizeof( line ), "efw: will Spawn %s seen=%d defer=%d\n",
			cn, s_seenN, s_deferStudio );
		EFW_LogLine( line );
	}
	EFW_SpawnRemember( pent, cn );
	return 1;
}

int EFW_RejectSpawn( edict_t *pent )
{
	const char *cn;

	cn = ( pent && pent->v.classname ) ? STRING( pent->v.classname ) : "";
	if( !cn )
		cn = "";
	/* Never ED_Free world or the listen-server pawn. pfnSpawn -1 on
	   worldspawn is why the WASM client stayed on "Can't cmd, not connected". */
	if( !strcmp( cn, "worldspawn" ) || !strcmp( cn, "player" ) )
		return 0;
	if( !s_skipThis )
		return 0;
	s_dropped++;
	if( !s_mapLive && ( s_dropped <= 6 || ( s_dropped % 50 ) == 0 ) )
	{
		char line[160];
		snprintf( line, sizeof( line ), "efw: reject #%d ents=%d %s\n",
			s_dropped, NUMBER_OF_ENTITIES(), cn[0] ? cn : "?" );
		EFW_LogLine( line );
	}
	return 1;
}

void EFW_OnServerActivate( void )
{
	char line[192];
	const char *map;

	map = ( gpGlobals && gpGlobals->mapname ) ? STRING( gpGlobals->mapname ) : "";
	if( s_mapLive && s_precacheMap[0] && map[0] && !strcmp( s_precacheMap, map ) )
	{
		static int s_liveSpam;
		s_liveSpam++;
		if( s_liveSpam <= 2 || ( s_liveSpam % 120 ) == 0 )
			EFW_LogLine( "efw: ServerActivate already live\n" );
		return;
	}
	if( s_mapLive )
	{
		EFW_LogLine( "efw: ServerActivate new map — reset spawn state\n" );
		EFW_OnServerDeactivate();
	}
	s_mapLive = 1;
	s_waitPawn = 0;
	s_thinkRestored = 0;
	s_studioDelay = 0;
	s_liveTicks = 0;
	snprintf( line, sizeof( line ),
		"efw: ServerActivate ents=%d max=%d dropped=%d passes=%d seen=%d markers=%d refugees=%d map=%s level=%d\n",
		NUMBER_OF_ENTITIES(), gpGlobals->maxEntities, s_dropped, s_worldPasses,
		s_seenN, s_markers, s_refugees,
		STRING( gpGlobals->mapname ), EFW_MapLevel() );
	EFW_LogLine( line );
}

void EFW_OnServerDeactivate( void )
{
	EFW_LogLine( "efw: ServerDeactivate\n" );
	EFW_ClearQueuedChangeLevel();
	g_efw.player = NULL;
	s_worldPrecache = 0;
	s_worldPrecacheDone = 0;
	s_worldPasses = 0;
	s_dropped = 0;
	s_mapLive = 0;
	s_skipThis = 0;
	s_deferStudio = 0;
	s_waitPawn = 0;
	s_thinkRestored = 0;
	s_studioDelay = 0;
	s_liveTicks = 0;
	s_precacheMap[0] = 0;
	s_precacheSeenN = 0;
	memset( s_precacheSeen, 0, sizeof( s_precacheSeen ) );
	EFW_SpawnResetSeen();
}

void EFW_WPrecache( void )
{
	/* FUN_100b2f80 after the HL weapon list: UTIL_PrecacheOtherWeapon
	   weapon_efw_Pliers @ 101054a4 through weapon_efw_WashingPowder. */
	static const char *kEfw[] = {
		"weapon_efw_Pliers",
		"weapon_efw_Lever",
		"weapon_efw_Branch",
		"weapon_efw_MobilePhone",
		"weapon_efw_IDTag",
		"weapon_efw_RedPhoneCard",
		"weapon_efw_GreenPhoneCard",
		"weapon_efw_BluePhoneCard",
		"weapon_efw_WashingPowder"
	};
	unsigned i;
	extern void UTIL_PrecacheOtherWeapon( const char *szClassname );
	static const char *kNpc[] = {
		"models/DetaineeMaleT0.mdl",
		"models/DetaineeMaleT1.mdl",
		"models/DetaineeMaleT2.mdl",
		"models/DetaineeMaleT3.mdl",
		"models/DetaineeMaleT4.mdl",
		"models/DetaineeMaleT5.mdl",
		"models/DetaineeMaleT6.mdl",
		"models/DetaineeMaleT7.mdl",
		"models/DetaineeFemaleT0.mdl",
		"models/DetaineeFemaleT1.mdl",
		"models/DetaineeFemaleT2.mdl",
		"models/Security.mdl",
		"models/tradesman.mdl",
		"models/barney.mdl",
		"models/player.mdl",
		"models/scientist.mdl"
	};
	for( i = 0; i < sizeof( kEfw ) / sizeof( kEfw[0] ); i++ )
		UTIL_PrecacheOtherWeapon( kEfw[i] );
	/* Load studio NPCs during CWorld::Precache so the first refugee SET_MODEL
	   does not stall ED_LoadFromFile; WASM Host_Frame restarts the lump
	   after that stall and never reaches markers. FUN_100c6000 list. */
	for( i = 0; i < sizeof( kNpc ) / sizeof( kNpc[0] ); i++ )
		PRECACHE_MODEL( (char *)kNpc[i] );
}

void EFW_OnDispatchSpawn( edict_t *pent )
{
	static int s_n;
	int used;
	const char *cn;

	if( s_mapLive )
		return;
	if( s_mapLive == 0 && s_worldPasses == 0 )
		s_n = 0;
	s_n++;
	used = NUMBER_OF_ENTITIES();
	cn = ( pent && pent->v.classname ) ? STRING( pent->v.classname ) : "?";
	{
		char line[192];
		snprintf( line, sizeof( line ), "efw: spawn #%d edict=%d ents=%d %s\n",
			s_n, pent ? ENTINDEX( pent ) : -1, used, cn ? cn : "?" );
		if( s_n <= 12 || ( s_n % 25 ) == 0 || ( used >= 700 && ( s_n % 25 ) == 0 )
			|| ( cn && !strncmp( cn, "monster_", 8 ) )
			|| ( cn && !strncmp( cn, "efw_", 4 ) )
			|| ( cn && !strncmp( cn, "worldspawn", 10 ) ) )
			EFW_LogLine( line );
	}
}

void EFW_Precache( void )
{
	PRECACHE_MODEL( "models/w_pliers.mdl" );
	PRECACHE_MODEL( "models/v_pliers.mdl" );
	PRECACHE_MODEL( "models/p_pliers.mdl" );
	PRECACHE_MODEL( "models/w_Pliers.mdl" );
	PRECACHE_MODEL( "models/v_Pliers.mdl" );
	PRECACHE_MODEL( "models/p_Pliers.mdl" );
	EFW_InitPA();
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
	static int s_preN;
	if( !pPlayer )
		return;
	s_preN++;
	if( s_preN <= 8 || ( s_preN % 60 ) == 1 )
	{
		char line[64];
		snprintf( line, sizeof( line ), "efw: PreThink n=%d live=%d\n", s_preN, s_liveTicks );
		EFW_LogLine( line );
	}
	if( g_efw.player != pPlayer )
		EFW_SetPlayer( pPlayer );
	if( !g_efw.inited )
		EFW_InitFromSpawn( pPlayer );
	if( g_efw.lastTime > 1.0f && gpGlobals->time + 0.5f < g_efw.lastTime )
		EFW_InitFromSpawn( pPlayer );
	/* Skip HUD/scan/look-use until StartFrame has proven it can return. */
	if( s_liveTicks < 8 )
		return;
	if( EFW_GetHudInt( 6 ) )
		pPlayer->pev->movetype = MOVETYPE_NONE;
	if( s_inUseLatch )
	{
		pPlayer->m_afButtonPressed |= IN_USE;
		s_inUseLatch = 0;
	}
	/* FUN_100c4af0: PE has no callers; attach to IN_USE so look-use runs. */
	if( pPlayer->m_afButtonPressed & IN_USE )
	{
		int hit = EFW_LookUse( pPlayer );
		EFW_DebugPrint( ">>> IN_USE look-use hit=%d", hit );
		if( hit )
			pPlayer->m_afButtonPressed &= ~IN_USE;
	}
	if( g_efw.lastTime != gpGlobals->time || !g_efw.inited )
		EFW_SendHudState();
}

#endif

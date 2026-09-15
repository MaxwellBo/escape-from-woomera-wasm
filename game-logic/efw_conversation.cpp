#ifndef CLIENT_DLL

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "efw_game.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define EFW_SCRIPT_CACHE 24

struct EfwScriptCache
{
	char name[EFW_TOPIC_LEN];
	EfwScript script;
	int loaded;
};

// Heap-backed so the 24 parsed trees (~7MB of BSS) are not baked into the
// SIDE_MODULE data section. Browsers refuse to sync-compile wasm > 8MB.
static EfwScriptCache *g_scripts;

static EfwScriptCache *EFW_ScriptSlots( void )
{
	if( !g_scripts )
		g_scripts = (EfwScriptCache *)calloc( EFW_SCRIPT_CACHE, sizeof( EfwScriptCache ) );
	return g_scripts;
}

const EfwScript *EFW_LoadScript( const char *scriptName )
{
	int i;
	int length = 0;
	char path[128];
	char *buf;
	EfwScriptCache *slots;
	EfwScriptCache *slot = NULL;

	if( !scriptName || !scriptName[0] )
		return NULL;
	slots = EFW_ScriptSlots();
	if( !slots )
		return NULL;

	for( i = 0; i < EFW_SCRIPT_CACHE; i++ )
	{
		if( slots[i].loaded && !strcmp( slots[i].name, scriptName ) )
			return &slots[i].script;
		if( !slots[i].loaded && !slot )
			slot = &slots[i];
	}
	if( !slot )
		slot = &slots[0];

	int fromMalloc = 0;
	snprintf( path, sizeof( path ), "Conversations/%s.txt", scriptName );
	buf = (char *)LOAD_FILE_FOR_ME( path, &length );
	if( !buf )
	{
		snprintf( path, sizeof( path ), "conversations/%s.txt", scriptName );
		buf = (char *)LOAD_FILE_FOR_ME( path, &length );
	}
	if( !buf )
	{
		const char *tries[] = {
			"/woomera/Conversations/%s.txt",
			"/rwdir/woomera/Conversations/%s.txt",
			NULL
		};
		int t;
		for( t = 0; tries[t] && !buf; t++ )
		{
			FILE *f;
			snprintf( path, sizeof( path ), tries[t], scriptName );
			f = fopen( path, "r" );
			if( !f )
				continue;
			if( fseek( f, 0, SEEK_END ) == 0 )
			{
				length = (int)ftell( f );
				fseek( f, 0, SEEK_SET );
				if( length > 0 )
				{
					buf = (char *)malloc( (size_t)length + 1 );
					if( buf )
					{
						length = (int)fread( buf, 1, (size_t)length, f );
						buf[length] = '\0';
						fromMalloc = 1;
					}
				}
			}
			fclose( f );
		}
	}
	if( !buf )
	{
		ALERT( at_console, "WARNING -- efwConversation::Squark -- character (%s) is not found\n", scriptName );
		return NULL;
	}
	EfwScript_Parse( &slot->script, scriptName, buf, length );
	if( fromMalloc )
		free( buf );
	else
		FREE_FILE( buf );
	strncpy( slot->name, scriptName, EFW_TOPIC_LEN - 1 );
	slot->name[EFW_TOPIC_LEN - 1] = '\0';
	slot->loaded = 1;
	return &slot->script;
}

static int EFW_Ieq( const char *a, const char *b )
{
	if( !a || !b )
		return 0;
	while( *a && *b )
	{
		char ca = *a;
		char cb = *b;
		if( ca >= 'A' && ca <= 'Z' )
			ca = (char)( ca - 'A' + 'a' );
		if( cb >= 'A' && cb <= 'Z' )
			cb = (char)( cb - 'A' + 'a' );
		if( ca != cb )
			return 0;
		a++;
		b++;
	}
	return *a == *b;
}

const char *EFW_ScriptNameForNpc( CBaseEntity *pNpc )
{
	const char *tn;
	static char buf[40];

	if( !pNpc )
		return "Misc";
	tn = STRING( pNpc->pev->targetname );
	if( !tn || !tn[0] )
		return "Misc";

	if( EFW_Ieq( tn, "efw_electrician" ) )
		return "Electrician";
	if( EFW_Ieq( tn, "efw_compound_gate_guard" ) )
		return "Gate_Guard";
	if( EFW_Ieq( tn, "Kitchen_Guard" ) )
		return "Kitchen_Guard";
	if( EFW_Ieq( tn, "Mail_Officer" ) )
		return "Mail_Officer";
	if( EFW_Ieq( tn, "Roster_Officer" ) )
		return "Roster_Officer";
	if( strstr( tn, "Patrol" ) || strstr( tn, "Guard" ) )
		return "Misc_Guard";
	if( EFW_Ieq( tn, "detainee" ) || EFW_Ieq( tn, "detainee_queue" ) )
		return "Detainee";

	strncpy( buf, tn, sizeof( buf ) - 1 );
	buf[sizeof( buf ) - 1] = '\0';
	return buf;
}

static int EFW_QuestionVisible( CBasePlayer *pPlayer, const char *npc, const EfwQuestion *q )
{
	int seen;

	if( !q->topic[0] || !strcmp( q->topic, "UNWANTED_ITEM" ) )
		return 0;
	if( !EFW_HasTopic( pPlayer, q->topic ) )
		return 0;
	seen = EFW_HasSeen( pPlayer, npc, q->topic );
	if( EfwFlags_Has( q->flags, "FirstTime" ) && seen )
		return 0;
	if( EfwFlags_Has( q->flags, "!FirstTime" ) && !seen )
		return 0;
	return 1;
}

static const EfwReply *EFW_PickReply( CBasePlayer *pPlayer, const char *npc, const EfwQuestion *q )
{
	int i;
	int seen = EFW_HasSeen( pPlayer, npc, q->topic );
	const EfwReply *fallback = NULL;

	for( i = 0; i < q->replyCount; i++ )
	{
		const EfwReply *r = &q->replies[i];
		if( EfwFlags_Has( r->flags, "FirstTime" ) )
		{
			if( !seen )
				return r;
			continue;
		}
		if( EfwFlags_Has( r->flags, "!FirstTime" ) )
		{
			if( seen )
				return r;
			continue;
		}
		if( !fallback )
			fallback = r;
	}
	return fallback;
}

static void EFW_WrapLine( char *dst, int dstSize, const char *src, int width )
{
	int used = (int)strlen( dst );
	int col = 0;
	int i = 0;
	int n = (int)strlen( src );

	while( i < n && used < dstSize - 2 )
	{
		int word = i;
		while( word < n && src[word] != ' ' )
			word++;
		int wlen = word - i;
		if( col + wlen >= width && col > 0 )
		{
			dst[used++] = '\n';
			col = 0;
			if( used >= dstSize - 2 )
				break;
		}
		if( used + wlen >= dstSize - 2 )
			wlen = dstSize - 3 - used;
		memcpy( dst + used, src + i, (size_t)wlen );
		used += wlen;
		col += wlen;
		i = word;
		if( i < n && src[i] == ' ' )
		{
			if( used < dstSize - 2 )
			{
				dst[used++] = ' ';
				col++;
			}
			i++;
		}
	}
	dst[used] = '\0';
}

static void EFW_ShowTopicMenu( CBasePlayer *pPlayer, CBaseEntity *pNpc )
{
	EfwState *st = EFW_GetState( pPlayer );
	const EfwScript *script;
	const char *npc = EFW_ScriptNameForNpc( pNpc );
	char menu[512];
	int bits = 0;
	int i;
	int slot;

	script = EFW_LoadScript( npc );
	if( !script )
	{
		char miss[96];
		snprintf( miss, sizeof( miss ), "No script for %s (Conversations/%s.txt).", npc, npc );
		EFW_Print( pPlayer, miss );
		EFW_SendHint( pPlayer, miss );
		return;
	}

	st->talking = ENTINDEX( pNpc->edict() );
	st->menuMode = EFW_MENU_TOPICS;
	st->menuCount = 0;
	st->talkStarted = gpGlobals->time;
	snprintf( menu, sizeof( menu ), "Talk to %s\n", npc );

	slot = 0;
	for( i = 0; i < script->questionCount && slot < 8; i++ )
	{
		const EfwQuestion *q = &script->questions[i];
		char line[80];
		if( !EFW_QuestionVisible( pPlayer, npc, q ) )
			continue;
		st->menuChoices[slot] = i;
		st->menuCount++;
		snprintf( line, sizeof( line ), "%d. %.60s\n", slot + 1, q->text[0] ? q->text : q->topic );
		if( strlen( menu ) + strlen( line ) < sizeof( menu ) - 1 )
			strcat( menu, line );
		bits |= ( 1 << slot );
		slot++;
	}

	if( !st->menuCount )
	{
		EFW_Print( pPlayer, "There is nothing more to say." );
		EFW_CloseTalk( pPlayer );
		return;
	}

	if( strlen( menu ) + 40 < sizeof( menu ) )
		strcat( menu, "Fire or 1-9 to choose" );

	EFW_Print( pPlayer, menu );
	EFW_SendHint( pPlayer, menu );
	EFW_ShowMenu( pPlayer, bits, 45, menu );
}

void EFW_CloseTalk( CBasePlayer *pPlayer )
{
	EfwState *st = EFW_GetState( pPlayer );
	st->talking = 0;
	st->menuMode = EFW_MENU_NONE;
	st->menuCount = 0;
	st->talkStarted = 0;
	EFW_ShowMenu( pPlayer, 0, 0, "" );
}

void EFW_StartTalk( CBasePlayer *pPlayer, CBaseEntity *pNpc )
{
	EfwState *st;
	int idx;
	if( !pPlayer || !pNpc )
		return;
	st = EFW_GetState( pPlayer );
	idx = ENTINDEX( pNpc->edict() );
	if( st->talking == idx && st->menuMode != EFW_MENU_NONE )
		return;
	EFW_ShowTopicMenu( pPlayer, pNpc );
}

void EFW_ContinueTalk( CBasePlayer *pPlayer )
{
	EfwState *st = EFW_GetState( pPlayer );
	CBaseEntity *pNpc;

	if( !st->talking )
	{
		EFW_CloseTalk( pPlayer );
		return;
	}
	pNpc = CBaseEntity::Instance( INDEXENT( st->talking ) );
	if( !pNpc )
	{
		EFW_CloseTalk( pPlayer );
		return;
	}
	EFW_ShowTopicMenu( pPlayer, pNpc );
}

void EFW_ChooseTalk( CBasePlayer *pPlayer, int slot )
{
	EfwState *st = EFW_GetState( pPlayer );
	CBaseEntity *pNpc;
	const EfwScript *script;
	const char *npc;
	const EfwQuestion *q;
	const EfwReply *r;
	char body[512];
	int qi;
	int i;

	if( slot < 1 || slot > st->menuCount )
		return;
	if( !st->talking )
		return;
	pNpc = CBaseEntity::Instance( INDEXENT( st->talking ) );
	if( !pNpc )
	{
		EFW_CloseTalk( pPlayer );
		return;
	}
	npc = EFW_ScriptNameForNpc( pNpc );
	script = EFW_LoadScript( npc );
	if( !script )
		return;
	qi = st->menuChoices[slot - 1];
	if( qi < 0 || qi >= script->questionCount )
		return;
	q = &script->questions[qi];
	r = EFW_PickReply( pPlayer, npc, q );
	EFW_MarkSeen( pPlayer, npc, q->topic );

	body[0] = '\0';
	if( r && r->text[0] )
		EFW_WrapLine( body, sizeof( body ), r->text, 42 );
	if( r )
	{
		for( i = 0; i < r->actionCount; i++ )
		{
			if( !strcmp( r->actions[i], "Goodbye" ) )
			{
				if( body[0] )
					EFW_Print( pPlayer, r->text );
				EFW_CloseTalk( pPlayer );
				return;
			}
			EFW_RunAction( pPlayer, r->actions[i] );
		}
	}

	if( !body[0] )
		strcpy( body, "..." );
	if( strlen( body ) + 16 < sizeof( body ) )
		strcat( body, "\n1. Continue" );
	{
		char tail[32];
		snprintf( tail, sizeof( tail ), "\nDiary %u hope %d", st->diary, (int)( st->hope + 0.5f ) );
		if( strlen( body ) + strlen( tail ) < sizeof( body ) )
			strcat( body, tail );
	}
	st->menuMode = EFW_MENU_CONTINUE;
	EFW_ShowMenu( pPlayer, 1, 45, body );
	if( r && r->text[0] )
		EFW_Print( pPlayer, r->text );
	if( body[0] )
		EFW_SendHint( pPlayer, body );
}

#endif

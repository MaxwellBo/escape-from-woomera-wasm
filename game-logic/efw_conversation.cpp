#ifndef CLIENT_DLL

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "efw_dll.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EFW_SCRIPT_CACHE 24

struct EfwScriptCache
{
	char name[EFW_TOPIC_LEN];
	EfwScript script;
	int loaded;
};

static EfwScriptCache *g_scripts;

static const char *kConversationFiles[] = {
	"Amir", "Detainee", "Electrician", "Elika", "Fashid",
	"Gate_Guard", "Gholan", "Hassan", "Kitchen_Guard", "Laleh",
	"Mail_Officer", "Misc", "Misc_Guard", "Mouhtaz", "Nasir",
	"Roster_Officer", "Shala", NULL
};

static EfwScriptCache *EFW_ScriptSlots( void )
{
	if( !g_scripts )
		g_scripts = (EfwScriptCache *)calloc( EFW_SCRIPT_CACHE, sizeof( EfwScriptCache ) );
	return g_scripts;
}

static const EfwScript *EFW_ParseFile( const char *scriptName )
{
	int i;
	int length = 0;
	char path[128];
	char *buf;
	EfwScriptCache *slots;
	EfwScriptCache *slot = NULL;
	int fromMalloc = 0;

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

	snprintf( path, sizeof( path ), "Conversations/%s.txt", scriptName );
	buf = (char *)LOAD_FILE_FOR_ME( path, &length );
	if( !buf )
	{
		snprintf( path, sizeof( path ), "conversations/%s.txt", scriptName );
		buf = (char *)LOAD_FILE_FOR_ME( path, &length );
	}
	if( !buf )
		return NULL;
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

void EFW_LoadAllConversations( void )
{
	int i;
	for( i = 0; kConversationFiles[i]; i++ )
		EFW_ParseFile( kConversationFiles[i] );
	EFW_AddKeyword( "ESCAPE", 1 );
	EFW_AddKeyword( "GREET", 1 );
	EFW_AddKeyword( "GOODBYE", 1 );
}

void EFW_Squark( const char *targetname )
{
	CBaseEntity *pEnt;
	if( !targetname || !targetname[0] )
		return;
	pEnt = UTIL_FindEntityByTargetname( NULL, targetname );
	if( !pEnt )
	{
		EFW_DebugPrint( "WARNING -- efwConversation::Squark -- character (%s) is not found", targetname );
		return;
	}
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

int EFW_IsTalkNpc( CBaseEntity *pEnt )
{
	const char *cn;
	if( !pEnt )
		return 0;
	cn = STRING( pEnt->pev->classname );
	return !strcmp( cn, "monster_refugee" )
		|| !strcmp( cn, "monster_barney" )
		|| !strcmp( cn, "monster_patrol_guard" )
		|| !strcmp( cn, "monster_efw_guard" );
}

static int EFW_QuestionVisible( const char *npc, const EfwQuestion *q )
{
	int seen;
	if( !q->topic[0] || !strcmp( q->topic, "UNWANTED_ITEM" ) )
		return 0;
	if( !EFW_HasKeyword( q->topic ) )
		return 0;
	seen = EFW_HasSeen( npc, q->topic );
	if( EfwFlags_Has( q->flags, "FirstTime" ) && seen )
		return 0;
	if( EfwFlags_Has( q->flags, "!FirstTime" ) && !seen )
		return 0;
	return 1;
}

static const EfwReply *EFW_PickReply( const char *npc, const EfwQuestion *q )
{
	int i;
	int seen = EFW_HasSeen( npc, q->topic );
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

void EFW_CloseTalk( void )
{
	EfwDllState *st = EFW_Dll();
	CBasePlayer *pPlayer = EFW_Player();
	st->talkActive = 0;
	st->talkNpc = NULL;
	st->menuCount = 0;
	st->menuMode = 0;
	st->talkStart = 0;
	if( pPlayer )
		EFW_CloseMenu( pPlayer );
}

void EFW_ShowConversationMenu( CBasePlayer *pPlayer, CBaseEntity *pNpc )
{
	EfwDllState *st = EFW_Dll();
	const EfwScript *script;
	const char *npc;
	char menu[512];
	int bits = 0;
	int i;
	int slot;

	if( !pPlayer || !pNpc )
		return;
	npc = EFW_ScriptNameForNpc( pNpc );
	script = EFW_ParseFile( npc );
	if( !script )
	{
		EFW_Squark( STRING( pNpc->pev->targetname ) );
		return;
	}

	st->talkNpc = pNpc;
	st->talkActive = 1;
	st->talkStart = gpGlobals->time;
	st->hideDist = EFW_HIDE_DIST;
	st->menuMode = 1;
	st->menuCount = 0;
	snprintf( menu, sizeof( menu ), "Talk to %s\n", npc );
	slot = 0;
	for( i = 0; i < script->questionCount && slot < EFW_MENU_LINES; i++ )
	{
		const EfwQuestion *q = &script->questions[i];
		char line[96];
		if( !EFW_QuestionVisible( npc, q ) )
			continue;
		st->menuChoices[slot] = i;
		st->menuCount++;
		snprintf( line, sizeof( line ), "%d. %.70s\n", slot + 1, q->text[0] ? q->text : q->topic );
		if( strlen( menu ) + strlen( line ) < sizeof( menu ) - 1 )
			strcat( menu, line );
		bits |= ( 1 << slot );
		slot++;
	}
	EFW_DebugPrint( "CONVERSATION   (%d messages)", st->menuCount );
	if( !st->menuCount )
	{
		EFW_DebugPrint( "<conversation inactive>" );
		EFW_CloseTalk();
		return;
	}
	EFW_ShowGoldMenu( pPlayer, bits, 20, menu );
}

void EFW_StartTalk( CBasePlayer *pPlayer, CBaseEntity *pNpc )
{
	EfwDllState *st;
	if( !pPlayer || !pNpc )
		return;
	EFW_SetPlayer( pPlayer );
	st = EFW_Dll();
	if( st->talkActive && st->talkNpc == pNpc && st->menuMode != 0 )
		return;
	EFW_DebugPrint( ">>> efw_Talk %s", STRING( pNpc->pev->targetname ) );
	EFW_ShowConversationMenu( pPlayer, pNpc );
}

void EFW_ChooseTalk( CBasePlayer *pPlayer, int slot )
{
	EfwDllState *st = EFW_Dll();
	CBaseEntity *pNpc;
	const EfwScript *script;
	const char *npc;
	const EfwQuestion *q;
	const EfwReply *r;
	char body[512];
	int qi;
	int i;

	if( !pPlayer || slot < 1 || slot > st->menuCount )
		return;
	pNpc = st->talkNpc;
	if( !pNpc )
		return;
	npc = EFW_ScriptNameForNpc( pNpc );
	script = EFW_ParseFile( npc );
	if( !script )
		return;
	if( st->menuMode == 2 )
	{
		EFW_ShowConversationMenu( pPlayer, pNpc );
		return;
	}
	qi = st->menuChoices[slot - 1];
	if( qi < 0 || qi >= script->questionCount )
		return;
	q = &script->questions[qi];
	r = EFW_PickReply( npc, q );
	EFW_MarkSeen( npc, q->topic );
	body[0] = '\0';
	if( r && r->text[0] )
	{
		snprintf( body, sizeof( body ), "%s\n1. Continue", r->text );
		if( r->actionCount )
		{
			for( i = 0; i < r->actionCount; i++ )
			{
				if( !strcmp( r->actions[i], "Goodbye" ) )
				{
					EFW_Print( pPlayer, r->text );
					EFW_CloseTalk();
					return;
				}
				EFW_RunScriptAction( pPlayer, r->actions[i] );
			}
		}
	}
	if( !body[0] )
		strcpy( body, "1. Continue" );
	st->menuMode = 2;
	st->menuCount = 1;
	st->menuChoices[0] = 0;
	EFW_ShowGoldMenu( pPlayer, 1, 20, body );
}

void EFW_ThinkConversation( void )
{
	EfwDllState *st = EFW_Dll();
	CBasePlayer *pPlayer;
	float dist;
	if( !st->talkActive )
		return;
	pPlayer = EFW_Player();
	if( !pPlayer || !st->talkNpc )
	{
		EFW_CloseTalk();
		return;
	}
	if( gpGlobals->time >= st->talkStart + EFW_TALK_TIMEOUT )
	{
		EFW_CloseTalk();
		return;
	}
	dist = ( st->talkNpc->pev->origin - pPlayer->pev->origin ).Length();
	if( dist >= st->hideDist )
	{
		EFW_DebugPrint( "Conversation hidden, partner too far" );
		EFW_CloseTalk();
	}
}

void EFW_TalkScan( void )
{
	CBasePlayer *pPlayer = EFW_Player();
	CBaseEntity *pScan = NULL;
	int count = 0;
	if( !pPlayer || !gmsgEFWCtPrv )
		return;
	while( ( pScan = UTIL_FindEntityInSphere( pScan, pPlayer->pev->origin, EFW_TALK_SCAN ) ) != NULL )
	{
		if( pScan == pPlayer )
			continue;
		if( EFW_IsTalkNpc( pScan ) )
			count++;
		if( count > 2 )
			break;
	}
	MESSAGE_BEGIN( MSG_ONE, gmsgEFWCtPrv, NULL, pPlayer->pev );
		WRITE_BYTE( count != 0 );
	MESSAGE_END();
}

#endif

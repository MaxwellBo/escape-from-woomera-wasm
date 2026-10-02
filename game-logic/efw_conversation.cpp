#ifndef CLIENT_DLL

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "efw_dll.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

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

static void EFW_RegisterDefaults( void );

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
		{
			return &slots[i].script;
		}
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
	EfwScript_SetErrorFn( EFW_YyError );
	EfwScript_SetFlexFn( EFW_FlexMsg );
	EfwScript_Parse( &slot->script, scriptName, buf, length );
	if( fromMalloc )
		free( buf );
	else
		FREE_FILE( buf );
	/* FUN_100c2660 returns DAT_10132460 after FUN_100c1dc0 scanner,
	   FUN_100c2360 0x4000 buffer, FUN_100be970 yyparse. */
	{
		static int s_parse;
		if( !s_parse )
		{
			s_parse = 1;
			EFW_DebugPrint( ">>> FUN_100c2660 %s questions=%d", scriptName, slot->script.questionCount );
			EFW_DebugPrint( ">>> FUN_100c1dc0" );
		}
		else
			EFW_DebugPrint( ">>> ParseFile %s questions=%d", scriptName, slot->script.questionCount );
	}
	strncpy( slot->name, scriptName, EFW_TOPIC_LEN - 1 );
	slot->name[EFW_TOPIC_LEN - 1] = '\0';
	slot->loaded = 1;
	return &slot->script;
}

/* FUN_100b8ff0 FindFirstFileA(gamedir\\Conversations\\*.txt). POSIX opendir
 * is the WASM stand-in; the v0.84 zip list is the fallback when the engine
 * search path is not a real directory. */
static int EFW_LoadConversationDir( const char *dir )
{
	DIR *d;
	struct dirent *ent;
	int n = 0;

	if( !dir || !dir[0] )
		return 0;
	d = opendir( dir );
	if( !d )
		return 0;
	while( ( ent = readdir( d ) ) != NULL )
	{
		const char *name = ent->d_name;
		size_t len;
		char stem[EFW_TOPIC_LEN];
		int stemLen;

		if( !name || name[0] == '.' )
			continue;
		len = strlen( name );
		if( len < 5 )
			continue;
		if( strcmp( name + len - 4, ".txt" ) && strcmp( name + len - 4, ".TXT" ) )
			continue;
		stemLen = (int)len - 4;
		if( stemLen >= EFW_TOPIC_LEN )
			stemLen = EFW_TOPIC_LEN - 1;
		memcpy( stem, name, (size_t)stemLen );
		stem[stemLen] = '\0';
		if( EFW_ParseFile( stem ) )
			n++;
	}
	closedir( d );
	return n;
}

void EFW_LoadAllConversations( void )
{
	int i;
	int n = 0;
	char gamedir[256];
	char path[300];
	{
		static int s_load;
		if( !s_load )
		{
			s_load = 1;
			EFW_DebugPrint( ">>> FUN_100b8ff0" );
		}
	}

	gamedir[0] = '\0';
	GET_GAME_DIR( gamedir );
	if( gamedir[0] )
	{
		snprintf( path, sizeof( path ), "%s/Conversations", gamedir );
		n = EFW_LoadConversationDir( path );
		if( !n )
		{
			snprintf( path, sizeof( path ), "/%s/Conversations", gamedir );
			n = EFW_LoadConversationDir( path );
		}
	}
	if( !n )
		n = EFW_LoadConversationDir( "/woomera/Conversations" );
	if( !n )
		n = EFW_LoadConversationDir( "Conversations" );
	if( !n )
	{
		for( i = 0; kConversationFiles[i]; i++ )
		{
			if( EFW_ParseFile( kConversationFiles[i] ) )
				n++;
		}
	}
	EFW_DebugPrint( "efwConversation::LoadAll %d files", n );
	/* FUN_100b9391: the only keywords set at load. FUN_100bfea0 hides
	   any other topic until AddTopic. DeleteTopic stores flag 0. */
	EFW_AddKeyword( "ESCAPE", 1 );
	EFW_AddKeyword( "GREET", 1 );
	EFW_AddKeyword( "GOODBYE", 1 );
	EFW_RegisterDefaults();
}

/* FUN_100ba080: bit 8 skips the 1.0s gate at 0x100dd618. The stamp lives
   on the character record, so a second speaker is not blocked by the first.
   sv.time does not move here; the pump clock does. force still writes the stamp. */
static int EFW_SquarkCooling( edict_t *ed, int force )
{
	static struct
	{
		edict_t *ed;
		float at;
	} slot[16];
	int i;
	int freeSlot;
	float now;
	if( !ed )
		return 0;
	now = EFW_HostClock();
	freeSlot = -1;
	for( i = 0; i < 16; i++ )
	{
		if( slot[i].ed == ed )
		{
			/* fcomp 1.0; test ah,0x41; jne return. Equal to 1.0 still holds. */
			if( !force && ( now - slot[i].at ) <= 1.0f )
				return 1;
			slot[i].at = now;
			return 0;
		}
		if( freeSlot < 0 && !slot[i].ed )
			freeSlot = i;
	}
	if( freeSlot < 0 )
		freeSlot = 0;
	slot[freeSlot].ed = ed;
	slot[freeSlot].at = now;
	return 0;
}

void EFW_Squark( const char *targetname, const char *text, int flags )
{
	CBaseEntity *pEnt;
	CBasePlayer *pPlayer;
	EfwDllState *st;
	char line[512];
	float range;
	if( !targetname || !targetname[0] )
		return;
	{
		static int s_squark;
		if( !s_squark )
		{
			s_squark = 1;
			EFW_DebugPrint( ">>> FUN_100ba040 %s", targetname );
		}
	}
	pEnt = UTIL_FindEntityByTargetname( NULL, targetname );
	if( !pEnt )
	{
		EFW_DebugPrint( "WARNING -- efwConversation::Squark -- character (%s) is not found", targetname );
		return;
	}
	if( !text || !text[0] )
		return;
	if( EFW_SquarkCooling( pEnt->edict(), flags & 8 ) )
		return;
	{
		static int s_show;
		if( !s_show )
		{
			s_show = 1;
			EFW_DebugPrint( ">>> FUN_100ba080 flags=%d", flags );
		}
	}
	/* 0x100ba164: bit 2 → 1024, bit 4 → 256, else 128. Stored at DAT_1011d130. */
	if( flags & 2 )
		range = 1024.0f;
	else if( flags & 4 )
		range = 256.0f;
	else
		range = 128.0f;
	{
		static int s_range;
		if( s_range < 4 )
		{
			s_range++;
			EFW_DebugPrint( "efw: squark range=%.0f flags=%d", range, flags );
		}
	}
	/* 0x100ba15a closes the previous line, then 0x100c6e60 shows this one
	   on the speaker so FUN_100c6c10 can hide it past the range. */
	EFW_CloseTalk();
	st = EFW_Dll();
	st->talkNpc = pEnt;
	st->talkActive = 1;
	st->menuMode = 0;
	st->hideDist = range;
	pPlayer = EFW_Player();
	snprintf( line, sizeof( line ), "%s: %s", targetname, text );
	if( pPlayer )
	{
		EFW_Print( pPlayer, line );
		EFW_ShowDllMenu( pPlayer, text, NULL, 0 );
	}
	st->talkStart = EFW_HostClock();
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

/* FUN_100b86b0 RegisterDefaults: targetname -> HUD display name. */
static const struct
{
	const char *target;
	const char *display;
} kDisplayNames[] = {
	{ "efw_compound_gate_guard", "Gate Guard" },
	{ "efw_electrician", "Electrician" },
	{ "detainee", "Detainee" },
	{ "detainee_queue", "Detainee in queue" },
	{ NULL, NULL }
};

static const char *EFW_DisplayName( const char *targetname )
{
	int i;
	if( !targetname || !targetname[0] )
		return targetname;
	for( i = 0; kDisplayNames[i].target; i++ )
	{
		if( EFW_Ieq( targetname, kDisplayNames[i].target ) )
			return kDisplayNames[i].display;
	}
	return targetname;
}

/* FUN_100b89a0 miss path: the ShowMenu speaker is the display name with
   underscores turned into spaces. A map hit already has the spaced name. */
const char *EFW_MenuSpeakerName( CBaseEntity *pNpc )
{
	static char buf[64];
	const char *tn;
	const char *name;
	int i;
	int j;

	buf[0] = '\0';
	if( !pNpc )
		return buf;
	tn = STRING( pNpc->pev->targetname );
	if( !tn || !tn[0] )
		return buf;
	name = EFW_DisplayName( tn );
	if( !name )
		return buf;
	j = 0;
	for( i = 0; name[i] && j < (int)sizeof( buf ) - 1; i++ )
		buf[j++] = ( name[i] == '_' ) ? ' ' : name[i];
	buf[j] = '\0';
	return buf;
}

static void EFW_RegisterDefaults( void )
{
	int n;
	for( n = 0; kDisplayNames[n].target; n++ )
		;
	EFW_DebugPrint( ">>> FUN_100b86b0 n=%d", n );
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

/* FUN_100bdc40: walk the question's flag range and call vtable+4
   (HasKeyword / FirstTime). WASM: unlocked topic + FirstTime seen-bit. */
static int EFW_QuestionVisible( const char *npc, const EfwQuestion *q )
{
	int seen;
	{
		static int s_match;
		if( !s_match )
		{
			s_match = 1;
			EFW_DebugPrint( ">>> FUN_100bdc40 %s %s", npc ? npc : "-", q->topic );
		}
	}
	/* A #Q prompt has no topic. FUN_100bdc40 passes an empty flag
	   range. UNWANTED_ITEM stays off the menu. */
	if( !strcmp( q->topic, "UNWANTED_ITEM" ) )
		return 0;
	if( q->topic[0] )
	{
		if( !EFW_HasKeyword( q->topic ) )
			return 0;
	}
	else if( q->depth <= 0 )
		return 0;
	seen = EFW_HasSeen( npc, q->topic );
	if( EfwFlags_Has( q->flags, "FirstTime" ) && seen )
		return 0;
	if( EfwFlags_Has( q->flags, "!FirstTime" ) && !seen )
		return 0;
	return 1;
}

/* FUN_100b9cd0: every reply whose flags all pass is pushed, then
   rand() (0x100c84e4) % count indexes that vector. FirstTime fails
   once the topic is seen; !FirstTime fails before that. No flags pass. */
static const EfwReply *EFW_PickReply( const char *npc, const EfwQuestion *q )
{
	const EfwReply *hits[EFW_MAX_REPLIES];
	int n = 0;
	int i;
	int seen = EFW_HasSeen( npc, q->topic );
	for( i = 0; i < q->replyCount && n < EFW_MAX_REPLIES; i++ )
	{
		const EfwReply *r = &q->replies[i];
		if( EfwFlags_Has( r->flags, "FirstTime" ) && seen )
			continue;
		if( EfwFlags_Has( r->flags, "!FirstTime" ) && !seen )
			continue;
		hits[n++] = r;
	}
	if( !n )
		return NULL;
	i = RANDOM_LONG( 0, n - 1 );
	if( n > 1 )
		EFW_DebugPrint( ">>> FUN_100b9cd0 n=%d i=%d", n, i );
	return hits[i];
}

void EFW_CloseTalk( void )
{
	EfwDllState *st = EFW_Dll();
	CBasePlayer *pPlayer = EFW_Player();
	{
		static int s_close;
		if( !s_close )
		{
			s_close = 1;
			EFW_DebugPrint( ">>> FUN_100c6950" );
		}
	}
	st->talkActive = 0;
	st->talkNpc = NULL;
	st->menuCount = 0;
	st->menuMode = 0;
	st->talkCursor = -1;
	st->talkStart = 0;
	st->prevQuestion[0] = '\0';
	st->speech[0] = '\0';
	st->speechAt = 0;
	if( pPlayer )
		EFW_CloseMenu( pPlayer );
}

void EFW_ShowConversationMenu( CBasePlayer *pPlayer, CBaseEntity *pNpc )
{
	EfwDllState *st = EFW_Dll();
	const EfwScript *script;
	const char *npc;
	const char *lines[EFW_MENU_LINES];
	char title[512];
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
	/* FUN_100b9bb9: time-speechAt < 20 uses the speech string. Otherwise
	   the title is the empty global at 0x10121c38. A fresh efw_Talk
	   clears speech, so the first open has no "Talk to" line. */
	if( st->speech[0] && gpGlobals->time - st->speechAt < EFW_TALK_TIMEOUT )
	{
		strncpy( title, st->speech, sizeof( title ) - 1 );
		title[sizeof( title ) - 1] = '\0';
		EFW_DebugPrint( ">>> FUN_100b9bb9 speech=1" );
	}
	else
	{
		title[0] = '\0';
		{
			static int s_emptyTitle;
			if( s_emptyTitle < 4 )
			{
				s_emptyTitle++;
				EFW_DebugPrint( "efw: menu title empty" );
			}
		}
	}
	/* FUN_100b9990: no current question shows depth 0 and skips
	   deeper lines. After an answer the target is that question's
	   depth plus one, the walk starts on the next line, a shallower
	   line ends the walk, and a deeper line is skipped. */
	{
		int target = 0;
		int begin = 0;
		if( st->talkCursor >= 0 && st->talkCursor < script->questionCount )
		{
			target = script->questions[st->talkCursor].depth + 1;
			begin = st->talkCursor + 1;
		}
		slot = 0;
		for( i = begin; i < script->questionCount && slot < 6; i++ )
		{
			const EfwQuestion *q = &script->questions[i];
			if( q->depth < target )
				break;
			if( q->depth != target )
				continue;
			if( !EFW_QuestionVisible( npc, q ) )
				continue;
			st->menuChoices[slot] = i;
			lines[slot] = q->text[0] ? q->text : q->topic;
			st->menuCount++;
			slot++;
		}
		EFW_DebugPrint( ">>> FUN_100b9990 depth=%d n=%d %s | %s",
			target, slot,
			slot > 0 ? lines[0] : "",
			slot > 1 ? lines[1] : "" );
	}
	EFW_DebugPrint( "CONVERSATION   (%d messages)", st->menuCount );
	if( !st->menuCount )
	{
		EFW_DebugPrint( "<conversation inactive>" );
		EFW_CloseTalk();
		return;
	}
	EFW_ShowDllMenu( pPlayer, title, lines, st->menuCount );
	/* FUN_100c6e60 writes DAT_1013487c from gpGlobals->time. FUN_100c6c10
	   closes the menu once that clock passes the stamp by 20s. sv.time
	   stays at the listen-server pause, so stamp the host clock after
	   ShowDllMenu overwrites the field. */
	st->talkStart = EFW_HostClock();
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
	{
		/* FUN_100c6420: MapLevel then FUN_100b9990 conversation match. */
		EFW_DebugPrint( ">>> FUN_100c6420 %s level=%d", STRING( pNpc->pev->targetname ), EFW_MapLevel() );
		EFW_DebugPrint( ">>> FUN_100b9990 %s", STRING( pNpc->pev->targetname ) );
	}
	(void)EFW_MapLevel();
	EFW_DebugPrint( ">>> efw_Talk %s", STRING( pNpc->pev->targetname ) );
	st->speech[0] = '\0';
	st->speechAt = 0;
	st->talkCursor = -1;
	EFW_ShowConversationMenu( pPlayer, pNpc );
}

/* FUN_100c4550: FUN_100b95a0(npc, weapon->classname, 8).
 * FUN_100bdc80 walks questions until type==1 (UNWANTED_ITEM); classname is
 * only a dummy std::string. Missing script → "Thanks, but I don't need it." */
void EFW_GiveUnwanted( CBasePlayer *pPlayer, CBaseEntity *pNpc )
{
	const char *npc;
	const char *tn;
	const EfwScript *script;
	const EfwQuestion *q;
	const EfwReply *r;
	const char *text = "Thanks, but I don't need it.";
	int qi;

	/* HostFwd leftover (pawn=0) still Squarks UNWANTED_ITEM so FUN_100c4550
	   / FUN_100ba080 quote without pvPrivateData. */
	if( !pNpc )
		return;
	tn = STRING( pNpc->pev->targetname );
	npc = EFW_ScriptNameForNpc( pNpc );
	script = EFW_ParseFile( npc );
	if( script )
	{
		{
			static int s_walk;
			if( !s_walk )
			{
				s_walk = 1;
				EFW_DebugPrint( ">>> FUN_100bdc80 UNWANTED_ITEM %s", npc ? npc : "?" );
			}
		}
		qi = EfwScript_FindQuestion( script, "UNWANTED_ITEM" );
		if( qi >= 0 )
		{
			q = &script->questions[qi];
			r = EFW_PickReply( npc, q );
			if( r && r->text[0] )
				text = r->text;
			EFW_MarkSeen( npc, q->topic );
		}
	}
	EFW_DebugPrint( ">>> efw_Give UNWANTED_ITEM %s", tn );
	EFW_DebugPrint( ">>> FUN_100c4550 FUN_100b95a0 %s", tn ? tn : "?" );
	EFW_DebugPrint( ">>> FUN_100b95a0 flag=8 %s", tn ? tn : "?" );
	EFW_CloseTalk();
	EFW_Squark( tn, text, 8 );
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

	static float lastPick;
	float now;
	if( !pPlayer || slot < 1 || slot > st->menuCount )
		return;
	/* sv.time stays at the listen-server pause, so a gpGlobals->time
	   gate accepts one menuselect and then drops the rest. */
	now = EFW_HostClock();
	if( now < lastPick + 0.3f )
		return;
	lastPick = now;
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
	/* FUN_100b9a77: the next menu is this question's depth plus one. */
	st->talkCursor = qi;
	/* FUN_100c69a0 copies the pressed ShowMenu line into DAT_10134480. */
	strncpy( st->prevQuestion, q->text[0] ? q->text : q->topic,
		sizeof( st->prevQuestion ) - 1 );
	st->prevQuestion[sizeof( st->prevQuestion ) - 1] = '\0';
	EFW_DebugPrint( ">>> menuselect %d  %s", slot, q->topic[0] ? q->topic : q->text );
	body[0] = '\0';
	if( r && r->text[0] )
	{
		strncpy( body, r->text, sizeof( body ) - 1 );
		body[sizeof( body ) - 1] = '\0';
		EFW_Print( pPlayer, r->text );
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
	/* FUN_100b9cd0 stores the reply (squark flag 8) then FUN_100b9990
	   shows the topic lines again. "Continue" is not in the DLL. */
	if( body[0] )
	{
		strncpy( st->speech, body, sizeof( st->speech ) - 1 );
		st->speech[sizeof( st->speech ) - 1] = '\0';
		st->speechAt = gpGlobals->time;
	}
	else
	{
		st->speech[0] = '\0';
		st->speechAt = 0;
	}
	st->menuMode = 1;
	EFW_ShowConversationMenu( pPlayer, pNpc );
}

void EFW_ThinkConversation( void )
{
	EfwDllState *st = EFW_Dll();
	CBasePlayer *pPlayer;
	float dist;
	{
		static int s_thinkConv;
		if( !s_thinkConv )
		{
			s_thinkConv = 1;
			EFW_DebugPrint( ">>> FUN_100c6c10 talk=%d", st->talkActive );
		}
	}

	if( !st->talkActive )
	{
		st->talkIdleTicks++;
		if( st->talkIdleTicks > 0x3c && st->diaryPending != -1 )
		{
			EFW_SetHudInt( 0, st->diaryPending );
			EFW_SetHudInt( 5, 1 );
			st->diaryPending = -1;
		}
	}
	else
	{
		pPlayer = EFW_Player();
		{
			static int s_now;
			if( !s_now )
			{
				s_now = 1;
				EFW_DebugPrint( ">>> FUN_100c5b60 t=%.2f", gpGlobals->time );
			}
		}
		{
			/* FUN_100c6c10: gpGlobals->time >= DAT_1013487c + 20. The
			   listen server leaves sv.time at 1.00, so both a squark and
			   a choice menu measure those 20 seconds on the host clock.
			   A null partner (narration pushes 0) skips the range test.
			   It does not close the menu on its own. */
			float now = EFW_HostClock();
			if( now >= st->talkStart + EFW_TALK_TIMEOUT )
			{
				if( !st->talkNpc )
					EFW_DebugPrint( "efw: caption timeout" );
				else if( st->menuMode == 0 )
					EFW_DebugPrint( "efw: squark timeout" );
				else
					EFW_DebugPrint( "efw: menu timeout" );
				EFW_CloseTalk();
			}
			else if( pPlayer && st->talkNpc )
			{
				dist = ( st->talkNpc->pev->origin - pPlayer->pev->origin ).Length();
				if( dist >= st->hideDist )
				{
					EFW_DebugPrint( "Conversation hidden, partner too far" );
					EFW_CloseTalk();
				}
			}
		}
		st->talkIdleTicks = 0;
	}
	EFW_ThinkPA();
}

static Vector EFW_AbsCenter( CBaseEntity *pEnt )
{
	Vector c;
	if( !pEnt )
		return g_vecZero;
	c = ( pEnt->pev->absmin + pEnt->pev->absmax ) * 0.5f;
	if( ( pEnt->pev->absmax - pEnt->pev->absmin ).Length() < 1.0f )
		c = pEnt->pev->origin;
	return c;
}

/* 0x100afc60 from the eye. fcomp 0x100f9158: keep when fraction >= 0.97. */
static int EFW_TraceClear( CBasePlayer *pPlayer, const Vector &target )
{
	TraceResult tr;
	Vector eye;
	if( !pPlayer )
		return 0;
	eye = pPlayer->EyePosition();
	UTIL_TraceLine( eye, target, dont_ignore_monsters, pPlayer->edict(), &tr );
	return tr.flFraction >= 0.97f;
}

static void EFW_FillScan( int type, const char *name, const Vector &pos )
{
	EfwDllState *st = EFW_Dll();
	EfwScanSlot *slot;
	if( st->scanCount >= EFW_MAX_SCAN )
		return;
	slot = &st->scan[st->scanCount];
	memset( slot, 0, sizeof( *slot ) );
	slot->type = type;
	if( name )
		strncpy( slot->name, name, EFW_SCAN_NAME - 1 );
	slot->zero = 0;
	slot->x = pos.x;
	slot->y = pos.y;
	slot->z = pos.z;
	st->scanCount++;
}

/* FUN_100440b0: same display names the client inventory and Give
   caption use. Slots 21, 22, and 23 are all "Phone Card". */
static const char *EFW_HtmlWepLabel( int id )
{
	static const char *kNames[] = {
		"Pliers", "Lever", "Branch", "SIM Card", "ID Tag",
		"Phone Card", "Phone Card", "Phone Card", "Washing Powder"
	};
	if( id < 16 || id > 24 )
		return "item";
	return kNames[id - 16];
}

static int EFW_HtmlHasWep( CBasePlayer *pPlayer, int id )
{
	if( id < 16 || id > 24 || !pPlayer )
		return 0;
	return ( EFW_WeaponMask( pPlayer ) & ( 1 << ( id - 16 ) ) ) != 0;
}

#define EFW_HTML_VGUI_MAX 6 /* FUN_100c6d70 six Panel* slots */
#define EFW_HTML_SW 640
#define EFW_HTML_SH 480

struct EfwHtmlVguiBtn
{
	int x, y, w, h;
	char cmd[96];
	char label[64];
	char spr[8];
};

static void EFW_HtmlVguiAdd( EfwHtmlVguiBtn *out, int *n, int x, int y, const char *label, const char *cmd, const char *spr = NULL )
{
	EfwHtmlVguiBtn *b;
	int w = 168;
	int h = 28;
	if( *n >= EFW_HTML_VGUI_MAX )
		return;
	if( x < 180 )
		x = 180;
	if( y < 180 )
		y = 180;
	if( x > EFW_HTML_SW - 180 )
		x = EFW_HTML_SW - 180;
	if( y > EFW_HTML_SH - 180 )
		y = EFW_HTML_SH - 180;
	/* FUN_10044f70 stores this projected point on every button. The client
	   spreads them with FUN_10045f20 (radius 130, index/count). A 32px
	   stagger here piled the 97px quads on top of each other. */
	b = &out[( *n )++];
	b->w = w;
	b->h = h;
	b->x = x - w / 2;
	b->y = y - h;
	b->spr[0] = '\0';
	if( spr && spr[0] )
	{
		strncpy( b->spr, spr, sizeof( b->spr ) - 1 );
		b->spr[sizeof( b->spr ) - 1] = '\0';
	}
	strncpy( b->label, label ? label : "", sizeof( b->label ) - 1 );
	b->label[sizeof( b->label ) - 1] = '\0';
	strncpy( b->cmd, cmd ? cmd : "", sizeof( b->cmd ) - 1 );
	b->cmd[sizeof( b->cmd ) - 1] = '\0';
}

/* FUN_10041fb0 (%C): a map hit is the display string. A miss copies the
   targetname and turns '_' into a space. */
static void EFW_PercentC( char *out, size_t n, const char *raw )
{
	static const struct
	{
		const char *key;
		const char *disp;
	} map[] = {
		{ "efw_compound_gate_guard", "Gate Guard" },
		{ "efw_electrician", "Electrician" },
		{ "detainee", "Detainee" },
		{ "detainee queue", "Detainee in queue" },
	};
	size_t i;

	if( !out || n < 1 )
		return;
	out[0] = '\0';
	if( !raw || !raw[0] )
	{
		strncpy( out, "them", n - 1 );
		out[n - 1] = '\0';
		return;
	}
	for( i = 0; i < sizeof( map ) / sizeof( map[0] ); i++ )
	{
		if( !strcmp( raw, map[i].key ) )
		{
			strncpy( out, map[i].disp, n - 1 );
			out[n - 1] = '\0';
			return;
		}
	}
	strncpy( out, raw, n - 1 );
	out[n - 1] = '\0';
	for( i = 0; out[i]; i++ )
	{
		if( out[i] == '_' )
			out[i] = ' ';
	}
}

/* Match client EFW_BuildVgui / FUN_10044f70 CommandButton set. */
static void EFW_HtmlBuild( EfwHtmlVguiBtn *out, int *n, const EfwScanSlot *s, CBasePlayer *pPlayer, int x, int y )
{
	char label[64];
	char cmd[96];
	char who[64];
	if( s->type == 0 )
	{
		int id;
		EFW_PercentC( who, sizeof( who ), s->name );
		{
			static int s_pc;
			if( s_pc < 4 && s->name[0] && strcmp( s->name, who ) )
			{
				s_pc++;
				EFW_DebugPrint( "efw: percentC %s -> %s", s->name, who );
			}
		}
		snprintf( label, sizeof( label ), "Talk to %s", who );
		snprintf( cmd, sizeof( cmd ), "efw_Talk %s", s->name[0] ? s->name : "" );
		EFW_HtmlVguiAdd( out, n, x, y, label, cmd );
		/* FUN_10044f70: columns of 10, stride 11. The red phone card
		   sits on the skipped index and never becomes a Give button. */
		for( id = 16; id <= 24; id++ )
		{
			if( !EFW_HudWeaponVisited( id ) )
				continue;
			if( !EFW_HtmlHasWep( pPlayer, id ) )
				continue;
			snprintf( label, sizeof( label ), "Give %s to %s", EFW_HtmlWepLabel( id ), who );
			snprintf( cmd, sizeof( cmd ), "efw_Give %d %s", id, s->name[0] ? s->name : "" );
			/* weapon+0xbc is the item sprite, the same handle the fly uses. */
			EFW_HtmlVguiAdd( out, n, x, y, label, cmd, "wep" );
		}
		return;
	}
	if( s->type == 1 )
	{
		if( !strcmp( s->name, "efw_IDTag_Position" ) )
		{
			/* FUN_10044f30(0x14). A miss jumps to the end of FUN_10044f70,
			   so the fence has no button until the ID tag is held. */
			if( EFW_HtmlHasWep( pPlayer, 20 ) )
			{
				snprintf( label, sizeof( label ), "Place %s on fence", EFW_HtmlWepLabel( 20 ) );
				snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %d %s", 20, s->name );
				EFW_HtmlVguiAdd( out, n, x, y, label, cmd, "wep" );
			}
		}
		else if( !strcmp( s->name, "efw_kitchen_bin" ) )
		{
			if( EFW_HtmlHasWep( pPlayer, 16 ) )
			{
				snprintf( label, sizeof( label ), "Hide %s in bin", EFW_HtmlWepLabel( 16 ) );
				snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %d %s", 16, s->name );
				EFW_HtmlVguiAdd( out, n, x, y, label, cmd, "wep" );
			}
		}
		else if( !strcmp( s->name, "efw_hiding_place" ) )
			EFW_HtmlVguiAdd( out, n, x, y, "Hide under the building", "efw_HideUnderBuilding" );
		else if( !strcmp( s->name, "efw_PliersMarker" ) )
			EFW_HtmlVguiAdd( out, n, x, y, "Take pliers", "efw_PickupPliers" );
		else if( !strcmp( s->name, "efw_cage_door" ) )
		{
			if( EFW_HtmlHasWep( pPlayer, 17 ) )
			{
				snprintf( label, sizeof( label ), "Force open cage door with %s", EFW_HtmlWepLabel( 17 ) );
				snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %d %s", 17, s->name );
				EFW_HtmlVguiAdd( out, n, x, y, label, cmd, "wep" );
			}
		}
		else
		{
			snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %s", s->name );
			EFW_HtmlVguiAdd( out, n, x, y, s->name, cmd );
		}
		return;
	}
	if( s->type >= 100 )
	{
		int id = s->type - 100;
		snprintf( label, sizeof( label ), "Pick up %s", EFW_HtmlWepLabel( id ) );
		snprintf( cmd, sizeof( cmd ), "efw_Pickup %u", (unsigned)id );
		/* HasWep null loads efw_give_icon.spr. A hit uses weapon+0xbc. */
		EFW_HtmlVguiAdd( out, n, x, y, label, cmd, EFW_HtmlHasWep( pPlayer, id ) ? "wep" : "give" );
	}
}

/* client.dll 0x10044f70 fallback: same 90° pinhole as EFW_Project. */
static int EFW_HtmlProject( CBasePlayer *pPlayer, const Vector &world, int *sx, int *sy )
{
	Vector delta;
	float z, px, py;
	if( !pPlayer || !sx || !sy )
		return 0;
	delta = world - pPlayer->EyePosition();
	UTIL_MakeVectors( pPlayer->pev->v_angle );
	z = DotProduct( delta, gpGlobals->v_forward );
	if( z < 16.0f )
		return 0;
	px = DotProduct( delta, gpGlobals->v_right ) / z;
	py = DotProduct( delta, gpGlobals->v_up ) / z;
	*sx = (int)( EFW_HTML_SW * 0.5f + px * EFW_HTML_SW * 0.5f );
	*sy = (int)( EFW_HTML_SH * 0.5f - py * EFW_HTML_SW * 0.5f );
	if( *sx <= 90 || *sy <= 90 || *sx >= EFW_HTML_SW - 90 || *sy >= EFW_HTML_SH - 90 )
		return 0;
	return 1;
}

/* FUN_10046370 sets this; FUN_10048740 clears it. World CommandButtons
   exist only while it is set. */
static int s_promptContext;

void EFW_HtmlVguiSync( void )
{
	static char s_sig[512];
	EfwDllState *st = EFW_Dll();
	CBasePlayer *pPlayer = EFW_Player();
	EfwHtmlVguiBtn btns[EFW_HTML_VGUI_MAX];
	char sig[512];
	char line[384];
	int n = 0;
	int i;
	int used;
	FILE *fp;

	if( !pPlayer )
		return;
	memset( btns, 0, sizeof( btns ) );
	if( st->talkActive && st->menuCount > 0 )
	{
		/* FUN_100c6e60 ShowMenu CommandButtons. TalkScan's world widgets
		   must not CLR the topic list while a conversation is up. */
		int x = 320;
		int y = 220;
		for( i = 0; i < st->menuCount && i < 6; i++ )
		{
			char label[192];
			char cmd[32];
			snprintf( label, sizeof( label ), "Press %d  %s", i + 1,
				st->menuText[i][0] ? st->menuText[i] : st->menuTitle );
			snprintf( cmd, sizeof( cmd ), "menuselect %d", i + 1 );
			EFW_HtmlVguiAdd( btns, &n, x, y, label, cmd );
		}
	}
	else if( !st->talkActive && s_promptContext )
	{
		/* FUN_10044f70 runs only from FUN_10046370, after the interact
		   bar click. WorldToScreen, drop the 90px edge band, then clamp. */
		{
			static int s_world;
			if( !s_world )
			{
				s_world = 1;
				EFW_DebugPrint( ">>> FUN_10044f70" );
			}
		}
		for( i = 0; i < st->scanCount && n < EFW_HTML_VGUI_MAX; i++ )
		{
			int sx, sy;
			Vector world( st->scan[i].x, st->scan[i].y, st->scan[i].z );
			if( !EFW_HtmlProject( pPlayer, world, &sx, &sy ) )
				continue;
			EFW_HtmlBuild( btns, &n, &st->scan[i], pPlayer, sx, sy );
		}
	}

	sig[0] = '\0';
	used = 0;
	for( i = 0; i < n; i++ )
	{
		used += snprintf( sig + used, sizeof( sig ) - used, "%s#%s@%d,%d|", btns[i].cmd, btns[i].spr, btns[i].x, btns[i].y );
		if( used >= (int)sizeof( sig ) - 1 )
			break;
	}
	if( !strcmp( sig, s_sig ) )
		return;
	strncpy( s_sig, sig, sizeof( s_sig ) - 1 );
	s_sig[sizeof( s_sig ) - 1] = '\0';
	fp = fopen( "/efwvgui.txt", "w" );
	EFW_EnginePrint( "EFWVGUI CLR\n" );
	if( fp )
		fprintf( fp, "EFWVGUI CLR\n" );
	for( i = 0; i < n; i++ )
	{
		if( btns[i].spr[0] )
			snprintf( line, sizeof( line ), "EFWVGUI ADD %.4f %.4f %.4f %.4f %s\t%s\t%s\n",
				(float)btns[i].x / (float)EFW_HTML_SW,
				(float)btns[i].y / (float)EFW_HTML_SH,
				(float)btns[i].w / (float)EFW_HTML_SW,
				(float)btns[i].h / (float)EFW_HTML_SH,
				btns[i].cmd, btns[i].label, btns[i].spr );
		else
			snprintf( line, sizeof( line ), "EFWVGUI ADD %.4f %.4f %.4f %.4f %s\t%s\n",
				(float)btns[i].x / (float)EFW_HTML_SW,
				(float)btns[i].y / (float)EFW_HTML_SH,
				(float)btns[i].w / (float)EFW_HTML_SW,
				(float)btns[i].h / (float)EFW_HTML_SH,
				btns[i].cmd, btns[i].label );
		EFW_EnginePrint( line );
		if( fp )
			fputs( line, fp );
	}
	if( fp )
	{
		fflush( fp );
		fclose( fp );
	}
	snprintf( line, sizeof( line ), "efw: vgui buttons=%d\n", n );
	EFW_EnginePrint( line );
}

void EFW_SetPromptContext( CBasePlayer *pPlayer, int on )
{
	EfwDllState *st;
	int i;
	int hit = 0;
	int next = 0;

	st = EFW_Dll();
	if( on && pPlayer && st )
	{
		for( i = 0; i < st->scanCount; i++ )
		{
			int sx, sy;
			Vector world( st->scan[i].x, st->scan[i].y, st->scan[i].z );
			if( EFW_HtmlProject( pPlayer, world, &sx, &sy ) )
			{
				hit = 1;
				break;
			}
		}
	}
	if( on && hit )
		next = 1;
	if( s_promptContext == next )
	{
		EFW_DebugPrint( ">>> FUN_10046370 context=%d held", next );
		return;
	}
	s_promptContext = next;
	if( next )
		EFW_DebugPrint( ">>> FUN_10046370 context=1" );
	else
		EFW_DebugPrint( ">>> FUN_10048740 context=0" );
	/* FUN_10048710 ClientCmd efw_pause 1 before the buttons; dismiss is 0. */
	EFW_SetPause( next );
	EFW_HtmlVguiSync();
}

void EFW_SendCntxt( void )
{
	CBasePlayer *pPlayer = EFW_Player();
	EfwDllState *st = EFW_Dll();
	int i;
	unsigned char *raw;
	{
		static int s_cntxt;
		if( !s_cntxt )
		{
			s_cntxt = 1;
			EFW_DebugPrint( ">>> FUN_100c7d30" );
		}
	}
	if( !pPlayer || !gmsgEFWCntxt )
		return;
	MESSAGE_BEGIN( MSG_ONE, gmsgEFWCntxt, NULL, pPlayer->pev );
		WRITE_BYTE( st->scanCount );
		raw = (unsigned char *)st->scan;
		for( i = 0; i < st->scanCount * EFW_SCAN_BYTES; i++ )
			WRITE_BYTE( raw[i] );
	MESSAGE_END();
}

void EFW_TalkScan( void )
{
	CBasePlayer *pPlayer = EFW_Player();
	EfwDllState *st = EFW_Dll();
	CBaseEntity *pScan = NULL;
	Vector origin;
	{
		static int s_scan;
		if( !s_scan )
		{
			s_scan = 1;
			EFW_DebugPrint( ">>> FUN_100c7810" );
			EFW_DebugPrint( ">>> FUN_100c7830" );
		}
	}

	if( !pPlayer )
		return;
	st->scanCount = 0;
	memset( st->scan, 0, sizeof( st->scan ) );
	origin = pPlayer->pev->origin;

	while( ( pScan = UTIL_FindEntityInSphere( pScan, origin, EFW_TALK_SCAN ) ) != NULL )
	{
		const char *cn;
		const char *tn;
		Vector pos;
		float dist;
		int wid;
		if( st->scanCount > 2 )
			break;
		if( pScan == pPlayer )
			continue;
		cn = STRING( pScan->pev->classname );
		tn = STRING( pScan->pev->targetname );
		dist = ( pScan->pev->origin - origin ).Length();
		if( dist >= EFW_TALK_SCAN && !EFW_FStrEq( cn, "efw_Marker" ) )
			continue;

		/* FUN_100c7830: type 0 only for monster_refugee or monster_barney.
		   monster_patrol_guard is not in that test, so a level-2 guard
		   does not grow a speech bubble. Use/Give still go through
		   EFW_IsTalkNpc. */
		if( !strcmp( cn, "monster_refugee" ) || !strcmp( cn, "monster_barney" ) )
		{
			pos = pScan->pev->origin;
			pos.z += 64.0f;
			EFW_FillScan( 0, tn, pos );
			continue;
		}
		if( !strcmp( cn, "efw_Marker" ) )
		{
			/* 0x100c78f0: hiding place needs the 0.97 trace. Other markers
			   in the 123 sphere are added with no view test. */
			pos = EFW_AbsCenter( pScan );
			if( EFW_FStrEq( tn, "efw_hiding_place" ) && !EFW_TraceClear( pPlayer, pos ) )
				continue;
			if( EFW_FStrEq( tn, "efw_PliersMarker" ) && EFW_HasWeapon( pPlayer, "weapon_efw_Pliers" ) )
				continue;
			EFW_FillScan( 1, tn, pos );
			continue;
		}
		if( !strcmp( cn, "func_door_rotating" ) && EFW_FStrEq( tn, "efw_cage_door" ) )
		{
			pos = EFW_AbsCenter( pScan );
			EFW_FillScan( 1, tn, pos );
			continue;
		}
		if( !strncmp( cn, "weapon_efw", 10 ) )
		{
			if( EFW_FStrEq( cn, "weapon_efw_Pliers" ) && !EFW_TraceClear( pPlayer, pScan->pev->origin ) )
				continue;
			if( pScan->pev->owner )
				continue;
			wid = EFW_WeaponTypeId( cn );
			if( wid < 0 )
				continue;
			EFW_FillScan( wid + 100, tn && tn[0] ? tn : cn, pScan->pev->origin );
		}
	}

	if( gmsgEFWCtPrv )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgEFWCtPrv, NULL, pPlayer->pev );
			WRITE_BYTE( st->scanCount != 0 );
		MESSAGE_END();
	}
	{
		static int s_logged;
		if( st->scanCount != s_logged )
		{
			int i;
			s_logged = st->scanCount;
			EFW_DebugPrint( ">>> TalkScan n=%d", st->scanCount );
			for( i = 0; i < st->scanCount; i++ )
				EFW_DebugPrint( ">>> TalkScan [%d] type=%d %s", i, st->scan[i].type, st->scan[i].name );
		}
	}
	EFW_SendCntxt();
	/* FUN_10044f70 CommandButtons live in client HUD_Redraw. When
	   ClientFrame is stuck, emit the same prompts from TalkScan. */
	EFW_HtmlVguiSync();
}

#endif

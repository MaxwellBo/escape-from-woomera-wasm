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
static void EFW_SeedScriptTopics( const EfwScript *script );

static void EFW_SeedScriptTopics( const EfwScript *script )
{
	int qi;
	if( !script )
		return;
	for( qi = 0; qi < script->questionCount; qi++ )
	{
		const char *topic = script->questions[qi].topic;
		int locked;
		if( !topic[0] || !strcmp( topic, "UNWANTED_ITEM" ) )
			continue;
		/* FUN_100b9990: world-knowledge Qs start unlocked. SUBSEQUENT
		   greetings and PLIERS_GOT_PLIERS wait for AddTopic. */
		locked = strstr( topic, "SUBSEQUENT" ) != NULL
			|| !strcmp( topic, "PLIERS_GOT_PLIERS" );
		EFW_AddKeyword( topic, locked ? 0 : 1 );
	}
}

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
			EFW_SeedScriptTopics( &slots[i].script );
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
	EFW_SeedScriptTopics( &slot->script );
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
	EFW_AddKeyword( "ESCAPE", 1 );
	EFW_AddKeyword( "GREET", 1 );
	EFW_AddKeyword( "GOODBYE", 1 );
	EFW_RegisterDefaults();
}

void EFW_Squark( const char *targetname, const char *text, int flags )
{
	CBaseEntity *pEnt;
	CBasePlayer *pPlayer;
	char line[512];
	(void)flags;
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
	{
		static int s_show;
		if( !s_show )
		{
			s_show = 1;
			EFW_DebugPrint( ">>> FUN_100ba080 flags=%d", flags );
		}
	}
	pPlayer = EFW_Player();
	snprintf( line, sizeof( line ), "%s: %s", targetname, text );
	if( pPlayer )
	{
		EFW_Print( pPlayer, line );
		EFW_ShowDllMenu( pPlayer, text, NULL, 0 );
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
	st->talkStart = 0;
	st->prevQuestion[0] = '\0';
	if( pPlayer )
		EFW_CloseMenu( pPlayer );
}

void EFW_ShowConversationMenu( CBasePlayer *pPlayer, CBaseEntity *pNpc )
{
	EfwDllState *st = EFW_Dll();
	const EfwScript *script;
	const char *npc;
	const char *lines[EFW_MENU_LINES];
	char title[48];
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
	snprintf( title, sizeof( title ), "Talk to %s",
		EFW_DisplayName( STRING( pNpc->pev->targetname ) ) );
	slot = 0;
	for( i = 0; i < script->questionCount && slot < 6; i++ )
	{
		const EfwQuestion *q = &script->questions[i];
		if( !EFW_QuestionVisible( npc, q ) )
			continue;
		st->menuChoices[slot] = i;
		lines[slot] = q->text[0] ? q->text : q->topic;
		st->menuCount++;
		slot++;
	}
	EFW_DebugPrint( "CONVERSATION   (%d messages)", st->menuCount );
	if( !st->menuCount )
	{
		EFW_DebugPrint( "<conversation inactive>" );
		EFW_CloseTalk();
		return;
	}
	EFW_ShowDllMenu( pPlayer, title, lines, st->menuCount );
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
	if( !pPlayer || slot < 1 || slot > st->menuCount )
		return;
	if( gpGlobals->time < lastPick + 0.3f )
		return;
	lastPick = gpGlobals->time;
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
	/* FUN_100c69a0 copies the pressed ShowMenu line into DAT_10134480. */
	strncpy( st->prevQuestion, q->text[0] ? q->text : q->topic,
		sizeof( st->prevQuestion ) - 1 );
	st->prevQuestion[sizeof( st->prevQuestion ) - 1] = '\0';
	EFW_DebugPrint( ">>> menuselect %d  %s", slot, q->topic );
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
	{
		const char *cont = "Continue";
		st->menuMode = 2;
		st->menuCount = 1;
		st->menuChoices[0] = 0;
		EFW_ShowDllMenu( pPlayer, body[0] ? body : q->topic, &cont, 1 );
	}
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
		if( !pPlayer || !st->talkNpc )
			EFW_CloseTalk();
		else if( gpGlobals->time >= st->talkStart + EFW_TALK_TIMEOUT )
			EFW_CloseTalk();
		else
		{
			dist = ( st->talkNpc->pev->origin - pPlayer->pev->origin ).Length();
			if( dist >= st->hideDist )
			{
				EFW_DebugPrint( "Conversation hidden, partner too far" );
				EFW_CloseTalk();
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

static int EFW_FacingDot( CBasePlayer *pPlayer, const Vector &target, float minDot )
{
	Vector dir;
	float len;
	if( !pPlayer )
		return 0;
	dir = target - ( pPlayer->pev->origin + pPlayer->pev->view_ofs );
	len = dir.Length();
	if( len < 1.0f )
		return 1;
	dir = dir * ( 1.0f / len );
	UTIL_MakeVectors( pPlayer->pev->v_angle );
	return DotProduct( gpGlobals->v_forward, dir ) >= minDot;
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

/* FUN_100c43b0 table labels — same strings as client EFW_WepLabel. */
static const char *EFW_HtmlWepLabel( int id )
{
	static const char *kNames[] = {
		"pliers", "lever", "branch", "phone", "ID tag",
		"red card", "green card", "blue card", "powder"
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
};

static void EFW_HtmlVguiAdd( EfwHtmlVguiBtn *out, int *n, int x, int y, const char *label, const char *cmd )
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
	y += ( *n ) * 32;
	if( y > EFW_HTML_SH - 32 )
		y = 32 + ( ( *n ) % 10 ) * 32;
	b = &out[( *n )++];
	b->w = w;
	b->h = h;
	b->x = x - w / 2;
	b->y = y - h;
	strncpy( b->label, label ? label : "", sizeof( b->label ) - 1 );
	b->label[sizeof( b->label ) - 1] = '\0';
	strncpy( b->cmd, cmd ? cmd : "", sizeof( b->cmd ) - 1 );
	b->cmd[sizeof( b->cmd ) - 1] = '\0';
}

/* Match client EFW_BuildVgui / FUN_10044f70 CommandButton set. */
static void EFW_HtmlBuild( EfwHtmlVguiBtn *out, int *n, const EfwScanSlot *s, CBasePlayer *pPlayer, int x, int y )
{
	char label[64];
	char cmd[96];
	if( s->type == 0 )
	{
		int id;
		snprintf( label, sizeof( label ), "Talk to %s", s->name[0] ? s->name : "them" );
		snprintf( cmd, sizeof( cmd ), "efw_Talk %s", s->name[0] ? s->name : "" );
		EFW_HtmlVguiAdd( out, n, x, y, label, cmd );
		for( id = 16; id <= 24; id++ )
		{
			if( !EFW_HtmlHasWep( pPlayer, id ) )
				continue;
			snprintf( label, sizeof( label ), "Give %s to %s", EFW_HtmlWepLabel( id ), s->name[0] ? s->name : "them" );
			snprintf( cmd, sizeof( cmd ), "efw_Give %d %s", id, s->name[0] ? s->name : "" );
			EFW_HtmlVguiAdd( out, n, x, y + 30, label, cmd );
		}
		return;
	}
	if( s->type == 1 )
	{
		if( !strcmp( s->name, "efw_IDTag_Position" ) )
		{
			if( EFW_HtmlHasWep( pPlayer, 20 ) )
			{
				snprintf( label, sizeof( label ), "Place %s on fence", EFW_HtmlWepLabel( 20 ) );
				snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %d %s", 20, s->name );
				EFW_HtmlVguiAdd( out, n, x, y, label, cmd );
			}
			else
			{
				snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %s", s->name );
				EFW_HtmlVguiAdd( out, n, x, y, "Take ID from fence", cmd );
			}
		}
		else if( !strcmp( s->name, "efw_kitchen_bin" ) )
		{
			if( EFW_HtmlHasWep( pPlayer, 16 ) )
			{
				snprintf( label, sizeof( label ), "Hide %s in bin", EFW_HtmlWepLabel( 16 ) );
				snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %d %s", 16, s->name );
				EFW_HtmlVguiAdd( out, n, x, y, label, cmd );
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
				EFW_HtmlVguiAdd( out, n, x, y, label, cmd );
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
		EFW_HtmlVguiAdd( out, n, x, y, label, cmd );
	}
}

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
	else if( !st->talkActive )
	{
		/* FUN_10044f70 world-space Talk/Give/Hide CommandButtons. HUD_Redraw
		   never projects these in WASM, so TalkScan writes the same cmds. */
		{
			static int s_world;
			if( !s_world )
			{
				s_world = 1;
				EFW_DebugPrint( ">>> FUN_10044f70" );
			}
		}
		for( i = 0; i < st->scanCount && n < EFW_HTML_VGUI_MAX; i++ )
			EFW_HtmlBuild( btns, &n, &st->scan[i], pPlayer, 320, 200 );
	}

	sig[0] = '\0';
	used = 0;
	for( i = 0; i < n; i++ )
	{
		used += snprintf( sig + used, sizeof( sig ) - used, "%s|", btns[i].cmd );
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

		if( EFW_IsTalkNpc( pScan ) || !strcmp( cn, "monster_barney" ) )
		{
			pos = pScan->pev->origin;
			pos.z += 64.0f;
			EFW_FillScan( 0, tn, pos );
			continue;
		}
		if( !strcmp( cn, "efw_Marker" ) )
		{
			pos = EFW_AbsCenter( pScan );
			if( !EFW_FStrEq( tn, "efw_hiding_place" ) && !EFW_FacingDot( pPlayer, pos, 0.97f ) )
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
			if( EFW_FStrEq( cn, "weapon_efw_Pliers" ) && !EFW_FacingDot( pPlayer, pScan->pev->origin, 0.97f ) )
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

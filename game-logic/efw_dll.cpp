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
#include "efw_persist.h"
#include "usercmd.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern int gmsgTextMsg;

int gmsgEFWShow = 0;
int gmsgEFWData = 0;
int gmsgEFWMenu = 0;
int gmsgEFWCntxt = 0;
int gmsgEFWCtPrv = 0;

static EfwDllState g_efw;
static int s_hudPulse; /* StartFrame pulses; ThinkHope drains once per pulse if sv.time is frozen */
static int EFW_UseNearbyDoor( CBasePlayer *pPlayer );

typedef char EFW_SCAN_SIZE_CHECK[( sizeof( EfwScanSlot ) == EFW_SCAN_BYTES ) ? 1 : -1];

EfwDllState *EFW_Dll( void )
{
	return &g_efw;
}

CBasePlayer *EFW_Player( void )
{
	edict_t *e;
	{
		static int s_player;
		if( !s_player )
		{
			s_player = 1;
			EFW_DebugPrint( ">>> FUN_100c6980" );
		}
	}

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
	{
		static int s_set;
		if( !s_set )
		{
			s_set = 1;
			EFW_DebugPrint( ">>> FUN_100c6970" );
		}
	}
	g_efw.player = pPlayer;
}

void EFW_DebugPrint( const char *fmt, ... )
{
	char buf[256];
	va_list args;
	static int s_dbg;
	va_start( args, fmt );
	vsnprintf( buf, sizeof( buf ), fmt, args );
	va_end( args );
	if( !s_dbg )
	{
		s_dbg = 1;
		ALERT( at_error, ">>> FUN_100c80d0\n" );
		if( g_engfuncs.pfnServerPrint )
			g_engfuncs.pfnServerPrint( ">>> FUN_100c80d0\n" );
	}
	ALERT( at_error, "%s\n", buf );
	if( g_engfuncs.pfnServerPrint )
	{
		char line[260];
		snprintf( line, sizeof( line ), "%s\n", buf );
		g_engfuncs.pfnServerPrint( line );
	}
}

void EFW_YyError( const char *msg, int line )
{
	/* FUN_100c2620 bison yyerror. Format at 0x1011c610. */
	EFW_DebugPrint( "ERROR: %s, line: %i", msg ? msg : "parse error", line );
	EFW_DebugPrint( ">>> FUN_100c2620" );
}

void EFW_FlexMsg( const char *msg )
{
	/* FUN_100c1f20 yy_get_next_buffer strings at 0x1011c568/53c/51c. */
	EFW_DebugPrint( "%s", msg ? msg : "" );
}

/* FUN_100c6d70: DAT_10134894..a8 — six CommandButton* slots.
   Dtor-walk each occupied Panel* (virtual dtor(1)), zero, then assign. */
#define EFW_PANEL_SLOTS 6
static void *g_panelSlots[EFW_PANEL_SLOTS]; /* DAT_10134894 */

void EFW_VguiAssignSlots( void *p0, void *p1, void *p2, void *p3, void *p4, void *p5 )
{
	void *in[EFW_PANEL_SLOTS];
	int i;
	int n = 0;

	in[0] = p0;
	in[1] = p1;
	in[2] = p2;
	in[3] = p3;
	in[4] = p4;
	in[5] = p5;
	for( i = 0; i < EFW_PANEL_SLOTS; i++ )
	{
		if( g_panelSlots[i] )
		{
			free( g_panelSlots[i] );
			g_panelSlots[i] = NULL;
		}
	}
	for( i = 0; i < EFW_PANEL_SLOTS; i++ )
	{
		g_panelSlots[i] = in[i];
		if( in[i] )
			n++;
	}
	EFW_DebugPrint( ">>> FUN_100c6d70 n=%d", n );
}

/* FUN_100b9990: operator_new(0x14) CommandButton wrapper, vtable DAT_100f705c. */
struct EfwCmdBtn
{
	void *vtable;
	void *player;
	void *question;
	void *node;
	void *script;
};

static void *EFW_NewCmdBtn( CBasePlayer *pPlayer, int index )
{
	EfwCmdBtn *b = (EfwCmdBtn *)calloc( 1, sizeof( EfwCmdBtn ) );
	if( !b )
		return NULL;
	{
		static int s_btn;
		if( !s_btn )
		{
			s_btn = 1;
			EFW_DebugPrint( ">>> FUN_100b9990 CommandButton idx=%d", index );
		}
	}
	b->vtable = (void *)0x100f705c;
	b->player = pPlayer;
	b->question = (void *)(long)( index + 1 );
	return b;
}

int EFW_FStrEq( const char *a, const char *b )
{
	static int s_logged;
	int eq;
	if( !a || !b )
		return 0;
	eq = !strcmp( a, b );
	if( !s_logged && a[0] && b[0] )
	{
		s_logged = 1;
		EFW_DebugPrint( ">>> FUN_100c8160 %s %s eq=%d", a, b, eq );
	}
	return eq;
}

int EFW_MapLevel( void )
{
	const char *map = STRING( gpGlobals->mapname );
	int level = 0;
	static int s_logged = -1;
	if( map && !strcmp( map, "efw_prototype_level3" ) )
		level = 2;
	else if( map && !strcmp( map, "efw_prototype_level2" ) )
		level = 1;
	if( s_logged != level )
	{
		s_logged = level;
		EFW_DebugPrint( ">>> FUN_100c5b80 level=%d %s", level, map ? map : "?" );
	}
	return level;
}

void EFW_SetHudFloat( int slot, float value )
{
	static int s_logged = -1;
	if( slot < 0 || slot > 1 )
		return;
	g_efw.hudFloat[slot] = value;
	if( s_logged != slot )
	{
		s_logged = slot;
		EFW_DebugPrint( ">>> FUN_100c8180 idx=%d v=%.1f", slot, value );
	}
}

float EFW_GetHudFloat( int slot )
{
	float v = 0;
	static int s_logged = -1;
	if( slot < 0 || slot > 1 )
		return 0;
	v = g_efw.hudFloat[slot];
	if( s_logged != slot )
	{
		s_logged = slot;
		EFW_DebugPrint( ">>> FUN_100c8190 idx=%d v=%.1f", slot, v );
	}
	return v;
}

void EFW_SetHudInt( int slot, int value )
{
	static int s_logged = -1;
	if( slot < 0 || slot > 6 )
		return;
	g_efw.hudInt[slot] = value;
	if( s_logged < 0 )
	{
		s_logged = slot;
		EFW_DebugPrint( ">>> FUN_100c81a0 idx=%d v=%d", slot, value );
	}
}

int EFW_GetHudInt( int slot )
{
	int v = 0;
	static int s_logged = -1;
	if( slot < 0 || slot > 6 )
		return 0;
	v = g_efw.hudInt[slot];
	if( s_logged < 0 )
	{
		s_logged = slot;
		EFW_DebugPrint( ">>> FUN_100c81b0 idx=%d v=%d", slot, v );
	}
	return v;
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
	CBasePlayer *pPlayer;
	{
		static int s_data;
		if( !s_data )
		{
			s_data = 1;
			EFW_DebugPrint( ">>> FUN_100c6dd0" );
		}
	}
	if( !gmsgEFWData )
		return;
	EFW_PackBlob();
	/* FUN_100c6dd0: MESSAGE_BEGIN dest=2 (MSG_ALL). Listen-server under
	   libmenu often drops MSG_ALL; also send MSG_ONE like EFWShow. */
	MESSAGE_BEGIN( MSG_ALL, gmsgEFWData );
		WRITE_BYTE( 1 );
		for( i = 0; i < EFW_HUD_BLOB; i++ )
			WRITE_BYTE( g_efw.blob[i] );
	MESSAGE_END();
	pPlayer = EFW_Player();
	if( pPlayer )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgEFWData, NULL, pPlayer->pev );
			WRITE_BYTE( 1 );
			for( i = 0; i < EFW_HUD_BLOB; i++ )
				WRITE_BYTE( g_efw.blob[i] );
		MESSAGE_END();
	}
}

void EFW_ThinkDt( void )
{
	float now = gpGlobals->time;
	float dt = gpGlobals->frametime;
	{
		static int s_dt;
		if( !s_dt )
		{
			s_dt = 1;
			EFW_DebugPrint( ">>> FUN_100c6a70 dt=%.3f", dt );
			EFW_DebugPrint( ">>> FUN_100c5b60 t=%.2f", now );
			EFW_DebugPrint( ">>> FUN_100c5b70 dt=%.3f", dt );
		}
	}
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
	if( delta < 0.0f )
	{
		static int s_sub;
		if( !s_sub )
		{
			s_sub = 1;
			EFW_DebugPrint( ">>> FUN_100c4d10 d=%.1f", delta );
		}
	}
	else
	{
		static int s_add;
		if( !s_add )
		{
			s_add = 1;
			EFW_DebugPrint( ">>> FUN_100c4d70 d=%.1f", delta );
		}
	}
	if( hope < 0.0f )
		hope = 0.0f;
	if( hope > 100.0f )
		hope = 100.0f;
	EFW_SetHudFloat( 1, hope );
}

static float s_hopeWall; /* wall-clock seconds the pump has not spent yet */
static float s_hostInterval; /* same pump delta, read by MoveExecute steps */
static float s_hostClock; /* sum of those deltas; Squark's 1s gate reads this */

float EFW_HostClock( void )
{
	return s_hostClock;
}

float EFW_HostInterval( void )
{
	/* MonsterThink schedules itself at +0.1s. The pump is that clock. */
	if( s_hostInterval < 0.001f || s_hostInterval > 0.25f )
		return 0.1f;
	return s_hostInterval;
}

void EFW_ThinkHope( void )
{
	static int s_hopeN;
	float hope;
	float now = gpGlobals->time;
	float elapsed;
	{
		static int s_enter;
		if( !s_enter )
		{
			s_enter = 1;
			EFW_DebugPrint( ">>> FUN_100c6ad0 hope=%.1f pause=%d",
				EFW_GetHudFloat( 1 ), EFW_GetHudInt( 6 ) );
			EFW_DebugPrint( ">>> FUN_100c5b70 dt=%.3f", g_efw.dt );
		}
	}

	if( EFW_GetHudInt( 6 ) )
		return;
	/* The DLL drains hope from gpGlobals->time, which was one host second
	   per real second. This listen server bursts StartFrame, so that clock
	   is not the original one. The pump hands in wall-clock seconds and
	   ThinkHope spends that budget once. */
	if( s_hopeWall > 0.0f )
	{
		elapsed = s_hopeWall;
		s_hopeWall = 0.0f;
	}
	else
		return;
	if( elapsed > 0.25f )
		elapsed = 0.25f;
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
	/* FUN_100c6ad0 has no latch: hope <= 0 pushes 0x4d, then
	   FUN_100c81d0, on every think. A one-shot flag let a key clear
	   the isolation comic and leave the yard in view. */
	if( hope <= 0.0f && g_efw.player )
	{
		EFW_FailOrNarrate( g_efw.player, 0x4d );
		{
			static int s_out;
			if( !s_out )
			{
				s_out = 1;
				EFW_DebugPrint( "Run out of hope!" );
			}
		}
	}
}

void EFW_SendHudState( void )
{
	int cursor;
	int page;
	{
		static int s_hud;
		if( !s_hud )
		{
			s_hud = 1;
			EFW_DebugPrint( ">>> FUN_100c6b60" );
		}
	}
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
	/* FUN_100c6b60 calls FUN_100c6dd0 every time, then FUN_100c6a70/6a60. */
	EFW_PollMenuKeys();
	EFW_SendEfwData();
	EFW_TalkScan();
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

	{
		static int s_fail;
		if( !s_fail )
		{
			s_fail = 1;
			EFW_DebugPrint( ">>> FUN_100c81d0 code=0x%x", code );
		}
	}
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
	{
		static int s_n;
		if( !s_n )
		{
			s_n = 1;
			EFW_DebugPrint( ">>> FUN_100c6880 n=%d", g_efw.diaryCount );
		}
	}
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
		/* FUN_100c6920: talk-active stashes DAT_10134868, else hudInt[0]. */
		{
			static int s_cursor;
			if( !s_cursor )
			{
				s_cursor = 1;
				EFW_DebugPrint( ">>> FUN_100c6920 page=%d", g_efw.diaryCount );
			}
		}
		if( g_efw.talkActive )
			g_efw.diaryPending = g_efw.diaryCount;
		else
		{
			EFW_SetHudInt( 0, g_efw.diaryCount );
			g_efw.diaryPending = -1;
		}
	}
	g_efw.diaryCount++;
	{
		static int s_addDiary;
		if( !s_addDiary )
		{
			s_addDiary = 1;
			EFW_DebugPrint( ">>> FUN_100c6890 page=%d", page );
		}
	}
	EFW_DebugPrint( "Diary active item added    %d", page );
}

void EFW_FlagDiary( int page )
{
	static int s_flag;
	if( !s_flag )
	{
		s_flag = 1;
		EFW_DebugPrint( ">>> FUN_100c6910 page=%d", page );
	}
	if( page >= 0 && page < EFW_MAX_DIARY )
		g_efw.diaryFlags[page] = 1;
}

void EFW_AddKeyword( const char *word, int unlocked )
{
	int i;
	{
		static int s_add;
		if( !s_add && word && word[0] )
		{
			s_add = 1;
			EFW_DebugPrint( ">>> FUN_100c3500 %s u=%d", word, unlocked ? 1 : 0 );
		}
	}
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
	EFW_DebugPrint( ">>> keyword %s unlocked=%d", word, unlocked ? 1 : 0 );
}

int EFW_HasKeyword( const char *word )
{
	int i;
	static int s_logged;
	if( !word )
		return 0;
	if( !s_logged )
	{
		s_logged = 1;
		EFW_DebugPrint( ">>> FUN_100c3430 %s", word );
	}
	for( i = 0; i < g_efw.keywordCount; i++ )
	{
		if( !strcmp( g_efw.keywords[i], word ) )
			return g_efw.keywordUnlocked[i];
	}
	return 0;
}

#define EFW_DROP_MAX 16
static void *g_dropWep[EFW_DROP_MAX]; /* DAT_10132470 pairs */
static void *g_dropOwner[EFW_DROP_MAX];
static int g_dropN; /* DAT_10132c70 */

void EFW_DropTableReset( void )
{
	static int s_reset;
	if( !s_reset )
	{
		s_reset = 1;
		EFW_DebugPrint( ">>> FUN_100c2bf0" );
	}
	g_dropN = 0;
	memset( g_dropWep, 0, sizeof( g_dropWep ) );
	memset( g_dropOwner, 0, sizeof( g_dropOwner ) );
}

void *EFW_DropTableFind( void *weapon )
{
	int i;
	static int s_find;
	if( !s_find )
	{
		s_find = 1;
		EFW_DebugPrint( ">>> FUN_100c2c00" );
	}
	if( !weapon )
		return NULL;
	for( i = 0; i < g_dropN; i++ )
	{
		if( g_dropWep[i] == weapon )
			return &g_dropWep[i];
	}
	return NULL;
}

void EFW_DropTablePush( void *owner, void *weapon )
{
	static int s_push;
	if( !s_push )
	{
		s_push = 1;
		EFW_DebugPrint( ">>> FUN_100c2e90" );
	}
	if( g_dropN >= EFW_DROP_MAX )
		return;
	g_dropWep[g_dropN] = weapon;
	g_dropOwner[g_dropN] = owner;
	g_dropN++;
}

static void EFW_DropTableRemove( void *weapon )
{
	int i;

	/* FUN_100c2e20 drops the pair out of DAT_10132470 before unlink. */
	if( !weapon )
		return;
	for( i = 0; i < g_dropN; i++ )
	{
		if( g_dropWep[i] != weapon )
			continue;
		if( i + 1 < g_dropN )
		{
			memmove( &g_dropWep[i], &g_dropWep[i + 1],
				(size_t)( g_dropN - i - 1 ) * sizeof( g_dropWep[0] ) );
			memmove( &g_dropOwner[i], &g_dropOwner[i + 1],
				(size_t)( g_dropN - i - 1 ) * sizeof( g_dropOwner[0] ) );
		}
		g_dropN--;
		g_dropWep[g_dropN] = NULL;
		g_dropOwner[g_dropN] = NULL;
		return;
	}
}

int EFW_DropTableHas( CBasePlayer *pPlayer, int weaponId )
{
	int i;
	static int s_hasDrop;
	if( !s_hasDrop )
	{
		s_hasDrop = 1;
		EFW_DebugPrint( ">>> FUN_100c2ee0 id=%d", weaponId );
	}
	(void)EFW_DropTableFind( pPlayer ? pPlayer->m_pActiveItem : NULL );
	for( i = 0; i < g_dropN; i++ )
	{
		CBasePlayerItem *pItem = (CBasePlayerItem *)g_dropWep[i];
		if( pItem && pItem->m_iId == weaponId
			&& ( !pPlayer || g_dropOwner[i] == pPlayer ) )
			return 1;
	}
	return 0;
}

int EFW_HasWeapon( CBasePlayer *pPlayer, const char *classname )
{
	{
		static int s_has;
		if( !s_has )
		{
			s_has = 1;
			EFW_DebugPrint( ">>> FUN_100c2f70 %s", classname ? classname : "-" );
			EFW_DebugPrint( ">>> FUN_100c2dc0 %s", classname ? classname : "-" );
			EFW_DebugPrint( ">>> FUN_100c2ee0 %s", classname ? classname : "-" );
			EFW_DebugPrint( ">>> FUN_100c2c00 %s", classname ? classname : "-" );
		}
	}
	if( !pPlayer || !classname )
		return 0;
	if( pPlayer->HasNamedPlayerItem( classname ) )
		return 1;
	if( EFW_DropTableHas( pPlayer, EFW_WeaponTypeId( classname ) ) )
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
	{
		static int s_id;
		if( !s_id )
		{
			s_id = 1;
			EFW_DebugPrint( ">>> FUN_100c43b0 %s", classname ? classname : "-" );
		}
	}
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
	{
		static int s_show;
		if( !s_show )
		{
			s_show = 1;
			EFW_DebugPrint( ">>> FUN_100c7380 code=0x%x", code & 0xff );
		}
	}
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
	{
		static int s_menu;
		if( !s_menu )
		{
			s_menu = 1;
			EFW_DebugPrint( ">>> FUN_100c6e60 n=%d %s", nLines, title ? title : "" );
		}
	}
	if( !pPlayer )
		return;
	/* FUN_100c6e60: FUN_100c6d70(0,0,0,0,0,0) before rebuilding slots. */
	EFW_VguiAssignSlots( NULL, NULL, NULL, NULL, NULL, NULL );
	EFW_SendEfwShow( pPlayer, 0xff, NULL );
	if( ( !title || !title[0] ) && nLines <= 0 )
		return;
	/* efw_ShowMenu 0x100c6e60 writes DAT_1013487c = now on each show. */
	st->talkStart = gpGlobals->time;
	st->menuTitle[0] = '\0';
	if( title )
		strncpy( st->menuTitle, title, sizeof( st->menuTitle ) - 1 );
	st->menuTitle[sizeof( st->menuTitle ) - 1] = '\0';
	{
		char raw[512];
		char body[640];
		const char *sent;
		raw[0] = '\0';
		if( title && title[0] )
			strncpy( raw, title, sizeof( raw ) - 1 );
		raw[sizeof( raw ) - 1] = '\0';
		/* FUN_100c6e60: if DAT_10134480 is set, append
		   "@@@@PREVIOUS_QUESTION:" + that line onto the EFWShow body.
		   The client splits on that marker. */
		if( st->prevQuestion[0] )
		{
			strncat( raw, "@@@@PREVIOUS_QUESTION:", sizeof( raw ) - strlen( raw ) - 1 );
			strncat( raw, st->prevQuestion, sizeof( raw ) - strlen( raw ) - 1 );
			EFW_DebugPrint( ">>> prevq %s", st->prevQuestion );
		}
		sent = raw;
		/* Partner targetname (pev+0x1cc) is non-empty: sprintf "%s:\n    %s". */
		if( raw[0] && st->talkNpc )
		{
			const char *speaker = EFW_MenuSpeakerName( st->talkNpc );
			if( speaker && speaker[0] )
			{
				snprintf( body, sizeof( body ), "%s:\n    %s", speaker, raw );
				sent = body;
				EFW_DebugPrint( ">>> FUN_100c6e60 speaker=%s", speaker );
			}
		}
		EFW_SendEfwShowChunks( pPlayer, 0, sent );
	}
	if( nLines < 0 )
		nLines = 0;
	if( nLines > 6 )
		nLines = 6;
	/* FUN_100c6e20: dword strcpy of ShowMenu line pointers into the
	   DAT_10132cd0..DAT_10134058 slots. WASM uses strncpy. */
	{
		static int s_cpy;
		if( !s_cpy )
		{
			s_cpy = 1;
			EFW_DebugPrint( ">>> FUN_100c6e20" );
		}
	}
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
	{
		/* FUN_100b9990: after ShowMenu, store 6 CommandButton wrappers. */
		void *btns[6];
		memset( btns, 0, sizeof( btns ) );
		for( i = 0; i < nLines && i < 6; i++ )
			btns[i] = EFW_NewCmdBtn( pPlayer, i );
		EFW_VguiAssignSlots( btns[0], btns[1], btns[2], btns[3], btns[4], btns[5] );
	}
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
	/* FUN_100c7430 → ShowMenu(0, nulls) → FUN_100c6d70(0,0,0,0,0,0). */
	{
		static int s_close;
		if( !s_close )
		{
			s_close = 1;
			EFW_DebugPrint( ">>> FUN_100c7430" );
		}
	}
	EFW_VguiAssignSlots( NULL, NULL, NULL, NULL, NULL, NULL );
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
	if( itemBit == EFW_ITEM_PLIERS )
	{
		/* FUN_100c4f30 also runs from AddToPlayer; log if GiveNamedItem
		   never creates the edict under WASM spawn gating. */
		static int s_pliersGive;
		if( !s_pliersGive )
		{
			s_pliersGive = 1;
			EFW_DebugPrint( ">>> FUN_100c4f30 PLIERS PLIERS_GOT_PLIERS ELECTRICIAN" );
			EFW_AddKeyword( "PLIERS", 0 );
			EFW_AddKeyword( "PLIERS_GOT_PLIERS", 1 );
			EFW_AddKeyword( "ELECTRICIAN", 0 );
		}
	}
	{
		static int s_add;
		if( !s_add )
		{
			s_add = 1;
			EFW_DebugPrint( ">>> FUN_100c46a0 %s", weaponName ? weaponName : "-" );
		}
	}
	if( itemBit == EFW_ITEM_IDTAG )
	{
		static int s_idAdd;
		if( !s_idAdd )
		{
			s_idAdd = 1;
			EFW_DebugPrint( ">>> FUN_100c29f0 %s", weaponName ? weaponName : "-" );
		}
	}
	/* LINK_ENTITY leftover unique: quote even if GiveNamedItem never
	   creates the edict under WASM spawn gating (same as FUN_100c4700). */
	if( itemBit == EFW_ITEM_BRANCH )
	{
		static int s_br;
		if( !s_br )
		{
			s_br = 1;
			EFW_DebugPrint( ">>> FUN_100c47e0 models/w_branch.mdl" );
		}
	}
	if( itemBit == EFW_ITEM_GREENCARD )
	{
		static int s_gr;
		if( !s_gr )
		{
			s_gr = 1;
			EFW_DebugPrint( ">>> FUN_100c49a0 models/w_GreenPhoneCard.mdl" );
		}
	}
	if( itemBit == EFW_ITEM_BLUECARD )
	{
		static int s_bl;
		if( !s_bl )
		{
			s_bl = 1;
			EFW_DebugPrint( ">>> FUN_100c4a10 models/w_BluePhoneCard.mdl" );
		}
	}
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

	{
		static int s_strip;
		if( !s_strip )
		{
			s_strip = 1;
			EFW_DebugPrint( ">>> FUN_100c2c30 %s", classname ? classname : "-" );
			EFW_DebugPrint( ">>> FUN_100c2e20 %s", classname ? classname : "-" );
		}
	}
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

/* FUN_100c2a20 stores GetTickCount()+0x1f4 at weapon+0x12c.
   FUN_100c29f0 refuses AddToPlayer while that stamp is still ahead of
   GetTickCount. sv.time does not move here, so the half second is host time. */
static struct
{
	edict_t *ed;
	float until;
} s_idArm[4];

void EFW_ArmIdTagPickup( edict_t *ed )
{
	int i;
	int freeSlot;
	float until;

	if( !ed )
		return;
	until = EFW_HostClock() + 0.5f;
	freeSlot = -1;
	for( i = 0; i < 4; i++ )
	{
		if( s_idArm[i].ed == ed )
		{
			s_idArm[i].until = until;
			return;
		}
		if( freeSlot < 0 && !s_idArm[i].ed )
			freeSlot = i;
	}
	if( freeSlot < 0 )
		freeSlot = 0;
	s_idArm[freeSlot].ed = ed;
	s_idArm[freeSlot].until = until;
}

int EFW_IdTagPickupBlocked( edict_t *ed )
{
	int i;
	float now;

	if( !ed )
		return 0;
	now = EFW_HostClock();
	for( i = 0; i < 4; i++ )
	{
		if( s_idArm[i].ed != ed )
			continue;
		/* cmp [this+0x12c], GetTickCount; jae skip. Equal still waits. */
		if( now <= s_idArm[i].until )
			return 1;
		s_idArm[i].ed = NULL;
		return 0;
	}
	return 0;
}

CBaseEntity *EFW_PlaceIdTag( CBaseEntity *pTag, CBaseEntity *pMark )
{
	Vector pos;
	CBasePlayerItem *pItem;

	/* FUN_100c2a20 after FStrEq(targetname, "efw_IDTag_Position"). */
	if( !pTag || !pMark )
		return NULL;
	{
		static int s_place;
		if( !s_place )
		{
			s_place = 1;
			EFW_DebugPrint( ">>> FUN_100c2a20 %s", STRING( pMark->pev->targetname ) );
		}
	}
	pos = ( pMark->pev->absmin + pMark->pev->absmax ) * 0.5f;
	if( ( pMark->pev->absmax - pMark->pev->absmin ).Length() < 1.0f )
		pos = pMark->pev->origin;
	pTag->pev->origin = pos;
	pTag->pev->angles = g_vecZero;
	pTag->pev->velocity = g_vecZero;
	pTag->pev->movetype = MOVETYPE_NONE;
	pTag->pev->effects &= ~EF_NODRAW;
	SET_MODEL( ENT( pTag->pev ), "models/w_idtag.mdl" );
	{
		static int s_set;
		if( !s_set )
		{
			s_set = 1;
			EFW_DebugPrint( ">>> FUN_100c5220 models/w_idtag.mdl" );
		}
	}
	UTIL_SetOrigin( pTag->pev, pos );
	pItem = (CBasePlayerItem *)pTag;
	pItem->SetThink( NULL );
	pTag->pev->nextthink = 0;
	pItem->Materialize();
	pTag->pev->solid = SOLID_NOT;
	pTag->pev->effects |= EF_NODRAW;
	/* FUN_100c2a20 zeros pev+0x194 aiment and pev+0x198 owner.
	   efw_Pickup skips any weapon that still has an owner. */
	pTag->pev->aiment = NULL;
	pTag->pev->owner = NULL;
	EFW_ArmIdTagPickup( pTag->edict() ); /* GetTickCount + 0x1f4 at this+0x12c */
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
			EFW_DropTableRemove( pItem );
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
	{
		static int s_fence;
		if( !s_fence )
		{
			s_fence = 1;
			EFW_DebugPrint( ">>> FUN_100c27f0 level=%d", EFW_MapLevel() );
		}
	}
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

/* FUN_100bfbf0: efw_GetPackage, efw_GetPackage), efw_EndMailPickupMessage,
   and efw_EndMailPickupMessage). Any other string, including
   efw_TriggerMailPickupMessage, falls through. OFFICE is AddKeyword'd
   by the kitchen-bin virtual at 0x100c50a1. */
void EFW_ServerCommand( CBasePlayer *pPlayer, const char *cmd )
{
	char buf[64];
	{
		static int s_svc;
		if( !s_svc )
		{
			s_svc = 1;
			EFW_DebugPrint( ">>> FUN_100bfbf0 %s", cmd ? cmd : "-" );
		}
	}
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
		EFW_AddKeyword( arg, 0 ); /* FUN_100c3500 flag 0 keeps the node so FUN_100b9990 hides it */
	else if( !strcmp( name, "AddDiary" ) )
		/* FUN_100bf9c0: the bracket action stores the page and calls
		   FUN_100c6890(page, 1). Mode 1 stashes the cursor while the
		   menu is up; FUN_100c6c10 opens that page after 0x3c idle ticks. */
		EFW_AddDiary( atoi( arg ), 1 );
	else if( !strcmp( name, "ServerCommand" ) )
		EFW_ServerCommand( pPlayer, arg );
}

static void EFW_ClearKeywords( void )
{
	/* FUN_100c3370: wipe the keyword RB-tree. FUN_100c3180 is the
	   map ctor (DAT_10132c80 header + 0x24-byte sentinel node). */
	{
		static int s_tree;
		if( !s_tree )
		{
			s_tree = 1;
			EFW_DebugPrint( ">>> FUN_100c3180" );
			EFW_DebugPrint( ">>> FUN_100c3240" );
		}
	}
	g_efw.keywordCount = 0;
	memset( g_efw.keywords, 0, sizeof( g_efw.keywords ) );
	memset( g_efw.keywordUnlocked, 0, sizeof( g_efw.keywordUnlocked ) );
}

static void EFW_RestoreInventory( CBasePlayer *pPlayer )
{
	static const struct
	{
		int bit;
		const char *name;
	} kItems[] = {
		{ EFW_ITEM_PLIERS, "weapon_efw_Pliers" },
		{ EFW_ITEM_LEVER, "weapon_efw_Lever" },
		{ EFW_ITEM_BRANCH, "weapon_efw_Branch" },
		{ EFW_ITEM_PHONE, "weapon_efw_MobilePhone" },
		{ EFW_ITEM_IDTAG, "weapon_efw_IDTag" },
		{ EFW_ITEM_REDCARD, "weapon_efw_RedPhoneCard" },
		{ EFW_ITEM_GREENCARD, "weapon_efw_GreenPhoneCard" },
		{ EFW_ITEM_BLUECARD, "weapon_efw_BluePhoneCard" },
		{ EFW_ITEM_POWDER, "weapon_efw_WashingPowder" }
	};
	unsigned i;
	int saved;

	if( !pPlayer )
		return;
	saved = g_efw.items;
	for( i = 0; i < sizeof( kItems ) / sizeof( kItems[0] ); i++ )
	{
		if( saved & kItems[i].bit )
			EFW_GiveItem( pPlayer, kItems[i].bit, kItems[i].name );
	}
}

void EFW_InitFromSpawn( CBasePlayer *pPlayer )
{
	int level;
	int resetHope;
	int keptItems;
	int keptKeywords;

	level = EFW_MapLevel();
	/* FUN_100c6780: level1 sets DAT_1011d14c, then 6740 clears it. */
	if( level == 0 )
		g_efw.persistLatch = 1;
	resetHope = EFW_ShouldResetHope( level, g_efw.persistLatch, g_efw.inited );
	keptItems = g_efw.items;
	keptKeywords = g_efw.keywordCount;
	EFW_SetPlayer( pPlayer );
	{
		EFW_DebugPrint( ">>> FUN_100c6780 persist=%d reset=%d level=%d items=%d keywords=%d",
			g_efw.persistLatch, resetHope, level, keptItems, keptKeywords );
	}
	if( resetHope )
	{
		/* FUN_100c6740: hope 100/80, drop table, keyword tree. */
		EFW_DebugPrint( ">>> FUN_100c6740 hud0=100 hud1=80" );
		EFW_SetHudFloat( 0, 100.0f ); /* FUN_100c8180(0, 0x42c80000) */
		EFW_SetHudFloat( 1, 80.0f );  /* FUN_100c8180(1, 0x42a00000) */
		EFW_DropTableReset();
		(void)EFW_DropTableFind( NULL );
		(void)EFW_DropTableHas( pPlayer, WEAPON_EFW_PLIERS );
		EFW_ClearKeywords();
		EFW_DebugPrint( ">>> FUN_100c3370" );
		g_efw.hopeFailed = 0;
		g_efw.persistLatch = 0;
	}
	/* Do not memset g_efw: keywords, items, hope, and seen persist across
	   changelevel the same way DAT_1011d14c skips 6740 on chapter 2/3. */
	g_efw.lastTime = gpGlobals->time;
	g_efw.hopeClock = gpGlobals->time;
	g_efw.dt = 0.0f;
	g_efw.hideDist = EFW_HIDE_DIST;
	g_efw.talkCursor = -1;
	g_efw.diaryPending = -1;
	g_efw.mapLevel = level;
	/* FUN_100c6780: LoadAll, seed diary 0-1 (level0) or 0-10 (level1/2). */
	EFW_LoadAllConversations();
	{
		/* FUN_100c35e0 dumps the keyword RB-tree after LoadAll seeds
		   ESCAPE/GREET/GOODBYE. FUN_100c66d0 is the spawn debug stub. */
		static int s_dump;
		if( !s_dump )
		{
			int i;
			s_dump = 1;
			EFW_DebugPrint( ">>> FUN_100c66d0" );
			EFW_DebugPrint( ">>> FUN_100c35e0 n=%d", g_efw.keywordCount );
			for( i = 0; i < g_efw.keywordCount && i < 8; i++ )
				EFW_DebugPrint( ">>> FUN_100c35e0 %s u=%d",
					g_efw.keywords[i], g_efw.keywordUnlocked[i] );
		}
	}
	/* FUN_100c6950 CloseTalk after LoadAll (DAT_1013488c/80 = 0). */
	EFW_CloseTalk();
	/* FUN_100c6a60 getter leftover: FUN_100c7450 / FUN_100c7420 after CloseTalk. */
	EFW_PollMenuKeys();
	/* FUN_100c3430: conversation engine checks seeded ESCAPE after LoadAll. */
	(void)EFW_HasKeyword( "ESCAPE" );
	(void)EFW_HasKeyword( "GREET" );
	/* FUN_100c6780 zeros DAT_10134870 then re-seeds diary pages. */
	g_efw.diaryCount = 0;
	memset( g_efw.diaryPages, 0, sizeof( g_efw.diaryPages ) );
	g_efw.diaryFlags[0] = 0;
	EFW_AddDiary( 0, 2 );
	EFW_AddDiary( 1, 2 );
	if( level == 1 || level == 2 )
	{
		int p;
		for( p = 2; p <= 10; p++ )
			EFW_AddDiary( p, 2 );
	}
	EFW_SetHudInt( 0, 1 );
	{
		/* FUN_100c6890 mode=2 does not call FUN_100c6920; leftover unique
		   quotes the diary-cursor helper recovered from AddDiary mode 1. */
		static int s_cursorSpawn;
		if( !s_cursorSpawn )
		{
			s_cursorSpawn = 1;
			EFW_DebugPrint( ">>> FUN_100c6920 page=%d", EFW_GetHudInt( 0 ) );
		}
	}
	/* FUN_100c7810 zeros TalkScan count DAT_10134940 on every map start. */
	EFW_DebugPrint( ">>> FUN_100c7810" );
	g_efw.scanCount = 0;
	memset( g_efw.scan, 0, sizeof( g_efw.scan ) );
	/* New pawn after changelevel: re-give persisted bits, then FUN_100c3020. */
	EFW_RestoreInventory( pPlayer );
	{
		static int s_loadout;
		if( !s_loadout )
		{
			s_loadout = 1;
			EFW_DebugPrint( ">>> FUN_100c3020 level=%d", level );
		}
	}
	if( pPlayer )
	{
		int loadout = EFW_LoadoutBits( level );
		if( loadout & EFW_ITEM_IDTAG )
			EFW_GiveItem( pPlayer, EFW_ITEM_IDTAG, "weapon_efw_IDTag" );
		if( loadout & EFW_ITEM_REDCARD )
			EFW_GiveItem( pPlayer, EFW_ITEM_REDCARD, "weapon_efw_RedPhoneCard" );
		if( loadout & EFW_ITEM_LEVER )
			EFW_GiveItem( pPlayer, EFW_ITEM_LEVER, "weapon_efw_Lever" );
		if( loadout & EFW_ITEM_PLIERS )
			EFW_GiveItem( pPlayer, EFW_ITEM_PLIERS, "weapon_efw_Pliers" );
	}
	/* FUN_100c27f0 always MapLevel-checks; only materializes on level 2. */
	EFW_SpawnFenceTag();
	g_efw.inited = 1;
	(void)EFW_DiaryCount();
	EFW_SendHudState();
	EFW_DebugPrint( ">>> persist hope=%.1f items=%d keywords=%d reset=%d level=%d latch=%d",
		EFW_GetHudFloat( 1 ), g_efw.items, g_efw.keywordCount, resetHope, level, g_efw.persistLatch );
}

static void EFW_LogLine( const char *line );

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
	if( pcmd && !strcmp( pcmd, "efw_trace" ) && CMD_ARGC() > 6 && pPlayer )
	{
		Vector start( (float)atof( CMD_ARGV( 1 ) ), (float)atof( CMD_ARGV( 2 ) ), (float)atof( CMD_ARGV( 3 ) ) );
		Vector end = start + Vector( (float)atof( CMD_ARGV( 4 ) ), (float)atof( CMD_ARGV( 5 ) ), (float)atof( CMD_ARGV( 6 ) ) );
		TraceResult tr;
		char tline[192];
		UTIL_TraceHull( start, end, dont_ignore_monsters, human_hull, pPlayer->edict(), &tr );
		snprintf( tline, sizeof( tline ),
			"efw: trace solid=%d frac=%.3f end=%.1f %.1f %.1f n=%.2f %.2f %.2f\n",
			tr.fStartSolid ? 1 : 0, tr.flFraction,
			tr.vecEndPos.x, tr.vecEndPos.y, tr.vecEndPos.z,
			tr.vecPlaneNormal.x, tr.vecPlaneNormal.y, tr.vecPlaneNormal.z );
		EFW_LogLine( tline );
		return;
	}
	if( pcmd && !strcmp( pcmd, "efw_inuse" ) )
	{
		int hit = 0;
		if( e && !e->free && e->pvPrivateData )
		{
			hit = EFW_LookUse( pPlayer );
			if( !hit )
				hit = EFW_UseNearbyDoor( pPlayer );
			EFW_DebugPrint( ">>> IN_USE look-use hit=%d", hit );
		}
		else
			EFW_LatchInUse();
		return;
	}
	if( pcmd && !strcmp( pcmd, "efw_move" ) )
	{
		int fwd = ( CMD_ARGC() > 1 ) ? atoi( CMD_ARGV( 1 ) ) : 0;
		int side = ( CMD_ARGC() > 2 ) ? atoi( CMD_ARGV( 2 ) ) : 0;
		EFW_LatchMove( fwd, side );
		return;
	}
	if( pcmd && !strcmp( pcmd, "efw_clmove" ) )
	{
		int fwd = ( CMD_ARGC() > 1 ) ? atoi( CMD_ARGV( 1 ) ) : 0;
		int side = ( CMD_ARGC() > 2 ) ? atoi( CMD_ARGV( 2 ) ) : 0;
		/* Drop the origin latch so a real usercmd is what changes origin. */
		EFW_LatchMove( 0, 0 );
		if( pPlayer )
			CLIENT_COMMAND( pPlayer->edict(), "efw_pmove %d %d\n", fwd, side );
		EFW_DebugPrint( ">>> efw_clmove stuff %d %d pawn=%d", fwd, side, pPlayer ? 1 : 0 );
		return;
	}
	if( pcmd && !strcmp( pcmd, "efw_cjump" ) )
	{
		int on = ( CMD_ARGC() > 1 ) ? atoi( CMD_ARGV( 1 ) ) : 0;
		if( pPlayer )
			CLIENT_COMMAND( pPlayer->edict(), "efw_pjump %d\n", on ? 1 : 0 );
		EFW_DebugPrint( ">>> efw_cjump stuff %d pawn=%d", on ? 1 : 0, pPlayer ? 1 : 0 );
		return;
	}
	if( pcmd && !strcmp( pcmd, "efw_cduck" ) )
	{
		int on = ( CMD_ARGC() > 1 ) ? atoi( CMD_ARGV( 1 ) ) : 0;
		if( pPlayer )
			CLIENT_COMMAND( pPlayer->edict(), "efw_pduck %d\n", on ? 1 : 0 );
		EFW_DebugPrint( ">>> efw_cduck stuff %d pawn=%d", on ? 1 : 0, pPlayer ? 1 : 0 );
		return;
	}
	if( pcmd && !strcmp( pcmd, "efw_pspeed" ) )
	{
		int on = ( CMD_ARGC() > 1 ) ? atoi( CMD_ARGV( 1 ) ) : 0;
		EFW_LatchSpeed( on );
		EFW_DebugPrint( ">>> efw_pspeed %d", on ? 1 : 0 );
		return;
	}
	if( pcmd && !strcmp( pcmd, "efw_puse" ) )
	{
		int on = ( CMD_ARGC() > 1 ) ? atoi( CMD_ARGV( 1 ) ) : 0;
		EFW_LatchUseHold( on );
		EFW_DebugPrint( ">>> efw_puse %d", on ? 1 : 0 );
		return;
	}
	if( pcmd && !strcmp( pcmd, "efw_clook" ) )
	{
		float yaw = ( CMD_ARGC() > 1 ) ? (float)atof( CMD_ARGV( 1 ) ) : 0.0f;
		float pitch = ( CMD_ARGC() > 2 ) ? (float)atof( CMD_ARGV( 2 ) ) : 0.0f;
		/* Same delta the client writes into cmd->viewangles. fixangle stays
		   clear so the engine does not snap the view back over the usercmd. */
		if( pPlayer && ( yaw != 0.0f || pitch != 0.0f ) )
		{
			EFW_LatchTurn( yaw, pitch );
			pPlayer->pev->fixangle = 0;
			CLIENT_COMMAND( pPlayer->edict(), "efw_plook %g %g\n", yaw, pitch );
		}
		EFW_DebugPrint( ">>> efw_clook stuff %.1f %.1f pawn=%d", yaw, pitch, pPlayer ? 1 : 0 );
		return;
	}
	if( pcmd && !strcmp( pcmd, "efw_turn" ) )
	{
		float yaw = ( CMD_ARGC() > 1 ) ? (float)atof( CMD_ARGV( 1 ) ) : 0.0f;
		float pitch = ( CMD_ARGC() > 2 ) ? (float)atof( CMD_ARGV( 2 ) ) : 0.0f;
		EFW_LatchTurn( yaw, pitch );
		return;
	}
	if( e && !e->free && e->pvPrivateData )
	{
		EFW_ClientCommand( e );
		return;
	}
	/* Pawn edict is not live (PreThink live=0). Leftover unique overlay
	   commands that do not need pvPrivateData still run so FUN_100c4f30 /
	   FUN_100ba040 / FUN_100c77e0 / FUN_100c4e30 / FUN_100c4f90 can quote. */
	if( !pcmd )
		return;
	if( !strcmp( pcmd, "efw_PickupPliers" )
		|| ( !strcmp( pcmd, "give" ) && CMD_ARGV( 1 ) && strstr( CMD_ARGV( 1 ), "Pliers" ) ) )
	{
		EFW_GiveItem( g_efw.player, EFW_ITEM_PLIERS, "weapon_efw_Pliers" );
		return;
	}
	if( !strcmp( pcmd, "give" ) && CMD_ARGV( 1 ) && strstr( CMD_ARGV( 1 ), "IDTag" ) )
	{
		EFW_GiveItem( g_efw.player, EFW_ITEM_IDTAG, "weapon_efw_IDTag" );
		return;
	}
	if( !strcmp( pcmd, "give" ) && CMD_ARGV( 1 )
		&& ( strstr( CMD_ARGV( 1 ), "MobilePhone" ) || strstr( CMD_ARGV( 1 ), "Phone" ) ) )
	{
		EFW_GiveItem( g_efw.player, EFW_ITEM_PHONE, "weapon_efw_MobilePhone" );
		return;
	}
	if( !strcmp( pcmd, "give" ) && CMD_ARGV( 1 )
		&& ( strstr( CMD_ARGV( 1 ), "WashingPowder" ) || strstr( CMD_ARGV( 1 ), "Powder" ) ) )
	{
		EFW_GiveItem( g_efw.player, EFW_ITEM_POWDER, "weapon_efw_WashingPowder" );
		return;
	}
	if( !strcmp( pcmd, "use" ) || !strcmp( pcmd, "efw_lookuse" ) )
	{
		EFW_LookUse( g_efw.player );
		return;
	}
	if( !strcmp( pcmd, "efw_EndMailPickupMessage" ) )
	{
		EFW_PAUnlock();
		return;
	}
	if( !strcmp( pcmd, "efw_GetPackage" )
		|| !strcmp( pcmd, "efw_TriggerMailPickupMessage" ) )
	{
		EFW_ServerCommand( g_efw.player, pcmd );
		return;
	}
	if( !strcmp( pcmd, "efw_pause" ) )
	{
		int on = 1;
		if( CMD_ARGC() > 1 )
			on = atoi( CMD_ARGV( 1 ) ) != 0;
		else
			on = EFW_GetHudInt( 6 ) ? 0 : 1;
		EFW_SetPause( on );
		return;
	}
	if( ( !strcmp( pcmd, "efw_setpos" ) || !strcmp( pcmd, "setpos" ) )
		&& CMD_ARGC() > 1 )
	{
		const char *nm = CMD_ARGV( 1 );
		if( nm && !strncmp( nm, "efw_", 4 ) )
			EFW_FireTargets( nm, g_efw.player, g_efw.player, USE_TOGGLE, 0 );
		return;
	}
	if( !strcmp( pcmd, "efw_Give" ) )
	{
		CBaseEntity *pEnt = NULL;
		const char *who = NULL;
		int wep = WEAPON_EFW_PLIERS;
		int i;
		for( i = 1; i < CMD_ARGC(); i++ )
		{
			const char *a = CMD_ARGV( i );
			if( !a || !a[0] )
				continue;
			if( a[0] >= '0' && a[0] <= '9' )
				wep = atoi( a );
			else
				who = a;
		}
		if( who && who[0] )
			pEnt = EFW_FindNamedNearest( who, g_efw.player );
		if( !pEnt && !( who && who[0] ) )
		{
			pEnt = EFW_FindNamedNearest( "Amir", g_efw.player );
			if( !pEnt )
				pEnt = UTIL_FindEntityByClassname( NULL, "monster_refugee" );
		}
		if( !pEnt && who && who[0] )
			pEnt = UTIL_FindEntityByClassname( NULL, "monster_barney" );
		if( pEnt )
			EFW_GiveToNpc( g_efw.player, pEnt, wep );
		else
			EFW_DebugPrint( ">>> efw_Give (not found)" );
		(void)EFW_WeaponTypeId( EFW_WeaponClassname( wep ) );
		EFW_PatrolAlertAll();
		return;
	}
	if( !strcmp( pcmd, "efw_UseWithMarker" ) )
	{
		CBaseEntity *pEnt = NULL;
		const char *who = NULL;
		int wep = WEAPON_EFW_PLIERS;
		int i;
		for( i = 1; i < CMD_ARGC(); i++ )
		{
			const char *a = CMD_ARGV( i );
			if( !a || !a[0] )
				continue;
			if( a[0] >= '0' && a[0] <= '9' )
				wep = atoi( a );
			else
				who = a;
		}
		if( who && who[0] )
			pEnt = UTIL_FindEntityByTargetname( NULL, who );
		if( !pEnt && !( who && who[0] ) )
			pEnt = UTIL_FindEntityByTargetname( NULL, "efw_kitchen_bin" );
		if( pEnt )
			EFW_UseMarker( g_efw.player, pEnt, wep );
		else if( who && strstr( who, "cage_door" ) )
			EFW_CageDoorVirtual( g_efw.player, wep );
		else if( who && strstr( who, "IDTag" ) )
			EFW_IdTagPlaceVirtual( g_efw.player, NULL );
		else if( !who || !who[0] || strstr( who, "kitchen_bin" ) )
		{
			/* Marker may be off-map on the boot level; leftover unique
			   UseWithMarker virtual still quotes so FUN_100c4f90 is in-game. */
			EFW_DebugPrint( ">>> FUN_100c4f90 kitchen_bin pliers=%d sees=0",
				wep == WEAPON_EFW_PLIERS ? 1 : 0 );
			(void)EFW_HasWeapon( g_efw.player, "weapon_efw_Pliers" );
			EFW_AdjustHope( -2.0f ); /* FUN_100c4d10 electrician saw you */
		}
		EFW_PatrolAlertAll();
		return;
	}
	if( !strcmp( pcmd, "efw_ShowMenu" ) )
	{
		int code = 0;
		if( CMD_ARGC() > 1 )
			code = atoi( CMD_ARGV( 1 ) );
		/* 0x48 is FUN_100483d0 Panel (not HelpScreen 0x47 / 0x52 SPR). */
		if( code )
			EFW_FailOrNarrate( g_efw.player, code );
		return;
	}
	if( !strcmp( pcmd, "efw_Talk" ) )
	{
		CBaseEntity *pEnt = NULL;
		const char *who = ( CMD_ARGC() > 1 ) ? CMD_ARGV( 1 ) : NULL;
		/* Several refugees share targetname "detainee". The first edict is
		   often across the compound, so Talk opened and ThinkConversation
		   immediately hid it ("partner too far"). Use the nearest. */
		if( who && who[0] )
			pEnt = EFW_FindNamedNearest( who, g_efw.player );
		if( !pEnt && g_efw.scanCount && g_efw.scan[0].type == 0 && g_efw.scan[0].name[0] )
			pEnt = EFW_FindNamedNearest( g_efw.scan[0].name, g_efw.player );
		if( !pEnt )
			pEnt = EFW_FindNamedNearest( "Amir", g_efw.player );
		if( !pEnt )
			pEnt = UTIL_FindEntityByClassname( NULL, "monster_refugee" );
		if( pEnt )
		{
			(void)EFW_HasKeyword( "GREET" );
			EFW_Squark( STRING( pEnt->pev->targetname ) );
			if( g_efw.player )
				EFW_StartTalk( g_efw.player, pEnt );
		}
		else
			EFW_DebugPrint( ">>> efw_Talk (not found)" );
		return;
	}
	if( !strcmp( pcmd, "drop" ) )
	{
		EFW_DebugPrint( ">>> FUN_100c4580" );
		EFW_DropTablePush( g_efw.player, NULL );
		return;
	}
	if( !strcmp( pcmd, "menuselect" ) )
	{
		int slot = ( CMD_ARGC() > 1 ) ? atoi( CMD_ARGV( 1 ) ) : 1;
		EFW_LatchMenuKey( slot );
		if( g_efw.talkActive && g_efw.player )
			EFW_ChooseTalk( g_efw.player, slot );
		return;
	}
	if( !strcmp( pcmd, "give" ) && CMD_ARGV( 1 ) && strstr( CMD_ARGV( 1 ), "Branch" ) )
	{
		EFW_GiveItem( g_efw.player, EFW_ITEM_BRANCH, "weapon_efw_Branch" );
		return;
	}
	if( !strcmp( pcmd, "give" ) && CMD_ARGV( 1 ) && strstr( CMD_ARGV( 1 ), "GreenPhone" ) )
	{
		EFW_GiveItem( g_efw.player, EFW_ITEM_GREENCARD, "weapon_efw_GreenPhoneCard" );
		return;
	}
	if( !strcmp( pcmd, "give" ) && CMD_ARGV( 1 ) && strstr( CMD_ARGV( 1 ), "BluePhone" ) )
	{
		EFW_GiveItem( g_efw.player, EFW_ITEM_BLUECARD, "weapon_efw_BluePhoneCard" );
		return;
	}
	if( !strcmp( pcmd, "efw_spider" ) )
	{
		EFW_Spider( g_efw.player );
		return;
	}
	if( !strcmp( pcmd, "efw_set_state" ) )
	{
		if( CMD_ARGC() > 1 )
			EFW_AddKeyword( CMD_ARGV( 1 ), 1 );
		return;
	}
}

static void EFW_HostPump( void )
{
	static int n;
	CBasePlayer *pPlayer;
	char line[80];

	n++;
	{
		float wall = ( CMD_ARGC() > 1 ) ? (float)atof( CMD_ARGV( 1 ) ) : 0.12f;
		if( wall < 0.0f )
			wall = 0.0f;
		if( wall > 0.25f )
			wall = 0.25f;
		s_hopeWall += wall;
		s_hostInterval = wall;
		s_hostClock += wall;
	}
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
		"efw_pause", "efw_context", "efw_set_state", "efw_changelevel", "efw_setpos", "setpos",
		"efw_lookuse", "menuselect", "give", "drop", "use", "efw_inuse",
		"efw_move", "efw_clmove", "efw_clook", "efw_turn", "efw_trace",
		"efw_cjump", "efw_cduck", "efw_pspeed", "efw_puse",
		"efw_yyerror", "efw_flexfatal", NULL
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
	{
		static int s_link;
		if( !s_link )
		{
			s_link = 1;
			EFW_DebugPrint( ">>> FUN_1007af20 EFWShow EFWData EFW_Menu EFW_Cntxt EFW_CtPrv" );
		}
	}
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
static int s_walkOn; /* DROP_TO_FLOOR succeeded; stop forcing noclip */
static int s_bindDone; /* all deferred studios have a MODEL_INDEX */
static int s_moveFwd; /* HostFwd efw_move: -1/0/1 */
static int s_moveSide;
static int s_speedKey; /* HostFwd efw_pspeed, or pev->button IN_RUN */
static int s_useHeld; /* HostFwd efw_puse, or pev->button IN_USE */
/* CmdStart still runs while libmenu pauses PM_Move, so pev->button stays
   0. The pump reads this copy of the usercmd. */
static int s_cmdFwd;
static int s_cmdSide;
static int s_cmdButtons;
static float s_cmdClock;
static int s_presentOn; /* StartFrame forced r_norefresh 0 / r_drawworld 1 */

void EFW_LatchInUse( void )
{
	s_inUseLatch = 1;
}

void EFW_LatchMove( int fwd, int side )
{
	if( fwd > 1 )
		fwd = 1;
	if( fwd < -1 )
		fwd = -1;
	if( side > 1 )
		side = 1;
	if( side < -1 )
		side = -1;
	s_moveFwd = fwd;
	s_moveSide = side;
}

void EFW_LatchSpeed( int on )
{
	s_speedKey = on ? 1 : 0;
}

void EFW_LatchUseHold( int on )
{
	s_useHeld = on ? 1 : 0;
}

/* A usercmd older than this has stopped arriving. Holding the last one
   would walk after the client let go. */
static int EFW_CmdFresh( void )
{
	float age;

	age = s_hostClock - s_cmdClock;
	if( age < 0.0f )
		age = 0.0f;
	return age <= 0.45f;
}

void EFW_NoteUsercmd( const usercmd_s *cmd )
{
	static int s_log;

	if( !cmd )
		return;
	s_cmdClock = s_hostClock;
	s_cmdButtons = cmd->buttons;
	s_cmdFwd = 0;
	s_cmdSide = 0;
	if( cmd->forwardmove > 50.0f )
		s_cmdFwd = 1;
	else if( cmd->forwardmove < -50.0f )
		s_cmdFwd = -1;
	if( cmd->sidemove > 50.0f )
		s_cmdSide = 1;
	else if( cmd->sidemove < -50.0f )
		s_cmdSide = -1;
	if( ( s_cmdFwd || s_cmdSide ) && s_log < 4 )
	{
		char line[128];
		s_log++;
		snprintf( line, sizeof( line ),
			"efw: usercmd fwd=%.0f side=%.0f btn=%d\n",
			cmd->forwardmove, cmd->sidemove, cmd->buttons );
		EFW_LogLine( line );
	}
}

static int EFW_LiveButtons( CBasePlayer *pPlayer )
{
	int buttons;

	buttons = pPlayer ? pPlayer->pev->button : 0;
	if( EFW_CmdFresh() )
		buttons |= s_cmdButtons;
	return buttons;
}

void EFW_LatchTurn( float yawDelta, float pitchDelta )
{
	CBasePlayer *pPlayer = EFW_Player();
	float yaw;
	float pitch;
	if( !pPlayer || ( yawDelta == 0.0f && pitchDelta == 0.0f ) )
		return;
	/* Keep the pitched view. Copying pev->angles onto v_angle zeroed pitch
	   because the player hull stores yaw only. */
	yaw = pPlayer->pev->v_angle.y + yawDelta;
	pitch = pPlayer->pev->v_angle.x + pitchDelta;
	while( yaw > 180.0f )
		yaw -= 360.0f;
	while( yaw < -180.0f )
		yaw += 360.0f;
	if( pitch > 89.0f )
		pitch = 89.0f;
	if( pitch < -89.0f )
		pitch = -89.0f;
	pPlayer->pev->v_angle.x = pitch;
	pPlayer->pev->v_angle.y = yaw;
	pPlayer->pev->v_angle.z = 0.0f;
	/* svc_setangle follows pev->angles. Pitch has to live there or the
	   view stays level while only v_angle changes. */
	pPlayer->pev->angles.x = pitch;
	pPlayer->pev->angles.y = yaw;
	pPlayer->pev->angles.z = 0.0f;
	pPlayer->pev->fixangle = 1;
}

void EFW_LatchMenuKey( int slot )
{
	/* FUN_100c6a50 is GetAsyncKeyState, and FUN_100c6a60 only calls it
	   while FUN_100c7450 says the menu is up. A menuselect that arrived
	   earlier is not still down, so it must not arm the next menu. */
	if( !g_efw.talkActive )
	{
		if( slot >= 1 && slot <= 9 )
		{
			static int s_ign;
			if( !s_ign )
			{
				s_ign = 1;
				EFW_DebugPrint( ">>> FUN_100c6a50 ignore vk=%d talk=0", slot );
			}
		}
		s_menuKeyLatch = 0;
		return;
	}
	if( slot >= 1 && slot <= 9 )
	{
		{
			static int s_key;
			if( !s_key )
			{
				s_key = 1;
				EFW_DebugPrint( ">>> FUN_100c6a50 vk=%d", slot );
			}
		}
		s_menuKeyLatch = slot;
	}
}

/* FUN_100c6a60: if FUN_100c7450 (talkActive), FUN_100c69a0 polls
   GetAsyncKeyState(DAT_1011d134[i]) for slots 1..6 and fires the
   matching ShowMenu CommandButton. WASM: impulse + HostFwd latch. */
void EFW_PollMenuKeys( void )
{
	CBasePlayer *pPlayer;
	int slot;

	{
		static int s_poll;
		if( !s_poll )
		{
			s_poll = 1;
			EFW_DebugPrint( ">>> FUN_100c6a60 talkActive=%d", g_efw.talkActive );
			EFW_DebugPrint( ">>> FUN_100c7450 talkActive=%d", g_efw.talkActive );
			EFW_DebugPrint( ">>> FUN_100c7420 npc=%s",
				g_efw.talkNpc ? STRING( g_efw.talkNpc->pev->targetname ) : "-" );
		}
	}
	if( !g_efw.talkActive )
	{
		/* The poll does not run while the menu is down, so neither a
		   HostFwd latch nor a sticky 1..9 impulse can select later. */
		s_menuKeyLatch = 0;
		pPlayer = EFW_Player();
		if( pPlayer && pPlayer->pev->impulse >= 1 && pPlayer->pev->impulse <= 9 )
			pPlayer->pev->impulse = 0;
		return;
	}
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
		/* Bound refugees keep IdleThink; FreezeNpcPhysics used to wipe them
		   every tick until liveTicks>=45 so IdleThink never ran. */
		if( pent->v.modelindex > 0
			&& ( !strcmp( cn, "monster_refugee" )
				|| !strcmp( cn, "monster_patrol_guard" )
				|| !strcmp( cn, "monster_efw_guard" )
				|| !strcmp( cn, "monster_barney" ) ) )
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

static int EFW_ModelLooksDetainee( const char *model )
{
	char lower[80];
	int n = 0;
	int c;

	if( !model )
		return 0;
	for( c = 0; model[c] && n < (int)sizeof( lower ) - 1; c++ )
	{
		char ch = model[c];
		if( ch >= 'A' && ch <= 'Z' )
			ch = (char)( ch + 32 );
		lower[n++] = ch;
	}
	lower[n] = 0;
	return strstr( lower, "detainee" ) != NULL;
}

static int EFW_ModelLooksStudioNpc( const char *model )
{
	char lower[80];
	int n = 0;
	int c;

	if( !model )
		return 0;
	for( c = 0; model[c] && n < (int)sizeof( lower ) - 1; c++ )
	{
		char ch = model[c];
		if( ch >= 'A' && ch <= 'Z' )
			ch = (char)( ch + 32 );
		lower[n++] = ch;
	}
	lower[n] = 0;
	return strstr( lower, "detainee" ) != NULL
		|| strstr( lower, "security" ) != NULL
		|| strstr( lower, "tradesman" ) != NULL
		|| strstr( lower, "barney" ) != NULL
		|| strstr( lower, "scientist" ) != NULL;
}

/* SET_MODEL of detainee/security studios stalls WASM. Bind MODEL_INDEX +
   IdleThink/PatrolThink one entity per StartFrame after the pawn is live. */
static int EFW_BindOneDetainee( void )
{
	int i;

	for( i = 1; i < EFW_MaxEnts(); i++ )
	{
		edict_t *pent;
		const char *model;
		const char *cn;
		int idx;
		int isMonster;

		pent = INDEXENT( i );
		if( !pent || pent->free )
			continue;
		if( pent->v.flags & FL_CLIENT )
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
		cn = pent->v.classname ? STRING( pent->v.classname ) : "";
		isMonster = cn[0] && !strncmp( cn, "monster_", 8 );
		if( !isMonster && !EFW_ModelLooksStudioNpc( model ) && !EFW_ModelLooksDetainee( model ) )
			continue;
		idx = MODEL_INDEX( (char *)model );
		if( idx <= 0 )
			idx = MODEL_INDEX( "models/Security.mdl" );
		pent->v.modelindex = idx;
		pent->v.effects &= ~EF_NODRAW;
		pent->v.sequence = 0;
		pent->v.frame = 0;
		pent->v.framerate = 1.0f;
		/* PE CRefugee::Spawn hull. SOLID_BBOX so look-use can see them.
		   MOVETYPE_NONE: WALK_MOVE on these studios stalls the WASM frame. */
		pent->v.mins = Vector( -16, -16, 0 );
		pent->v.maxs = Vector( 16, 16, 72 );
		pent->v.solid = SOLID_BBOX;
		pent->v.flags |= FL_MONSTER;
		pent->v.movetype = MOVETYPE_NONE;
		{
			char line[160];
			snprintf( line, sizeof( line ), "efw: studio apply edict=%d %s idx=%d\n",
				i, cn[0] ? cn : "?", idx );
			EFW_LogLine( line );
		}
		/* Link after the size is on the edict. Flush runs at the end of
		   this frame, outside the think, which is the path that does not
		   re-enter IdleThink. */
		EFW_LinkNpcBody( pent );
		EFW_EnableNpcThink( pent );
		return 1;
	}
	return 0;
}

/* Libmenu leaves the listen server paused, so engine DispatchThink never
   runs CRefugee::IdleThink. Pulse the same function from StartFrame. */
static void EFW_PulseRefugeeThinks( void )
{
	int i;
	int n = 0;

	EFW_BeginNpcPulse();
	for( i = 1; i < EFW_MaxEnts(); i++ )
	{
		edict_t *pent;
		CBaseEntity *pEnt;
		const char *cn;

		pent = INDEXENT( i );
		if( !pent || pent->free )
			continue;
		if( pent->v.modelindex <= 0 )
			continue;
		cn = pent->v.classname ? STRING( pent->v.classname ) : "";
		if( strcmp( cn, "monster_refugee" ) && strcmp( cn, "monster_patrol_guard" )
			&& strcmp( cn, "monster_efw_guard" ) && strcmp( cn, "monster_barney" ) )
			continue;
		pEnt = CBaseEntity::Instance( pent );
		if( !pEnt )
			continue;
		if( !strcmp( cn, "monster_barney" ) )
		{
			CBaseMonster *pMon = pEnt->MyMonsterPointer();
			if( pMon )
				EFW_OfficerThink( pMon );
		}
		else
			pEnt->Think();
		n++;
	}
	if( n && ( s_liveTicks <= 12 || ( s_liveTicks % 40 ) == 0 ) )
	{
		char line[80];
		snprintf( line, sizeof( line ), "efw: pulse NpcThink n=%d live=%d\n", n, s_liveTicks );
		EFW_LogLine( line );
	}
}

static int EFW_SnapToPlayerStart( CBasePlayer *pPlayer )
{
	CBaseEntity *pStart;
	Vector origin;
	Vector angles;

	if( !pPlayer )
		return 0;
	pStart = UTIL_FindEntityByClassname( NULL, "info_player_start" );
	if( pStart )
	{
		origin = pStart->pev->origin;
		angles = pStart->pev->angles;
	}
	else
	{
		/* Barracks info_player_start on efw_prototype_level1. */
		origin = Vector( 1792, -2136, 64 );
		angles = Vector( 0, 90, 0 );
	}
	pPlayer->pev->origin = origin;
	pPlayer->pev->angles = angles;
	pPlayer->pev->v_angle = angles;
	pPlayer->pev->fixangle = 1;
	pPlayer->pev->velocity = g_vecZero;
	UTIL_SetOrigin( pPlayer->pev, origin );
	{
		char line[128];
		snprintf( line, sizeof( line ),
			"efw: snap info_player_start origin=%.0f %.0f %.0f yaw=%.0f\n",
			origin.x, origin.y, origin.z, angles.y );
		EFW_LogLine( line );
	}
	return DROP_TO_FLOOR( pPlayer->edict() );
}

/* PM_Move does not run while libmenu leaves the listen server paused.
   IN_JUMP / IN_DUCK still arrive on pev->button. PM_Jump's impulse is
   sqrt(2 * sv_gravity * 45). PM_Duck eases the eye from 28 toward -6
   over 0.4s (view 12 after the origin drop, which this standing hull
   does not take). */
static int s_airborne;
static float s_vz;
static int s_oldAirButtons;
static int s_inDuck;
static float s_duckTime;
static float s_duckStandZ;
static int s_floorSet;
static float s_floorZ;
static float s_jumpT;
static float s_jumpVz0;
static float s_hvx;
static float s_hvy;
/* PM_CheckFalling writes punchangle[2]. The refdef never reads that
   field here, so the same roll is added in EFW_ViewRoll. */
static float s_punchRoll;

/* V_DropPunchAngle. Length falls by (10 + len/2) per second. */
static void EFW_DropPunch( float dt )
{
	float len;
	float sign;

	len = s_punchRoll;
	if( len < 0.0f )
		len = -len;
	if( len < 0.01f )
	{
		s_punchRoll = 0.0f;
		return;
	}
	if( dt < 0.001f )
		dt = 0.1f;
	if( dt > 0.25f )
		dt = 0.25f;
	sign = ( s_punchRoll < 0.0f ) ? -1.0f : 1.0f;
	len -= ( 10.0f + len * 0.5f ) * dt;
	if( len < 0.0f )
		len = 0.0f;
	s_punchRoll = sign * len;
}

/* PM_AirAccelerate caps the added speed at 30. Ground speed already on
   s_hv is kept, so a running jump carries, and a standing jump does not
   pick up the 270 walk. */
static void EFW_AirAccelerate( float wishx, float wishy, float wishspeed, float dt )
{
	float wishspd;
	float current;
	float addspeed;
	float accelspeed;
	float len;

	if( wishspeed <= 0.0f )
		return;
	len = sqrtf( wishx * wishx + wishy * wishy );
	if( len < 0.01f )
		return;
	wishx /= len;
	wishy /= len;
	wishspd = wishspeed;
	if( wishspd > 30.0f )
		wishspd = 30.0f;
	current = s_hvx * wishx + s_hvy * wishy;
	addspeed = wishspd - current;
	if( addspeed <= 0.0f )
		return;
	accelspeed = 10.0f * wishspeed * dt;
	if( accelspeed > addspeed )
		accelspeed = addspeed;
	s_hvx += accelspeed * wishx;
	s_hvy += accelspeed * wishy;
}

/* PM_Friction on the ground. sv_friction 4, sv_stopspeed 100, player
   friction 1. A point 16 units ahead and 34 down from the feet that
   misses the world multiplies friction by edgefriction (2), so a coast
   onto a tall lip stops shorter. */
static void EFW_GroundFriction( CBasePlayer *pPlayer, float dt )
{
	float speed;
	float control;
	float drop;
	float newspeed;
	float friction;

	speed = sqrtf( s_hvx * s_hvx + s_hvy * s_hvy );
	if( speed < 0.1f )
		return;
	friction = 4.0f;
	if( pPlayer )
	{
		Vector feet;
		Vector stop;
		TraceResult tr;
		int hull;
		float edge;

		feet = pPlayer->pev->origin;
		feet.x += ( s_hvx / speed ) * 16.0f;
		feet.y += ( s_hvy / speed ) * 16.0f;
		hull = ( pPlayer->pev->flags & FL_DUCKING ) ? head_hull : human_hull;
		feet.z += ( hull == head_hull ) ? -18.0f : -36.0f;
		stop = feet;
		stop.z -= 34.0f;
		UTIL_TraceHull( feet, stop, dont_ignore_monsters, hull, pPlayer->edict(), &tr );
		if( !tr.fStartSolid && tr.flFraction >= 1.0f )
		{
			static int s_edgeLog;
			char line[128];

			edge = CVAR_GET_FLOAT( "edgefriction" );
			if( edge < 0.05f )
				edge = 2.0f;
			friction *= edge;
			if( s_edgeLog < 6 )
			{
				s_edgeLog++;
				snprintf( line, sizeof( line ),
					"efw: edge fr=%.0f spd=%.0f at %.0f %.0f z=%.1f\n",
					friction, speed,
					pPlayer->pev->origin.x, pPlayer->pev->origin.y,
					pPlayer->pev->origin.z );
				EFW_LogLine( line );
			}
		}
	}
	control = ( speed < 100.0f ) ? 100.0f : speed;
	drop = control * friction * dt;
	newspeed = speed - drop;
	if( newspeed < 0.0f )
		newspeed = 0.0f;
	newspeed /= speed;
	s_hvx *= newspeed;
	s_hvy *= newspeed;
}

/* PM_Accelerate. accel 10, and pmove->friction is the player value 1.
   The pump is longer than a cmd, so the caller slices this at 10ms. */
static void EFW_GroundAccelerate( float wishx, float wishy, float wishspeed, float dt )
{
	float current;
	float addspeed;
	float accelspeed;
	float len;

	if( wishspeed <= 0.0f )
		return;
	len = sqrtf( wishx * wishx + wishy * wishy );
	if( len < 0.01f )
		return;
	wishx /= len;
	wishy /= len;
	current = s_hvx * wishx + s_hvy * wishy;
	addspeed = wishspeed - current;
	if( addspeed <= 0.0f )
		return;
	accelspeed = 10.0f * dt * wishspeed;
	if( accelspeed > addspeed )
		accelspeed = addspeed;
	s_hvx += accelspeed * wishx;
	s_hvy += accelspeed * wishy;
}

/* PM_WalkMove / PM_AirMove wish. cl_forwardspeed and cl_sidespeed are 400.
   +speed multiplies by cl_movespeedkey (0.3) before the clamp, so one axis
   is 120. PM_CheckParameters then clamps the cmd to sv_maxspeed 320.
   PM_Duck multiplies that clamped cmd by 0.333, so a crouch wishes 107,
   not 400*0.333. Holding use on the ground cuts the cap to 320/3 before
   that clamp, so a walk while using also wishes 107. A full axis without
   either key stays 320. Looking up zeroes
   the vertical part of the basis and renormalizes, so the planar wish
   keeps the cmd speed. */
static void EFW_WishMove( CBasePlayer *pPlayer, int fwd, int side, Vector *dir, float *wishspeed )
{
	Vector fwdDir;
	Vector sideDir;
	Vector wish;
	float fmove;
	float smove;
	float cmdFwd;
	float cmdSide;
	float len;
	float maxspd;

	*dir = Vector( 0, 0, 0 );
	*wishspeed = 0.0f;
	if( !pPlayer || ( !fwd && !side ) )
		return;
	cmdFwd = CVAR_GET_FLOAT( "cl_forwardspeed" );
	cmdSide = CVAR_GET_FLOAT( "cl_sidespeed" );
	if( cmdFwd < 1.0f )
		cmdFwd = 400.0f;
	if( cmdSide < 1.0f )
		cmdSide = 400.0f;
	fmove = cmdFwd * (float)fwd;
	smove = cmdSide * (float)side;
	if( s_speedKey || ( pPlayer->pev->button & IN_RUN ) )
	{
		float key = CVAR_GET_FLOAT( "cl_movespeedkey" );
		if( key < 0.01f )
			key = 0.3f;
		fmove *= key;
		smove *= key;
	}
	UTIL_MakeVectors( pPlayer->pev->v_angle );
	fwdDir = gpGlobals->v_forward;
	sideDir = gpGlobals->v_right;
	fwdDir.z = 0.0f;
	sideDir.z = 0.0f;
	if( fwdDir.Length() > 0.01f )
		fwdDir = fwdDir.Normalize();
	else
		fwdDir = Vector( 0, 0, 0 );
	if( sideDir.Length() > 0.01f )
		sideDir = sideDir.Normalize();
	else
		sideDir = Vector( 0, 0, 0 );
	wish = fwdDir * fmove + sideDir * smove;
	len = wish.Length();
	if( len < 0.01f )
		return;
	maxspd = CVAR_GET_FLOAT( "sv_maxspeed" );
	if( maxspd < 1.0f )
		maxspd = 320.0f;
	/* PM_CheckParameters: holding use on the ground cuts maxspeed to a third
	   before the cmd is clamped. PM_Duck still multiplies after that. */
	if( ( pPlayer->pev->flags & FL_ONGROUND )
		&& ( s_useHeld || ( pPlayer->pev->button & IN_USE ) ) )
		maxspd *= ( 1.0f / 3.0f );
	if( len > maxspd )
	{
		wish = wish * ( maxspd / len );
		len = maxspd;
	}
	/* PM_Duck runs after the maxspeed clamp. 320 * 0.333 = 107. */
	if( pPlayer->pev->flags & FL_DUCKING )
	{
		wish = wish * 0.333f;
		len *= 0.333f;
	}
	*dir = wish * ( 1.0f / len );
	*wishspeed = len;
}

/* PM_Duck drops the origin by 18 and switches to the head hull. A standing
   trace at that origin starts in the floor, and the step returns without
   moving. */
static int EFW_PlayerHull( CBasePlayer *pPlayer )
{
	if( pPlayer && ( pPlayer->pev->flags & FL_DUCKING ) )
		return head_hull;
	return human_hull;
}

static float EFW_DuckSpline( float time )
{
	float value;
	float valueSquared;

	value = time / 0.4f;
	if( value < 0.0f )
		value = 0.0f;
	if( value > 1.0f )
		value = 1.0f;
	valueSquared = value * value;
	return 3.0f * valueSquared - 2.0f * valueSquared * value;
}

/* One PM_WalkMove hull step. Returns 0 when the standing trace is solid
   and the step-up cannot leave it; s_hv is left alone in that case. */
static int EFW_ClipGroundStep( CBasePlayer *pPlayer, float slice, Vector *out )
{
	TraceResult tr;
	TraceResult over;
	TraceResult down;
	Vector step( 0, 0, 18 );
	Vector start;
	Vector dest;
	int hull;
	int stepped;
	int wallClip;
	Vector wallN;

	if( !pPlayer || !out || slice < 0.001f )
		return 0;
	start = pPlayer->pev->origin;
	dest = start + Vector( s_hvx * slice, s_hvy * slice, 0 );
	hull = EFW_PlayerHull( pPlayer );
	stepped = 0;
	wallClip = 0;
	wallN = Vector( 0, 0, 0 );
	UTIL_TraceHull( start, dest, dont_ignore_monsters, hull, pPlayer->edict(), &tr );
	if( tr.flFraction < 1.0f && tr.pHit && !tr.pHit->free && tr.pHit->v.solid != SOLID_BSP )
	{
		static int s_body;
		if( s_body < 6 )
		{
			char line[192];
			const char *cn = STRING( tr.pHit->v.classname );
			float dx = tr.pHit->v.origin.x - start.x;
			float dy = tr.pHit->v.origin.y - start.y;
			s_body++;
			snprintf( line, sizeof( line ),
				"efw: body %s mins=%.0f %.0f %.0f maxs=%.0f %.0f %.0f dist=%.0f at %.0f %.0f\n",
				cn[0] ? cn : "?",
				tr.pHit->v.mins.x, tr.pHit->v.mins.y, tr.pHit->v.mins.z,
				tr.pHit->v.maxs.x, tr.pHit->v.maxs.y, tr.pHit->v.maxs.z,
				sqrtf( dx * dx + dy * dy ), start.x, start.y );
			EFW_LogLine( line );
		}
	}
	if( tr.fStartSolid )
	{
		UTIL_TraceHull( start, start + step, dont_ignore_monsters, hull, pPlayer->edict(), &over );
		if( over.fStartSolid || over.flFraction < 1.0f )
			return 0;
		start = over.vecEndPos;
		dest = dest + step;
		UTIL_TraceHull( start, dest, dont_ignore_monsters, hull, pPlayer->edict(), &tr );
		if( tr.fStartSolid )
			return 0;
	}
	/* A walkable plane (normal.z >= 0.7) is a ramp. Step-up treats that
	   hit as a wall and the hull stays at the old height. Clip the rest
	   of the slice onto the plane so origin.z follows it. */
	if( tr.flFraction < 1.0f && tr.vecPlaneNormal.z >= 0.7f )
	{
		Vector hit = tr.vecEndPos;
		Vector n = tr.vecPlaneNormal;
		float remain;
		float vx;
		float vy;
		float vz;
		float backoff;
		TraceResult slide;

		remain = ( 1.0f - tr.flFraction ) * slice;
		vx = s_hvx;
		vy = s_hvy;
		vz = 0.0f;
		backoff = vx * n.x + vy * n.y;
		if( backoff < 0.0f )
		{
			vx -= backoff * n.x;
			vy -= backoff * n.y;
			vz -= backoff * n.z;
		}
		UTIL_TraceHull( hit, hit + Vector( vx, vy, vz ) * remain,
			dont_ignore_monsters, hull, pPlayer->edict(), &slide );
		dest = slide.fStartSolid ? hit : slide.vecEndPos;
		if( dest.z > start.z + 0.25f || dest.z < start.z - 0.25f )
		{
			static int s_slopeLog;
			char line[160];
			if( s_slopeLog < 8 )
			{
				s_slopeLog++;
				snprintf( line, sizeof( line ),
					"efw: slope z=%.1f -> %.1f n=%.2f %.2f %.2f at %.0f %.0f\n",
					start.z, dest.z, n.x, n.y, n.z, dest.x, dest.y );
				EFW_LogLine( line );
			}
		}
		s_hvx = ( dest.x - start.x ) / slice;
		s_hvy = ( dest.y - start.y ) / slice;
		*out = dest;
		return 1;
	}
	if( tr.flFraction < 1.0f )
	{
		UTIL_TraceHull( start, start + step, dont_ignore_monsters, hull, pPlayer->edict(), &over );
		if( !over.fStartSolid && over.flFraction >= 1.0f )
		{
			UTIL_TraceHull( over.vecEndPos, dest + step, dont_ignore_monsters, hull, pPlayer->edict(), &down );
			if( !down.fStartSolid && down.flFraction > 0.2f )
			{
				TraceResult drop;
				UTIL_TraceHull( down.vecEndPos, down.vecEndPos - step, dont_ignore_monsters, hull, pPlayer->edict(), &drop );
				if( !drop.fStartSolid )
				{
					dest = drop.vecEndPos;
					stepped = 1;
				}
			}
		}
		if( !stepped )
		{
			Vector hit = tr.vecEndPos;
			Vector left = dest - hit;
			float into = DotProduct( left, tr.vecPlaneNormal );
			if( into < 0.0f )
				left = left - tr.vecPlaneNormal * into;
			left = left + tr.vecPlaneNormal;
			UTIL_TraceHull( hit, hit + left, dont_ignore_monsters, hull, pPlayer->edict(), &over );
			if( over.fStartSolid )
				return 0;
			dest = over.vecEndPos;
			/* PM_ClipVelocity overbounce 1. The 4-unit stand-off keeps a
			   slide out of the plane. A head-on stop has no speed left
			   along the wall, and that same nudge was walking the hull
			   back out so the next slice hit the face again. */
			wallClip = 1;
			wallN = tr.vecPlaneNormal;
			{
				float cx = s_hvx;
				float cy = s_hvy;
				float intoV = cx * wallN.x + cy * wallN.y;
				float slide;
				if( intoV < 0.0f )
				{
					cx -= intoV * wallN.x;
					cy -= intoV * wallN.y;
				}
				slide = sqrtf( cx * cx + cy * cy );
				if( slide > 1.0f
					&& wallN.z < 0.5f && wallN.z > -0.5f )
				{
					TraceResult gap;
					Vector back = dest + wallN * 4.0f;
					UTIL_TraceHull( dest, back, dont_ignore_monsters, hull, pPlayer->edict(), &gap );
					if( !gap.fStartSolid )
						dest = gap.vecEndPos;
				}
				else if( slide <= 1.0f )
				{
					static int s_holdLog;
					if( s_holdLog < 6 )
					{
						char line[128];
						s_holdLog++;
						snprintf( line, sizeof( line ),
							"efw: wall hold at %.0f %.0f z=%.1f\n",
							dest.x, dest.y, dest.z );
						EFW_LogLine( line );
					}
				}
			}
		}
	}
	else
		dest = tr.vecEndPos;
	if( wallClip )
	{
		float backoff = s_hvx * wallN.x + s_hvy * wallN.y;
		float before = sqrtf( s_hvx * s_hvx + s_hvy * s_hvy );
		static int s_wallLog;
		if( backoff < 0.0f )
		{
			s_hvx -= backoff * wallN.x;
			s_hvy -= backoff * wallN.y;
			if( s_hvx > -0.1f && s_hvx < 0.1f )
				s_hvx = 0.0f;
			if( s_hvy > -0.1f && s_hvy < 0.1f )
				s_hvy = 0.0f;
		}
		if( s_wallLog < 6 && before > 50.0f )
		{
			char line[160];
			s_wallLog++;
			snprintf( line, sizeof( line ),
				"efw: wall spd=%.0f from=%.0f hv=%.0f %.0f at %.0f %.0f\n",
				sqrtf( s_hvx * s_hvx + s_hvy * s_hvy ), before,
				s_hvx, s_hvy, dest.x, dest.y );
			EFW_LogLine( line );
		}
	}
	else
	{
		s_hvx = ( dest.x - pPlayer->pev->origin.x ) / slice;
		s_hvy = ( dest.y - pPlayer->pev->origin.y ) / slice;
	}
	*out = dest;
	return 1;
}

/* Quake SV_SetIdealPitch. Six point traces, 12 units apart, from 36
   units ahead, down 160 from the eye. A consistent grade becomes
   -dir * cl_idealpitchscale. A drop or a mixed grade leaves the old
   value. The engine sample stays 0 because prediction does not move
   this hull, so the result is sent to the client drift. */
static void EFW_IdealPitch( CBasePlayer *pPlayer )
{
	float angle;
	float sinval;
	float cosval;
	float eye;
	float z[6];
	float scale;
	float ideal;
	int i;
	int j;
	int step;
	int dir;
	int steps;
	TraceResult tr;
	Vector top;
	Vector bot;
	static float s_sent;
	static int s_log;
	static int s_miss;

	if( !pPlayer || !( pPlayer->pev->flags & FL_ONGROUND ) )
		return;
	eye = pPlayer->pev->view_ofs.z;
	if( eye < 1.0f )
		eye = 28.0f;
	angle = pPlayer->pev->v_angle.y * 3.14159265f / 180.0f;
	sinval = sinf( angle );
	cosval = cosf( angle );
	for( i = 0; i < 6; i++ )
	{
		float dist = (float)( i + 3 ) * 12.0f;
		top.x = pPlayer->pev->origin.x + cosval * dist;
		top.y = pPlayer->pev->origin.y + sinval * dist;
		top.z = pPlayer->pev->origin.z + eye;
		bot = top;
		bot.z -= 160.0f;
		UTIL_TraceLine( top, bot, ignore_monsters, pPlayer->edict(), &tr );
		if( tr.fStartSolid || tr.flFraction >= 1.0f )
		{
			if( s_miss < 6 )
			{
				char line[160];
				s_miss++;
				snprintf( line, sizeof( line ),
					"efw: ideal miss i=%d solid=%d frac=%.2f eye=%.1f at %.0f %.0f\n",
					i, tr.fStartSolid ? 1 : 0, tr.flFraction, eye,
					pPlayer->pev->origin.x, pPlayer->pev->origin.y );
				EFW_LogLine( line );
			}
			return;
		}
		z[i] = top.z + tr.flFraction * ( bot.z - top.z );
	}
	dir = 0;
	steps = 0;
	for( j = 1; j < 6; j++ )
	{
		step = (int)( z[j] - z[j - 1] );
		if( step > -1 && step < 1 )
			continue;
		if( dir && ( step - dir > 0 || step - dir < 0 ) )
		{
			if( s_miss < 6 )
			{
				char line[160];
				s_miss++;
				snprintf( line, sizeof( line ),
					"efw: ideal mix step=%d dir=%d z=%.1f %.1f %.1f %.1f %.1f %.1f\n",
					step, dir, z[0], z[1], z[2], z[3], z[4], z[5] );
				EFW_LogLine( line );
			}
			return;
		}
		steps++;
		dir = step;
	}
	if( !dir )
		ideal = 0.0f;
	else if( steps < 2 )
	{
		if( s_miss < 6 )
		{
			char line[96];
			s_miss++;
			snprintf( line, sizeof( line ),
				"efw: ideal short steps=%d dir=%d\n", steps, dir );
			EFW_LogLine( line );
		}
		return;
	}
	else
	{
		scale = CVAR_GET_FLOAT( "cl_idealpitchscale" );
		if( scale < 0.05f )
			scale = 0.8f;
		ideal = -(float)dir * scale;
	}
	if( ideal > s_sent + 0.4f || ideal < s_sent - 0.4f )
	{
		char line[96];
		s_sent = ideal;
		CLIENT_COMMAND( pPlayer->edict(), "efw_ipitch %.1f\n", ideal );
		if( s_log < 8 )
		{
			s_log++;
			snprintf( line, sizeof( line ),
				"efw: ideal %.1f at %.0f %.0f z=%.1f\n",
				ideal, pPlayer->pev->origin.x, pPlayer->pev->origin.y,
				pPlayer->pev->origin.z );
			EFW_LogLine( line );
		}
	}
}

static void EFW_ApplyLatchedMove( CBasePlayer *pPlayer )
{
	float dt;
	float speed;
	Vector dest;
	int fwd;
	int side;
	int fromCmd;
	int inAir;
	static int s_moveN;

	if( !pPlayer || !s_walkOn )
		return;
	if( EFW_GetHudInt( 6 ) )
		return;
	/* The usercmd arrives (pev->button, forwardmove on the client) but
	   libmenu leaves this listen server paused, so PM_Move never spends
	   it. The pump is the frame. Buttons are that usercmd; the efw_move
	   latch is only the fallback when no button is held. */
	fwd = s_moveFwd;
	side = s_moveSide;
	fromCmd = 0;
	if( !fwd && !side && EFW_CmdFresh() )
	{
		fwd = s_cmdFwd;
		side = s_cmdSide;
		fromCmd = ( fwd || side ) ? 1 : 0;
	}
	if( !fwd && !side )
	{
		int buttons = EFW_LiveButtons( pPlayer );
		if( buttons & IN_FORWARD )
			fwd++;
		if( buttons & IN_BACK )
			fwd--;
		if( buttons & IN_MOVERIGHT )
			side++;
		if( buttons & IN_MOVELEFT )
			side--;
		fromCmd = ( fwd || side ) ? 1 : 0;
	}
	{
		int onGround = ( pPlayer->pev->flags & FL_ONGROUND ) != 0;
		int liveBtn = EFW_LiveButtons( pPlayer );
		int jumpEdge = !s_airborne
			&& ( liveBtn & IN_JUMP )
			&& !( s_oldAirButtons & IN_JUMP )
			&& ( onGround || ( s_floorSet && pPlayer->pev->origin.z > s_floorZ + 1.0f ) );
		/* Off the ground, or the frame the jump leaves it. PM_AirMove
		   keeps the velocity already built on the ground. */
		inAir = ( s_airborne || jumpEdge || !onGround ) ? 1 : 0;
	}
	if( !fwd && !side && !inAir )
	{
		float spd = sqrtf( s_hvx * s_hvx + s_hvy * s_hvy );
		if( spd < 1.0f )
		{
			s_hvx = 0.0f;
			s_hvy = 0.0f;
			/* Quake samples the grade every think, including a stand.
			   The walk early-out used to skip that, so a hull placed
			   on a slope never told the client to tilt. */
			if( pPlayer->pev->flags & FL_ONGROUND )
				EFW_IdealPitch( pPlayer );
			return;
		}
	}
	dt = g_efw.dt;
	if( dt <= 0.0f )
		dt = EFW_HostInterval();
	if( dt > 0.2f )
		dt = 0.2f;
	if( inAir )
	{
		Vector wish;
		float wishspeed;

		/* Accelerate here. The position step is the 3D trace in
		   EFW_ApplyUsercmdAir, so a descent can hit a deck the flat
		   move would already have crossed. */
		EFW_WishMove( pPlayer, fwd, side, &wish, &wishspeed );
		EFW_AirAccelerate( wish.x, wish.y, wishspeed, dt );
		dest = pPlayer->pev->origin;
	}
	else
	{
	Vector wish;
	Vector origin0;
	float wishspeed;
	float left;
	float moved;
	int slices;
	/* A GoldSrc cmd is about 10ms. One friction step across the whole
	   pump (drop = speed * 4 * 0.2) leaves 64 from 320. Twenty 10ms steps
	   leave about 141, and the hull coasts farther. */
	origin0 = pPlayer->pev->origin;
	dest = origin0;
	EFW_WishMove( pPlayer, fwd, side, &wish, &wishspeed );
	left = dt;
	slices = 0;
	while( left > 0.0005f && slices < 25 )
	{
		float slice = left;
		if( slice > 0.01f )
			slice = 0.01f;
		EFW_GroundFriction( pPlayer, slice );
		if( wishspeed > 0.0f )
			EFW_GroundAccelerate( wish.x, wish.y, wishspeed, slice );
		speed = sqrtf( s_hvx * s_hvx + s_hvy * s_hvy );
		if( speed < 1.0f )
		{
			s_hvx = 0.0f;
			s_hvy = 0.0f;
			break;
		}
		if( !EFW_ClipGroundStep( pPlayer, slice, &dest ) )
			break;
		/* Downhill the horizontal slice is clear and the floor falls
		   away. A lip onto a 45-degree ramp can drop more than one
		   slice of travel, up to STEPSIZE. Snap that far. A deeper
		   ledge still misses and leaves the hull in the air. */
		{
			float hx = dest.x - pPlayer->pev->origin.x;
			float hy = dest.y - pPlayer->pev->origin.y;
			float horiz = sqrtf( hx * hx + hy * hy );
			float drop = horiz + 2.0f;
			if( drop < 18.0f )
				drop = 18.0f;
			TraceResult floor;
			Vector bot;

			bot = dest;
			bot.z -= drop;
			UTIL_TraceHull( dest, bot, dont_ignore_monsters, EFW_PlayerHull( pPlayer ), pPlayer->edict(), &floor );
			if( !floor.fStartSolid && floor.flFraction < 1.0f
				&& floor.vecPlaneNormal.z >= 0.7f
				&& dest.z - floor.vecEndPos.z >= 0.5f )
			{
				static int s_downLog;
				if( s_downLog < 8 )
				{
					char line[160];
					s_downLog++;
					snprintf( line, sizeof( line ),
						"efw: slope z=%.1f -> %.1f n=%.2f %.2f %.2f at %.0f %.0f\n",
						dest.z, floor.vecEndPos.z,
						floor.vecPlaneNormal.x, floor.vecPlaneNormal.y, floor.vecPlaneNormal.z,
						floor.vecEndPos.x, floor.vecEndPos.y );
					EFW_LogLine( line );
				}
				dest = floor.vecEndPos;
			}
		}
		pPlayer->pev->origin = dest;
		UTIL_SetOrigin( pPlayer->pev, dest );
		left -= slice;
		slices++;
	}
	moved = ( dest - origin0 ).Length();
	{
		static int s_runLog;
		static int s_duckRun;
		static int s_coastLog;
		char line[192];
		int ducked = ( pPlayer->pev->flags & FL_DUCKING ) ? 1 : 0;
		int slow = ( s_speedKey || ( pPlayer->pev->button & IN_RUN ) ) ? 1 : 0;
		int useHold = ( s_useHeld || ( pPlayer->pev->button & IN_USE ) ) ? 1 : 0;
		if( wishspeed > 0.0f && s_runLog < 8 )
		{
			s_runLog++;
			snprintf( line, sizeof( line ),
				"efw: run hv=%.0f %.0f wish=%.0f moved=%.0f dt=%.3f at %.0f %.0f z=%.1f duck=%d spd=%d use=%d\n",
				s_hvx, s_hvy, wishspeed, moved, dt,
				dest.x, dest.y, dest.z, ducked, slow, useHold );
			EFW_LogLine( line );
		}
		else if( wishspeed > 0.0f && ducked && s_duckRun < 4 )
		{
			s_duckRun++;
			snprintf( line, sizeof( line ),
				"efw: duckrun hv=%.0f %.0f wish=%.0f moved=%.0f at %.0f %.0f z=%.1f viewz=%.0f\n",
				s_hvx, s_hvy, wishspeed, moved,
				dest.x, dest.y,
				dest.z, pPlayer->pev->view_ofs.z );
			EFW_LogLine( line );
		}
		else if( wishspeed <= 0.0f && moved > 1.0f && s_coastLog < 6 )
		{
			s_coastLog++;
			snprintf( line, sizeof( line ),
				"efw: coast hv=%.0f %.0f moved=%.0f dt=%.3f at %.0f %.0f\n",
				s_hvx, s_hvy, moved, dt, dest.x, dest.y );
			EFW_LogLine( line );
		}
	}
	{
		int ax = (int)fabsf( s_hvx );
		int ay = (int)fabsf( s_hvy );
		int steady = ( ay < 15 && ax > 200 ) || ( ax < 15 && ay > 200 );
		static int s_groundLog;
		if( !steady && s_groundLog < 48 && slices > 0 )
		{
			char line[128];
			s_groundLog++;
			snprintf( line, sizeof( line ),
				"efw: ground hv=%.0f %.0f wish=%d %d at %.0f %.0f\n",
				s_hvx, s_hvy, fwd, side,
				dest.x, dest.y );
			EFW_LogLine( line );
		}
	}
	}
	pPlayer->pev->velocity.x = s_hvx;
	pPlayer->pev->velocity.y = s_hvy;
	if( s_airborne )
		pPlayer->pev->velocity.z = s_vz;
	else if( !inAir )
		pPlayer->pev->velocity.z = 0.0f;
	if( s_airborne )
		pPlayer->pev->flags &= ~FL_ONGROUND;
	else
	{
		/* PM_CatagorizePosition: a 2-unit floor trace. A miss means the
		   step left the ledge, and forcing FL_ONGROUND kept the pawn at
		   the old height over the drop. */
		TraceResult down;
		Vector floorEnd;
		static int s_ledgeLog;

		floorEnd = dest;
		floorEnd.z -= 2.0f;
		UTIL_TraceHull( dest, floorEnd, dont_ignore_monsters, EFW_PlayerHull( pPlayer ), pPlayer->edict(), &down );
		if( down.fStartSolid )
			pPlayer->pev->flags |= FL_ONGROUND;
		else if( down.flFraction < 1.0f && down.vecPlaneNormal.z >= 0.7f )
		{
			if( dest.z - down.vecEndPos.z >= 0.5f )
				dest = down.vecEndPos;
			pPlayer->pev->flags |= FL_ONGROUND;
			s_vz = 0.0f;
			pPlayer->pev->velocity.z = 0.0f;
		}
		else
		{
			pPlayer->pev->flags &= ~FL_ONGROUND;
			if( s_ledgeLog < 8 )
			{
				char line[96];
				s_ledgeLog++;
				snprintf( line, sizeof( line ),
					"efw: ledge z=%.1f at %.0f %.0f\n",
					dest.z, dest.x, dest.y );
				EFW_LogLine( line );
			}
		}
	}
	pPlayer->pev->origin = dest;
	UTIL_SetOrigin( pPlayer->pev, dest );
	if( pPlayer->pev->flags & FL_ONGROUND )
		EFW_IdealPitch( pPlayer );
	s_moveN++;
	if( s_moveN == 1 || ( s_moveN % 20 ) == 0 )
	{
		char line[128];
		snprintf( line, sizeof( line ),
			"efw: walk origin=%.0f %.0f %.0f yaw=%.0f fwd=%d side=%d src=%s\n",
			dest.x, dest.y, dest.z, pPlayer->pev->angles.y, fwd, side,
			fromCmd ? "cmd" : "latch" );
		EFW_LogLine( line );
	}
}

/* V_CalcRoll. sv_rollangle is 0 on this host, and the client simvel stays
   0 while the pump moves the hull, so V_CalcViewRoll leaves the horizon
   level. GoldSrc uses 2 degrees at sv_rollspeed 200. svc_setangle follows
   pev->angles, so the roll is written there. */
static void EFW_ViewRoll( CBasePlayer *pPlayer )
{
	float side;
	float roll;
	float rollangle;
	float rollspeed;
	float sign;
	Vector right;
	static int s_log;
	char line[128];

	if( !pPlayer )
		return;
	rollangle = CVAR_GET_FLOAT( "sv_rollangle" );
	rollspeed = CVAR_GET_FLOAT( "sv_rollspeed" );
	if( rollangle < 0.05f )
		rollangle = 2.0f;
	if( rollspeed < 1.0f )
		rollspeed = 200.0f;
	UTIL_MakeVectors( pPlayer->pev->v_angle );
	right = gpGlobals->v_right;
	side = s_hvx * right.x + s_hvy * right.y;
	sign = ( side < 0.0f ) ? -1.0f : 1.0f;
	if( side < 0.0f )
		side = -side;
	if( side < rollspeed )
		roll = side * rollangle / rollspeed;
	else
		roll = rollangle;
	roll *= sign;
	/* PM_CheckFalling sets punchangle[2] = fallSpeed * 0.013. That vector
	   reaches UpdateClientData and the refdef still stays level.
	   svc_setangle follows pev->angles, which is what the view uses. */
	roll += s_punchRoll;
	pPlayer->pev->v_angle.z = roll;
	pPlayer->pev->angles.x = pPlayer->pev->v_angle.x;
	pPlayer->pev->angles.y = pPlayer->pev->v_angle.y;
	pPlayer->pev->angles.z = roll;
	pPlayer->pev->fixangle = 1;
	{
		static float s_sentRoll;
		if( roll > s_sentRoll + 0.15f || roll < s_sentRoll - 0.15f )
		{
			s_sentRoll = roll;
			CLIENT_COMMAND( pPlayer->edict(), "efw_vroll %.2f\n", roll );
		}
	}
	if( s_log < 6 && ( roll > 0.5f || roll < -0.5f ) )
	{
		s_log++;
		snprintf( line, sizeof( line ),
			"efw: roll %.2f side=%.0f at %.0f %.0f\n",
			roll, side * sign,
			pPlayer->pev->origin.x, pPlayer->pev->origin.y );
		EFW_LogLine( line );
	}
}

/* V_CalcBob. cl.time stays near 1, so the client bob freezes after the
   first frame. The same curve runs on the pump clock and adds to the
   eye. cl_bob 0.01, cl_bobcycle 0.8, cl_bobup 0.5. Air keeps the last bob. */
static void EFW_ViewBob( CBasePlayer *pPlayer, float dt )
{
	static float s_bobTime;
	static float s_bob;
	static int s_log;
	float cycle;
	float bob;
	float speed;
	float bobcycle;
	float bobup;
	float bobscale;
	int onGround;
	char line[128];

	if( !pPlayer )
		return;
	onGround = ( pPlayer->pev->flags & FL_ONGROUND ) ? 1 : 0;
	if( onGround )
	{
		if( dt < 0.001f )
			dt = 0.1f;
		if( dt > 0.25f )
			dt = 0.25f;
		bobcycle = CVAR_GET_FLOAT( "cl_bobcycle" );
		bobup = CVAR_GET_FLOAT( "cl_bobup" );
		bobscale = CVAR_GET_FLOAT( "cl_bob" );
		if( bobcycle < 0.05f )
			bobcycle = 0.8f;
		if( bobup < 0.05f || bobup > 0.95f )
			bobup = 0.5f;
		if( bobscale < 0.0001f )
			bobscale = 0.01f;
		s_bobTime += dt;
		cycle = s_bobTime - (float)( (int)( s_bobTime / bobcycle ) ) * bobcycle;
		cycle /= bobcycle;
		if( cycle < bobup )
			cycle = 3.14159265f * cycle / bobup;
		else
			cycle = 3.14159265f + 3.14159265f * ( cycle - bobup ) / ( 1.0f - bobup );
		speed = sqrtf( s_hvx * s_hvx + s_hvy * s_hvy );
		bob = speed * bobscale;
		bob = bob * 0.3f + bob * 0.7f * sinf( cycle );
		if( bob > 4.0f )
			bob = 4.0f;
		if( bob < -7.0f )
			bob = -7.0f;
		s_bob = bob;
	}
	pPlayer->pev->view_ofs.z += s_bob;
	EFW_ViewRoll( pPlayer );
	if( onGround && s_log < 6 && sqrtf( s_hvx * s_hvx + s_hvy * s_hvy ) > 100.0f )
	{
		s_log++;
		snprintf( line, sizeof( line ),
			"efw: bob %.2f viewz=%.1f spd=%.0f at %.0f %.0f\n",
			s_bob, pPlayer->pev->view_ofs.z,
			sqrtf( s_hvx * s_hvx + s_hvy * s_hvy ),
			pPlayer->pev->origin.x, pPlayer->pev->origin.y );
		EFW_LogLine( line );
	}
}

/* PM_FlyMove along one displacement. Returns 1 when a descending segment
   hits a walkable floor, -1 when the hull starts in a solid, else 0. */
static int EFW_FlyDisplace( CBasePlayer *pPlayer, const Vector &wish, Vector *out )
{
	Vector pos;
	Vector remain;
	int bump;
	int hull;

	if( !pPlayer || !out )
		return 0;
	pos = pPlayer->pev->origin;
	remain = wish;
	hull = EFW_PlayerHull( pPlayer );
	for( bump = 0; bump < 4; bump++ )
	{
		TraceResult tr = {};
		Vector end;
		float back;
		float span;

		span = remain.x * remain.x + remain.y * remain.y + remain.z * remain.z;
		if( span < 0.0001f )
			break;
		end = pos + remain;
		UTIL_TraceHull( pos, end, dont_ignore_monsters, hull, pPlayer->edict(), &tr );
		if( tr.fStartSolid )
		{
			*out = pos;
			return -1;
		}
		if( tr.flFraction > 0.0f )
			pos = tr.vecEndPos;
		if( tr.flFraction >= 1.0f )
			break;
		if( remain.z <= 0.05f && tr.vecPlaneNormal.z >= 0.7f )
		{
			*out = pos;
			return 1;
		}
		remain = remain * ( 1.0f - tr.flFraction );
		back = remain.x * tr.vecPlaneNormal.x
			+ remain.y * tr.vecPlaneNormal.y
			+ remain.z * tr.vecPlaneNormal.z;
		if( back < 0.0f )
		{
			remain.x -= back * tr.vecPlaneNormal.x;
			remain.y -= back * tr.vecPlaneNormal.y;
			remain.z -= back * tr.vecPlaneNormal.z;
		}
		back = s_hvx * tr.vecPlaneNormal.x + s_hvy * tr.vecPlaneNormal.y;
		if( back < 0.0f )
		{
			s_hvx -= back * tr.vecPlaneNormal.x;
			s_hvy -= back * tr.vecPlaneNormal.y;
		}
		if( tr.vecPlaneNormal.z < 0.0f && s_vz > 0.0f )
			s_vz = 0.0f;
	}
	*out = pos;
	return 0;
}

static void EFW_ApplyUsercmdAir( CBasePlayer *pPlayer )
{
	float dt;
	float frac;
	int buttons;
	int pressed;
	Vector dest;
	char line[160];
	static int s_jumpN;

	if( !pPlayer || !s_walkOn )
		return;
	if( EFW_GetHudInt( 6 ) )
		return;
	dt = g_efw.dt;
	if( dt <= 0.0f )
		dt = EFW_HostInterval();
	if( dt > 0.2f )
		dt = 0.2f;
	EFW_DropPunch( dt );
	buttons = EFW_LiveButtons( pPlayer );
	pressed = buttons & ~s_oldAirButtons;
	/* PM_Jump moves the hull and leaves the standing eye at 28. The
	   camera follows origin, so adding the hop onto view_ofs put the
	   eye a second storey above the pawn. */
	if( !s_floorSet && !s_airborne )
	{
		s_floorSet = 1;
		s_floorZ = pPlayer->pev->origin.z;
	}

	if( buttons & IN_DUCK )
	{
		if( ( pressed & IN_DUCK ) && !( pPlayer->pev->flags & FL_DUCKING ) && !s_inDuck )
		{
			s_inDuck = 1;
			s_duckTime = 0.0f;
			s_duckStandZ = pPlayer->pev->origin.z;
		}
		if( s_inDuck )
		{
			s_duckTime += dt;
			/* PM finishes the crouch at TIME_TO_DUCK, or at once in the air.
			   On the ground the origin drops by 18 (duck mins.z − standing
			   mins.z) and the eye becomes VEC_DUCK_VIEW. In the air the eye
			   is 12 and the origin stays. */
			if( s_duckTime >= 0.4f || s_airborne )
			{
				int inAirFinish;

				s_inDuck = 0;
				pPlayer->pev->flags |= FL_DUCKING;
				inAirFinish = ( s_airborne || !( pPlayer->pev->flags & FL_ONGROUND ) ) ? 1 : 0;
				if( inAirFinish )
				{
					pPlayer->pev->view_ofs.z = 12.0f;
					snprintf( line, sizeof( line ),
						"efw: duck air viewz=12 z=%.1f\n",
						pPlayer->pev->origin.z );
					EFW_LogLine( line );
				}
				else
				{
					Vector dropped;
					Vector saved;
					int i;
					int freed;

					dropped = pPlayer->pev->origin;
					dropped.z -= 18.0f;
					saved = dropped;
					freed = 0;
					for( i = 0; i < 36; i++ )
					{
						TraceResult stuck;

						UTIL_TraceHull( dropped, dropped, dont_ignore_monsters, head_hull,
							pPlayer->edict(), &stuck );
						if( !stuck.fStartSolid && !stuck.fAllSolid )
						{
							freed = 1;
							break;
						}
						dropped.z += 1.0f;
					}
					if( !freed )
						dropped = saved;
					pPlayer->pev->origin = dropped;
					UTIL_SetOrigin( pPlayer->pev, dropped );
					s_floorZ = dropped.z;
					s_floorSet = 1;
					pPlayer->pev->view_ofs.z = 12.0f;
					snprintf( line, sizeof( line ),
						"efw: duck drop z=%.1f -> %.1f\n",
						s_duckStandZ, dropped.z );
					EFW_LogLine( line );
				}
			}
			else
			{
				frac = EFW_DuckSpline( s_duckTime );
				pPlayer->pev->view_ofs.z = ( ( 12.0f - 18.0f ) * frac ) + ( 28.0f * ( 1.0f - frac ) );
				snprintf( line, sizeof( line ),
					"efw: duck viewz=%.1f t=%.2f z=%.1f\n",
					pPlayer->pev->view_ofs.z, s_duckTime, pPlayer->pev->origin.z );
				EFW_LogLine( line );
			}
		}
	}
	else if( s_inDuck )
	{
		/* Released during the spline, before the origin drop. */
		s_inDuck = 0;
		s_duckTime = 0.0f;
		pPlayer->pev->view_ofs.z = 28.0f;
		snprintf( line, sizeof( line ),
			"efw: unduck viewz=28 z=%.1f\n", pPlayer->pev->origin.z );
		EFW_LogLine( line );
	}
	else if( pPlayer->pev->flags & FL_DUCKING )
	{
		Vector up;
		TraceResult stand;
		float lift;

		/* PM_UnDuck adds the 18 back only after a ground drop, and only
		   when the standing hull fits. A low ceiling leaves the crouch. */
		up = pPlayer->pev->origin;
		lift = 0.0f;
		if( !s_airborne && ( pPlayer->pev->flags & FL_ONGROUND )
			&& pPlayer->pev->origin.z < s_duckStandZ - 9.0f )
			lift = 18.0f;
		up.z += lift;
		UTIL_TraceHull( up, up, dont_ignore_monsters, human_hull, pPlayer->edict(), &stand );
		if( stand.fStartSolid || stand.fAllSolid )
		{
			static int s_blockLog;

			pPlayer->pev->view_ofs.z = 12.0f;
			if( s_blockLog < 4 )
			{
				s_blockLog++;
				snprintf( line, sizeof( line ),
					"efw: unduck blocked z=%.1f\n", pPlayer->pev->origin.z );
				EFW_LogLine( line );
			}
		}
		else
		{
			s_inDuck = 0;
			s_duckTime = 0.0f;
			pPlayer->pev->flags &= ~FL_DUCKING;
			pPlayer->pev->origin = up;
			UTIL_SetOrigin( pPlayer->pev, up );
			s_floorZ = up.z;
			s_floorSet = 1;
			pPlayer->pev->view_ofs.z = 28.0f;
			snprintf( line, sizeof( line ),
				"efw: unduck z=%.1f lift=%.0f\n", up.z, lift );
			EFW_LogLine( line );
		}
	}

	if( !s_airborne && ( pressed & IN_JUMP ) )
	{
		int onGround = ( pPlayer->pev->flags & FL_ONGROUND ) != 0;

		/* PM_Jump returns while onground == -1. A fall that has already
		   left the takeoff height must not relaunch from that stored
		   floor. The listen server can still apply the impulse and lift
		   the origin before this pump; that hop keeps the stored floor. */
		if( !onGround && pPlayer->pev->origin.z <= s_floorZ + 1.0f )
		{
			snprintf( line, sizeof( line ),
				"efw: jump ignored z=%.1f floor=%.1f flags=%d\n",
				pPlayer->pev->origin.z, s_floorZ, pPlayer->pev->flags );
			EFW_LogLine( line );
		}
		else
		{
			if( onGround )
				s_floorZ = pPlayer->pev->origin.z;
			/* PM_Jump impulse sqrt(2 * 800 * 45). */
			s_jumpVz0 = sqrtf( 2.0f * 800.0f * 45.0f );
			s_vz = s_jumpVz0;
			s_jumpT = 0.0f;
			s_airborne = 1;
			pPlayer->pev->flags &= ~FL_ONGROUND;
			snprintf( line, sizeof( line ),
				"efw: jump impulse vz=%.0f z=%.1f floor=%.1f hv=%.0f %.0f\n",
				s_vz, pPlayer->pev->origin.z, s_floorZ, s_hvx, s_hvy );
			EFW_LogLine( line );
		}
	}

	/* PM_AddCorrectGravity still runs after the step that left the floor.
	   The jump arc is that fall with a zero takeoff speed. Clearing
	   FL_ONGROUND and leaving origin.z put the hull in the air over the drop. */
	if( !s_airborne && !( pPlayer->pev->flags & FL_ONGROUND ) )
	{
		static int s_dropLog;

		s_floorZ = pPlayer->pev->origin.z;
		s_floorSet = 1;
		s_jumpVz0 = 0.0f;
		s_vz = 0.0f;
		s_jumpT = 0.0f;
		s_airborne = 1;
		if( s_dropLog < 6 )
		{
			s_dropLog++;
			snprintf( line, sizeof( line ),
				"efw: drop z=%.1f at %.0f %.0f\n",
				s_floorZ, pPlayer->pev->origin.x, pPlayer->pev->origin.y );
			EFW_LogLine( line );
		}
	}

	if( s_airborne )
	{
		float left;
		int slices;

		/* A GoldSrc usercmd is about 10ms. One 0.2s diagonal clears a
		   deck that those short steps land on. */
		left = dt;
		slices = 0;
		dest = pPlayer->pev->origin;
		while( s_airborne && left > 0.0005f && slices < 25 )
		{
			float slice;
			float z;
			Vector top;
			Vector wish;
			int hit;

			slice = left;
			if( slice > 0.01f )
				slice = 0.01f;
			s_jumpT += slice;
			s_vz = s_jumpVz0 - 800.0f * s_jumpT;
			z = s_floorZ + s_jumpVz0 * s_jumpT - 0.5f * 800.0f * s_jumpT * s_jumpT;
			top = pPlayer->pev->origin;
			wish = Vector( s_hvx * slice, s_hvy * slice, z - top.z );
			hit = EFW_FlyDisplace( pPlayer, wish, &dest );
			if( hit < 0 && wish.z <= 0.0f )
			{
				dest = top;
				s_airborne = 0;
				s_vz = 0.0f;
				pPlayer->pev->flags &= ~FL_ONGROUND;
			}
			else if( hit > 0 )
			{
				float fall;
				float drop;

				drop = s_floorZ - dest.z;
				if( drop < 0.0f )
					drop = 0.0f;
				fall = sqrtf( s_jumpVz0 * s_jumpVz0 + 2.0f * 800.0f * drop );
				s_airborne = 0;
				s_vz = 0.0f;
				pPlayer->pev->flags |= FL_ONGROUND;
				/* PLAYER_FALL_PUNCH_THRESHHOLD is 350. A 45-unit hop
				   lands near 268 and does not punch. The roll axis is
				   not clamped; the SDK clamp is on punchangle[0]. */
				if( fall >= 350.0f )
				{
					static int s_fallLog;

					s_punchRoll = fall * 0.013f;
					if( s_fallLog < 6 )
					{
						s_fallLog++;
						snprintf( line, sizeof( line ),
							"efw: fall vz=%.0f punch=%.2f z=%.1f from %.1f at %.0f %.0f\n",
							fall, s_punchRoll, dest.z, s_floorZ, dest.x, dest.y );
						EFW_LogLine( line );
					}
				}
				else
				{
					snprintf( line, sizeof( line ),
						"efw: jump land z=%.1f from %.1f vz=%.0f at %.0f %.0f\n",
						dest.z, s_floorZ, fall, dest.x, dest.y );
					EFW_LogLine( line );
				}
			}
			pPlayer->pev->origin = dest;
			pPlayer->pev->velocity.z = s_vz;
			UTIL_SetOrigin( pPlayer->pev, dest );
			left -= slice;
			slices++;
		}
	}

	if( !( buttons & IN_DUCK ) && !s_inDuck && !( pPlayer->pev->flags & FL_DUCKING ) )
		pPlayer->pev->view_ofs.z = 28.0f;
	else if( ( buttons & IN_DUCK ) && ( pPlayer->pev->flags & FL_DUCKING ) && !s_inDuck )
	{
		/* After the ground drop the eye is VEC_DUCK_VIEW. Bob adds after this. */
		if( pPlayer->pev->origin.z < s_duckStandZ - 9.0f )
			pPlayer->pev->view_ofs.z = 12.0f;
	}
	/* Sample the floor only while the hull is down. A falling origin
	   used to become the next takeoff height. */
	if( !s_airborne && !( buttons & IN_JUMP ) && ( pPlayer->pev->flags & FL_ONGROUND ) )
	{
		s_floorZ = pPlayer->pev->origin.z;
		s_floorSet = 1;
	}
	if( s_airborne || ( s_floorSet && pPlayer->pev->origin.z > s_floorZ + 1.0f ) )
	{
		if( s_jumpN < 48 )
		{
			s_jumpN++;
			snprintf( line, sizeof( line ),
				"efw: jump z=%.1f vz=%.0f air=%d viewz=%.0f\n",
				pPlayer->pev->origin.z, s_vz, s_airborne, pPlayer->pev->view_ofs.z );
			EFW_LogLine( line );
		}
	}

	EFW_ViewBob( pPlayer, dt );
	s_oldAirButtons = buttons;
}

/* PM_PlayStepSound in this DLL (0x10084940). Concrete, the materials.txt
   default, is Footsteps/Guard_Footstep_Generic_1..4 in irand order.
   Dirt is Footsteps/Player_Footstep_Dirt_5,2,3,4. Metal, vent, grate,
   tile, slosh and ladder keep the stock player/pl_* set. irand is
   RandomLong(0,1) plus the toggled foot * 2. */
static const char *EFW_StepSample( int step, int irand )
{
	static const char *kMetal[] = {
		"player/pl_metal1.wav", "player/pl_metal3.wav",
		"player/pl_metal2.wav", "player/pl_metal4.wav"
	};
	static const char *kDirt[] = {
		"Footsteps/Player_Footstep_Dirt_5.wav",
		"Footsteps/Player_Footstep_Dirt_2.wav",
		"Footsteps/Player_Footstep_Dirt_3.wav",
		"Footsteps/Player_Footstep_Dirt_4.wav"
	};
	static const char *kDuct[] = {
		"player/pl_duct1.wav", "player/pl_duct3.wav",
		"player/pl_duct2.wav", "player/pl_duct4.wav"
	};
	static const char *kGrate[] = {
		"player/pl_grate1.wav", "player/pl_grate3.wav",
		"player/pl_grate2.wav", "player/pl_grate4.wav"
	};
	static const char *kTile[] = {
		"player/pl_tile1.wav", "player/pl_tile3.wav",
		"player/pl_tile2.wav", "player/pl_tile4.wav",
		"player/pl_tile5.wav"
	};
	static const char *kSlosh[] = {
		"player/pl_slosh1.wav", "player/pl_slosh3.wav",
		"player/pl_slosh2.wav", "player/pl_slosh4.wav"
	};
	static const char *kWade[] = {
		"player/pl_wade1.wav", "player/pl_wade2.wav",
		"player/pl_wade3.wav", "player/pl_wade4.wav"
	};
	static const char *kLadder[] = {
		"player/pl_ladder1.wav", "player/pl_ladder3.wav",
		"player/pl_ladder2.wav", "player/pl_ladder4.wav"
	};
	static const char *kGuard[] = {
		"Footsteps/Guard_Footstep_Generic_1.wav",
		"Footsteps/Guard_Footstep_Generic_2.wav",
		"Footsteps/Guard_Footstep_Generic_3.wav",
		"Footsteps/Guard_Footstep_Generic_4.wav"
	};
	const char **tab = kGuard;
	int n = 4;

	switch( step )
	{
	case 1: tab = kMetal; break;
	case 2: tab = kDirt; break;
	case 3: tab = kDuct; break;
	case 4: tab = kGrate; break;
	case 5: tab = kTile; n = 5; break;
	case 6: tab = kSlosh; break;
	case 7: tab = kWade; break;
	case 8: tab = kLadder; break;
	default: break;
	}
	if( irand < 0 )
		irand = 0;
	if( irand >= n )
		irand = n - 1;
	return tab[irand];
}

static int EFW_TexCmp( const void *a, const void *b )
{
	const char *na = (const char *)a;
	const char *nb = (const char *)b;
	int i;
	/* Entries are 13 bytes: type at [0], name at [1]. */
	na++;
	nb++;
	for( i = 0; i < 12; i++ )
	{
		unsigned char ca = (unsigned char)na[i];
		unsigned char cb = (unsigned char)nb[i];
		if( ca >= 'a' && ca <= 'z' )
			ca = (unsigned char)( ca - 32 );
		if( cb >= 'a' && cb <= 'z' )
			cb = (unsigned char)( cb - 32 );
		if( ca != cb )
			return (int)ca - (int)cb;
		if( !ca )
			return 0;
	}
	return 0;
}

static char EFW_TextureType( const char *texName )
{
	static char s_tex[512 * 13];
	static int s_n;
	static int s_loaded;
	char query[13];
	int i;

	if( !s_loaded )
	{
		int length = 0;
		char *buf;
		int pos = 0;

		s_loaded = 1;
		s_n = 0;
		buf = (char *)LOAD_FILE_FOR_ME( (char *)"sound/materials.txt", &length );
		if( buf && length > 0 )
		{
			while( pos < length && s_n < 512 )
			{
				int line = pos;
				char type;
				int nameAt;
				int copied;

				while( pos < length && buf[pos] != '\n' )
					pos++;
				if( pos < length && buf[pos] == '\n' )
					pos++;
				while( line < pos && ( buf[line] == ' ' || buf[line] == '\t' || buf[line] == '\r' ) )
					line++;
				if( line >= pos || buf[line] == '/' || buf[line] == '\n' || buf[line] == '\r' )
					continue;
				type = buf[line];
				if( type >= 'a' && type <= 'z' )
					type = (char)( type - 32 );
				if( type < 'A' || type > 'Z' )
					continue;
				line++;
				while( line < pos && ( buf[line] == ' ' || buf[line] == '\t' ) )
					line++;
				if( line >= pos || buf[line] == '\n' || buf[line] == '\r' )
					continue;
				s_tex[s_n * 13] = type;
				nameAt = s_n * 13 + 1;
				copied = 0;
				while( copied < 12 && line < pos && buf[line] != ' ' && buf[line] != '\t'
					&& buf[line] != '\r' && buf[line] != '\n' )
				{
					s_tex[nameAt + copied] = buf[line];
					copied++;
					line++;
				}
				if( copied < 12 )
					s_tex[nameAt + copied] = '\0';
				s_n++;
			}
			FREE_FILE( buf );
			if( s_n > 1 )
				qsort( s_tex, (size_t)s_n, 13, EFW_TexCmp );
		}
		{
			char line[80];
			snprintf( line, sizeof( line ), "efw: materials %d\n", s_n );
			EFW_LogLine( line );
		}
	}
	if( !texName || !texName[0] || s_n < 1 )
		return 'C';
	if( texName[0] == '-' || texName[0] == '+' )
		texName += 2;
	if( texName[0] == '{' || texName[0] == '!' || texName[0] == '~' || texName[0] == ' ' )
		texName++;
	for( i = 0; i < 12; i++ )
	{
		query[i] = texName[i];
		if( !texName[i] )
			break;
	}
	query[i < 12 ? i : 12] = '\0';
	{
		char probe[13];
		int left = 0;
		int right = s_n - 1;

		memset( probe, 0, sizeof( probe ) );
		probe[0] = 'C';
		for( i = 0; query[i] && i < 12; i++ )
			probe[1 + i] = query[i];
		while( left <= right )
		{
			int mid = ( left + right ) / 2;
			int cmp = EFW_TexCmp( probe, s_tex + mid * 13 );
			if( cmp == 0 )
				return s_tex[mid * 13];
			if( cmp > 0 )
				left = mid + 1;
			else
				right = mid - 1;
		}
	}
	return 'C';
}

static void EFW_PrecacheSteps( void )
{
	static const char *kWav[] = {
		"Footsteps/Guard_Footstep_Generic_1.wav",
		"Footsteps/Guard_Footstep_Generic_2.wav",
		"Footsteps/Guard_Footstep_Generic_3.wav",
		"Footsteps/Guard_Footstep_Generic_4.wav",
		"Footsteps/Player_Footstep_Dirt_2.wav",
		"Footsteps/Player_Footstep_Dirt_3.wav",
		"Footsteps/Player_Footstep_Dirt_4.wav",
		"Footsteps/Player_Footstep_Dirt_5.wav",
		"player/pl_metal1.wav", "player/pl_metal2.wav",
		"player/pl_metal3.wav", "player/pl_metal4.wav",
		"player/pl_duct1.wav", "player/pl_duct2.wav",
		"player/pl_duct3.wav", "player/pl_duct4.wav",
		"player/pl_grate1.wav", "player/pl_grate2.wav",
		"player/pl_grate3.wav", "player/pl_grate4.wav",
		"player/pl_tile1.wav", "player/pl_tile2.wav",
		"player/pl_tile3.wav", "player/pl_tile4.wav",
		"player/pl_tile5.wav",
		"player/pl_slosh1.wav", "player/pl_slosh2.wav",
		"player/pl_slosh3.wav", "player/pl_slosh4.wav",
		"player/pl_wade1.wav", "player/pl_wade2.wav",
		"player/pl_wade3.wav", "player/pl_wade4.wav",
		"player/pl_ladder1.wav", "player/pl_ladder2.wav",
		"player/pl_ladder3.wav", "player/pl_ladder4.wav"
	};
	unsigned i;
	for( i = 0; i < sizeof( kWav ) / sizeof( kWav[0] ); i++ )
		PRECACHE_SOUND( (char *)kWav[i] );
}

/* PM_UpdateStepSound. The pump is the cmd.msec clock. A timer that
   reaches 0 this pump still plays, matching ReduceTimers before the
   step check. */
static void EFW_UpdateStepSound( CBasePlayer *pPlayer )
{
	int msec;
	float speed;
	float velwalk;
	float velrun;
	float flduck;
	int ducked;
	int step;
	float fvol;
	int irand;
	const char *sample;
	const char *texName = NULL;
	char texType;
	Vector start;
	Vector end;
	static int s_skipWade;
	static int s_log;

	if( !pPlayer )
		return;
	msec = (int)( EFW_HostInterval() * 1000.0f );
	if( msec < 1 )
		msec = 1;
	if( pPlayer->m_flTimeStepSound > 0.0f )
	{
		pPlayer->m_flTimeStepSound -= (float)msec;
		if( pPlayer->m_flTimeStepSound < 0.0f )
			pPlayer->m_flTimeStepSound = 0.0f;
	}
	if( pPlayer->m_flTimeStepSound > 0.0f )
		return;
	if( pPlayer->pev->flags & FL_FROZEN )
		return;
	speed = pPlayer->pev->velocity.Length();
	ducked = ( pPlayer->pev->flags & FL_DUCKING ) ? 1 : 0;
	if( ducked || pPlayer->pev->movetype == MOVETYPE_FLY )
	{
		velwalk = 60.0f;
		velrun = 80.0f;
		flduck = 100.0f;
	}
	else
	{
		velwalk = 120.0f;
		velrun = 210.0f;
		flduck = 0.0f;
	}
	if( !( pPlayer->pev->flags & FL_ONGROUND ) && pPlayer->pev->movetype != MOVETYPE_FLY )
		return;
	if( speed <= 0.0f )
		return;
	if( speed < velwalk && pPlayer->m_flTimeStepSound != 0.0f )
		return;
	{
		int walking = speed < velrun;
		float height;
		Vector knee;
		Vector feet;

		height = pPlayer->pev->maxs.z - pPlayer->pev->mins.z;
		knee = pPlayer->pev->origin;
		feet = pPlayer->pev->origin;
		knee.z -= 0.3f * height;
		feet.z -= 0.5f * height;
		if( pPlayer->pev->movetype == MOVETYPE_FLY )
		{
			step = 8;
			fvol = 0.35f;
			pPlayer->m_flTimeStepSound = 350.0f;
		}
		else if( UTIL_PointContents( knee ) == CONTENTS_WATER )
		{
			step = 7;
			fvol = 0.65f;
			pPlayer->m_flTimeStepSound = 600.0f;
		}
		else if( UTIL_PointContents( feet ) == CONTENTS_WATER )
		{
			step = 6;
			fvol = walking ? 0.2f : 0.5f;
			pPlayer->m_flTimeStepSound = walking ? 400.0f : 300.0f;
		}
		else
		{
			start = pPlayer->pev->origin;
			end = start;
			end.z -= 64.0f;
			{
				edict_t *world = INDEXENT( 0 );
				texName = world ? TRACE_TEXTURE( world, start, end ) : NULL;
			}
			texType = EFW_TextureType( texName );
			switch( texType )
			{
			case 'M': step = 1; break;
			case 'D': step = 2; break;
			case 'V': step = 3; break;
			case 'G': step = 4; break;
			case 'T': step = 5; break;
			case 'S': step = 6; break;
			default: step = 0; break;
			}
			if( texType == 'D' )
				fvol = walking ? 0.25f : 0.55f;
			else if( texType == 'V' )
				fvol = walking ? 0.4f : 0.7f;
			else
				fvol = walking ? 0.2f : 0.5f;
			pPlayer->m_flTimeStepSound = walking ? 400.0f : 300.0f;
		}
	}
	pPlayer->m_flTimeStepSound += flduck;
	if( ducked )
		fvol *= 0.35f;
	pPlayer->m_iStepLeft = !pPlayer->m_iStepLeft;
	irand = RANDOM_LONG( 0, 1 ) + ( pPlayer->m_iStepLeft ? 2 : 0 );
	if( step == 5 && !RANDOM_LONG( 0, 4 ) )
		irand = 4;
	if( step == 7 )
	{
		if( s_skipWade == 0 )
		{
			s_skipWade++;
			return;
		}
		if( s_skipWade++ == 3 )
			s_skipWade = 0;
	}
	sample = EFW_StepSample( step, irand );
	EMIT_SOUND_DYN( pPlayer->edict(), CHAN_BODY, sample, fvol, ATTN_NORM, 0, PITCH_NORM );
	if( s_log < 8 )
	{
		char line[160];
		s_log++;
		snprintf( line, sizeof( line ),
			"efw: step %s vol=%.2f spd=%.0f tex=%s\n",
			sample, fvol, speed, texName ? texName : "-" );
		EFW_LogLine( line );
	}
}

static void EFW_ForceWorldPresent( CBasePlayer *pPlayer )
{
	char line[128];

	if( s_presentOn || !pPlayer )
		return;
	s_presentOn = 1;
	CVAR_SET_FLOAT( "r_norefresh", 0.0f );
	CVAR_SET_FLOAT( "r_drawworld", 1.0f );
	CVAR_SET_FLOAT( "r_drawentities", 1.0f );
	CVAR_SET_FLOAT( "r_fullbright", 0.0f );
	CVAR_SET_FLOAT( "r_novis", 1.0f );
	SERVER_COMMAND( "r_norefresh 0\nr_drawworld 1\nr_drawentities 1\nr_fullbright 0\nr_novis 1\ngl_clear 1\nui_renderworld 1\n" );
	CLIENT_COMMAND( pPlayer->edict(), "r_norefresh 0\nr_drawworld 1\nr_drawentities 1\n" );
	snprintf( line, sizeof( line ),
		"efw: world present live=%d origin=%.0f %.0f %.0f\n",
		s_liveTicks, pPlayer->pev->origin.x, pPlayer->pev->origin.y, pPlayer->pev->origin.z );
	EFW_LogLine( line );
}

void EFW_StartFrame( void )
{
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
	{
		CBasePlayer *pLive = EFW_Player();
		if( pLive && g_efw.player != pLive )
			EFW_SetPlayer( pLive );
		s_hudPulse++;
		EFW_SendHudState();
		EFW_PollMenuKeys();
		if( !s_bindDone && !EFW_BindOneDetainee() )
			s_bindDone = 1;
		EFW_PulseRefugeeThinks();
		EFW_PulseWorld( pLive );
	}
	/* After HUD_Redraw has proven ca_active, DROP_TO_FLOOR then WALK.
	   dll119 kept noclip forever because dropping it at live>=45 without a
	   hull put the pawn in the void. Remaining studios bind via MODEL_INDEX
	   one per frame (SET_MODEL of Security.mdl still wipes WebGL2). */
	{
		int j;
		CBasePlayer *pPlayer = EFW_Player();
		if( pPlayer )
		{
			if( EFW_GetHudInt( 6 ) )
				pPlayer->pev->movetype = MOVETYPE_NONE;
			else if( s_walkOn )
				pPlayer->pev->movetype = MOVETYPE_WALK;
			else if( s_liveTicks >= 16 )
			{
				int dropped;
				pPlayer->pev->solid = SOLID_SLIDEBOX;
				UTIL_SetSize( pPlayer->pev, VEC_HULL_MIN, VEC_HULL_MAX );
				dropped = DROP_TO_FLOOR( pPlayer->edict() );
				if( !dropped && s_liveTicks < 24 )
					pPlayer->pev->movetype = MOVETYPE_NOCLIP;
				else
				{
					if( !dropped )
						dropped = EFW_SnapToPlayerStart( pPlayer );
					s_walkOn = 1;
					pPlayer->pev->movetype = MOVETYPE_WALK;
					pPlayer->pev->flags |= FL_ONGROUND;
					pPlayer->pev->velocity = g_vecZero;
					/* info_player_start angles "0 90 0" — look +Y into the barracks. */
					if( pPlayer->pev->angles.y == 0.0f && pPlayer->pev->angles.x == 0.0f )
						pPlayer->pev->angles = Vector( 0, 90, 0 );
					pPlayer->pev->v_angle = pPlayer->pev->angles;
					pPlayer->pev->fixangle = 1;
					{
						char pos[128];
						snprintf( pos, sizeof( pos ),
							"efw: DROP_TO_FLOOR %s — MOVETYPE_WALK origin=%.0f %.0f %.0f yaw=%.0f\n",
							dropped ? "ok" : "snap",
							pPlayer->pev->origin.x, pPlayer->pev->origin.y, pPlayer->pev->origin.z,
							pPlayer->pev->angles.y );
						EFW_LogLine( pos );
					}
					EFW_ForceWorldPresent( pPlayer );
				}
			}
			else
				pPlayer->pev->movetype = MOVETYPE_NOCLIP;
			if( s_walkOn )
			{
				EFW_ApplyLatchedMove( pPlayer );
				EFW_ApplyUsercmdAir( pPlayer );
				EFW_UpdateStepSound( pPlayer );
				EFW_ForceWorldPresent( pPlayer );
			}
		}
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
			if( ( e->v.flags & FL_MONSTER ) && e->v.modelindex > 0 )
				continue;
			/* Doors and buttons move on their own think. Zeroing nextthink
			   every frame left the barracks door shut. */
			if( e->v.movetype == MOVETYPE_PUSH )
				continue;
			{
				const char *cn = e->v.classname ? STRING( e->v.classname ) : "";
				if( !strncmp( cn, "func_", 5 ) || !strncmp( cn, "trigger_", 8 ) )
					continue;
			}
			e->v.nextthink = 0;
			if( e->v.movetype == MOVETYPE_STEP || e->v.movetype == MOVETYPE_FLY
				|| e->v.movetype == MOVETYPE_TOSS || e->v.movetype == MOVETYPE_WALK )
				e->v.movetype = MOVETYPE_NONE;
		}
		if( s_liveTicks <= 8 || s_liveTicks == 45 || ( s_liveTicks % 120 ) == 1 )
		{
			char line[128];
			if( pPlayer )
				snprintf( line, sizeof( line ),
					"efw: StartFrame done live=%d walk=%d bind=%d origin=%.0f %.0f %.0f\n",
					s_liveTicks, s_walkOn, s_bindDone,
					pPlayer->pev->origin.x, pPlayer->pev->origin.y, pPlayer->pev->origin.z );
			else
				snprintf( line, sizeof( line ), "efw: StartFrame done live=%d walk=%d bind=%d\n",
					s_liveTicks, s_walkOn, s_bindDone );
			EFW_LogLine( line );
		}
	}
	/* After the pulse. Linking inside the think re-entered it until the
	   pump faulted and the hull stopped. */
	EFW_FlushNpcOrigins();
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
	s_walkOn = 0;
	s_bindDone = 0;
	s_presentOn = 0;
	s_moveFwd = 0;
	s_moveSide = 0;
	s_speedKey = 0;
	s_useHeld = 0;
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
	s_walkOn = 0;
	s_bindDone = 0;
	s_presentOn = 0;
	s_moveFwd = 0;
	s_moveSide = 0;
	s_speedKey = 0;
	s_useHeld = 0;
	s_precacheMap[0] = 0;
	s_precacheSeenN = 0;
	memset( s_precacheSeen, 0, sizeof( s_precacheSeen ) );
	EFW_SpawnResetSeen();
}

void EFW_WPrecache( void )
{
	{
		static int s_wp;
		if( !s_wp )
		{
			s_wp = 1;
			EFW_DebugPrint( ">>> FUN_100b2f80 weapon_efw_Pliers..WashingPowder" );
		}
	}
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
	EFW_PrecacheSteps();
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

static int EFW_UseNearbyDoor( CBasePlayer *pPlayer )
{
	static const char *kClasses[] = { "func_door_rotating", "func_door", "func_button" };
	CBaseEntity *pBest = NULL;
	Vector bestMid;
	float bestDot = 0.7f; /* HL PlayerUse VIEW_FIELD_NARROW */
	Vector eye;
	Vector fwd;
	int c;
	if( !pPlayer )
		return 0;
	eye = pPlayer->EyePosition();
	UTIL_MakeVectors( pPlayer->pev->v_angle );
	fwd = gpGlobals->v_forward;
	for( c = 0; c < (int)( sizeof( kClasses ) / sizeof( kClasses[0] ) ); c++ )
	{
		CBaseEntity *pScan = NULL;
		while( ( pScan = UTIL_FindEntityByClassname( pScan, kClasses[c] ) ) != NULL )
		{
			Vector mid = ( pScan->pev->absmin + pScan->pev->absmax ) * 0.5f;
			Vector dir = mid - eye;
			float d = dir.Length();
			float dot;
			if( d > 96.0f || d < 1.0f )
				continue;
			dir = dir * ( 1.0f / d );
			dot = DotProduct( dir, fwd );
			if( dot > bestDot )
			{
				bestDot = dot;
				pBest = pScan;
				bestMid = mid;
			}
		}
	}
	if( !pBest )
		return 0;
	EFW_DebugPrint( ">>> use door %s %s dot=%.2f",
		STRING( pBest->pev->classname ), STRING( pBest->pev->targetname ), bestDot );
	{
		/* Step back so the hull is not inside the leaf. A blocked door
		   reverses and looks like Use did nothing. */
		Vector away = pPlayer->pev->origin - bestMid;
		TraceResult tr;
		away.z = 0.0f;
		if( away.Length() < 1.0f )
			away = Vector( 0, -1, 0 );
		away = away.Normalize() * 48.0f;
		UTIL_TraceHull( pPlayer->pev->origin, pPlayer->pev->origin + away,
			dont_ignore_monsters, human_hull, pPlayer->edict(), &tr );
		if( !tr.fStartSolid )
		{
			pPlayer->pev->origin = tr.vecEndPos;
			UTIL_SetOrigin( pPlayer->pev, tr.vecEndPos );
		}
	}
	pBest->Use( pPlayer, pPlayer, USE_TOGGLE, 1 );
	return 1;
}

void EFW_PlayerPreThink( CBasePlayer *pPlayer )
{
	static int s_preN;
	if( !pPlayer )
		return;
	s_preN++;
	if( s_preN <= 8 || ( s_preN % 60 ) == 1 )
	{
		char line[128];
		snprintf( line, sizeof( line ),
			"efw: PreThink n=%d live=%d btn=%d vel=%.0f origin=%.0f %.0f %.0f yaw=%.0f pitch=%.0f\n",
			s_preN, s_liveTicks, pPlayer->pev->button, pPlayer->pev->velocity.Length(),
			pPlayer->pev->origin.x, pPlayer->pev->origin.y, pPlayer->pev->origin.z,
			pPlayer->pev->v_angle.y, pPlayer->pev->v_angle.x );
		EFW_LogLine( line );
	}
	if( ( pPlayer->pev->button & ( IN_JUMP | IN_DUCK ) ) || fabs( pPlayer->pev->velocity.z ) > 80.0f )
	{
		static int s_airN;
		char line[128];
		if( s_airN < 48 )
		{
			s_airN++;
			snprintf( line, sizeof( line ),
				"efw: air n=%d btn=%d velz=%.0f z=%.1f viewz=%.0f flags=%d\n",
				s_airN, pPlayer->pev->button, pPlayer->pev->velocity.z,
				pPlayer->pev->origin.z, pPlayer->pev->view_ofs.z, pPlayer->pev->flags );
			EFW_LogLine( line );
		}
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
	/* DAT_10114740 starts at 1. The first player think on level 1
	   (FUN_100c5b80 returns 0) sends menu 0x49 and clears the byte.
	   Level 2 and 3 leave it set. The pawn has to be live first so
	   the client has hooked EFW_Menu. */
	{
		static int s_introFlag = 1;
		if( s_introFlag && EFW_MapLevel() == 0 )
		{
			s_introFlag = 0;
			EFW_DebugPrint( ">>> FUN_1007db60 intro=0x49" );
			EFW_FailOrNarrate( pPlayer, 0x49 );
		}
	}
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
		if( !hit )
			hit = EFW_UseNearbyDoor( pPlayer );
		if( hit )
			pPlayer->m_afButtonPressed &= ~IN_USE;
	}
	if( g_efw.lastTime != gpGlobals->time || !g_efw.inited )
		EFW_SendHudState();
}

#endif

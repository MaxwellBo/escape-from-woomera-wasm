#ifndef CLIENT_DLL

// Gate / pause / hide / electrician LOS recovered from EscapeFromWoomera.dll
// FUN_100c7da0, FUN_100c7510, FUN_100c59c0, ClientCommand hide/changelevel.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "efw_dll.h"

#include <string.h>
#include <stdio.h>

void EFW_UseNamed( const char *targetname, CBaseEntity *pActivator, CBaseEntity *pCaller, int useType, float value )
{
	CBaseEntity *pEnt;
	if( !targetname || !targetname[0] )
		return;
	pEnt = UTIL_FindEntityByTargetname( NULL, targetname );
	if( pEnt )
		pEnt->Use( pActivator, pCaller, (USE_TYPE)useType, value );
}

int EFW_ElectricianSees( CBasePlayer *pPlayer )
{
	CBaseEntity *pEl;
	Vector delta;
	float dist;
	float dot;

	if( !pPlayer )
		return 0;
	pEl = UTIL_FindEntityByTargetname( NULL, "efw_electrician" );
	if( !pEl )
		return 0;
	delta = pPlayer->pev->origin - pEl->pev->origin;
	dist = delta.Length();
	if( dist < 256.0f )
		return 1;
	if( dist < 1.0f )
		return 1;
	delta = delta * ( 1.0f / dist );
	UTIL_MakeVectors( pEl->pev->angles );
	dot = DotProduct( gpGlobals->v_forward, delta );
	/* 0.61086476 rad ~ 35 degrees, cos ≈ 0.819 */
	if( dot > 0.819f )
		return 1;
	return 0;
}

void EFW_SetPause( int on )
{
	CBasePlayer *pPlayer = EFW_Player();
	CBaseEntity *pGuard;
	EFW_SetHudInt( 6, on ? 1 : 0 );
	if( !pPlayer )
		return;
	pPlayer->pev->movetype = on ? MOVETYPE_NONE : MOVETYPE_WALK;
	/* FUN_100c74a0: freeze monster_patrol_guard while paused. */
	pGuard = NULL;
	while( ( pGuard = UTIL_FindEntityByClassname( pGuard, "monster_patrol_guard" ) ) != NULL )
	{
		pGuard->pev->framerate = on ? 0.0f : 1.0f;
		pGuard->pev->movetype = on ? MOVETYPE_NONE : MOVETYPE_STEP;
		pGuard->pev->nextthink = gpGlobals->time + 0.05f;
		UTIL_SetOrigin( pGuard->pev, pGuard->pev->origin );
	}
	if( on )
		EFW_DebugPrint( "efw_pause 1" );
}

static char s_queuedMap[64];
static int s_queuedChange;

void EFW_ChangeLevel( const char *map )
{
	if( !map || !map[0] )
		return;
	/* Classic Quake transition (no landmark). Skip save/restore and the
	   sv.framecount<15 infinite-changelevel lock so WASM Host can finish
	   STATE_CHANGELEVEL → SV_ExecChangeLevel on the next COM_Frame. */
	CVAR_SET_FLOAT( "sv_validate_changelevel", 0.0f );
	CVAR_SET_FLOAT( "sv_newunit", 1.0f );
	strncpy( s_queuedMap, map, sizeof( s_queuedMap ) - 1 );
	s_queuedMap[sizeof( s_queuedMap ) - 1] = '\0';
	s_queuedChange = 1;
	/* Queue until engine StartFrame (inside Host_Frame). HostFwd JS runs
	   between COM_Frames so COM_ChangeLevel's nextstate is lost before
	   Host_RunFrame can promote STATE_CHANGELEVEL. */
	EFW_DebugPrint( ">>> efw_changelevel queued %s time=%.2f validate=%.0f newunit=%.0f",
		map, gpGlobals->time,
		CVAR_GET_FLOAT( "sv_validate_changelevel" ),
		CVAR_GET_FLOAT( "sv_newunit" ) );
}

void EFW_RunQueuedChangeLevel( void )
{
	if( !s_queuedChange )
		return;
	s_queuedChange = 0;
	EFW_DebugPrint( ">>> CHANGE_LEVEL StartFrame %s time=%.2f", s_queuedMap, gpGlobals->time );
	/* PE ClientCommand: pfnChangeLevel. Xash COM_ChangeLevel sets
	   Host nextstate = STATE_CHANGELEVEL; Exec runs on the next COM_Frame. */
	CHANGE_LEVEL( s_queuedMap, NULL );
	EFW_DebugPrint( ">>> CHANGE_LEVEL returned %s", s_queuedMap );
}

void EFW_HideUnderBuilding( CBasePlayer *pPlayer )
{
	int level;

	if( !pPlayer )
		return;
	/* ClientCommand 0x1001b969: MapLevel, then HasKeyword / HasWeapon. */
	level = EFW_MapLevel();
	EFW_DebugPrint( ">>> HideUnder level=%d pliers=%d lever=%d PliersInBin=%d Gholan=%d",
		level,
		EFW_HasWeapon( pPlayer, "weapon_efw_Pliers" ),
		EFW_HasWeapon( pPlayer, "weapon_efw_Lever" ),
		EFW_HasKeyword( "PliersInBin" ),
		EFW_HasKeyword( "GholanAgreedToPloy" ) );
	if( level == 0 )
	{
		if( !EFW_HasKeyword( "PliersInBin" ) )
			EFW_FailOrNarrate( pPlayer, 0x40 );
		else if( !EFW_HasKeyword( "GholanAgreedToPloy" ) )
			EFW_FailOrNarrate( pPlayer, 0x41 );
		else if( !EFW_HasWeapon( pPlayer, "weapon_efw_Lever" ) )
			EFW_FailOrNarrate( pPlayer, 0x42 );
		else
			EFW_FailOrNarrate( pPlayer, 0x3f );
		return;
	}
	if( level == 1 )
	{
		if( !EFW_HasWeapon( pPlayer, "weapon_efw_Pliers" ) )
			EFW_FailOrNarrate( pPlayer, 0x44 );
		else
			EFW_FailOrNarrate( pPlayer, 0x43 );
	}
}

int EFW_FireTargets( const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, int useType, float value )
{
	CBasePlayer *pPlayer;
	(void)pCaller;
	if( !targetName || !targetName[0] )
		return 0;
	pPlayer = ( pActivator && pActivator->IsPlayer() ) ? (CBasePlayer *)pActivator : EFW_Player();
	if( !pPlayer || !pPlayer->IsPlayer() )
		return 0;

	if( EFW_FStrEq( targetName, "efw_kitchen_door" ) )
	{
		if( !EFW_HasWeapon( pPlayer, "weapon_efw_Pliers" ) )
		{
			if( !EFW_HasKeyword( "OnKitchenRoster" ) )
			{
				EFW_Squark( "Kitchen_Guard", "Where do you think you're going? You can't enter without a kitchen pass.", 4 );
				return 1;
			}
			if( EFW_MapLevel() == 0 )
			{
				if( !EFW_HasKeyword( "HasEnteredTheKitchenForTheFirstTime" ) )
				{
					EFW_AddKeyword( "HasEnteredTheKitchenForTheFirstTime", 1 );
					EFW_Squark( "Kitchen_Guard", "RAR-124? Go in, you're on the roster.", 4 );
				}
				EFW_UseNamed( "efw_actual_kitchen_door", pPlayer, pPlayer, useType, value );
				return 1;
			}
			if( EFW_MapLevel() == 2 )
			{
				if( !EFW_HasKeyword( "HasEnteredTheKitchenInChapterTwo" ) )
				{
					EFW_AddKeyword( "HasEnteredTheKitchenInChapterTwo", 1 );
					EFW_Squark( "Kitchen_Guard", "Back again? OK, go on in.", 4 );
					EFW_UseNamed( "efw_actual_kitchen_door", pPlayer, pPlayer, useType, value );
				}
			}
		}
		else
			EFW_FailOrNarrate( pPlayer, 0x3c );
		return 1;
	}

	if( EFW_FStrEq( targetName, "efw_2ndcompound_entry" ) )
	{
		if( !EFW_HasKeyword( "Player'sIDTagOnFence" ) )
		{
			if( !EFW_HasKeyword( "HeardCompoundGateGuardOnce" ) )
			{
				EFW_Squark( "efw_compound_gate_guard",
					"Hold it! To enter the main compound, leave your ID tag on the board so we know you're there. And don't forget to collect it again when your return.",
					4 );
				EFW_AddKeyword( "HeardCompoundGateGuardOnce", 1 );
			}
			else
			{
				EFW_Squark( "efw_compound_gate_guard",
					"No tag on the board, no entry. Leave your ID Tag on the board to enter.",
					4 );
			}
			return 1;
		}
		if( UTIL_FindEntityByTargetname( NULL, "efw_compound_gate" ) )
		{
			EFW_UseNamed( "efw_compound_gate", pPlayer, pPlayer, useType, value );
			EFW_Print( pPlayer, "efw: compound gate opens" );
		}
		return 1;
	}

	if( EFW_FStrEq( targetName, "efw_1stcompound_entry" ) )
	{
		if( !EFW_HasKeyword( "Player'sIDTagOnFence" ) )
		{
			EFW_UseNamed( "efw_1stcompound_gate", pPlayer, pPlayer, useType, value );
			EFW_Print( pPlayer, "efw: 1st compound gate opens" );
		}
		else
			EFW_Squark( "efw_compound_gate_guard",
				"The gate doesn't open til you collect your tag. No tag, no entry.",
				4 );
		return 1;
	}

	if( EFW_FStrEq( targetName, "efw_approach_bin" ) )
	{
		if( !EFW_HasWeapon( pPlayer, "weapon_efw_Pliers" ) )
		{
			EFW_GiveItem( pPlayer, EFW_ITEM_PLIERS, "weapon_efw_Pliers" );
			EFW_FailOrNarrate( pPlayer, 0x45 );
			EFW_AdjustHope( 15.0f );
		}
		return 1;
	}
	return 0;
}

/* FUN_100c7590 / 75c0 / 75e0 / 7670 / 7740 — camp PA + Dingaling. */
struct EfwCue
{
	const char *sample;
};

struct EfwPA
{
	EfwCue slot[5];
	float gap;       /* +0x50 init 120, unused by think */
	float timer;     /* +0x54 init 5 */
	float lastTime;  /* +0x58 */
	int index;       /* +0x5c */
	int rarLock;     /* +0x60 */
	float remainUntil;
	int inited;
};

static EfwPA g_pa;

void EFW_PlayCue( const char *sample )
{
	CBaseEntity *pSrc;
	edict_t *ed;
	int pitch;

	if( !sample || !sample[0] )
	{
		EFW_DebugPrint( "efw: play (null)" );
		return;
	}
	pSrc = UTIL_FindEntityByTargetname( NULL, sample );
	if( pSrc )
		ed = pSrc->edict();
	else if( EFW_Player() )
		ed = EFW_Player()->edict();
	else
		return;
	pitch = 100 - RANDOM_LONG( 0, 19 );
	EMIT_SOUND_DYN( ed, CHAN_ITEM, sample, 1.0f, 1.25f, 0, pitch );
	EFW_DebugPrint( "efw: play %s", sample );
	if( EFW_Player() )
	{
		char line[96];
		snprintf( line, sizeof( line ), "efw: play %s", sample );
		EFW_Print( EFW_Player(), line );
	}
}

void EFW_InitPA( void )
{
	int i;
	static const char *kAnn[] = {
		"Ann_RAR_124.wav",
		"Ann_HAM_103.wav",
		"Ann_TRE_046.wav",
		"Ann_PHA_216.wav",
		"callToPrayer.wav"
	};
	for( i = 0; i < 5; i++ )
	{
		g_pa.slot[i].sample = kAnn[i];
		PRECACHE_SOUND( (char *)kAnn[i] );
	}
	g_pa.gap = 120.0f;
	g_pa.timer = 5.0f;
	g_pa.lastTime = 0.0f;
	g_pa.index = 0;
	g_pa.rarLock = 0;
	g_pa.remainUntil = 0.0f;
	g_pa.inited = 1;
	PRECACHE_SOUND( "Dingaling.wav" );
}

void EFW_ThinkPA( void )
{
	float now;
	float remain;
	const char *sample;
	if( !g_pa.inited )
		EFW_InitPA();
	if( EFW_MapLevel() != 0 )
		return;
	now = gpGlobals->time;
	remain = g_pa.remainUntil - now;
	if( remain <= 0.5f && g_pa.timer > 5.0f )
	{
		if( g_pa.rarLock == 1 )
			sample = g_pa.slot[0].sample;
		else
			sample = g_pa.slot[g_pa.index + 1].sample;
		EFW_PlayCue( sample );
		g_pa.timer = 0.0f;
		g_pa.index++;
		if( g_pa.index > 3 )
			g_pa.index = 0;
		g_pa.remainUntil = now + ( sample && strstr( sample, "callToPrayer" ) ? 8.0f : 4.0f );
	}
	if( g_pa.lastTime > 0.0f )
		g_pa.timer += now - g_pa.lastTime;
	else if( now > 0.0f )
		g_pa.timer += now;
	g_pa.lastTime = now;
}

void EFW_PALockRAR( void )
{
	if( !g_pa.inited )
		EFW_InitPA();
	g_pa.rarLock = 1;
	EFW_PlayCue( g_pa.slot[0].sample );
	g_pa.timer = 0.0f;
}

void EFW_PAUnlock( void )
{
	g_pa.rarLock = 0;
}

#endif

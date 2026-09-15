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
	EFW_SetHudInt( 6, on ? 1 : 0 );
	if( !pPlayer )
		return;
	pPlayer->pev->movetype = on ? MOVETYPE_NONE : MOVETYPE_WALK;
	if( on )
		EFW_DebugPrint( "efw_pause 1" );
}

void EFW_ChangeLevel( const char *map )
{
	if( !map || !map[0] )
		return;
	EFW_DebugPrint( ">>> efw_changelevel %s", map );
	CHANGE_LEVEL( (char *)map, NULL );
}

void EFW_HideUnderBuilding( CBasePlayer *pPlayer )
{
	int level;
	int tag;
	int pliers;

	if( !pPlayer )
		return;
	level = EFW_MapLevel();
	tag = EFW_HasKeyword( "Player'sIDTagOnFence" );
	pliers = EFW_HasWeapon( pPlayer, "weapon_efw_Pliers" );
	if( level == 0 )
	{
		if( tag )
			EFW_FailOrNarrate( pPlayer, 0x3f );
		else
			EFW_FailOrNarrate( pPlayer, 0x40 );
		return;
	}
	if( pliers )
		EFW_FailOrNarrate( pPlayer, 0x43 );
	else
		EFW_FailOrNarrate( pPlayer, 0x42 );
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
			EFW_UseNamed( "efw_compound_gate", pPlayer, pPlayer, useType, value );
		return 1;
	}

	if( EFW_FStrEq( targetName, "efw_1stcompound_entry" ) )
	{
		if( !EFW_HasKeyword( "Player'sIDTagOnFence" ) )
			EFW_UseNamed( "efw_1stcompound_gate", pPlayer, pPlayer, useType, value );
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

#endif

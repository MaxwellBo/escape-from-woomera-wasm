#ifndef CLIENT_DLL

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "efw_dll.h"

#include <stdio.h>
#include <string.h>

static Vector EFW_Place( CBaseEntity *pEnt )
{
	Vector c;
	if( !pEnt )
		return g_vecZero;
	c = ( pEnt->pev->absmin + pEnt->pev->absmax ) * 0.5f;
	if( ( pEnt->pev->absmax - pEnt->pev->absmin ).Length() < 1.0f )
		c = pEnt->pev->origin;
	return c;
}

static CBaseEntity *EFW_NearestTalkNpc( CBasePlayer *pPlayer, float dist )
{
	CBaseEntity *pScan = NULL;
	CBaseEntity *pBest = NULL;
	float best = dist;
	if( !pPlayer )
		return NULL;
	while( ( pScan = UTIL_FindEntityInSphere( pScan, pPlayer->pev->origin, dist ) ) != NULL )
	{
		float d;
		if( pScan == pPlayer || !EFW_IsTalkNpc( pScan ) )
			continue;
		d = ( pScan->pev->origin - pPlayer->pev->origin ).Length();
		if( d < best )
		{
			best = d;
			pBest = pScan;
		}
	}
	return pBest;
}

static CBaseEntity *EFW_NearestMarker( CBasePlayer *pPlayer, float dist )
{
	CBaseEntity *pScan = NULL;
	CBaseEntity *pBest = NULL;
	float best = dist;
	if( !pPlayer )
		return NULL;
	while( ( pScan = UTIL_FindEntityByClassname( pScan, "efw_Marker" ) ) != NULL )
	{
		float d = ( EFW_Place( pScan ) - pPlayer->pev->origin ).Length();
		if( d < best )
		{
			best = d;
			pBest = pScan;
		}
	}
	return pBest;
}

CBaseEntity *EFW_AimEntity( CBasePlayer *pPlayer, float dist )
{
	TraceResult tr;
	CBaseEntity *pHit;
	CBaseEntity *pScan;
	CBaseEntity *pBest;
	float best;

	if( !pPlayer )
		return NULL;
	UTIL_MakeVectors( pPlayer->pev->v_angle );
	UTIL_TraceLine( pPlayer->pev->origin + pPlayer->pev->view_ofs,
		pPlayer->pev->origin + pPlayer->pev->view_ofs + gpGlobals->v_forward * dist,
		dont_ignore_monsters, pPlayer->edict(), &tr );
	if( tr.flFraction < 1.0f && tr.pHit )
	{
		pHit = CBaseEntity::Instance( tr.pHit );
		if( pHit && pHit != pPlayer )
			return pHit;
	}
	pBest = NULL;
	best = dist;
	pScan = NULL;
	while( ( pScan = UTIL_FindEntityInSphere( pScan, pPlayer->pev->origin, dist ) ) != NULL )
	{
		const char *cn;
		float d;
		if( pScan == pPlayer )
			continue;
		cn = STRING( pScan->pev->classname );
		if( strncmp( cn, "monster_", 8 ) && strcmp( cn, "efw_Marker" ) && strncmp( cn, "weapon_efw", 10 ) )
			continue;
		d = ( EFW_Place( pScan ) - ( pPlayer->pev->origin + pPlayer->pev->view_ofs ) ).Length();
		if( d < best )
		{
			best = d;
			pBest = pScan;
		}
	}
	if( !pBest )
		pBest = EFW_NearestMarker( pPlayer, dist );
	return pBest;
}

void EFW_GiveToNpc( CBasePlayer *pPlayer, CBaseEntity *pNpc )
{
	EfwDllState *st = EFW_Dll();
	if( !pPlayer || !pNpc )
		return;
	if( st->items & EFW_ITEM_PLIERS )
	{
		st->items &= ~EFW_ITEM_PLIERS;
		EFW_AddKeyword( "PLIERS_GOT_PLIERS", 1 );
		EFW_Print( pPlayer, "You hand over the pliers." );
		EFW_StartTalk( pPlayer, pNpc );
	}
}

void EFW_UseMarker( CBasePlayer *pPlayer, CBaseEntity *pMarker )
{
	const char *name;
	EfwDllState *st = EFW_Dll();
	if( !pPlayer || !pMarker )
		return;
	EFW_CloseTalk();
	name = STRING( pMarker->pev->targetname );

	if( !strcmp( name, "efw_PliersMarker" ) )
	{
		if( !( st->items & EFW_ITEM_PLIERS ) )
		{
			EFW_GiveItem( pPlayer, EFW_ITEM_PLIERS, "weapon_efw_Pliers" );
			EFW_AddKeyword( "PLIERS_GOT_PLIERS", 1 );
		}
		EFW_ShowGoldMenu( pPlayer, 0, 12,
			"You wait until the electrician is not looking, and quickly grab the pliers from the workbench. He doesn't notice, and you hide them under your shirt. Heart pounding, you wonder how to safely get them to Amir." );
		return;
	}
	if( !strcmp( name, "efw_kitchen_bin" ) )
	{
		if( st->items & EFW_ITEM_PLIERS )
		{
			st->items &= ~EFW_ITEM_PLIERS;
			EFW_ShowGoldMenu( pPlayer, 0, 12,
				"Again, you wait for the ideal moment to retrieve the pliers from under your shirt and slowly lower them into the bin, careful to not make a sound." );
		}
		else
		{
			EFW_GiveItem( pPlayer, EFW_ITEM_PLIERS, "weapon_efw_Pliers" );
			EFW_ShowGoldMenu( pPlayer, 0, 12,
				"You recognise the bin in front of you as the one from the kitchen earlier today. You open the top and dig around inside, and sure enough, the pliers are still there. You retrieve them from the foodscraps and rubbish, and hide them in your clothes." );
		}
		return;
	}
	if( !strcmp( name, "efw_hiding_place" ) )
	{
		if( st->items & EFW_ITEM_PLIERS )
			EFW_ShowGoldMenu( pPlayer, 0, 10,
				"You return to the hiding place, with the pliers safely tucked away underneath your shirt." );
		else
			EFW_ShowGoldMenu( pPlayer, 0, 10,
				"You could hide again, but you haven't got the pliers yet." );
		return;
	}
	if( !strcmp( name, "efw_IDTag_Position" ) )
	{
		EFW_GiveItem( pPlayer, EFW_ITEM_IDTAG, "weapon_efw_IDTag" );
		return;
	}
}

void EFW_Spider( CBasePlayer *pPlayer )
{
	CBaseEntity *pMark;
	if( !pPlayer )
		return;
	pMark = EFW_NearestMarker( pPlayer, 140.0f );
	if( pMark )
		EFW_UseMarker( pPlayer, pMark );
}

static void EFW_ToggleDiary( void )
{
	int open = EFW_GetHudInt( 5 ) ? 0 : 1;
	EFW_SetHudInt( 5, open );
	EFW_SendEfwData();
}

static void EFW_StepDiary( int dir )
{
	int cursor = EFW_GetHudInt( 0 ) + dir;
	int n = EFW_DiaryCount();
	if( n <= 0 )
		return;
	if( cursor < 0 )
		cursor = n - 1;
	if( cursor >= n )
		cursor = 0;
	EFW_SetHudInt( 0, cursor );
	EFW_SendEfwData();
}

int EFW_ClientCommand( edict_t *pEntity )
{
	CBasePlayer *pPlayer;
	const char *pcmd;
	int arg0 = 0;

	if( !pEntity || !pEntity->pvPrivateData )
		return 0;
	pPlayer = GetClassPtr( (CBasePlayer *)&pEntity->v );
	pcmd = CMD_ARGV( 0 );
	if( !pcmd )
		return 0;
	if( FStrEq( pcmd, "cmd" ) && CMD_ARGC() > 1 )
	{
		arg0 = 1;
		pcmd = CMD_ARGV( 1 );
	}

	if( FStrEq( pcmd, "menuselect" ) )
	{
		int slot = atoi( CMD_ARGV( arg0 + 1 ) );
		if( EFW_Dll()->talkActive )
			EFW_ChooseTalk( pPlayer, slot );
		return 1;
	}
	if( FStrEq( pcmd, "efw_Talk" ) )
	{
		CBaseEntity *pEnt = NULL;
		if( CMD_ARGC() > arg0 + 1 )
			pEnt = UTIL_FindEntityByTargetname( NULL, CMD_ARGV( arg0 + 1 ) );
		if( !pEnt )
			pEnt = EFW_AimEntity( pPlayer, 384.0f );
		if( !pEnt || !EFW_IsTalkNpc( pEnt ) )
			pEnt = EFW_NearestTalkNpc( pPlayer, 384.0f );
		if( pEnt && EFW_IsTalkNpc( pEnt ) )
			EFW_StartTalk( pPlayer, pEnt );
		else
			EFW_DebugPrint( ">>> efw_Talk (not found)" );
		return 1;
	}
	if( FStrEq( pcmd, "efw_Give" ) )
	{
		CBaseEntity *pEnt = EFW_AimEntity( pPlayer, 160.0f );
		if( CMD_ARGC() > arg0 + 1 )
		{
			CBaseEntity *named = UTIL_FindEntityByTargetname( NULL, CMD_ARGV( arg0 + 1 ) );
			if( named )
				pEnt = named;
		}
		if( !pEnt || !EFW_IsTalkNpc( pEnt ) )
			pEnt = EFW_NearestTalkNpc( pPlayer, 160.0f );
		if( pEnt && EFW_IsTalkNpc( pEnt ) )
			EFW_GiveToNpc( pPlayer, pEnt );
		return 1;
	}
	if( FStrEq( pcmd, "efw_spider" ) || FStrEq( pcmd, "efw_Pickup" ) )
	{
		EFW_Spider( pPlayer );
		return 1;
	}
	if( FStrEq( pcmd, "efw_UseWithMarker" ) )
	{
		CBaseEntity *pEnt = EFW_AimEntity( pPlayer, 128.0f );
		if( pEnt && !strcmp( STRING( pEnt->pev->classname ), "efw_Marker" ) )
			EFW_UseMarker( pPlayer, pEnt );
		return 1;
	}
	if( FStrEq( pcmd, "efw_diary" ) )
	{
		EFW_ToggleDiary();
		return 1;
	}
	if( FStrEq( pcmd, "efw_diary_next" ) )
	{
		EFW_StepDiary( 1 );
		return 1;
	}
	if( FStrEq( pcmd, "efw_diary_prev" ) )
	{
		EFW_StepDiary( -1 );
		return 1;
	}
	if( FStrEq( pcmd, "efw_ShowMenu" ) )
	{
		int code = atoi( CMD_ARGV( arg0 + 1 ) );
		if( code )
			EFW_FailOrNarrate( pPlayer, code );
		return 1;
	}
	if( FStrEq( pcmd, "efw_HelpScreen" ) )
	{
		EFW_FailOrNarrate( pPlayer, 0x47 );
		return 1;
	}
	if( FStrEq( pcmd, "efw_HideUnderBuilding" ) )
		return 1;
	if( FStrEq( pcmd, "efw_PickupPliers" ) )
	{
		EFW_Squark( "efw_electrician" );
		EFW_GiveItem( pPlayer, EFW_ITEM_PLIERS, "weapon_efw_Pliers" );
		return 1;
	}
	if( FStrEq( pcmd, "efw_pause" ) )
	{
		int on = EFW_GetHudInt( 6 ) ? 0 : 1;
		EFW_SetHudInt( 6, on );
		return 1;
	}
	if( FStrEq( pcmd, "efw_set_state" ) )
	{
		if( CMD_ARGC() > arg0 + 1 )
			EFW_AddKeyword( CMD_ARGV( arg0 + 1 ), 1 );
		return 1;
	}
	if( FStrEq( pcmd, "efw_changelevel" ) )
		return 1;
	return 0;
}

#endif

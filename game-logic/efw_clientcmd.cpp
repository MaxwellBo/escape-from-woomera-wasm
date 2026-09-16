#ifndef CLIENT_DLL

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "efw_dll.h"

#include <stdio.h>
#include <string.h>

static void EFW_Relocate( CBasePlayer *pPlayer, const Vector &pos )
{
	CBaseEntity *pScan;
	char line[96];

	if( !pPlayer )
		return;
	UTIL_SetOrigin( pPlayer->pev, pos );
	pPlayer->pev->velocity = g_vecZero;
	pPlayer->pev->flags &= ~FL_ONGROUND;
	snprintf( line, sizeof( line ), "efw: setpos %.0f %.0f %.0f", pos.x, pos.y, pos.z );
	EFW_Print( pPlayer, line );
	pScan = NULL;
	while( ( pScan = UTIL_FindEntityInSphere( pScan, pos, 96.0f ) ) != NULL )
	{
		const char *cn;
		if( pScan == pPlayer )
			continue;
		cn = STRING( pScan->pev->classname );
		if( !strncmp( cn, "trigger_", 8 ) )
			pScan->Touch( pPlayer );
	}
}

static CBaseEntity *EFW_FindGoto( const char *name )
{
	CBaseEntity *pEnt;
	if( !name || !name[0] )
		return NULL;
	pEnt = UTIL_FindEntityByTargetname( NULL, name );
	if( pEnt )
		return pEnt;
	pEnt = NULL;
	while( ( pEnt = UTIL_FindEntityByClassname( pEnt, "trigger_multiple" ) ) != NULL )
	{
		if( EFW_FStrEq( STRING( pEnt->pev->target ), name ) )
			return pEnt;
	}
	return NULL;
}

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
	const char *tn;
	if( !pPlayer || !pNpc )
		return;
	tn = STRING( pNpc->pev->targetname );
	if( EFW_FStrEq( tn, "Amir" ) )
	{
		CBasePlayerItem *pItem;
		int slot;
		st->items &= ~EFW_ITEM_PLIERS;
		for( slot = 0; slot < MAX_ITEM_TYPES; slot++ )
		{
			for( pItem = pPlayer->m_rgpPlayerItems[slot]; pItem; )
			{
				CBasePlayerItem *pNext = pItem->m_pNext;
				if( !strcmp( STRING( pItem->pev->classname ), "weapon_efw_Pliers" )
					|| !strcmp( STRING( pItem->pev->classname ), "weapon_efw_Pilers" ) )
					pPlayer->RemovePlayerItem( pItem, true );
				pItem = pNext;
			}
		}
		EFW_Squark( "Amir", "Well done! Your bravery and cleverness have helped bring us all one step closer to freedom!", 10 );
		EFW_FlagDiary( 2 ); /* FUN_100c6910(2) */
		EFW_FailOrNarrate( pPlayer, 0x4c );
		return;
	}
	if( EFW_FStrEq( tn, "Fashid" ) )
	{
		EFW_Squark( "Fashid", "Thanks, but I don't want them. A word of warning though, my friend. The guard outside often searches us, so you should find a way to hide them or smuggle them out.", 8 );
		return;
	}
	if( EFW_FStrEq( tn, "Nasir" ) )
	{
		EFW_Squark( "Nasir", "Well done, but you'll have to hide them in here somewhere, or the guard will find them when you leave the kitchen.", 8 );
		return;
	}
	if( EFW_FStrEq( tn, "Mouhtaz" ) )
	{
		EFW_Squark( "Mouhtaz", "Are you crazy? Whatever you do, don't try to leave with them. The guard outside may search you!", 8 );
		return;
	}
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
		int had;
		if( EFW_ElectricianSees( pPlayer ) )
		{
			EFW_Squark( "efw_electrician", "Dammit! Electrician saw you, can't put pliers in bin.", 4 );
			return;
		}
		had = EFW_HasWeapon( pPlayer, "weapon_efw_Pliers" );
		if( !had )
			EFW_GiveItem( pPlayer, EFW_ITEM_PLIERS, "weapon_efw_Pliers" );
		if( EFW_MapLevel() == 0 )
		{
			if( had )
				EFW_FailOrNarrate( pPlayer, 0x3d );
		}
		else
			EFW_ShowGoldMenu( pPlayer, 0, 12,
				"You wait until the electrician is not looking, and quickly grab the pliers from the workbench. He doesn't notice, and you hide them under your shirt. Heart pounding, you wonder how to safely get them to Amir." );
		return;
	}
	if( !strcmp( name, "efw_kitchen_bin" ) )
	{
		if( st->items & EFW_ITEM_PLIERS )
		{
			st->items &= ~EFW_ITEM_PLIERS;
			EFW_FailOrNarrate( pPlayer, 0x3e );
		}
		else
		{
			EFW_GiveItem( pPlayer, EFW_ITEM_PLIERS, "weapon_efw_Pliers" );
			EFW_FailOrNarrate( pPlayer, 0x44 );
		}
		return;
	}
	if( !strcmp( name, "efw_hiding_place" ) )
	{
		EFW_HideUnderBuilding( pPlayer );
		return;
	}
	if( !strcmp( name, "efw_IDTag_Position" ) )
	{
		/* FUN_100c2a20 place / FUN_100c29f0 collect. */
		if( EFW_HasWeapon( pPlayer, "weapon_efw_IDTag" ) )
		{
			EFW_StripWeapon( pPlayer, "weapon_efw_IDTag", EFW_ITEM_IDTAG );
			EFW_AddKeyword( "Player'sIDTagOnFence", 1 );
			EFW_Print( pPlayer, "ID Tag has been placed on the wall" );
			EFW_Squark( "efw_compound_gate_guard", "Okay RAR-124, you can pass.", 4 );
		}
		else if( EFW_HasKeyword( "Player'sIDTagOnFence" ) )
		{
			EFW_GiveItem( pPlayer, EFW_ITEM_IDTAG, "weapon_efw_IDTag" );
			EFW_AddKeyword( "Player'sIDTagOnFence", 0 );
			EFW_Print( pPlayer, "You just picked up the IDTag." );
		}
		return;
	}
	if( !strcmp( name, "efw_cage_door" ) )
	{
		if( EFW_HasWeapon( pPlayer, "weapon_efw_Lever" ) )
			EFW_UseNamed( "efw_cage_door", pPlayer, pPlayer, USE_TOGGLE, 0 );
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
	CBasePlayer *pPlayer = EFW_Player();
	EFW_SetHudInt( 5, open );
	EFW_DebugPrint( ">>> efw_diary open=%d page=%d", open, EFW_GetHudInt( 1 ) );
	if( pPlayer )
		EFW_Print( pPlayer, open ? "Diary opened" : "Diary closed" );
	EFW_SendHudState();
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

static const char *EFW_CmdName( int arg0 )
{
	int argc = CMD_ARGC();
	const char *a;
	if( argc > arg0 + 2 )
		return CMD_ARGV( arg0 + 2 );
	if( argc <= arg0 + 1 )
		return NULL;
	a = CMD_ARGV( arg0 + 1 );
	if( a && a[0] && ( a[0] < '0' || a[0] > '9' ) )
		return a;
	return NULL;
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
		EFW_DebugPrint( ">>> ClientCommand menuselect %d (talk=%d)", slot, EFW_Dll()->talkActive );
		if( EFW_Dll()->talkActive )
			EFW_ChooseTalk( pPlayer, slot );
		return 1;
	}
	if( FStrEq( pcmd, "efw_Talk" ) )
	{
		CBaseEntity *pEnt = NULL;
		const char *who = EFW_CmdName( arg0 );
		if( who && who[0] )
			pEnt = UTIL_FindEntityByTargetname( NULL, who );
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
		CBaseEntity *pEnt = NULL;
		const char *who = EFW_CmdName( arg0 );
		if( who && who[0] )
			pEnt = UTIL_FindEntityByTargetname( NULL, who );
		if( !pEnt )
			pEnt = EFW_AimEntity( pPlayer, 160.0f );
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
		CBaseEntity *pEnt = NULL;
		const char *who = EFW_CmdName( arg0 );
		if( who && who[0] )
			pEnt = UTIL_FindEntityByTargetname( NULL, who );
		if( !pEnt )
			pEnt = EFW_AimEntity( pPlayer, 128.0f );
		if( pEnt && EFW_FStrEq( STRING( pEnt->pev->targetname ), "efw_cage_door" ) )
		{
			EFW_UseMarker( pPlayer, pEnt );
			return 1;
		}
		if( !pEnt || strcmp( STRING( pEnt->pev->classname ), "efw_Marker" ) )
			pEnt = EFW_NearestMarker( pPlayer, 140.0f );
		if( pEnt )
			EFW_UseMarker( pPlayer, pEnt );
		return 1;
	}
	if( FStrEq( pcmd, "efw_setpos" ) || FStrEq( pcmd, "setpos" ) )
	{
		if( CMD_ARGC() > arg0 + 3 )
		{
			Vector pos;
			pos.x = (float)atof( CMD_ARGV( arg0 + 1 ) );
			pos.y = (float)atof( CMD_ARGV( arg0 + 2 ) );
			pos.z = (float)atof( CMD_ARGV( arg0 + 3 ) );
			EFW_Relocate( pPlayer, pos );
		}
		else if( CMD_ARGC() > arg0 + 1 )
		{
			CBaseEntity *pEnt = EFW_FindGoto( CMD_ARGV( arg0 + 1 ) );
			if( pEnt )
				EFW_Relocate( pPlayer, EFW_Place( pEnt ) );
			else
				EFW_DebugPrint( ">>> efw_setpos (not found) %s", CMD_ARGV( arg0 + 1 ) );
		}
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
	{
		EFW_HideUnderBuilding( pPlayer );
		return 1;
	}
	if( FStrEq( pcmd, "efw_PickupPliers" ) )
	{
		if( EFW_ElectricianSees( pPlayer ) )
			EFW_Squark( "efw_electrician", "Dammit! Electrician saw you, can't put pliers in bin.", 4 );
		else
			EFW_GiveItem( pPlayer, EFW_ITEM_PLIERS, "weapon_efw_Pliers" );
		return 1;
	}
	if( FStrEq( pcmd, "efw_pause" ) )
	{
		int on = 1;
		if( CMD_ARGC() > arg0 + 1 )
			on = atoi( CMD_ARGV( arg0 + 1 ) ) != 0;
		else
			on = EFW_GetHudInt( 6 ) ? 0 : 1;
		EFW_SetPause( on );
		return 1;
	}
	if( FStrEq( pcmd, "efw_set_state" ) )
	{
		if( CMD_ARGC() > arg0 + 1 )
			EFW_AddKeyword( CMD_ARGV( arg0 + 1 ), 1 );
		return 1;
	}
	if( FStrEq( pcmd, "efw_changelevel" ) )
	{
		if( CMD_ARGC() > arg0 + 1 )
			EFW_ChangeLevel( CMD_ARGV( arg0 + 1 ) );
		return 1;
	}
	if( FStrEq( pcmd, "efw_GetPackage" )
		|| FStrEq( pcmd, "efw_EndMailPickupMessage" )
		|| FStrEq( pcmd, "efw_TriggerMailPickupMessage" ) )
	{
		EFW_ServerCommand( pPlayer, pcmd );
		return 1;
	}
	return 0;
}

#endif

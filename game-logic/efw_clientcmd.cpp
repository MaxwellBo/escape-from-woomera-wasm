#ifndef CLIENT_DLL

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "efw_dll.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

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

/* FUN_100c4af0 vtable+0x114: weapons AddToPlayer (0x100c4690);
   CBaseEntity default is ret 4. Marker classname branch is unfinished in
   the PE (stub); call EFW_UseMarker so the recovered path is playable. */
static int EFW_LookUse114( CBaseEntity *pEnt, CBasePlayer *pPlayer )
{
	const char *cn;

	if( !pEnt || !pPlayer )
		return 0;
	cn = STRING( pEnt->pev->classname );
	if( !strncmp( cn, "weapon_", 7 ) )
	{
		if( pEnt->pev->owner )
			return 0;
		ALERT( at_error, "efw: look-use pickup %s\n", cn );
		return ( (CBasePlayerItem *)pEnt )->AddToPlayer( pPlayer ) ? 1 : 0;
	}
	if( EFW_FStrEq( cn, "efw_Marker" ) )
	{
		ALERT( at_error, "efw: look-use marker %s\n", STRING( pEnt->pev->targetname ) );
		EFW_UseMarker( pPlayer, pEnt, 0 );
		return 1;
	}
	return 0;
}

int EFW_LookUse( CBasePlayer *pPlayer )
{
	/* FUN_100c4af0: cdecl player look-use. Sphere 96 from EyePosition,
	   acos(dot) < 0.17453278 (~10°), TraceLine fraction 0.97,
	   efw_Marker classname @ 0x1011c9ac, vtable+0x114.
	   Brush markers have origin 0; aim/trace uses abs-center so the
	   recovered cone points at the bmodel the player is looking at. */
	Vector eye;
	CBaseEntity *pEnt;
	TraceResult tr;
	CBaseEntity *pHit;
	Vector dir;
	Vector dest;
	float len;
	float ang;
	float dot;

	if( !pPlayer )
		return 0;
	UTIL_MakeVectors( pPlayer->pev->v_angle );
	eye = pPlayer->EyePosition();
	pEnt = NULL;
	while( ( pEnt = UTIL_FindEntityInSphere( pEnt, eye, 96.0f ) ) != NULL )
	{
		if( pEnt == pPlayer )
			continue;
		dest = EFW_Place( pEnt );
		dir.x = dest.x - eye.x;
		dir.y = dest.y - eye.y;
		dir.z = dest.z - eye.z;
		len = dir.Length();
		if( len == 0.0f )
			dir = Vector( 0.0f, 0.0f, 1.0f );
		else
			dir = dir * ( 1.0f / len );
		dot = DotProduct( dir, gpGlobals->v_forward );
		if( dot > 1.0f )
			dot = 1.0f;
		if( dot < -1.0f )
			dot = -1.0f;
		ang = (float)acos( (double)dot );
		if( ang >= 0.17453278f )
			continue;
		UTIL_TraceLine( eye, dest, dont_ignore_monsters, pPlayer->edict(), &tr );
		if( tr.flFraction >= 0.97f )
		{
			if( EFW_LookUse114( pEnt, pPlayer ) )
				return 1;
			continue;
		}
		pHit = ( tr.pHit ) ? CBaseEntity::Instance( tr.pHit ) : NULL;
		if( pHit && EFW_FStrEq( STRING( pHit->pev->classname ), "efw_Marker" ) )
		{
			if( EFW_LookUse114( pHit, pPlayer ) )
				return 1;
		}
	}
	return 0;
}

void EFW_GiveToNpc( CBasePlayer *pPlayer, CBaseEntity *pNpc, int weaponId )
{
	const char *tn;
	if( !pPlayer || !pNpc )
		return;
	tn = STRING( pNpc->pev->targetname );
	if( weaponId <= 0 && pPlayer->m_pActiveItem )
		weaponId = pPlayer->m_pActiveItem->m_iId;
	if( weaponId <= 0 )
		return;

	/* FUN_100c5240: MobilePhone Give virtual. Else FUN_100c4550. */
	if( weaponId == WEAPON_EFW_MOBILEPHONE )
	{
		if( EFW_FStrEq( tn, "Gholan" ) && EFW_HasKeyword( "GotHintAboutHiding" ) )
		{
			EFW_StripWeapon( pPlayer, "weapon_efw_MobilePhone", EFW_ITEM_PHONE );
			EFW_Squark( "Gholan",
				"Well done, my friend. When you next go to the main compound, rest assured I'll remove the tag. You'll be free to hide then.",
				0 );
			EFW_AddKeyword( "GholanAgreedToPloy", 1 );
			EFW_AddKeyword( "IDTAG", 0 );
			EFW_AddKeyword( "HIDING", 0 );
			EFW_AdjustHope( 10.0f );
			EFW_AddDiary( 13, 1 );
			return;
		}
		EFW_GiveUnwanted( pPlayer, pNpc );
		return;
	}

	/* FUN_100c5330: WashingPowder Give virtual. Else FUN_100c4550. */
	if( weaponId == WEAPON_EFW_WASHINGPOWDER )
	{
		if( EFW_FStrEq( tn, "Mouhtaz" ) )
		{
			EFW_StripWeapon( pPlayer, "weapon_efw_WashingPowder", EFW_ITEM_POWDER );
			EFW_Squark( "Mouhtaz",
				"Mustafa, this is far too kind of you! The only thing I can offer you in exchange is this length of metal pipe I have found and have been hiding. Perhaps you can find a use for it?",
				2 );
			EFW_GiveItem( pPlayer, EFW_ITEM_LEVER, "weapon_efw_Lever" );
			EFW_AddDiary( 17, 1 );
			EFW_AdjustHope( 15.0f );
			return;
		}
		EFW_GiveUnwanted( pPlayer, pNpc );
		return;
	}

	/* FUN_100c4e30: Pliers Give virtual. Unmatched NPC → FUN_100c4550. */
	if( weaponId == WEAPON_EFW_PLIERS )
	{
		if( EFW_FStrEq( tn, "Amir" ) )
		{
			EFW_StripWeapon( pPlayer, "weapon_efw_Pliers", EFW_ITEM_PLIERS );
			EFW_StripWeapon( pPlayer, "weapon_efw_Pilers", EFW_ITEM_PILERS );
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
		EFW_GiveUnwanted( pPlayer, pNpc );
		return;
	}

	/* Lever / Branch / IDTag / *PhoneCard default Give = FUN_100c4550. */
	EFW_GiveUnwanted( pPlayer, pNpc );
}

void EFW_UseMarker( CBasePlayer *pPlayer, CBaseEntity *pMarker, int weaponId )
{
	const char *name;
	static int electricianFails;
	if( !pPlayer || !pMarker )
		return;
	if( weaponId <= 0 && pPlayer->m_pActiveItem )
		weaponId = pPlayer->m_pActiveItem->m_iId;
	EFW_CloseTalk();
	name = STRING( pMarker->pev->targetname );

	if( !strcmp( name, "efw_PliersMarker" ) )
	{
		int had;
		if( EFW_ElectricianSees( pPlayer ) )
		{
			electricianFails++;
			EFW_DebugPrint( "Dammit! Electrician saw you, can't put pliers in bin. Fail count: %i.", electricianFails );
			EFW_Squark( "efw_electrician", "Oi! Put that back!", 10 );
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
		/* FUN_100c4f90: pliers UseWithMarker on efw_kitchen_bin. */
		if( weaponId != WEAPON_EFW_PLIERS && !EFW_HasWeapon( pPlayer, "weapon_efw_Pliers" ) )
			return;
		if( EFW_ElectricianSees( pPlayer ) )
		{
			electricianFails++;
			EFW_DebugPrint( "Dammit! Electrician saw you, can't put pliers in bin. Fail count: %i.", electricianFails );
			EFW_Squark( "efw_electrician", "Oi! Put that back!", 10 );
			EFW_AdjustHope( -2.0f ); /* FUN_100c4d10(2.0) */
			return;
		}
		EFW_FailOrNarrate( pPlayer, 0x3e );
		EFW_StripWeapon( pPlayer, "weapon_efw_Pliers", EFW_ITEM_PLIERS );
		EFW_StripWeapon( pPlayer, "weapon_efw_Pilers", EFW_ITEM_PILERS );
		EFW_AddKeyword( "PliersInBin", 1 );
		EFW_AddKeyword( "PLIERS_GOT_PLIERS", 0 );
		EFW_AdjustHope( 10.0f );
		EFW_PALockRAR();
		EFW_AddKeyword( "OFFICE", 1 );
		EFW_AddDiary( 6, 0 );
		return;
	}
	if( !strcmp( name, "efw_hiding_place" ) )
	{
		EFW_HideUnderBuilding( pPlayer );
		return;
	}
	if( !strcmp( name, "efw_IDTag_Position" ) )
	{
		/* FUN_100c2a20 is the IDTag weapon UseWithMarker virtual (place only). */
		if( weaponId == WEAPON_EFW_IDTAG || EFW_HasWeapon( pPlayer, "weapon_efw_IDTag" ) )
			EFW_PlacePlayerIdTag( pPlayer, pMarker );
		return;
	}
	if( !strcmp( name, "efw_cage_door" ) )
	{
		/* FUN_100c50d0 lever / FUN_100c5180 branch. */
		if( weaponId == WEAPON_EFW_BRANCH )
		{
			EFW_Print( pPlayer, "Oh, you've broken the branch attempting to open the cage door! The door stays locked! Try something else." );
			EFW_StripWeapon( pPlayer, "weapon_efw_Branch", EFW_ITEM_BRANCH );
			EFW_GiveItem( pPlayer, EFW_ITEM_LEVER, "weapon_efw_Lever" );
			return;
		}
		if( weaponId == WEAPON_EFW_LEVER || EFW_HasWeapon( pPlayer, "weapon_efw_Lever" ) )
		{
			EFW_Print( pPlayer, "Good work; you've openned the cage door, by using the metal lever." );
			EFW_UseNamed( "efw_cage_door", pPlayer, pPlayer, USE_TOGGLE, 0 );
			EFW_StripWeapon( pPlayer, "weapon_efw_Lever", EFW_ITEM_LEVER );
		}
		return;
	}
}

void EFW_Spider( CBasePlayer *pPlayer )
{
	/* ClientCommand 0x1001b450: FUN_100c7820 (scanCount>0) else "Ignoring spider". */
	if( !pPlayer )
		return;
	if( EFW_Dll()->scanCount <= 0 )
	{
		EFW_DebugPrint( "Ignoring spider" );
		ALERT( at_error, "Ignoring spider\n" );
		if( g_engfuncs.pfnServerPrint )
			g_engfuncs.pfnServerPrint( "Ignoring spider\n" );
		return;
	}
	EFW_DebugPrint( ">>> efw_spider scan=%d FailOrNarrate 0x48", EFW_Dll()->scanCount );
	EFW_SendCntxt();
	EFW_FailOrNarrate( pPlayer, 0x48 );
}

void EFW_Pickup( CBasePlayer *pPlayer, const char *arg )
{
	CBaseEntity *pEnt = NULL;
	const char *cn = NULL;

	/* ClientCommand 0x1001b5d7: FindEntityByClassname(0, argv[1]), skip if owner set, Touch player.
	   Client HUD formats `efw_Pickup %u` with weapon id 16..24 (FUN_10044f70). */
	if( !pPlayer || !arg || !arg[0] )
		return;
	if( !strncmp( arg, "weapon_", 7 ) )
		cn = arg;
	else
		cn = EFW_WeaponClassname( atoi( arg ) );
	if( !cn )
		return;
	while( ( pEnt = UTIL_FindEntityByClassname( pEnt, cn ) ) != NULL )
	{
		if( !pEnt->pev->owner )
			break;
	}
	if( !pEnt || pEnt->pev->owner )
		return;
	pEnt->Touch( pPlayer );
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
		int wep = 0;
		if( CMD_ARGC() > arg0 + 1 )
		{
			const char *a = CMD_ARGV( arg0 + 1 );
			if( a && a[0] >= '0' && a[0] <= '9' )
				wep = atoi( a );
		}
		if( who && who[0] )
			pEnt = UTIL_FindEntityByTargetname( NULL, who );
		if( !pEnt )
			pEnt = EFW_AimEntity( pPlayer, 160.0f );
		if( !pEnt || !EFW_IsTalkNpc( pEnt ) )
			pEnt = EFW_NearestTalkNpc( pPlayer, 160.0f );
		if( pEnt && EFW_IsTalkNpc( pEnt ) )
			EFW_GiveToNpc( pPlayer, pEnt, wep );
		return 1;
	}
	if( FStrEq( pcmd, "efw_spider" ) )
	{
		EFW_Spider( pPlayer );
		return 1;
	}
	if( FStrEq( pcmd, "efw_lookuse" ) )
	{
		int hit;
		CBaseEntity *pMark;
		Vector dest;
		Vector back;
		Vector ang;
		float yaw;
		TraceResult tr;
		CBaseEntity *pHit;
		const char *hitCn;

		pMark = EFW_NearestMarker( pPlayer, 256.0f );
		if( pMark )
		{
			dest = EFW_Place( pMark );
			back = dest - pPlayer->pev->origin;
			back.z = 0;
			if( back.Length() < 8.0f )
				back = Vector( 40.0f, 0.0f, 0.0f );
			else
				back = back.Normalize() * 48.0f;
			/* Stand off the brush so TraceLine is not an inside-solid miss. */
			EFW_Relocate( pPlayer, dest - back + Vector( 0, 0, 8 ) );
			ang = UTIL_VecToAngles( dest - pPlayer->EyePosition() );
			pPlayer->pev->angles.y = ang.y;
			pPlayer->pev->v_angle.y = ang.y;
			pPlayer->pev->v_angle.x = -ang.x;
			pPlayer->pev->angles.x = 0;
			UTIL_MakeVectors( pPlayer->pev->v_angle );
			UTIL_TraceLine( pPlayer->EyePosition(), dest, dont_ignore_monsters,
				pPlayer->edict(), &tr );
			pHit = ( tr.pHit ) ? CBaseEntity::Instance( tr.pHit ) : NULL;
			hitCn = pHit ? STRING( pHit->pev->classname ) : "-";
			yaw = (float)acos( DotProduct(
				( dest - pPlayer->EyePosition() ).Normalize(),
				gpGlobals->v_forward ) );
			EFW_DebugPrint( ">>> look-use prep %s frac=%.2f hit=%s cone=%.3f",
				STRING( pMark->pev->targetname ), tr.flFraction, hitCn, yaw );
		}
		hit = EFW_LookUse( pPlayer );
		EFW_DebugPrint( ">>> efw_lookuse hit=%d", hit );
		return 1;
	}
	if( FStrEq( pcmd, "efw_Pickup" ) )
	{
		const char *arg = NULL;
		if( CMD_ARGC() > arg0 + 1 )
			arg = CMD_ARGV( arg0 + 1 );
		EFW_Pickup( pPlayer, arg );
		return 1;
	}
	if( FStrEq( pcmd, "efw_UseWithMarker" ) )
	{
		CBaseEntity *pEnt = NULL;
		const char *who = EFW_CmdName( arg0 );
		int wep = 0;
		if( CMD_ARGC() > arg0 + 1 )
		{
			const char *a = CMD_ARGV( arg0 + 1 );
			if( a && a[0] >= '0' && a[0] <= '9' )
				wep = atoi( a );
		}
		if( who && who[0] )
			pEnt = UTIL_FindEntityByTargetname( NULL, who );
		if( !pEnt )
			pEnt = EFW_AimEntity( pPlayer, 128.0f );
		if( pEnt && EFW_FStrEq( STRING( pEnt->pev->targetname ), "efw_cage_door" ) )
		{
			EFW_UseMarker( pPlayer, pEnt, wep );
			return 1;
		}
		if( !pEnt || strcmp( STRING( pEnt->pev->classname ), "efw_Marker" ) )
			pEnt = EFW_NearestMarker( pPlayer, 140.0f );
		if( pEnt )
			EFW_UseMarker( pPlayer, pEnt, wep );
		return 1;
	}
	if( FStrEq( pcmd, "efw_setpos" ) || FStrEq( pcmd, "setpos" ) )
	{
		if( CMD_ARGC() > arg0 + 4 )
		{
			CBaseEntity *pEnt;
			Vector pos;
			const char *who = CMD_ARGV( arg0 + 1 );
			pEnt = UTIL_FindEntityByTargetname( NULL, who );
			if( pEnt )
			{
				pos.x = (float)atof( CMD_ARGV( arg0 + 2 ) );
				pos.y = (float)atof( CMD_ARGV( arg0 + 3 ) );
				pos.z = (float)atof( CMD_ARGV( arg0 + 4 ) );
				UTIL_SetOrigin( pEnt->pev, pos );
				EFW_DebugPrint( ">>> efw_setpos %s %.0f %.0f %.0f", who, pos.x, pos.y, pos.z );
				return 1;
			}
		}
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
		EFW_FailOrNarrate( pPlayer, 0x52 ); /* ClientCommand 0x1001baab push 0x52 */
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
			EFW_Squark( "efw_electrician", "Oi! Put that back!", 10 );
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"
#include "efw.h"

#ifndef CLIENT_DLL
#include "efw_dll.h"
#endif

struct EfwWeaponDef
{
	const char *classname;
	int id;
	int slotPos;
	const char *vmodel;
	const char *wmodel;
	const char *pmodel;
	int itemBit;
};

static const EfwWeaponDef g_efwWeapons[] = {
	{ "weapon_efw_Pliers", WEAPON_EFW_PLIERS, 1, "models/v_pliers.mdl", "models/w_pliers.mdl", "models/p_pliers.mdl", EFW_ITEM_PLIERS },
	{ "weapon_efw_Pilers", WEAPON_EFW_PILERS, 1, "models/v_pilers.mdl", "models/w_pilers.mdl", "models/p_pilers.mdl", EFW_ITEM_PILERS },
	{ "weapon_efw_Lever", WEAPON_EFW_LEVER, 2, "models/v_lever.mdl", "models/w_lever.mdl", "models/p_lever.mdl", EFW_ITEM_LEVER },
	{ "weapon_efw_Branch", WEAPON_EFW_BRANCH, 3, "models/v_branch.mdl", "models/w_branch.mdl", "models/p_branch.mdl", EFW_ITEM_BRANCH },
	{ "weapon_efw_MobilePhone", WEAPON_EFW_MOBILEPHONE, 4, "models/v_mobilephone.mdl", "models/w_mobilephone.mdl", "models/p_mobilephone.mdl", EFW_ITEM_PHONE },
	{ "weapon_efw_IDTag", WEAPON_EFW_IDTAG, 5, "models/v_idtag.mdl", "models/w_idtag.mdl", "models/p_idtag.mdl", EFW_ITEM_IDTAG },
	{ "weapon_efw_RedPhoneCard", WEAPON_EFW_REDPHONECARD, 6, "models/v_redphonecard.mdl", "models/w_redphonecard.mdl", "models/p_redphonecard.mdl", EFW_ITEM_REDCARD },
	{ "weapon_efw_GreenPhoneCard", WEAPON_EFW_GREENPHONECARD, 7, "models/v_greenphonecard.mdl", "models/w_greenphonecard.mdl", "models/p_greenphonecard.mdl", EFW_ITEM_GREENCARD },
	{ "weapon_efw_BluePhoneCard", WEAPON_EFW_BLUEPHONECARD, 8, "models/v_bluephonecard.mdl", "models/w_bluephonecard.mdl", "models/p_bluephonecard.mdl", EFW_ITEM_BLUECARD },
	{ "weapon_efw_WashingPowder", WEAPON_EFW_WASHINGPOWDER, 9, "models/v_washingpowder.mdl", "models/w_washingpowder.mdl", "models/p_washingpowder.mdl", EFW_ITEM_POWDER },
};

static const EfwWeaponDef *EFW_FindDef( const char *classname )
{
	unsigned i;
	if( !classname )
		return &g_efwWeapons[0];
	for( i = 0; i < sizeof( g_efwWeapons ) / sizeof( g_efwWeapons[0] ); i++ )
	{
		if( !strcmp( classname, g_efwWeapons[i].classname ) )
			return &g_efwWeapons[i];
	}
	return &g_efwWeapons[0];
}

class CEfwWeapon : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int GetItemInfo( ItemInfo *p );
	int AddToPlayer( CBasePlayer *pPlayer );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	int iItemSlot( void ) { return 1; }
	BOOL ShouldWeaponIdle( void ) { return TRUE; }
	void WeaponIdle( void );
};

void CEfwWeapon::Spawn( void )
{
	const EfwWeaponDef *def = EFW_FindDef( STRING( pev->classname ) );
	Precache();
	m_iId = def->id;
	SET_MODEL( ENT( pev ), def->wmodel );
	m_iClip = -1;
	FallInit();
}

void CEfwWeapon::Precache( void )
{
	const EfwWeaponDef *def = EFW_FindDef( STRING( pev->classname ) );
	PRECACHE_MODEL( (char *)def->vmodel );
	PRECACHE_MODEL( (char *)def->wmodel );
	PRECACHE_MODEL( (char *)def->pmodel );
}

int CEfwWeapon::GetItemInfo( ItemInfo *p )
{
	const EfwWeaponDef *def = EFW_FindDef( STRING( pev->classname ) );
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 1;
	p->iPosition = def->slotPos;
	p->iId = m_iId = def->id;
	p->iWeight = 0;
	p->iFlags = 0;
	return 1;
}

int CEfwWeapon::AddToPlayer( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
	const EfwWeaponDef *defEarly = EFW_FindDef( STRING( pev->classname ) );
	/* FUN_100c29f0: GetTickCount must pass this+0x12c before FUN_100c46a0. */
	if( defEarly->itemBit == EFW_ITEM_IDTAG && pev->dmgtime && gpGlobals->time < pev->dmgtime )
		return FALSE;
#endif
	if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
#ifndef CLIENT_DLL
		const EfwWeaponDef *def = EFW_FindDef( STRING( pev->classname ) );
		char picked[80];
		const char *pretty;
		EFW_Dll()->items |= def->itemBit;
		pretty = def->classname;
		if( !strncmp( pretty, "weapon_efw_", 11 ) )
			pretty += 11;
		snprintf( picked, sizeof( picked ), "You just picked up the %s.", pretty );
		EFW_Print( pPlayer, picked );
		if( def->itemBit == EFW_ITEM_IDTAG )
			EFW_AddKeyword( "Player'sIDTagOnFence", 0 );
		if( def->itemBit == EFW_ITEM_PLIERS )
		{
			/* FUN_100c4f30: parent AddToPlayer, then MapLevel==0 FailOrNarrate
			   0x3d, then PLIERS / PLIERS_GOT_PLIERS / ELECTRICIAN. */
			static int s_pliersAdd;
			if( !s_pliersAdd )
			{
				s_pliersAdd = 1;
				EFW_DebugPrint( ">>> FUN_100c4f30 PLIERS PLIERS_GOT_PLIERS ELECTRICIAN" );
			}
			EFW_AddKeyword( "PLIERS", 0 );
			EFW_AddKeyword( "PLIERS_GOT_PLIERS", 1 );
			EFW_AddKeyword( "ELECTRICIAN", 0 );
			if( EFW_MapLevel() == 0 )
				EFW_FailOrNarrate( pPlayer, 0x3d );
		}
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
#endif
		return TRUE;
	}
	return FALSE;
}

BOOL CEfwWeapon::Deploy()
{
	const EfwWeaponDef *def = EFW_FindDef( STRING( pev->classname ) );
	return DefaultDeploy( def->vmodel, def->pmodel, 0, "crowbar" );
}

void CEfwWeapon::Holster( int skiplocal )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.4f;
	SendWeaponAnim( 0 );
}

void CEfwWeapon::PrimaryAttack()
{
	/* Overlay weapons have no PrimaryAttack fire; UseWithMarker / Give are VGUI ClientCmds. */
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.35f;
}

void CEfwWeapon::SecondaryAttack()
{
#ifndef CLIENT_DLL
	if( m_pPlayer )
	{
		CBaseEntity *pEnt = EFW_AimEntity( m_pPlayer, 110.0f );
		if( pEnt && EFW_IsTalkNpc( pEnt ) )
			EFW_GiveToNpc( m_pPlayer, pEnt, m_iId );
	}
#endif
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4f;
}

void CEfwWeapon::WeaponIdle()
{
	if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;
	SendWeaponAnim( 0 );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0f;
}

LINK_ENTITY_TO_CLASS( weapon_efw_Pliers, CEfwWeapon )
LINK_ENTITY_TO_CLASS( weapon_efw_Pilers, CEfwWeapon )
LINK_ENTITY_TO_CLASS( weapon_efw_Lever, CEfwWeapon )
LINK_ENTITY_TO_CLASS( weapon_efw_Branch, CEfwWeapon )
LINK_ENTITY_TO_CLASS( weapon_efw_MobilePhone, CEfwWeapon )
LINK_ENTITY_TO_CLASS( weapon_efw_IDTag, CEfwWeapon )
LINK_ENTITY_TO_CLASS( weapon_efw_BluePhoneCard, CEfwWeapon )
LINK_ENTITY_TO_CLASS( weapon_efw_GreenPhoneCard, CEfwWeapon )
LINK_ENTITY_TO_CLASS( weapon_efw_RedPhoneCard, CEfwWeapon )
LINK_ENTITY_TO_CLASS( weapon_efw_WashingPowder, CEfwWeapon )

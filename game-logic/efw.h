#pragma once
#if !defined(EFW_H)
#define EFW_H

// Hooks into hlsdk-portable. Gameplay lives in the DLL-derived port
// (addresses in decompile/recovered/NAMES.md).

#ifndef CLIENT_DLL
class CBasePlayer;
class CBaseEntity;
struct edict_s;
typedef struct edict_s edict_t;

int EFW_ClientCommand( edict_t *pEntity );
void EFW_PlayerSpawn( CBasePlayer *pPlayer );
void EFW_PlayerPreThink( CBasePlayer *pPlayer );
void EFW_LinkUserMessages( void );
void EFW_OverrideNpcModel( CBaseEntity *pEntity );
int EFW_RefugeeCount( void );
void EFW_Precache( void );
void EFW_StartTalk( CBasePlayer *pPlayer, CBaseEntity *pNpc );
void EFW_UseMarker( CBasePlayer *pPlayer, CBaseEntity *pMarker, int weaponId = 0 );
void EFW_Spider( CBasePlayer *pPlayer ); /* ClientCommand 0x1001b450 */
void EFW_Pickup( CBasePlayer *pPlayer, const char *arg ); /* ClientCommand 0x1001b5d7 */
void EFW_GiveToNpc( CBasePlayer *pPlayer, CBaseEntity *pNpc, int weaponId = 0 );
void EFW_StripWeapon( CBasePlayer *pPlayer, const char *classname, int itemBit );
void EFW_PatrolAlertAll( void ); /* FUN_100c5480 */
CBaseEntity *EFW_AimEntity( CBasePlayer *pPlayer, float dist );
int EFW_LookUse( CBasePlayer *pPlayer ); /* FUN_100c4af0 */
int EFW_IsTalkNpc( CBaseEntity *pEnt );
int EFW_FireTargets( const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, int useType, float value );
void EFW_OnDispatchSpawn( edict_t *pent ); /* edict-budget log during map spawn */
int EFW_ShouldSpawn( edict_t *pent ); /* skip nested worldspawn during FUN_100b2f80 */
int EFW_RejectSpawn( edict_t *pent ); /* pfnSpawn -1 drops duplicate entity-lump passes */
int EFW_BeginWorldPrecache( void ); /* re-entry guard around CWorld::Precache */
void EFW_EndWorldPrecache( void );
int EFW_PrecacheOnce( const char *szClassname ); /* one CREATE_NAMED_ENTITY per class */
void EFW_WPrecache( void ); /* FUN_100b2f80 tail: weapon_efw_* */
void EFW_OnServerActivate( void );
void EFW_OnServerDeactivate( void );
#endif

/* FUN_100c43b0 table 0x100f81e0: Pliers, Lever, Branch, MobilePhone, IDTag, Red, Green, Blue, Powder. */
#define WEAPON_EFW_PLIERS			16
#define WEAPON_EFW_LEVER			17
#define WEAPON_EFW_BRANCH			18
#define WEAPON_EFW_MOBILEPHONE		19
#define WEAPON_EFW_IDTAG			20
#define WEAPON_EFW_REDPHONECARD		21
#define WEAPON_EFW_GREENPHONECARD	22
#define WEAPON_EFW_BLUEPHONECARD		23
#define WEAPON_EFW_WASHINGPOWDER	24
#define WEAPON_EFW_PILERS			WEAPON_EFW_PLIERS

#define EFW_ITEM_PLIERS			( 1 << 0 )
#define EFW_ITEM_PILERS			( 1 << 1 )
#define EFW_ITEM_LEVER			( 1 << 2 )
#define EFW_ITEM_BRANCH			( 1 << 3 )
#define EFW_ITEM_PHONE			( 1 << 4 )
#define EFW_ITEM_IDTAG			( 1 << 5 )
#define EFW_ITEM_BLUECARD		( 1 << 6 )
#define EFW_ITEM_GREENCARD		( 1 << 7 )
#define EFW_ITEM_REDCARD		( 1 << 8 )
#define EFW_ITEM_POWDER			( 1 << 9 )
#define EFW_ITEM_SIM			( 1 << 10 )

#endif

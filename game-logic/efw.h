#pragma once
#if !defined(EFW_H)
#define EFW_H

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
void EFW_Spider( CBasePlayer *pPlayer );
void EFW_Precache( void );
#endif

#define WEAPON_EFW_PLIERS			16
#define WEAPON_EFW_PILERS			17
#define WEAPON_EFW_LEVER			18
#define WEAPON_EFW_BRANCH			19
#define WEAPON_EFW_MOBILEPHONE		20
#define WEAPON_EFW_IDTAG			21
#define WEAPON_EFW_BLUEPHONECARD	22
#define WEAPON_EFW_GREENPHONECARD	23
#define WEAPON_EFW_REDPHONECARD		24
#define WEAPON_EFW_WASHINGPOWDER	25

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

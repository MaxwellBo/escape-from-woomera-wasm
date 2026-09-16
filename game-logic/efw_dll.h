#pragma once
#ifndef CLIENT_DLL

#include "efw.h"
#include "efw_script.h"

// Layout recovered from EscapeFromWoomera.dll:
//   DAT_10134948 float slots (FUN_100c8180/8190)
//   DAT_10134950 int slots   (FUN_100c81a0/81b0)
//   EFWData send is FUN_100c6dd0: WRITE_BYTE(1) + 36 raw bytes.
//   Scan slots DAT_101348b0 are 0x30 bytes (FUN_100c7830 / FUN_100c7d30).

#define EFW_HUD_BLOB 36
#define EFW_MAX_DIARY 24 /* collected slots; flags indexed by page number (AddDiary 0..16) */
#define EFW_MAX_KEYWORDS 48
#define EFW_MAX_SEEN 80
#define EFW_MENU_LINES 7
#define EFW_MAX_SCAN 3
#define EFW_SCAN_NAME 29
#define EFW_SCAN_BYTES 0x30

#define EFW_HIDE_DIST 200.0f   /* DAT_1011d130; also 0x43480000 ShowMenu arg */
#define EFW_TALK_TIMEOUT 20.0f /* DAT_1011d128 */
#define EFW_TALK_SCAN 123.0f   /* 0x42f60000 in efw_TalkScan */

/* FUN_100c7830 slot: type int, name 0x1d at +4, xyz at +0x24. */
#pragma pack(push, 1)
struct EfwScanSlot
{
	int type; /* 0 NPC, 1 marker/door, 100+ weapon */
	char name[EFW_SCAN_NAME];
	char zero;
	unsigned char pad[2];
	float x, y, z;
};
#pragma pack(pop)

struct EfwDllState
{
	float hudFloat[2];
	int hudInt[7];
	unsigned char blob[EFW_HUD_BLOB];

	float lastTime;
	float dt;
	float hopeClock;
	float hudRetry;
	int diaryPages[EFW_MAX_DIARY]; /* DAT_10134444 */
	int diaryCount;     /* DAT_10134870 */
	int diaryFlags[EFW_MAX_DIARY]; /* DAT_10134474 */
	int diaryPending;   /* DAT_10134868 */

	CBasePlayer *player; /* DAT_10134888 */
	CBaseEntity *talkNpc; /* DAT_1013488c */
	int talkActive;      /* DAT_10134880 */
	float talkStart;     /* DAT_1013487c */
	float hideDist;      /* DAT_1011d130 */

	char keywords[EFW_MAX_KEYWORDS][EFW_TOPIC_LEN];
	int keywordUnlocked[EFW_MAX_KEYWORDS]; /* FUN_100c3500 flag at node+0x1c */
	int keywordCount;
	char seen[EFW_MAX_SEEN][64];
	int seenCount;

	int menuCount;
	int menuChoices[EFW_MENU_LINES];
	int menuMode; /* 0 none, 1 topics, 2 reply */
	char menuTitle[80];
	char menuText[EFW_MENU_LINES][160];
	char prevQuestion[160]; /* DAT_10134480; FUN_100c69a0 copies the hotkey line */

	int items;
	int mapLevel; /* FUN_100c5b80: 0 level1, 1 level2, 2 level3 */
	int inited;
	int hopeFailed;
	int scanCount; /* DAT_10134940 */
	EfwScanSlot scan[EFW_MAX_SCAN]; /* DAT_101348b0 */
	int talkIdleTicks; /* DAT_10134884: delay pending diary while idle */
};

EfwDllState *EFW_Dll( void );
CBasePlayer *EFW_Player( void ); /* 0x100c6980 */
void EFW_SetPlayer( CBasePlayer *pPlayer ); /* 0x100c6970 */
int EFW_MapLevel( void ); /* 0x100c5b80 */
void EFW_SetHudFloat( int slot, float value ); /* 0x100c8180 */
float EFW_GetHudFloat( int slot ); /* 0x100c8190 */
void EFW_SetHudInt( int slot, int value ); /* 0x100c81a0 */
int EFW_GetHudInt( int slot ); /* 0x100c81b0 */
void EFW_ThinkDt( void ); /* 0x100c6a70 */
void EFW_ThinkHope( void ); /* 0x100c6ad0 */
void EFW_SendHudState( void ); /* 0x100c6b60 */
void EFW_SendEfwData( void ); /* 0x100c6dd0 */
void EFW_FailOrNarrate( CBasePlayer *pPlayer, int code ); /* 0x100c81d0 */
void EFW_AdjustHope( float delta ); /* 0x100c4d70 */
void EFW_AddDiary( int page, int mode ); /* 0x100c6890 */
void EFW_FlagDiary( int page ); /* 0x100c6910 — flag by page number */
int EFW_DiaryCount( void ); /* 0x100c6880 */
void EFW_YyError( const char *msg, int line ); /* FUN_100c2620 bison yyerror */
void EFW_FlexMsg( const char *msg ); /* FUN_100c1f20 flex refill / fatals */
void EFW_VguiAssignSlots( void *p0, void *p1, void *p2, void *p3, void *p4, void *p5 ); /* FUN_100c6d70 */
void EFW_AddKeyword( const char *word, int unlocked ); /* 0x100c3500 */
int EFW_HasKeyword( const char *word ); /* 0x100c3430 — unlocked flag */
void EFW_DebugPrint( const char *fmt, ... ); /* 0x100c80d0 */
int EFW_FStrEq( const char *a, const char *b ); /* 0x100c8160 */
void EFW_ShowGoldMenu( CBasePlayer *pPlayer, int bits, int seconds, const char *text );
void EFW_ShowDllMenu( CBasePlayer *pPlayer, const char *title, const char **lines, int nLines );
void EFW_CloseMenu( CBasePlayer *pPlayer );
void EFW_Print( CBasePlayer *pPlayer, const char *text );
void EFW_GiveItem( CBasePlayer *pPlayer, int itemBit, const char *weaponName );
int EFW_HasWeapon( CBasePlayer *pPlayer, const char *classname ); /* 0x100c2f70 */
void EFW_DropTableReset( void ); /* FUN_100c2bf0 DAT_10132c70 = 0 */
void EFW_DropTablePush( void *owner, void *weapon ); /* FUN_100c2e90 Holster tail */
void *EFW_DropTableFind( void *weapon ); /* FUN_100c2c00 */
int EFW_DropTableHas( CBasePlayer *pPlayer, int weaponId ); /* FUN_100c2ee0 */
int EFW_HasSeen( const char *npc, const char *topic );
void EFW_MarkSeen( const char *npc, const char *topic );
void EFW_RunScriptAction( CBasePlayer *pPlayer, const char *action );
void EFW_ServerCommand( CBasePlayer *pPlayer, const char *cmd ); /* 0x100bfbf0 */
void EFW_InitFromSpawn( CBasePlayer *pPlayer ); /* 0x100c6740 + 0x100c6780 */

void EFW_LoadAllConversations( void ); /* 0x100b8ff0 */
void EFW_ThinkConversation( void ); /* 0x100c6c10 */
void EFW_TalkScan( void ); /* 0x100c7830 */
void EFW_HtmlVguiSync( void ); /* FUN_10044f70 stand-in: TalkScan → /efwvgui.txt */
void EFW_PollMenuKeys( void ); /* FUN_100c6a60 → FUN_100c69a0 GetAsyncKeyState */
void EFW_LatchInUse( void ); /* HostFwd +use when usercmds do not flush */
void EFW_LatchMenuKey( int slot ); /* FUN_100c6a50 */
void EFW_SendCntxt( void ); /* 0x100c7d30 */
void EFW_Squark( const char *targetname, const char *text = 0, int flags = 0 ); /* 0x100ba040 */
void EFW_GiveUnwanted( CBasePlayer *pPlayer, CBaseEntity *pNpc ); /* FUN_100c4550 → FUN_100b95a0 flag 8 */
void EFW_ShowConversationMenu( CBasePlayer *pPlayer, CBaseEntity *pNpc ); /* 0x100c6e60 */
void EFW_CloseTalk( void );
void EFW_ChooseTalk( CBasePlayer *pPlayer, int slot );
const char *EFW_ScriptNameForNpc( CBaseEntity *pNpc );
int EFW_IsTalkNpc( CBaseEntity *pEnt );

int EFW_FireTargets( const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, int useType, float value ); /* 0x100c7da0 */
void EFW_HideUnderBuilding( CBasePlayer *pPlayer ); /* ClientCommand 0x1001b969 */
void EFW_SetPause( int on ); /* 0x100c7510 */
void EFW_ChangeLevel( const char *map ); /* ClientCommand 0x1001b2f3 */
int EFW_ElectricianSees( CBasePlayer *pPlayer ); /* 0x100c59c0 */
int EFW_LookUse( CBasePlayer *pPlayer ); /* FUN_100c4af0 */
int EFW_WeaponTypeId( const char *classname ); /* 0x100c43b0 */
const char *EFW_WeaponClassname( int id ); /* inverse of 0x100c43b0 table */
int EFW_WeaponMask( CBasePlayer *pPlayer ); /* inventory bits for ids 16..31, packed into EFWData */
void EFW_UseNamed( const char *targetname, CBaseEntity *pActivator, CBaseEntity *pCaller, int useType, float value );
void EFW_StripWeapon( CBasePlayer *pPlayer, const char *classname, int itemBit );
CBaseEntity *EFW_PlaceIdTag( CBaseEntity *pTag, CBaseEntity *pMarker ); /* FUN_100c2a20 */
CBaseEntity *EFW_MaterializeIdTag( CBaseEntity *pMarker ); /* FUN_100c27f0 then FUN_100c2a20 */
CBaseEntity *EFW_PlacePlayerIdTag( CBasePlayer *pPlayer, CBaseEntity *pMarker );
void EFW_PatrolAlertAll( void ); /* FUN_100c5480: every monster_patrol_guard chases the player */
void EFW_OnDispatchSpawn( edict_t *pent ); /* NUMBER_OF_ENTITIES log; Host_ErrorInit cap is 1200 */
int EFW_ShouldSpawn( edict_t *pent );
int EFW_RejectSpawn( edict_t *pent );
int EFW_BeginWorldPrecache( void );
void EFW_EndWorldPrecache( void );
int EFW_PrecacheOnce( const char *szClassname );
void EFW_WPrecache( void ); /* FUN_100b2f80 after HL weapons */
void EFW_OnServerActivate( void );
void EFW_OnServerDeactivate( void );
void EFW_EnableNpcThink( edict_t *pent );
void EFW_InitPA( void ); /* 0x100c5fb0 / 0x100c7670 */
void EFW_ThinkPA( void ); /* 0x100c7740, from ThinkConversation when maplevel==0 */
void EFW_PlayCue( const char *sample ); /* 0x100c75e0 */
void EFW_PALockRAR( void ); /* 0x100c77c0 — rarLock=1, play Ann_RAR_124, timer=0 */
void EFW_PAUnlock( void ); /* 0x100c77e0 — rarLock=0 */

extern int gmsgEFWShow;
extern int gmsgEFWData;
extern int gmsgEFWMenu;
extern int gmsgEFWCntxt;
extern int gmsgEFWCtPrv;

#endif

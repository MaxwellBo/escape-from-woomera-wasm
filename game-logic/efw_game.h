#pragma once
#ifndef CLIENT_DLL

#include "efw.h"
#include "efw_script.h"

#define EFW_MAX_TOPICS 48
#define EFW_MAX_SEEN 80
#define EFW_MENU_NONE 0
#define EFW_MENU_TOPICS 1
#define EFW_MENU_CONTINUE 2
#define EFW_MENU_TEXT 3

#define EFW_PLIERS_NONE 0
#define EFW_PLIERS_STOLEN 1
#define EFW_PLIERS_IN_BIN 2
#define EFW_PLIERS_RECOVERED 3

struct EfwState
{
	int hope;
	unsigned int diary;
	int diaryPage;
	int diaryOpen;
	int talking; // entindex, 0 = none
	int menuMode;
	int menuChoices[9];
	int menuCount;
	char topics[EFW_MAX_TOPICS][EFW_TOPIC_LEN];
	int topicCount;
	char seen[EFW_MAX_SEEN][64];
	int seenCount;
	int items;
	float nextHopeDrain;
	int pliersState;
	int idTagOffBoard;
	int hidOnce;
	int failCount;
	int packageGot;
	int rosterOn;
	float hudRetry;
	int hudPulses;
	char hint[180];
	float autoTalkAt;
};

EfwState *EFW_GetState( CBasePlayer *pPlayer );
void EFW_AddTopic( CBasePlayer *pPlayer, const char *topic );
void EFW_DeleteTopic( CBasePlayer *pPlayer, const char *topic );
int EFW_HasTopic( CBasePlayer *pPlayer, const char *topic );
int EFW_HasSeen( CBasePlayer *pPlayer, const char *npc, const char *topic );
void EFW_MarkSeen( CBasePlayer *pPlayer, const char *npc, const char *topic );
void EFW_AddDiary( CBasePlayer *pPlayer, int page );
void EFW_AdjustHope( CBasePlayer *pPlayer, int delta );
void EFW_SendHope( CBasePlayer *pPlayer );
void EFW_SendHint( CBasePlayer *pPlayer, const char *text );
void EFW_SendDiary( CBasePlayer *pPlayer );
void EFW_ShowMenu( CBasePlayer *pPlayer, int bits, int seconds, const char *text );
void EFW_Print( CBasePlayer *pPlayer, const char *text );
void EFW_StartTalk( CBasePlayer *pPlayer, CBaseEntity *pNpc );
void EFW_ContinueTalk( CBasePlayer *pPlayer );
void EFW_ChooseTalk( CBasePlayer *pPlayer, int slot );
void EFW_CloseTalk( CBasePlayer *pPlayer );
void EFW_GiveToNpc( CBasePlayer *pPlayer, CBaseEntity *pNpc );
void EFW_UseMarker( CBasePlayer *pPlayer, CBaseEntity *pMarker );
CBaseEntity *EFW_AimEntity( CBasePlayer *pPlayer, float dist );
const char *EFW_ScriptNameForNpc( CBaseEntity *pNpc );
const EfwScript *EFW_LoadScript( const char *scriptName );
void EFW_GiveItem( CBasePlayer *pPlayer, int itemBit, const char *weaponName );
int EFW_WeaponToItem( const char *classname );
void EFW_RunAction( CBasePlayer *pPlayer, const char *action );
int EFW_IsTalkNpc( CBaseEntity *pEnt );

#endif

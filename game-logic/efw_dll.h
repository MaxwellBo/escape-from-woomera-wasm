#pragma once
#ifndef CLIENT_DLL

#include "efw.h"
#include "efw_script.h"

// Layout recovered from EscapeFromWoomera.dll:
//   DAT_10134948 float slots (FUN_100c8180/8190)
//   DAT_10134950 int slots   (FUN_100c81a0/81b0)
//   EFWData send is FUN_100c6dd0: WRITE_BYTE(1) + 36 raw bytes.

#define EFW_HUD_BLOB 36
#define EFW_MAX_DIARY 12
#define EFW_MAX_KEYWORDS 48
#define EFW_MAX_SEEN 80
#define EFW_MENU_LINES 7

#define EFW_HIDE_DIST 200.0f   /* DAT_1011d130; also 0x43480000 ShowMenu arg */
#define EFW_TALK_TIMEOUT 20.0f /* DAT_1011d128 */
#define EFW_TALK_SCAN 123.0f   /* 0x42f60000 in efw_TalkScan */

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
	int keywordCount;
	char seen[EFW_MAX_SEEN][64];
	int seenCount;

	int menuCount;
	int menuChoices[EFW_MENU_LINES];
	int menuMode; /* 0 none, 1 topics, 2 reply */

	int items;
	int mapLevel; /* FUN_100c5b80: 0 level1, 1 level2, 2 level3 */
	int inited;
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
void EFW_AddDiary( int page, int mode ); /* 0x100c6890 */
int EFW_DiaryCount( void ); /* 0x100c6880 */
void EFW_AddKeyword( const char *word, int unlocked ); /* 0x100c3500 */
int EFW_HasKeyword( const char *word );
void EFW_DebugPrint( const char *fmt, ... ); /* 0x100c80d0 */
int EFW_FStrEq( const char *a, const char *b ); /* 0x100c8160 */
void EFW_ShowGoldMenu( CBasePlayer *pPlayer, int bits, int seconds, const char *text );
void EFW_CloseMenu( CBasePlayer *pPlayer );
void EFW_Print( CBasePlayer *pPlayer, const char *text );
void EFW_GiveItem( CBasePlayer *pPlayer, int itemBit, const char *weaponName );
int EFW_HasSeen( const char *npc, const char *topic );
void EFW_MarkSeen( const char *npc, const char *topic );
void EFW_RunScriptAction( CBasePlayer *pPlayer, const char *action );
void EFW_InitFromSpawn( CBasePlayer *pPlayer ); /* 0x100c6740 + 0x100c6780 */

void EFW_LoadAllConversations( void ); /* 0x100b8ff0 */
void EFW_ThinkConversation( void ); /* 0x100c6c10 */
void EFW_TalkScan( void ); /* 0x100c7830 */
void EFW_Squark( const char *targetname ); /* 0x100ba040 */
void EFW_ShowConversationMenu( CBasePlayer *pPlayer, CBaseEntity *pNpc ); /* 0x100c6e60 */
void EFW_CloseTalk( void );
void EFW_ChooseTalk( CBasePlayer *pPlayer, int slot );
const char *EFW_ScriptNameForNpc( CBaseEntity *pNpc );
int EFW_IsTalkNpc( CBaseEntity *pEnt );

extern int gmsgEFWShow;
extern int gmsgEFWData;
extern int gmsgEFWMenu;
extern int gmsgEFWCntxt;
extern int gmsgEFWCtPrv;

#endif

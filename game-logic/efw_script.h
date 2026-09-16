#pragma once
// Parse Conversations/*.txt in the original v0.84 Q<>/A<> language
// (efwConversation::ParseFile 0x100c2660).

#define EFW_MAX_QUESTIONS 48
#define EFW_MAX_REPLIES 6
#define EFW_MAX_ACTIONS 8
#define EFW_TOPIC_LEN 40
#define EFW_TEXT_LEN 512
#define EFW_ACTION_LEN 48
#define EFW_FLAG_LEN 24

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	char flags[EFW_FLAG_LEN];
	char text[EFW_TEXT_LEN];
	char actions[EFW_MAX_ACTIONS][EFW_ACTION_LEN];
	int actionCount;
} EfwReply;

typedef struct
{
	char topic[EFW_TOPIC_LEN];
	char flags[EFW_FLAG_LEN];
	char text[EFW_TEXT_LEN];
	EfwReply replies[EFW_MAX_REPLIES];
	int replyCount;
} EfwQuestion;

typedef struct
{
	char name[EFW_TOPIC_LEN];
	EfwQuestion questions[EFW_MAX_QUESTIONS];
	int questionCount;
} EfwScript;

typedef void (*EfwScript_ErrorFn)( const char *msg, int line );
typedef void (*EfwScript_FlexFn)( const char *msg ); /* FUN_100c1f20 yy_get_next_buffer */

void EfwScript_Clear( EfwScript *script );
void EfwScript_SetErrorFn( EfwScript_ErrorFn fn ); /* FUN_100c2620 bison yyerror */
void EfwScript_SetFlexFn( EfwScript_FlexFn fn ); /* FUN_100c1dc0/2360/1f20..2640 / FUN_100be970 */
int EfwScript_Parse( EfwScript *script, const char *name, const char *src, int len );
int EfwScript_FindQuestion( const EfwScript *script, const char *topic );
int EfwFlags_Has( const char *flags, const char *token );
void EfwScript_FlexProbe( void ); /* FUN_100c1f20/2220/22b0/2410/20a0/2550..27b0 flex */

#ifdef __cplusplus
}
#endif

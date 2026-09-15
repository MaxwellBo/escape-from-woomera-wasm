#pragma once
// Engine-free Escape from Woomera conversation script parser.
// Parses Conversations/*.txt from the v0.84 demo.

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

void EfwScript_Clear( EfwScript *script );
int EfwScript_Parse( EfwScript *script, const char *name, const char *src, int len );
int EfwScript_FindQuestion( const EfwScript *script, const char *topic );
int EfwFlags_Has( const char *flags, const char *token );
int EfwTopicUnlockedByDefault( const char *topic );

#ifdef __cplusplus
}
#endif

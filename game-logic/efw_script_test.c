#include "efw_script.h"
#include <stdio.h>
#include <string.h>

static const char kMail[] =
	"Q<OFFICE>: I'm RAR-124.\n"
	"A: parcel. [ServerCommand(efw_GetPackage) DeleteTopic(OFFICE) ServerCommand(efw_EndMailPickupMessage) AddDiary(15)]\n";

static const char kRoster[] =
	"Q<KITCHEN>: kitchens?\n"
	"A: roster. [AddTopic(WORK) DeleteTopic(KITCHEN) ServerCommand(efw_TriggerMailPickupMessage) AddDiary(4)]\n"
	"Q<WORK>: work?\n"
	"A<FirstTime>: ok. [DeleteTopic(WORK) AddTopic(OnKitchenRoster)]\n";

static const char kMouhtaz[] =
	"Q<GREET>: Hello.\n"
	"A: later. [AddTopic(GREET_MOUHTAZ_SUBSEQUENT) AddDiary (16)]\n";

static int expect_action( const EfwReply *r, int i, const char *want )
{
	if( i >= r->actionCount )
	{
		printf( "missing action %d want %s\n", i, want );
		return 1;
	}
	if( strcmp( r->actions[i], want ) != 0 )
	{
		printf( "action %d got '%s' want '%s'\n", i, r->actions[i], want );
		return 1;
	}
	return 0;
}

static char s_flexCap[24][96];
static int s_flexN;
static char s_capMsg[64];
static int s_capLine;

static void capture_flex( const char *msg )
{
	if( s_flexN < 24 && msg )
	{
		snprintf( s_flexCap[s_flexN], sizeof( s_flexCap[0] ), "%s", msg );
		s_flexN++;
	}
}

static void capture_yyerror( const char *msg, int line )
{
	snprintf( s_capMsg, sizeof( s_capMsg ), "%s", msg ? msg : "" );
	s_capLine = line;
}

int main( void )
{
	EfwScript s;
	int fail = 0;

	EfwScript_Parse( &s, "Mail_Officer", kMail, -1 );
	fail += expect_action( &s.questions[0].replies[0], 0, "ServerCommand(efw_GetPackage)" );
	fail += expect_action( &s.questions[0].replies[0], 1, "DeleteTopic(OFFICE)" );
	fail += expect_action( &s.questions[0].replies[0], 2, "ServerCommand(efw_EndMailPickupMessage)" );
	fail += expect_action( &s.questions[0].replies[0], 3, "AddDiary(15)" );

	EfwScript_Parse( &s, "Roster_Officer", kRoster, -1 );
	fail += expect_action( &s.questions[0].replies[0], 2, "ServerCommand(efw_TriggerMailPickupMessage)" );
	fail += expect_action( &s.questions[1].replies[0], 1, "AddTopic(OnKitchenRoster)" );

	EfwScript_Parse( &s, "Mouhtaz", kMouhtaz, -1 );
	fail += expect_action( &s.questions[0].replies[0], 1, "AddDiary(16)" );

	{
		static const char kUnwanted[] =
			"Q<GREET>: Hello.\n"
			"A: hi.\n"
			"UNWANTED_ITEM\n"
			"A<FirstTime>: Keep it, my friend. It may be useful to you later.\n"
			"A<!FirstTime>: Keep it for yourself.\n";
		int qi;
		EfwScript_Parse( &s, "Amir", kUnwanted, -1 );
		qi = EfwScript_FindQuestion( &s, "UNWANTED_ITEM" );
		if( qi < 0 )
		{
			printf( "missing UNWANTED_ITEM question\n" );
			fail++;
		}
		else if( s.questions[qi].replyCount != 2 )
		{
			printf( "UNWANTED_ITEM replies %d want 2\n", s.questions[qi].replyCount );
			fail++;
		}
		else if( !EfwFlags_Has( s.questions[qi].replies[0].flags, "FirstTime" ) )
		{
			printf( "UNWANTED_ITEM FirstTime flag missing\n" );
			fail++;
		}
	}

	{
		static const char kBad[] =
			"Q<GREET>: Hello.\n"
			"A: hi.\n"
			"GARBAGE TOKEN\n";
		s_capMsg[0] = '\0';
		s_capLine = -1;
		EfwScript_SetErrorFn( capture_yyerror );
		EfwScript_Parse( &s, "yyerror", kBad, -1 );
		EfwScript_SetErrorFn( NULL );
		if( s_capLine != 3 || strcmp( s_capMsg, "parse error" ) != 0 )
		{
			printf( "FUN_100c2620 got '%s' line %d want parse error line 3\n", s_capMsg, s_capLine );
			fail++;
		}
		else
			printf( "ERROR: %s, line: %i\n", s_capMsg, s_capLine );
	}

	{
		int i;
		int gotMiss = 0, gotOv = 0, gotIn = 0;
		int gotPush = 0, gotLast = 0, gotMem = 0, gotAct = 0;
		s_flexN = 0;
		EfwScript_SetFlexFn( capture_flex );
		EfwScript_FlexProbe();
		EfwScript_SetFlexFn( NULL );
		for( i = 0; i < s_flexN; i++ )
		{
			if( strstr( s_flexCap[i], "end of buffer missed" ) )
				gotMiss = 1;
			if( strstr( s_flexCap[i], "scanner input buffer overflow" ) )
				gotOv = 1;
			if( strstr( s_flexCap[i], "input in flex scanner failed" ) )
				gotIn = 1;
			if( strstr( s_flexCap[i], "push-back overflow" ) )
				gotPush = 1;
			if( strstr( s_flexCap[i], "unexpected last match in yyinput()" ) )
				gotLast = 1;
			if( strstr( s_flexCap[i], "out of dynamic memory in yy_create_buffer()" ) )
				gotMem = 1;
			if( strstr( s_flexCap[i], "no action found" ) )
				gotAct = 1;
		}
		if( !gotMiss || !gotOv || !gotIn )
		{
			printf( "FUN_100c1f20 missing fatals miss=%d ov=%d in=%d n=%d\n",
				gotMiss, gotOv, gotIn, s_flexN );
			fail++;
		}
		else
			printf( "fatal flex scanner internal error--end of buffer missed\n" );
		if( !gotPush || !gotLast || !gotMem || !gotAct )
		{
			printf( "flex leftover fatals push=%d last=%d mem=%d act=%d n=%d\n",
				gotPush, gotLast, gotMem, gotAct, s_flexN );
			fail++;
		}
		else
			printf( "flex scanner push-back overflow\n" );
	}

	if( fail )
	{
		printf( "FAIL %d\n", fail );
		return 1;
	}
	printf( "ok\n" );
	return 0;
}

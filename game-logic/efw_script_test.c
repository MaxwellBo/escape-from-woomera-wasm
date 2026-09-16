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

	if( fail )
	{
		printf( "FAIL %d\n", fail );
		return 1;
	}
	printf( "ok\n" );
	return 0;
}

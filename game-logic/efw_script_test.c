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

	if( fail )
	{
		printf( "FAIL %d\n", fail );
		return 1;
	}
	printf( "ok\n" );
	return 0;
}

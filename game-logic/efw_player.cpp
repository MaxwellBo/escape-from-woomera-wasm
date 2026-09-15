#ifndef CLIENT_DLL

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "client.h"
#include "cdll_dll.h"
#include "efw_game.h"

#include <stdio.h>
#include <string.h>

extern int gmsgShowMenu;
extern int gmsgTextMsg;

static CBaseEntity *EFW_NearestTalkNpc( CBasePlayer *pPlayer, float dist );

static void EFW_WriteShare( const char *name, const char *text )
{
	char dir[64];
	char path[160];
	FILE *f;
	if( !name || !text )
		return;
	dir[0] = '\0';
	GET_GAME_DIR( dir );
	if( dir[0] )
	{
		snprintf( path, sizeof( path ), "%s/%s", dir, name );
		f = fopen( path, "w" );
		if( f )
		{
			fputs( text, f );
			fclose( f );
		}
	}
	snprintf( path, sizeof( path ), "/woomera/%s", name );
	f = fopen( path, "w" );
	if( f )
	{
		fputs( text, f );
		fclose( f );
	}
	snprintf( path, sizeof( path ), "/rwdir/woomera/%s", name );
	f = fopen( path, "w" );
	if( f )
	{
		fputs( text, f );
		fclose( f );
	}
}

static int gmsgHope = 0;
static int gmsgEfwDiary = 0;
static int gmsgEfwHint = 0;

static cvar_t efw_hope_cvar = { "efw_hope", "0", FCVAR_SERVER };
static cvar_t efw_hud_cvar = { "efw_hud", "", FCVAR_SERVER };
static cvar_t efw_diary_cvar = { "efw_diary_state", "0", FCVAR_SERVER };

static EfwState g_efw[33];

static int EFW_Index( CBasePlayer *pPlayer )
{
	int i;
	if( !pPlayer )
		return 0;
	i = ENTINDEX( pPlayer->edict() );
	if( i < 1 || i > 32 )
		return 0;
	return i;
}

EfwState *EFW_GetState( CBasePlayer *pPlayer )
{
	int i = EFW_Index( pPlayer );
	if( !i )
		return &g_efw[0];
	return &g_efw[i];
}

static void EFW_HostClientCmd( void )
{
	const char *a0 = CMD_ARGV( 0 );
	if( g_engfuncs.pfnServerPrint )
		g_engfuncs.pfnServerPrint( "efw: host command received\n" );
	ALERT( at_console, "efw: host cmd argc=%d argv0=%s\n", CMD_ARGC(), a0 ? a0 : "?" );
	edict_t *ed = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	if( !ed || !ed->pvPrivateData )
		return;
	EFW_ClientCommand( ed );
}

void EFW_LinkUserMessages( void )
{
	if( !gmsgHope )
		gmsgHope = REG_USER_MSG( "Hope", 1 );
	if( !gmsgEfwDiary )
		gmsgEfwDiary = REG_USER_MSG( "EfwDiary", 6 );
	if( !gmsgEfwHint )
		gmsgEfwHint = REG_USER_MSG( "EfwHint", -1 );
	{
		static int cvars_registered;
		static int cmds_registered;
		if( !cvars_registered )
		{
			cvars_registered = 1;
			CVAR_REGISTER( &efw_hope_cvar );
			CVAR_REGISTER( &efw_hud_cvar );
			CVAR_REGISTER( &efw_diary_cvar );
		}
		if( !cmds_registered && g_engfuncs.pfnAddServerCommand )
		{
			cmds_registered = 1;
			g_engfuncs.pfnAddServerCommand( "efw_Talk", EFW_HostClientCmd );
			g_engfuncs.pfnAddServerCommand( "efw_diary", EFW_HostClientCmd );
			g_engfuncs.pfnAddServerCommand( "efw_diary_next", EFW_HostClientCmd );
			g_engfuncs.pfnAddServerCommand( "efw_diary_prev", EFW_HostClientCmd );
			g_engfuncs.pfnAddServerCommand( "efw_spider", EFW_HostClientCmd );
			g_engfuncs.pfnAddServerCommand( "efw_HelpScreen", EFW_HostClientCmd );
		}
	}
}

void EFW_Precache( void )
{
	PRECACHE_MODEL( "models/DetaineeMaleT0.mdl" );
	PRECACHE_MODEL( "models/DetaineeMaleT1.mdl" );
	PRECACHE_MODEL( "models/DetaineeMaleT2.mdl" );
	PRECACHE_MODEL( "models/DetaineeMaleT3.mdl" );
	PRECACHE_MODEL( "models/DetaineeFemaleT0.mdl" );
	PRECACHE_MODEL( "models/DetaineeFemaleT1.mdl" );
	PRECACHE_MODEL( "models/DetaineeFemaleT2.mdl" );
	PRECACHE_MODEL( "models/Security.mdl" );
	PRECACHE_MODEL( "models/tradesman.mdl" );
	PRECACHE_MODEL( "models/w_pliers.mdl" );
	PRECACHE_MODEL( "models/v_pliers.mdl" );
	PRECACHE_MODEL( "models/p_pliers.mdl" );
	PRECACHE_MODEL( "models/w_washingpowder.mdl" );
	PRECACHE_MODEL( "models/v_washingpowder.mdl" );
	PRECACHE_MODEL( "models/p_washingpowder.mdl" );
	PRECACHE_MODEL( "models/w_mobilephone.mdl" );
	PRECACHE_MODEL( "models/w_idtag.mdl" );
}

void EFW_SendHope( CBasePlayer *pPlayer )
{
	EfwState *st;
	int hope;

	if( !gmsgHope )
		EFW_LinkUserMessages();
	if( !pPlayer || !gmsgHope )
		return;
	st = EFW_GetState( pPlayer );
	hope = st->hope;
	if( hope < 0 )
		hope = 0;
	if( hope > 100 )
		hope = 100;
	pPlayer->pev->armorvalue = hope;
	CVAR_SET_FLOAT( "efw_hope", (float)hope );
	if( gmsgHope )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgHope, NULL, pPlayer->pev );
			WRITE_BYTE( hope );
		MESSAGE_END();
	}
}

void EFW_SendHint( CBasePlayer *pPlayer, const char *text )
{
	EfwState *st;
	if( !gmsgEfwHint )
		EFW_LinkUserMessages();
	if( !pPlayer || !text )
		return;
	st = EFW_GetState( pPlayer );
	strncpy( st->hint, text, sizeof( st->hint ) - 1 );
	st->hint[sizeof( st->hint ) - 1] = '\0';
	{
		char flat[180];
		int i;
		strncpy( flat, st->hint, sizeof( flat ) - 1 );
		flat[sizeof( flat ) - 1] = '\0';
		for( i = 0; flat[i]; i++ )
		{
			if( flat[i] == '\n' )
				flat[i] = '|';
		}
		CVAR_SET_STRING( "efw_hud", flat );
		EFW_WriteShare( "efw_hud.txt", flat );
		{
			hudtextparms_t hp;
			memset( &hp, 0, sizeof( hp ) );
			hp.x = -1;
			hp.y = 0.28f;
			hp.r1 = 255;
			hp.g1 = 180;
			hp.b1 = 40;
			hp.a1 = 255;
			hp.r2 = 255;
			hp.g2 = 180;
			hp.b2 = 40;
			hp.a2 = 255;
			hp.fadeinTime = 0;
			hp.fadeoutTime = 0.2f;
			hp.holdTime = 12.0f;
			hp.channel = 2;
			{
				char msg[192];
				int i;
				strncpy( msg, st->hint, sizeof( msg ) - 1 );
				msg[sizeof( msg ) - 1] = '\0';
				for( i = 0; msg[i]; i++ )
				{
					if( msg[i] == '|' )
						msg[i] = '\n';
				}
				UTIL_HudMessage( pPlayer, hp, msg );
			}
		}
	}
	if( gmsgEfwHint )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgEfwHint, NULL, pPlayer->pev );
			WRITE_STRING( st->hint );
		MESSAGE_END();
	}
	UTIL_ShowMessage( st->hint, pPlayer );
}

void EFW_SendDiary( CBasePlayer *pPlayer )
{
	EfwState *st;
	char buf[24];

	if( !gmsgEfwDiary )
		EFW_LinkUserMessages();
	if( !pPlayer )
		return;
	st = EFW_GetState( pPlayer );
	if( gmsgEfwDiary )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgEfwDiary, NULL, pPlayer->pev );
			WRITE_BYTE( st->diaryOpen ? 1 : 0 );
			WRITE_BYTE( st->diaryPage );
			WRITE_LONG( (int)st->diary );
		MESSAGE_END();
	}
	snprintf( buf, sizeof( buf ), "%d %d %u", st->diaryOpen ? 1 : 0, st->diaryPage, st->diary );
	CVAR_SET_STRING( "efw_diary_state", buf );
	EFW_WriteShare( "efw_diary.txt", buf );
}

void EFW_AdjustHope( CBasePlayer *pPlayer, int delta )
{
	EfwState *st = EFW_GetState( pPlayer );
	st->hope += delta;
	if( st->hope > 100 )
		st->hope = 100;
	if( st->hope <= 0 )
	{
		st->hope = 0;
		EFW_Print( pPlayer, "Run out of hope!" );
	}
	EFW_SendHope( pPlayer );
}

void EFW_AddDiary( CBasePlayer *pPlayer, int page )
{
	EfwState *st = EFW_GetState( pPlayer );
	if( page < 1 || page > 31 )
		return;
	st->diary |= ( 1u << page );
	st->diaryPage = page;
	EFW_AdjustHope( pPlayer, 8 );
	EFW_SendDiary( pPlayer );
	ALERT( at_console, "Diary active item added    %d,\n", page );
}

int EFW_HasTopic( CBasePlayer *pPlayer, const char *topic )
{
	EfwState *st = EFW_GetState( pPlayer );
	int i;
	if( !topic || !topic[0] )
		return 0;
	for( i = 0; i < st->topicCount; i++ )
	{
		if( !strcmp( st->topics[i], topic ) )
			return 1;
	}
	return 0;
}

void EFW_AddTopic( CBasePlayer *pPlayer, const char *topic )
{
	EfwState *st = EFW_GetState( pPlayer );
	if( !topic || !topic[0] || EFW_HasTopic( pPlayer, topic ) )
		return;
	if( st->topicCount >= EFW_MAX_TOPICS )
		return;
	strncpy( st->topics[st->topicCount], topic, EFW_TOPIC_LEN - 1 );
	st->topics[st->topicCount][EFW_TOPIC_LEN - 1] = '\0';
	st->topicCount++;
}

void EFW_DeleteTopic( CBasePlayer *pPlayer, const char *topic )
{
	EfwState *st = EFW_GetState( pPlayer );
	int i;
	for( i = 0; i < st->topicCount; i++ )
	{
		if( !strcmp( st->topics[i], topic ) )
		{
			st->topics[i][0] = '\0';
			return;
		}
	}
}

int EFW_HasSeen( CBasePlayer *pPlayer, const char *npc, const char *topic )
{
	EfwState *st = EFW_GetState( pPlayer );
	char key[64];
	int i;
	snprintf( key, sizeof( key ), "%s|%s", npc ? npc : "", topic ? topic : "" );
	for( i = 0; i < st->seenCount; i++ )
	{
		if( !strcmp( st->seen[i], key ) )
			return 1;
	}
	return 0;
}

void EFW_MarkSeen( CBasePlayer *pPlayer, const char *npc, const char *topic )
{
	EfwState *st = EFW_GetState( pPlayer );
	char key[64];
	if( EFW_HasSeen( pPlayer, npc, topic ) )
		return;
	if( st->seenCount >= EFW_MAX_SEEN )
		return;
	snprintf( key, sizeof( key ), "%s|%s", npc ? npc : "", topic ? topic : "" );
	strncpy( st->seen[st->seenCount], key, sizeof( st->seen[0] ) - 1 );
	st->seen[st->seenCount][sizeof( st->seen[0] ) - 1] = '\0';
	st->seenCount++;
}

void EFW_Print( CBasePlayer *pPlayer, const char *text )
{
	char buf[190];
	const char *p;
	int n;

	if( !pPlayer || !text )
		return;
	p = text;
	while( *p )
	{
		n = 0;
		while( p[n] && n < 180 )
			n++;
		if( n >= 180 )
		{
			while( n > 120 && p[n] != ' ' )
				n--;
			if( n < 40 )
				n = 180;
		}
		memcpy( buf, p, (size_t)n );
		buf[n] = '\0';
		ClientPrint( pPlayer->pev, HUD_PRINTTALK, buf );
		p += n;
		while( *p == ' ' )
			p++;
	}
}

void EFW_ShowMenu( CBasePlayer *pPlayer, int bits, int seconds, const char *text )
{
	char chunk[121];
	const char *p;
	int n;
	int first;

	if( !pPlayer || !gmsgShowMenu )
		return;
	if( !text )
		text = "";
	p = text;
	first = 1;
	if( !bits )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgShowMenu, NULL, pPlayer->pev );
			WRITE_SHORT( 0 );
			WRITE_CHAR( 0 );
			WRITE_BYTE( 0 );
			WRITE_STRING( "" );
		MESSAGE_END();
		return;
	}
	do
	{
		n = 0;
		while( p[n] && n < 120 )
			n++;
		memcpy( chunk, p, (size_t)n );
		chunk[n] = '\0';
		p += n;
		MESSAGE_BEGIN( MSG_ONE, gmsgShowMenu, NULL, pPlayer->pev );
			WRITE_SHORT( bits );
			WRITE_CHAR( seconds );
			WRITE_BYTE( p[0] ? 1 : 0 );
			WRITE_STRING( first ? chunk : chunk );
		MESSAGE_END();
		first = 0;
	}
	while( *p );
}

static const char *kDefaultTopics[] = {
	"GREET", "ESCAPE", "GOODBYE", "OFFICE", "KITCHEN", "HIDING", "IDTAG", NULL
};

void EFW_PlayerSpawn( CBasePlayer *pPlayer )
{
	EfwState *st;
	int i;

	if( !pPlayer )
		return;
	st = EFW_GetState( pPlayer );
	memset( st, 0, sizeof( *st ) );
	st->hope = 72;
	st->diary = ( 1u << 1 );
	st->diaryPage = 1;
	st->nextHopeDrain = gpGlobals->time + 25.0f;
	for( i = 0; kDefaultTopics[i]; i++ )
		EFW_AddTopic( pPlayer, kDefaultTopics[i] );

	if( !( pPlayer->pev->weapons & ( 1 << WEAPON_SUIT ) ) )
		pPlayer->GiveNamedItem( "item_suit" );

	EFW_Precache();
	EFW_LinkUserMessages();
	EFW_SendHope( pPlayer );
	EFW_SendDiary( pPlayer );
	CLIENT_COMMAND( pPlayer->edict(), "bind i efw_diary\nbind [ efw_diary_prev\nbind ] efw_diary_next\nbind e +use\n" );
	{
		edict_t *pent;
		Vector spot;
		spot = pPlayer->pev->origin + Vector( 36, 80, 0 );
		pent = CREATE_NAMED_ENTITY( MAKE_STRING( "monster_refugee" ) );
		if( !FNullEnt( pent ) )
		{
			pent->v.origin = spot;
			pent->v.angles = Vector( 0, 270, 0 );
			pent->v.targetname = MAKE_STRING( "Amir" );
			DispatchSpawn( pent );
		}
	}
	{
		char buf[180];
		CBaseEntity *pScan = NULL;
		CBaseEntity *pBest = NULL;
		float best = 1e30f;
		while( ( pScan = UTIL_FindEntityInSphere( pScan, pPlayer->pev->origin, 4096.0f ) ) != NULL )
		{
			float d;
			if( pScan == pPlayer || !EFW_IsTalkNpc( pScan ) )
				continue;
			d = ( pScan->pev->origin - pPlayer->pev->origin ).Length();
			if( d < best )
			{
				best = d;
				pBest = pScan;
			}
		}
		if( pBest )
			snprintf( buf, sizeof( buf ), "People nearby: %d. Nearest %s (%.0fu). E or click to talk, I for diary.",
				EFW_RefugeeCount(), EFW_ScriptNameForNpc( pBest ), best );
		else
			snprintf( buf, sizeof( buf ), "People nearby: %d. Press E to talk, I for the diary.", EFW_RefugeeCount() );
		ClientPrint( pPlayer->pev, HUD_PRINTCENTER, buf );
		ClientPrint( pPlayer->pev, HUD_PRINTTALK, buf );
		EFW_Print( pPlayer, buf );
		EFW_SendHint( pPlayer, buf );
	}
	st->hudRetry = gpGlobals->time + 0.5f;
	st->hudPulses = 40;
	st->autoTalkAt = gpGlobals->time + 1.0f;
	CVAR_SET_FLOAT( "pausable", 0 );
	SERVER_COMMAND( "pausable 0\nunpause\n" );
}

static const char *kPackageText =
	"The package is from a pen-friend, a member of a refugee support group in Melbourne. "
	"The letter accompanying it brings you some hope, knowing that there is someone in this country that cares about your fate. "
	"Inside the package are some chocolate bars, which you give to some children, and a box of washing powder. "
	"Your suspicions aroused by mysterious rattling sound, you feel inside the box and discover a SIM card for a mobile phone.";

void EFW_GiveItem( CBasePlayer *pPlayer, int itemBit, const char *weaponName )
{
	EfwState *st = EFW_GetState( pPlayer );
	st->items |= itemBit;
	if( weaponName && weaponName[0] )
		pPlayer->GiveNamedItem( (char *)weaponName );
}

int EFW_WeaponToItem( const char *classname )
{
	if( !classname )
		return 0;
	if( strstr( classname, "Pliers" ) || strstr( classname, "pliers" ) )
		return EFW_ITEM_PLIERS;
	if( strstr( classname, "Pilers" ) )
		return EFW_ITEM_PILERS;
	if( strstr( classname, "Lever" ) )
		return EFW_ITEM_LEVER;
	if( strstr( classname, "Branch" ) )
		return EFW_ITEM_BRANCH;
	if( strstr( classname, "MobilePhone" ) )
		return EFW_ITEM_PHONE;
	if( strstr( classname, "IDTag" ) )
		return EFW_ITEM_IDTAG;
	if( strstr( classname, "BluePhone" ) )
		return EFW_ITEM_BLUECARD;
	if( strstr( classname, "GreenPhone" ) )
		return EFW_ITEM_GREENCARD;
	if( strstr( classname, "RedPhone" ) )
		return EFW_ITEM_REDCARD;
	if( strstr( classname, "Washing" ) )
		return EFW_ITEM_POWDER;
	return 0;
}

static void EFW_ServerCommand( CBasePlayer *pPlayer, const char *cmd )
{
	EfwState *st = EFW_GetState( pPlayer );
	if( !strcmp( cmd, "efw_TriggerMailPickupMessage" ) )
	{
		EFW_Print( pPlayer, "Loudspeakers: RAR-124, report to the office to collect mail." );
		EFW_AddTopic( pPlayer, "OFFICE" );
	}
	else if( !strcmp( cmd, "efw_GetPackage" ) )
	{
		if( !st->packageGot )
		{
			st->packageGot = 1;
			EFW_GiveItem( pPlayer, EFW_ITEM_POWDER | EFW_ITEM_SIM, "weapon_efw_WashingPowder" );
			EFW_AdjustHope( pPlayer, 18 );
			EFW_Print( pPlayer, kPackageText );
		}
	}
	else if( !strcmp( cmd, "efw_EndMailPickupMessage" ) )
	{
		EFW_Print( pPlayer, "The mail window closes." );
	}
}

void EFW_RunAction( CBasePlayer *pPlayer, const char *action )
{
	char name[48];
	char arg[32];
	const char *open;
	const char *close;

	if( !action || !action[0] )
		return;
	if( !strcmp( action, "Goodbye" ) )
	{
		EFW_CloseTalk( pPlayer );
		return;
	}
	open = strchr( action, '(' );
	if( !open )
		return;
	close = strchr( open, ')' );
	if( !close )
		return;
	strncpy( name, action, (size_t)( open - action ) );
	name[open - action] = '\0';
	while( name[0] && name[strlen( name ) - 1] == ' ' )
		name[strlen( name ) - 1] = '\0';
	strncpy( arg, open + 1, (size_t)( close - open - 1 ) );
	arg[close - open - 1] = '\0';

	if( !strcmp( name, "AddTopic" ) )
		EFW_AddTopic( pPlayer, arg );
	else if( !strcmp( name, "DeleteTopic" ) )
		EFW_DeleteTopic( pPlayer, arg );
	else if( !strcmp( name, "AddDiary" ) )
		EFW_AddDiary( pPlayer, atoi( arg ) );
	else if( !strcmp( name, "ServerCommand" ) )
		EFW_ServerCommand( pPlayer, arg );
}

CBaseEntity *EFW_AimEntity( CBasePlayer *pPlayer, float dist )
{
	TraceResult tr;
	CBaseEntity *pHit;
	CBaseEntity *pScan;
	CBaseEntity *pBest;
	float best;

	if( !pPlayer )
		return NULL;
	UTIL_MakeVectors( pPlayer->pev->v_angle );
	UTIL_TraceLine( pPlayer->pev->origin + pPlayer->pev->view_ofs,
		pPlayer->pev->origin + pPlayer->pev->view_ofs + gpGlobals->v_forward * dist,
		dont_ignore_monsters, pPlayer->edict(), &tr );
	if( tr.flFraction < 1.0f && tr.pHit )
	{
		pHit = CBaseEntity::Instance( tr.pHit );
		if( pHit && pHit != pPlayer )
			return pHit;
	}

	pBest = NULL;
	best = dist * dist;
	pScan = NULL;
	while( ( pScan = UTIL_FindEntityInSphere( pScan, pPlayer->pev->origin, dist ) ) != NULL )
	{
		Vector delta;
		float d2;
		const char *cn;
		if( pScan == pPlayer )
			continue;
		cn = STRING( pScan->pev->classname );
		if( strncmp( cn, "monster_", 8 ) && strcmp( cn, "efw_Marker" ) && strncmp( cn, "weapon_efw", 10 ) )
			continue;
		delta = pScan->pev->origin - ( pPlayer->pev->origin + pPlayer->pev->view_ofs );
		d2 = delta.Length();
		if( d2 < best )
		{
			best = d2;
			pBest = pScan;
		}
	}
	return pBest;
}

int EFW_IsTalkNpc( CBaseEntity *pEnt )
{
	const char *cn;
	if( !pEnt )
		return 0;
	cn = STRING( pEnt->pev->classname );
	return !strcmp( cn, "monster_refugee" )
		|| !strcmp( cn, "monster_barney" )
		|| !strcmp( cn, "monster_patrol_guard" )
		|| !strcmp( cn, "monster_efw_guard" );
}

void EFW_UseMarker( CBasePlayer *pPlayer, CBaseEntity *pMarker )
{
	EfwState *st = EFW_GetState( pPlayer );
	const char *name;
	const char *map;

	if( !pMarker )
		return;
	name = STRING( pMarker->pev->targetname );
	map = STRING( gpGlobals->mapname );

	if( !strcmp( name, "efw_PliersMarker" ) )
	{
		if( st->pliersState == EFW_PLIERS_NONE )
		{
			st->pliersState = EFW_PLIERS_STOLEN;
			EFW_GiveItem( pPlayer, EFW_ITEM_PLIERS, "weapon_efw_Pliers" );
			EFW_AddTopic( pPlayer, "PLIERS_GOT_PLIERS" );
			EFW_Print( pPlayer, "You wait until the electrician is not looking, and quickly grab the pliers from the workbench. He doesn't notice, and you hide them under your shirt. Heart pounding, you wonder how to safely get them to Amir." );
		}
		return;
	}
	if( !strcmp( name, "efw_kitchen_bin" ) )
	{
		if( st->pliersState == EFW_PLIERS_STOLEN && ( st->items & EFW_ITEM_PLIERS ) )
		{
			st->pliersState = EFW_PLIERS_IN_BIN;
			st->items &= ~EFW_ITEM_PLIERS;
			EFW_Print( pPlayer, "Again, you wait for the ideal moment to retrieve the pliers from under your shirt and slowly lower them into the bin, careful to not make a sound." );
		}
		else if( st->pliersState == EFW_PLIERS_IN_BIN )
		{
			st->pliersState = EFW_PLIERS_RECOVERED;
			EFW_GiveItem( pPlayer, EFW_ITEM_PLIERS, "weapon_efw_Pliers" );
			EFW_Print( pPlayer, "You recognise the bin in front of you as the one from the kitchen earlier today. You open the top and dig around inside, and sure enough, the pliers are still there. You retrieve them from the foodscraps and rubbish, and hide them in your clothes. Now to work out how to safely get these back to your fellow plotters." );
		}
		return;
	}
	if( !strcmp( name, "efw_hiding_place" ) )
	{
		if( st->pliersState == EFW_PLIERS_STOLEN )
		{
			EFW_Print( pPlayer, "You return to the hiding place, with the pliers safely tucked away underneath your shirt." );
			st->hidOnce = 1;
		}
		else if( !st->idTagOffBoard )
		{
			EFW_Print( pPlayer, "You could hide here, but you'd be caught at dusk when the guards saw your ID tag and came searching." );
		}
		else if( st->pliersState == EFW_PLIERS_NONE )
		{
			EFW_Print( pPlayer, "You could hide here and come out at night to get the pliers, if only you had a way to break into the rubbish bin cage." );
		}
		else
		{
			EFW_Print( pPlayer, "You realise that this is an ideal place to hide yourself for the next few hours, and wait until night falls. Now that the trader has agreed to take your ID tag from the fence, you won't be missed." );
			st->hidOnce = 1;
		}
		return;
	}
	if( !strcmp( name, "efw_IDTag_Position" ) )
	{
		if( st->idTagOffBoard )
		{
			st->idTagOffBoard = 0;
			EFW_Print( pPlayer, "You collect your ID tag from the checkpoint board." );
		}
		else
		{
			st->idTagOffBoard = 1;
			EFW_Print( pPlayer, "You leave your ID tag on the board so the guards know you have entered the compound." );
		}
		return;
	}
	(void)map;
}

void EFW_GiveToNpc( CBasePlayer *pPlayer, CBaseEntity *pNpc )
{
	EfwState *st = EFW_GetState( pPlayer );
	const char *npc;
	const EfwScript *script;

	if( !pNpc )
		return;
	npc = EFW_ScriptNameForNpc( pNpc );

	if( ( st->items & EFW_ITEM_PLIERS ) && !strcmp( npc, "Amir" ) )
	{
		st->items &= ~EFW_ITEM_PLIERS;
		st->pliersState = EFW_PLIERS_RECOVERED;
		EFW_DeleteTopic( pPlayer, "PLIERS" );
		EFW_DeleteTopic( pPlayer, "PLIERS_GOT_PLIERS" );
		EFW_AddDiary( pPlayer, 6 );
		EFW_AdjustHope( pPlayer, 12 );
		EFW_Print( pPlayer, "Amir takes the pliers and hides them in his clothes. The escape group can move forward." );
		return;
	}
	if( ( st->items & EFW_ITEM_SIM ) && !strcmp( npc, "Gholan" ) )
	{
		st->items &= ~EFW_ITEM_SIM;
		st->idTagOffBoard = 1;
		EFW_AddTopic( pPlayer, "GotHintAboutHiding" );
		EFW_Print( pPlayer, "Gholan takes the SIM card. He will remove your ID tag from the checkpoint while you hide." );
		return;
	}
	if( ( st->items & EFW_ITEM_POWDER ) && !strcmp( npc, "Mouhtaz" ) )
	{
		st->items &= ~EFW_ITEM_POWDER;
		EFW_AdjustHope( pPlayer, 6 );
		EFW_Print( pPlayer, "Mouhtaz thanks you for the washing powder." );
		return;
	}

	script = EFW_LoadScript( npc );
	if( script )
	{
		int qi = EfwScript_FindQuestion( script, "UNWANTED_ITEM" );
		if( qi >= 0 )
		{
			const EfwQuestion *q = &script->questions[qi];
			int first = !EFW_HasSeen( pPlayer, npc, "UNWANTED_ITEM" );
			int i;
			for( i = 0; i < q->replyCount; i++ )
			{
				const EfwReply *r = &q->replies[i];
				int ok = 1;
				if( EfwFlags_Has( r->flags, "FirstTime" ) && !first )
					ok = 0;
				if( EfwFlags_Has( r->flags, "!FirstTime" ) && first )
					ok = 0;
				if( ok )
				{
					if( r->text[0] )
						EFW_Print( pPlayer, r->text );
					EFW_MarkSeen( pPlayer, npc, "UNWANTED_ITEM" );
					return;
				}
			}
		}
	}
	EFW_Print( pPlayer, "They don't want that." );
}

void EFW_Spider( CBasePlayer *pPlayer )
{
	CBaseEntity *pEnt;
	const char *cn;

	if( !pPlayer )
		return;
	pEnt = EFW_AimEntity( pPlayer, 160.0f );
	if( !pEnt || ( !EFW_IsTalkNpc( pEnt ) && strcmp( STRING( pEnt->pev->classname ), "efw_Marker" ) ) )
	{
		CBaseEntity *nearNpc = EFW_NearestTalkNpc( pPlayer, 160.0f );
		if( nearNpc )
			pEnt = nearNpc;
	}
	if( !pEnt )
	{
		ALERT( at_aiconsole, "Ignoring spider\n" );
		return;
	}
	cn = STRING( pEnt->pev->classname );
	if( EFW_IsTalkNpc( pEnt ) )
	{
		EFW_StartTalk( pPlayer, pEnt );
		return;
	}
	if( !strcmp( cn, "efw_Marker" ) )
	{
		EFW_UseMarker( pPlayer, pEnt );
		return;
	}
	if( !strncmp( cn, "weapon_efw", 10 ) )
	{
		pEnt->Touch( pPlayer );
		return;
	}
}

static void EFW_ToggleDiary( CBasePlayer *pPlayer )
{
	EfwState *st = EFW_GetState( pPlayer );
	st->diaryOpen = !st->diaryOpen;
	EFW_SendDiary( pPlayer );
}

static void EFW_StepDiary( CBasePlayer *pPlayer, int dir )
{
	EfwState *st = EFW_GetState( pPlayer );
	int page = st->diaryPage;
	int i;
	for( i = 0; i < 31; i++ )
	{
		page += dir;
		if( page < 1 )
			page = 31;
		if( page > 31 )
			page = 1;
		if( st->diary & ( 1u << page ) )
		{
			st->diaryPage = page;
			st->diaryOpen = 1;
			EFW_SendDiary( pPlayer );
			return;
		}
	}
}

static void EFW_HelpScreen( CBasePlayer *pPlayer )
{
	EfwState *st = EFW_GetState( pPlayer );
	st->diaryOpen = 1;
	if( st->diary & ( 1u << 1 ) )
		st->diaryPage = 1;
	EFW_SendDiary( pPlayer );
	EFW_Print( pPlayer, "Objectives: talk to the others, keep hope alive, find a way out. Press I for the diary, E or click to talk." );
}

static CBaseEntity *EFW_NearestTalkNpc( CBasePlayer *pPlayer, float dist )
{
	CBaseEntity *pScan = NULL;
	CBaseEntity *pBest = NULL;
	float best = dist;
	if( !pPlayer )
		return NULL;
	while( ( pScan = UTIL_FindEntityInSphere( pScan, pPlayer->pev->origin, dist ) ) != NULL )
	{
		float d;
		if( pScan == pPlayer || !EFW_IsTalkNpc( pScan ) )
			continue;
		d = ( pScan->pev->origin - pPlayer->pev->origin ).Length();
		if( d < best )
		{
			best = d;
			pBest = pScan;
		}
	}
	return pBest;
}

int EFW_ClientCommand( edict_t *pEntity )
{
	CBasePlayer *pPlayer;
	const char *pcmd;

	if( !pEntity || !pEntity->pvPrivateData )
		return 0;
	pPlayer = GetClassPtr( (CBasePlayer *)&pEntity->v );
	int arg0 = 0;
	pcmd = CMD_ARGV( 0 );
	if( !pcmd )
		return 0;
	if( FStrEq( pcmd, "cmd" ) && CMD_ARGC() > 1 )
	{
		arg0 = 1;
		pcmd = CMD_ARGV( 1 );
	}

	if( FStrEq( pcmd, "menuselect" ) )
	{
		EfwState *st = EFW_GetState( pPlayer );
		int slot = atoi( CMD_ARGV( arg0 + 1 ) );
		if( st->menuMode == EFW_MENU_TOPICS )
			EFW_ChooseTalk( pPlayer, slot );
		else if( st->menuMode == EFW_MENU_CONTINUE || st->menuMode == EFW_MENU_TEXT )
			EFW_ContinueTalk( pPlayer );
		return 1;
	}
	if( FStrEq( pcmd, "efw_Talk" ) )
	{
		CBaseEntity *pEnt = NULL;
		if( CMD_ARGC() > 1 )
		{
			CBaseEntity *named = UTIL_FindEntityByTargetname( NULL, CMD_ARGV( 1 ) );
			ALERT( at_console, "efw: talk lookup '%s' -> %s\n", CMD_ARGV( 1 ),
				named ? STRING( named->pev->classname ) : "(not found)" );
			if( named )
				pEnt = named;
		}
		if( !pEnt )
			pEnt = EFW_AimEntity( pPlayer, 256.0f );
		if( !pEnt || !EFW_IsTalkNpc( pEnt ) )
			pEnt = EFW_NearestTalkNpc( pPlayer, 256.0f );
		if( pEnt && EFW_IsTalkNpc( pEnt ) )
			EFW_StartTalk( pPlayer, pEnt );
		else
		{
			EFW_Print( pPlayer, "No one to talk to." );
			EFW_SendHint( pPlayer, "No one to talk to." );
		}
		return 1;
	}
	if( FStrEq( pcmd, "efw_spider" ) || FStrEq( pcmd, "pickup" ) )
	{
		EFW_Spider( pPlayer );
		return 1;
	}
	if( FStrEq( pcmd, "efw_Give" ) )
	{
		CBaseEntity *pEnt = EFW_AimEntity( pPlayer, 110.0f );
		if( pEnt && EFW_IsTalkNpc( pEnt ) )
			EFW_GiveToNpc( pPlayer, pEnt );
		return 1;
	}
	if( FStrEq( pcmd, "efw_Pickup" ) )
	{
		EFW_Spider( pPlayer );
		return 1;
	}
	if( FStrEq( pcmd, "efw_UseWithMarker" ) )
	{
		CBaseEntity *pEnt = EFW_AimEntity( pPlayer, 128.0f );
		if( pEnt && !strcmp( STRING( pEnt->pev->classname ), "efw_Marker" ) )
			EFW_UseMarker( pPlayer, pEnt );
		return 1;
	}
	if( FStrEq( pcmd, "efw_diary" ) )
	{
		EFW_ToggleDiary( pPlayer );
		return 1;
	}
	if( FStrEq( pcmd, "efw_diary_next" ) )
	{
		EFW_StepDiary( pPlayer, 1 );
		return 1;
	}
	if( FStrEq( pcmd, "efw_diary_prev" ) )
	{
		EFW_StepDiary( pPlayer, -1 );
		return 1;
	}
	if( FStrEq( pcmd, "efw_HelpScreen" ) )
	{
		EFW_HelpScreen( pPlayer );
		return 1;
	}
	if( FStrEq( pcmd, "efw_ShowMenu" ) )
		return 1;
	if( FStrEq( pcmd, "efw_GetPackage" ) )
	{
		EFW_ServerCommand( pPlayer, "efw_GetPackage" );
		return 1;
	}
	if( FStrEq( pcmd, "efw_TriggerMailPickupMessage" ) )
	{
		EFW_ServerCommand( pPlayer, "efw_TriggerMailPickupMessage" );
		return 1;
	}
	if( FStrEq( pcmd, "efw_EndMailPickupMessage" ) )
	{
		EFW_ServerCommand( pPlayer, "efw_EndMailPickupMessage" );
		return 1;
	}
	if( FStrEq( pcmd, "efw_pause" ) || FStrEq( pcmd, "efw_set_state" ) || FStrEq( pcmd, "efw_changelevel" ) )
		return 1;
	return 0;
}

void EFW_PlayerPreThink( CBasePlayer *pPlayer )
{
	EfwState *st;

	if( !pPlayer )
		return;
	st = EFW_GetState( pPlayer );

	st->thinkFrames++;
	if( ( st->thinkFrames % 30 ) == 1 )
	{
		CBaseEntity *pNear;
		char beat[192];
		pPlayer->pev->armorvalue = ( st->thinkFrames & 32 ) ? (float)st->hope : (float)( st->hope ? st->hope - 1 : 0 );
		pPlayer->pev->iuser1 = (int)( gpGlobals->time * 10.0f );
		pPlayer->pev->iuser2 = st->talking;
		{
			hudtextparms_t hp;
			memset( &hp, 0, sizeof( hp ) );
			hp.x = -1;
			hp.y = 0.45f;
			hp.r1 = 255;
			hp.g1 = 255;
			hp.b1 = 0;
			hp.a1 = 255;
			hp.r2 = 255;
			hp.g2 = 255;
			hp.b2 = 0;
			hp.a2 = 255;
			hp.holdTime = 1.2f;
			hp.channel = 3;
			UTIL_HudMessage( pPlayer, hp, "EFW-TALK" );
		}
		if( !st->talking )
		{
			pNear = EFW_NearestTalkNpc( pPlayer, 256.0f );
			if( pNear )
				EFW_StartTalk( pPlayer, pNear );
		}
		EFW_SendHope( pPlayer );
		EFW_SendDiary( pPlayer );
		snprintf( beat, sizeof( beat ), "t=%.0f talking=%d | %s", gpGlobals->time, st->talking, st->hint );
		EFW_WriteShare( "efw_hud.txt", beat );
	}

	if( st->autoTalkAt && gpGlobals->time >= st->autoTalkAt )
	{
		CBaseEntity *pNear;
		st->autoTalkAt = 0;
		pNear = EFW_NearestTalkNpc( pPlayer, 200.0f );
		if( pNear )
			EFW_StartTalk( pPlayer, pNear );
	}

	if( ( pPlayer->m_afButtonPressed & IN_USE ) || ( pPlayer->m_afButtonPressed & IN_ATTACK ) )
	{
		CBaseEntity *pEnt = EFW_AimEntity( pPlayer, 160.0f );
		if( !pEnt || ( !EFW_IsTalkNpc( pEnt ) && strcmp( STRING( pEnt->pev->classname ), "efw_Marker" ) ) )
			pEnt = EFW_NearestTalkNpc( pPlayer, 160.0f );
		if( pEnt && ( EFW_IsTalkNpc( pEnt ) || !strcmp( STRING( pEnt->pev->classname ), "efw_Marker" ) ) )
		{
			EFW_Spider( pPlayer );
			pPlayer->m_afButtonPressed &= ~( IN_USE | IN_ATTACK );
			pPlayer->pev->button &= ~( IN_USE | IN_ATTACK );
		}
	}

	if( st->talking )
	{
		CBaseEntity *pNpc = CBaseEntity::Instance( INDEXENT( st->talking ) );
		if( !pNpc || ( pNpc->pev->origin - pPlayer->pev->origin ).Length() > 160.0f )
		{
			EFW_Print( pPlayer, "Conversation hidden, partner too far" );
			EFW_CloseTalk( pPlayer );
		}
	}

	if( st->nextHopeDrain && gpGlobals->time >= st->nextHopeDrain )
	{
		st->nextHopeDrain = gpGlobals->time + 28.0f;
		if( st->hope > 1 )
			EFW_AdjustHope( pPlayer, -1 );
		else
			EFW_AdjustHope( pPlayer, -st->hope );
	}
}

#endif

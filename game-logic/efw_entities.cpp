#ifndef CLIENT_DLL

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "activity.h"
#include "player.h"
#include "efw_dll.h"

#include <string.h>
#include <stdio.h>

static int g_refugeeCount;

int EFW_RefugeeCount( void )
{
	return g_refugeeCount;
}

static void EFW_SetVisibleModel( CBaseEntity *pEntity, const char *preferred )
{
	const char *choices[4];
	int n = 0;
	int i;

	if( preferred && preferred[0] )
		choices[n++] = preferred;
	choices[n++] = "models/player.mdl";
	choices[n++] = "models/barney.mdl";
	choices[n++] = "models/scientist.mdl";

	for( i = 0; i < n; i++ )
	{
		PRECACHE_MODEL( (char *)choices[i] );
		SET_MODEL( ENT( pEntity->pev ), choices[i] );
		if( pEntity->pev->modelindex > 0 )
			return;
	}
}

static int EFW_NameIs( const char *tn, const char *a )
{
	if( !tn || !a )
		return 0;
	return !stricmp( tn, a );
}

static int EFW_IsFemale( const char *tn )
{
	return EFW_NameIs( tn, "Elika" ) || EFW_NameIs( tn, "Laleh" ) || EFW_NameIs( tn, "Shala" );
}

void EFW_OverrideNpcModel( CBaseEntity *pEntity )
{
	const char *tn;
	const char *model = NULL;

	if( !pEntity )
		return;
	tn = STRING( pEntity->pev->targetname );
	if( EFW_NameIs( tn, "efw_electrician" ) )
		model = "models/tradesman.mdl";
	else if( EFW_NameIs( tn, "Roster_Officer" ) || EFW_NameIs( tn, "Mail_Officer" )
		|| EFW_NameIs( tn, "Kitchen_Guard" ) || EFW_NameIs( tn, "efw_compound_gate_guard" )
		|| ( tn && strstr( tn, "Guard" ) ) || ( tn && strstr( tn, "Patrol" ) ) )
		model = "models/Security.mdl";

	if( model )
	{
		PRECACHE_MODEL( (char *)model );
		SET_MODEL( ENT( pEntity->pev ), model );
	}
}

class CRefugee : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int Classify( void );
	int ObjectCaps( void ) { return CBaseMonster::ObjectCaps() | FCAP_IMPULSE_USE; }
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void EXPORT TalkUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT IdleThink( void );
	int m_iWalkState; /* this+0x284: 3 = walking toward player */
};

LINK_ENTITY_TO_CLASS( monster_refugee, CRefugee )

int CRefugee::Classify( void )
{
	return CLASS_PLAYER_ALLY;
}

void CRefugee::SetYawSpeed( void )
{
	pev->yaw_speed = 90;
}

void CRefugee::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	CBaseMonster::HandleAnimEvent( pEvent );
}

void CRefugee::TalkUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if( pActivator && pActivator->IsPlayer() )
		EFW_StartTalk( (CBasePlayer *)pActivator, this );
}

void CRefugee::IdleThink( void )
{
	CBasePlayer *pPlayer;
	const char *tn;
	Vector delta;
	float dist;
	static int s_walkTick; /* DAT_10132ca8, shared across refugees */

	// CRefugee::IdleThink 0x100c6440
	pev->framerate = 1.0f;
	pev->nextthink = gpGlobals->time + 0.1f;
	if( pev->health == 2.0f )
	{
		UTIL_SetSize( pev, g_vecZero, g_vecZero );
		StudioFrameAdvance();
		return;
	}
	UTIL_FindEntityByTargetname( NULL, "mad_scientist_entity" );
	UTIL_SetSize( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 72 ) );
	tn = STRING( pev->targetname );
	pPlayer = EFW_Player();
	if( pPlayer && !EFW_FStrEq( tn, "queue" ) )
	{
		s_walkTick++;
		delta = pPlayer->pev->origin - pev->origin;
		dist = delta.Length();
		pev->movetype = MOVETYPE_STEP;
		if( ( s_walkTick % 0x52 ) == 0 && dist > 100.0f && dist < 300.0f )
		{
			SetActivity( ACT_WALK );
			m_iWalkState = 3;
			m_hEnemy = pPlayer;
			EFW_DebugPrint( "now walking %s", ( tn && tn[0] ) ? tn : "?" );
			{
				char line[80];
				snprintf( line, sizeof( line ), "now walking %s", ( tn && tn[0] ) ? tn : "?" );
				EFW_Print( pPlayer, line );
			}
		}
		if( m_iWalkState == 3 && dist < 100.0f )
			SetActivity( ACT_IDLE );
		if( m_iWalkState == 3 && dist >= 100.0f )
		{
			delta.z = 0;
			if( delta.Length() > 1.0f )
			{
				pev->angles.y = UTIL_VecToYaw( delta );
				WALK_MOVE( ENT( pev ), pev->angles.y, 8.0f, WALKMOVE_NORMAL );
			}
		}
	}
	StudioFrameAdvance();
}

void CRefugee::Precache( void )
{
	PRECACHE_MODEL( "models/DetaineeMaleT0.mdl" );
	PRECACHE_MODEL( "models/DetaineeMaleT1.mdl" );
	PRECACHE_MODEL( "models/DetaineeMaleT2.mdl" );
	PRECACHE_MODEL( "models/DetaineeMaleT3.mdl" );
	PRECACHE_MODEL( "models/DetaineeFemaleT0.mdl" );
	PRECACHE_MODEL( "models/DetaineeFemaleT1.mdl" );
	PRECACHE_MODEL( "models/DetaineeFemaleT2.mdl" );
}

void CRefugee::Spawn( void )
{
	const char *tn;
	const char *model;
	int variant;

	Precache();
	PRECACHE_MODEL( "models/player.mdl" );
	PRECACHE_MODEL( "models/barney.mdl" );
	tn = STRING( pev->targetname );
	variant = ENTINDEX( edict() );
	if( EFW_IsFemale( tn ) )
	{
		static const char *fem[] = {
			"models/DetaineeFemaleT0.mdl",
			"models/DetaineeFemaleT1.mdl",
			"models/DetaineeFemaleT2.mdl"
		};
		model = fem[variant % 3];
	}
	else
	{
		static const char *male[] = {
			"models/DetaineeMaleT0.mdl",
			"models/DetaineeMaleT1.mdl",
			"models/DetaineeMaleT2.mdl",
			"models/DetaineeMaleT3.mdl"
		};
		model = male[variant % 4];
	}

	EFW_SetVisibleModel( this, model );
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->health = 100;
	pev->takedamage = DAMAGE_NO;
	pev->view_ofs = Vector( 0, 0, 50 );
	m_flFieldOfView = 0.5;
	m_MonsterState = MONSTERSTATE_NONE;
	g_refugeeCount++;
	ALERT( at_console, "efw: refugee %s model %s at %.0f %.0f %.0f\n",
		( tn && tn[0] ) ? tn : "(unnamed)", STRING( pev->model ), pev->origin.x, pev->origin.y, pev->origin.z );
	m_iWalkState = 0;
	SetUse( &CRefugee::TalkUse );
	SetThink( &CRefugee::IdleThink );
	pev->nextthink = gpGlobals->time + 0.1f;
}

class CPatrolGuard : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int Classify( void );
	int ObjectCaps( void ) { return CBaseMonster::ObjectCaps() | FCAP_IMPULSE_USE; }
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void EXPORT TalkUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT PatrolThink( void );
	void WalkToward( const Vector &dest );
	int CanSeePlayer( CBasePlayer *pPlayer );
	int CanHearPlayer( CBasePlayer *pPlayer );
	float Dist2D( CBaseEntity *pOther );
	int m_iAlert; /* this+0x398 */
	Vector m_vecLastSeen; /* this+0x39c */
	float m_flAlertTime; /* this+0x3a8 */
	float m_flStateTime; /* this+0x3ac */
	int m_iCaught;
};

LINK_ENTITY_TO_CLASS( monster_patrol_guard, CPatrolGuard )
LINK_ENTITY_TO_CLASS( monster_efw_guard, CPatrolGuard )

int CPatrolGuard::Classify( void )
{
	return CLASS_PLAYER_ALLY;
}

void CPatrolGuard::SetYawSpeed( void )
{
	pev->yaw_speed = 120;
}

void CPatrolGuard::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	CBaseMonster::HandleAnimEvent( pEvent );
}

void CPatrolGuard::TalkUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if( pActivator && pActivator->IsPlayer() )
		EFW_StartTalk( (CBasePlayer *)pActivator, this );
}

void CPatrolGuard::Precache( void )
{
	PRECACHE_MODEL( "models/Security.mdl" );
}

int CPatrolGuard::CanSeePlayer( CBasePlayer *pPlayer )
{
	Vector dir;
	float dist;
	float dot;
	TraceResult tr;
	Vector from;
	Vector to;

	if( !pPlayer )
		return 0;
	dir = pPlayer->pev->origin - pev->origin;
	dist = dir.Length();
	if( dist > 512.0f )
		return 0;
	if( dist < 1.0f )
		return 1;
	dir = dir * ( 1.0f / dist );
	UTIL_MakeVectors( pev->angles );
	dot = DotProduct( gpGlobals->v_forward, dir );
	/* FUN_100c5c50: angle < 1.0471967 rad (60°), cos ≈ 0.5 */
	if( dot < 0.5f )
		return 0;
	from = pev->origin + Vector( 0, 0, 40 );
	to = pPlayer->pev->origin + Vector( 0, 0, 40 );
	UTIL_TraceLine( from, to, ignore_monsters, edict(), &tr );
	if( tr.flFraction >= 1.0f )
		return 1;
	to.z += 40.0f;
	UTIL_TraceLine( from, to, ignore_monsters, edict(), &tr );
	return tr.flFraction >= 1.0f;
}

int CPatrolGuard::CanHearPlayer( CBasePlayer *pPlayer )
{
	Vector d;
	if( !pPlayer )
		return 0;
	d = pPlayer->pev->origin - pev->origin;
	d.z = 0;
	if( d.Length() > 256.0f )
		return 0;
	d = pPlayer->pev->velocity;
	d.z = 0;
	return d.Length() > 80.0f;
}

float CPatrolGuard::Dist2D( CBaseEntity *pOther )
{
	Vector d;
	if( !pOther )
		return 9999.0f;
	d = pOther->pev->origin - pev->origin;
	d.z = 0;
	return d.Length();
}

void CPatrolGuard::WalkToward( const Vector &dest )
{
	Vector delta = dest - pev->origin;
	delta.z = 0;
	if( delta.Length() < 12.0f )
		return;
	pev->angles.y = UTIL_VecToYaw( delta );
	SetActivity( ACT_WALK );
	WALK_MOVE( ENT( pev ), pev->angles.y, 8.0f, WALKMOVE_NORMAL );
}

void EFW_PatrolAlertAll( void )
{
	CBaseEntity *pGuard = NULL;
	CBasePlayer *pPlayer = EFW_Player();
	while( ( pGuard = UTIL_FindEntityByClassname( pGuard, "monster_patrol_guard" ) ) != NULL )
	{
		CPatrolGuard *pg = (CPatrolGuard *)pGuard;
		pg->m_hEnemy = pPlayer;
		pg->m_iAlert = 4;
		if( pPlayer )
			pg->m_vecLastSeen = pPlayer->pev->origin;
		pg->SetActivity( ACT_WALK );
	}
}

void CPatrolGuard::PatrolThink( void )
{
	CBasePlayer *pPlayer = EFW_Player();
	const char *tn = STRING( pev->targetname );
	int see = 0;
	int hear = 0;
	float now = gpGlobals->time;

	pev->nextthink = now + 0.1f;
	/* FUN_100c7490 / DAT_101348ac pause. */
	if( EFW_GetHudInt( 6 ) )
	{
		pev->framerate = 0.0f;
		pev->movetype = MOVETYPE_NONE;
		StudioFrameAdvance();
		return;
	}
	pev->framerate = 1.0f;
	pev->movetype = MOVETYPE_STEP;
	see = CanSeePlayer( pPlayer );
	hear = CanHearPlayer( pPlayer );
	if( hear )
		EFW_DebugPrint( "can hear player!!!!!!!!" );

	/* FUN_100c54e0 patrol alert FSM, this+0x398. */
	switch( m_iAlert )
	{
	case 0:
		if( see || hear )
		{
			m_iAlert = 1;
			if( pPlayer )
				m_vecLastSeen = pPlayer->pev->origin;
			m_flAlertTime = now;
			m_flStateTime = now;
		}
		break;
	case 1:
		if( !see || hear )
		{
			m_iAlert = 0;
			break;
		}
		if( now - m_flAlertTime >= 0.7f )
		{
			EFW_Squark( tn, "Hey, what was that? I thought I saw something.", 10 );
			if( pPlayer )
				m_vecLastSeen = pPlayer->pev->origin;
			m_flAlertTime = now;
			m_flStateTime = now;
			m_iAlert = 3;
		}
		break;
	case 2:
		if( see || hear )
		{
			EFW_Squark( tn, "Hmmm?", 10 );
			m_iAlert = 3;
			m_flStateTime = now;
			break;
		}
		if( now - m_flAlertTime < 5.0f || now - m_flStateTime < 1.0f )
			break;
		EFW_Squark( tn, "Ah, guess it was nothing...", 10 );
		m_iAlert = 0;
		break;
	case 3:
		if( !see )
		{
			m_flStateTime = now;
			m_iAlert = 2;
			break;
		}
		if( pPlayer )
			m_vecLastSeen = pPlayer->pev->origin;
		if( now - m_flStateTime >= 1.0f || Dist2D( pPlayer ) < 128.0f )
		{
			EFW_Squark( tn, "Halt! Put your hands up, and get down on the ground!", 10 );
			m_flStateTime = now;
			EFW_PatrolAlertAll();
			m_hEnemy = pPlayer;
		}
		break;
	case 4:
		if( pPlayer )
			m_vecLastSeen = pPlayer->pev->origin;
		if( Dist2D( pPlayer ) < 75.0f && !m_iCaught )
		{
			m_iCaught = 1;
			EFW_AdjustHope( -10.0f );
			EFW_FailOrNarrate( pPlayer, 0x46 );
		}
		break;
	default:
		break;
	}

	if( m_iAlert == 4 )
	{
		WalkToward( m_vecLastSeen );
		m_hEnemy = pPlayer;
	}
	else if( m_iAlert == 2 || m_iAlert == 3 )
		WalkToward( m_vecLastSeen );
	else if( !FStringNull( pev->target ) )
	{
		if( !m_pGoalEnt )
			m_pGoalEnt = UTIL_FindEntityByTargetname( NULL, STRING( pev->target ) );
		if( m_pGoalEnt )
		{
			Vector delta = m_pGoalEnt->pev->origin - pev->origin;
			delta.z = 0;
			if( delta.Length() < 32.0f )
			{
				if( !FStringNull( m_pGoalEnt->pev->target ) )
					m_pGoalEnt = UTIL_FindEntityByTargetname( NULL, STRING( m_pGoalEnt->pev->target ) );
			}
			else
			{
				pev->angles.y = UTIL_VecToYaw( delta );
				WALK_MOVE( ENT( pev ), pev->angles.y, 8.0f, WALKMOVE_NORMAL );
			}
		}
	}
	StudioFrameAdvance();
}

void CPatrolGuard::Spawn( void )
{
	Precache();
	SET_MODEL( ENT( pev ), "models/Security.mdl" );
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->health = 80;
	pev->view_ofs = Vector( 0, 0, 50 );
	m_flFieldOfView = 0.5;
	m_MonsterState = MONSTERSTATE_NONE;
	m_iAlert = 0;
	m_flAlertTime = 0;
	m_flStateTime = 0;
	m_iCaught = 0;
	SetUse( &CPatrolGuard::TalkUse );
	SetThink( &CPatrolGuard::PatrolThink );
	pev->nextthink = gpGlobals->time + 0.5f;
}

class CEfwMarker : public CBaseEntity
{
public:
	void Spawn( void );
	int ObjectCaps( void ) { return CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE; }
	void EXPORT MarkerUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

LINK_ENTITY_TO_CLASS( efw_Marker, CEfwMarker )

void CEfwMarker::MarkerUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if( pActivator && pActivator->IsPlayer() )
		EFW_UseMarker( (CBasePlayer *)pActivator, this );
}

void CEfwMarker::Spawn( void )
{
	/* FUN_100c30a0: solid=0, movetype=7, SET_MODEL, DROP_TO_FLOOR,
	   EF_NODRAW unless showtriggers. */
	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_PUSH;
	pev->solid = SOLID_NOT;
	SET_MODEL( ENT( pev ), STRING( pev->model ) );
	DROP_TO_FLOOR( ENT( pev ) );
	if( CVAR_GET_FLOAT( "showtriggers" ) == 0.0f )
		pev->effects |= EF_NODRAW;
	SetUse( &CEfwMarker::MarkerUse );
}

#endif

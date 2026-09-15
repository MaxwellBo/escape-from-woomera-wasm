#ifndef CLIENT_DLL

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "player.h"
#include "efw_game.h"

#include <string.h>
#include <stdio.h>

static int g_refugeeCount;

int EFW_RefugeeCount( void )
{
	return g_refugeeCount;
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

	SET_MODEL( ENT( pev ), model );
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
		( tn && tn[0] ) ? tn : "(unnamed)", model, pev->origin.x, pev->origin.y, pev->origin.z );
	MonsterInit();
	SetUse( &CRefugee::TalkUse );
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

void CPatrolGuard::PatrolThink( void )
{
	pev->nextthink = gpGlobals->time + 0.1f;
	StudioFrameAdvance();
	if( !m_pGoalEnt && !FStringNull( pev->target ) )
		m_pGoalEnt = UTIL_FindEntityByTargetname( NULL, STRING( pev->target ) );
	if( !m_pGoalEnt )
		return;
	{
		Vector delta = m_pGoalEnt->pev->origin - pev->origin;
		delta.z = 0;
		if( delta.Length() < 32.0f )
		{
			if( !FStringNull( m_pGoalEnt->pev->target ) )
				m_pGoalEnt = UTIL_FindEntityByTargetname( NULL, STRING( m_pGoalEnt->pev->target ) );
			return;
		}
		pev->angles.y = UTIL_VecToYaw( delta );
		WALK_MOVE( ENT( pev ), pev->angles.y, 8.0f, WALKMOVE_NORMAL );
	}
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
	MonsterInit();
	SetUse( &CPatrolGuard::TalkUse );
	if( !FStringNull( pev->target ) )
	{
		SetThink( &CPatrolGuard::PatrolThink );
		pev->nextthink = gpGlobals->time + 0.5f;
	}
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
	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_PUSH;
	pev->solid = SOLID_BSP;
	SET_MODEL( ENT( pev ), STRING( pev->model ) );
	SetUse( &CEfwMarker::MarkerUse );
}

#endif

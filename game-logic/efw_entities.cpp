#ifndef CLIENT_DLL

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "activity.h"
#include "player.h"
#include "efw_dll.h"
#include "studio.h"

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
		if( EFW_DeferStudio() )
		{
			pEntity->pev->model = MAKE_STRING( choices[i] );
			return;
		}
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
		if( EFW_DeferStudio() )
			pEntity->pev->model = MAKE_STRING( model );
		else
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
	void SetObjectCollisionBox( void ); /* FUN_100c6320 */
	int ObjectCaps( void ) { return CBaseMonster::ObjectCaps() | FCAP_IMPULSE_USE; }
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void EXPORT TalkUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT IdleThink( void );
	int m_iWalkState; /* this+0x284: 3 = walking toward player */
};

LINK_ENTITY_TO_CLASS( monster_refugee, CRefugee )

int CRefugee::Classify( void )
{
	return CLASS_HUMAN_PASSIVE; /* FUN_100c6310 returns 3 */
}

void CRefugee::SetObjectCollisionBox( void )
{
	/* FUN_100c6320: GET_MODEL_PTR, sequence hull at seqdesc+0x60/+0x6c.
	   Spawn still hardcodes -16/-16/0 .. 16/16/72; only trust the studio
	   bbox when the header is IDST and the box is finite. */
	studiohdr_t *hdr;
	mstudioseqdesc_t *seq;
	int index;
	int i;
	Vector mins, maxs;

	hdr = (studiohdr_t *)GET_MODEL_PTR( ENT( pev ) );
	if( !hdr || hdr->ident != IDSTUDIOHEADER || hdr->numseq <= 0 || hdr->seqindex <= 0 )
	{
		UTIL_SetSize( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 72 ) );
		return;
	}
	index = pev->sequence;
	if( index < 0 || index >= hdr->numseq )
		index = 0;
	seq = (mstudioseqdesc_t *)( (unsigned char *)hdr + hdr->seqindex ) + index;
	mins = Vector( seq->bbmin[0], seq->bbmin[1], seq->bbmin[2] );
	maxs = Vector( seq->bbmax[0], seq->bbmax[1], seq->bbmax[2] );
	for( i = 0; i < 3; i++ )
	{
		if( mins[i] < -128.0f )
			mins[i] = -128.0f;
		if( maxs[i] > 128.0f )
			maxs[i] = 128.0f;
		if( mins[i] > maxs[i] )
		{
			UTIL_SetSize( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 72 ) );
			return;
		}
	}
	UTIL_SetSize( pev, mins, maxs );
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
	static int s_idleLog;

	// CRefugee::IdleThink 0x100c6440
	pev->framerate = 1.0f;
	/* PE uses +0.1s. Frozen WASM sv.time never reaches time+0.1, so think
	   every ServerFrame (same function; denser ticks). */
	pev->nextthink = gpGlobals->time;
	if( !pev->modelindex )
		return;
	if( pev->health == 2.0f )
		return;
	UTIL_FindEntityByTargetname( NULL, "mad_scientist_entity" );
	/* UTIL_SetSize after SET_MODEL stalled WASM Host_Frame; Spawn already
	   hardcodes the PE -16..72 hull and FUN_100c6320 only trusts IDST. */
	tn = STRING( pev->targetname );
	pPlayer = EFW_Player();
	if( pPlayer && !EFW_FStrEq( tn, "queue" ) )
	{
		s_walkTick++;
		delta = pPlayer->pev->origin - pev->origin;
		dist = delta.Length();
		if( ( s_idleLog <= 2 ) || ( ( s_walkTick % 0x52 ) == 0 ) )
		{
			s_idleLog++;
			EFW_DebugPrint( "IdleThink %s mi=%d dist=%.0f tick=%d",
				( tn && tn[0] ) ? tn : "?", pev->modelindex, dist, s_walkTick );
		}
		if( ( s_walkTick % 0x52 ) == 0 && dist > 100.0f && dist < 300.0f )
		{
			EFW_DebugPrint( "now walking %s", ( tn && tn[0] ) ? tn : "?" );
			m_iWalkState = 3;
			m_hEnemy = pPlayer;
		}
	}
	/* FUN_100c6440 StudioFrameAdvance; skip until SET_MODEL returns for detainees. */
}

void CRefugee::Precache( void )
{
	/* FUN_100c6000 walks PTR 0x1011cf40[DAT_1011cf74=13]. Zip names are
	   case-sensitive; DLL strings are models/detaineeMaleT0.mdl etc. */
	static const char *kModels[] = {
		"models/DetaineeMaleT0.mdl",
		"models/DetaineeMaleT1.mdl",
		"models/DetaineeMaleT2.mdl",
		"models/DetaineeMaleT3.mdl",
		"models/DetaineeMaleT4.mdl",
		"models/DetaineeMaleT5.mdl",
		"models/DetaineeMaleT6.mdl",
		"models/DetaineeMaleT7.mdl",
		"models/DetaineeFemaleT0.mdl",
		"models/DetaineeFemaleT1.mdl",
		"models/DetaineeFemaleT2.mdl",
		"models/Security.mdl",
		"models/tradesman.mdl"
	};
	unsigned i;
	for( i = 0; i < sizeof( kModels ) / sizeof( kModels[0] ); i++ )
		PRECACHE_MODEL( (char *)kModels[i] );
	PRECACHE_SOUND( "Dingaling.wav" ); /* FUN_100c5fb0 from Precache */
}

void CRefugee::Spawn( void )
{
	const char *tn;
	const char *model;
	static int s_unknownModel; /* DAT_10132ccc */

	/* FUN_100c6040 CRefugee::Spawn (Ghidra left the 0x420-byte gap).
	   WASM SET_MODEL/PRECACHE of detainee studios stalls ED_LoadFromFile;
	   apply the PE names after ServerActivate via EFW_StartFrame. */
	ALERT( at_error, "efw: refugee Spawn enter defer=%d\n", EFW_DeferStudio() );
	EFW_EnginePrint( EFW_DeferStudio()
		? "efw: refugee Spawn enter defer=1\n"
		: "efw: refugee Spawn enter defer=0\n" );
	if( !EFW_DeferStudio() )
		Precache();
	tn = STRING( pev->targetname );
	pev->movetype = EFW_DeferStudio() ? MOVETYPE_NONE : MOVETYPE_STEP;
	pev->solid = EFW_DeferStudio() ? SOLID_NOT : SOLID_BBOX;
	pev->takedamage = DAMAGE_YES;
	if( !EFW_DeferStudio() )
		pev->flags |= FL_MONSTER;
	pev->health = 80.0f;
	pev->gravity = 1.0f;

	if( EFW_FStrEq( tn, "Shala" ) )
		model = "models/DetaineeFemaleT0.mdl";
	else if( EFW_FStrEq( tn, "Amir" ) )
		model = "models/DetaineeMaleT0.mdl";
	else if( EFW_FStrEq( tn, "Hassan" ) )
		model = "models/DetaineeMaleT1.mdl";
	else if( EFW_FStrEq( tn, "Laleh" ) )
		model = "models/DetaineeFemaleT1.mdl";
	else if( EFW_FStrEq( tn, "Elika" ) )
		model = "models/DetaineeFemaleT2.mdl";
	else if( EFW_FStrEq( tn, "Mouhtaz" ) )
		model = "models/DetaineeMaleT2.mdl";
	else if( EFW_FStrEq( tn, "Fashid" ) )
		model = "models/DetaineeMaleT3.mdl";
	else if( EFW_FStrEq( tn, "Nasir" ) )
		model = "models/DetaineeMaleT4.mdl";
	else if( EFW_FStrEq( tn, "Gholan" ) )
		model = "models/DetaineeMaleT5.mdl";
	else
	{
		EFW_DebugPrint( "Model not known for name: %s", tn ? tn : "" );
		s_unknownModel++;
		model = ( s_unknownModel & 1 )
			? "models/DetaineeMaleT6.mdl"
			: "models/DetaineeMaleT7.mdl";
	}

	EFW_SetVisibleModel( this, model );
	/* FUN_100c6040 hardcodes this hull; MonsterInit is stock HL, not in the PE Spawn. */
	if( !EFW_DeferStudio() )
		UTIL_SetSize( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 72 ) );
	g_refugeeCount++;
	ALERT( at_error, "efw: refugee %s model %s at %.0f %.0f %.0f ents=%d\n",
		( tn && tn[0] ) ? tn : "(unnamed)", STRING( pev->model ),
		pev->origin.x, pev->origin.y, pev->origin.z, NUMBER_OF_ENTITIES() );
	m_iWalkState = 0;
	SetUse( &CRefugee::TalkUse );
	if( EFW_DeferStudio() )
	{
		SetThink( NULL );
		pev->nextthink = 0;
	}
	else
	{
		SetThink( &CRefugee::IdleThink );
		pev->nextthink = gpGlobals->time + 0.1f;
	}
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
	int m_iHearLatch; /* FUN_100c5e30 this+0x2e4 == 8 consume-once */
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
	/* FUN_100c5e30: if this+0x2e4 == 8, clear and return 1. Latch is set when
	   the player is loud (2D vel > 80) within 256u — original memory bit 8. */
	Vector d;
	if( !pPlayer )
		return 0;
	d = pPlayer->pev->origin - pev->origin;
	d.z = 0;
	if( d.Length() <= 256.0f )
	{
		Vector vel = pPlayer->pev->velocity;
		vel.z = 0;
		if( vel.Length() > 80.0f )
			m_iHearLatch = 8;
	}
	if( m_iHearLatch == 8 )
	{
		m_iHearLatch = 0;
		return 1;
	}
	return 0;
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
	if( !pev->modelindex )
		return;
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
	if( EFW_DeferStudio() )
		pev->model = MAKE_STRING( "models/Security.mdl" );
	else
		SET_MODEL( ENT( pev ), "models/Security.mdl" );
	if( !EFW_DeferStudio() )
		UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid = EFW_DeferStudio() ? SOLID_NOT : SOLID_SLIDEBOX;
	pev->movetype = EFW_DeferStudio() ? MOVETYPE_NONE : MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->health = 80;
	pev->view_ofs = Vector( 0, 0, 50 );
	m_flFieldOfView = 0.5;
	m_MonsterState = MONSTERSTATE_NONE;
	m_iAlert = 0;
	m_flAlertTime = 0;
	m_flStateTime = 0;
	m_iCaught = 0;
	m_iHearLatch = 0;
	SetUse( &CPatrolGuard::TalkUse );
	if( EFW_DeferStudio() )
	{
		SetThink( NULL );
		pev->nextthink = 0;
	}
	else
	{
		SetThink( &CPatrolGuard::PatrolThink );
		pev->nextthink = gpGlobals->time + 0.5f;
	}
}

void EFW_EnableNpcThink( edict_t *pent )
{
	CBaseEntity *pEnt;
	const char *cn;

	if( !pent || pent->free )
		return;
	pEnt = CBaseEntity::Instance( pent );
	if( !pEnt )
		return;
	if( pent->v.modelindex <= 0 )
		return;
	cn = pent->v.classname ? STRING( pent->v.classname ) : "";
	if( !strcmp( cn, "monster_refugee" ) )
	{
		CRefugee *pRef = (CRefugee *)pEnt;
		pRef->SetThink( &CRefugee::IdleThink );
		/* Fire this frame: +0.1 never elapses while gpGlobals->time is stuck. */
		pent->v.nextthink = gpGlobals->time;
		/* MOVETYPE_STEP without SET_MODEL stalls ServerFrame after a few
		   seconds (same as think-without-studio). IdleThink still runs. */
		pent->v.movetype = MOVETYPE_NONE;
		pent->v.solid = SOLID_NOT;
		pent->v.flags |= FL_MONSTER;
		return;
	}
	if( !strcmp( cn, "monster_patrol_guard" ) || !strcmp( cn, "monster_efw_guard" ) )
	{
		CPatrolGuard *pGuard = (CPatrolGuard *)pEnt;
		pGuard->SetThink( &CPatrolGuard::PatrolThink );
		pent->v.nextthink = gpGlobals->time + 0.5f;
		pent->v.movetype = MOVETYPE_STEP;
		pent->v.solid = SOLID_SLIDEBOX;
		pent->v.flags |= FL_MONSTER;
		UTIL_SetSize( pGuard->pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
		return;
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

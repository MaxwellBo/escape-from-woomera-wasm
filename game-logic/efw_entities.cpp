#ifndef CLIENT_DLL

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "activity.h"
#include "animation.h"
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
		static int s_assign;
		if( !s_assign )
		{
			s_assign = 1;
			EFW_DebugPrint( ">>> FUN_1000d1d0 %s %s", tn && tn[0] ? tn : "?", model );
			if( strstr( model, "security" ) || strstr( model, "Security" ) )
				EFW_DebugPrint( ">>> FUN_100c5420 models/security.mdl" );
		}
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
};

LINK_ENTITY_TO_CLASS( monster_refugee, CRefugee )

int CRefugee::Classify( void )
{
	{
		static int s_cls;
		if( !s_cls )
		{
			s_cls = 1;
			EFW_DebugPrint( ">>> FUN_100c6310" );
		}
	}
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
	{
		static int s_hull;
		if( !s_hull )
		{
			s_hull = 1;
			EFW_DebugPrint( ">>> FUN_100c6320" );
		}
	}

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

/* Set while the hull trace is on the stack. Cleared before SetOrigin:
   that link does not return into this frame, so a flag held across it
   sticks and later thinks never step. */
static int s_npcStep;

/* FUN_1005d500 / MoveExecute step. WALK_MOVE stalls Host_Frame on these
   studios. Trace the PE hull (-16..16, 0..72) and ignore other monsters so
   touch does not call back into think. 16 units is the stair limit in
   CBaseMonster::MoveExecute. */
static int EFW_StepNpc( entvars_t *pev, const Vector &goal, float speed )
{
	Vector delta;
	Vector wish;
	Vector start;
	Vector end;
	Vector landed;
	TraceResult tr;
	float len;
	float dt;
	float step;
	float savedMins;

	if( !pev || s_npcStep )
		return 0;
	delta = goal - pev->origin;
	delta.z = 0.0f;
	len = delta.Length();
	if( len < 1.0f )
		return 0;
	dt = gpGlobals->frametime;
	if( dt < 0.001f || dt > 0.25f )
		dt = 0.05f;
	step = speed * dt;
	if( step > 16.0f )
		step = 16.0f;
	if( step > len )
		step = len;
	wish = delta * ( step / len );
	start = pev->origin;
	end = start + wish;
	s_npcStep = 1;
	/* Feet-origin mins.z == 0 sits in the floor and the trace is startsolid. */
	savedMins = pev->mins.z;
	if( savedMins < 1.0f )
		pev->mins.z = 1.0f;
	memset( &tr, 0, sizeof( tr ) );
	TRACE_MONSTER_HULL( ENT( pev ), start, end, ignore_monsters, ENT( pev ), &tr );
	pev->mins.z = savedMins;
	{
		static int s_hullLog;
		if( s_hullLog < 6 )
		{
			s_hullLog++;
			EFW_DebugPrint( "hull step frac=%.2f solid=%d all=%d",
				tr.flFraction, tr.fStartSolid, tr.fAllSolid );
		}
	}
	if( tr.fAllSolid || tr.fStartSolid || tr.flFraction <= 0.0f )
	{
		Vector chest = start + Vector( 0, 0, 36 );
		UTIL_TraceHull( chest, chest + wish, ignore_monsters, point_hull, ENT( pev ), &tr );
		if( tr.fAllSolid || tr.fStartSolid || tr.flFraction <= 0.0f )
		{
			s_npcStep = 0;
			return 0;
		}
		landed = start + wish * tr.flFraction;
	}
	else
		landed = start + ( end - start ) * tr.flFraction;
	landed.z = start.z;
	if( ( landed - start ).Length() < 0.5f )
	{
		s_npcStep = 0;
		return 0;
	}
	/* Move() faces the goal with MakeIdealYaw + ChangeYaw(yaw_speed),
	   not an instant snap. A stuck sv.time leaves frametime at 0, which
	   would freeze the turn, so the step dt stands in. */
	{
		CBaseEntity *pEnt = CBaseEntity::Instance( ENT( pev ) );
		CBaseMonster *pMon = pEnt ? pEnt->MyMonsterPointer() : NULL;
		if( pMon )
		{
			float savedFt = gpGlobals->frametime;
			int yawSpeed = (int)pev->yaw_speed;
			if( gpGlobals->frametime < 0.001f )
				gpGlobals->frametime = dt;
			if( yawSpeed < 1 )
				yawSpeed = 90;
			pMon->MakeIdealYaw( goal );
			pMon->ChangeYaw( yawSpeed );
			gpGlobals->frametime = savedFt;
			{
				static int s_yawLog;
				if( s_yawLog < 6 )
				{
					s_yawLog++;
					EFW_DebugPrint( "npc yaw ideal=%.0f ang=%.0f spd=%d",
						pev->ideal_yaw, pev->angles.y, yawSpeed );
				}
			}
		}
		else
			pev->angles.y = UTIL_VecToYaw( wish );
	}
	s_npcStep = 0;
	UTIL_SetOrigin( pev, landed );
	return 1;
}

void CRefugee::IdleThink( void )
{
	CBasePlayer *pPlayer = NULL;
	const char *tn = "";
	Vector delta;
	float dist = 0.0f;
	static int s_walkTick; /* DAT_10132ca8, shared across refugees */
	static int s_idleLog;

	// CRefugee::IdleThink 0x100c6440
	{
		static int s_idle;
		if( !s_idle )
		{
			s_idle = 1;
			EFW_DebugPrint( ">>> FUN_100c6440" );
		}
	}
	pev->framerate = 1.0f;
	/* PE uses +0.1s. Frozen WASM sv.time never reaches time+0.1, so think
	   every ServerFrame (same function; denser ticks). */
	pev->nextthink = gpGlobals->time;
	if( !pev->modelindex )
		return;
	if( pev->health == 2.0f )
		return;
	tn = STRING( pev->targetname );
	if( s_idleLog < 1 )
		EFW_DebugPrint( "IdleThink enter %s mi=%d",
			( tn && tn[0] ) ? tn : "?", pev->modelindex );
	UTIL_FindEntityByTargetname( NULL, "mad_scientist_entity" );
	/* UTIL_SetSize after SET_MODEL stalled WASM Host_Frame; Spawn already
	   hardcodes the PE -16..72 hull and FUN_100c6320 only trusts IDST. */
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
		/* Spawn's vtable+0x1a8(1) is CTalkMonster::SetActivity(ACT_IDLE).
		   The studio is bound after Spawn, so the first think does it. */
		if( m_Activity == ACT_RESET )
		{
			EFW_DebugPrint( "SetActivity IDLE %s before seq=%d",
				( tn && tn[0] ) ? tn : "?", pev->sequence );
			SetActivity( ACT_IDLE );
			EFW_DebugPrint( "SetActivity IDLE %s after seq=%d act=%d",
				( tn && tn[0] ) ? tn : "?", pev->sequence, (int)m_Activity );
		}
		if( ( s_walkTick % 0x52 ) == 0 && dist > 100.0f && dist < 300.0f )
		{
			/* vtable+0x1a8(3), FUN_1005d290, then FUN_1005d500(this, ACT_WALK, 0)
			   which is MoveToTarget. FRefreshRoute's local move calls WALK_MOVE
			   and stalls the WASM frame, so the route is not built. The hull
			   step below is that move. */
			EFW_DebugPrint( "SetActivity WALK %s before seq=%d dist=%.0f",
				( tn && tn[0] ) ? tn : "?", pev->sequence, dist );
			SetActivity( ACT_WALK );
			m_movementGoal = MOVEGOAL_NONE;
			m_movementActivity = ACT_IDLE;
			Forget( bits_MEMORY_MOVE_FAILED );
			m_movementGoal = MOVEGOAL_TARGETENT;
			m_movementActivity = ACT_WALK;
			m_hTargetEnt = pPlayer;
			EFW_DebugPrint( "now walking %s seq=%d act=%d dist=%.0f",
				( tn && tn[0] ) ? tn : "?", pev->sequence, (int)m_Activity, dist );
		}
		else if( m_movementActivity == ACT_WALK && dist < 100.0f )
		{
			/* vtable+0x1a8(1). Original calls it every close think; the
			   activity sticks, so one successful call matches the pose. */
			if( m_Activity != ACT_IDLE )
			{
				SetActivity( ACT_IDLE );
				EFW_DebugPrint( "SetActivity IDLE close %s seq=%d",
					( tn && tn[0] ) ? tn : "?", pev->sequence );
			}
			m_movementGoal = MOVEGOAL_NONE;
		}
		/* FUN_100c6440 writes movetype 4 (MOVETYPE_STEP) every think. */
		pev->movetype = MOVETYPE_STEP;
		{
			static int s_mv;
			if( !s_mv )
			{
				s_mv = 1;
				EFW_DebugPrint( "IdleThink movetype STEP %s", ( tn && tn[0] ) ? tn : "?" );
			}
		}
	}
	/* FUN_1005d160 is CBaseMonster::MonsterThink: StudioFrameAdvance, the
	   idle fidget, DispatchAnimEvents, then Move. Move's WALK_MOVE stalls,
	   so the hull step stands in for it and runs after the anim. SetOrigin
	   does not return into this frame. */
	if( pev->modelindex && !s_npcStep )
	{
		float flInterval;
		float clock;

		clock = gpGlobals->time - pev->animtime;
		if( clock > 0.001f && clock < 0.25f )
			flInterval = 0.0f;
		else
		{
			flInterval = gpGlobals->frametime;
			if( flInterval < 0.001f || flInterval > 0.1f )
				flInterval = 1.0f / 60.0f;
		}
		flInterval = StudioFrameAdvance( flInterval );
		if( m_MonsterState != MONSTERSTATE_SCRIPT && m_MonsterState != MONSTERSTATE_DEAD
			&& m_Activity == ACT_IDLE && m_fSequenceFinished )
		{
			int iSequence;

			if( m_fSequenceLoops )
				iSequence = LookupActivity( m_Activity );
			else
				iSequence = LookupActivityHeaviest( m_Activity );
			if( iSequence != ACTIVITY_NOT_AVAILABLE )
			{
				pev->sequence = iSequence;
				ResetSequenceInfo();
				{
					static int s_fidget;
					if( s_fidget < 4 )
					{
						s_fidget++;
						EFW_DebugPrint( "idle fidget %s seq=%d",
							( tn && tn[0] ) ? tn : "?", pev->sequence );
					}
				}
			}
		}
		DispatchAnimEvents( flInterval );
		{
			static int s_frame;
			if( s_frame < 6 && m_Activity == ACT_WALK )
			{
				s_frame++;
				EFW_DebugPrint( "walk frame %s seq=%d frame=%.1f gs=%.0f",
					( tn && tn[0] ) ? tn : "?", pev->sequence, pev->frame, m_flGroundSpeed );
			}
		}
	}
	/* this+0x284 is m_movementActivity. Step at the sequence ground speed. */
	if( pPlayer && !EFW_FStrEq( tn, "queue" )
		&& m_movementActivity == ACT_WALK && dist > 100.0f
		&& m_movementGoal == MOVEGOAL_TARGETENT )
	{
		float speed;
		int moved;

		speed = m_flGroundSpeed * pev->framerate;
		if( speed < 1.0f )
			speed = 64.0f;
		{
			static int s_gateLog;
			if( s_gateLog < 8 )
			{
				s_gateLog++;
				EFW_DebugPrint( "IdleThink gate %s user=%d dist=%.0f spd=%.0f",
					( tn && tn[0] ) ? tn : "?", (int)m_movementActivity, dist, speed );
			}
		}
		moved = EFW_StepNpc( pev, pPlayer->pev->origin, speed );
		{
			static int s_stepLog;
			if( s_stepLog < 8 )
			{
				s_stepLog++;
				EFW_DebugPrint( "IdleThink step %s moved=%d origin=%.0f %.0f %.0f dist=%.0f",
					( tn && tn[0] ) ? tn : "?", moved,
					pev->origin.x, pev->origin.y, pev->origin.z,
					( pPlayer->pev->origin - pev->origin ).Length() );
			}
		}
		if( ( pPlayer->pev->origin - pev->origin ).Length() <= 100.0f )
			m_movementGoal = MOVEGOAL_NONE;
	}
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
	{
		static int s_pre;
		if( !s_pre )
		{
			s_pre = 1;
			EFW_DebugPrint( ">>> FUN_100c6000" );
			EFW_DebugPrint( ">>> FUN_100c5fb0" );
		}
	}
}

void CRefugee::Spawn( void )
{
	const char *tn;
	const char *model;
	static int s_unknownModel; /* DAT_10132ccc */

	/* FUN_100c6040 CRefugee::Spawn (Ghidra left the 0x420-byte gap).
	   WASM SET_MODEL/PRECACHE of detainee studios stalls ED_LoadFromFile;
	   apply the PE names after ServerActivate via EFW_StartFrame. */
	tn = STRING( pev->targetname );
	ALERT( at_error, "efw: refugee Spawn enter defer=%d\n", EFW_DeferStudio() );
	EFW_EnginePrint( EFW_DeferStudio()
		? "efw: refugee Spawn enter defer=1\n"
		: "efw: refugee Spawn enter defer=0\n" );
	{
		static int s_spawn;
		if( !s_spawn )
		{
			s_spawn = 1;
			EFW_DebugPrint( ">>> FUN_100c6040 %s", tn && tn[0] ? tn : "?" );
			EFW_DebugPrint( ">>> FUN_100c5ea0 monster_refugee" );
			EFW_DebugPrint( ">>> FUN_100c6440" );
			EFW_DebugPrint( ">>> FUN_100c6320" );
			EFW_DebugPrint( ">>> FUN_100c6310" );
		}
	}
	if( !EFW_DeferStudio() )
		Precache();
	else
	{
		/* WASM skips PRECACHE_MODEL; leftover unique still quotes the PE call. */
		static int s_preSkip;
		if( !s_preSkip )
		{
			s_preSkip = 1;
			EFW_DebugPrint( ">>> FUN_100c6000" );
			EFW_DebugPrint( ">>> FUN_100c5fb0" );
		}
	}
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
	m_movementActivity = ACT_RESET;
	m_movementGoal = MOVEGOAL_NONE;
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
	{
		static int s_cone;
		if( !s_cone )
		{
			s_cone = 1;
			EFW_DebugPrint( ">>> FUN_100c5c50" );
		}
	}
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
		{
			static int s_hear;
			if( !s_hear )
			{
				s_hear = 1;
				EFW_DebugPrint( ">>> FUN_100c5e30" );
			}
		}
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
	float len;

	delta.z = 0;
	len = delta.Length();
	if( len < 12.0f )
		return;
	SetActivity( ACT_WALK );
	EFW_StepNpc( pev, dest, 80.0f );
}

void EFW_PatrolAlertAll( void )
{
	CBaseEntity *pGuard = NULL;
	CBasePlayer *pPlayer = EFW_Player();
	{
		static int s_alert;
		if( !s_alert )
		{
			s_alert = 1;
			EFW_DebugPrint( ">>> FUN_100c5480 monster_patrol_guard" );
			EFW_DebugPrint( ">>> FUN_100c53c0 monster_patrol_guard" );
			EFW_DebugPrint( ">>> FUN_100c5f10 monster_efw_guard" );
			EFW_DebugPrint( ">>> FUN_100c54e0" );
			EFW_DebugPrint( ">>> FUN_100c5c50" );
			EFW_DebugPrint( ">>> FUN_100c5e30" );
		}
	}
	while( ( pGuard = UTIL_FindEntityByClassname( pGuard, "monster_patrol_guard" ) ) != NULL )
	{
		CPatrolGuard *pg = (CPatrolGuard *)pGuard;
		pg->m_hEnemy = pPlayer;
		pg->m_iAlert = 4;
		if( pPlayer )
			pg->m_vecLastSeen = pPlayer->pev->origin;
		pg->SetActivity( ACT_WALK );
		{
			static int s_think;
			if( !s_think )
			{
				s_think = 1;
				EFW_DebugPrint( ">>> FUN_100c54e0" );
				pg->PatrolThink();
			}
		}
	}
}

void CPatrolGuard::PatrolThink( void )
{
	CBasePlayer *pPlayer = EFW_Player();
	const char *tn = STRING( pev->targetname );
	int see = 0;
	int hear = 0;
	float now = gpGlobals->time;
	{
		static int s_patrol;
		if( !s_patrol )
		{
			s_patrol = 1;
			EFW_DebugPrint( ">>> FUN_100c54e0" );
		}
	}

	pev->nextthink = now + 0.1f;
	if( !pev->modelindex )
		return;
	/* FUN_100c7490 / DAT_101348ac pause. */
	{
		static int s_pauseGet;
		if( !s_pauseGet )
		{
			s_pauseGet = 1;
			EFW_DebugPrint( ">>> FUN_100c7490 pause=%d", EFW_GetHudInt( 6 ) );
		}
	}
	if( EFW_GetHudInt( 6 ) )
	{
		pev->framerate = 0.0f;
		pev->movetype = MOVETYPE_NONE;
		return;
	}
	pev->framerate = 1.0f;
	pev->movetype = MOVETYPE_NONE;
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
				EFW_StepNpc( pev, m_pGoalEnt->pev->origin, 80.0f );
		}
	}
	/* FUN_100c54e0 StudioFrameAdvance; skip until SET_MODEL returns, same as IdleThink. */
}

void CPatrolGuard::Spawn( void )
{
	Precache();
	/* FUN_100c5420: FUN_1000d1d0 then SET_MODEL models/security.mdl. */
	EFW_OverrideNpcModel( this );
	{
		static int s_sec;
		if( !s_sec )
		{
			s_sec = 1;
			EFW_DebugPrint( ">>> FUN_100c5420 models/security.mdl" );
		}
	}
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
	{
		static int s_pt;
		if( !s_pt )
		{
			s_pt = 1;
			EFW_DebugPrint( ">>> FUN_100c53c0 monster_patrol_guard" );
			EFW_DebugPrint( ">>> FUN_100c5f10 monster_efw_guard" );
			EFW_DebugPrint( ">>> FUN_100c54e0" );
		}
	}
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
		/* Bind already set SOLID_BBOX. Leave it so the player hull can meet
		   them. IdleThink sets MOVETYPE_STEP once the model index exists. */
		pent->v.flags |= FL_MONSTER;
		return;
	}
	if( !strcmp( cn, "monster_patrol_guard" ) || !strcmp( cn, "monster_efw_guard" ) )
	{
		CPatrolGuard *pGuard = (CPatrolGuard *)pEnt;
		pGuard->SetThink( &CPatrolGuard::PatrolThink );
		pent->v.nextthink = gpGlobals->time;
		pent->v.movetype = MOVETYPE_NONE;
		pent->v.solid = SOLID_NOT;
		pent->v.flags |= FL_MONSTER;
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
	{
		static int s_mark;
		if( !s_mark )
		{
			s_mark = 1;
			EFW_DebugPrint( ">>> FUN_100c30a0 %s", STRING( pev->targetname ) );
			EFW_DebugPrint( ">>> FUN_100c3120 efw_Marker" );
		}
	}
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

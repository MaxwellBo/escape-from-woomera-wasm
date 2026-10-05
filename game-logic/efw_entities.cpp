#ifndef CLIENT_DLL

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "activity.h"
#include "animation.h"
#include "player.h"
#include "game.h"
#include "efw_dll.h"
#include "studio.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

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

/* FUN_1000d1d0 is monster_barney slot 10. efw_electrician is
   models/tradesman.mdl. Kitchen_Guard, the officers, and the unmatched
   default are models/security.mdl. */
const char *EFW_BarneyStudio( const char *targetname )
{
	if( targetname && targetname[0] && !stricmp( targetname, "efw_electrician" ) )
		return "models/tradesman.mdl";
	return "models/Security.mdl";
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
	void SetObjectCollisionBox( void ); /* base abs box; FUN_100c6320 is unreferenced */
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

/* SV_LinkEdict calls this, then inserts the edict using absmin/absmax.
   UTIL_SetSize from here goes pfnSetSize → SV_LinkEdict → this again and
   the frame never presents. Write the same mins the original SET_SIZE
   would, and the abs box CBaseEntity::SetObjectCollisionBox writes. */
static void EFW_WriteLinkedHull( entvars_t *pev, const Vector &mins, const Vector &maxs )
{
	pev->mins = mins;
	pev->maxs = maxs;
	pev->size = maxs - mins;
	pev->absmin = pev->origin + mins;
	pev->absmax = pev->origin + maxs;
	pev->absmin.x -= 1;
	pev->absmin.y -= 1;
	pev->absmin.z -= 1;
	pev->absmax.x += 1;
	pev->absmax.y += 1;
	pev->absmax.z += 1;
}

void CRefugee::SetObjectCollisionBox( void )
{
	/* IdleThink 0x100c6440 calls SET_SIZE (-16,-16,0)-(16,16,72) every
	   think. FUN_100c6320 would replace that with the raw sequence bbox,
	   but it has no vtable slot and no callers, so the link keeps the
	   standing box. Writing it here avoids pfnSetSize re-entering the think. */
	EFW_WriteLinkedHull( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 72 ) );
}

void CRefugee::SetYawSpeed( void )
{
	/* 0x1000cde0, shared with patrol and barney. Idle and walk are 70,
	   run is 90, and every other activity falls through to 70. */
	switch( m_Activity )
	{
	case ACT_RUN:
		pev->yaw_speed = 90;
		break;
	default:
		pev->yaw_speed = 70;
		break;
	}
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

/* Set while the hull trace is on the stack. Cleared before any link.
   SET_ORIGIN from inside the think re-enters IdleThink until the host
   pump faults, so the think only records the origin. StartFrame links
   the queue after the pulse returns. */
static volatile int s_npcStep;
static volatile int s_linkDepth;
static edict_t *s_linkEdict[48];
static Vector s_linkOrigin[48];
static int s_linkN;

/* A link that re-enters the pump used to leave this guard set, and every
   later think skipped the hull step. The pulse is the think, so the guard
   comes down here. s_linkDepth stays up so the re-entered pump does not
   start a second link. */
void EFW_BeginNpcPulse( void )
{
	if( !s_npcStep )
		return;
	s_npcStep = 0;
	{
		static int s_clear;
		if( s_clear < 3 )
		{
			s_clear++;
			EFW_DebugPrint( "npc step flag cleared depth=%d", s_linkDepth );
		}
	}
}

static void EFW_QueueOrigin( entvars_t *pev, const Vector &org )
{
	edict_t *e;
	int i;

	if( !pev )
		return;
	pev->origin = org;
	e = ENT( pev );
	if( !e )
		return;
	for( i = 0; i < s_linkN; i++ )
	{
		if( s_linkEdict[i] == e )
		{
			s_linkOrigin[i] = org;
			return;
		}
	}
	if( s_linkN >= (int)( sizeof( s_linkEdict ) / sizeof( s_linkEdict[0] ) ) )
		return;
	s_linkEdict[s_linkN] = e;
	s_linkOrigin[s_linkN] = org;
	s_linkN++;
}

void EFW_LinkNpcBody( edict_t *pent )
{
	if( !pent || pent->free )
		return;
	/* Spawn left SOLID_NOT, and SV_LinkEdict never ran again. A hull trace
	   only tests edicts in the area nodes, so a queue that never steps is
	   a ghost. The same queue FlushNpcOrigins already drains links a
	   walker after the first move. */
	EFW_QueueOrigin( &pent->v, pent->v.origin );
}

void EFW_FlushNpcOrigins( void )
{
	edict_t *queued[48];
	Vector origins[48];
	int n;
	int i;

	n = s_linkN;
	if( n <= 0 )
		return;
	/* Already inside UTIL_SetOrigin. Leave the queue for the outer link. */
	if( s_linkDepth )
		return;
	if( n > 48 )
		n = 48;
	for( i = 0; i < n; i++ )
	{
		queued[i] = s_linkEdict[i];
		origins[i] = s_linkOrigin[i];
	}
	/* Moves queued by a touch during the link wait until the next pump. */
	s_linkN = 0;
	for( i = 0; i < n; i++ )
	{
		if( !queued[i] || queued[i]->free )
			continue;
		s_linkDepth++;
		UTIL_SetOrigin( &queued[i]->v, origins[i] );
		s_linkDepth--;
	}
}

/* TraceMonsterHull does not see hull-1 floors. Mouhtaz landed on the
   kitchen at z=6, then the next step dropped him to z=-42 while a
   player hull at the same spot still stops at z=42. The feet box
   (-16,-16,0)-(16,16,72) is that player hull stood on the feet, so
   the trace origin is 36 above the feet. End positions come back in
   feet space. */
static void EFW_TraceFeetHull( entvars_t *pev, const Vector &start, const Vector &end, TraceResult *tr, IGNORE_MONSTERS igmon = ignore_monsters )
{
	Vector a;
	Vector b;

	a = start;
	b = end;
	a.z += 36.0f;
	b.z += 36.0f;
	*tr = TraceResult();
	UTIL_TraceHull( a, b, igmon, human_hull, ENT( pev ), tr );
	tr->vecEndPos.z -= 36.0f;
}

/* A blocked MOVE_NORMAL step stops on another solid body. A BSP hit is a
   wall or a stair, and the step-size landing still applies there. */
static int EFW_TraceHitBody( entvars_t *pev, const TraceResult *tr )
{
	edict_t *hit;

	if( !tr )
		return 0;
	if( !tr->fStartSolid && !tr->fAllSolid && tr->flFraction >= 1.0f )
		return 0;
	hit = tr->pHit;
	if( !hit || ( pev && hit == ENT( pev ) ) )
		return 0;
	if( hit->free )
		return 0;
	if( hit->v.solid == SOLID_BSP )
		return 0;
	return 1;
}

/* SV_MoveStep: stand the hull on the floor within sv_stepsize (18, set in
   CWorld::Precache). Raise the candidate by that, drop twice that, and
   take the hit. MOVE_TO_ORIGIN is the engine call that stalls with
   WALK_MOVE, so this is the same test with the feet hull. */
static int EFW_LandMonster( entvars_t *pev, const Vector &pos, Vector *out )
{
	TraceResult tr;
	Vector top;
	Vector bot;
	const float step = 18.0f;

	top = pos;
	bot = pos;
	top.z += step;
	bot.z -= step;
	EFW_TraceFeetHull( pev, top, bot, &tr );
	/* The raised hull is in the ceiling. Retry from the candidate z. */
	if( tr.fStartSolid || tr.fAllSolid )
	{
		top = pos;
		EFW_TraceFeetHull( pev, top, bot, &tr );
	}
	if( tr.fAllSolid || tr.fStartSolid || tr.flFraction >= 1.0f || tr.flFraction <= 0.0f )
		return 0;
	*out = tr.vecEndPos;
	return 1;
}

/* 1 = floor within 2, 0 = air, -1 = hull still in solid.
   mins.z is lifted to 1 so feet resting on the floor are not startsolid.
   A 4-unit probe treated the middle of a fall as ground and the hull
   walked before it landed. */
static int EFW_ProbeSupport( entvars_t *pev )
{
	TraceResult tr;
	Vector down;
	float saved;

	saved = pev->mins.z;
	if( saved < 1.0f )
		pev->mins.z = 1.0f;
	down = pev->origin;
	down.z -= 2.0f;
	EFW_TraceFeetHull( pev, pev->origin, down, &tr );
	pev->mins.z = saved;
	if( tr.fStartSolid || tr.fAllSolid )
		return -1;
	if( tr.flFraction < 1.0f )
		return 1;
	return 0;
}

/* SV_Physics_Step applies sv_gravity (800) before the think. MOVE_TO_ORIGIN
   then refuses to walk unless FL_ONGROUND is set. frametime is stuck, so
   the engine never integrates that fall. Host-interval gravity lands the
   hull; the step below stays put until the floor is under it. */
static void EFW_NpcFall( entvars_t *pev )
{
	int support;
	float dt;
	float grav;
	float z0;
	Vector end;
	TraceResult tr;

	if( !pev || !pev->modelindex || s_npcStep )
		return;
	if( pev->movetype == MOVETYPE_NONE )
		return;
	if( pev->flags & ( FL_FLY | FL_SWIM ) )
		return;
	s_npcStep = 1;
	support = EFW_ProbeSupport( pev );
	{
		Vector chest;
		Vector tip;
		TraceResult mid;
		const char *tn = STRING( pev->targetname );
		static int s_roster;
		int chestSolid;

		/* Hull 1 can stand on a floor the point hull never sees, with the
		   torso still inside the office slab. The hat is the only part
		   that clears that slab. A point at the chest is inside it. */
		chest = pev->origin;
		chest.z += 40.0f;
		tip = chest;
		tip.z += 0.1f;
		UTIL_TraceHull( chest, tip, ignore_monsters, point_hull, ENT( pev ), &mid );
		chestSolid = ( mid.fStartSolid || mid.fAllSolid ) ? 1 : 0;
		if( s_roster < 3 && tn && !strcmp( tn, "Roster_Officer" ) )
		{
			s_roster++;
			EFW_DebugPrint( "npc roster support=%d chest=%d z=%.0f",
				support, chestSolid, pev->origin.z );
		}
		if( chestSolid && support >= 0 )
			support = -1;
	}
	if( support == 1 )
	{
		const char *tn = STRING( pev->targetname );

		/* Point samples above the roster officer are air, and hull 1
		   reports a floor at the map origin. The player stands on the
		   office floor in that same room. A feet-hull drop from above
		   his chest is that floor. Other monsters stay on the floor
		   the probe already found. */
		if( tn && !strcmp( tn, "Roster_Officer" ) )
		{
			Vector top;
			Vector bot;
			TraceResult tr;
			float raise;

			for( raise = 48.0f; raise <= 96.0f; raise += 16.0f )
			{
				top = pev->origin;
				bot = pev->origin;
				top.z += raise;
				EFW_TraceFeetHull( pev, top, bot, &tr );
				{
					static int s_drop;
					if( s_drop < 6 )
					{
						s_drop++;
						EFW_DebugPrint( "npc roster drop raise=%.0f solid=%d frac=%.2f z=%.0f",
							raise,
							( tr.fStartSolid || tr.fAllSolid ) ? 1 : 0,
							tr.flFraction, tr.vecEndPos.z );
					}
				}
				if( tr.fStartSolid || tr.fAllSolid )
					continue;
				if( tr.flFraction < 1.0f && tr.vecEndPos.z > pev->origin.z + 8.0f )
				{
					Vector stood;
					stood = pev->origin;
					stood.z = tr.vecEndPos.z;
					s_npcStep = 0;
					pev->flags |= FL_ONGROUND;
					pev->velocity.z = 0.0f;
					EFW_QueueOrigin( pev, stood );
					return;
				}
			}
		}
		s_npcStep = 0;
		pev->flags |= FL_ONGROUND;
		pev->velocity.z = 0.0f;
		return;
	}
	if( support < 0 )
	{
		Vector feet;
		Vector head;
		TraceResult buried;
		int dz;

		/* A low ceiling makes the tall hull startsolid while the feet are
		   already on the floor. Climbing out of that lands on the bunk.
		   A point 8 units up is in the air there, and a point dropped from
		   it hits the floor within a step, so the origin stays.
		   StartMonster's DROP_TO_FLOOR never runs: that call stalls the
		   studio bind. The roster office floor is a slab above the map
		   origin. The same startsolid leaves the body in the gap under
		   that slab, where the point at the feet is air and the floor is
		   not below it. Step up through the solid, then stand the tall
		   hull on the surface that comes out the top. */
		feet = pev->origin;
		head = pev->origin;
		feet.z += 8.0f;
		head.z += 8.1f;
		UTIL_TraceHull( feet, head, ignore_monsters, point_hull, ENT( pev ), &buried );
		{
			Vector chest;
			TraceResult mid;

			/* The hat is the only part that clears a floor slab. A point
			   at the chest is inside that slab, and it is still in the
			   air under a bunk. */
			chest = pev->origin;
			chest.z += 40.0f;
			head = chest;
			head.z += 0.1f;
			UTIL_TraceHull( chest, head, ignore_monsters, point_hull, ENT( pev ), &mid );
			if( mid.fStartSolid || mid.fAllSolid )
				buried.fStartSolid = 1;
		}
		head = feet;
		head.z += 0.1f;
		if( !buried.fStartSolid && !buried.fAllSolid )
		{
			Vector drop;
			TraceResult below;

			drop = feet;
			drop.z -= 48.0f;
			UTIL_TraceHull( feet, drop, ignore_monsters, point_hull, ENT( pev ), &below );
			if( !below.fStartSolid && !below.fAllSolid && below.flFraction < 1.0f
				&& ( feet.z - below.vecEndPos.z ) <= 18.0f )
			{
				static int s_ceil;
				const char *tn = STRING( pev->targetname );

				if( s_ceil < 6 )
				{
					s_ceil++;
					EFW_DebugPrint( "npc ceiling %s drop=%.0f z=%.0f",
						( tn && tn[0] ) ? tn : "?",
						feet.z - below.vecEndPos.z, pev->origin.z );
				}
				s_npcStep = 0;
				pev->flags &= ~FL_ONGROUND;
				return;
			}
		}
		{
			int seenSolid = ( buried.fStartSolid || buried.fAllSolid ) ? 1 : 0;

			for( dz = 4; dz <= 160; dz += 4 )
			{
				Vector test;
				Vector stood;
				TraceResult up;

				test = pev->origin;
				test.z += (float)dz + 8.0f;
				head = test;
				head.z += 0.1f;
				UTIL_TraceHull( test, head, ignore_monsters, point_hull, ENT( pev ), &up );
				if( up.fStartSolid || up.fAllSolid )
				{
					seenSolid = 1;
					continue;
				}
				if( !seenSolid )
					continue;
				test.z -= 8.0f;
				/* LandMonster uses hull 1. That hull falls through the
				   office slab onto the floor under it. A drop of more
				   than one step means the surface the point just left
				   is the one to stand on. */
				if( EFW_LandMonster( pev, test, &stood ) && ( test.z - stood.z ) <= 18.0f )
					test = stood;
				else
				{
					Vector drop;
					TraceResult skin;

					drop = test;
					drop.z -= 24.0f;
					UTIL_TraceHull( test, drop, ignore_monsters, point_hull, ENT( pev ), &skin );
					if( !skin.fStartSolid && !skin.fAllSolid && skin.flFraction < 1.0f
						&& ( test.z - skin.vecEndPos.z ) <= 18.0f )
						test.z = skin.vecEndPos.z;
				}
				{
					static int s_unbury;
					const char *tn = STRING( pev->targetname );
					if( s_unbury < 8 )
					{
						s_unbury++;
						EFW_DebugPrint( "npc unbury %s z=%.0f -> %.0f at %.0f %.0f",
							( tn && tn[0] ) ? tn : "?",
							pev->origin.z, test.z, test.x, test.y );
					}
				}
				s_npcStep = 0;
				pev->flags |= FL_ONGROUND;
				pev->velocity.z = 0.0f;
				EFW_QueueOrigin( pev, test );
				return;
			}
		}
		s_npcStep = 0;
		pev->flags &= ~FL_ONGROUND;
		return;
	}
	/* The 2-unit probe only looks down. Feet already at or under the
	   surface read as air, and one gravity step (800*dt^2) is a
	   fraction==1 teleport through that floor. LandMonster also looks
	   up one step. A floor there means the hull is standing: set
	   FL_ONGROUND and leave the origin alone. Linking here re-enters
	   the think and overflows the host pump. */
	{
		Vector stood;

		if( EFW_LandMonster( pev, pev->origin, &stood ) )
		{
			float drop = pev->origin.z - stood.z;

			if( drop <= 1.0f )
			{
				static int s_stand;

				s_npcStep = 0;
				pev->flags |= FL_ONGROUND;
				pev->velocity.z = 0.0f;
				if( s_stand < 8 )
				{
					s_stand++;
					EFW_DebugPrint( "npc stand z=%.0f floor=%.0f at %.0f %.0f",
						pev->origin.z, stood.z, pev->origin.x, pev->origin.y );
				}
				return;
			}
		}
	}
	pev->flags &= ~FL_ONGROUND;
	dt = EFW_HostInterval();
	/* One Euler step of 800*dt^2 tunnels a thin floor once dt is the
	   whole pump. The walk spends that gap; the fall stays on the
	   quarter-second step that still hits the hull. */
	if( dt > 0.25f )
		dt = 0.25f;
	grav = 800.0f;
	if( pev->gravity > 0.0f )
		grav *= pev->gravity;
	pev->velocity.z -= grav * dt;
	if( pev->velocity.z < -2000.0f )
		pev->velocity.z = -2000.0f;
	z0 = pev->origin.z;
	end = pev->origin;
	end.z += pev->velocity.z * dt;
	EFW_TraceFeetHull( pev, pev->origin, end, &tr );
	s_npcStep = 0;
	if( tr.fStartSolid || tr.fAllSolid )
		return;
	if( tr.flFraction < 1.0f && tr.flFraction > 0.0f )
	{
		end = tr.vecEndPos;
		pev->velocity.z = 0.0f;
		pev->flags |= FL_ONGROUND;
		{
			static int s_land;
			float dz = end.z - z0;
			if( dz < 0.0f )
				dz = -dz;
			if( dz >= 1.0f && s_land < 6 )
			{
				s_land++;
				EFW_DebugPrint( "npc land z=%.0f -> %.0f at %.0f %.0f",
					z0, end.z, end.x, end.y );
			}
		}
		EFW_QueueOrigin( pev, end );
		return;
	}
	{
		static int s_fall;
		float dz = end.z - z0;
		if( dz < 0.0f )
			dz = -dz;
		if( dz >= 1.0f && s_fall < 6 )
		{
			s_fall++;
			EFW_DebugPrint( "npc fall z=%.0f -> %.0f at %.0f %.0f",
				z0, end.z, end.x, end.y );
		}
	}
	EFW_QueueOrigin( pev, end );
}

/* FUN_1005d500 / MoveExecute. WALK_MOVE stalls Host_Frame on these studios.
   Trace the PE hull (-16..16, 0..72). A horizontal step hits other bodies
   the way MOVE_NORMAL does; the floor probe still ignores them so a
   neighbor is not a stair. MoveExecute walks
   groundSpeed * framerate * interval in chunks of 16 (the stair limit). */
static void EFW_PeChangeYaw( CBaseMonster *pMon, int yawSpeed );

/* SV_CheckBottom. Four corners in the floor means the hull is standing.
   The real drop starts one step above the feet and stops one step below.
   A floor deeper than sv_stepsize (18) is a miss, so the chase tries
   another heading instead of walking off that curb.
   Point traces match SV_PointContents / MOVE_NOMONSTERS. */
static int EFW_CheckBottom( entvars_t *pev, const Vector &pos )
{
	Vector mins;
	Vector maxs;
	Vector start;
	Vector stop;
	TraceResult tr;
	float mid;
	int x;
	int y;
	int easy;
	const float step = 18.0f;

	if( !pev )
		return 0;
	mins = pos + Vector( -16.0f, -16.0f, 0.0f );
	maxs = pos + Vector( 16.0f, 16.0f, 72.0f );
	easy = 1;
	for( x = 0; x <= 1; x++ )
	{
		for( y = 0; y <= 1; y++ )
		{
			start.x = x ? maxs.x : mins.x;
			start.y = y ? maxs.y : mins.y;
			start.z = mins.z - 1.0f;
			UTIL_TraceHull( start, start, ignore_monsters, point_hull, ENT( pev ), &tr );
			if( !tr.fStartSolid && !tr.fAllSolid )
				easy = 0;
		}
	}
	if( easy )
		return 1;
	start.x = stop.x = ( mins.x + maxs.x ) * 0.5f;
	start.y = stop.y = ( mins.y + maxs.y ) * 0.5f;
	/* GoldSrc starts this column one step up and stops one step down. */
	start.z = mins.z + step;
	stop.z = start.z - 2.0f * step;
	UTIL_TraceHull( start, stop, ignore_monsters, point_hull, ENT( pev ), &tr );
	/* A solid start reports the start point. That column is still a floor
	   when the drop from there stays inside one step. */
	if( tr.flFraction >= 1.0f )
		return 0;
	if( tr.fStartSolid || tr.fAllSolid )
		mid = start.z;
	else
		mid = tr.vecEndPos.z;
	for( x = 0; x <= 1; x++ )
	{
		for( y = 0; y <= 1; y++ )
		{
			float ez;

			start.x = stop.x = x ? maxs.x : mins.x;
			start.y = stop.y = y ? maxs.y : mins.y;
			UTIL_TraceHull( start, stop, ignore_monsters, point_hull, ENT( pev ), &tr );
			if( tr.flFraction >= 1.0f )
				return 0;
			ez = ( tr.fStartSolid || tr.fAllSolid ) ? start.z : tr.vecEndPos.z;
			if( mid - ez > step )
				return 0;
		}
	}
	return 1;
}

/* SV_MoveStep. The hull column at the full wish is the only landing.
   A wall in that column refuses the step, so the chase can turn along
   the face. A fraction of the wish is not a move. */
static int EFW_MoveStep( entvars_t *pev, const Vector &start, const Vector &move, Vector *out, edict_t **ppHit = NULL )
{
	Vector top;
	Vector bot;
	TraceResult tr;
	const float step = 18.0f;
	int partial;
	const char *tn;

	if( ppHit )
		*ppHit = NULL;
	if( !pev || !out )
		return 0;
	partial = ( pev->flags & FL_PARTIALGROUND ) ? 1 : 0;
	top = start + move;
	top.z += step;
	bot = top;
	bot.z -= step * 2.0f;
	EFW_TraceFeetHull( pev, top, bot, &tr, dont_ignore_monsters );
	if( tr.fAllSolid )
	{
		if( ppHit )
			*ppHit = tr.pHit;
		return 0;
	}
	if( tr.fStartSolid )
	{
		top.z -= step;
		EFW_TraceFeetHull( pev, top, bot, &tr, dont_ignore_monsters );
		if( tr.fAllSolid || tr.fStartSolid )
		{
			if( ppHit )
				*ppHit = tr.pHit;
			return 0;
		}
	}
	tn = STRING( pev->targetname );
	if( tr.flFraction >= 1.0f )
	{
		if( !partial )
		{
			if( tn && !strcmp( tn, "Elika" ) )
			{
				static int s_hold;

				if( s_hold < 6 )
				{
					s_hold++;
					EFW_DebugPrint( "bottom hold at %.0f %.0f z=%.0f",
						( start + move ).x, ( start + move ).y, start.z );
				}
			}
			return -1;
		}
		*out = start + move;
		pev->flags &= ~FL_ONGROUND;
		if( tn && !strcmp( tn, "Elika" ) )
		{
			static int s_air;

			if( s_air < 4 )
			{
				s_air++;
				EFW_DebugPrint( "partial fall Elika at %.0f %.0f z=%.0f",
					out->x, out->y, out->z );
			}
		}
		return 3;
	}
	if( EFW_TraceHitBody( pev, &tr ) )
	{
		if( ppHit )
			*ppHit = tr.pHit;
		return 0;
	}
	*out = tr.vecEndPos;
	if( !EFW_CheckBottom( pev, *out ) )
	{
		if( !partial )
		{
			if( tn && !strcmp( tn, "Elika" ) )
			{
				static int s_hold;

				if( s_hold < 6 )
				{
					s_hold++;
					EFW_DebugPrint( "bottom hold at %.0f %.0f z=%.0f",
						out->x, out->y, out->z );
				}
			}
			return -1;
		}
		return 2;
	}
	pev->flags &= ~FL_PARTIALGROUND;
	return 1;
}

/* SV_MoveToOrigin MOVE_NORMAL steps along ideal_yaw. When that step cannot
   be taken, SV_NewChaseDir2 tries the diagonal, then the two cardinals.
   RandomLong(0, 1) swaps those cardinals when the east-west gap is at
   least the north-south gap. A second RandomLong, only after those
   probes miss, picks the 45-degree sweep. A probe that moves does not
   replace ideal_yaw, so the next chunk tries the same heading. This is
   that search with the hull trace, not WALK_MOVE. */
/* Set when every chase probe fails and ideal_yaw is restored to the
   snapped heading. The next chunk of this MoveExecute steps along it. */
static int s_chaseRestored;

static int EFW_TryChunk( entvars_t *pev, const Vector &start, const Vector &dir, float step, Vector *out )
{
	Vector wish;
	Vector move;
	int kind;

	wish = dir;
	wish.z = 0.0f;
	if( wish.Length() < 0.001f )
		return 0;
	wish = wish.Normalize();
	move = wish * step;
	kind = EFW_MoveStep( pev, start, move, out );
	if( kind <= 0 )
		return 0;
	return 1;
}

static float EFW_NormYaw360( float yaw )
{
	while( yaw < 0.0f )
		yaw += 360.0f;
	while( yaw >= 360.0f )
		yaw -= 360.0f;
	return yaw;
}

/* One SV_StepDirection probe. Turnaround is held until the last call.
   A yaw already probed is not walked again. ideal_yaw stays put. */
static int EFW_ChaseProbe( entvars_t *pev, const Vector &start, float yaw, float turnaround, int last, float step, float *tried, int *ntried, Vector *out )
{
	float ny;
	int j;
	Vector dir;
	Vector landed;

	ny = EFW_NormYaw360( yaw );
	if( fabsf( ny - turnaround ) < 0.5f && !last )
		return 0;
	for( j = 0; j < *ntried; j++ )
	{
		if( fabsf( tried[j] - ny ) < 0.5f )
			return 0;
	}
	if( *ntried < 16 )
		tried[( *ntried )++] = ny;
	dir.x = cosf( ny * 0.01745329252f );
	dir.y = sinf( ny * 0.01745329252f );
	dir.z = 0.0f;
	/* SV_StepDirection walks this yaw and leaves ideal_yaw on the
	   heading Move() already stored. The next chunk tries that
	   heading again. */
	if( !EFW_TryChunk( pev, start, dir, step, &landed ) )
		return 0;
	{
		const char *tn = STRING( pev->targetname );
		static int s_chase;
		static int s_elikaChase;

		if( tn && !strcmp( tn, "Elika" ) && s_elikaChase < 6 )
		{
			s_elikaChase++;
			EFW_DebugPrint( "chase step Elika ideal=%.0f probe=%.0f at %.0f %.0f -> %.0f %.0f",
				pev->ideal_yaw, ny,
				start.x, start.y, landed.x, landed.y );
		}
		else if( s_chase < 6 )
		{
			s_chase++;
			EFW_DebugPrint( "chase dir yaw=%.0f origin=%.0f %.0f -> %.0f %.0f",
				ny, start.x, start.y, landed.x, landed.y );
		}
	}
	*out = landed;
	return 1;
}

static int EFW_ChaseChunk( entvars_t *pev, const Vector &start, const Vector &goal, float step, Vector *out )
{
	float deltax;
	float deltay;
	float dirx;
	float diry;
	float olddir;
	float turnaround;
	float tried[16];
	float cardinal[2];
	int ntried;
	int i;
	int roll;
	int yFirst;

	deltax = goal.x - start.x;
	deltay = goal.y - start.y;
	dirx = ( deltax > 10.0f ) ? 0.0f : ( deltax < -10.0f ) ? 180.0f : -1.0f;
	diry = ( deltay < -10.0f ) ? 270.0f : ( deltay > 10.0f ) ? 90.0f : -1.0f;
	/* SV_NewChaseDir snaps the heading it will restore if every probe fails. */
	olddir = EFW_NormYaw360( ( (int)( pev->ideal_yaw / 45.0f ) ) * 45.0f );
	turnaround = EFW_NormYaw360( olddir - 180.0f );
	ntried = 0;
	if( dirx >= 0.0f && diry >= 0.0f )
	{
		float diag;

		if( dirx == 0.0f )
			diag = ( diry == 90.0f ) ? 45.0f : 315.0f;
		else
			diag = ( diry == 90.0f ) ? 135.0f : 215.0f;
		if( EFW_ChaseProbe( pev, start, diag, turnaround, 0, step, tried, &ntried, out ) )
			return 1;
	}
	/* RandomLong(0, 1) runs only after the diagonal misses. When it is
	   set, or the north-south gap is larger, the Y cardinal is first. */
	roll = RANDOM_LONG( 0, 1 );
	yFirst = roll || ( fabsf( deltay ) > fabsf( deltax ) );
	if( yFirst )
	{
		cardinal[0] = diry;
		cardinal[1] = dirx;
	}
	else
	{
		cardinal[0] = dirx;
		cardinal[1] = diry;
	}
	for( i = 0; i < 2; i++ )
	{
		if( cardinal[i] < 0.0f )
			continue;
		if( !EFW_ChaseProbe( pev, start, cardinal[i], turnaround, 0, step, tried, &ntried, out ) )
			continue;
		{
			static int s_order;
			static int s_swap;
			float pick = EFW_NormYaw360( cardinal[i] );

			/* roll 1 with the east-west gap at least as large is the
			   swap. The longer axis would have been first otherwise. */
			if( roll && fabsf( deltay ) <= fabsf( deltax ) && s_swap < 4 )
			{
				s_swap++;
				EFW_DebugPrint( "chase swap roll=1 pick=%.0f dx=%.0f dy=%.0f",
					pick, deltax, deltay );
			}
			else if( s_order < 4 )
			{
				s_order++;
				EFW_DebugPrint( "chase order roll=%d yfirst=%d pick=%.0f dx=%.0f dy=%.0f",
					roll, yFirst, pick, deltax, deltay );
			}
		}
		return 1;
	}
	if( EFW_ChaseProbe( pev, start, olddir, turnaround, 0, step, tried, &ntried, out ) )
		return 1;
	/* The sweep roll is reached only when the cardinals and the snapped
	   heading missed. 1 walks 0 through 315. 0 walks 315 through 0. */
	roll = RANDOM_LONG( 0, 1 );
	if( roll )
	{
		for( i = 0; i < 8; i++ )
		{
			if( EFW_ChaseProbe( pev, start, (float)( i * 45 ), turnaround, 0, step, tried, &ntried, out ) )
			{
				static int s_sweep;

				if( s_sweep < 3 )
				{
					s_sweep++;
					EFW_DebugPrint( "chase sweep roll=1 pick=%.0f dx=%.0f dy=%.0f",
						(float)( i * 45 ), deltax, deltay );
				}
				return 1;
			}
		}
	}
	else
	{
		for( i = 7; i >= 0; i-- )
		{
			if( EFW_ChaseProbe( pev, start, (float)( i * 45 ), turnaround, 0, step, tried, &ntried, out ) )
			{
				static int s_sweep;

				if( s_sweep < 3 )
				{
					s_sweep++;
					EFW_DebugPrint( "chase sweep roll=0 pick=%.0f dx=%.0f dy=%.0f",
						(float)( i * 45 ), deltax, deltay );
				}
				return 1;
			}
		}
	}
	if( EFW_ChaseProbe( pev, start, turnaround, turnaround, 1, step, tried, &ntried, out ) )
		return 1;
	/* Every probe failed. Put ideal_yaw back on the snapped heading.
	   A hull that is already off a full floor gets FL_PARTIALGROUND,
	   and the next chunk may step anyway. */
	pev->ideal_yaw = olddir;
	{
		int partial = 0;
		const char *tn = STRING( pev->targetname );

		if( !EFW_CheckBottom( pev, start ) )
		{
			pev->flags |= FL_PARTIALGROUND;
			partial = 1;
		}
		s_chaseRestored = 1;
		if( tn && !strcmp( tn, "Elika" ) )
		{
			static int s_restore;

			if( s_restore < 4 )
			{
				s_restore++;
				EFW_DebugPrint( "chase restore Elika ideal=%.0f ang=%.0f partial=%d",
					pev->ideal_yaw, pev->angles.y, partial );
			}
		}
		else
		{
			static int s_stuck;

			if( s_stuck < 4 )
			{
				s_stuck++;
				EFW_DebugPrint( "chase stuck %.0f %.0f partial=%d",
					start.x, start.y, partial );
			}
		}
	}
	return 0;
}

/* CheckLocalMove walks the lookahead in 16-unit steps and stops one unit
   short. WALK_MOVE and DROP_TO_FLOOR stall the frame, so this is that
   walk with the same hull step MoveExecute uses. A step whose hull hits
   the movement target is a clear walk: the original compares trace_ent
   with that entity and returns LOCALMOVE_VALID. A landing more than 64
   below the check point is a miss, unless that target is in the air.
   Flags from the probe are put back. */
static int EFW_LocalMove( entvars_t *pev, const Vector &start, const Vector &end, float *pflDist, edict_t *pTarget = NULL, int *pOnTarget = NULL )
{
	float yaw;
	float dist;
	float traveled;
	Vector pos;
	int flags;

	if( pflDist )
		*pflDist = 0.0f;
	if( pOnTarget )
		*pOnTarget = 0;
	if( !pev )
		return 0;
	yaw = UTIL_VecToYaw( end - start );
	dist = ( end - start ).Length2D();
	pos = start;
	flags = pev->flags;
	traveled = 0.0f;
	while( traveled < dist )
	{
		float stepSize;
		Vector wish;
		Vector landed;
		float yawRad;
		int kind;

		stepSize = 16.0f;
		if( traveled + 16.0f >= dist - 1.0f )
			stepSize = ( dist - traveled ) - 1.0f;
		if( stepSize < 0.001f )
			break;
		yawRad = yaw * 0.01745329252f;
		wish.x = cosf( yawRad ) * stepSize;
		wish.y = sinf( yawRad ) * stepSize;
		wish.z = 0.0f;
		{
			edict_t *hit = NULL;

			kind = EFW_MoveStep( pev, pos, wish, &landed, &hit );
			/* 0x1005e408: a failed step whose trace_ent is the target
			   is LOCALMOVE_VALID. The probe stops there. */
			if( kind == 0 && pTarget && hit == pTarget )
			{
				if( pOnTarget )
					*pOnTarget = 1;
				break;
			}
		}
		if( kind <= 0 )
		{
			pev->flags = flags;
			if( pflDist )
				*pflDist = traveled;
			return 0;
		}
		pos = landed;
		traveled += 16.0f;
	}
	pev->flags = flags;
	/* The z test runs for a clear walk and for a walk accepted because
	   it met the target. An airborne target skips it. */
	if( !( pev->flags & ( FL_FLY | FL_SWIM ) ) )
	{
		int checkZ = 1;

		if( pTarget && !( pTarget->v.flags & FL_ONGROUND ) )
			checkZ = 0;
		if( checkZ && fabsf( end.z - pos.z ) > 64.0f )
			return 1;
	}
	return 2;
}

/* FTriangulate. The apex steps out from the blockage by the hull width.
   Both legs have to be a clear local move. The far leg is the route
   goal, so a goal inside a wall does not become a detour. */
static int EFW_Triangulate( entvars_t *pev, const Vector &start, const Vector &end, float flDist, Vector *apex, edict_t *pTarget )
{
	Vector delta;
	Vector forward;
	Vector side;
	Vector left;
	Vector right;
	float sizeX;
	float len;
	int i;

	if( !pev || !apex )
		return 0;
	sizeX = pev->size.x;
	if( sizeX < 24.0f )
		sizeX = 24.0f;
	else if( sizeX > 48.0f )
		sizeX = 48.0f;
	delta = end - start;
	len = delta.Length();
	if( len < 0.001f )
		return 0;
	forward = delta * ( 1.0f / len );
	side.x = forward.y;
	side.y = -forward.x;
	side.z = 0.0f;
	right = start + forward * ( flDist + sizeX ) + side * ( sizeX * 3.0f );
	left = start + forward * ( flDist + sizeX ) - side * ( sizeX * 3.0f );
	side = side * ( sizeX * 2.0f );
	for( i = 0; i < 8; i++ )
	{
		if( EFW_LocalMove( pev, start, right, NULL, pTarget, NULL ) == 2
			&& EFW_LocalMove( pev, right, end, NULL, pTarget, NULL ) == 2 )
		{
			*apex = right;
			return 1;
		}
		if( EFW_LocalMove( pev, start, left, NULL, pTarget, NULL ) == 2
			&& EFW_LocalMove( pev, left, end, NULL, pTarget, NULL ) == 2 )
		{
			*apex = left;
			return 1;
		}
		right = right + side;
		left = left - side;
	}
	return 0;
}

/* InsertWaypoint (0x1005e890) pushes the apex in front of the current
   waypoint, then RouteSimplify (0x1005d5a0) rewrites the chain. A clear
   trace to the next point drops the apex. A clear trace to the midpoint
   of this point and the next stores that midpoint. A clear trace between
   the two midpoints stores both. The constant at 0x100dd614 is 0.5.
   Fewer than two points returns at 0x1005d5de. A later block inserts
   again and simplifies against the points still ahead, so the tail of
   the route stays in the walk. A detour is not a pure targetent route,
   so the lookahead target stays clear until the chain is done. */
struct EFW_DetourSlot
{
	entvars_t *pev;
	Vector pt[8];
	int n;
	int i;
};

static EFW_DetourSlot s_detour[8];

static EFW_DetourSlot *EFW_DetourSlotFor( entvars_t *pev, int create )
{
	int i;
	int freeSlot;

	freeSlot = -1;
	if( !pev )
		return NULL;
	for( i = 0; i < 8; i++ )
	{
		if( s_detour[i].n > 0 && s_detour[i].pev == pev )
			return &s_detour[i];
		if( freeSlot < 0 && s_detour[i].n <= 0 )
			freeSlot = i;
	}
	if( !create )
		return NULL;
	if( freeSlot < 0 )
		freeSlot = 0;
	s_detour[freeSlot].pev = pev;
	s_detour[freeSlot].n = 0;
	s_detour[freeSlot].i = 0;
	return &s_detour[freeSlot];
}

static void EFW_ClearDetour( entvars_t *pev )
{
	EFW_DetourSlot *slot;

	slot = EFW_DetourSlotFor( pev, 0 );
	if( slot )
		slot->n = 0;
}

/* RouteSimplify over the points InsertWaypoint just wrote. The last
   point is always kept. outCount is 1 when every earlier point was cut. */
static int EFW_RouteSimplify( entvars_t *pev, const Vector *inRoute, int count, edict_t *pTarget, Vector *outRoute )
{
	Vector vecStart;
	int i;
	int outCount;

	if( !pev || !inRoute || !outRoute || count <= 0 )
		return 0;
	if( count < 2 )
	{
		outRoute[0] = inRoute[0];
		return 1;
	}
	if( count > 8 )
		count = 8;
	vecStart = pev->origin;
	outCount = 0;
	for( i = 0; i < count - 1; i++ )
	{
		Vector next;
		Vector here;
		Vector vecTest;
		Vector vecSplit;

		next = inRoute[i + 1];
		here = inRoute[i];
		if( EFW_LocalMove( pev, vecStart, next, NULL, pTarget, NULL ) == 2 )
			continue;
		vecTest = ( next + here ) * 0.5f;
		vecSplit = ( here + vecStart ) * 0.5f;
		if( EFW_LocalMove( pev, vecStart, vecTest, NULL, pTarget, NULL ) == 2 )
			outRoute[outCount] = vecTest;
		else if( EFW_LocalMove( pev, vecSplit, vecTest, NULL, pTarget, NULL ) == 2 )
		{
			if( outCount < 14 )
			{
				outRoute[outCount] = vecSplit;
				outCount++;
			}
			outRoute[outCount] = vecTest;
		}
		else
			outRoute[outCount] = here;
		vecStart = outRoute[outCount];
		outCount++;
		if( outCount >= 15 )
			break;
	}
	if( outCount < 16 )
	{
		outRoute[outCount] = inRoute[count - 1];
		outCount++;
	}
	return outCount;
}

static int EFW_Near2D( const Vector &a, const Vector &b )
{
	Vector d;

	d = a - b;
	d.z = 0.0f;
	return d.Length() < 8.0f;
}

static int EFW_StepNpc( entvars_t *pev, const Vector &goal, float speed, float dt, edict_t *pTarget = NULL, const Vector *pStepToward = NULL )
{
	Vector delta;
	Vector start;
	Vector landed;
	Vector moveGoal;
	float len;
	float len3;
	float cap;
	float total;
	float moved;
	int chunks;
	int onDetour;
	int cutCorner;
	int stepNext;
	int localOk;
	float cutDist;
	Vector chaseGoal;

	if( !pev || s_npcStep )
		return 0;
	/* MOVE_TO_ORIGIN walks only once FL_ONGROUND is set. An embedded hull
	   is not airborne; the lift below still has to pull it out of the floor. */
	if( !( pev->flags & ( FL_ONGROUND | FL_FLY | FL_SWIM ) ) )
	{
		int support;

		s_npcStep = 1;
		support = EFW_ProbeSupport( pev );
		s_npcStep = 0;
		if( support == 0 )
			return 0;
	}
	/* A stored apex is the current waypoint. Within 8 units this think
	   still faces that point. ShouldAdvanceRoute runs after the local
	   move, so the slot advances only once that probe has not failed. */
	moveGoal = goal;
	onDetour = 0;
	cutCorner = 0;
	stepNext = 0;
	localOk = 0;
	cutDist = 0.0f;
	{
		EFW_DetourSlot *slot = EFW_DetourSlotFor( pev, 0 );

		if( slot && slot->n > 0 )
		{
			Vector ad;
			float away;

			if( slot->i < 0 || slot->i >= slot->n )
				slot->i = 0;
			ad = slot->pt[slot->i] - pev->origin;
			ad.z = 0.0f;
			away = ad.Length();
			moveGoal = slot->pt[slot->i];
			onDetour = 1;
			pTarget = NULL;
			if( away <= 8.0f )
			{
				cutCorner = 1;
				cutDist = away;
			}
		}
	}
	/* Move() normalizes the 3D delta for the facing vector, then
	   stores Length2D. The step budget is that ground distance,
	   capped at 200. A goal above the hull keeps the same horizontal
	   budget as a goal on the floor, so the walk ends on that point.
	   A corner within 8 keeps that leftover as the budget. */
	delta = moveGoal - pev->origin;
	len3 = delta.Length();
	delta.z = 0.0f;
	len = delta.Length();
	if( len < 1.0f && !cutCorner )
		return 0;
	if( cutCorner )
		cap = cutDist;
	else
		cap = len;
	if( cap > 200.0f )
		cap = 200.0f;
	/* Move() faces the goal with MakeIdealYaw + ChangeYaw(yaw_speed)
	   once, before the 16-unit chunks, including the think whose
	   StudioFrameAdvance interval is 0. A leftover under 1 unit has
	   no direction to normalize; the previous think already faced it. */
	if( len >= 1.0f )
	{
		CBaseEntity *pEnt = CBaseEntity::Instance( ENT( pev ) );
		CBaseMonster *pMon = pEnt ? pEnt->MyMonsterPointer() : NULL;
		Vector wish = delta * ( 1.0f / len );
		if( pMon )
		{
			int yawSpeed = (int)pev->yaw_speed;
			float before;
			/* 0x1000cde0 default is 70 when SetActivity has not run. */
			if( yawSpeed < 1 )
				yawSpeed = 70;
			before = pev->angles.y;
			pMon->MakeIdealYaw( moveGoal );
			EFW_PeChangeYaw( pMon, yawSpeed );
			{
				static entvars_t *s_yawWho;
				static int s_yawLog;
				if( !s_yawWho )
					s_yawWho = pev;
				if( pev == s_yawWho && s_yawLog < 6 )
				{
					s_yawLog++;
					EFW_DebugPrint( "npc yaw ideal=%.0f before=%.0f ang=%.0f spd=%d",
						pev->ideal_yaw, before, pev->angles.y, yawSpeed );
				}
			}
		}
		else
			pev->angles.y = UTIL_VecToYaw( wish );
	}
	/* ResetSequenceInfo stores animtime = now. MoveExecute then gets a
	   zero interval and does not translate. A SetActivity that ran after
	   the anim call still has that clock; push it ahead so the next pump
	   is not another hold. dt is the interval StudioFrameAdvance returned. */
	{
		float skew = pev->animtime - gpGlobals->time;
		if( skew < 0.0f )
			skew = -skew;
		if( dt < 0.001f || skew <= 0.001f )
		{
			if( skew <= 0.001f )
				pev->animtime = gpGlobals->time + 0.25f;
			return 0;
		}
	}
	total = speed * dt;
	if( total > cap )
		total = cap;
	/* A corner with no leftover still advances below. Any other
	   zero wish returns before the probe. */
	if( total < 0.001f && !cutCorner )
		return 0;
	{
		const char *tn = STRING( pev->targetname );
		static int s_budget;

		if( s_budget < 4 && len3 > len + 0.5f )
		{
			s_budget++;
			EFW_DebugPrint( "step budget %s dist2=%.0f dist3=%.0f wish=%.1f",
				( tn && tn[0] ) ? tn : "?", len, len3, total );
		}
	}
	s_npcStep = 1;
	chaseGoal = moveGoal;
	/* Move() traces origin + normalize(3D delta) * flCheckDist before
	   MoveExecute. A miss tries a detour. No detour means this think
	   does not spend the step. */
	{
		Vector dir = moveGoal - pev->origin;
		float dirLen = dir.Length();
		Vector checkEnd;
		float reached;
		int local;
		int onTarget;
		const char *tn = STRING( pev->targetname );

		if( dirLen > 0.001f )
			checkEnd = pev->origin + dir * ( cap / dirLen );
		else
			checkEnd = pev->origin;
		onTarget = 0;
		local = EFW_LocalMove( pev, pev->origin, checkEnd, &reached, pTarget, &onTarget );
		if( local == 2 )
		{
			localOk = 1;
			static int s_clear;
			static int s_target;

			if( onTarget && s_target < 4 )
			{
				s_target++;
				EFW_DebugPrint( "local target %s cap=%.0f",
					( tn && tn[0] ) ? tn : "?", cap );
			}
			else if( onDetour )
			{
				static int s_keep;

				if( s_keep < 4 )
				{
					s_keep++;
					EFW_DebugPrint( "detour keep %s dist=%.0f at=%.0f %.0f apex=%.0f %.0f",
						( tn && tn[0] ) ? tn : "?", len,
						pev->origin.x, pev->origin.y, moveGoal.x, moveGoal.y );
				}
			}
			else if( s_clear < 2 && tn && !strcmp( tn, "efw_electrician" ) )
			{
				s_clear++;
				EFW_DebugPrint( "local clear %s cap=%.0f", tn, cap );
			}
		}
		else
		{
			Vector apex;

			if( !EFW_Triangulate( pev, pev->origin, moveGoal, reached, &apex, pTarget ) )
			{
				static int s_block;
				CBaseEntity *pEnt;
				CBaseMonster *pMon;
				const char *cn;

				if( onDetour )
					EFW_ClearDetour( pev );
				/* Move() at 0x1005f5ac calls Stop(), then TaskFail when
				   m_moveWaitTime is 0. Stop stores ACT_IDLE and that Move
				   returns. Refugee IdleThink calls Move again only while
				   the walk goal is still stored. Drop the goal and the
				   detour. The next 0x52 gate is MoveToTarget, which would
				   have rebuilt the route. Patrol and officer thinks store
				   their own goal again, so this give-up is the refugee. */
				pEnt = CBaseEntity::Instance( ENT( pev ) );
				pMon = pEnt ? pEnt->MyMonsterPointer() : NULL;
				cn = STRING( pev->classname );
				if( pMon && cn && !strcmp( cn, "monster_refugee" ) )
				{
					pMon->SetActivity( ACT_IDLE );
					pMon->m_movementActivity = ACT_IDLE;
					pMon->m_movementGoal = MOVEGOAL_NONE;
				}
				if( s_block < 4 )
				{
					s_block++;
					EFW_DebugPrint( "local block %s reached=%.0f cap=%.0f",
						( tn && tn[0] ) ? tn : "?", reached, cap );
					if( pMon && cn && !strcmp( cn, "monster_refugee" ) )
						EFW_DebugPrint( "move fail %s act=%d goal=%d origin=%.0f %.0f",
							( tn && tn[0] ) ? tn : "?",
							(int)pMon->m_Activity, pMon->m_movementGoal,
							pev->origin.x, pev->origin.y );
				}
				s_npcStep = 0;
				return 0;
			}
			chaseGoal = apex;
			{
				EFW_DetourSlot *keep = EFW_DetourSlotFor( pev, 1 );
				Vector inRoute[8];
				Vector outRoute[16];
				int inCount;
				int outCount;
				int k;
				int ncopy;
				static int s_detour;

				if( s_detour < 4 )
				{
					s_detour++;
					EFW_DebugPrint( "local detour %s apex=%.0f %.0f reached=%.0f",
						( tn && tn[0] ) ? tn : "?", apex.x, apex.y, reached );
				}
				/* InsertWaypoint puts the apex at the current index and
				   shifts the points still ahead. The first insert's tail
				   is the goal Move was walking. */
				inRoute[0] = apex;
				inCount = 1;
				if( onDetour && keep && keep->n > keep->i )
				{
					for( k = keep->i; k < keep->n && inCount < 8; k++ )
						inRoute[inCount++] = keep->pt[k];
				}
				else
					inRoute[inCount++] = moveGoal;
				outCount = EFW_RouteSimplify( pev, inRoute, inCount, onDetour ? NULL : pTarget, outRoute );
				if( keep && outCount <= 1 && !onDetour )
				{
					static int s_drop;

					keep->n = 0;
					chaseGoal = moveGoal;
					if( s_drop < 4 )
					{
						s_drop++;
						EFW_DebugPrint( "simplify drop %s next=%.0f %.0f",
							( tn && tn[0] ) ? tn : "?", moveGoal.x, moveGoal.y );
					}
				}
				else if( keep && outCount > 0 )
				{
					ncopy = outCount;
					if( ncopy > 8 )
						ncopy = 8;
					for( k = 0; k < ncopy; k++ )
						keep->pt[k] = outRoute[k];
					if( outCount > 8 )
						keep->pt[7] = outRoute[outCount - 1];
					keep->i = 0;
					keep->n = ncopy;
					chaseGoal = keep->pt[0];
					if( !onDetour )
					{
						Vector mid;
						Vector split;
						static int s_log;

						mid = ( moveGoal + apex ) * 0.5f;
						split = ( apex + pev->origin ) * 0.5f;
						if( s_log < 4 )
						{
							s_log++;
							if( EFW_Near2D( chaseGoal, mid ) )
								EFW_DebugPrint( "simplify cut %s raw=%.0f %.0f cut=%.0f %.0f",
									( tn && tn[0] ) ? tn : "?", apex.x, apex.y,
									chaseGoal.x, chaseGoal.y );
							else if( EFW_Near2D( chaseGoal, split ) && ncopy > 1 )
								EFW_DebugPrint( "simplify split %s raw=%.0f %.0f split=%.0f %.0f cut=%.0f %.0f",
									( tn && tn[0] ) ? tn : "?", apex.x, apex.y,
									chaseGoal.x, chaseGoal.y, keep->pt[1].x, keep->pt[1].y );
							else
								EFW_DebugPrint( "simplify keep %s apex=%.0f %.0f",
									( tn && tn[0] ) ? tn : "?", apex.x, apex.y );
						}
					}
					else if( !EFW_Near2D( chaseGoal, apex ) )
					{
						static int s_chain;
						Vector tail;

						if( keep->n > 1 )
							tail = keep->pt[1];
						else
							tail = chaseGoal;
						if( s_chain < 4 )
						{
							s_chain++;
							EFW_DebugPrint( "simplify chain %s apex=%.0f %.0f walk=%.0f %.0f tail=%.0f %.0f",
								( tn && tn[0] ) ? tn : "?", apex.x, apex.y,
								chaseGoal.x, chaseGoal.y, tail.x, tail.y );
						}
					}
				}
			}
		}
	}
	/* ShouldAdvanceRoute (0x1005f6e0) runs after a clear local move or
	   a finished triangulate. The facing and the budget stay on the
	   point this think already aimed at. The chase goal becomes the
	   route point AdvanceRoute just stored. The last point clears the
	   slot the way MovementComplete does, and this think still spends
	   only the leftover. */
	if( cutCorner )
	{
		EFW_DetourSlot *slot = EFW_DetourSlotFor( pev, 0 );
		const char *tn = STRING( pev->targetname );

		if( slot && slot->n > 0 )
		{
			slot->i++;
			if( slot->i >= slot->n )
			{
				static int s_arrive;

				slot->n = 0;
				if( s_arrive < 4 )
				{
					s_arrive++;
					EFW_DebugPrint( "detour arrive %s dist=%.1f wish=%.1f",
						( tn && tn[0] ) ? tn : "?", cutDist, total );
				}
			}
			else
			{
				static int s_remain;

				/* AdvanceRoute stored the next slot. MoveExecute walks
				   that point. The body already faced the one we left. */
				chaseGoal = slot->pt[slot->i];
				stepNext = 1;
				if( s_remain < 4 )
				{
					s_remain++;
					EFW_DebugPrint( "detour remain %s dist=%.1f wish=%.1f next=%.0f %.0f",
						( tn && tn[0] ) ? tn : "?", cutDist, total,
						chaseGoal.x, chaseGoal.y );
				}
			}
		}
		if( total < 0.001f )
		{
			s_npcStep = 0;
			return 0;
		}
	}
	/* A path corner within 8 advances before MoveExecute. The local
	   probe already used the old point, and that leftover is the budget.
	   The hull walks the next point. A detour slot owns this think
	   instead, and a failed probe keeps the apex it just stored. */
	if( pStepToward && !onDetour && localOk && !stepNext )
	{
		chaseGoal = *pStepToward;
		stepNext = 1;
	}
	start = pev->origin;
	landed = start;
	moved = 0.0f;
	chunks = 0;
	/* MoveExecute spends the wish in 16-unit steps with no chunk cap.
	   Eight chunks is 128 units: walk speed 64 across the 2s pump clamp.
	   The run sequence is 345, so that same clamp wishes 690. 48 chunks
	   is 768 units and still covers it. */
	while( total > 0.001f && chunks < 48 )
	{
		Vector wish;
		Vector stepLand;
		float step;
		float stepLen;

		/* MoveExecute spends the Length2D budget in 16-unit chunks.
		   The last chunk is the remainder of that ground distance. */
		step = total;
		if( step > 16.0f )
			step = 16.0f;
		/* SV_StepDirection steps along the ideal_yaw Move() stored.
		   It does not turn the body. A chase that failed has put
		   ideal_yaw back on the snapped heading for this chunk.
		   After AdvanceRoute, MoveExecute ignores that yaw and walks
		   the route point just stored. The body stays on the old one. */
		if( s_chaseRestored )
		{
			const char *tn = STRING( pev->targetname );

			s_chaseRestored = 0;
			if( tn && !strcmp( tn, "Elika" ) )
			{
				static int s_again;

				if( s_again < 4 )
				{
					s_again++;
					EFW_DebugPrint( "chase again Elika ang=%.0f ideal=%.0f",
						pev->angles.y, pev->ideal_yaw );
				}
			}
		}
		{
			float stepYaw = pev->ideal_yaw;

			if( stepNext )
			{
				Vector aim = chaseGoal - start;

				aim.z = 0.0f;
				if( aim.Length() > 0.001f )
					stepYaw = UTIL_VecToYaw( aim );
			}
			{
				float yawRad = stepYaw * 0.01745329252f;

				wish.x = cosf( yawRad ) * step;
				wish.y = sinf( yawRad ) * step;
				wish.z = 0.0f;
			}
		}
		{
			int kind;

			/* SV_MoveStep accepts the full column or nothing. A wall
			   closer than this chunk refuses it, and the chase below
			   sidesteps for this chunk only. */
			kind = EFW_MoveStep( pev, start, wish, &stepLand );
			if( kind <= 0 )
			{
				if( kind == 0 )
				{
					static int s_refuse;
					const char *tn = STRING( pev->targetname );

					if( tn && !strcmp( tn, "Elika" ) && s_refuse < 4 )
					{
						s_refuse++;
						EFW_DebugPrint( "step refuse Elika at %.0f %.0f wish=%.0f %.0f",
							start.x, start.y, wish.x, wish.y );
					}
				}
				stepLen = 0.0f;
			}
			else
			{
				float dz = stepLand.z - start.z;

				if( dz < 0.0f )
					dz = -dz;
				if( dz >= 1.0f )
				{
					static int s_ground;
					if( s_ground < 6 )
					{
						s_ground++;
						EFW_DebugPrint( "step ground z=%.0f -> %.0f at %.0f %.0f",
							start.z, stepLand.z, stepLand.x, stepLand.y );
					}
				}
				if( kind == 2 )
				{
					static int s_part;

					if( s_part < 4 )
					{
						s_part++;
						EFW_DebugPrint( "partial step at %.0f %.0f z=%.0f",
							stepLand.x, stepLand.y, stepLand.z );
					}
				}
				stepLen = ( stepLand - start ).Length();
				{
					const char *tn = STRING( pev->targetname );

					if( tn && !strcmp( tn, "Elika" ) )
					{
						static int s_took;

						if( s_took < 6 )
						{
							s_took++;
							EFW_DebugPrint( "step take Elika ideal=%.0f at %.0f %.0f -> %.0f %.0f",
								pev->ideal_yaw, start.x, start.y, stepLand.x, stepLand.y );
						}
					}
				}
			}
		}
		/* SV_MoveToOrigin: a blocked ideal_yaw step calls SV_NewChaseDir2
		   instead of stopping on the wall. ideal_yaw stays, so the next
		   chunk tries that heading again. */
		/* A leftover under half a unit is still the chunk that was
		   asked for. Chase only when the landing is also under half
		   of that request. */
		if( stepLen < 0.5f && stepLen < step * 0.5f )
		{
			Vector chased;
			int chase;

			chase = EFW_ChaseChunk( pev, start, chaseGoal, step, &chased );
			if( !chase )
			{
				/* ideal_yaw is the snapped heading again. Spend this
				   chunk and let the next one step along it. */
				total -= step;
				if( !( pev->flags & FL_ONGROUND ) )
					break;
				continue;
			}
			stepLand = chased;
			stepLen = ( stepLand - start ).Length();
			if( stepLen < 0.5f && stepLen < step * 0.5f )
				break;
		}
		/* The air step already cleared FL_ONGROUND. Another chunk would
		   be MoveToOrigin with that flag clear, which does not move. */
		if( !( pev->flags & FL_ONGROUND ) )
		{
			if( stepLen >= 0.5f || stepLen >= step * 0.5f )
			{
				landed = stepLand;
				start = stepLand;
				moved += stepLen;
				chunks++;
			}
			break;
		}
		landed = stepLand;
		start = stepLand;
		moved += stepLen;
		chunks++;
		/* MoveExecute always subtracts the requested chunk. A fraction just
		   under 1 is still a clear step; stop only when the next chunk
		   cannot move. */
		total -= step;
	}
	{
		static int s_exec;
		if( s_exec < 6 && chunks > 0 )
		{
			s_exec++;
			EFW_DebugPrint( "move execute iv=%.3f spd=%.0f wish=%.1f moved=%.1f chunks=%d",
				dt, speed, speed * dt, moved, chunks );
		}
		else if( s_exec < 12 && speed > 120.0f && chunks > 0 )
		{
			s_exec++;
			EFW_DebugPrint( "move execute run iv=%.3f spd=%.0f wish=%.1f moved=%.1f chunks=%d",
				dt, speed, speed * dt, moved, chunks );
		}
	}
	if( chunks > 8 )
	{
		static int s_long;
		if( s_long < 8 )
		{
			s_long++;
			EFW_DebugPrint( "move execute long iv=%.3f spd=%.0f wish=%.1f moved=%.1f chunks=%d",
				dt, speed, speed * dt, moved, chunks );
		}
	}
	s_chaseRestored = 0;
	if( moved < 0.5f )
	{
		s_npcStep = 0;
		return 0;
	}
	s_npcStep = 0;
	EFW_QueueOrigin( pev, landed );
	return 1;
}

/* PE ChangeYaw 0x100603b0: speed = yawSpeed * frametime * 10.
   monsteryawspeedfix is the later SDK path (yawSpeed * delta * 2) and
   sv.time does not move between thinks, so that path never sees a
   frame. The host pump is the frametime this call would have had. */
static void EFW_PeChangeYaw( CBaseMonster *pMon, int yawSpeed )
{
	float savedFix;
	float savedFrame;

	if( !pMon )
		return;
	savedFix = monsteryawspeedfix.value;
	savedFrame = gpGlobals->frametime;
	monsteryawspeedfix.value = 0.0f;
	gpGlobals->frametime = EFW_HostInterval();
	pMon->ChangeYaw( yawSpeed );
	monsteryawspeedfix.value = savedFix;
	gpGlobals->frametime = savedFrame;
}

/* FUN_1009b420: if bits_CAP_TURN_HEAD, yaw toward the point and
   SetBoneController(0). Spawn writes that capability as 0x7c0. */
static void EFW_IdleHeadTurn( CBaseMonster *pMon, const Vector &spot )
{
	float yaw;

	if( !pMon || !( pMon->m_afCapability & bits_CAP_TURN_HEAD ) )
		return;
	if( !pMon->pev->modelindex )
		return;
	yaw = UTIL_VecToYaw( spot - pMon->pev->origin ) - pMon->pev->angles.y;
	if( yaw > 180.0f )
		yaw -= 360.0f;
	if( yaw < -180.0f )
		yaw += 360.0f;
	pMon->SetBoneController( 0, yaw );
}

static float EFW_NpcGroundSpeed( CBaseMonster *pMon )
{
	float speed;

	speed = pMon->m_flGroundSpeed * pMon->pev->framerate;
	if( speed < 1.0f )
		speed = 64.0f;
	return speed;
}

/* FUN_1005d160 anim half: StudioFrameAdvance, idle fidget, DispatchAnimEvents.
   Move()'s WALK_MOVE is not called. Returns the interval MoveExecute uses. */
static float EFW_AdvanceNpcAnim( CBaseMonster *pMon, const char *name )
{
	entvars_t *pev;
	float flInterval;
	float skew;

	if( !pMon )
		return 0.0f;
	pev = pMon->pev;
	if( !pev->modelindex || s_npcStep )
		return 0.0f;
	/* Physics runs before MonsterThink. Land, then animate. */
	EFW_NpcFall( pev );
	if( s_npcStep )
		return 0.0f;
	/* MonsterThink calls StudioFrameAdvance(0). ResetSequenceInfo sets
	   animtime to gpGlobals->time, so that call returns 0 and the feet
	   do not move on the SetActivity think. A clock that was just reset
	   (or still 0) gets interval 0. After the call, animtime is stamped
	   0.25s ahead. That keeps the next skew above the hold, and it meets
	   the client clock, which leads gpGlobals->time by that same 0.25s.
	   StudioEstimateFrame adds (cl.time - animtime) * framerate * fps, so
	   a stamp one second behind drew the pose 1.25s ahead of this frame.
	   A stamp a full second ahead makes the step-position ratio largely
	   negative and walks the model off its origin. */
	if( !pev->animtime )
		pev->animtime = ( gpGlobals->time > 0.0f ) ? gpGlobals->time : 0.001f;
	skew = pev->animtime - gpGlobals->time;
	if( skew < 0.0f )
		skew = -skew;
	if( skew <= 0.001f )
		flInterval = 0.0f;
	else
		flInterval = EFW_HostInterval();
	flInterval = pMon->StudioFrameAdvance( flInterval );
	if( pMon->m_MonsterState != MONSTERSTATE_SCRIPT && pMon->m_MonsterState != MONSTERSTATE_DEAD
		&& pMon->m_Activity == ACT_IDLE && pMon->m_fSequenceFinished )
	{
		int iSequence;

		if( pMon->m_fSequenceLoops )
			iSequence = pMon->LookupActivity( pMon->m_Activity );
		else
			iSequence = pMon->LookupActivityHeaviest( pMon->m_Activity );
		if( iSequence != ACTIVITY_NOT_AVAILABLE )
		{
			pev->sequence = iSequence;
			pMon->ResetSequenceInfo();
			{
				static int s_fidget;
				if( s_fidget < 4 )
				{
					s_fidget++;
					EFW_DebugPrint( "idle fidget %s seq=%d",
						( name && name[0] ) ? name : "?", pev->sequence );
				}
			}
		}
	}
	pMon->DispatchAnimEvents( flInterval );
	if( skew <= 0.001f && pMon->m_Activity == ACT_WALK && name && EFW_FStrEq( name, "Amir" ) )
	{
		static int s_hold;
		if( s_hold < 4 )
		{
			s_hold++;
			EFW_DebugPrint( "seq hold Amir frame=%.1f origin=%.0f %.0f",
				pev->frame, pev->origin.x, pev->origin.y );
		}
	}
	{
		static int s_frame;
		if( s_frame < 6 && ( pMon->m_Activity == ACT_WALK || pMon->m_Activity == ACT_RUN ) )
		{
			s_frame++;
			EFW_DebugPrint( "walk frame %s seq=%d frame=%.1f iv=%.3f gs=%.0f",
				( name && name[0] ) ? name : "?", pev->sequence, pev->frame,
				flInterval, pMon->m_flGroundSpeed );
		}
	}
	/* StudioFrameAdvance and ResetSequenceInfo both store animtime = now.
	   Leave it 0.25s ahead so the next pump is not another hold and the
	   client does not add a second of sequence on top of this frame. */
	pev->animtime = gpGlobals->time + 0.25f;
	return flInterval;
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
	/* 0x100c64c0 SET_SIZE (-16,-16,0)-(16,16,72) every think, including
	   the deadflag==2 path after it zeroes the box. The link virtual is
	   the base abs box, so this write is the hull the player stops on. */
	EFW_WriteLinkedHull( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 72 ) );
	/* FUN_1005d160 ends IdleThink with vtable+8(0.1), overwriting the
	   earlier +3. Frozen sv.time never reaches it, so the pulse is the think. */
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
	/* The standing box was written above. pfnSetSize from the think
	   re-enters the link virtual and stalls the frame. */
	pPlayer = EFW_Player();
	/* 0x100c8160 is strstr(targetname, "queue"), not exact equality.
	   The yard line is named detainee_queue and must not approach. */
	if( pPlayer && tn && strstr( tn, "queue" ) )
	{
		delta = pPlayer->pev->origin - pev->origin;
		dist = delta.Length();
		if( m_Activity == ACT_RESET )
			SetActivity( ACT_IDLE );
		pev->movetype = MOVETYPE_STEP;
		{
			static int s_qtick;
			static int s_qlog;
			s_qtick++;
			if( s_qlog < 6 && dist > 100.0f && dist < 300.0f && ( s_qtick % 15 ) == 1 )
			{
				s_qlog++;
				EFW_DebugPrint( "queue stay %s dist=%.0f origin=%.0f %.0f",
					tn, dist, pev->origin.x, pev->origin.y );
			}
		}
	}
	else if( pPlayer && !EFW_FStrEq( tn, "queue" ) )
	{
		/* IdleThink pushes 3.0 through vtable+8, then FUN_1005d160
		   pushes 0.1 and that write is the one left in nextthink.
		   Queue names never enter this branch. */
		float now = EFW_HostClock();
		int slot = ENTINDEX( edict() );
		static float s_nextPe[512];
		static int s_peLog;
		int peThink = 0;

		if( slot > 0 && slot < 512
			&& ( s_nextPe[slot] <= 0.0f || now >= s_nextPe[slot] ) )
		{
			s_nextPe[slot] = now + 0.1f;
			peThink = 1;
			if( s_peLog < 4 )
			{
				s_peLog++;
				EFW_DebugPrint( ">>> FUN_1005d160 +0.1 %s t=%.1f",
					( tn && tn[0] ) ? tn : "?", now );
			}
		}
		if( peThink )
		{
		{
			static float s_slipAt;
			/* The roster thinks in one pump, so DAT_10132ca8 strides by
			   that count. The count shares a factor of 2 with 0x52, and
			   the other half never lands on the multiple (Nasir stayed
			   idle in front of the camera while Mouhtaz walked). PE frames
			   are shorter than the 0.1s think, so membership slips. One
			   extra count per pump is that slip. */
			if( s_slipAt != now )
			{
				static int s_slipLog;
				s_slipAt = now;
				s_walkTick++;
				if( s_slipLog < 1 )
				{
					s_slipLog = 1;
					EFW_DebugPrint( "walk slip tick=%d", s_walkTick );
				}
			}
		}
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
			/* FUN_1005d500 is this first Move. Later thinks keep stepping
			   from FUN_1005d160 while the goal stays in the 100..300 band. */
			{
				float speed = EFW_NpcGroundSpeed( this );
				float dt = EFW_HostInterval();
				int moved;
				static int s_peMove;

				if( dt < 0.001f )
					dt = 0.05f;
				moved = EFW_StepNpc( pev, pPlayer->pev->origin, speed, dt, pPlayer->edict() );
				if( s_peMove < 6 )
				{
					s_peMove++;
					EFW_DebugPrint( "pe move %s moved=%d dist=%.0f origin=%.0f %.0f",
						( tn && tn[0] ) ? tn : "?", moved, dist,
						pev->origin.x, pev->origin.y );
				}
			}
		}
		else if( m_movementActivity == ACT_WALK && dist < 100.0f )
		{
			/* 0x100c6654: movement activity ACT_WALK and dist < 100 calls
			   vtable+0x1a8(ACT_IDLE). Move() at 0x1005d500 runs only in the
			   100..300 band. ResetSequenceInfo sets animtime to now, so the
			   host-interval advance must not play that sequence; framerate 0
			   is that zero step. The next IdleThink entry restores it. */
			SetActivity( ACT_IDLE );
			pev->framerate = 0.0f;
			EFW_ClearDetour( pev );
			{
				static int s_close;
				static float s_yaw;
				static float s_px, s_py;
				static int s_held;
				if( s_close < 3 )
				{
					s_close++;
					s_yaw = pev->angles.y;
					s_px = pPlayer->pev->origin.x;
					s_py = pPlayer->pev->origin.y;
					EFW_DebugPrint( "close idle %s yaw=%.0f dist=%.0f player=%.0f %.0f",
						( tn && tn[0] ) ? tn : "?", pev->angles.y, dist,
						pPlayer->pev->origin.x, pPlayer->pev->origin.y );
				}
				else if( !s_held )
				{
					float dx = pPlayer->pev->origin.x - s_px;
					float dy = pPlayer->pev->origin.y - s_py;
					if( dx * dx + dy * dy > 80.0f * 80.0f )
					{
						s_held = 1;
						EFW_DebugPrint( "close hold %s yaw %.0f -> %.0f player=%.0f %.0f",
							( tn && tn[0] ) ? tn : "?", s_yaw, pev->angles.y,
							pPlayer->pev->origin.x, pPlayer->pev->origin.y );
					}
				}
			}
		}
		else if( m_movementGoal == MOVEGOAL_TARGETENT
			&& m_movementActivity == ACT_WALK
			&& dist > 100.0f && dist < 300.0f )
		{
			/* FUN_1005d160 calls vtable+0x154 (0x1005f200) on every think
			   once the modulo branch has stored the target goal. That is
			   the rest of the walk. Move() itself stalls, so one hull
			   step is that call. */
			float speed = EFW_NpcGroundSpeed( this );
			float dt = EFW_HostInterval();
			int moved;
			static int s_follow;

			if( dt < 0.001f )
				dt = 0.05f;
			moved = EFW_StepNpc( pev, pPlayer->pev->origin, speed, dt, pPlayer->edict() );
			if( s_follow < 8 )
			{
				s_follow++;
				EFW_DebugPrint( "pe follow %s moved=%d dist=%.0f origin=%.0f %.0f",
					( tn && tn[0] ) ? tn : "?", moved, dist,
					pev->origin.x, pev->origin.y );
			}
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
	}
	/* FUN_1005d160 still advances the pose on the pulses the 0.1s
	   gate skips, so the client frame does not stall. */
	EFW_AdvanceNpcAnim( this, tn );
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
	/* Same 0x1000cde0 as refugees: idle/walk 70, run 90. */
	switch( m_Activity )
	{
	case ACT_RUN:
		pev->yaw_speed = 90;
		break;
	default:
		pev->yaw_speed = 70;
		break;
	}
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
	/* FUN_100c5c50: first TRACE_LINE is origin to origin. If that
	   fraction is below 1, both ends are raised 40 and traced again. */
	from = pev->origin;
	to = pPlayer->pev->origin;
	UTIL_TraceLine( from, to, ignore_monsters, edict(), &tr );
	{
		float feet = tr.flFraction;
		float chest = -1.0f;
		int hit;
		if( feet < 1.0f )
		{
			from.z += 40.0f;
			to.z += 40.0f;
			UTIL_TraceLine( from, to, ignore_monsters, edict(), &tr );
			chest = tr.flFraction;
		}
		hit = ( feet >= 1.0f || chest >= 1.0f ) ? 1 : 0;
		{
			static int s_ray;
			if( s_ray < 4 )
			{
				const char *rn = STRING( pev->targetname );
				s_ray++;
				EFW_DebugPrint( "see ray %s feet=%.2f z=%.0f->%.0f chest=%.2f hit=%d",
					( rn && rn[0] ) ? rn : "?", feet, pev->origin.z,
					pPlayer->pev->origin.z, chest, hit );
			}
		}
		return hit;
	}
}

int CPatrolGuard::CanHearPlayer( CBasePlayer *pPlayer )
{
	(void)pPlayer;
	/* FUN_100c5e30 is 29 bytes. this+0x2e4 is m_iTriggerCondition, written
	   by KeyValue for "TriggerCondition". 8 is AITRIGGER_HEARPLAYER. The
	   function clears that field and returns 1. It does not read velocity.
	   Level 2 guards have no TriggerCondition key, so this stays 0. */
	if( m_iTriggerCondition != AITRIGGER_HEARPLAYER )
		return 0;
	{
		static int s_hear;
		if( !s_hear )
		{
			s_hear = 1;
			EFW_DebugPrint( ">>> FUN_100c5e30" );
		}
	}
	m_iTriggerCondition = AITRIGGER_NONE;
	return 1;
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
		/* FUN_100c5480: EHANDLE +0x168 = player, MoveToTarget(ACT_RUN), alert 4.
		   FRefreshRoute stalls, so only the goal fields are stored. */
		pg->m_hTargetEnt = pPlayer;
		pg->m_hEnemy = pPlayer;
		pg->m_moveWaitTime = 0;
		pg->m_movementActivity = ACT_RUN;
		pg->m_movementGoal = MOVEGOAL_TARGETENT;
		pg->m_iAlert = 4;
		if( pPlayer )
			pg->m_vecLastSeen = pPlayer->pev->origin;
		if( pg->m_Activity != ACT_RUN )
			pg->SetActivity( ACT_RUN );
		{
			const char *gn = STRING( pg->pev->targetname );
			EFW_DebugPrint( "patrol alert RUN %s seq=%d act=%d",
				( gn && gn[0] ) ? gn : "?", pg->pev->sequence, (int)pg->m_Activity );
		}
	}
}

void CPatrolGuard::PatrolThink( void )
{
	CBasePlayer *pPlayer = EFW_Player();
	const char *tn = STRING( pev->targetname );
	int see = 0;
	int hear = 0;
	/* FUN_100c5b60 reads gpGlobals->time. The listen server stays paused,
	   so that clock never reaches the 0.7s notice or the later 1s and 5s
	   waits. The pump clock is those seconds. nextthink stays on sv.time;
	   the StartFrame pulse is what runs this think. */
	float now = EFW_HostClock();
	{
		static int s_patrol;
		if( !s_patrol )
		{
			s_patrol = 1;
			EFW_DebugPrint( ">>> FUN_100c54e0" );
		}
	}

	pev->nextthink = gpGlobals->time + 0.1f;
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
	/* FUN_100c54e0 writes movetype 4 (MOVETYPE_STEP) on the non-pause path. */
	pev->movetype = MOVETYPE_STEP;
	{
		static int s_mv;
		if( !s_mv )
		{
			s_mv = 1;
			EFW_DebugPrint( "patrol movetype STEP" );
		}
	}
	/* The print call sits before the switch and consumes TriggerCondition.
	   Each case calls FUN_100c5e30 again, so the state machine does not
	   see the same true result. */
	if( CanHearPlayer( pPlayer ) )
		EFW_DebugPrint( "can hear player!!!!!!!!" );
	see = CanSeePlayer( pPlayer );
	hear = CanHearPlayer( pPlayer );

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
			EFW_DebugPrint( "efw: patrol notice %s dt=%.2f",
				( tn && tn[0] ) ? tn : "?", now - m_flAlertTime );
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
	{
		static int s_sense;
		float spd = 0.0f;
		if( pPlayer )
		{
			Vector vel = pPlayer->pev->velocity;
			vel.z = 0;
			spd = vel.Length();
		}
		if( ( see || hear || m_iAlert ) && s_sense < 6 )
		{
			s_sense++;
			EFW_DebugPrint( "patrol sense %s see=%d hear=%d alert=%d vel=%.0f trig=%d",
				( tn && tn[0] ) ? tn : "?", see, hear, m_iAlert, spd,
				m_iTriggerCondition );
		}
		if( see && spd > 80.0f )
		{
			static int s_runSee;
			if( s_runSee < 4 )
			{
				s_runSee++;
				EFW_DebugPrint( "patrol sense run %s see=%d hear=%d alert=%d vel=%.0f",
					( tn && tn[0] ) ? tn : "?", see, hear, m_iAlert, spd );
			}
		}
	}

	/* Tail of FUN_100c54e0. MoveToTarget / MoveToLocation call FRefreshRoute
	   (WALK_MOVE) and stall, so only their goal fields are stored. Alert 0
	   falls through to MonsterThink; the path_corner walk stands in for the
	   schedule RunAI would run and that we still cannot call. */
	if( m_iAlert == 4 )
	{
		if( m_movementGoal == MOVEGOAL_NONE && pPlayer )
		{
			m_hTargetEnt = pPlayer;
			m_moveWaitTime = 0;
			m_movementActivity = ACT_RUN;
			m_movementGoal = MOVEGOAL_TARGETENT;
			if( m_Activity != ACT_RUN )
			{
				SetActivity( ACT_RUN );
				EFW_DebugPrint( "patrol chase RUN %s seq=%d",
					( tn && tn[0] ) ? tn : "?", pev->sequence );
			}
		}
		m_hEnemy = pPlayer;
	}
	else if( m_iAlert == 2 || m_iAlert == 3 )
	{
		Vector seen = m_vecLastSeen - pev->origin;
		seen.z = 0;
		/* Move() advances a location route at ShouldAdvanceRoute's 8. */
		if( seen.Length() > 8.0f )
		{
			m_moveWaitTime = 0;
			m_movementActivity = ACT_WALK;
			m_movementGoal = MOVEGOAL_LOCATION;
			m_vecMoveGoal = m_vecLastSeen;
			if( m_Activity != ACT_WALK )
			{
				SetActivity( ACT_WALK );
				EFW_DebugPrint( "patrol investigate WALK %s seq=%d",
					( tn && tn[0] ) ? tn : "?", pev->sequence );
			}
		}
		if( pPlayer )
			m_hTargetEnt = pPlayer;
	}
	else if( m_Activity == ACT_RESET )
		SetActivity( ACT_IDLE );
	if( m_iAlert == 2 || m_iAlert == 3 || m_iAlert == 4 )
		EFW_IdleHeadTurn( this, m_vecLastSeen );

	/* MonsterThink anim, then the hull step stands in for Move. */
	float flInterval = EFW_AdvanceNpcAnim( this, tn );
	{
		static entvars_t *s_animWho;
		static int s_animLog;
		if( !s_animWho )
			s_animWho = pev;
		if( pev == s_animWho && s_animLog < 8 )
		{
			s_animLog++;
			EFW_DebugPrint( "patrol anim %s alert=%d seq=%d frame=%.2f act=%d mt=%d",
				( tn && tn[0] ) ? tn : "?", m_iAlert, pev->sequence, pev->frame,
				(int)m_Activity, pev->movetype );
		}
	}
	if( m_movementGoal == MOVEGOAL_TARGETENT && pPlayer && Dist2D( pPlayer ) > 8.0f )
	{
		float speed = EFW_NpcGroundSpeed( this );
		int moved = EFW_StepNpc( pev, pPlayer->pev->origin, speed, flInterval, pPlayer->edict() );
		{
			static int s_chaseLog;
			if( s_chaseLog < 6 )
			{
				s_chaseLog++;
				EFW_DebugPrint( "patrol chase step %s moved=%d seq=%d act=%d spd=%.0f origin=%.0f %.0f dist=%.0f",
					( tn && tn[0] ) ? tn : "?", moved, pev->sequence, (int)m_Activity, speed,
					pev->origin.x, pev->origin.y, Dist2D( pPlayer ) );
			}
		}
	}
	else if( m_movementGoal == MOVEGOAL_LOCATION )
	{
		Vector delta = m_vecMoveGoal - pev->origin;
		float remain;
		delta.z = 0;
		remain = delta.Length();
		/* FUN_1005f6e0: the waypoint distance is 2D, and the route
		   advances at 8. A 12-unit stop left the last-seen walk short. */
		if( remain > 8.0f )
		{
			if( remain < 20.0f )
			{
				static int s_close;
				if( s_close < 6 )
				{
					s_close++;
					EFW_DebugPrint( "investigate close %s dist=%.1f",
						( tn && tn[0] ) ? tn : "?", remain );
				}
			}
			/* Move (0x1005f200) passes m_hTargetEnt when the goal is
			   MOVEGOAL_LOCATION. A probe that meets that edict is a
			   clear walk, and the hull step still stops on the body. */
			EFW_StepNpc( pev, m_vecMoveGoal, EFW_NpcGroundSpeed( this ), flInterval,
				pPlayer ? pPlayer->edict() : NULL );
		}
		else
		{
			{
				static int s_arrive;
				if( s_arrive < 4 )
				{
					s_arrive++;
					EFW_DebugPrint( "investigate arrive %s dist=%.1f",
						( tn && tn[0] ) ? tn : "?", remain );
				}
			}
			m_movementGoal = MOVEGOAL_NONE;
			if( m_Activity != ACT_IDLE )
				SetActivity( ACT_IDLE );
		}
	}
	else if( ( m_iAlert == 0 || m_iAlert == 1 ) && !FStringNull( pev->target ) )
	{
		if( !m_pGoalEnt )
			m_pGoalEnt = UTIL_FindEntityByTargetname( NULL, STRING( pev->target ) );
		if( m_pGoalEnt )
		{
			Vector delta = m_pGoalEnt->pev->origin - pev->origin;
			float cornerDist;
			float speed;
			int moved;
			delta.z = 0;
			cornerDist = delta.Length();
			moved = 0;
			/* Move faces this corner, then ShouldAdvanceRoute. The leftover
			   budget stays that distance. MoveExecute walks the next point. */
			if( m_Activity != ACT_WALK )
				SetActivity( ACT_WALK );
			{
				CBaseEntity *next = NULL;
				Vector faceAt = m_pGoalEnt->pev->origin;
				Vector nextAt;
				int haveNext = 0;

				if( cornerDist <= 8.0f && !FStringNull( m_pGoalEnt->pev->target ) )
				{
					next = UTIL_FindEntityByTargetname( NULL, STRING( m_pGoalEnt->pev->target ) );
					if( next )
					{
						nextAt = next->pev->origin;
						haveNext = 1;
					}
				}
			if( m_Activity == ACT_WALK && cornerDist > 1.0f )
			{
				speed = EFW_NpcGroundSpeed( this );
				moved = EFW_StepNpc( pev, faceAt, speed, flInterval, NULL,
					haveNext ? &nextAt : NULL );
				{
					static int s_stepLog;
					if( s_stepLog < 6 )
					{
						s_stepLog++;
						EFW_DebugPrint( "patrol step %s moved=%d seq=%d spd=%.0f origin=%.0f %.0f",
							( tn && tn[0] ) ? tn : "?", moved, pev->sequence, speed,
							pev->origin.x, pev->origin.y );
					}
				}
			}
			if( haveNext )
			{
				static int s_hold;

				if( s_hold < 4 )
				{
					s_hold++;
					EFW_DebugPrint( "corner hold %s dist=%.1f moved=%d origin=%.0f %.0f next=%.0f %.0f",
						( tn && tn[0] ) ? tn : "?", cornerDist, moved,
						pev->origin.x, pev->origin.y,
						nextAt.x, nextAt.y );
				}
				/* A zero-interval SetActivity think returns before the
				   leftover. Keep this corner so the next think still
				   spends that distance toward the point AdvanceRoute stores. */
				if( moved || cornerDist <= 1.0f )
					m_pGoalEnt = next;
			}
			}
		}
	}
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
	/* FUN_1000d1d0: m_afCapability = 0x7c0 (use, hear, doors, turn head). */
	m_afCapability = bits_CAP_USE | bits_CAP_HEAR | bits_CAP_AUTO_DOORS
		| bits_CAP_OPEN_DOORS | bits_CAP_TURN_HEAD;
	m_flAlertTime = 0;
	m_flStateTime = 0;
	m_iCaught = 0;
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

/* monster_barney in the PE is this mod's officer/electrician, not a stock
   security guard. Spawn's MonsterInit stores MonsterInitThink, which jumps
   to StartMonster. StartMonster's DROP_TO_FLOOR and WALK_MOVE stall once
   the studio is bound, so this is the safe tail: STEP, MonsterThink's
   animation, and the path_corner walk StartMonster would schedule. */
void EFW_OfficerThink( CBaseMonster *pMon )
{
	entvars_t *pev;
	const char *tn;

	if( !pMon )
		return;
	pev = pMon->pev;
	if( !pev->modelindex || s_npcStep )
		return;
	tn = STRING( pev->targetname );
	if( EFW_GetHudInt( 6 ) )
	{
		pev->framerate = 0.0f;
		pev->movetype = MOVETYPE_NONE;
		return;
	}
	pev->framerate = 1.0f;
	/* Roster_Officer (166 -962 0) stands inside the roster desk. Mail_Officer
	   (326 -899 10) and efw_compound_gate_guard (310 -435 0) stand inside the
	   yard tarp. A feet hull still fits in those pockets, so a startsolid
	   test leaves the mesh under the cover. MOVETYPE_STEP then shoves a
	   raised hull back under the floor. Step +X, the open side, drop the
	   feet hull onto the floor there, and keep the spot only when a
	   chest-height ray from 64 units that way is clear. Skip the step push
	   so the engine cannot put them back under the cover. */
	{
		int slot;

		slot = -1;
		if( tn && !strcmp( tn, "Roster_Officer" ) )
			slot = 0;
		else if( tn && !strcmp( tn, "Mail_Officer" ) )
			slot = 1;
		else if( tn && !strcmp( tn, "efw_compound_gate_guard" ) )
			slot = 2;
		if( slot >= 0 )
		{
			static Vector s_home[3];
			static int s_haveHome[3];
			static int s_stood[3];

			if( !s_haveHome[slot] )
			{
				s_home[slot] = pev->origin;
				s_home[slot].z = 0.0f;
				s_haveHome[slot] = 1;
			}
			if( !s_stood[slot] )
			{
				int ring;
				int ringMax;
				static const float kDrop[3] = { 80.0f, 64.0f, 48.0f };

				/* Roster keeps the office floor at z=44. Mail and the gate
				   guard keep the yard gravel at z=0. A drop onto the tarp
				   (mail landed at z=61) still has a clear chest ray above
				   the cover, so a yard landing above the gravel is dropped. */
				ringMax = ( slot == 0 ) ? 80 : 160;
				for( ring = 0; ring <= ringMax; ring += 4 )
				{
					int hi;
					int landed;
					Vector stood;
					Vector from;
					Vector chest;
					TraceResult los;

					stood.x = s_home[slot].x + (float)ring;
					stood.y = s_home[slot].y;
					stood.z = 0.0f;
					landed = 0;
					if( slot == 0 )
					{
						TraceResult tr;
						Vector end;

						stood.z = 44.0f;
						end = stood;
						end.z += 1.0f;
						EFW_TraceFeetHull( pev, stood, end, &tr );
						if( tr.fStartSolid || tr.fAllSolid )
							continue;
						landed = 1;
					}
					else
					{
						for( hi = 0; hi < 3 && !landed; hi++ )
						{
							TraceResult tr;
							Vector top;
							Vector bot;

							top.x = stood.x;
							top.y = stood.y;
							top.z = kDrop[hi];
							bot = top;
							bot.z = -16.0f;
							EFW_TraceFeetHull( pev, top, bot, &tr );
							if( tr.fStartSolid || tr.fAllSolid )
								continue;
							if( tr.flFraction >= 1.0f || tr.flFraction <= 0.0f )
								continue;
							stood.z = tr.vecEndPos.z;
							landed = 1;
						}
						if( !landed || stood.z > 16.0f )
						{
							stood.z = 0.0f;
							landed = 1;
						}
					}
					if( !landed )
						continue;
					from = stood;
					from.x += 64.0f;
					from.z += 48.0f;
					chest = stood;
					chest.z += 48.0f;
					UTIL_TraceLine( from, chest, ignore_monsters, ENT( pev ), &los );
					if( los.fStartSolid || los.fAllSolid || los.flFraction < 0.99f )
						continue;
					EFW_QueueOrigin( pev, stood );
					EFW_DebugPrint( "npc stand %s x=%.0f y=%.0f z=%.0f ring=%d",
						tn, stood.x, stood.y, stood.z, ring );
					s_stood[slot] = 1;
					break;
				}
			}
			/* The +X step leaves the desk and the tarp. Map yaw 180
			   looks back along -X into that cover, so the open side
			   meets the back. Face +X. The gate guard's yaw 270 looks
			   along the gate, not into the tarp, and stays. */
			if( s_stood[slot] && slot != 2 )
			{
				float yaw = pev->angles.y;
				while( yaw < 0.0f )
					yaw += 360.0f;
				while( yaw >= 360.0f )
					yaw -= 360.0f;
				if( yaw > 1.0f )
				{
					static int s_face;
					pev->angles.y = 0.0f;
					pev->ideal_yaw = 0.0f;
					if( s_face < 2 )
					{
						s_face++;
						EFW_DebugPrint( "officer face %s yaw=0", tn );
					}
				}
			}
			pev->movetype = MOVETYPE_NONE;
			pev->velocity = Vector( 0, 0, 0 );
			/* FUN_1000d1d0 calls vtable+0x134, MonsterInit. StartMonster
			   then sets ACT_IDLE, and that fills m_flFrameRate from
			   look_idle. Without it StudioFrameAdvance adds nothing and
			   the uniform stays on frame 0. */
			if( pMon->m_Activity == ACT_RESET )
			{
				pMon->SetActivity( ACT_IDLE );
				{
					static int s_idle;
					if( s_idle < 3 )
					{
						s_idle++;
						EFW_DebugPrint( "officer idle %s seq=%d rate=%.0f",
							tn, pev->sequence, pMon->m_flFrameRate );
					}
				}
			}
			EFW_AdvanceNpcAnim( pMon, tn );
			return;
		}
	}
	pev->movetype = MOVETYPE_STEP;
	{
		static int s_mv;
		if( !s_mv )
		{
			s_mv = 1;
			EFW_DebugPrint( "officer movetype STEP" );
		}
	}
	if( !FStringNull( pev->target ) )
	{
		if( !pMon->m_pGoalEnt )
			pMon->m_pGoalEnt = UTIL_FindEntityByTargetname( NULL, STRING( pev->target ) );
	}
	/* The corner within 8 is still this think's goal. AdvanceRoute runs
	   after the leftover step, so SetActivity stays on the walk he is in. */
	if( pMon->m_pGoalEnt && !FStringNull( pev->target ) )
	{
		if( pMon->m_Activity != ACT_WALK )
			pMon->SetActivity( ACT_WALK );
	}
	else if( pMon->m_Activity == ACT_RESET || pMon->m_Activity == ACT_WALK )
	{
		if( pMon->m_Activity != ACT_IDLE )
			pMon->SetActivity( ACT_IDLE );
		EFW_ClearDetour( pev );
	}
	float flInterval = EFW_AdvanceNpcAnim( pMon, tn );
	{
		static int s_anim;
		if( s_anim < 8 && EFW_FStrEq( tn, "efw_electrician" ) )
		{
			s_anim++;
			EFW_DebugPrint( "officer anim %s seq=%d frame=%.2f act=%d mt=%d",
				tn, pev->sequence, pev->frame, (int)pMon->m_Activity, pev->movetype );
		}
	}
	if( pMon->m_pGoalEnt && pMon->m_Activity == ACT_WALK )
	{
		Vector delta = pMon->m_pGoalEnt->pev->origin - pev->origin;
		float cornerDist;
		float speed;
		int moved;
		delta.z = 0;
		cornerDist = delta.Length();
		moved = 0;
		/* ShouldAdvanceRoute uses the distance from the start of Move.
		   AdvanceRoute stores the next corner, then MoveExecute spends
		   that leftover toward it. Yaw stays on the corner just reached. */
		{
			CBaseEntity *next = NULL;
			Vector faceAt = pMon->m_pGoalEnt->pev->origin;
			Vector nextAt;
			int haveNext = 0;

			if( cornerDist <= 8.0f && !FStringNull( pMon->m_pGoalEnt->pev->target ) )
			{
				next = UTIL_FindEntityByTargetname( NULL, STRING( pMon->m_pGoalEnt->pev->target ) );
				if( next )
				{
					nextAt = next->pev->origin;
					haveNext = 1;
				}
			}
			if( cornerDist > 1.0f )
			{
				speed = EFW_NpcGroundSpeed( pMon );
				moved = EFW_StepNpc( pev, faceAt, speed, flInterval, NULL,
					haveNext ? &nextAt : NULL );
				{
					static int s_step;
					if( s_step < 6 && EFW_FStrEq( tn, "efw_electrician" ) )
					{
						s_step++;
						EFW_DebugPrint( "officer step %s moved=%d seq=%d spd=%.0f origin=%.0f %.0f",
							tn, moved, pev->sequence, speed, pev->origin.x, pev->origin.y );
					}
				}
			}
			if( haveNext )
			{
				static int s_hold;

				if( s_hold < 4 )
				{
					s_hold++;
					EFW_DebugPrint( "corner hold %s dist=%.1f moved=%d origin=%.0f %.0f next=%.0f %.0f",
						( tn && tn[0] ) ? tn : "?", cornerDist, moved,
						pev->origin.x, pev->origin.y,
						nextAt.x, nextAt.y );
				}
				if( moved || cornerDist <= 1.0f )
					pMon->m_pGoalEnt = next;
			}
		}
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
		/* Bind already set SOLID_BBOX. Leave it. PatrolThink sets
		   MOVETYPE_STEP once the model index exists. */
		pent->v.flags |= FL_MONSTER;
		return;
	}
	if( !strcmp( cn, "monster_barney" ) )
	{
		/* Stock CallMonsterThink runs StartMonster's route through WALK_MOVE.
		   The pulse calls EFW_OfficerThink instead. */
		pEnt->SetThink( NULL );
		pent->v.nextthink = 0;
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

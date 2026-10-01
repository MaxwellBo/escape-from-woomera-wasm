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
		EFW_WriteLinkedHull( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 72 ) );
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
			EFW_WriteLinkedHull( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 72 ) );
			return;
		}
	}
	EFW_WriteLinkedHull( pev, mins, maxs );
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
static void EFW_TraceFeetHull( entvars_t *pev, const Vector &start, const Vector &end, TraceResult *tr )
{
	Vector a;
	Vector b;

	a = start;
	b = end;
	a.z += 36.0f;
	b.z += 36.0f;
	*tr = TraceResult();
	UTIL_TraceHull( a, b, ignore_monsters, human_hull, ENT( pev ), tr );
	tr->vecEndPos.z -= 36.0f;
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
	if( support == 1 )
	{
		s_npcStep = 0;
		pev->flags |= FL_ONGROUND;
		pev->velocity.z = 0.0f;
		return;
	}
	if( support < 0 )
	{
		Vector stood;
		int up;
		/* Inside a hull-1 floor. The first clear feet spot above is the
		   surface the player stands on; a basement hit is the miss. */
		for( up = 1; up <= 24; up++ )
		{
			Vector raised = pev->origin;
			TraceResult hole;

			raised.z += (float)up * 4.0f;
			EFW_TraceFeetHull( pev, raised, raised, &hole );
			if( hole.fStartSolid || hole.fAllSolid )
				continue;
			if( EFW_LandMonster( pev, raised, &stood ) && stood.z > pev->origin.z + 1.0f )
			{
				static int s_out;

				if( s_out < 6 )
				{
					s_out++;
					EFW_DebugPrint( "floor escape z=%.0f -> %.0f at %.0f %.0f",
						pev->origin.z, stood.z, stood.x, stood.y );
				}
				pev->flags |= FL_ONGROUND;
				pev->velocity.z = 0.0f;
				s_npcStep = 0;
				EFW_QueueOrigin( pev, stood );
				return;
			}
			break;
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
   Trace the PE hull (-16..16, 0..72) and ignore other monsters so touch
   does not call back into think. MoveExecute walks
   groundSpeed * framerate * interval in chunks of 16 (the stair limit). */
static void EFW_PeChangeYaw( CBaseMonster *pMon, int yawSpeed );

/* SV_MoveToOrigin MOVE_NORMAL steps along ideal_yaw. When that step cannot
   be taken, SV_NewChaseDir2 tries the diagonal, the two cardinals, then
   the other 45-degree headings. This is that search with the hull trace,
   not WALK_MOVE. */
static int EFW_TryChunk( entvars_t *pev, const Vector &start, const Vector &dir, float step, Vector *out )
{
	Vector wish;
	Vector end;
	Vector stepLand;
	TraceResult tr;
	float savedMins;
	float horiz;

	wish = dir;
	wish.z = 0.0f;
	if( wish.Length() < 0.001f )
		return 0;
	wish = wish.Normalize();
	end = start + wish * step;
	savedMins = pev->mins.z;
	if( savedMins < 1.0f )
		pev->mins.z = 1.0f;
	EFW_TraceFeetHull( pev, start, end, &tr );
	if( tr.fAllSolid || tr.fStartSolid )
	{
		int stepUp;
		float baseZ = start.z;

		for( stepUp = 1; stepUp <= 9; stepUp++ )
		{
			Vector raised = start;

			raised.z = baseZ + stepUp * 2.0f;
			EFW_TraceFeetHull( pev, raised, raised, &tr );
			if( !tr.fStartSolid && !tr.fAllSolid )
			{
				end = raised + wish * step;
				EFW_TraceFeetHull( pev, raised, end, &tr );
				break;
			}
		}
	}
	pev->mins.z = savedMins;
	/* Same acceptance as the direct chunk: a destination with a floor
	   counts, even when the feet-level trace started in the ground. */
	if( EFW_LandMonster( pev, end, &stepLand ) )
	{
		horiz = ( stepLand - start ).Length2D();
		if( horiz >= 0.5f )
		{
			*out = stepLand;
			return 1;
		}
	}
	if( tr.fAllSolid || tr.fStartSolid || tr.flFraction <= 0.0f )
		return 0;
	stepLand = start + ( end - start ) * tr.flFraction;
	{
		Vector grounded;

		if( EFW_LandMonster( pev, stepLand, &grounded ) )
			stepLand = grounded;
		else
			stepLand.z = start.z;
	}
	horiz = ( stepLand - start ).Length2D();
	if( horiz < 0.5f )
		return 0;
	*out = stepLand;
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

static int EFW_ChaseChunk( entvars_t *pev, const Vector &start, const Vector &goal, float step, Vector *out )
{
	float deltax;
	float deltay;
	float dirx;
	float diry;
	float turnaround;
	float tryYaw[16];
	int ntry;
	int i;
	Vector landed;

	deltax = goal.x - start.x;
	deltay = goal.y - start.y;
	dirx = ( deltax > 10.0f ) ? 0.0f : ( deltax < -10.0f ) ? 180.0f : -1.0f;
	diry = ( deltay < -10.0f ) ? 270.0f : ( deltay > 10.0f ) ? 90.0f : -1.0f;
	turnaround = EFW_NormYaw360( ( (int)( pev->ideal_yaw / 45.0f ) ) * 45.0f - 180.0f );
	ntry = 0;
	if( dirx >= 0.0f && diry >= 0.0f )
	{
		float diag;

		if( dirx == 0.0f )
			diag = ( diry == 90.0f ) ? 45.0f : 315.0f;
		else
			diag = ( diry == 90.0f ) ? 135.0f : 225.0f;
		tryYaw[ntry++] = diag;
	}
	if( fabsf( deltay ) > fabsf( deltax ) )
	{
		if( diry >= 0.0f )
			tryYaw[ntry++] = diry;
		if( dirx >= 0.0f )
			tryYaw[ntry++] = dirx;
	}
	else
	{
		if( dirx >= 0.0f )
			tryYaw[ntry++] = dirx;
		if( diry >= 0.0f )
			tryYaw[ntry++] = diry;
	}
	tryYaw[ntry++] = EFW_NormYaw360( ( (int)( pev->ideal_yaw / 45.0f ) ) * 45.0f );
	for( i = 0; i < 8; i++ )
		tryYaw[ntry++] = (float)( i * 45 );
	tryYaw[ntry++] = turnaround;
	for( i = 0; i < ntry; i++ )
	{
		float yaw = EFW_NormYaw360( tryYaw[i] );
		Vector dir;
		int seen;
		int j;

		if( fabsf( yaw - turnaround ) < 0.5f && i + 1 != ntry )
			continue;
		seen = 0;
		for( j = 0; j < i; j++ )
		{
			if( fabsf( EFW_NormYaw360( tryYaw[j] ) - yaw ) < 0.5f )
			{
				seen = 1;
				break;
			}
		}
		if( seen )
			continue;
		dir.x = cosf( yaw * 0.01745329252f );
		dir.y = sinf( yaw * 0.01745329252f );
		dir.z = 0.0f;
		if( !EFW_TryChunk( pev, start, dir, step, &landed ) )
			continue;
		{
			static int s_chase;
			if( s_chase < 6 )
			{
				s_chase++;
				EFW_DebugPrint( "chase dir yaw=%.0f origin=%.0f %.0f -> %.0f %.0f",
					yaw, start.x, start.y, landed.x, landed.y );
			}
		}
		*out = landed;
		return 1;
	}
	{
		static int s_stuck;
		if( s_stuck < 4 )
		{
			s_stuck++;
			EFW_DebugPrint( "chase stuck %.0f %.0f", start.x, start.y );
		}
	}
	return 0;
}

static int EFW_StepNpc( entvars_t *pev, const Vector &goal, float speed, float dt )
{
	Vector delta;
	Vector start;
	Vector landed;
	float len;
	float total;
	float moved;
	int chunks;

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
	delta = goal - pev->origin;
	delta.z = 0.0f;
	len = delta.Length();
	if( len < 1.0f )
		return 0;
	/* Move() faces the goal with MakeIdealYaw + ChangeYaw(yaw_speed)
	   once, before the 16-unit chunks, including the think whose
	   StudioFrameAdvance interval is 0. */
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
			pMon->MakeIdealYaw( goal );
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
	   the anim call still has that clock; rewind it so the next pump is
	   not another hold. dt is the interval StudioFrameAdvance returned. */
	{
		float skew = pev->animtime - gpGlobals->time;
		if( skew < 0.0f )
			skew = -skew;
		if( dt < 0.001f || skew <= 0.001f )
		{
			if( skew <= 0.001f )
				pev->animtime = gpGlobals->time - 1.0f;
			return 0;
		}
	}
	total = speed * dt;
	if( total > len )
		total = len;
	if( total < 0.001f )
		return 0;
	s_npcStep = 1;
	start = pev->origin;
	landed = start;
	moved = 0.0f;
	chunks = 0;
	while( total > 0.001f && chunks < 8 )
	{
		Vector wish;
		Vector end;
		Vector stepLand;
		TraceResult tr;
		float savedMins;
		float step;
		float stepLen;
		float remain;

		delta = goal - start;
		delta.z = 0.0f;
		remain = delta.Length();
		if( remain < 1.0f )
			break;
		step = total;
		if( step > 16.0f )
			step = 16.0f;
		if( step > remain )
			step = remain;
		wish = delta * ( step / remain );
		end = start + wish;
		savedMins = pev->mins.z;
		if( savedMins < 1.0f )
			pev->mins.z = 1.0f;
		EFW_TraceFeetHull( pev, start, end, &tr );
		/* Feet-origin mins.z == 0 sits in the floor and the trace is
		   startsolid. StartMonster adds 1 to origin.z; DROP_TO_FLOOR
		   cannot lift an origin already inside the floor. */
		if( tr.fAllSolid || tr.fStartSolid )
		{
			int stepUp;
			float baseZ = start.z;
			/* sv_stepsize is 18. A taller climb is the pop past a ceiling. */
			for( stepUp = 1; stepUp <= 9; stepUp++ )
			{
				Vector raised = start;
				raised.z = baseZ + stepUp * 2.0f;
				EFW_TraceFeetHull( pev, raised, raised, &tr );
				if( !tr.fStartSolid && !tr.fAllSolid )
				{
					start = raised;
					end = start + wish;
					EFW_TraceFeetHull( pev, start, end, &tr );
					{
						static int s_lift;
						if( s_lift < 6 )
						{
							s_lift++;
							EFW_DebugPrint( "floor lift z=%.0f -> %.0f solid=%d frac=%.2f",
								baseZ, start.z, tr.fStartSolid, tr.flFraction );
						}
					}
					break;
				}
			}
		}
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
		/* Prefer the step-size landing at the full chunk. A blocked hull
		   falls back to the partial slide, then lands if a floor is near. */
		if( EFW_LandMonster( pev, end, &stepLand ) )
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
		}
		else if( tr.fAllSolid || tr.fStartSolid || tr.flFraction <= 0.0f )
		{
			Vector chest = start + Vector( 0, 0, 36 );
			UTIL_TraceHull( chest, chest + wish, ignore_monsters, point_hull, ENT( pev ), &tr );
			if( tr.fAllSolid || tr.fStartSolid || tr.flFraction <= 0.0f )
				break;
			stepLand = start + wish * tr.flFraction;
			stepLand.z = start.z;
		}
		else
		{
			stepLand = start + ( end - start ) * tr.flFraction;
			{
				Vector grounded;
				if( EFW_LandMonster( pev, stepLand, &grounded ) )
					stepLand = grounded;
				else
					stepLand.z = start.z;
			}
		}
		stepLen = ( stepLand - start ).Length();
		/* SV_MoveToOrigin: a blocked ideal_yaw step calls SV_NewChaseDir2
		   instead of stopping on the wall. */
		if( stepLen < 0.5f )
		{
			Vector chased;

			if( !EFW_ChaseChunk( pev, start, goal, step, &chased ) )
				break;
			stepLand = chased;
			stepLen = ( stepLand - start ).Length();
			if( stepLen < 0.5f )
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
	   do not move on the SetActivity think. sv.time does not advance
	   between pumps, so a literal 0 every think would freeze the pose.
	   A clock that was just reset (or still 0) gets interval 0. After
	   the call, animtime is rewound one second so the next stuck clock
	   is a real host interval. */
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
	   Leave it a second behind so the next pump is not another hold. */
	pev->animtime = gpGlobals->time - 1.0f;
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
			/* 0x100c6654: movement activity ACT_WALK and dist < 100 calls
			   vtable+0x1a8(ACT_IDLE). Move() at 0x1005d500 runs only in the
			   100..300 band. ResetSequenceInfo sets animtime to now, so the
			   host-interval advance must not play that sequence; framerate 0
			   is that zero step. The next IdleThink entry restores it. */
			SetActivity( ACT_IDLE );
			pev->framerate = 0.0f;
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
	/* FUN_1005d160 anim half, then the hull step stands in for Move.
	   The returned interval is 0 on the SetActivity think. */
	float flInterval = EFW_AdvanceNpcAnim( this, tn );
	/* this+0x284 is m_movementActivity. Step at the sequence ground speed. */
	if( pPlayer && !( tn && strstr( tn, "queue" ) )
		&& m_movementActivity == ACT_WALK && m_Activity == ACT_WALK
		&& dist > 100.0f
		&& m_movementGoal == MOVEGOAL_TARGETENT )
	{
		float speed;
		int moved;

		speed = EFW_NpcGroundSpeed( this );
		{
			static int s_why;
			if( s_why < 4 )
			{
				s_why++;
				EFW_DebugPrint( "step why flag=%d dt=%.3f ground=%d mi=%d",
					s_npcStep, flInterval,
					( pev->flags & FL_ONGROUND ) ? 1 : 0, pev->modelindex );
			}
		}
		{
			static int s_gateLog;
			if( s_gateLog < 8 )
			{
				s_gateLog++;
				EFW_DebugPrint( "IdleThink gate %s user=%d dist=%.0f spd=%.0f",
					( tn && tn[0] ) ? tn : "?", (int)m_movementActivity, dist, speed );
			}
		}
		moved = EFW_StepNpc( pev, pPlayer->pev->origin, speed, flInterval );
		{
			float left = ( pPlayer->pev->origin - pev->origin ).Length();
			static int s_approach;

			if( left < 130.0f && s_approach < 3 )
			{
				s_approach++;
				EFW_DebugPrint( "approach %s dist=%.0f origin=%.0f %.0f %.0f",
					( tn && tn[0] ) ? tn : "?", left,
					pev->origin.x, pev->origin.y, pev->origin.z );
			}
		}
		if( moved && EFW_FStrEq( tn, "Amir" ) )
		{
			static int s_amirStep;
			if( s_amirStep < 4 )
			{
				s_amirStep++;
				EFW_DebugPrint( "seq step Amir moved=%d origin=%.0f %.0f %.0f frame=%.1f",
					moved, pev->origin.x, pev->origin.y, pev->origin.z, pev->frame );
			}
		}
		{
			static int s_stepLog;
			static int s_stepN;
			static int s_sink;
			s_stepN++;
			if( s_stepLog < 8 )
			{
				s_stepLog++;
				EFW_DebugPrint( "IdleThink step %s moved=%d origin=%.0f %.0f %.0f dist=%.0f",
					( tn && tn[0] ) ? tn : "?", moved,
					pev->origin.x, pev->origin.y, pev->origin.z,
					( pPlayer->pev->origin - pev->origin ).Length() );
			}
			else if( s_stepN == 40 || s_stepN == 160 )
			{
				EFW_DebugPrint( "step late %s moved=%d origin=%.0f %.0f %.0f dist=%.0f",
					( tn && tn[0] ) ? tn : "?", moved,
					pev->origin.x, pev->origin.y, pev->origin.z,
					( pPlayer->pev->origin - pev->origin ).Length() );
			}
			if( pev->origin.z < -1.0f && s_sink < 4 )
			{
				s_sink++;
				EFW_DebugPrint( "npc sink %s z=%.0f at %.0f %.0f",
					( tn && tn[0] ) ? tn : "?",
					pev->origin.z, pev->origin.x, pev->origin.y );
			}
		}
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
		int moved = EFW_StepNpc( pev, pPlayer->pev->origin, speed, flInterval );
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
			EFW_StepNpc( pev, m_vecMoveGoal, EFW_NpcGroundSpeed( this ), flInterval );
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
			float speed;
			int moved;
			delta.z = 0;
			/* FUN_1005f6e0 ShouldAdvanceRoute: waypoint dist <= 8. */
			if( delta.Length() <= 8.0f && !FStringNull( m_pGoalEnt->pev->target ) )
			{
				{
					static int s_corner;
					if( s_corner < 4 )
					{
						s_corner++;
						EFW_DebugPrint( "corner advance %s dist=%.1f",
							( tn && tn[0] ) ? tn : "?", delta.Length() );
					}
				}
				m_pGoalEnt = UTIL_FindEntityByTargetname( NULL, STRING( m_pGoalEnt->pev->target ) );
				if( m_pGoalEnt )
				{
					delta = m_pGoalEnt->pev->origin - pev->origin;
					delta.z = 0;
				}
			}
			if( m_pGoalEnt && delta.Length() > 8.0f )
			{
				if( m_Activity != ACT_WALK )
					SetActivity( ACT_WALK );
				speed = EFW_NpcGroundSpeed( this );
				moved = EFW_StepNpc( pev, m_pGoalEnt->pev->origin, speed, flInterval );
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
		if( pMon->m_pGoalEnt )
		{
			Vector delta = pMon->m_pGoalEnt->pev->origin - pev->origin;
			delta.z = 0;
			/* FUN_1005f6e0 ShouldAdvanceRoute: waypoint dist <= 8. */
			if( delta.Length() <= 8.0f && !FStringNull( pMon->m_pGoalEnt->pev->target ) )
			{
				{
					static int s_corner;
					if( s_corner < 4 )
					{
						s_corner++;
						EFW_DebugPrint( "corner advance %s dist=%.1f",
							tn, delta.Length() );
					}
				}
				pMon->m_pGoalEnt = UTIL_FindEntityByTargetname( NULL, STRING( pMon->m_pGoalEnt->pev->target ) );
			}
		}
	}
	if( pMon->m_pGoalEnt && !FStringNull( pev->target ) )
	{
		Vector delta = pMon->m_pGoalEnt->pev->origin - pev->origin;
		delta.z = 0;
		if( delta.Length() > 8.0f && pMon->m_Activity != ACT_WALK )
			pMon->SetActivity( ACT_WALK );
	}
	else if( pMon->m_Activity == ACT_RESET || pMon->m_Activity == ACT_WALK )
	{
		if( pMon->m_Activity != ACT_IDLE )
			pMon->SetActivity( ACT_IDLE );
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
		float speed;
		int moved;
		delta.z = 0;
		if( delta.Length() > 8.0f )
		{
			speed = EFW_NpcGroundSpeed( pMon );
			moved = EFW_StepNpc( pev, pMon->m_pGoalEnt->pev->origin, speed, flInterval );
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

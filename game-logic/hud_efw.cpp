#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "triangleapi.h"
#include "screenfade.h"
#include "shake.h"
#include "const.h"
#include "usercmd.h"
#include "kbutton.h"
#include "ref_params.h"
#include "efw.h"
#include "pm_defs.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// Client half of EFWData / EFWShow / EFW_Menu / EFW_CtPrv / EFW_Cntxt
// (client.dll 0x100419e0, FUN_10044f70 prompts, FUN_10047830 storyboards).

#define EFW_MAX_SCAN 3
#define EFW_SCAN_NAME 29
#define EFW_SCAN_BYTES 0x30

#pragma pack(push, 1)
struct EfwScanSlot
{
	int type;
	char name[EFW_SCAN_NAME];
	char zero;
	unsigned char pad[2];
	float x, y, z;
};
#pragma pack(pop)

static float g_hope = -1;
static float g_clientHudFloat[2]; /* DAT_100bc498; FUN_10047650 / FUN_10047660 */
static int g_clientHudInt[7]; /* DAT_100bc4a0; FUN_10047670 */
static float g_hudDrawTime; /* DAT_100a95ac; FUN_1001db00 flTime */
static float g_contextOpenedAt; /* DAT_100bc354; FUN_10046370 */
static int g_hudMsgCount; /* DAT_100baed8; FUN_10041a20 */
static int g_hudMsgBase; /* DAT_100baedc; FUN_10041a30 */
/* DAT_100bc884 stride 0x10, codes 0x3c..0x52. FUN_10047720 sprintf. */
static char g_showMenuSlot[23][24];
static unsigned char g_panel48[0xd4]; /* FUN_10048650 operator_new(0xd4) Panel */
static int g_panel48On;
static int g_diaryPage;
static int g_diaryOpen;
static int g_mapLevel; /* hudInt[3]; FUN_1001db00 clock am/pm + hour base */
static int g_menuCode;
static int g_weaponId = -1;
static HSPRITE g_hDiary;
static HSPRITE g_hLogo; /* sprites/efw_artslogo.spr; FUN_1001db00 tail */
static int g_loadedPage = -1;
/* 0x100b7824, 14 slots, stride 0x3e8. Menu lines are 0..6. */
static char g_menuLine[7][1000];
static int g_menuOn;
static EfwScanSlot g_scan[EFW_MAX_SCAN];
static int g_scanCount;
static HSPRITE g_hBubble;
static HSPRITE g_hHide;
static HSPRITE g_hPliers;
static HSPRITE g_hItem[9]; /* ids 16..24, weapon_s from sprites/weapon_efw_*.txt */
static HSPRITE g_hGive;
static HSPRITE g_hStory;
static HSPRITE g_hGrey; /* FUN_100436c0 sprites/efw_grey.spr */
static HSPRITE g_hAscale; /* FUN_10044e30 sprites/ascale.spr */
static int g_storyCode;
static int g_storyPauseSent;
static char g_storyChange[64];
static float g_storyFade; /* DAT_100baf10 */
static int g_weaponMask;
static int g_contextMode; /* DAT_100bc338; FUN_10046370 / FUN_100463c0 */
static float g_contextDismissAt; /* DAT_100bc9f4; 0.4s debounce on 0x48 */
static char g_caption[1024]; /* DAT_100baf04; FUN_10048790 */
static int g_captionLen; /* DAT_100baf08 */
static float g_captionAt; /* DAT_100baf14 */
static float g_captionAge;
static int g_iconFlyOn; /* DAT_100bc384; FUN_100464c0 +0x24 */
static int g_iconFlyLog;
static float g_iconFlyAt; /* DAT_100bc388 */
static float g_iconFlyFrom[3]; /* DAT_100bc360 / 364 / 368 */
static float g_iconFlyTo[3]; /* DAT_100bc36c / 370 / 374 */
static float g_iconFlySize0; /* DAT_100bc378 */
static float g_iconFlySize1; /* DAT_100bc37c */
/* FUN_10046040 hover caption. The page reports the 32px-square hit;
   the glyphs are pfnDrawCharacter, not the VGUI scheme face. */
static char g_hotCap[192];
static int g_hotCapX;
static int g_hotCapY;
static float g_menuVeil; /* DAT_100a95b8; FUN_1001db00 conversation veil */
static float g_invFade; /* DAT_100a95bc; FUN_10043dd0(fade) */
static float g_diaryFade; /* DAT_100a95c4; diary SPR wipe */
static int g_diaryFadePage; /* DAT_100a95c0 */

#ifndef K_MOUSE1
#define K_MOUSE1 107
#define K_MOUSE2 108
#endif

static void EFW_StartIconFly( float x, float y, HSPRITE spr ); /* FUN_100464c0 */
static void EFW_FlyDismissedButton( void );
static void EFW_IconFly_f( void );
static HSPRITE EFW_ItemIcon( int id ); /* weapon+0xbc, the "weapon" line */
static int EFW_DrawWrapped( int x, int y, int xmax, const char *text, int r, int g, int b );
static void EFW_DrawTriQuad( float x1, float y1, float x2, float y2,
	float r, float g, float b, float v, HSPRITE spr );
static int EFW_AscaleAlpha( float v );

#define EFW_VGUI_MAX 6 /* FUN_100c6d70 DAT_10134894..a8 — six CommandButton slots */
struct EfwVguiBtn
{
	int x, y, w, h;
	char cmd[96];
	char label[64];
	char spr[8];
	HSPRITE icon;
};
static EfwVguiBtn g_vgui[EFW_VGUI_MAX];
static int g_vguiN;
static char g_vguiSig[512];

static HSPRITE EFW_LoadSpr( const char *path )
{
	return SPR_Load( path );
}

/* FUN_10042140 registers four keys at DAT_100baee8. The plate and the
   talk caption both resolve through FUN_10041fb0, not a second table. */
static void EFW_ClientRegisterDefaults( void )
{
	gEngfuncs.Con_Printf( ">>> FUN_10042140 n=4 Gate Guard Electrician Detainee Detainee in queue\n" );
}

/* FUN_10044880: map name → 2=level3, 1=level2, 0=level1. */
static int EFW_MapLevelFromName( void )
{
	const char *level;
	const char *base;
	int ml;
	static int s_logged = -1;

	level = gEngfuncs.pfnGetLevelName ? gEngfuncs.pfnGetLevelName() : NULL;
	if( !level )
		level = "";
	base = strrchr( level, '/' );
	if( !base )
		base = strrchr( level, '\\' );
	base = base ? base + 1 : level;
	if( !base[0] )
		return g_mapLevel;
	if( strstr( base, "efw_prototype_level3" ) )
		ml = 2;
	else if( strstr( base, "efw_prototype_level2" ) )
		ml = 1;
	else
		ml = 0;
	if( ml != s_logged && base[0] )
	{
		s_logged = ml;
		gEngfuncs.Con_Printf( ">>> FUN_10044880 level=%d %s\n", ml, base );
	}
	return ml;
}

/* FUN_10047650: *(DAT_100bc498 + idx*4) = v */
static void EFW_SetClientHudFloat( int idx, float v )
{
	if( idx >= 0 && idx < 2 )
		g_clientHudFloat[idx] = v;
}

/* FUN_10047660: return *(float*)(DAT_100bc498 + idx*4). Hope is slot 1. */
static float EFW_GetClientHudFloat( int idx )
{
	float v = 0.0f;
	static float s_loggedV = -9999.0f;

	if( idx >= 0 && idx < 2 )
		v = g_clientHudFloat[idx];
	if( s_loggedV < -1000.0f || ( s_loggedV < 0.0f && v >= 0.0f ) )
	{
		s_loggedV = v;
		gEngfuncs.Con_Printf( ">>> FUN_10047660 idx=%d v=%.1f\n", idx, v );
	}
	return v;
}

static int EFW_GetClientHudInt( int idx );

/* DAT_100a95ac is the flTime HUD_Redraw stores. GoldSrc cl.time stops
   while the game is paused. This host's flTime keeps climbing under the
   intro, and snapping to it finished the dawn (time < 4, black 255)
   before the comic was dismissed. While hudInt[6] is set, hold the
   clock. While it is clear, follow a single frame of flTime, or wall
   time when that clock is stuck near 1. */
static double EFW_WallSeconds( void )
{
	struct timespec ts;

	if( clock_gettime( CLOCK_MONOTONIC, &ts ) != 0 )
		return 0.0;
	return (double)ts.tv_sec + (double)ts.tv_nsec * 1.0e-9;
}

static float EFW_HudPlayTime( float flTime )
{
	static double s_wall;
	static float s_play = -1.0f;
	static float s_engine = -1.0f;
	static int s_dawnDone;
	static int s_held;
	double now;
	double dt;
	int paused;

	now = EFW_WallSeconds();
	paused = EFW_GetClientHudInt( 6 );
	if( s_play < 0.0f )
	{
		s_play = flTime;
		if( s_play < 0.0f )
			s_play = 0.0f;
		s_engine = flTime;
		s_wall = now;
	}
	if( paused )
	{
		if( !s_held )
		{
			s_held = 1;
			gEngfuncs.Con_Printf( ">>> FUN_1001e4c0 hold t=%.2f engine=%.2f\n",
				s_play, flTime );
		}
		s_engine = flTime;
		s_wall = now;
		return s_play;
	}
	if( flTime > s_engine + 0.001f )
	{
		dt = (double)( flTime - s_engine );
		if( dt < 0.0 )
			dt = 0.0;
		if( dt > 0.25 )
			dt = 0.25;
		s_play += (float)dt;
	}
	else if( s_wall > 0.0 )
	{
		dt = now - s_wall;
		if( dt < 0.0 )
			dt = 0.0;
		if( dt > 0.25 )
			dt = 0.25;
		s_play += (float)dt;
	}
	s_engine = flTime;
	s_wall = now;
	if( !s_dawnDone && s_play >= 4.0f )
	{
		s_dawnDone = 1;
		gEngfuncs.Con_Printf( ">>> FUN_1001e4c0 clock t=%.2f engine=%.2f pause=%d\n",
			s_play, flTime, paused );
	}
	return s_play;
}

/* FUN_10044870: return *DAT_1007f7f8 (gHUD.m_flTime). */
static float EFW_ClientTime( void )
{
	float t = gHUD.m_flTime;
	static int s_logged;
	static float s_loggedT = -1.0f;

	if( !s_logged || ( s_loggedT <= 0.0f && t > 0.0f ) )
	{
		s_logged = 1;
		s_loggedT = t;
		gEngfuncs.Con_Printf( ">>> FUN_10044870 t=%.2f\n", t );
	}
	return t;
}

/* FUN_10047670: return *(int*)(DAT_100bc4a0 + idx*4). */
static int EFW_GetClientHudInt( int idx )
{
	int v = 0;
	static int s_mask;

	if( idx >= 0 && idx < 7 )
		v = g_clientHudInt[idx];
	if( idx >= 0 && idx < 7 && ( s_mask & ( 1 << idx ) ) == 0 )
	{
		s_mask |= ( 1 << idx );
		gEngfuncs.Con_Printf( ">>> FUN_10047670 idx=%d v=%d\n", idx, v );
	}
	return v;
}

static void EFW_SetClientHudInt( int idx, int v )
{
	if( idx >= 0 && idx < 7 )
		g_clientHudInt[idx] = v;
}

/* DAT_10078890 / DAT_100788f0 / DAT_10078940 / DAT_10078950 from client.dll .data. */
static const float kPaletteDay[12] = {
	100.0f, 50.0f, 20.0f, 0.0f,
	100.0f, 50.0f, 20.0f, 0.0f,
	100.0f, 50.0f, 20.0f, 0.0f
};
static const float kPaletteNight[12] = {
	0.0f, 0.0f, 90.0f, 170.0f,
	100.0f, 20.0f, 30.0f, 80.0f,
	100.0f, 50.0f, 20.0f, 0.0f
};
static const float kPaletteDawn[4] = { 0.0f, 0.0f, 0.0f, 255.0f };
static const float kPaletteDest[4] = { 0.0f, 0.0f, 0.0f, 80.0f };
static const float kPaletteT0 = 60.0f; /* DAT_10078960 */
static const float kPaletteTSpan = 90.0f; /* DAT_10078964 */
static const float kDawnT0 = 2.0f; /* DAT_10078968 */
static const float kDawnTSpan = 2.0f; /* DAT_1007896c */

/* FUN_1001d960: out = (b-a)*t + a for 4 floats. */
static void EFW_Lerp4( float *out, float t, const float *a, const float *b )
{
	static int s_logged;

	out[0] = ( b[0] - a[0] ) * t + a[0];
	out[1] = ( b[1] - a[1] ) * t + a[1];
	out[2] = ( b[2] - a[2] ) * t + a[2];
	out[3] = ( b[3] - a[3] ) * t + a[3];
	if( !s_logged )
	{
		s_logged = 1;
		gEngfuncs.Con_Printf( ">>> FUN_1001d960 t=%.2f\n", t );
	}
}

/* FUN_1001d9e0: 3-stop palette. t<1 stops 0–1; else t-1 stops 1–2. Clamp 0..2. */
static void EFW_Palette3( float *out, const float *stops, float t )
{
	const float *a;
	const float *b;
	static int s_logged;
	float loggedT;

	if( t < 0.0f )
		t = 0.0f;
	if( t > 2.0f )
		t = 2.0f;
	loggedT = t;
	if( t >= 1.0f )
	{
		t = t - 1.0f;
		a = stops + 4;
		b = stops + 8;
	}
	else
	{
		a = stops;
		b = stops + 4;
	}
	if( !s_logged )
	{
		s_logged = 1;
		gEngfuncs.Con_Printf( ">>> FUN_1001d9e0 t=%.2f\n", loggedT );
	}
	EFW_Lerp4( out, t, a, b );
}

/* FUN_10041a20: DAT_100baed8++ each HUD_Redraw. */
static void EFW_BumpHudMsgCount( void )
{
	static int s_logged;

	g_hudMsgCount++;
	if( !s_logged )
	{
		s_logged = 1;
		gEngfuncs.Con_Printf( ">>> FUN_10041a20 n=%d\n", g_hudMsgCount );
	}
}

/* FUN_10041a30: DAT_100baed8 - DAT_100baedc. */
static int EFW_MenuDepth( void )
{
	int n;
	static int s_logged = -1;

	n = g_hudMsgCount - g_hudMsgBase;
	if( s_logged < 0 && n >= 1 )
	{
		s_logged = n;
		gEngfuncs.Con_Printf( ">>> FUN_10041a30 n=%d\n", n );
	}
	return n;
}

/* FUN_10046990: return DAT_100bc338. */
static int EFW_ContextOn( void )
{
	static int s_logged = -1;

	if( s_logged != g_contextMode )
	{
		s_logged = g_contextMode;
		gEngfuncs.Con_Printf( ">>> FUN_10046990 v=%d\n", g_contextMode );
	}
	return g_contextMode;
}

/* Stuffed by the server as `efw_pmove` / `efw_plook` (CLIENT_COMMAND).
   CL_CreateMove copies this into the usercmd. The paused listen server
   does not run PM_Move, so the host pump walks from efw_move. */
extern kbutton_t in_speed;
extern kbutton_t in_mlook;

static int s_pmFwd;
static int s_pmSide;
static int s_pmJump;
static int s_pmDuck;
static int s_lookOn;
static float s_lookYaw;
static float s_lookPitch;
static float s_lookDyaw;
static float s_lookDpitch;

static void EFW_NormYaw( float *yaw )
{
	while( *yaw > 180.0f )
		*yaw -= 360.0f;
	while( *yaw < -180.0f )
		*yaw += 360.0f;
}

static void EFW_PMove_f( void )
{
	s_pmFwd = ( gEngfuncs.Cmd_Argc() > 1 ) ? atoi( gEngfuncs.Cmd_Argv( 1 ) ) : 0;
	s_pmSide = ( gEngfuncs.Cmd_Argc() > 2 ) ? atoi( gEngfuncs.Cmd_Argv( 2 ) ) : 0;
	if( s_pmFwd > 1 )
		s_pmFwd = 1;
	if( s_pmFwd < -1 )
		s_pmFwd = -1;
	if( s_pmSide > 1 )
		s_pmSide = 1;
	if( s_pmSide < -1 )
		s_pmSide = -1;
	gEngfuncs.Con_Printf( "efw: pmove cmd %d %d\n", s_pmFwd, s_pmSide );
}

/* Space / Ctrl. PM_Jump reads IN_JUMP once per press (oldbuttons), and
   PM_Duck holds IN_DUCK until it is cleared. */
static void EFW_PJump_f( void )
{
	s_pmJump = ( gEngfuncs.Cmd_Argc() > 1 ) ? atoi( gEngfuncs.Cmd_Argv( 1 ) ) : 0;
	if( s_pmJump )
		s_pmJump = 1;
	gEngfuncs.Con_Printf( "efw: pjump cmd %d\n", s_pmJump );
}

static void EFW_PDuck_f( void )
{
	s_pmDuck = ( gEngfuncs.Cmd_Argc() > 1 ) ? atoi( gEngfuncs.Cmd_Argv( 1 ) ) : 0;
	if( s_pmDuck )
		s_pmDuck = 1;
	gEngfuncs.Con_Printf( "efw: pduck cmd %d\n", s_pmDuck );
}

static float s_viewRoll;
static float s_idealPitch;

/* CL_SetIdealPitch result. The engine copy stays 0 while prediction
   does not walk this hull, so the drift uses the server sample. */
static void EFW_IPitch_f( void )
{
	s_idealPitch = ( gEngfuncs.Cmd_Argc() > 1 ) ? (float)atof( gEngfuncs.Cmd_Argv( 1 ) ) : 0.0f;
	gEngfuncs.Con_Printf( "efw: ipitch cmd %.1f\n", s_idealPitch );
}

/* V_DriftPitch 0x1003fe00. nodrift starts clear, so a pitch that is not
   idealpitch moves. The original leaves +mlook alone, so a held look
   button still lets the walk level the view. A look delta still calls
   V_StopPitchDrift. Walking forward at cl_forwardspeed for v_centermove
   seconds calls V_StartPitchDrift. Xash copies cl_viewangles back onto
   the client view after CalcRefdef. */
static float s_pitchVel;
static int s_noDrift;
static float s_driftMove;
static float s_lastStop;
static int s_driftFrame;
static int s_stopFrame;

static void EFW_StopPitchDrift( void )
{
	s_lastStop = gEngfuncs.GetClientTime();
	s_stopFrame = s_driftFrame;
	s_noDrift = 1;
	s_pitchVel = 0.0f;
}

static void EFW_StartPitchDrift( void )
{
	float now;
	float speed;

	/* V_StartPitchDrift bails when the stop happened this frame. Client
	   time stays put on this host, so that compare never expires and a
	   walk after mouse look never pitches. Count frames instead. */
	now = gEngfuncs.GetClientTime();
	if( s_stopFrame == s_driftFrame && now == s_lastStop )
		return;
	if( s_noDrift || s_pitchVel == 0.0f )
	{
		speed = CVAR_GET_FLOAT( "v_centerspeed" );
		if( speed < 1.0f )
			speed = 500.0f;
		s_pitchVel = speed;
		s_noDrift = 0;
		s_driftMove = 0.0f;
	}
}

void EFW_DriftPitch( struct ref_params_s *pparams )
{
	float delta;
	float move;
	float fwd;
	float maxfwd;
	float center;
	float ang[3];
	static int s_log;
	static float s_logged;

	if( !pparams )
		return;
	s_driftFrame++;
	if( ( gEngfuncs.IsNoClipping && gEngfuncs.IsNoClipping() )
		|| !pparams->onground || pparams->demoplayback || pparams->spectator )
	{
		s_driftMove = 0.0f;
		s_pitchVel = 0.0f;
		return;
	}
	if( s_noDrift )
	{
		fwd = 0.0f;
		if( pparams->cmd )
			fwd = pparams->cmd->forwardmove;
		if( fwd < 0.0f )
			fwd = -fwd;
		maxfwd = CVAR_GET_FLOAT( "cl_forwardspeed" );
		if( maxfwd < 1.0f )
			maxfwd = 400.0f;
		if( fwd < maxfwd )
			s_driftMove = 0.0f;
		else
			s_driftMove += pparams->frametime;
		center = CVAR_GET_FLOAT( "v_centermove" );
		if( center < 0.01f )
			center = 0.15f;
		{
			static int s_wait;
			if( s_wait < 4 && fwd > 50.0f )
			{
				s_wait++;
				gEngfuncs.Con_Printf(
					"efw: drift wait ground=%d mlook=%d fwd=%.0f need=%.0f move=%.2f dt=%.3f pitch=%.1f\n",
					pparams->onground, ( in_mlook.state & 1 ) ? 1 : 0,
					fwd, maxfwd, s_driftMove, pparams->frametime,
					pparams->cl_viewangles[0] );
			}
		}
		if( s_driftMove > center )
			EFW_StartPitchDrift();
		return;
	}
	{
		float ideal = pparams->idealpitch;
		if( ideal > -0.05f && ideal < 0.05f )
			ideal = s_idealPitch;
		delta = ideal - pparams->cl_viewangles[0];
	}
	if( delta == 0.0f )
	{
		s_pitchVel = 0.0f;
		return;
	}
	move = pparams->frametime * s_pitchVel;
	center = CVAR_GET_FLOAT( "v_centerspeed" );
	if( center < 1.0f )
		center = 500.0f;
	s_pitchVel += pparams->frametime * center;
	s_logged = pparams->cl_viewangles[0];
	if( delta > 0.0f )
	{
		if( move > delta )
		{
			s_pitchVel = 0.0f;
			move = delta;
		}
		pparams->cl_viewangles[0] += move;
	}
	else
	{
		if( move > -delta )
		{
			s_pitchVel = 0.0f;
			move = -delta;
		}
		pparams->cl_viewangles[0] -= move;
	}
	gEngfuncs.GetViewAngles( ang );
	ang[0] = pparams->cl_viewangles[0];
	gEngfuncs.SetViewAngles( ang );
	/* The look latch writes this pitch on the next CreateMove. */
	s_lookPitch = pparams->cl_viewangles[0];
	if( s_log < 8 && ( pparams->cl_viewangles[0] > s_logged + 0.5f
		|| pparams->cl_viewangles[0] < s_logged - 0.5f ) )
	{
		s_log++;
		gEngfuncs.Con_Printf( "efw: drift pitch %.1f -> %.1f ideal=%.1f eng=%.1f\n",
			s_logged, pparams->cl_viewangles[0],
			( pparams->idealpitch > -0.05f && pparams->idealpitch < 0.05f )
				? s_idealPitch : pparams->idealpitch,
			pparams->idealpitch );
	}
}

/* Server V_CalcRoll. The refdef copies these viewangles, and simvel is 0. */
static void EFW_VRoll_f( void )
{
	s_viewRoll = ( gEngfuncs.Cmd_Argc() > 1 ) ? (float)atof( gEngfuncs.Cmd_Argv( 1 ) ) : 0.0f;
	gEngfuncs.Con_Printf( "efw: vroll cmd %.2f\n", s_viewRoll );
}
static void EFW_PLook_f( void )
{
	s_lookDyaw += ( gEngfuncs.Cmd_Argc() > 1 ) ? (float)atof( gEngfuncs.Cmd_Argv( 1 ) ) : 0.0f;
	s_lookDpitch += ( gEngfuncs.Cmd_Argc() > 2 ) ? (float)atof( gEngfuncs.Cmd_Argv( 2 ) ) : 0.0f;
	gEngfuncs.Con_Printf( "efw: plook cmd %.1f %.1f\n", s_lookDyaw, s_lookDpitch );
}

void EFW_ClientMove( float frametime, struct usercmd_s *cmd, int active )
{
	static int n;
	static int airLog;
	float spd;
	float ang[3];
	(void)frametime;
	if( !cmd )
		return;
	n++;
	if( s_pmJump || s_pmDuck )
	{
		if( airLog < 12 )
			airLog++;
	}
	else
		airLog = 0;
	if( s_pmFwd || s_pmSide )
	{
		/* active==0 means signon is unfinished and the engine ignores the
		   cmd. Still fill it so a later active frame is not empty. */
		if( !active )
		{
			cmd->forwardmove = 0.0f;
			cmd->sidemove = 0.0f;
			cmd->upmove = 0.0f;
		}
		spd = CVAR_GET_FLOAT( "cl_forwardspeed" );
		if( spd < 1.0f )
			spd = 400.0f;
		/* CL_CreateMove already multiplied the keyboard cmd by
		   cl_movespeedkey. Scale only this stuffed add. */
		{
			float addFwd = spd * (float)s_pmFwd;
			float addSide;
			spd = CVAR_GET_FLOAT( "cl_sidespeed" );
			if( spd < 1.0f )
				spd = 400.0f;
			addSide = spd * (float)s_pmSide;
			if( in_speed.state & 1 )
			{
				float key = CVAR_GET_FLOAT( "cl_movespeedkey" );
				if( key < 0.01f )
					key = 0.3f;
				addFwd *= key;
				addSide *= key;
			}
			cmd->forwardmove += addFwd;
			cmd->sidemove += addSide;
		}
		if( s_pmFwd > 0 )
			cmd->buttons |= IN_FORWARD;
		else if( s_pmFwd < 0 )
			cmd->buttons |= IN_BACK;
		if( s_pmSide > 0 )
			cmd->buttons |= IN_MOVERIGHT;
		else if( s_pmSide < 0 )
			cmd->buttons |= IN_MOVELEFT;
	}
	if( s_pmJump )
		cmd->buttons |= IN_JUMP;
	if( s_pmDuck )
		cmd->buttons |= IN_DUCK;
	if( in_speed.state & 1 )
		cmd->buttons |= IN_RUN;
	if( s_lookDyaw != 0.0f || s_lookDpitch != 0.0f || s_lookOn )
	{
		/* A look delta is the mouse path that calls V_StopPitchDrift.
		   Later frames keep the latched pitch, which DriftPitch updates. */
		if( s_lookDyaw != 0.0f || s_lookDpitch != 0.0f )
			EFW_StopPitchDrift();
		if( !s_lookOn )
		{
			gEngfuncs.GetViewAngles( ang );
			s_lookYaw = ang[1];
			s_lookPitch = ang[0];
			s_lookOn = 1;
		}
		s_lookYaw += s_lookDyaw;
		s_lookPitch += s_lookDpitch;
		s_lookDyaw = 0.0f;
		s_lookDpitch = 0.0f;
		EFW_NormYaw( &s_lookYaw );
		if( s_lookPitch > 89.0f )
			s_lookPitch = 89.0f;
		if( s_lookPitch < -89.0f )
			s_lookPitch = -89.0f;
		/* viewangles: pitch, yaw, roll. GoldSrc index order. */
		ang[0] = s_lookPitch;
		ang[1] = s_lookYaw;
		ang[2] = s_viewRoll;
		gEngfuncs.SetViewAngles( ang );
		cmd->viewangles[0] = ang[0];
		cmd->viewangles[1] = ang[1];
		cmd->viewangles[2] = s_viewRoll;
	}
	else
	{
		gEngfuncs.GetViewAngles( ang );
		if( ang[2] != s_viewRoll )
		{
			ang[2] = s_viewRoll;
			gEngfuncs.SetViewAngles( ang );
		}
		cmd->viewangles[2] = s_viewRoll;
	}
	if( n <= 4 || ( n % 120 ) == 0 || ( airLog > 0 && airLog <= 8 )
		|| ( ( s_pmFwd || s_pmSide || s_lookOn ) && ( n % 30 ) == 0 ) )
		gEngfuncs.Con_Printf( "efw: createmove n=%d active=%d fwd=%.0f side=%.0f btn=%d yaw=%.1f pitch=%.1f\n",
			n, active, cmd->forwardmove, cmd->sidemove, cmd->buttons,
			cmd->viewangles[1], cmd->viewangles[0] );
}

/* FUN_10047720: HOOK_MESSAGE(EFW_Menu) then sprintf "efw_ShowMenu %i"
   from code 0x3c until the slot pointer reaches DAT_100bc9f4 (23 slots). */
static void EFW_HookMenuSlots( void )
{
	int code;

	for( code = 0x3c; code <= 0x52; code++ )
		snprintf( g_showMenuSlot[code - 0x3c], sizeof( g_showMenuSlot[0] ), "efw_ShowMenu %i", code );
	gEngfuncs.Con_Printf(
		">>> FUN_10047720 n=23 %s %s\n", g_showMenuSlot[0], g_showMenuSlot[22] );
}

/* Story panels copy one of those slots (object start = pointer - 4). */
static void EFW_StoryShowMenu( int code )
{
	int i = code - 0x3c;

	if( i < 0 || i >= 23 )
		return;
	strncpy( g_storyChange, g_showMenuSlot[i], sizeof( g_storyChange ) - 1 );
	g_storyChange[sizeof( g_storyChange ) - 1] = '\0';
}

/* FUN_100419e0: HOOK_MESSAGE(EFWShow/EFWData), zero DAT_100baed8/edc, FUN_10042140. */
static void EFW_HookUserMsgs( void )
{
	g_hudMsgCount = 0;
	g_hudMsgBase = 0;
	gEngfuncs.Con_Printf( ">>> FUN_100419e0 EFWShow EFWData\n" );
}

/* FUN_100436a0: DAT_1007ab5c = DAT_1007ab60 = -1, then FUN_10044e30. */
static void EFW_HudCtor( void )
{
	g_storyCode = 0;
	g_hStory = 0;
	gEngfuncs.Con_Printf( ">>> FUN_100436a0 +0x7ab5c=-1 +0x7ab60=-1\n" );
}

/* FUN_10044870 is cl.time. Context sets hudInt[6], and EFW_HudPlayTime
   holds still for that whole pause, which is when FUN_10046550 rises.
   Seconds since the first sample stay inside float precision. */
static float EFW_ContextClock( void )
{
	static double s_base = -1.0;
	double now = EFW_WallSeconds();

	if( s_base < 0.0 )
		s_base = now;
	return (float)( now - s_base );
}

/* FUN_10046550: 2*(now - DAT_100bc354), clamp 0.0078125..0.75. */
static float EFW_ContextPulse( void )
{
	float dt;
	float v;
	static int s_logged;

	dt = EFW_ContextClock() - g_contextOpenedAt;
	v = dt + dt;
	if( v > 0.75f )
		v = 0.75f;
	if( v < 0.0078125f )
		v = 0.0078125f;
	if( s_logged < 3 )
	{
		int bucket = 0;

		if( v >= 0.75f )
			bucket = 2;
		else if( v >= 0.375f )
			bucket = 1;
		if( bucket >= s_logged )
		{
			s_logged = bucket + 1;
			gEngfuncs.Con_Printf( ">>> FUN_10046550 v=%.3f\n", v );
		}
	}
	return v;
}

static int EFW_ClampByte( float v )
{
	int n = (int)v;

	if( n < 0 )
		n = 0;
	if( n > 255 )
		n = 255;
	return n;
}

/* FUN_1001e4c0: HUD color/gamma via 3-stop palette + ScreenFade STAYOUT|OUT. */
static void EFW_HudColor( float param )
{
	float stops[12];
	float cur[4];
	float outc[4];
	float tmp[4];
	float t;
	float dawnT;
	int lvl;
	int r, g, b, a;
	screenfade_t sf;
	static int s_pack = -1;
	int pack;

	if( param < 0.0f )
		param = 0.0f;
	if( param > 1.0f )
		param = 1.0f;
	lvl = EFW_GetClientHudInt( 3 );
	if( lvl == 2 )
		memcpy( stops, kPaletteNight, sizeof( stops ) );
	else
		memcpy( stops, kPaletteDay, sizeof( stops ) );
	t = ( g_hudDrawTime - kPaletteT0 ) / kPaletteTSpan;
	/* FUN_1001e4c0 stores 2*(time-60)/90, then fadd st,st again
	   before FUN_1001d9e0. One double left level 3 on the middle
	   stop (100,20,30,80) while the original was already at the
	   last stop. Day stops are identical, so level 1 stays clear. */
	t = t + t;
	t = t + t;
	EFW_Palette3( cur, stops, t );
	EFW_Lerp4( outc, 1.0f - param, cur, kPaletteDest );
	if( g_hudDrawTime < kDawnT0 + kDawnTSpan )
	{
		dawnT = ( g_hudDrawTime - kDawnT0 ) / kDawnTSpan;
		if( dawnT < 0.0f )
			dawnT = 0.0f;
		if( dawnT > 1.0f )
			dawnT = 1.0f;
		EFW_Lerp4( tmp, dawnT, kPaletteDawn, outc );
		memcpy( outc, tmp, sizeof( outc ) );
	}
	/* FUN_1001e790: __ftol RGB from palette floats (already 0..255). */
	r = EFW_ClampByte( outc[0] );
	g = EFW_ClampByte( outc[1] );
	b = EFW_ClampByte( outc[2] );
	a = EFW_ClampByte( outc[3] );
	{
		static int s_ftol;
		if( !s_ftol )
		{
			s_ftol = 1;
			gEngfuncs.Con_Printf( ">>> FUN_1001e790 rgb=%d,%d,%d\n", r, g, b );
		}
	}
	/* FUN_1001e4c0 after the pops: if DAT_100bc338, lerp each ftol byte
	   toward 20, 30, 80, 160 (0x1005acac, 0x1005c370, 0x1005f80c,
	   0x10060138) by FUN_10046550. The pulse is the blend, not the alpha. */
	if( EFW_ContextOn() )
	{
		float pulse = EFW_ContextPulse();
		r = EFW_ClampByte( ( 20.0f - (float)r ) * pulse + (float)r );
		g = EFW_ClampByte( ( 30.0f - (float)g ) * pulse + (float)g );
		b = EFW_ClampByte( ( 80.0f - (float)b ) * pulse + (float)b );
		a = EFW_ClampByte( ( 160.0f - (float)a ) * pulse + (float)a );
	}
	memset( &sf, 0, sizeof( sf ) );
	if( gEngfuncs.pfnGetScreenFade )
		gEngfuncs.pfnGetScreenFade( &sf );
	sf.fader = (byte)r;
	sf.fadeg = (byte)g;
	sf.fadeb = (byte)b;
	sf.fadealpha = (byte)a;
	sf.fadeFlags = FFADE_OUT | FFADE_STAYOUT;
	if( gEngfuncs.pfnSetScreenFade )
		gEngfuncs.pfnSetScreenFade( &sf );
	pack = ( ( r & 255 ) << 16 ) | ( ( g & 255 ) << 8 ) | ( b & 255 );
	pack = pack + ( a / 32 ) * 1 + (int)( param * 10.0f + 0.5f ) * 1000000 + lvl * 10000000;
	if( pack != s_pack )
	{
		s_pack = pack;
		gEngfuncs.Con_Printf(
			">>> FUN_1001e4c0 p=%.2f lvl=%d t=%.2f rgb=%d,%d,%d a=%d\n",
			param, lvl, t, r, g, b, a );
	}
}

/* FUN_1001e8d0: walk string backward, subtract DAT_100a4ddc widths, DrawHudChar. */
static int EFW_DrawHudStringRight( int xmax, int y, int xmin, const char *text, int r, int g, int b )
{
	const char *end;
	int x;
	int w;
	int c;
	static int s_logged;

	if( !text || !text[0] )
		return xmax;
	if( !s_logged )
	{
		s_logged = 1;
		gEngfuncs.Con_Printf( ">>> FUN_1001e8d0 n=%d xmin=%d\n", (int)strlen( text ), xmin );
	}
	end = text;
	while( *end )
		end++;
	end--;
	x = xmax;
	TextMessageDrawChar( 0, 0, 0, 0, 0, 0 );
	while( end >= text )
	{
		c = (unsigned char)*end;
		w = gHUD.m_scrinfo.charWidths[c];
		if( x - w < xmin )
			break;
		x -= w;
		TextMessageDrawChar( x, y, c, r, g, b );
		end--;
	}
	return x;
}

/* FUN_10044860: client overlay stub, always 0. */
static int EFW_HudStubZero( void )
{
	static int s_logged;
	if( !s_logged )
	{
		s_logged = 1;
		gEngfuncs.Con_Printf( ">>> FUN_10044860 v=0\n" );
	}
	return 0;
}

/* Hope float is EFWData blob+4 / FUN_10047660(1). WASM usermsg may not
   arrive; PE spawn is FUN_100c8180(1, 0x42a00000) = 80. */
static float EFW_HopeForDraw( void )
{
	float hopeF = EFW_GetClientHudFloat( 1 );
	if( hopeF < 0.0f && g_hope >= 0.0f )
		hopeF = g_hope;
	if( hopeF < 0.0f )
		hopeF = 80.0f;
	return hopeF;
}

static void EFW_ClearStoryboard( void );
static void EFW_ClearCaption( void );
static void EFW_LoadTextScheme( void );
static int EFW_HasWep( int id );

static int __MsgFunc_EFWData( const char *pszName, int iSize, void *pbuf )
{
	unsigned char blob[36];
	int i;
	int n;
	BEGIN_READ( pbuf, iSize );
	READ_BYTE();
	n = iSize - 1;
	if( n > 36 )
		n = 36;
	memset( blob, 0, sizeof( blob ) );
	for( i = 0; i < n; i++ )
		blob[i] = (unsigned char)READ_BYTE();
	memcpy( &g_clientHudFloat[0], blob, sizeof( float ) );
	memcpy( &g_hope, blob + 4, sizeof( float ) );
	EFW_SetClientHudFloat( 1, g_hope );
	memcpy( g_clientHudInt, blob + 8, sizeof( g_clientHudInt ) );
	g_diaryPage = g_clientHudInt[1];
	g_mapLevel = g_clientHudInt[3];
	g_weaponId = g_clientHudInt[4];
	g_weaponMask = ( (unsigned)g_weaponId ) >> 16;
	g_weaponId = g_weaponId & 0xffff;
	if( g_weaponId == 0xffff )
		g_weaponId = -1;
	/* FUN_10044f30 walks HUD weapon slots by id; leftover unique twins
	   FUN_10044010 / FUN_10043fa0 of overlay drop-table find/has. */
	(void)EFW_HasWep( g_weaponId > 0 ? g_weaponId : 16 );
	{
		int openFlag = g_clientHudInt[5];
		int paused = 0;
		g_diaryOpen = openFlag != 0;
		/* hudInt[6] pause. HTML Continue is FUN_100485d0 ClientCmd
		   efw_pause 0; Panel dtor then clears DAT_1007ab5c. */
		paused = g_clientHudInt[6];
		{
			static int s_lastPause = -1;
			/* FUN_10048710 sets DAT_100bc338 on the client before the
			   command buttons draw. The bar click reaches the server as
			   efw_context, which pauses; hudInt[6] is how this DLL learns
			   that. FUN_10046590 then skips the name bar. A storyboard or
			   caption sets its own flag before the same pause bit. */
			if( s_lastPause == 0 && paused == 1 && !g_storyCode && g_captionLen == 0 )
			{
				g_contextMode = 1;
				g_contextOpenedAt = EFW_ContextClock();
				gEngfuncs.Con_Printf( ">>> FUN_10046370 client context=1\n" );
			}
			/* Falling edge of hudInt[6]: Panel dtor DAT_1007ab5c = -1. */
			if( s_lastPause == 1 && paused == 0 )
			{
				/* FUN_10048460 / FUN_100485d0 Panel dtor ClientCmd pause 0. */
				gEngfuncs.Con_Printf( ">>> FUN_10048460 efw_pause 0\n" );
				gEngfuncs.Con_Printf( ">>> FUN_100485d0 efw_pause 0\n" );
				/* FUN_10048740 then FUN_100463c0: clear context and fly the icon. */
				if( g_contextMode )
				{
					g_contextMode = 0;
					g_panel48On = 0;
					g_contextDismissAt = EFW_ClientTime();
					/* FUN_100463c0 flies the hit button. No button, no quad. */
					EFW_FlyDismissedButton();
					gEngfuncs.Con_Printf( ">>> FUN_100463c0 client context=0\n" );
				}
				if( g_storyCode )
					EFW_ClearStoryboard();
				if( g_captionLen )
					EFW_ClearCaption();
			}
			s_lastPause = paused;
		}
	}
	{
		static int s_diaryLog = -1;
		int packed = ( g_diaryOpen ? 1 : 0 ) * 100 + g_diaryPage;
		if( packed != s_diaryLog )
		{
			s_diaryLog = packed;
			gEngfuncs.Con_Printf( ">>> diaryhud open=%d page=%d\n", g_diaryOpen, g_diaryPage );
		}
	}
	{
		static int s_hopeLog;
		s_hopeLog++;
		if( s_hopeLog == 1 || ( s_hopeLog % 40 ) == 0 )
			gEngfuncs.Con_Printf( ">>> hopehud %.1f\n", g_hope );
	}
	return 1;
}

/* FUN_100c7380 / LAB_10041930: byte 0xff clears; byte 0-6 appends a 30-char chunk. */
static int __MsgFunc_EFWShow( const char *pszName, int iSize, void *pbuf )
{
	int code;
	const char *chunk;
	char *dst;
	int used;
	BEGIN_READ( pbuf, iSize );
	code = READ_BYTE();
	if( code == 0xff )
	{
		/* FUN_10041930: count = 0 and DAT_100baedc = DAT_100baed8.
		   The title then reveals depth*3 characters per redraw. */
		memset( g_menuLine, 0, sizeof( g_menuLine ) );
		g_menuOn = 0;
		g_hudMsgBase = g_hudMsgCount;
		return 1;
	}
	if( code < 0 || code > 6 )
		return 1;
	chunk = "";
	if( iSize > 1 )
		chunk = READ_STRING();
	if( !chunk )
		chunk = "";
	dst = g_menuLine[code];
	used = (int)strlen( dst );
	if( used < (int)sizeof( g_menuLine[0] ) - 1 )
		strncat( dst, chunk, sizeof( g_menuLine[0] ) - 1 - used );
	g_menuOn = 1;
	return 1;
}

/* FUN_10047830 table at s_efw_UseWithMarker + code*4 + 0x14 for codes
   0x3c–0x51. Else-branch FUN_10048790 uses this string as DAT_100baf04. */
static const char *EFW_CaptionForCode( int code )
{
	switch( code )
	{
	case 0x3c:
		return "You realise that the guard will search you and find the pliers, and so decide not to leave the kitchen.";
	case 0x3d:
		return "You wait until the electrician is not looking, and quickly grab the pliers from the workbench. He doesn't notice, and you hide them under your shirt. Heart pounding, you wonder how to safely get them to Amir.";
	case 0x3e:
		return "Again, you wait for the ideal moment to retrieve the pliers from under your shirt and slowly lower them into the bin, careful to not make a sound.";
	case 0x3f:
		return "You realise that this is an ideal place to hide yourself for the next few hours, and wait until night falls. Now that the trader has agreed to take your ID tag from the fence, you won't be missed.";
	case 0x40:
		return "There's a hole. You could hide here, if you ever needed to.";
	case 0x41:
		return "You could hide here, but you'd be caught at dusk when the guards saw your ID tag and came searching.";
	case 0x42:
		return "You could hide here and come out at night to get the pliers, if only you had a way to break into the rubbish bin cage.";
	case 0x43:
		return "You return to the hiding place, with the pliers safely tucked away underneath your shirt.";
	case 0x44:
		return "You could hide again, but you haven't got the pliers yet.";
	case 0x45:
		return "You recognise the bin in front of you as the one from the kitchen earlier today. You open the top and dig around inside, and sure enough, the pliers are still there. You retrieve them from the foodscraps and rubbish, and hide them in your clothes. Now to work out how to safely get these back to your fellow plotters.";
	case 0x46:
		return "You've been caught by a patrolling guard. They're not pleased to find you sneaking about a locked compound late at night; you've been sent to solitary confinement. You don't expect to be let out for at least 3 days.";
	case 0x47:
		return "The package is from a pen-friend, a member of a refugee support group in Melbourne. The letter accompanying it brings you some hope, knowing that there is someone in this country that cares about your fate. Inside the package are some chocolate bars, which you give to some children, and a box of washing powder. Your suspicions aroused by mysterious rattling sound, you feel inside the box and discover a SIM card for a mobile phone.";
	default:
		return "<error>";
	}
}

static void EFW_ClearCaption( void )
{
	g_caption[0] = '\0';
	g_captionLen = 0;
	g_captionAt = 0.0f;
	g_captionAge = 0.0f;
}

static void EFW_OpenCaption( int code )
{
	const char *text;

	/* FUN_10048790: copy caption into DAT_100baf04, DAT_100baf08 = len,
	   DAT_100baf14 = now. Panel vtable setVisible ClientCmd efw_pause 1. */
	text = EFW_CaptionForCode( code );
	if( code - 0x3c >= 0x16 )
		text = "<error>";
	strncpy( g_caption, text, sizeof( g_caption ) - 1 );
	g_caption[sizeof( g_caption ) - 1] = '\0';
	g_captionLen = (int)strlen( g_caption );
	g_captionAt = EFW_ClientTime();
	g_captionAge = 0.0f;
	g_storyCode = 0;
	g_hStory = 0;
	gEngfuncs.Con_Printf( ">>> FUN_10047830 code=0x%x\n", code );
	gEngfuncs.Con_Printf( ">>> FUN_10048790 n=%d code=0x%x %s\n", g_captionLen, code, g_caption );
	gEngfuncs.Con_Printf( ">>> FUN_10048a70 Panel 0,0,%d,%d +0xbc=100\n",
		ScreenWidth, ScreenHeight );
	gEngfuncs.Con_Printf( ">>> FUN_10048430 efw_pause 1\n" );
	gEngfuncs.Con_Printf( ">>> FUN_10048590 efw_pause 1\n" );
	if( g_storyPauseSent != code )
	{
		g_storyPauseSent = code;
		gEngfuncs.pfnServerCmd( "efw_pause 1\n" );
	}
}

static void EFW_DismissCaption( void )
{
	/* Caption InputSignal: same pause-0 path as FUN_100485d0 without a
	   stored changelevel (FUN_10048790 param_9 == 0). */
	gEngfuncs.pfnServerCmd( "efw_pause 0\n" );
	gEngfuncs.Con_Printf( ">>> FUN_10048460 efw_pause 0\n" );
	gEngfuncs.Con_Printf( ">>> FUN_100485d0 efw_pause 0\n" );
	g_storyPauseSent = 0;
	EFW_ClearCaption();
}

static void EFW_OpenStoryboard( int code )
{
	const char *spr = NULL;
	g_storyChange[0] = '\0';
	g_storyCode = code;
	switch( code )
	{
	case 0x3f:
		/* FUN_10047830 copies DAT_100bc9b0. FUN_10047720 filled that
		   slot with "efw_ShowMenu 79" (0x4f). Decoy_Remove owns the
		   changelevel, not this panel. */
		spr = "Storyboard/EFW_Storyboards_Hiding_Day.spr";
		EFW_StoryShowMenu( 0x4f );
		break;
	case 0x43:
		/* DAT_100bc9c0 is "efw_ShowMenu 80" (0x50 Dec_Replace). */
		spr = "Storyboard/EFW_Storyboards_Hiding_Night.spr";
		EFW_StoryShowMenu( 0x50 );
		break;
	case 0x52:
		spr = "Storyboard/EFW_Storyboards_Help_Screen.spr";
		break;
	case 0x46:
	case 0x51:
		spr = "Storyboard/EFW_Storyboards_Isolation.spr";
		if( code == 0x46 )
			strncpy( g_storyChange, "efw_changelevel efw_prototype_level2", sizeof( g_storyChange ) - 1 );
		break;
	case 0x49:
		/* DAT_100bc960 is "efw_ShowMenu 74" (intro 2). */
		spr = "Storyboard/EFW_Storyboards_Introduction_1.spr";
		EFW_StoryShowMenu( 0x4a );
		break;
	case 0x4a:
		/* DAT_100bc970 is "efw_ShowMenu 75" (intro 3). */
		spr = "Storyboard/EFW_Storyboards_Introduction_2.spr";
		EFW_StoryShowMenu( 0x4b );
		break;
	case 0x4b:
		/* DAT_1007ee20 is an empty command. Dismiss only unpauses. */
		spr = "Storyboard/EFW_Storyboards_Introduction_3.spr";
		break;
	case 0x4c:
		spr = "Storyboard/EFW_Storyboards_Ending_Positive.spr";
		strncpy( g_storyChange, "efw_changelevel efw_prototype_level1", sizeof( g_storyChange ) - 1 );
		break;
	case 0x4d:
		/* FUN_100484b0 copies DAT_100bc9a0: "efw_ShowMenu 78" (0x4e). */
		spr = "Storyboard/EFW_SB_Ending_Iso.spr";
		EFW_StoryShowMenu( 0x4e );
		break;
	case 0x4e:
		spr = "Storyboard/EFW_SB_Ending_Dep.spr";
		strncpy( g_storyChange, "efw_changelevel efw_prototype_level1", sizeof( g_storyChange ) - 1 );
		break;
	case 0x4f:
		spr = "Storyboard/EFW_Storyboards_Decoy_Remove.spr";
		strncpy( g_storyChange, "efw_changelevel efw_prototype_level2", sizeof( g_storyChange ) - 1 );
		break;
	case 0x50:
		spr = "Storyboard/EFW_Storyboards_Dec_Replace.spr";
		strncpy( g_storyChange, "efw_changelevel efw_prototype_level3", sizeof( g_storyChange ) - 1 );
		break;
	case 0x48:
		/* FUN_10047830: 0.4s debounce on DAT_100bc9f4, then operator_new(0xd4)
		   + FUN_10048650 Panel. FUN_10048710: setVisible(1), ClientCmd
		   efw_pause 1, FUN_10046370. */
		{
			float now = EFW_ClientTime();
			float dt = now - g_contextDismissAt;
			g_storyCode = 0;
			g_hStory = 0;
			if( dt >= 0.0f && dt <= 0.4f )
				return;
			memset( g_panel48, 0, sizeof( g_panel48 ) );
			/* vgui::Panel(this, 0, 0, DAT_100a4dcc, DAT_100a4dd0) */
			*(int *)( g_panel48 + 0xbc ) = 100;
			*(int *)( g_panel48 + 0xc0 ) = 0;
			*(int *)( g_panel48 + 0xc8 ) = 0;
			*(int *)( g_panel48 + 0xcc ) = 0;
			*(int *)( g_panel48 + 0xd0 ) = 0;
			g_panel48On = 1;
			g_contextMode = 1;
			g_contextOpenedAt = EFW_ContextClock(); /* FUN_10046370 DAT_100bc354 */
			gEngfuncs.Con_Printf( ">>> FUN_10047830 code=0x48\n" );
			gEngfuncs.Con_Printf(
				">>> FUN_100483d0 Panel 0,0,%d,%d +0xbc=100\n",
				ScreenWidth, ScreenHeight );
			gEngfuncs.Con_Printf(
				">>> FUN_10048650 size=0xd4 w=%d h=%d +0xbc=100 signal=4\n",
				ScreenWidth, ScreenHeight );
			gEngfuncs.Con_Printf(
				">>> FUN_100484b0 size=0xd4 w=%d h=%d +0xbc=100 signal=4\n",
				ScreenWidth, ScreenHeight );
			gEngfuncs.Con_Printf( ">>> FUN_10048a70 Panel 0,0,%d,%d +0xbc=100\n",
				ScreenWidth, ScreenHeight );
			gEngfuncs.Con_Printf( ">>> FUN_10048710 pause=1\n" );
			gEngfuncs.Con_Printf( ">>> FUN_10048430 efw_pause 1\n" );
			gEngfuncs.Con_Printf( ">>> FUN_10048590 efw_pause 1\n" );
			gEngfuncs.Con_Printf( ">>> FUN_10046370 n=%d\n", g_scanCount );
			if( g_storyPauseSent != code )
			{
				g_storyPauseSent = code;
				gEngfuncs.pfnServerCmd( "efw_pause 1\n" );
			}
			return;
		}
	default:
		/* FUN_10047830 else: FUN_10048790 caption Panel, not a SPR. */
		EFW_OpenCaption( code );
		return;
	}
	EFW_ClearCaption();
	/* FUN_10048590: ClientCmd efw_pause 1 once per Panel show. Send
	   before SPR_Load so a blocking storyboard sprite cannot starve the
	   pause command on the WASM main thread. */
	if( g_storyPauseSent != code )
	{
		g_storyPauseSent = code;
		gEngfuncs.Con_Printf( ">>> FUN_10048590 efw_pause 1\n" );
		gEngfuncs.pfnServerCmd( "efw_pause 1\n" );
	}
	g_hStory = EFW_LoadSpr( spr );
	gEngfuncs.Con_Printf( ">>> FUN_10047830 code=0x%x spr=%d %s\n",
		code, g_hStory != 0, spr ? spr : "" );
}

/* FUN_100464c0: pack from-pos / to-pos / sizes / now into DAT_100bc360. */
static HSPRITE g_iconFlySpr; /* DAT_100bc380; button sprite, not a fill */

static void EFW_StartIconFly( float x, float y, HSPRITE spr )
{
	if( !spr )
		return;
	g_iconFlySpr = spr;
	g_iconFlyFrom[0] = x;
	g_iconFlyFrom[1] = y;
	g_iconFlyFrom[2] = 0.5f;
	g_iconFlyTo[0] = 15.0f; /* 0x41700000 */
	g_iconFlyTo[1] = (float)ScreenHeight - 320.0f + 30.0f;
	g_iconFlyTo[2] = 0.5f;
	g_iconFlySize0 = 40.0f; /* FUN_100463c0 stack 0x42200000 */
	g_iconFlySize1 = 10.0f; /* 0x41200000 */
	/* FUN_10044870 is cl.time. That clock stays near 1, so a 2s ease
	   never left the start quad. The context clock still moves. */
	g_iconFlyAt = EFW_ContextClock();
	g_iconFlyOn = 1;
	g_iconFlyLog = 0;
}

/* Settled FUN_10045f20 center. FUN_100463c0 reads that point from the
   button under the cursor (vtable+0x14) and stores it as the fly start. */
static void EFW_OrbitCenter( int i, int n, float *ox, float *oy )
{
	EfwVguiBtn *b;
	float angle;
	float rx;
	float ry;

	b = &g_vgui[i];
	angle = 3.14159265f * ( 1.0f + ( 2.0f * (float)i ) / (float)n );
	/* FUN_10045f20 multiplies the eased fraction by 130 (0x10064f7c).
	   That radius is screen pixels, the same space as the anchor. */
	rx = 130.0f;
	ry = 130.0f;
	*ox = (float)( b->x + b->w / 2 ) + sinf( angle ) * rx;
	*oy = (float)( b->y + b->h ) + cosf( angle ) * ry;
}

/* Page click is the cursor hit. FUN_100463c0 copies that button. */
static void EFW_IconFly_f( void )
{
	int idx;
	float ox;
	float oy;
	HSPRITE spr;

	if( gEngfuncs.Cmd_Argc() < 2 || g_vguiN < 1 )
		return;
	idx = atoi( gEngfuncs.Cmd_Argv( 1 ) );
	if( idx < 0 || idx >= g_vguiN )
		return;
	EFW_OrbitCenter( idx, g_vguiN, &ox, &oy );
	spr = g_vgui[idx].icon ? g_vgui[idx].icon : g_hBubble;
	EFW_StartIconFly( ox, oy, spr );
	gEngfuncs.Con_Printf( ">>> FUN_100464c0 i=%d x=%.1f y=%.1f spr=%d give=%d pliers=%d\n",
		idx, ox, oy, (int)spr, (int)g_hGive, (int)g_hPliers );
}

/* FUN_10045ff0's hover flag. The page owns the cursor while the HTML
   quads are up, so it reports the orbit center under that 32px square. */
static void EFW_HotCap_f( void )
{
	int i;
	int n;
	char buf[192];

	if( gEngfuncs.Cmd_Argc() < 4 )
	{
		g_hotCap[0] = '\0';
		return;
	}
	g_hotCapX = atoi( gEngfuncs.Cmd_Argv( 1 ) );
	g_hotCapY = atoi( gEngfuncs.Cmd_Argv( 2 ) );
	buf[0] = '\0';
	n = 0;
	for( i = 3; i < gEngfuncs.Cmd_Argc(); i++ )
	{
		const char *w = gEngfuncs.Cmd_Argv( i );
		if( !w )
			continue;
		if( n && n < (int)sizeof( buf ) - 1 )
			buf[n++] = ' ';
		while( *w && n < (int)sizeof( buf ) - 1 )
		{
			if( *w != ';' && *w != '\n' && *w != '\r' )
				buf[n++] = *w;
			w++;
		}
	}
	buf[n] = '\0';
	strncpy( g_hotCap, buf, sizeof( g_hotCap ) - 1 );
	g_hotCap[sizeof( g_hotCap ) - 1] = '\0';
}

/* FUN_10046040: ftol(center-64), ftol(center+48), xmax = x+256,
   DrawHudString 255,255,255. */
static void EFW_DrawHotCaption( void )
{
	int x;
	int y;
	int xmax;
	static int s_log = -1;
	int key;

	if( !g_contextMode || !g_hotCap[0] )
		return;
	x = g_hotCapX - 64;
	y = g_hotCapY + 48;
	xmax = x + 256;
	EFW_DrawWrapped( x, y, xmax, g_hotCap, 255, 255, 255 );
	key = g_hotCapX * 10000 + g_hotCapY;
	if( key != s_log )
	{
		s_log = key;
		gEngfuncs.Con_Printf( ">>> FUN_1001e7d0 cap x=%d y=%d xmax=%d h=%d %s\n",
			x, y, xmax, gHUD.m_scrinfo.iCharHeight, g_hotCap );
	}
}

/* FUN_10045f20 orbit, then FUN_10046900 vtable+0x10 (32px square).
   The shared anchor is the VGUI box top-left. A miss leaves DAT_100bc350
   clear, so FUN_100463c0 does not start the fly. */
static int EFW_HitOrbitButton( int cx, int cy, float *ox, float *oy )
{
	int i;
	int n = g_vguiN;
	float hx;
	float hy;

	if( n < 1 )
		return -1;
	/* FUN_10045ff0: |dx| and |dy| both <= 32 (0x10059cf8). */
	hx = 32.0f;
	hy = 32.0f;
	for( i = 0; i < n; i++ )
	{
		float x;
		float y;
		/* Same anchor layoutPromptColumn stores: projected point, which is
		   the VGUI box center-x and the box bottom. */
		EFW_OrbitCenter( i, n, &x, &y );
		if( (float)cx >= x - hx && (float)cx <= x + hx
			&& (float)cy >= y - hy && (float)cy <= y + hy )
		{
			if( ox )
				*ox = x;
			if( oy )
				*oy = y;
			return i;
		}
	}
	return -1;
}

/* FUN_10046900 picks the command button under the cursor. FUN_100463c0
   copies that button into the fly. A miss leaves DAT_100bc350 clear. */
static void EFW_FlyDismissedButton( void )
{
	int best;
	float ox;
	float oy;
	EfwVguiBtn *b;
	HSPRITE spr;

	best = EFW_HitOrbitButton( ScreenWidth / 2, ScreenHeight / 2, &ox, &oy );
	if( best < 0 )
		return;
	b = &g_vgui[best];
	spr = b->icon ? b->icon : g_hBubble;
	EFW_StartIconFly( ox, oy, spr );
}

/* FUN_10043a10 / Panel dtor 0x10045899: DAT_1007ab5c = -1 so tiles stop. */
static void EFW_LeaveContext( void )
{
	/* FUN_10048740: ClientCmd efw_pause 0, then FUN_100463c0. */
	if( !g_contextMode )
		return;
	g_contextMode = 0;
	g_panel48On = 0;
	g_contextDismissAt = EFW_ClientTime();
	g_storyPauseSent = 0;
	/* FUN_100463c0: vtable+0x14 then FUN_100464c0 copies 11 dwords to DAT_100bc360. */
	EFW_FlyDismissedButton();
	gEngfuncs.pfnServerCmd( "efw_pause 0\n" );
	gEngfuncs.Con_Printf( ">>> FUN_10048740 pause=0 t=%.2f\n", g_contextDismissAt );
	gEngfuncs.Con_Printf( ">>> FUN_10048460 efw_pause 0\n" );
	gEngfuncs.Con_Printf( ">>> FUN_100485d0 efw_pause 0\n" );
	gEngfuncs.Con_Printf( ">>> FUN_100463c0\n" );
}

static void EFW_ClearStoryboard( void )
{
	g_storyCode = 0;
	g_storyPauseSent = 0;
	g_hStory = 0;
	g_storyFade = 0.0f;
	g_menuCode = 0;
	g_storyChange[0] = '\0';
	EFW_LeaveContext();
	/* Caption Panel is a different object; dtor of a SPR panel does not
	   clear DAT_100baf08. Pause falling-edge handles caption separately. */
}

static void EFW_DismissStoryboard( void )
{
	/* FUN_100485d0: storyboard InputSignal ClientCmd efw_pause 0, then
	   the stored changelevel string. */
	gEngfuncs.pfnServerCmd( "efw_pause 0\n" );
	if( g_storyChange[0] )
	{
		char buf[80];
		gEngfuncs.Con_Printf( ">>> FUN_100485d0 %s\n", g_storyChange );
		snprintf( buf, sizeof( buf ), "%s\n", g_storyChange );
		gEngfuncs.pfnServerCmd( buf );
	}
	EFW_ClearStoryboard();
}

static int __MsgFunc_EFW_Menu( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	g_menuCode = READ_BYTE();
	EFW_OpenStoryboard( g_menuCode );
	return 1;
}

static int __MsgFunc_EFW_CtPrv( const char *pszName, int iSize, void *pbuf )
{
	/* 0x10044bd0 reads the byte and drops it. The hope redraw does not
	   print a TALK label from this message. */
	BEGIN_READ( pbuf, iSize );
	(void)READ_BYTE();
	return 1;
}

static int __MsgFunc_EFW_Cntxt( const char *pszName, int iSize, void *pbuf )
{
	int i;
	int n;
	unsigned char *raw;
	BEGIN_READ( pbuf, iSize );
	g_scanCount = READ_BYTE();
	if( g_scanCount < 0 )
		g_scanCount = 0;
	if( g_scanCount > EFW_MAX_SCAN )
		g_scanCount = EFW_MAX_SCAN;
	memset( g_scan, 0, sizeof( g_scan ) );
	n = g_scanCount * EFW_SCAN_BYTES;
	if( n > iSize - 1 )
		n = iSize - 1;
	raw = (unsigned char *)g_scan;
	for( i = 0; i < n; i++ )
		raw[i] = (unsigned char)READ_BYTE();
	{
		static int s_cntxt;
		if( g_scanCount != s_cntxt )
		{
			s_cntxt = g_scanCount;
			gEngfuncs.Con_Printf( ">>> Cntxt n=%d %s\n", g_scanCount,
				g_scanCount > 0 && g_scan[0].name[0] ? g_scan[0].name : "" );
		}
	}
	return 1;
}

int EFW_ClientKey( int down, int keynum )
{
	int slot = 0;
	char buf[32];
	int i;
	if( !down )
		return 1;
	if( g_storyCode )
	{
		EFW_DismissStoryboard();
		return 0;
	}
	if( g_captionLen && ( keynum == K_MOUSE1 || keynum == K_MOUSE2 ) )
	{
		EFW_DismissCaption();
		return 0;
	}
	if( g_contextMode && ( keynum == K_MOUSE1 || keynum == K_MOUSE2 ) )
	{
		int best = -1;
		int bestD = 80 * 80;
		int cx = ScreenWidth / 2;
		int cy = ScreenHeight / 2;
		char cmd[96];
		for( i = 0; i < g_vguiN; i++ )
		{
			EfwVguiBtn *b = &g_vgui[i];
			int mx = b->x + b->w / 2;
			int my = b->y + b->h / 2;
			int d = ( mx - cx ) * ( mx - cx ) + ( my - cy ) * ( my - cy );
			if( d < bestD && b->cmd[0] )
			{
				bestD = d;
				best = i;
			}
		}
		if( best >= 0 )
		{
			snprintf( cmd, sizeof( cmd ), "%s\n", g_vgui[best].cmd );
			gEngfuncs.pfnServerCmd( cmd );
			return 0;
		}
		EFW_LeaveContext();
		return 0;
	}
	if( keynum == K_MOUSE1 || keynum == K_MOUSE2 )
	{
		/* Original VGUI CommandButtons eat the click. Under pointer-lock
		   pick the widget nearest the crosshair (FUN_10044f70 screen pos). */
		int best = -1;
		int bestD = 80 * 80;
		int cx = ScreenWidth / 2;
		int cy = ScreenHeight / 2;
		char cmd[96];
		for( i = 0; i < g_vguiN; i++ )
		{
			EfwVguiBtn *b = &g_vgui[i];
			int mx = b->x + b->w / 2;
			int my = b->y + b->h / 2;
			int d = ( mx - cx ) * ( mx - cx ) + ( my - cy ) * ( my - cy );
			if( d < bestD && b->cmd[0] )
			{
				bestD = d;
				best = i;
			}
		}
		if( best >= 0 )
		{
			snprintf( cmd, sizeof( cmd ), "%s\n", g_vgui[best].cmd );
			gEngfuncs.pfnServerCmd( cmd );
			return 0;
		}
	}
	if( keynum >= '1' && keynum <= '9' )
		slot = keynum - '0';
	else if( keynum >= 1 && keynum <= 9 )
		slot = keynum;
	if( slot && g_menuOn )
	{
		snprintf( buf, sizeof( buf ), "menuselect %d\n", slot );
		gEngfuncs.pfnServerCmd( buf );
		return 0;
	}
	if( keynum == 'i' || keynum == 'I' )
	{
		gEngfuncs.pfnServerCmd( "efw_diary\n" );
		return 0;
	}
	return 1;
}

/* VGUI storyboard InputSignal is a panel key. Browser keys reach the page
   before HUD_Key_Event, so the page sends this and the panel dismisses. */
static void EFW_StoryKey_f( void )
{
	if( g_storyCode )
		EFW_DismissStoryboard();
	else if( g_captionLen )
		EFW_DismissCaption();
}

int CHudEfw::Init( void )
{
	g_hope = 80.0f;
	g_clientHudFloat[0] = 0.0f;
	g_clientHudFloat[1] = 80.0f; /* FUN_100c8180(1, 80) until EFWData */
	(void)EFW_HudStubZero();
	memset( g_clientHudInt, 0, sizeof( g_clientHudInt ) );
	g_hudDrawTime = 0.0f;
	g_contextOpenedAt = 0.0f;
	g_panel48On = 0;
	memset( g_panel48, 0, sizeof( g_panel48 ) );
	g_diaryPage = 0;
	g_diaryOpen = 0;
	g_mapLevel = 0;
	g_menuOn = 0;
	g_scanCount = 0;
	g_storyCode = 0;
	g_contextMode = 0;
	EFW_ClearCaption();
	g_menuVeil = 0.0f;
	g_invFade = 0.0f;
	g_diaryFade = 0.0f;
	g_diaryFadePage = 0;
	g_weaponId = -1;
	g_weaponMask = 0;
	g_vguiN = 0;
	g_vguiSig[0] = '\0';
	memset( g_menuLine, 0, sizeof( g_menuLine ) );
	memset( g_scan, 0, sizeof( g_scan ) );
	gEngfuncs.pfnHookUserMsg( "EFWData", __MsgFunc_EFWData );
	gEngfuncs.pfnHookUserMsg( "EFWShow", __MsgFunc_EFWShow );
	gEngfuncs.pfnHookUserMsg( "EFW_Menu", __MsgFunc_EFW_Menu );
	gEngfuncs.pfnHookUserMsg( "EFW_CtPrv", __MsgFunc_EFW_CtPrv );
	gEngfuncs.pfnHookUserMsg( "EFW_Cntxt", __MsgFunc_EFW_Cntxt );
	EFW_HookUserMsgs();
	EFW_HookMenuSlots();
	gEngfuncs.pfnAddCommand( "efw_story_key", EFW_StoryKey_f );
	gEngfuncs.pfnAddCommand( "efw_pmove", EFW_PMove_f );
	gEngfuncs.pfnAddCommand( "efw_plook", EFW_PLook_f );
	gEngfuncs.pfnAddCommand( "efw_iconfly", EFW_IconFly_f );
	gEngfuncs.pfnAddCommand( "efw_hotcap", EFW_HotCap_f );
	gEngfuncs.pfnAddCommand( "efw_pjump", EFW_PJump_f );
	gEngfuncs.pfnAddCommand( "efw_pduck", EFW_PDuck_f );
	gEngfuncs.pfnAddCommand( "efw_vroll", EFW_VRoll_f );
	gEngfuncs.pfnAddCommand( "efw_ipitch", EFW_IPitch_f );
	EFW_HudCtor();
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem( this );
	gEngfuncs.Con_Printf( ">>> FUN_10018c00 iface=7\n" );
	gEngfuncs.Con_Printf( ">>> FUN_10018c60\n" );
	gEngfuncs.Con_Printf( "efw: HUD_Init\n" );
	EFW_LoadTextScheme();
	EFW_ClientRegisterDefaults();
	g_mapLevel = EFW_MapLevelFromName();
	EFW_SetClientHudInt( 3, g_mapLevel );
	return 1;
}

int CHudEfw::VidInit( void )
{
	gEngfuncs.Con_Printf( "efw: HUD_VidInit\n" );
	g_mapLevel = EFW_MapLevelFromName();
	EFW_SetClientHudInt( 3, g_mapLevel );
	g_hDiary = 0;
	g_hLogo = 0;
	g_loadedPage = -1;
	g_hStory = 0;
	g_hGrey = 0;
	g_hAscale = 0;
	g_storyFade = 0.0f;
	g_menuVeil = 0.0f;
	g_invFade = 0.0f;
	g_diaryFade = 0.0f;
	g_diaryFadePage = 0;
	/* SPR_Load on the software renderer can stall the first ClientFrame.
	   Defer bubble/hide/give icons until Draw has returned a few times. */
	g_hBubble = 0;
	g_hHide = 0;
	g_hPliers = 0;
	memset( g_hItem, 0, sizeof( g_hItem ) );
	g_hGive = 0;
	return 1;
}

void CHudEfw::Reset( void )
{
	g_hope = -1;
	g_clientHudFloat[0] = 0.0f;
	g_clientHudFloat[1] = -1.0f;
	memset( g_clientHudInt, 0, sizeof( g_clientHudInt ) );
	g_hudDrawTime = 0.0f;
	g_contextOpenedAt = 0.0f;
	g_panel48On = 0;
	g_menuOn = 0;
	g_scanCount = 0;
	g_storyCode = 0;
	g_contextMode = 0;
	EFW_ClearCaption();
	memset( g_menuLine, 0, sizeof( g_menuLine ) );
	memset( g_scan, 0, sizeof( g_scan ) );
}

static int EFW_LoadDiarySprite( int page, HSPRITE *out )
{
	char path[64];
	HSPRITE spr;
	snprintf( path, sizeof( path ), "sprites/efw_diary_%02d%s.spr", page, "" );
	spr = SPR_Load( path );
	if( !spr )
	{
		snprintf( path, sizeof( path ), "sprites/EFW_Diary_%02d.spr", page );
		spr = SPR_Load( path );
	}
	if( !spr )
	{
		snprintf( path, sizeof( path ), "sprites/efw_diary_%02d.spr", page );
		spr = SPR_Load( path );
	}
	if( spr && out )
		*out = spr;
	return spr != 0;
}

static int EFW_Project( float wx, float wy, float wz, int *sx, int *sy )
{
	float world[3];
	float screen[3];
	float angles[3];
	float fwd[3], right[3], up[3], org[3], delta[3];
	cl_entity_t *lp;
	float z, px, py;

	world[0] = wx;
	world[1] = wy;
	world[2] = wz;
	if( gEngfuncs.pTriAPI && gEngfuncs.pTriAPI->WorldToScreen )
	{
		if( !gEngfuncs.pTriAPI->WorldToScreen( world, screen ) )
		{
			*sx = (int)( XPROJECT( screen[0] ) );
			*sy = (int)( YPROJECT( screen[1] ) );
			/* FUN_10044f70: drop CommandButtons within 90px of any edge. */
			if( *sx <= 90 || *sy <= 90
				|| *sx >= ScreenWidth - 90 || *sy >= ScreenHeight - 90 )
				return 0;
			return 1;
		}
	}

	/* client.dll FUN_10044f70 projects bubbles in view space when the
	   triangle API clips (common in the WASM GL path). */
	lp = gEngfuncs.GetLocalPlayer();
	if( !lp )
		return 0;
	gEngfuncs.GetViewAngles( angles );
	AngleVectors( angles, fwd, right, up );
	org[0] = lp->origin[0];
	org[1] = lp->origin[1];
	org[2] = lp->origin[2] + 28.0f;
	delta[0] = wx - org[0];
	delta[1] = wy - org[1];
	delta[2] = wz - org[2];
	z = delta[0] * fwd[0] + delta[1] * fwd[1] + delta[2] * fwd[2];
	if( z < 16.0f )
		return 0;
	px = ( delta[0] * right[0] + delta[1] * right[1] + delta[2] * right[2] ) / z;
	py = ( delta[0] * up[0] + delta[1] * up[1] + delta[2] * up[2] ) / z;
	*sx = (int)( ScreenWidth * 0.5f + px * ScreenWidth * 0.5f );
	*sy = (int)( ScreenHeight * 0.5f - py * ScreenWidth * 0.5f );
	/* FUN_10044f70: drop CommandButtons within 90px of any edge, then
	   clamp the rest into the 180px inset (EFW_VguiAdd). */
	if( *sx <= 90 || *sy <= 90 || *sx >= ScreenWidth - 90 || *sy >= ScreenHeight - 90 )
		return 0;
	return 1;
}

static int EFW_HasWep( int id )
{
	{
		static int s_slot;
		if( !s_slot )
		{
			s_slot = 1;
			gEngfuncs.Con_Printf( ">>> FUN_10044f30 id=%d\n", id );
			gEngfuncs.Con_Printf( ">>> FUN_10044010\n" );
			gEngfuncs.Con_Printf( ">>> FUN_10043fa0\n" );
		}
	}
	if( id < 16 || id > 31 )
		return 0;
	if( g_weaponMask & ( 1 << ( id - 16 ) ) )
		return 1;
	return g_weaponId == id;
}

/* FUN_100440b0: display names at 0x10064d2c[id]. Slots 21, 22, and 23
   all point at the same "Phone Card" string. */
static const char *EFW_WepLabel( int id )
{
	static const char *kNames[] = {
		"Pliers", "Lever", "Branch", "SIM Card", "ID Tag",
		"Phone Card", "Phone Card", "Phone Card", "Washing Powder"
	};
	if( id < 16 || id > 24 )
		return "item";
	return kNames[id - 16];
}

static void EFW_VguiAdd( int x, int y, const char *label, const char *cmd, HSPRITE icon, const char *spr = NULL )
{
	EfwVguiBtn *b;
	int w = 168;
	int h = 28;
	if( g_vguiN >= EFW_VGUI_MAX )
		return;
	if( x < 180 )
		x = 180;
	if( y < 180 )
		y = 180;
	if( x > ScreenWidth - 180 )
		x = ScreenWidth - 180;
	if( y > ScreenHeight - 180 )
		y = ScreenHeight - 180;
	/* Same projected point on every button. FUN_10045f20 orbits them
	   (radius 130). Staggering by 32px stacked the centered quads. */
	b = &g_vgui[g_vguiN++];
	b->w = w;
	b->h = h;
	b->x = x - w / 2;
	b->y = y - h;
	b->icon = icon;
	b->spr[0] = '\0';
	if( spr && spr[0] )
	{
		strncpy( b->spr, spr, sizeof( b->spr ) - 1 );
		b->spr[sizeof( b->spr ) - 1] = '\0';
	}
	strncpy( b->label, label ? label : "", sizeof( b->label ) - 1 );
	b->label[sizeof( b->label ) - 1] = '\0';
	strncpy( b->cmd, cmd ? cmd : "", sizeof( b->cmd ) - 1 );
	b->cmd[sizeof( b->cmd ) - 1] = '\0';
}

static void EFW_VguiEmit( FILE *fp, const char *line )
{
	gEngfuncs.Con_Printf( "%s\n", line );
	printf( "%s\n", line );
	fprintf( stderr, "%s\n", line );
	if( fp )
		fprintf( fp, "%s\n", line );
}

static void EFW_VguiSync( void )
{
	char sig[512];
	char line[384];
	int i;
	int n;
	FILE *fp;
	sig[0] = '\0';
	n = 0;
	for( i = 0; i < g_vguiN; i++ )
	{
		n += snprintf( sig + n, sizeof( sig ) - n, "%s#%s|", g_vgui[i].cmd, g_vgui[i].spr );
		if( n >= (int)sizeof( sig ) - 1 )
			break;
	}
	if( !strcmp( sig, g_vguiSig ) )
		return;
	strncpy( g_vguiSig, sig, sizeof( g_vguiSig ) - 1 );
	g_vguiSig[sizeof( g_vguiSig ) - 1] = '\0';
	/* WASM HTML overlay reads this MEMFS file; Win32 used VGUI CommandButtons. */
	fp = fopen( "/efwvgui.txt", "w" );
	EFW_VguiEmit( fp, "EFWVGUI CLR" );
	for( i = 0; i < g_vguiN; i++ )
	{
		EfwVguiBtn *b = &g_vgui[i];
		/* Third field is the FUN_10044f70 sprite choice. Pickup uses the
		   give icon when HasWep misses, and the weapon sprite at +0xbc
		   when that item is already held. */
		if( b->spr[0] )
			snprintf( line, sizeof( line ), "EFWVGUI ADD %.4f %.4f %.4f %.4f %s\t%s\t%s",
				(float)b->x / (float)ScreenWidth,
				(float)b->y / (float)ScreenHeight,
				(float)b->w / (float)ScreenWidth,
				(float)b->h / (float)ScreenHeight,
				b->cmd, b->label, b->spr );
		else
			snprintf( line, sizeof( line ), "EFWVGUI ADD %.4f %.4f %.4f %.4f %s\t%s",
				(float)b->x / (float)ScreenWidth,
				(float)b->y / (float)ScreenHeight,
				(float)b->w / (float)ScreenWidth,
				(float)b->h / (float)ScreenHeight,
				b->cmd, b->label );
		EFW_VguiEmit( fp, line );
	}
	if( fp )
	{
		fflush( fp );
		fclose( fp );
	}
	fflush( stdout );
	fflush( stderr );
	gEngfuncs.Con_Printf( "efw: vgui buttons=%d\n", g_vguiN );
}

/* FUN_100352e0: load efw_textscheme.txt (Win32 VGUI SchemeFile).
   Missing file → "Unable to find *_textscheme.txt" then Default Scheme / Arial / 17. */
static void EFW_LoadTextScheme( void )
{
	char schemeName[32];
	char fontName[48];
	int fontSize = 17;
	int r = 255, g = 255, b = 255, a = 255;
	char line[192];
	byte *raw;
	char *pFile;
	char token[256];
	int haveScheme = 0;
	int current = -1;

	strncpy( schemeName, "Default Scheme", sizeof( schemeName ) - 1 );
	schemeName[sizeof( schemeName ) - 1] = '\0';
	strncpy( fontName, "Arial", sizeof( fontName ) - 1 );
	fontName[sizeof( fontName ) - 1] = '\0';

	raw = gEngfuncs.COM_LoadFile( "efw_textscheme.txt", 5, NULL );
	if( !raw )
	{
		gEngfuncs.Con_Printf( "Unable to find *_textscheme.txt\n" );
	}
	else
	{
		pFile = (char *)raw;
		pFile = gEngfuncs.COM_ParseFile( pFile, token );
		while( pFile && token[0] )
		{
			char paramName[64];
			char paramValue[64];
			strncpy( paramName, token, sizeof( paramName ) - 1 );
			paramName[sizeof( paramName ) - 1] = '\0';
			pFile = gEngfuncs.COM_ParseFile( pFile, token );
			if( !pFile || stricmp( token, "=" ) )
			{
				if( current < 0 )
					gEngfuncs.Con_Printf( "error parsing font scheme text file at file start - expected '=', found '%s''\n", token );
				else
					gEngfuncs.Con_Printf( "error parsing font scheme text file at scheme '%s' - expected '=', found '%s''\n", schemeName, token );
				break;
			}
			pFile = gEngfuncs.COM_ParseFile( pFile, token );
			strncpy( paramValue, token, sizeof( paramValue ) - 1 );
			paramValue[sizeof( paramValue ) - 1] = '\0';
			if( !stricmp( paramName, "SchemeName" ) )
			{
				haveScheme = 1;
				current++;
				strncpy( schemeName, paramValue, sizeof( schemeName ) - 1 );
				schemeName[sizeof( schemeName ) - 1] = '\0';
			}
			else if( current < 0 )
			{
				gEngfuncs.Con_Printf( "font scheme text file MUST start with a 'SchemeName'\n" );
				break;
			}
			else if( !stricmp( paramName, "FontName" ) )
			{
				strncpy( fontName, paramValue, sizeof( fontName ) - 1 );
				fontName[sizeof( fontName ) - 1] = '\0';
			}
			else if( !stricmp( paramName, "FontSize" ) )
				fontSize = atoi( paramValue );
			else if( !stricmp( paramName, "FgColor" ) )
				sscanf( paramValue, "%d %d %d %d", &r, &g, &b, &a );
			pFile = gEngfuncs.COM_ParseFile( pFile, token );
		}
		gEngfuncs.COM_FreeFile( raw );
		(void)haveScheme;
	}
	gEngfuncs.Con_Printf( ">>> FUN_100352e0 scheme=%s font=%s size=%d\n", schemeName, fontName, fontSize );
	snprintf( line, sizeof( line ), "EFWVGUI SCHEME %s\t%s\t%d\t%d,%d,%d,%d",
		schemeName, fontName, fontSize, r, g, b, a );
	EFW_VguiEmit( NULL, line );
}

/* FUN_10041fb0 (%C): a map hit is the display string. A miss copies the
   targetname and turns '_' into a space. "Talk to %C" / "Give %i to %C". */
static void EFW_PercentC( char *out, size_t n, const char *raw )
{
	static const struct
	{
		const char *key;
		const char *disp;
	} map[] = {
		{ "efw_compound_gate_guard", "Gate Guard" },
		{ "efw_electrician", "Electrician" },
		{ "detainee", "Detainee" },
		{ "detainee queue", "Detainee in queue" },
	};
	size_t i;

	if( !out || n < 1 )
		return;
	out[0] = '\0';
	if( !raw || !raw[0] )
	{
		strncpy( out, "them", n - 1 );
		out[n - 1] = '\0';
		return;
	}
	for( i = 0; i < sizeof( map ) / sizeof( map[0] ); i++ )
	{
		if( !strcmp( raw, map[i].key ) )
		{
			strncpy( out, map[i].disp, n - 1 );
			out[n - 1] = '\0';
			return;
		}
	}
	strncpy( out, raw, n - 1 );
	out[n - 1] = '\0';
	for( i = 0; out[i]; i++ )
	{
		if( out[i] == '_' )
			out[i] = ' ';
	}
}

static void EFW_BuildVgui( const EfwScanSlot *s, int x, int y )
{
	char label[64];
	char cmd[96];
	char who[64];
	if( s->type == 0 )
	{
		int id;
		EFW_PercentC( who, sizeof( who ), s->name );
		{
			static int s_pc;
			if( s_pc < 4 && s->name[0] && strcmp( s->name, who ) )
			{
				s_pc++;
				gEngfuncs.Con_Printf( "efw: percentC %s -> %s\n", s->name, who );
			}
		}
		snprintf( label, sizeof( label ), "Talk to %s", who );
		snprintf( cmd, sizeof( cmd ), "efw_Talk %s", s->name[0] ? s->name : "" );
		EFW_VguiAdd( x, y, label, cmd, g_hBubble );
		/* FUN_10044f70 walks DAT_100a37a8 in columns of 10, stride 11.
		   Index 21, the red phone card, is the column that walk skips. */
		for( id = 16; id <= 24; id++ )
		{
			if( !EFW_HudWeaponVisited( id ) )
				continue;
			if( !EFW_HasWep( id ) )
				continue;
			snprintf( label, sizeof( label ), "Give %s to %s", EFW_WepLabel( id ), who );
			snprintf( cmd, sizeof( cmd ), "efw_Give %d %s", id, s->name[0] ? s->name : "" );
			/* The button stores weapon+0xbc. That handle is the orbit quad
			   and the fly (FUN_100464c0). */
			EFW_VguiAdd( x, y, label, cmd, EFW_ItemIcon( id ), "wep" );
		}
		return;
	}
	if( s->type == 1 )
	{
		if( !strcmp( s->name, "efw_IDTag_Position" ) )
		{
			/* FUN_10044f30(0x14). A miss jumps to the end of FUN_10044f70,
			   so the fence has no button until the ID tag is held. */
			if( EFW_HasWep( 20 ) )
			{
				snprintf( label, sizeof( label ), "Place %s on fence", EFW_WepLabel( 20 ) );
				snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %d %s", 20, s->name );
				EFW_VguiAdd( x, y, label, cmd, EFW_ItemIcon( 20 ), "wep" );
			}
		}
		else if( !strcmp( s->name, "efw_kitchen_bin" ) )
		{
			if( EFW_HasWep( 16 ) )
			{
				snprintf( label, sizeof( label ), "Hide %s in bin", EFW_WepLabel( 16 ) );
				snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %d %s", 16, s->name );
				EFW_VguiAdd( x, y, label, cmd, EFW_ItemIcon( 16 ), "wep" );
			}
		}
		else if( !strcmp( s->name, "efw_hiding_place" ) )
		{
			EFW_VguiAdd( x, y, "Hide under the building", "efw_HideUnderBuilding", g_hHide );
		}
		else if( !strcmp( s->name, "efw_PliersMarker" ) )
		{
			EFW_VguiAdd( x, y, "Take pliers", "efw_PickupPliers", g_hPliers );
		}
		else if( !strcmp( s->name, "efw_cage_door" ) )
		{
			if( EFW_HasWep( 17 ) )
			{
				snprintf( label, sizeof( label ), "Force open cage door with %s", EFW_WepLabel( 17 ) );
				snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %d %s", 17, s->name );
				EFW_VguiAdd( x, y, label, cmd, EFW_ItemIcon( 17 ), "wep" );
			}
		}
		else
		{
			snprintf( cmd, sizeof( cmd ), "efw_UseWithMarker %s", s->name );
			EFW_VguiAdd( x, y, s->name, cmd, 0 );
		}
		return;
	}
	if( s->type >= 100 )
	{
		int id = s->type - 100;
		snprintf( label, sizeof( label ), "Pick up %s", EFW_WepLabel( id ) );
		snprintf( cmd, sizeof( cmd ), "efw_Pickup %u", (unsigned)id );
		/* HasWep null loads efw_give_icon.spr. A hit uses weapon+0xbc.
		   The same handle is what FUN_100464c0 flies to the corner. */
		EFW_VguiAdd( x, y, label, cmd,
			EFW_HasWep( id ) ? EFW_ItemIcon( id ) : g_hGive,
			EFW_HasWep( id ) ? "wep" : "give" );
	}
}

static void EFW_DrawScanPrompts( int r, int g, int b )
{
	int i;
	g_vguiN = 0;
	for( i = 0; i < g_scanCount; i++ )
	{
		EfwScanSlot *s = &g_scan[i];
		int x, y;
		int projected = EFW_Project( s->x, s->y, s->z, &x, &y );
		if( projected )
		{
			static int s_bub;
			s_bub++;
			if( s_bub <= 3 || ( s_bub % 40 ) == 0 )
				gEngfuncs.Con_Printf( ">>> bubble %s sx=%d sy=%d type=%d\n",
					s->name[0] ? s->name : "?", x, y, s->type );
			/* FUN_10044f70 is only reached from FUN_10046370, the interact
			   bar click. g_contextMode is that flag. */
			if( g_contextMode )
				EFW_BuildVgui( s, x, y );
		}
		/* FUN_10044f70 returns without creating CommandButtons when the
		   projection misses or sits in the 90px edge band. */
	}
	/* FUN_10044f70 draws these as VGUI image buttons. The browser stand-in
	   is the HTML overlay (EFW_VguiSync), which uses the same sprites.
	   SPR_DrawHoles here as well stacked a second 128px icon on every prompt. */
	(void)r;
	(void)g;
	(void)b;
	EFW_VguiSync();
}

/* FUN_10043a70 / FUN_10043af0 call pfnFillRGBA (0x100a501c). GoldSrc
   blends that, so the 64 tile is opaque 20,20,20 and the 3px frame is
   opaque 50,50,50. This engine's FillRGBA adds: the frame read back
   117,112,107 on the diary strip. */
static void EFW_FillBlend( int x, int y, int w, int h, int r, int g, int b, int a )
{
	if( gEngfuncs.pfnFillRGBABlend )
		gEngfuncs.pfnFillRGBABlend( x, y, w, h, r, g, b, a );
	else
		FillRGBA( x, y, w, h, r, g, b, a );
}

/* FUN_10043af0: 4-edge FillRGBA border of thickness param_9. */
static void EFW_DrawBoxBorder( int x, int y, int w, int h, int r, int g, int b, int a, int t )
{
	int inner;
	if( t < 1 )
		t = 1;
	EFW_FillBlend( x, y, w, t, r, g, b, a );
	EFW_FillBlend( x, y + h - t, w, t, r, g, b, a );
	inner = h - t * 2;
	if( inner < 1 )
		inner = 1;
	EFW_FillBlend( x, y + t, t, inner, r, g, b, a );
	EFW_FillBlend( x + w - t, y + t, t, inner, r, g, b, a );
}

/* weapon_s sprite named by sprites/weapon_efw_*.txt. Slots 21..23 share
   efw_item_phonecard.spr. FUN_10043a70 draws [weapon+0xa8], which
   LoadWeaponSprites fills from that weapon_s line. */
static HSPRITE EFW_ItemIcon( int id )
{
	static const char *kPath[] = {
		"sprites/efw_item_pliers.spr",
		"sprites/efw_item_lever.spr",
		"sprites/efw_item_branch.spr",
		"sprites/efw_item_simcard.spr",
		"sprites/efw_item_idtag.spr",
		"sprites/efw_item_phonecard.spr",
		"sprites/efw_item_phonecard.spr",
		"sprites/efw_item_phonecard.spr",
		"sprites/efw_item_washingpowder.spr"
	};
	int i;
	if( id < 16 || id > 24 )
		return 0;
	i = id - 16;
	if( !g_hItem[i] )
		g_hItem[i] = EFW_LoadSpr( kPath[i] );
	return g_hItem[i];
}

/* FUN_10043a70: 64x64 dark tile + 3px grey frame, then SPR_Draw of
   weapon+0xa8 (hActive) at (x-32, y-32). The call is pfnSPR_Draw
   (0x100a5004), not DrawHoles. Rect is NULL, so the whole 128 sprite. */
static void EFW_DrawInvIcon( int x, int y, HSPRITE icon )
{
	wrect_t rc;
	int dw, dh;
	EFW_FillBlend( x, y, 64, 64, 20, 20, 20, 255 );
	EFW_DrawBoxBorder( x, y, 64, 64, 50, 50, 50, 255, 3 );
	{
		static int s_tile;
		if( !s_tile )
		{
			s_tile = 1;
			gEngfuncs.Con_Printf( ">>> FUN_10043a70 blend tile 20 frame 50\n" );
		}
	}
	if( !icon )
		return;
	dw = SPR_Width( icon, 0 );
	dh = SPR_Height( icon, 0 );
	if( dw < 1 )
		dw = 64;
	if( dh < 1 )
		dh = 64;
	rc.left = 0;
	rc.top = 0;
	rc.right = dw;
	rc.bottom = dh;
	SPR_Set( icon, 255, 255, 255 );
	SPR_Draw( 0, x - 32, y - 32, &rc );
}

/* FUN_10043dd0: bottom inventory strip when the diary fade is up.
   Walks DAT_100a37a8 weapon slots; x starts at 60, +128 per owned item.
   param_1 slides the strip in from the right; items only when fade==1.0. */
static void EFW_DrawInventoryStrip( float fade )
{
	int id;
	int n;
	int x;
	int y;
	int x0;
	int namesN;
	char names[160];
	static int s_invLog = -1;
	int packed;

	if( fade <= 0.0f )
		return;
	x0 = (int)( (float)ScreenWidth * ( 1.0f - fade ) );
	/* FUN_10043dd0: FUN_1001d750 from x=width*(1-fade) to width, y=height-190
	   to height, RGB 0.2,0.2,0.2, UV v=0.8, spr=0. Row 204 of ascale.spr
	   stores 216, so the strip is that grey. pfnFillRGBA would add it. */
	EFW_DrawTriQuad( (float)x0, (float)( ScreenHeight - 190 ),
		(float)ScreenWidth, (float)ScreenHeight,
		0.2f, 0.2f, 0.2f, 0.8f, 0 );
	{
		static int s_quad;
		if( !s_quad && fade > 0.5f )
		{
			s_quad = 1;
			gEngfuncs.Con_Printf( ">>> FUN_10043dd0 quad %d %d %d 190 rgb=51,51,51 a=%d\n",
				x0, ScreenHeight - 190, ScreenWidth - x0, EFW_AscaleAlpha( 0.8f ) );
		}
	}
	if( fade < 1.0f )
	{
		packed = (int)( fade * 10.0f + 0.5f );
		if( packed != s_invLog )
		{
			s_invLog = packed;
			gEngfuncs.Con_Printf( ">>> FUN_10043dd0 fade=%.2f\n", fade );
		}
		return;
	}
	/* FUN_10043dd0: fild(ScreenHeight) minus the float at 0x10064d44
	   (108.8), then ftol. The grey strip still starts at height-190.
	   Names are FUN_1001e7d0 45px above that icon row, xmax = x+64.
	   A space once the pen passes xmax-100 starts the next 15px line,
	   so "Phone Card" is two lines above the icon. */
	x = 60;
	y = (int)( (float)ScreenHeight - 108.8f );
	n = 0;
	names[0] = '\0';
	namesN = 0;
	/* Same DAT_100a37a8 walk as the Give buttons: skip id 21. */
	for( id = 16; id <= 24; id++ )
	{
		HSPRITE icon;
		if( !EFW_HudWeaponVisited( id ) )
			continue;
		if( !EFW_HasWep( id ) )
			continue;
		icon = EFW_ItemIcon( id );
		EFW_DrawInvIcon( x, y, icon );
		EFW_DrawWrapped( x, y - 45, x + 64, EFW_WepLabel( id ), 255, 255, 255 );
		if( namesN < (int)sizeof( names ) - 24 )
			namesN += snprintf( names + namesN, sizeof( names ) - namesN, "%s%s:%d",
				n ? "," : "", EFW_WepLabel( id ), (int)icon );
		n++;
		x += 128;
	}
	if( ( 100 + n ) != s_invLog )
	{
		s_invLog = 100 + n;
		gEngfuncs.Con_Printf( ">>> FUN_10043a70 %s\n", names );
	}
}

/* FUN_10044bf0: half extents, SpriteTexture, RenderMode 4, CullFace TRI_NONE,
   TRI_QUADS centered on the fly point. */
static void EFW_DrawCenteredSpr( float cx, float cy, float w, float h )
{
	struct model_s *model;
	float hx;
	float hy;
	static int s_logged;

	if( !g_iconFlySpr )
		return;
	if( w < 1.0f )
		w = 1.0f;
	if( h < 1.0f )
		h = 1.0f;
	hx = w * 0.5f;
	hy = h * 0.5f;
	if( !s_logged )
	{
		s_logged = 1;
		gEngfuncs.Con_Printf( ">>> FUN_10044bf0 x=%.1f y=%.1f w=%.1f h=%.1f spr=%d\n",
			cx, cy, w, h, g_iconFlySpr != 0 );
	}
	model = (struct model_s *)gEngfuncs.GetSpritePointer( g_iconFlySpr );
	if( gEngfuncs.pTriAPI && model && gEngfuncs.pTriAPI->SpriteTexture( model, 0 ) )
	{
		gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
		gEngfuncs.pTriAPI->CullFace( TRI_NONE );
		gEngfuncs.pTriAPI->Color4f( 1.0f, 1.0f, 1.0f, 1.0f );
		gEngfuncs.pTriAPI->Begin( TRI_QUADS );
		gEngfuncs.pTriAPI->TexCoord2f( 0.0f, 0.0f );
		gEngfuncs.pTriAPI->Vertex3f( cx - hx, cy - hy, 0.5f );
		gEngfuncs.pTriAPI->TexCoord2f( 1.0f, 0.0f );
		gEngfuncs.pTriAPI->Vertex3f( cx + hx, cy - hy, 0.5f );
		gEngfuncs.pTriAPI->TexCoord2f( 1.0f, 1.0f );
		gEngfuncs.pTriAPI->Vertex3f( cx + hx, cy + hy, 0.5f );
		gEngfuncs.pTriAPI->TexCoord2f( 0.0f, 1.0f );
		gEngfuncs.pTriAPI->Vertex3f( cx - hx, cy + hy, 0.5f );
		gEngfuncs.pTriAPI->End();
		return;
	}
	{
		wrect_t rc;
		int dw = (int)w;
		int dh = (int)h;
		if( dw < 1 )
			dw = 1;
		if( dh < 1 )
			dh = 1;
		rc.left = 0;
		rc.top = 0;
		rc.right = dw;
		rc.bottom = dh;
		SPR_Set( g_iconFlySpr, 255, 255, 255 );
		SPR_DrawHoles( 0, (int)( cx - hx ), (int)( cy - hy ), &rc );
	}
}

/* FUN_10046590 tail: 2s cubic ease from DAT_100bc360 to DAT_100bc36c. */
static void EFW_DrawIconFly( void )
{
	float now;
	float dt;
	float t;
	float cx;
	float cy;
	float sz;

	if( !g_iconFlyOn )
		return;
	now = EFW_ContextClock();
	if( now < g_iconFlyAt )
	{
		g_iconFlyOn = 0;
		return;
	}
	dt = now - g_iconFlyAt;
	/* 0x10059ea0 is 2.0s. A stuck client clock used to pin the quad, so a
	   run of unchanged timestamps still ends the fly. */
	{
		static int s_stuck;
		static float s_lastDt = -1.0f;
		if( dt > 2.0f )
		{
			g_iconFlyOn = 0;
			g_iconFlyLog = 0;
			s_stuck = 0;
			return;
		}
		if( dt == s_lastDt )
		{
			s_stuck++;
			if( s_stuck > 90 )
			{
				g_iconFlyOn = 0;
				g_iconFlyLog = 0;
				s_stuck = 0;
				return;
			}
		}
		else
		{
			s_stuck = 0;
			s_lastDt = dt;
		}
	}
	t = dt * 3.3333333f;
	if( t < 0.0f )
		t = 0.0f;
	if( t > 1.0f )
		t = 1.0f;
	t = 1.0f - t;
	t = 1.0f - t * t * t;
	cx = ( g_iconFlyTo[0] - g_iconFlyFrom[0] ) * t + g_iconFlyFrom[0];
	cy = ( g_iconFlyTo[1] - g_iconFlyFrom[1] ) * t + g_iconFlyFrom[1];
	sz = ( g_iconFlySize1 - g_iconFlySize0 ) * t + g_iconFlySize0;
	sz = sz + sz;
	if( g_iconFlyLog < 3 )
	{
		int bucket = 0;

		if( t >= 0.99f )
			bucket = 2;
		else if( t >= 0.5f )
			bucket = 1;
		if( bucket >= g_iconFlyLog )
		{
			g_iconFlyLog = bucket + 1;
			gEngfuncs.Con_Printf( ">>> FUN_10046590 fly t=%.2f x=%.1f y=%.1f sz=%.1f\n",
				t, cx, cy, sz );
		}
	}
	EFW_DrawCenteredSpr( cx, cy, sz, sz );
}

/* FUN_10046590: idle bar while DAT_100bc338==0 and DAT_100bc490!=0.
   DAT_100bc490 is the Cntxt scan count; DAT_100bc38c is the nearest name. */
/* FUN_1001d750 spr==0 paints sprites/ascale.spr with RenderMode
   kRenderTransAlpha and UV v=1 (the opaque row) at RGB 0,0,0.
   That tri call sticks on this renderer and the next world frame is black.
   pfnFillRGBA is additive, so a black rect adds nothing. pfnFillRGBABlend
   is the alpha fill of the same rectangle: opaque black behind the lines. */
static void EFW_DrawAscaleQuad( float x1, float y1, float x2, float y2, float v )
{
	int x, y, w, h;
	static int s_logged;

	(void)v;
	x = (int)x1;
	y = (int)y1;
	w = (int)( x2 - x1 );
	h = (int)( y2 - y1 );
	if( w < 1 )
		w = 1;
	if( h < 1 )
		h = 1;
	if( !s_logged )
	{
		s_logged = 1;
		gEngfuncs.Con_Printf( ">>> FUN_1001d750 blend %d %d %d %d\n", x, y, w, h );
	}
	if( gEngfuncs.pfnFillRGBABlend )
		gEngfuncs.pfnFillRGBABlend( x, y, w, h, 0, 0, 0, 255 );
	else
		FillRGBA( x, y, w, h, 0, 0, 0, 255 );
}

static void EFW_DrawInteractPrompt( void )
{
	int x;
	int y;
	int w;
	int nameW;
	const char *name;
	static char s_interact[32];
	static int s_wasStory;
	static int s_noneLog;

	if( g_contextMode )
	{
		if( g_vguiN <= 0 )
		{
			gHUD.DrawHudString( ScreenWidth / 2 - 75, ScreenHeight / 2 - 100,
				ScreenWidth, "No interactive objects", 255, 127, 127 );
			if( !s_noneLog )
			{
				s_noneLog = 1;
				gEngfuncs.Con_Printf( ">>> FUN_10046590 none\n" );
			}
		}
		else
			s_noneLog = 0;
		/* FUN_10046040 draws this after the quad, with the HUD glyphs. */
		EFW_DrawHotCaption();
		return;
	}
	s_noneLog = 0;
	if( g_storyCode )
	{
		s_wasStory = 1;
		return;
	}
	if( s_wasStory )
	{
		s_wasStory = 0;
		s_interact[0] = '\0';
	}
	/* FUN_10044950 writes DAT_100bc38c. A slot has to project inside
	   the 90px margin before the bar appears. The name is the last type-0
	   slot that does. A raw name of "Detainee" is left blank. Anything
	   else goes through FUN_10041fb0: a map hit is the display string,
	   and a miss turns '_' into a space. Markers turn the bar on and
	   do not replace that name. */
	if( g_scanCount <= 0 )
	{
		if( s_interact[0] )
		{
			s_interact[0] = '\0';
			gEngfuncs.Con_Printf( ">>> FUN_10046590 interact=\n" );
		}
		return;
	}
	{
		int i;
		int show = 0;
		const char *picked = "";
		static char s_plate[64];
		for( i = 0; i < g_scanCount; i++ )
		{
			int sx, sy;
			if( !EFW_Project( g_scan[i].x, g_scan[i].y, g_scan[i].z, &sx, &sy ) )
				continue;
			show = 1;
			if( g_scan[i].type != 0 )
				continue;
			if( !g_scan[i].name[0] || !strcmp( g_scan[i].name, "Detainee" ) )
				continue;
			picked = g_scan[i].name;
		}
		if( !show )
		{
			if( s_interact[0] )
			{
				s_interact[0] = '\0';
				gEngfuncs.Con_Printf( ">>> FUN_10046590 interact=\n" );
			}
			return;
		}
		/* Empty stays empty. FUN_10041fb0 would turn that into "them",
		   which is the talk-caption fallback, not the name plate. */
		s_plate[0] = '\0';
		if( picked[0] )
			EFW_PercentC( s_plate, sizeof( s_plate ), picked );
		name = s_plate;
	}
	w = 240;
	x = ScreenWidth / 2 - 120;
	y = ScreenHeight - 25;
	/* FUN_10046590: FUN_1001d750 from (width/2-120, height-25) to
	   (width/2+120, height), then the click line at height-20. */
	EFW_DrawAscaleQuad( (float)x, (float)y, (float)( x + w ), (float)ScreenHeight, 1.0f );
	gHUD.DrawHudString( ScreenWidth / 2 - 115, ScreenHeight - 20, ScreenWidth,
		"Click left mouse button to interact", 255, 255, 255 );
	if( name[0] )
	{
		nameW = (int)strlen( name ) * 7;
		/* Name panel: x from (width±nameW)/2 ∓ 5, y from height-45 to height-25.
		   nameW is strlen*7, the same width the PE computes before the quad. */
		EFW_DrawAscaleQuad(
			(float)( ( ScreenWidth - nameW ) / 2 - 5 ),
			(float)( ScreenHeight - 45 ),
			(float)( ( ScreenWidth + nameW ) / 2 + 5 ),
			(float)( ScreenHeight - 25 ),
			1.0f );
		gHUD.DrawHudString( ( ScreenWidth - nameW ) / 2, ScreenHeight - 40,
			ScreenWidth, name, 255, 255, 255 );
	}
	if( strcmp( s_interact, name ) )
	{
		strncpy( s_interact, name, sizeof( s_interact ) - 1 );
		s_interact[sizeof( s_interact ) - 1] = '\0';
		gEngfuncs.Con_Printf( ">>> FUN_10046590 interact=%s\n", name[0] ? name : "-" );
	}
}

/* FUN_100436c0: SPR_Set(0,0,0) then SPR_Draw of sprites/efw_grey.spr,
   stepped across the framebuffer. The sprite is index-alpha and every
   texel is index 128, which the gradient palette stores as
   RGBA(255,255,255,128). pfnSPR_Draw is kRenderTransAlpha (blend off,
   alpha test greater than 0) modulated by that black color, so each
   tile replaces the framebuffer with black. A 16px tile is thousands of
   draws on this present and never returns. The texels are uniform, so
   one opaque black fill is the same picture. FillRGBA adds light and
   left the letterbox showing through the comic. */
static void EFW_DrawGreyVeil( void )
{
	static int s_logged;

	if( !g_hGrey )
		g_hGrey = EFW_LoadSpr( "sprites/efw_grey.spr" );
	if( !g_hAscale )
	{
		g_hAscale = EFW_LoadSpr( "sprites/ascale.spr" ); /* FUN_10044e30 */
		gEngfuncs.Con_Printf( ">>> FUN_10044e30 ascale=%d grey=%d\n", g_hAscale != 0, g_hGrey != 0 );
	}
	if( !s_logged )
	{
		s_logged = 1;
		gEngfuncs.Con_Printf( ">>> FUN_100436c0 grey=%d rgb=0,0,0\n", g_hGrey != 0 );
	}
	if( gEngfuncs.pfnFillRGBABlend )
		gEngfuncs.pfnFillRGBABlend( 0, 0, ScreenWidth, ScreenHeight, 0, 0, 0, 255 );
	else
		FillRGBA( 0, 0, ScreenWidth, ScreenHeight, 0, 0, 0, 255 );
}

/* FUN_10043750: up to 12 frames of a 256px storyboard SPR in a 4x3 grid
   centered on an 800x600 box, last column/row clipped. */
static void EFW_DrawStoryboardTiles( HSPRITE spr )
{
	int frames, i, ox, oy, col, row, x, y, w, h;
	wrect_t rc;
	static int s_logged;

	if( !spr )
		return;
	frames = SPR_Frames( spr );
	if( frames < 1 )
		frames = 1;
	if( frames > 12 )
		frames = 12;
	ox = ( ScreenWidth - 800 ) / 2;
	oy = ( ScreenHeight - 600 ) / 2;
	SPR_Set( spr, 255, 255, 255 );
	for( i = 0; i < frames; i++ )
	{
		col = i % 4;
		row = i / 4;
		x = ox + col * 256;
		y = oy + row * 256;
		w = 256;
		h = 256;
		if( x + w > ox + 800 )
			w = ox + 800 - x;
		if( y + h > oy + 600 )
			h = oy + 600 - y;
		if( w < 1 || h < 1 )
			continue;
		rc.left = 0;
		rc.top = 0;
		rc.right = w;
		rc.bottom = h;
		SPR_DrawHoles( i, x, y, &rc );
	}
	if( s_logged != g_storyCode )
	{
		s_logged = g_storyCode;
		gEngfuncs.Con_Printf( ">>> FUN_10043750 frames=%d code=0x%x\n", frames, g_storyCode );
	}
}

/* sprites/ascale.spr, one index per row. FUN_1001d750 samples that
   row for every vertex. v*255 is the row, not the stored index. */
static int EFW_AscaleAlpha( float v )
{
	static const unsigned char kRow[256] = {
		0,0,0,1,1,2,2,4,4,4,5,5,6,7,8,8,
		9,9,10,11,11,12,13,13,14,15,16,17,18,18,19,20,
		20,22,22,24,24,25,26,27,28,28,29,31,31,33,33,34,
		35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,
		51,52,53,55,55,57,58,59,60,61,62,64,64,66,67,68,
		69,70,71,72,74,75,76,77,78,80,81,82,83,84,86,87,
		88,89,90,91,93,94,95,96,98,99,100,101,103,104,105,106,
		108,109,110,111,112,114,115,117,117,119,120,121,122,124,125,126,
		128,128,130,131,132,134,135,136,138,139,140,142,142,144,145,146,
		147,149,150,151,153,153,155,156,157,159,159,161,162,164,165,166,
		167,168,169,171,172,173,175,175,177,178,179,180,181,183,184,185,
		186,187,188,190,191,192,193,194,195,196,197,199,199,201,201,203,
		204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,
		220,221,222,223,223,225,226,226,227,228,229,230,231,231,233,233,
		234,235,236,237,237,238,239,240,240,241,242,243,243,244,245,246,
		246,247,248,248,249,249,250,251,251,252,252,253,254,254,254,255
	};
	int row;

	if( v < 0.0f )
		v = 0.0f;
	if( v > 1.0f )
		v = 1.0f;
	row = (int)( v * 255.0f + 0.5f );
	if( row < 0 )
		row = 0;
	if( row > 255 )
		row = 255;
	return kRow[row];
}

/* FUN_1001d750: TRIAPI textured quad. param_9!=0 uses SpriteTexture of that
   SPR (RenderMode normal, UV 0..1). param_9==0 loads sprites/ascale.spr,
   RenderMode kRenderTransAlpha, CullFace TRI_NONE, Color4f alpha 1, and
   every vertex at UV (u, v). ascale is index-alpha: row 0 is clear and
   row 255 (v=1) is opaque, and the RGB is the vertex color. */
static void EFW_DrawTriQuad( float x1, float y1, float x2, float y2,
	float r, float g, float b, float v, HSPRITE spr )
{
	int ir, ig, ib, ia, x, y, w, h;
	static int s_logged = -2;
	int key;

	if( spr == 0 && !g_hAscale )
	{
		g_hAscale = EFW_LoadSpr( "sprites/ascale.spr" );
		gEngfuncs.Con_Printf( ">>> FUN_10044e30 ascale=%d grey=%d\n",
			g_hAscale != 0, g_hGrey != 0 );
	}

	key = spr ? 1 : 0;
	if( s_logged != key )
	{
		s_logged = key;
		gEngfuncs.Con_Printf( ">>> FUN_1001d750 spr=%d rgb=%.2f,%.2f,%.2f v=%.2f\n",
			spr != 0, r, g, b, v );
	}

	ir = (int)( r * 255.0f + 0.5f );
	ig = (int)( g * 255.0f + 0.5f );
	ib = (int)( b * 255.0f + 0.5f );
	if( ir < 0 ) ir = 0; if( ir > 255 ) ir = 255;
	if( ig < 0 ) ig = 0; if( ig > 255 ) ig = 255;
	if( ib < 0 ) ib = 0; if( ib > 255 ) ib = 255;
	x = (int)x1;
	y = (int)y1;
	w = (int)( x2 - x1 );
	h = (int)( y2 - y1 );
	if( w < 1 )
		w = 1;
	if( h < 1 )
		h = 1;

	if( spr != 0 )
	{
		wrect_t rc;
		rc.left = 0;
		rc.top = 0;
		rc.right = w;
		rc.bottom = h;
		SPR_Set( spr, ir, ig, ib );
		SPR_DrawHoles( 0, x, y, &rc );
		return;
	}

	/* FUN_1001d750 pins every vertex to this v, so the quad is one row of
	   sprites/ascale.spr. That file is not v*255: row 204 (v=0.8) stores
	   216, and row 255 stores 255. pfnFillRGBA would add the color, so a
	   (0, 0, 0.2) panel only tints the floor. The tri call sticks on this
	   renderer. pfnFillRGBABlend is that row. */
	ia = EFW_AscaleAlpha( v );
	if( ia < 1 )
		return;
	{
		static int s_blend;
		if( !s_blend )
		{
			s_blend = 1;
			gEngfuncs.Con_Printf( ">>> FUN_1001d750 blend %d %d %d %d rgb=%d,%d,%d a=%d\n",
				x, y, w, h, ir, ig, ib, ia );
		}
	}
	if( gEngfuncs.pfnFillRGBABlend )
		gEngfuncs.pfnFillRGBABlend( x, y, w, h, ir, ig, ib, ia );
	else
		FillRGBA( x, y, w, h, ir, ig, ib, ia );
}

/* FUN_1001e7d0: width is the console glyph at 0x100a4ddc. Soft wrap when
   the pen passes xmax-100 on a space, hard wrap at xmax, then y += 15. */
static int EFW_GlyphWidth( int ch )
{
	int w;

	if( ch < 0 || ch > 255 )
		return 8;
	w = gHUD.m_scrinfo.charWidths[ch];
	if( w < 1 )
		w = 8;
	return w;
}

/* Returns the y FUN_1001e7d0 stores (one 15px line past the last drawn row). */
static int EFW_DrawWrapped( int x, int y, int xmax, const char *text, int r, int g, int b )
{
	char line[256];
	int n = 0;
	int cx = 0;
	int cy = y;
	int i;
	int ch;
	int w;
	static int s_logged;

	if( !text || !text[0] )
		return y + 15;
	if( !s_logged && text[0] )
	{
		s_logged = 1;
		gEngfuncs.Con_Printf( ">>> FUN_1001e7d0 n=%d xmax=%d i=%d M=%d sp=%d\n",
			(int)strlen( text ), xmax,
			EFW_GlyphWidth( 'i' ), EFW_GlyphWidth( 'M' ), EFW_GlyphWidth( ' ' ) );
	}
	line[0] = '\0';
	for( i = 0; text[i]; i++ )
	{
		ch = (unsigned char)text[i];
		w = EFW_GlyphWidth( ch );
		if( ch == '\n' || ( ch == ' ' && x + cx + w > xmax - 100 ) || x + cx + w > xmax )
		{
			line[n] = '\0';
			if( n )
				gHUD.DrawHudString( x, cy, xmax, line, r, g, b );
			cy += 15;
			n = 0;
			cx = 0;
			if( ch == '\n' || ch == ' ' )
				continue;
		}
		if( n < (int)sizeof( line ) - 1 )
		{
			line[n++] = (char)ch;
			cx += w;
		}
	}
	line[n] = '\0';
	if( n )
		gHUD.DrawHudString( x, cy, xmax, line, r, g, b );
	return cy + 15;
}

/* FUN_10043bb0: growing center veil (FUN_1001d750 RGB 0,0,0.2 / 0,0,0.6),
   caption at x=0x4b, continue at (width-300, height-0x73) once fade>=1. */
static void EFW_DrawLetterbox( float flTime )
{
	float fade;
	float half;
	int y0;
	int y1;
	int yText;
	int h;
	int w;
	static int s_logged;

	if( g_captionLen <= 0 )
	{
		s_logged = 0;
		return;
	}
	h = ScreenHeight;
	w = ScreenWidth;
	(void)flTime;
	{
		float now = EFW_ClientTime();
		if( now > g_captionAt )
			g_captionAge = now - g_captionAt;
		else
			g_captionAge += 0.05f;
	}
	fade = g_captionAge * 3.3333333f;
	if( fade > 1.0f )
		fade = 1.0f;
	if( fade < 0.0f )
		fade = 0.0f;
	half = ( fade + 1.0f ) * 0.5f * ( (float)h - 100.0f ) * 0.5f;
	y0 = (int)( (float)h * 0.5f - half );
	y1 = (int)( (float)h * 0.5f + half );
	if( y0 < 0 )
		y0 = 0;
	if( y1 > h )
		y1 = h;
	/* FUN_1001d750 RGB 0,0,0.6 borders then 0,0,0.2 veil, param_9=0 ascale. */
	EFW_DrawTriQuad( 0.0f, (float)y0 - 1.0f, (float)w, (float)y0,
		0.0f, 0.0f, 0.6f, 1.0f, 0 );
	EFW_DrawTriQuad( 0.0f, (float)y0, (float)w, (float)( h / 2 ),
		0.0f, 0.0f, 0.2f, 1.0f, 0 );
	EFW_DrawTriQuad( 0.0f, (float)( h / 2 ), (float)w, (float)y1,
		0.0f, 0.0f, 0.2f, 1.0f, 0 );
	EFW_DrawTriQuad( 0.0f, (float)y1, (float)w, (float)y1 + 1.0f,
		0.0f, 0.0f, 0.6f, 1.0f, 0 );
	/* FUN_10043bb0: pen is h/2 - half + 40, then FUN_1001e7d0 at x=0x4b. */
	yText = (int)( (float)h * 0.5f - half + 40.0f );
	if( yText < 0 )
		yText = 0;
	EFW_DrawWrapped( 0x4b, yText, w - 0x4b, g_caption, 255, 255, 255 );
	if( fade >= 1.0f )
	{
		EFW_DrawWrapped( w - 300, h - 0x73, w,
			"Press left mouse button to continue", 255, 255, 255 );
		if( !s_logged )
		{
			s_logged = 1;
			gEngfuncs.Con_Printf( ">>> FUN_10043bb0 fade=1.00 y=%d\n", yText );
		}
	}
}

/* FUN_1001db00: ((target-old)*0.2+old-0.5)*1.03+0.5, clamp 0..1. */
static float EFW_LerpHudFade( float oldv, float target )
{
	float v = ( ( ( target - oldv ) * 0.2f + oldv ) - 0.5f ) * 1.03f + 0.5f;
	if( v < 0.0f )
		v = 0.0f;
	if( v > 1.0f )
		v = 1.0f;
	return v;
}

static void EFW_DrawDiarySpr( int page, float fade )
{
	int dw, dh, dx, dy, vis;

	if( fade <= 0.0f )
		return;
	if( g_loadedPage != page )
	{
		g_hDiary = 0;
		EFW_LoadDiarySprite( page, &g_hDiary );
		g_loadedPage = page;
	}
	if( !g_hDiary )
		return;
	dw = SPR_Width( g_hDiary, 0 );
	dh = SPR_Height( g_hDiary, 0 );
	if( dw < 1 )
		dw = 256;
	if( dh < 1 )
		dh = 256;
	/* FUN_1001d750: x = width-sprW, y = height-sprW, y2 = y + sprH*fade.
	   UV is 0..1 across that quad, so the page squashes while it wipes
	   instead of clipping the bottom lines. RGB 1,1,1, RenderMode normal. */
	dx = ScreenWidth - dw;
	dy = ScreenHeight - dw;
	{
		float y2 = (float)dy + (float)dh * fade;
		struct model_s *model;
		static int s_tri = -1;

		if( y2 <= (float)dy + 0.5f )
			return;
		model = (struct model_s *)gEngfuncs.GetSpritePointer( g_hDiary );
		if( gEngfuncs.pTriAPI && model
			&& gEngfuncs.pTriAPI->SpriteTexture( model, 0 ) )
		{
			if( s_tri != 1 )
			{
				s_tri = 1;
				gEngfuncs.Con_Printf( ">>> FUN_1001d750 diary uv fade=%.2f\n", fade );
			}
			gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
			gEngfuncs.pTriAPI->CullFace( TRI_NONE );
			gEngfuncs.pTriAPI->Color4f( 1.0f, 1.0f, 1.0f, 1.0f );
			gEngfuncs.pTriAPI->Begin( TRI_QUADS );
			gEngfuncs.pTriAPI->TexCoord2f( 0.0f, 0.0f );
			gEngfuncs.pTriAPI->Vertex3f( (float)dx, (float)dy, 0.5f );
			gEngfuncs.pTriAPI->TexCoord2f( 1.0f, 0.0f );
			gEngfuncs.pTriAPI->Vertex3f( (float)( dx + dw ), (float)dy, 0.5f );
			gEngfuncs.pTriAPI->TexCoord2f( 1.0f, 1.0f );
			gEngfuncs.pTriAPI->Vertex3f( (float)( dx + dw ), y2, 0.5f );
			gEngfuncs.pTriAPI->TexCoord2f( 0.0f, 1.0f );
			gEngfuncs.pTriAPI->Vertex3f( (float)dx, y2, 0.5f );
			gEngfuncs.pTriAPI->End();
			return;
		}
		if( s_tri != 0 )
		{
			s_tri = 0;
			gEngfuncs.Con_Printf( ">>> FUN_1001d750 diary holes fade=%.2f\n", fade );
		}
		vis = (int)( (float)dh * fade );
		if( vis < 1 )
			return;
		EFW_DrawTriQuad( (float)dx, (float)dy, (float)( dx + dw ), (float)( dy + vis ),
			1.0f, 1.0f, 1.0f, 1.0f, g_hDiary );
	}
}

/* DAT_100a95b8 veil, DAT_100a95bc inventory, DAT_100a95c4 diary SPR. */
static void EFW_TickHudFades( void )
{
	int menuOn = ( g_menuOn && !g_contextMode ) ? 1 : 0;
	int diaryOn = ( g_diaryOpen && !g_menuOn && !g_contextMode ) ? 1 : 0;
	float targetDiary = diaryOn ? 1.0f : 0.0f;
	int page;
	static int s_fadeLog = -1;
	int packed;

	/* FUN_1001db00: FUN_10047670(1) page, (2) flags, (5) diaryOpen. */
	page = EFW_GetClientHudInt( 1 );
	EFW_GetClientHudInt( 2 );
	EFW_GetClientHudInt( 5 );
	if( page == 0 )
		page = g_diaryPage;

	g_menuVeil = EFW_LerpHudFade( g_menuVeil, menuOn ? 1.0f : 0.0f );
	g_invFade = EFW_LerpHudFade( g_invFade, targetDiary );
	g_diaryFade = EFW_LerpHudFade( g_diaryFade, targetDiary );
	if( g_diaryFadePage >= 1 && page >= 1 && g_diaryFadePage != page && g_diaryFade == 1.0f )
		g_diaryFade = 0.0f;
	else
	{
		if( g_diaryFade > 0.99f && page > 0 )
			g_diaryFadePage = page;
		if( g_diaryFade < 0.01f && page == 0 )
			g_diaryFadePage = 0;
	}
	packed = ( g_diaryFade >= 1.0f ? 11 : (int)( g_diaryFade * 10.0f + 0.5f ) ) * 1000
		+ ( g_invFade >= 1.0f ? 11 : (int)( g_invFade * 10.0f + 0.5f ) ) * 100
		+ ( g_menuVeil >= 1.0f ? 11 : (int)( g_menuVeil * 10.0f + 0.5f ) ) * 10
		+ ( g_diaryOpen ? 1 : 0 );
	if( packed != s_fadeLog )
	{
		s_fadeLog = packed;
		gEngfuncs.Con_Printf(
			">>> FUN_1001db00 diaryfade=%.2f inv=%.2f veil=%.2f page=%d\n",
			g_diaryFade, g_invFade, g_menuVeil, page );
	}
}

static void EFW_DrawMenuVeil( void )
{
	float vw;
	if( g_menuVeil <= 0.0f )
		return;
	vw = (float)ScreenWidth * g_menuVeil * 1.1f;
	if( vw < 1.0f )
		return;
	/* FUN_1001db00: FillRGBA 2px rule at the veil top, RGB 100,100,150,
	   then FUN_1001d750(0, height-0x140, width*veil*1.1, height, 0, 0, 0.2, 1, 0).
	   v=1 is the opaque ascale row, so the panel covers that rule. */
	FillRGBA( 0, ScreenHeight - 0x140, (int)vw, 2, 100, 100, 150, 255 );
	EFW_DrawTriQuad( 0.0f, (float)( ScreenHeight - 0x140 ), vw, (float)ScreenHeight,
		0.0f, 0.0f, 0.2f, 1.0f, 0 );
}

static void EFW_DrawDiaryWipe( void )
{
	int cur = g_diaryPage;
	int prev = g_diaryFadePage;

	if( g_diaryFade <= 0.0f && g_invFade <= 0.0f )
		return;
	if( cur == 0 )
	{
		prev = 0;
		cur = g_diaryFadePage;
	}
	if( prev != cur && prev > 0 )
		EFW_DrawDiarySpr( prev, 1.0f );
	if( g_diaryFade > 0.0f )
		EFW_DrawDiarySpr( cur, g_diaryFade );
	/* FUN_1001db00 draws the page sprite and then the hope ticks.
	   It does not draw a DIARY caption. */
}

/* FUN_1001daa0: 10×(28×6) ticks at (20,120), stacked up by 12px.
   Filled (200,0,0,255) while ticks >= i; else (200,200,200,10). */
static void EFW_DrawHopeTicks( int ticks )
{
	int i;
	int y;
	static int s_logged = -1;

	/* FUN_1001daa0 calls pfnFillRGBA (engine index 11) for every tick.
	   Filled bars are 200,0,0,255. Empty bars are 200,200,200,10.
	   This host's FillRGBA adds, so a solid red bar sampled as
	   228,47,89. Those stay on pfnFillRGBABlend. Blend of alpha 10
	   left the sky above the stack unchanged, so the empty bars go
	   back through FillRGBA and deposit the faint grey. */
	y = 0x78 - 6;
	for( i = 0; i < 10; i++ )
	{
		if( ticks < i )
			FillRGBA( 0x14, y, 0x1c, 6, 200, 200, 200, 10 );
		else if( gEngfuncs.pfnFillRGBABlend )
			gEngfuncs.pfnFillRGBABlend( 0x14, y, 0x1c, 6, 200, 0, 0, 255 );
		else
			FillRGBA( 0x14, y, 0x1c, 6, 200, 0, 0, 255 );
		y -= 0xc;
	}
	if( s_logged != ticks )
	{
		s_logged = ticks;
		gEngfuncs.Con_Printf( ">>> FUN_1001daa0 ticks=%d hope=%.1f\n", ticks, g_hope );
	}
}

/* FUN_1001db00 tail: sprintf "%2d.%02d %s" into a stack buffer and drop it.
   sprites/efw_artslogo.spr is SPR_DrawAdditive at (width-sprW-5, 5),
   SPR_Set to fade*100. Index 0 in that sprite is black, so holes kept a box. */
static void EFW_DrawArtsClock( float flTime )
{
	int rgb;
	int dw;
	int x;
	float t0;
	float fade;

	if( flTime < 0.0f )
		flTime = 0.0f;
	t0 = 5.0f;
	fade = 1.0f - ( flTime - t0 ) * 0.2f;
	if( fade < 0.0f )
		fade = 0.0f;
	if( fade > 1.0f )
		fade = 1.0f;
	rgb = (int)( fade * 100.0f );
	if( rgb < 0 )
		rgb = 0;
	if( fade > 0.0f && !g_hLogo )
		g_hLogo = EFW_LoadSpr( "sprites/efw_artslogo.spr" );
	if( fade <= 0.0f || !g_hLogo )
	{
		if( fade <= 0.0f )
		{
			static int s_gone;
			if( !s_gone )
			{
				s_gone = 1;
				gEngfuncs.Con_Printf( "efw: arts logo faded t=%.1f\n", flTime );
			}
		}
		return;
	}
	dw = SPR_Width( g_hLogo, 0 );
	if( dw < 1 )
		dw = 64;
	x = ScreenWidth - dw - 5;
	SPR_Set( g_hLogo, rgb, rgb, rgb );
	SPR_DrawAdditive( 0, x, 5, NULL );
}

/* -dev 1 paints every client Con_Printf on the view. Cvar_SetValue
   does not move this cvar; the console command does. A frozen client
   clock keeps a line while cl.time - stamp is not greater than
   con_notifytime, so 0 never expires the line stamped at that clock.
   -1 does. One ClientCmd after drawing has started. */
static void EFW_SilenceNotify( void )
{
	static int s_sent;
	if( s_sent )
		return;
	s_sent = 1;
	{
		static char cmd[] = "con_notifytime -1\n";
		gEngfuncs.pfnClientCmd( cmd );
	}
}

/* StudioEstimateFrame adds (cl.time - animtime) * framerate * fps on top of
   the networked frame when cl.time is past animtime, and adds nothing when
   animtime is still ahead. A stamp one second behind leaves dlt at 1.25
   once the client leads gpGlobals->time by 0.25s, about ten walk frames
   past the server pose. A stamp 0.25s ahead meets that lead (dlt 0). While
   both engine clocks are still sitting on 1, the same stamp leaves dlt at
   -0.25 and the estimator draws the server frame. */
static void EFW_LogPoseClock( float flTime )
{
	cl_entity_t *lp;
	cl_entity_t *best;
	float bestD;
	int i;
	static int s_n;

	if( s_n >= 5 )
		return;
	s_n++;
	lp = gEngfuncs.GetLocalPlayer();
	best = NULL;
	bestD = 1.0e9f;
	for( i = 1; i <= 1024; i++ )
	{
		cl_entity_t *ent = gEngfuncs.GetEntityByIndex( i );
		float dx, dy, d;

		if( !ent || ent->curstate.modelindex <= 0 || ent->player )
			continue;
		if( ent->curstate.movetype != MOVETYPE_STEP && ent->curstate.sequence <= 0 )
			continue;
		if( !lp )
		{
			best = ent;
			break;
		}
		dx = ent->origin[0] - lp->origin[0];
		dy = ent->origin[1] - lp->origin[1];
		d = dx * dx + dy * dy;
		if( d < bestD )
		{
			bestD = d;
			best = ent;
		}
	}
	if( !best )
	{
		gEngfuncs.Con_Printf( "pose clock cl=%.3f none\n", flTime );
		return;
	}
	gEngfuncs.Con_Printf(
		"pose clock cl=%.3f anim=%.3f dlt=%.3f frame=%.1f fr=%.2f seq=%d mt=%d org=%.0f %.0f\n",
		flTime, best->curstate.animtime, flTime - best->curstate.animtime,
		best->curstate.frame, best->curstate.framerate, best->curstate.sequence,
		best->curstate.movetype, best->origin[0], best->origin[1] );
}

/* Level 2 env_sprite lamps: rendermode 5, renderamt 150, scale 0.5,
   sprites/yellow_flare.spr. Stock CSprite::Spawn leaves those keys.
   Called every frame; prints once per map, including modelindex 0. */
static void EFW_LogWorldSprites( void )
{
	int i;
	int n5;
	int nz;
	int shown;
	const char *map;
	static char s_map[64];

	map = gEngfuncs.pfnGetLevelName ? gEngfuncs.pfnGetLevelName() : "";
	if( !map || !map[0] )
		return;
	if( !strcmp( s_map, map ) )
		return;
	strncpy( s_map, map, sizeof( s_map ) - 1 );
	s_map[sizeof( s_map ) - 1] = '\0';
	n5 = 0;
	nz = 0;
	shown = 0;
	for( i = 1; i <= 1024; i++ )
	{
		cl_entity_t *ent = gEngfuncs.GetEntityByIndex( i );
		int mode5;
		int nearz;

		if( !ent || ent->player )
			continue;
		mode5 = ent->curstate.rendermode == 5;
		nearz = ent->origin[2] > 200.0f && ent->origin[2] < 280.0f;
		if( mode5 )
			n5++;
		if( nearz && ent->curstate.modelindex > 0 )
			nz++;
		if( !mode5 && ent->curstate.scale <= 0.01f && !nearz )
			continue;
		if( shown >= 8 )
			continue;
		shown++;
		gEngfuncs.Con_Printf(
			">>> sprite i=%d mi=%d mode=%d amt=%d scale=%.2f fx=%d mdl=%d org=%.0f %.0f %.0f\n",
			i, ent->curstate.modelindex, ent->curstate.rendermode,
			ent->curstate.renderamt, ent->curstate.scale, ent->curstate.effects,
			ent->model ? 1 : 0,
			ent->origin[0], ent->origin[1], ent->origin[2] );
	}
	gEngfuncs.Con_Printf( ">>> sprite map=%s mode5=%d nearz=%d\n", map, n5, nz );
}

/* Level-2 lamps are env_sprite yellow_flare, rendermode 5, scale 0.5.
   The client has the model, but this WebGL ref draws those sprites with
   no coverage. kFlare32 is sprites/yellow_flare.spr averaged to 8x8 blocks.
   FillRGBA on this host is additive, same as kRenderTransAdd. */
static const unsigned char kFlare32[32 * 32 * 3] = {
	0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,6,0,0,11,1,1,11,1,1,
	10,1,1,11,1,2,13,2,2,14,2,3,14,3,3,15,3,3,16,3,3,17,4,4,
	17,3,4,16,3,3,16,3,4,16,3,3,14,3,3,13,2,2,12,1,2,10,1,1,
	10,1,1,11,1,1,9,0,0,5,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,4,0,0,8,0,0,11,1,0,11,1,1,12,1,2,
	13,2,2,15,3,3,17,4,4,18,4,4,19,4,4,20,4,4,21,4,5,22,5,6,
	22,5,6,22,4,5,22,5,6,20,5,5,19,4,5,17,3,4,16,3,3,14,2,3,
	12,2,2,11,1,1,11,1,1,11,1,0,7,0,0,2,0,0,0,0,0,0,0,0,
	0,0,0,1,0,0,4,0,0,9,1,0,11,1,1,11,1,1,12,2,2,15,3,3,
	17,3,4,19,4,4,22,5,6,24,5,6,25,6,7,26,6,7,27,6,8,29,7,9,
	29,7,9,28,7,8,28,8,9,27,7,9,24,5,6,23,5,5,20,4,4,18,4,4,
	16,3,4,14,2,3,12,2,2,11,1,1,11,1,0,7,0,0,2,0,0,0,0,0,
	0,0,0,4,0,0,10,1,0,11,1,1,11,1,1,13,2,2,16,3,3,19,4,4,
	22,5,6,24,5,6,28,7,9,31,8,10,32,8,10,33,8,10,34,9,10,35,9,11,
	36,10,11,35,10,11,36,11,12,34,9,11,31,7,9,29,6,8,26,6,7,24,5,6,
	20,4,5,17,3,4,16,3,3,13,2,2,11,1,1,11,1,0,8,0,0,2,0,0,
	3,0,0,9,1,0,11,1,1,12,2,2,14,2,3,17,3,4,20,4,4,24,5,6,
	28,7,8,30,7,9,34,9,11,38,12,13,39,11,12,41,11,12,42,12,13,44,12,13,
	45,13,14,44,13,14,44,13,15,41,12,13,38,10,11,35,9,11,32,8,10,29,7,9,
	26,6,7,23,5,6,19,4,5,16,3,4,13,2,2,11,1,1,11,1,0,7,0,0,
	8,0,0,11,1,1,12,2,2,14,3,3,18,4,4,21,4,5,25,5,7,29,7,8,
	33,8,10,37,10,12,41,11,12,47,14,15,49,14,16,50,14,16,52,15,16,54,16,17,
	55,16,18,55,16,18,55,17,18,50,14,16,46,13,14,43,12,13,39,11,12,35,9,11,
	32,8,10,29,7,9,25,6,7,20,4,4,16,3,3,13,2,2,11,1,1,11,1,0,
	11,1,0,12,1,1,14,3,3,18,4,4,21,4,5,26,6,8,30,7,9,34,9,11,
	39,11,12,45,13,14,49,14,15,56,17,18,61,19,21,65,19,21,74,21,22,82,23,23,
	84,24,24,82,24,25,75,23,25,62,18,20,57,16,18,52,15,16,48,13,15,42,12,13,
	39,11,13,34,10,11,29,7,9,24,5,6,19,4,4,16,3,3,12,2,2,12,1,1,
	12,1,1,13,2,2,16,3,4,21,5,6,26,6,8,30,7,10,36,10,11,41,11,12,
	47,13,14,53,15,16,61,18,20,79,22,23,116,32,30,128,33,29,127,33,30,124,33,31,
	124,34,32,128,36,33,132,36,32,124,31,28,99,26,25,70,20,21,57,16,18,52,15,17,
	47,14,15,39,11,12,33,8,10,28,6,8,24,5,6,19,4,4,15,3,3,12,2,2,
	12,2,2,15,3,3,19,4,4,24,5,6,30,8,10,36,11,12,41,12,13,48,14,15,
	55,16,17,68,19,21,112,29,27,129,34,30,112,36,36,102,36,37,100,35,37,103,37,39,
	105,38,41,107,41,43,102,37,39,100,33,34,116,33,31,129,32,29,94,25,25,65,20,22,
	54,16,17,45,12,14,38,10,12,33,8,10,27,6,8,22,4,5,17,3,4,14,2,3,
	13,2,3,17,3,4,22,5,5,28,6,8,34,8,10,41,12,13,49,15,16,57,17,19,
	76,22,23,126,31,28,112,32,32,100,35,37,108,41,43,119,48,50,120,47,49,123,49,51,
	125,51,53,129,54,56,119,47,49,111,42,44,104,37,40,100,33,34,127,36,33,112,30,28,
	63,18,20,52,15,16,44,12,13,37,10,11,31,7,10,26,6,7,20,4,5,16,3,3,
	15,3,3,20,4,4,25,6,7,31,7,10,37,10,11,45,13,14,55,16,17,76,23,24,
	129,34,30,106,33,33,102,36,38,114,43,45,125,50,53,142,63,64,145,63,64,148,65,65,
	152,67,68,155,71,73,141,60,61,131,54,56,121,48,50,113,44,46,103,37,39,118,33,31,
	110,28,26,61,17,19,50,14,16,42,12,13,35,9,11,29,7,9,23,5,6,18,4,4,
	17,3,4,22,5,5,28,6,8,34,9,11,41,11,12,50,14,16,63,18,20,120,30,27,
	111,35,35,106,39,42,118,46,48,131,54,56,148,65,66,164,77,78,172,85,86,174,86,87,
	178,90,92,179,93,94,166,78,79,156,70,71,144,63,64,131,55,57,112,42,44,99,34,36,
	123,33,30,93,25,24,57,16,18,47,13,14,38,10,12,32,8,10,27,6,8,21,5,6,
	19,4,4,25,5,6,31,7,9,37,10,11,46,13,14,56,16,17,91,24,24,121,33,31,
	100,35,37,118,47,49,137,59,61,152,68,69,168,81,82,184,96,98,198,112,116,197,110,114,
	201,116,120,200,115,119,189,101,104,179,91,93,168,81,83,145,63,64,125,50,52,109,40,43,
	98,33,34,128,32,29,69,19,21,52,15,17,44,13,14,37,11,12,30,8,10,23,5,7,
	20,4,5,26,6,7,33,8,10,40,11,12,49,14,15,63,18,19,124,31,28,99,33,34,
	109,40,43,126,51,53,151,67,68,175,88,89,189,101,103,202,116,120,215,140,144,220,149,152,
	224,159,161,218,147,150,208,126,130,202,117,121,182,94,96,162,75,75,139,59,61,119,47,49,
	103,37,39,116,34,33,102,28,28,60,19,21,48,14,16,38,11,12,30,8,9,24,5,6,
	23,5,7,29,8,9,36,10,12,43,12,13,53,15,16,78,21,22,125,33,30,99,35,37,
	117,45,48,138,58,60,162,75,76,185,97,100,207,125,129,220,152,154,237,189,186,246,216,204,
	249,224,210,243,204,197,233,180,179,213,138,141,197,109,113,176,88,89,154,69,70,133,57,59,
	116,46,49,102,36,37,125,32,29,62,18,19,49,14,15,39,11,12,32,8,10,26,6,7,
	24,5,7,30,8,10,38,11,13,48,14,16,59,19,21,101,28,28,115,35,34,106,40,42,
	125,50,52,146,65,66,172,84,85,196,108,112,215,142,145,240,198,193,252,239,219,255,254,241,
	255,254,245,254,251,233,249,225,210,229,173,172,209,129,134,193,106,110,168,82,84,140,61,63,
	116,44,47,97,34,36,127,33,29,70,20,21,51,14,16,41,11,12,33,9,10,27,6,7,
	24,5,7,30,8,10,37,10,12,47,13,15,58,17,19,112,29,27,106,34,35,111,43,46,
	134,58,60,158,75,77,186,98,100,207,125,129,227,169,170,249,226,210,255,254,244,255,255,255,
	255,255,255,255,255,253,254,251,233,244,209,200,218,150,153,195,107,110,168,81,83,143,62,64,
	120,47,50,100,36,38,123,33,31,79,22,22,52,15,16,42,12,13,34,9,11,27,6,8,
	24,5,6,30,7,9,37,10,11,46,13,14,58,17,18,114,29,27,102,33,34,109,41,44,
	131,54,56,154,71,73,182,93,96,206,125,131,232,181,180,253,240,221,255,255,253,255,255,255,
	255,255,255,255,255,255,255,254,244,247,217,206,220,155,158,198,111,116,171,84,85,145,64,65,
	122,48,51,101,36,39,120,33,31,83,23,23,53,15,17,42,12,13,34,9,11,28,6,8,
	24,5,6,30,7,9,37,10,11,46,13,14,58,16,18,113,29,26,101,32,33,107,40,43,
	129,52,55,153,70,72,181,92,94,205,123,129,232,180,179,252,238,219,255,255,251,255,255,255,
	255,255,255,255,255,254,254,253,241,247,217,205,223,162,164,200,115,120,175,89,90,148,67,70,
	125,50,53,101,37,40,123,34,31,81,22,23,53,15,16,42,12,13,34,9,11,28,6,8,
	23,5,6,29,7,9,36,10,11,45,13,14,57,17,18,105,28,26,109,33,33,105,40,42,
	127,52,55,152,71,73,179,93,95,204,122,127,223,163,165,244,209,200,254,250,233,255,255,251,
	255,255,252,255,254,246,253,240,222,236,192,188,213,141,147,191,102,106,163,79,81,142,62,65,
	121,49,51,101,38,41,130,36,34,78,24,25,55,17,19,44,13,14,35,10,12,28,7,9,
	24,6,7,30,8,9,36,10,12,45,13,14,54,15,17,85,23,23,123,36,34,105,41,44,
	128,54,57,144,65,67,162,78,80,187,99,102,208,132,138,229,178,177,244,209,200,252,237,220,
	252,238,221,250,229,213,238,195,191,224,165,167,202,120,126,177,89,92,154,71,73,133,56,58,
	113,43,46,97,34,36,127,33,30,65,19,20,50,15,16,41,12,13,34,10,11,28,7,9,
	21,4,5,27,6,8,34,9,11,44,13,14,55,17,19,73,24,26,132,36,32,98,34,37,
	110,42,45,131,54,56,150,68,70,169,83,85,193,107,111,211,137,143,223,163,166,233,184,183,
	230,181,180,230,179,178,217,151,156,203,122,128,186,100,103,166,82,84,141,62,64,123,49,51,
	104,39,42,107,33,33,111,28,26,60,17,19,48,14,15,39,11,13,31,8,10,25,5,7,
	22,5,6,28,8,10,35,11,12,41,13,14,49,14,16,59,17,19,106,27,26,111,32,32,
	102,38,40,118,46,49,136,57,60,153,71,74,173,88,90,185,100,102,198,118,123,209,135,141,
	206,129,135,204,127,133,195,113,117,179,93,95,162,78,80,149,68,70,136,59,62,111,43,45,
	95,33,36,128,33,30,79,22,23,54,16,17,44,12,13,35,9,11,30,8,9,24,6,7,
	19,4,5,24,6,7,30,8,9,35,9,11,43,12,13,52,15,16,71,20,22,130,33,30,
	98,32,34,104,39,42,121,48,51,141,62,65,149,68,70,162,77,80,172,87,88,180,95,97,
	175,90,92,174,88,90,172,88,90,156,72,75,145,65,67,130,53,56,116,45,48,105,41,44,
	112,34,33,111,28,26,60,17,19,49,14,15,41,12,13,34,9,11,27,6,8,21,4,5,
	17,3,3,21,4,5,26,6,7,32,8,10,39,11,12,48,14,15,57,16,18,87,23,24,
	127,33,30,96,33,35,109,43,46,118,47,49,131,54,57,141,62,65,150,69,71,154,72,75,
	151,69,71,149,68,70,149,69,71,137,59,61,125,50,53,114,44,47,99,36,39,104,33,33,
	130,35,32,71,21,23,52,15,16,44,12,13,36,10,11,30,7,9,25,6,7,20,4,5,
	17,3,3,18,4,4,24,5,6,30,8,9,36,10,11,41,11,12,49,14,15,59,17,18,
	95,25,25,130,36,33,99,34,35,101,37,40,110,42,45,120,48,51,129,55,58,130,54,57,
	129,53,55,127,52,54,124,51,53,119,48,51,106,39,42,97,35,37,107,33,33,126,32,28,
	75,21,22,58,18,20,49,15,16,39,11,12,32,8,10,27,6,8,22,4,5,17,3,3,
	17,3,3,18,3,4,21,5,6,26,6,7,30,7,9,36,10,11,43,12,13,50,14,16,
	61,19,21,89,25,25,127,32,29,112,33,33,97,34,36,100,36,39,109,43,46,108,41,44,
	107,40,43,106,40,42,102,37,40,102,39,42,98,32,34,120,33,31,120,31,28,71,20,22,
	55,15,17,48,13,15,42,13,14,37,11,13,29,7,9,24,5,6,19,4,4,17,3,3,
	14,2,2,18,3,3,18,3,4,22,4,5,27,6,7,31,8,10,37,10,11,45,13,14,
	52,15,17,56,16,18,70,20,22,106,27,26,129,34,30,121,34,32,112,36,36,102,32,33,
	100,32,33,103,32,33,111,32,31,128,36,33,125,33,29,91,24,24,62,18,19,55,16,18,
	47,13,14,40,11,12,35,9,11,31,8,10,27,7,9,21,4,5,18,3,3,17,3,3,
	8,1,1,15,2,3,18,3,3,18,4,4,23,5,6,27,6,8,33,9,11,38,11,13,
	42,12,13,47,13,14,53,15,17,58,16,18,68,20,22,86,24,25,108,30,29,112,28,26,
	113,29,27,110,28,26,96,25,25,78,22,23,66,20,22,56,16,17,50,14,16,45,13,14,
	40,12,13,34,9,11,30,7,9,25,6,7,22,5,6,19,4,5,18,4,3,13,2,2,
	5,0,0,10,1,1,16,3,3,18,3,3,19,4,4,23,5,6,28,7,9,31,8,10,
	35,9,11,40,11,13,43,12,13,49,14,16,51,14,16,56,17,19,59,18,20,57,16,18,
	57,16,18,57,16,18,55,15,17,53,15,17,53,16,18,47,13,14,41,11,12,37,10,11,
	33,9,11,30,7,9,25,6,7,21,4,5,18,4,4,18,3,3,14,2,2,8,1,1,
	3,0,0,6,0,0,10,1,2,16,3,3,18,4,4,20,4,5,22,5,6,25,6,7,
	29,7,9,33,9,11,35,9,11,40,12,13,42,12,13,46,14,15,47,14,15,46,13,14,
	46,13,14,46,13,14,45,13,14,43,12,13,42,12,14,39,11,13,34,9,11,31,8,10,
	28,6,8,25,6,7,21,4,5,18,4,4,17,3,3,14,2,3,8,1,1,5,0,0,
	1,0,0,3,0,0,6,0,0,10,1,1,16,3,3,19,3,4,19,4,4,21,4,5,
	25,6,7,26,6,7,30,7,9,32,8,10,34,9,11,37,11,13,38,11,12,37,10,11,
	37,10,11,37,10,11,36,10,11,34,9,11,33,9,11,32,9,11,28,7,8,25,6,7,
	22,5,5,20,4,4,19,4,4,18,3,3,14,2,2,8,1,1,5,0,0,2,0,0,
};

/* 90-degree projection. Rejects sprites behind the eye. */
/* kRenderTransAdd depth-tests. A point trace (hull 2) ignores the player,
   steps out of the eye if that start is solid, and stops 48 units short
   of the sprite so the lamp housing is not the occluder. A human-hull
   trace from the eye is startsolid and is not used. */
static int EFW_FlareOnScreen( float wx, float wy, float wz, float *sx, float *sy, float *dist )
{
	cl_entity_t *lp;
	float angles[3];
	float fwd[3], right[3], up[3], delta[3];
	float z, px, py;
	float eye[3];

	lp = gEngfuncs.GetLocalPlayer();
	if( !lp )
		return 0;
	gEngfuncs.GetViewAngles( angles );
	AngleVectors( angles, fwd, right, up );
	eye[0] = lp->origin[0];
	eye[1] = lp->origin[1];
	eye[2] = lp->origin[2] + 28.0f;
	delta[0] = wx - eye[0];
	delta[1] = wy - eye[1];
	delta[2] = wz - eye[2];
	z = delta[0] * fwd[0] + delta[1] * fwd[1] + delta[2] * fwd[2];
	if( z < 16.0f )
		return 0;
	px = ( delta[0] * right[0] + delta[1] * right[1] + delta[2] * right[2] ) / z;
	py = ( delta[0] * up[0] + delta[1] * up[1] + delta[2] * up[2] ) / z;
	*sx = ScreenWidth * 0.5f + px * (float)ScreenWidth * 0.5f;
	*sy = ScreenHeight * 0.5f - py * (float)ScreenWidth * 0.5f;
	*dist = z;
	return 1;
}

static void EFW_BlitFlare( int x, int y, int size, int amt )
{
	int cell;
	int i, j;

	if( size < 32 )
		size = 32;
	if( size > 512 )
		size = 512;
	cell = size / 32;
	if( cell < 1 )
		cell = 1;
	if( amt < 1 )
		amt = 1;
	if( amt > 255 )
		amt = 255;
	for( j = 0; j < 32; j++ )
	{
		for( i = 0; i < 32; i++ )
		{
			const unsigned char *rgb = kFlare32 + ( j * 32 + i ) * 3;
			int a = ( rgb[0] + rgb[1] + rgb[2] ) / 3;

			a = a * amt / 255;
			if( a < 6 )
				continue;
			FillRGBA( x + i * cell, y + j * cell, cell, cell, rgb[0], rgb[1], rgb[2], a );
		}
	}
}

/* ignore_pe is a physent slot, not the edict index. The eye sits inside
   the player box, and that slot was coming back as a fraction-0 hit. */
extern "C" int PM_GetPhysEntInfo( int ent );

/* Point hull. Pull the end 48 units toward the camera so the lamp brush
   around the sprite origin is not the hit. A wall on that segment hides
   the lamp. The player box does not. */
static int EFW_FlareBlocked( float wx, float wy, float wz, float *outFrac )
{
	cl_entity_t *lp;
	pmtrace_t *tr;
	float eye[3];
	float target[3];
	float start[3];
	float dir[3];
	float len;
	float pull;
	int ignore;

	if( outFrac )
		*outFrac = 1.0f;
	if( !gEngfuncs.PM_TraceLine )
		return 0;
	lp = gEngfuncs.GetLocalPlayer();
	if( !lp )
		return 0;
	eye[0] = lp->origin[0];
	eye[1] = lp->origin[1];
	eye[2] = lp->origin[2] + 28.0f;
	dir[0] = wx - eye[0];
	dir[1] = wy - eye[1];
	dir[2] = wz - eye[2];
	len = sqrtf( dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2] );
	if( len < 8.0f )
		return 0;
	pull = 48.0f;
	if( pull > len * 0.5f )
		pull = 0.0f;
	target[0] = eye[0] + dir[0] * ( ( len - pull ) / len );
	target[1] = eye[1] + dir[1] * ( ( len - pull ) / len );
	target[2] = eye[2] + dir[2] * ( ( len - pull ) / len );
	start[0] = eye[0];
	start[1] = eye[1];
	start[2] = eye[2];
	ignore = -1;
	tr = gEngfuncs.PM_TraceLine( start, target, PM_TRACELINE_PHYSENTSONLY, 2, ignore );
	if( tr && tr->ent > 0 && PM_GetPhysEntInfo( tr->ent ) == lp->index )
	{
		ignore = tr->ent;
		tr = gEngfuncs.PM_TraceLine( start, target, PM_TRACELINE_PHYSENTSONLY, 2, ignore );
	}
	if( tr && ( tr->startsolid || tr->fraction <= 0.001f ) )
	{
		start[0] = eye[0] + dir[0] * ( 32.0f / len );
		start[1] = eye[1] + dir[1] * ( 32.0f / len );
		start[2] = eye[2] + dir[2] * ( 32.0f / len );
		tr = gEngfuncs.PM_TraceLine( start, target, PM_TRACELINE_PHYSENTSONLY, 2, ignore );
	}
	if( !tr || tr->startsolid || tr->fraction <= 0.001f )
		return 0;
	if( outFrac )
		*outFrac = tr->fraction;
	return tr->fraction < 0.95f;
}

static void EFW_DrawWorldFlares( void )
{
	static int s_logged;
	static int s_prevN = -1;
	static int s_prevHide = -1;
	static int s_sx, s_sy, s_w;
	int i;
	int n;
	int hide;
	float hideFrac;

	n = 0;
	hide = 0;
	hideFrac = 1.0f;
	for( i = 1; i <= 1024; i++ )
	{
		cl_entity_t *ent = gEngfuncs.GetEntityByIndex( i );
		float sx, sy, dist, pix, frac;
		int w, amt;

		if( !ent || ent->player )
			continue;
		if( ent->curstate.rendermode != 5 || ent->curstate.scale <= 0.01f )
			continue;
		if( ent->curstate.modelindex <= 0 )
			continue;
		if( !EFW_FlareOnScreen( ent->origin[0], ent->origin[1], ent->origin[2], &sx, &sy, &dist ) )
			continue;
		if( EFW_FlareBlocked( ent->origin[0], ent->origin[1], ent->origin[2], &frac ) )
		{
			hide++;
			if( frac < hideFrac )
				hideFrac = frac;
			continue;
		}
		pix = ( 256.0f * ent->curstate.scale ) * (float)ScreenWidth / ( 2.0f * dist );
		w = (int)pix;
		if( w < 8 )
			continue;
		amt = ent->curstate.renderamt;
		if( amt <= 0 )
			amt = 150;
		EFW_BlitFlare( (int)( sx - w * 0.5f ), (int)( sy - w * 0.5f ), w, amt );
		s_sx = (int)sx;
		s_sy = (int)sy;
		s_w = w;
		n++;
	}
	/* First on-screen set, then again when a wall changes the hide
	   count. Stop after a handful so the console does not scroll. */
	if( s_logged < 4 && ( n > 0 || hide > 0 ) && ( n != s_prevN || hide != s_prevHide ) )
	{
		s_prevN = n;
		s_prevHide = hide;
		s_logged++;
		gEngfuncs.Con_Printf( ">>> flare blit n=%d hide=%d frac=%.2f xy=%d %d w=%d\n",
			n, hide, (double)hideFrac, s_sx, s_sy, s_w );
	}
}

int CHudEfw::Draw( float flTime )
{
	static int s_drawN;
	int r, g, b;

	s_drawN++;
	EFW_SilenceNotify();
	if( ( s_drawN % 30 ) == 0 )
		EFW_LogWorldSprites();
	if( s_drawN == 40 || s_drawN == 100 || s_drawN == 180 || s_drawN == 260 )
		EFW_LogPoseClock( flTime );
	if( s_drawN <= 8 || ( s_drawN % 120 ) == 1 )
		gEngfuncs.Con_Printf( "efw: HUD_Draw n=%d\n", s_drawN );
	/* First ClientFrame never returned after this log under software
	   rasterize. Skip FillRGBA/SPR until a few frames have completed. */
	if( s_drawN <= 8 )
	{
		EFW_BumpHudMsgCount();
		gEngfuncs.Con_Printf( "efw: HUD_Draw skip n=%d\n", s_drawN );
		return 1;
	}
	EFW_DrawWorldFlares();
	if( s_drawN <= 16 || ( s_drawN % 60 ) == 1 )
	{
		cl_entity_t *lp = gEngfuncs.GetLocalPlayer();
		float ang[3];
		gEngfuncs.GetViewAngles( ang );
		if( lp )
			gEngfuncs.Con_Printf( "efw: view origin=%.0f %.0f %.0f yaw=%.0f idx=%d\n",
				lp->origin[0], lp->origin[1], lp->origin[2], ang[1], lp->index );
		else
			gEngfuncs.Con_Printf( "efw: view origin=(no local player)\n" );
	}
	/* FUN_10044f30 leftover unique: quote even if interact / inv HUD
	   never walks slots (HIDEHUD_ALL / no look-use on boot map). */
	{
		static int s_redraw;
		if( !s_redraw )
		{
			s_redraw = 1;
			gEngfuncs.Con_Printf( ">>> FUN_10018c90\n" );
			gEngfuncs.Con_Printf( ">>> FUN_10044f70\n" );
			gEngfuncs.Con_Printf( ">>> FUN_100442f0 weapon_efw_Pliers\n" );
		}
	}
	(void)EFW_HasWep( g_weaponId > 0 ? g_weaponId : 16 );
	if( !g_hBubble )
		g_hBubble = EFW_LoadSpr( "sprites/efw_speech_bubble.spr" );
	if( !g_hHide )
		g_hHide = EFW_LoadSpr( "sprites/efw_hide_icon.spr" );
	if( !g_hPliers )
		g_hPliers = EFW_LoadSpr( "sprites/efw_item_pliers.spr" );
	if( !g_hGive )
		g_hGive = EFW_LoadSpr( "sprites/efw_give_icon.spr" );

	g_hudDrawTime = EFW_HudPlayTime( flTime );
	g_mapLevel = EFW_MapLevelFromName();
	EFW_SetClientHudInt( 3, g_mapLevel );
	(void)EFW_ClientTime();
	EFW_BumpHudMsgCount();
	/* FUN_1001db00: FUN_1001e4c0(1.0) then menu darken from FUN_10041a30. */
	EFW_HudColor( 1.0f );
	if( ( g_menuOn || g_hudMsgCount > 0 ) && !EFW_ContextOn() )
	{
		float depth;
		int n = EFW_MenuDepth();
		/* PE: (float)FUN_10041a30()*0.1 clamp 0..1, then 1-f*0.35 when DAT_100baee0>=1. */
		if( g_menuOn )
		{
			depth = (float)n * 0.1f;
			if( depth < 0.0f )
				depth = 0.0f;
			if( depth > 1.0f )
				depth = 1.0f;
			EFW_HudColor( 1.0f - depth * 0.35000002f );
		}
	}

	if( gHUD.m_iHideHUDDisplay & HIDEHUD_ALL )
	{
		EFW_DrawIconFly();
		EFW_DrawLetterbox( flTime );
		/* FUN_1001db00 draws the arts logo after FUN_10043bb0. */
		EFW_DrawArtsClock( g_hudDrawTime );
		return 1;
	}

	UnpackRGB( r, g, b, RGB_YELLOWISH );
	{
		float hopeF = EFW_HopeForDraw();
		int whole;
		int ticks;
		/* 0x1001e1ab: FUN_10047660(1), ftol, *0.1, ftol, then
		   FUN_1001daa0(0x14, 0x78, ticks). No HOPE word, no digits,
		   and no TALK label beside the bars. */
		if( hopeF < 0.0f )
			hopeF = 0.0f;
		whole = (int)hopeF;
		ticks = (int)( (float)whole * 0.1f );
		EFW_DrawHopeTicks( ticks );
	}

	EFW_DrawScanPrompts( r, g, b );

	if( g_storyCode && g_hStory )
	{
		/* HUD_Redraw calls FUN_10043a10 after FUN_1001db00, so the
		   letterbox is already up and the tiles cover it. */
		EFW_DrawLetterbox( flTime );
		/* FUN_1001db00 draws the arts logo after the letterbox and
		   before HUD_Redraw calls FUN_10043a10. The tiles cover it. */
		EFW_DrawArtsClock( g_hudDrawTime );
		/* FUN_10043a10: grey veil, fade DAT_100baf10 += 0.1, then FUN_10043750. */
		g_storyFade += 0.1f;
		if( g_storyFade > 1.0f )
			g_storyFade = 1.0f;
		EFW_DrawGreyVeil();
		EFW_DrawStoryboardTiles( g_hStory );
		/* FUN_10043a10 stops after the tiles. "Press left mouse button to
		   continue" is the caption letterbox (FUN_10043bb0), not this path. */
		return 1;
	}

	if( g_menuCode == 0x4d && !g_hStory )
		gHUD.DrawHudString( 16, 48, ScreenWidth - 16, "Run out of hope!", r, g, b );

	EFW_TickHudFades();
	EFW_DrawMenuVeil();

	if( g_menuOn )
	{
		/* FUN_1001db00: title starts at (ScreenHeight-320)+25, x=0x19,
		   xmax = ScreenWidth*0.75. The body reveals depth*3 characters.
		   Choices use "Press [%d]     %s" once that count covers line 0.
		   Previous text is the part after @@@@PREVIOUS_QUESTION:. */
		static const char kPrevMark[] = "@@@@PREVIOUS_QUESTION:";
		char body[1000];
		char prev[1000];
		const char *hit;
		int reveal;
		int fullLen;
		int x;
		int y;
		int xmax;
		int li;
		int choices;
		static int s_menuLog = -1;

		reveal = EFW_MenuDepth() * 3;
		if( reveal < 0 )
			reveal = 0;
		fullLen = (int)strlen( g_menuLine[0] );
		x = 0x19;
		y = ScreenHeight - 320 + 25;
		xmax = (int)( (float)ScreenWidth * 0.75f );
		body[0] = '\0';
		prev[0] = '\0';
		hit = strstr( g_menuLine[0], kPrevMark );
		if( hit )
		{
			int n = (int)( hit - g_menuLine[0] );
			if( n >= (int)sizeof( body ) )
				n = (int)sizeof( body ) - 1;
			memcpy( body, g_menuLine[0], (size_t)n );
			body[n] = '\0';
			strncpy( prev, hit + sizeof( kPrevMark ) - 1, sizeof( prev ) - 1 );
			prev[sizeof( prev ) - 1] = '\0';
		}
		else
		{
			strncpy( body, g_menuLine[0], sizeof( body ) - 1 );
			body[sizeof( body ) - 1] = '\0';
		}
		if( reveal < (int)strlen( body ) )
			body[reveal] = '\0';
		if( prev[0] )
		{
			EFW_DrawWrapped( x, y, xmax, prev, 100, 200, 100 );
			y += 0x23;
		}
		/* FUN_1001de18 pushes 200, 200, 0. That is the yellow body line. */
		y = EFW_DrawWrapped( x, y, xmax, body, 200, 200, 0 ) + 0x23;
		choices = ( reveal >= fullLen ) ? 1 : 0;
		{
			int choiceY[7] = {};
			int choiceH[7] = {};
			if( choices )
			{
				for( li = 1; li <= 6; li++ )
				{
					char press[320];
					int rowY;
					if( !g_menuLine[li][0] )
						continue;
					snprintf( press, sizeof( press ), "Press [%d]     %s", li, g_menuLine[li] );
					rowY = y;
					y = EFW_DrawWrapped( x, y, xmax, press, 100, 200, 100 ) + 0x0a;
					choiceY[li] = rowY;
					choiceH[li] = y - rowY;
				}
			}
			{
				int key = choices ? ( 100000 + fullLen ) : reveal;
				if( s_menuLog >= 100000 && reveal <= 3 )
					s_menuLog = -1;
				if( key != s_menuLog && ( reveal <= 9 || choices ) )
				{
					s_menuLog = key;
					gEngfuncs.Con_Printf(
						">>> FUN_1001db00 menu reveal=%d/%d choices=%d y0=%d body=200,200,0\n",
						reveal, fullLen, choices, ScreenHeight - 295 );
					/* The page parks the menuselect strips until these
					   rows exist. y is the DrawHudString pen for that
					   Press line; h includes the 0x0a gap. */
					if( choices )
					{
						for( li = 1; li <= 6; li++ )
						{
							if( !g_menuLine[li][0] )
								continue;
							gEngfuncs.Con_Printf(
								">>> FUN_1001db00 hit %d y=%d h=%d\n",
								li, choiceY[li], choiceH[li] );
						}
					}
				}
			}
		}
	}

	EFW_DrawInventoryStrip( g_invFade );
	EFW_DrawDiaryWipe();
	/* FUN_10046590 runs after the diary wipe. The name plate and the
	   click bar sit on top of the grey strip. Drawing them earlier left
	   the 0.8 veil over the glyphs (white text reads back as ~92). */
	EFW_DrawInteractPrompt();
	/* FUN_1001db00 calls FUN_10046590 after the menu panel, so the
	   shrinking quad sits on top of that veil. */
	EFW_DrawIconFly();
	/* FUN_10043bb0 runs after the diary sprite and the interact bar.
	   The center veil covers the page. The bottom 50px, including the
	   click bar, stays in view. Drawing the veil earlier left the
	   notebook and the grey strip on top of the caption. */
	EFW_DrawLetterbox( flTime );
	/* FUN_1001db00 draws sprites/efw_artslogo.spr after FUN_10043bb0.
	   The sprite is 128px at y=5, so the lower rows sit on the veil.
	   Drawing it earlier left that half under the blue. */
	EFW_DrawArtsClock( g_hudDrawTime );
	return 1;
}

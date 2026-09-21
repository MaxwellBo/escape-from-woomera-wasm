#!/usr/bin/env python3
"""Copy Escape from Woomera overlay sources into an hlsdk-portable tree and patch hooks."""
from __future__ import annotations

import shutil
import sys
from pathlib import Path

MARKER = "/* EFW_OVERLAY */"

ROOT = Path(__file__).resolve().parent


def once(path: Path, needle: str, insert: str) -> None:
    text = path.read_text(encoding="utf-8", errors="replace")
    if insert.strip() in text:
        return
    if needle not in text:
        raise SystemExit(f"patch failed: needle not found in {path}\n{needle!r}")
    path.write_text(text.replace(needle, insert, 1), encoding="utf-8")


def ensure_contains(path: Path, snippet: str) -> None:
    text = path.read_text(encoding="utf-8", errors="replace")
    if snippet in text:
        return
    path.write_text(text + "\n" + snippet + "\n", encoding="utf-8")


def main() -> None:
    if len(sys.argv) != 2:
        print("usage: overlay.py /path/to/hlsdk-portable", file=sys.stderr)
        sys.exit(2)
    sdk = Path(sys.argv[1]).resolve()
    dlls = sdk / "dlls"
    cldll = sdk / "cl_dll"
    if not dlls.is_dir() or not cldll.is_dir():
        raise SystemExit(f"not an hlsdk-portable tree: {sdk}")

    # Re-apply onto a clean SDK so file lists and HUD hooks are not stacked.
    if (sdk / ".git").is_dir() or (sdk / ".git").is_file():
        import subprocess
        tracked = [
            "CMakeLists.txt",
            "dlls/CMakeLists.txt",
            "cl_dll/CMakeLists.txt",
            "dlls/client.cpp",
            "dlls/player.cpp",
            "dlls/game.cpp",
            "dlls/gamerules.cpp",
            "dlls/multiplay_gamerules.cpp",
            "dlls/barney.cpp",
            "dlls/cbase.cpp",
            "dlls/subs.cpp",
            "dlls/world.cpp",
            "dlls/weapons.cpp",
            "dlls/util.cpp",
            "cl_dll/hud.h",
            "cl_dll/hud.cpp",
            "cl_dll/input.cpp",
            "cl_dll/cdll_int.cpp",
        ]
        subprocess.run(["git", "-C", str(sdk), "checkout", "--", *tracked], check=False)
    for leftover in (
        "efw_game.h",
        "efw_player.cpp",
        "hud_hope.cpp",
        "hud_diary.cpp",
    ):
        for folder in (dlls, cldll):
            path = folder / leftover
            if path.exists():
                path.unlink()

    copies = {
        "efw.h": dlls / "efw.h",
        "efw_dll.h": dlls / "efw_dll.h",
        "efw_script.h": dlls / "efw_script.h",
        "efw_script.cpp": dlls / "efw_script.cpp",
        "efw_dll.cpp": dlls / "efw_dll.cpp",
        "efw_conversation.cpp": dlls / "efw_conversation.cpp",
        "efw_clientcmd.cpp": dlls / "efw_clientcmd.cpp",
        "efw_entities.cpp": dlls / "efw_entities.cpp",
        "efw_weapons.cpp": dlls / "efw_weapons.cpp",
        "efw_world.cpp": dlls / "efw_world.cpp",
        "hud_efw.cpp": cldll / "hud_efw.cpp",
    }
    for src_name, dest in copies.items():
        shutil.copy2(ROOT / src_name, dest)

    cmake_dlls = dlls / "CMakeLists.txt"
    once(
        cmake_dlls,
        "\txen.cpp\n\tzombie.cpp\n",
        "\txen.cpp\n"
        "\tzombie.cpp\n"
        f"\tefw_script.cpp # EFW_OVERLAY\n"
        "\tefw_dll.cpp\n"
        "\tefw_conversation.cpp\n"
        "\tefw_clientcmd.cpp\n"
        "\tefw_entities.cpp\n"
        "\tefw_weapons.cpp\n"
        "\tefw_world.cpp\n",
    )

    cmake_cl = cldll / "CMakeLists.txt"
    once(
        cmake_cl,
        "\t../dlls/crowbar.cpp\n",
        "\t../dlls/crowbar.cpp\n"
        f"\t../dlls/efw_weapons.cpp # EFW_OVERLAY\n",
    )
    once(
        cmake_cl,
        "\tflashlight.cpp\n",
        "\tflashlight.cpp\n"
        f"\thud_efw.cpp # EFW_OVERLAY\n",
    )

    client_cpp = dlls / "client.cpp"
    once(
        client_cpp,
        '#include "weapons.h"\n',
        '#include "weapons.h"\n'
        f'#include "efw.h" {MARKER}\n',
    )
    once(
        client_cpp,
        "\tif( !pEntity->pvPrivateData )\n"
        "\t\treturn;\n"
        "\n"
        "\tentvars_t *pev = &pEntity->v;\n",
        "\tif( !pEntity->pvPrivateData )\n"
        "\t\treturn;\n"
        "\n"
        "\tentvars_t *pev = &pEntity->v;\n"
        f"\tif( EFW_ClientCommand( pEntity ) ) {MARKER}\n"
        "\t\treturn;\n",
    )
    once(
        client_cpp,
        "void ServerDeactivate( void )\n"
        "{\n"
        "	//ALERT( at_console, \"ServerDeactivate()\\n\" );\n",
        "void ServerDeactivate( void )\n"
        "{\n"
        f"	EFW_OnServerDeactivate(); {MARKER}\n"
        "	//ALERT( at_console, \"ServerDeactivate()\\n\" );\n",
    )
    once(
        client_cpp,
        "void ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )\n"
        "{\n"
        "	int		i;\n"
        "	CBaseEntity	*pClass;\n"
        "\n"
        "	//ALERT( at_console, \"ServerActivate()\\n\" );\n",
        "void ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )\n"
        "{\n"
        "	int		i;\n"
        "	CBaseEntity	*pClass;\n"
        "\n"
        f"	EFW_OnServerActivate(); {MARKER}\n"
        "	//ALERT( at_console, \"ServerActivate()\\n\" );\n",
    )
    once(
        client_cpp,
        "void StartFrame( void )\n"
        "{\n"
        "	//ALERT( at_console, \"SV_Physics( %g, frametime %g )\\n\", gpGlobals->time, gpGlobals->frametime );\n",
        "void StartFrame( void )\n"
        "{\n"
        f"	EFW_StartFrame(); {MARKER}\n"
        f"	EFW_RunQueuedChangeLevel(); {MARKER}\n"
        "	//ALERT( at_console, \"SV_Physics( %g, frametime %g )\\n\", gpGlobals->time, gpGlobals->frametime );\n",
    )

    player_cpp = dlls / "player.cpp"
    once(
        player_cpp,
        '#include "hltv.h"\n',
        '#include "hltv.h"\n'
        f'#include "efw.h" {MARKER}\n',
    )
    once(
        player_cpp,
        "\tgmsgStatusText = REG_USER_MSG( \"StatusText\", -1 );\n"
        "\tgmsgStatusValue = REG_USER_MSG( \"StatusValue\", 3 );\n"
        "}\n",
        "\tgmsgStatusText = REG_USER_MSG( \"StatusText\", -1 );\n"
        "\tgmsgStatusValue = REG_USER_MSG( \"StatusValue\", 3 );\n"
        f"\tEFW_LinkUserMessages(); {MARKER}\n"
        "}\n",
    )
    once(
        player_cpp,
        "\tm_afButtonReleased = buttonsChanged & ( ~pev->button );\t// The ones not down are \"released\"\n"
        "\n"
        "\tg_pGameRules->PlayerThink( this );\n",
        "\tm_afButtonReleased = buttonsChanged & ( ~pev->button );\t// The ones not down are \"released\"\n"
        "\n"
        f"\tEFW_PlayerPreThink( this ); {MARKER}\n"
        "\tg_pGameRules->PlayerThink( this );\n",
    )
    once(
        player_cpp,
        "\tg_pGameRules->PlayerSpawn( this );\n"
        "}\n",
        "\tg_pGameRules->PlayerSpawn( this );\n"
        f"\tEFW_PlayerSpawn( this ); {MARKER}\n"
        "}\n",
    )
    # Original SendHudState is PreThink-only. Do not also tick from UpdateClientData.
    player_text = player_cpp.read_text(encoding="utf-8", errors="replace")
    player_text = player_text.replace(
        "void CBasePlayer::UpdateClientData( void )\n"
        "{\n"
        "	EFW_PlayerPreThink( this ); /* EFW_OVERLAY */\n",
        "void CBasePlayer::UpdateClientData( void )\n"
        "{\n",
        1,
    )
    player_text = player_text.replace(
        "	EFW_PlayerHudPulse( this ); /* EFW_OVERLAY */\n",
        "",
    )
    player_cpp.write_text(player_text, encoding="utf-8")

    game_cpp = dlls / "game.cpp"
    once(
        game_cpp,
        '#include "game.h"\n'
        '#include "vcs_info.h"\n',
        '#include "game.h"\n'
        '#include "vcs_info.h"\n'
        f'#include "efw.h" {MARKER}\n',
    )
    once(
        game_cpp,
        '\tSERVER_COMMAND( "exec skill.cfg\\n" );\n'
        "}\n",
        '\tSERVER_COMMAND( "exec skill.cfg\\n" );\n'
        f"\tEFW_LinkUserMessages(); {MARKER}\n"
        "}\n",
    )

    gamerules = dlls / "gamerules.cpp"
    once(
        gamerules,
        "	if( !gpGlobals->deathmatch )\n",
        "	if( 1 || !gpGlobals->deathmatch ) /* EFW_OVERLAY: documentary SP */\n",
    )

    multiplay = dlls / "multiplay_gamerules.cpp"
    once(
        multiplay,
        "BOOL CHalfLifeMultiplay::FAllowMonsters( void )\n"
        "{\n"
        "	return ( allowmonsters.value != 0 );\n"
        "}\n",
        "BOOL CHalfLifeMultiplay::FAllowMonsters( void )\n"
        "{\n"
        "	return TRUE; /* EFW_OVERLAY */\n"
        "}\n",
    )

    subs = dlls / "subs.cpp"
    once(
        subs,
        '#include "doors.h"\n',
        '#include "doors.h"\n'
        f'#include "efw.h" {MARKER}\n',
    )
    once(
        subs,
        "void FireTargets( const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )\n"
        "{\n"
        "	edict_t *pentTarget = NULL;\n"
        "	if( !targetName )\n"
        "		return;\n",
        "void FireTargets( const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )\n"
        "{\n"
        "	edict_t *pentTarget = NULL;\n"
        "	if( !targetName )\n"
        "		return;\n"
        f"	if( EFW_FireTargets( targetName, pActivator, pCaller, (int)useType, value ) ) {MARKER}\n"
        "		return;\n",
    )

    cbase_cpp = dlls / "cbase.cpp"
    once(
        cbase_cpp,
        '#include\t"game.h"\n',
        '#include\t"game.h"\n'
        f'#include "efw.h" {MARKER}\n',
    )
    once(
        cbase_cpp,
        "\t\tpEntity->Spawn();\n"
        "\n"
        "\t\t// Try to get the pointer again, in case the spawn function deleted the entity.\n",
        f"\t\tEFW_OnDispatchSpawn( pent ); {MARKER}\n"
        f"\t\tif( EFW_ShouldSpawn( pent ) )\n"
        "\t\t\tpEntity->Spawn();\n"
        "\n"
        "\t\t// Try to get the pointer again, in case the spawn function deleted the entity.\n",
    )
    once(
        cbase_cpp,
        "\treturn 0;\n"
        "}\n"
        "\n"
        "void DispatchKeyValue( edict_t *pentKeyvalue, KeyValueData *pkvd )\n",
        f"\tif( EFW_RejectSpawn( pent ) ) {MARKER}\n"
        "\t\treturn -1;\n"
        "\treturn 0;\n"
        "}\n"
        "\n"
        "void DispatchKeyValue( edict_t *pentKeyvalue, KeyValueData *pkvd )\n",
    )

    world_cpp = dlls / "world.cpp"
    once(
        world_cpp,
        '#include "teamplay_gamerules.h"\n',
        '#include "teamplay_gamerules.h"\n'
        f'#include "efw.h" {MARKER}\n',
    )
    once(
        world_cpp,
        "void CWorld::Precache( void )\n"
        "{\n"
        "	g_pLastSpawn = NULL;\n",
        "void CWorld::Precache( void )\n"
        "{\n"
        f"	if( !EFW_BeginWorldPrecache() ) {MARKER}\n"
        "		return;\n"
        "	g_pLastSpawn = NULL;\n",
    )
    once(
        world_cpp,
        "		CVAR_SET_FLOAT( \"mp_defaultteam\", 0.0f );\n"
        "	}\n"
        "}\n",
        "		CVAR_SET_FLOAT( \"mp_defaultteam\", 0.0f );\n"
        "	}\n"
        f"	EFW_EndWorldPrecache(); {MARKER}\n"
        "}\n",
    )

    weapons_cpp = dlls / "weapons.cpp"
    once(
        weapons_cpp,
        '#include "gamerules.h"\n',
        '#include "gamerules.h"\n'
        f'#include "efw.h" {MARKER}\n',
    )
    once(
        weapons_cpp,
        "void UTIL_PrecacheOtherWeapon( const char *szClassname )\n"
        "{\n"
        "	edict_t	*pent;\n"
        "\n"
        "	pent = CREATE_NAMED_ENTITY( MAKE_STRING( szClassname ) );\n",
        "void UTIL_PrecacheOtherWeapon( const char *szClassname )\n"
        "{\n"
        "	edict_t	*pent;\n"
        "\n"
        f"	if( !EFW_PrecacheOnce( szClassname ) ) {MARKER}\n"
        "		return;\n"
        "	pent = CREATE_NAMED_ENTITY( MAKE_STRING( szClassname ) );\n",
    )
    once(
        weapons_cpp,
        "	g_sModelIndexFireball = PRECACHE_MODEL( \"sprites/zerogxplode.spr\" );// fireball\n",
        f"	EFW_WPrecache(); {MARKER}\n"
        "	g_sModelIndexFireball = PRECACHE_MODEL( \"sprites/zerogxplode.spr\" );// fireball\n",
    )

    util_cpp = dlls / "util.cpp"
    once(
        util_cpp,
        '#include "byteswap.h"\n',
        '#include "byteswap.h"\n'
        f'#include "efw.h" {MARKER}\n',
    )
    once(
        util_cpp,
        "void UTIL_PrecacheOther( const char *szClassname )\n"
        "{\n"
        "	edict_t	*pent;\n"
        "\n"
        "	pent = CREATE_NAMED_ENTITY( MAKE_STRING( szClassname ) );\n",
        "void UTIL_PrecacheOther( const char *szClassname )\n"
        "{\n"
        "	edict_t	*pent;\n"
        "\n"
        f"	if( !EFW_PrecacheOnce( szClassname ) ) {MARKER}\n"
        "		return;\n"
        "	pent = CREATE_NAMED_ENTITY( MAKE_STRING( szClassname ) );\n",
    )

    barney = dlls / "barney.cpp"
    once(
        barney,
        '#include\t"soundent.h"\n',
        '#include\t"soundent.h"\n'
        f'#include "efw.h" {MARKER}\n',
    )
    once(
        barney,
        '\tSET_MODEL( ENT( pev ), "models/barney.mdl" );\n'
        "\tUTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );\n",
        '\tif( !EFW_DeferStudio() )\n'
        '\t\tSET_MODEL( ENT( pev ), "models/barney.mdl" );\n'
        '\telse\n'
        '\t\tpev->model = MAKE_STRING( "models/barney.mdl" );\n'
        f"\tEFW_OverrideNpcModel( this ); {MARKER}\n"
        "\tUTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );\n",
    )

    hud_h = cldll / "hud.h"
    once(
        hud_h,
        "\tint m_iWidth;\t\t// width of the battery innards\n"
        "};\n"
        "\n"
        "//\n"
        "//-----------------------------------------------------\n"
        "//\n"
        "const int maxHUDMessages = 16;\n",
        "\tint m_iWidth;\t\t// width of the battery innards\n"
        "};\n"
        "\n"
        f"class CHudEfw: public CHudBase {MARKER}\n"
        "{\n"
        "public:\n"
        "\tint Init( void );\n"
        "\tint VidInit( void );\n"
        "\tint Draw( float flTime );\n"
        "\tvoid Reset( void );\n"
        "};\n"
        "\n"
        "//\n"
        "//-----------------------------------------------------\n"
        "//\n"
        "const int maxHUDMessages = 16;\n",
    )
    once(
        hud_h,
        "\tCHudFlashlight\tm_Flash;\n",
        "\tCHudFlashlight\tm_Flash;\n"
        f"\tCHudEfw\t\tm_Efw; {MARKER}\n",
    )

    hud_cpp = cldll / "hud.cpp"
    once(
        hud_cpp,
        "\tm_Flash.Init();\n",
        "\tm_Flash.Init();\n"
        f"\tm_Efw.Init(); {MARKER}\n",
    )

    cdll_int = cldll / "cdll_int.cpp"
    once(
        cdll_int,
        "int DLLEXPORT HUD_Redraw( float time, int intermission )\n"
        "{\n"
        "	gHUD.Redraw( time, intermission );\n"
        "\n"
        "	return 1;\n"
        "}\n",
        "int DLLEXPORT HUD_Redraw( float time, int intermission )\n"
        "{\n"
        "	static int s_efwRedraw;\n"
        "	s_efwRedraw++;\n"
        "	if( s_efwRedraw <= 32 || ( s_efwRedraw % 120 ) == 1 )\n"
        "		gEngfuncs.Con_Printf( \"efw: HUD_Redraw n=%d\\n\", s_efwRedraw );\n"
        "	/* TeamFortressViewport paint overflows after skip expires.\n"
        "	   EFW widgets plus the HTML overlay are the VGUI stand-in. */\n"
        "	gHUD.m_Efw.Draw( time );\n"
        "	if( s_efwRedraw <= 8 || ( s_efwRedraw % 120 ) == 1 )\n"
        "		gEngfuncs.Con_Printf( \"efw: HUD_Redraw skip n=%d\\n\", s_efwRedraw );\n"
        "	return 1;\n"
        "}\n",
    )
    once(
        cdll_int,
        "void DLLEXPORT HUD_Frame( double time )\n"
        "{\n"
        "	GetClientVoiceMgr()->Frame(time);\n"
        "}\n",
        "void DLLEXPORT HUD_Frame( double time )\n"
        "{\n"
        "	static int s_efwFrame;\n"
        "	s_efwFrame++;\n"
        "	if( s_efwFrame <= 32 )\n"
        "		return;\n"
        "	GetClientVoiceMgr()->Frame(time);\n"
        "}\n",
    )

    input_cpp = cldll / "input.cpp"
    once(
        input_cpp,
        "int DLLEXPORT HUD_Key_Event( int down, int keynum, const char *pszCurrentBinding )\n"
        "{\n"
        "	if (gViewPort)\n"
        "		return gViewPort->KeyInput(down, keynum, pszCurrentBinding);\n"
        "	return 1;\n"
        "}\n",
        "extern int EFW_ClientKey( int down, int keynum ); /* EFW_OVERLAY */\n"
        "int DLLEXPORT HUD_Key_Event( int down, int keynum, const char *pszCurrentBinding )\n"
        "{\n"
        "	if( EFW_ClientKey( down, keynum ) == 0 )\n"
        "		return 0;\n"
        "	if (gViewPort)\n"
        "		return gViewPort->KeyInput(down, keynum, pszCurrentBinding);\n"
        "	return 1;\n"
        "}\n",
    )

    cmake_root = sdk / "CMakeLists.txt"
    # xash3d-fwgs@1.2.2:
    # - SIDE_MODULE=1 keeps dlsym entry points (GiveFnptrsToDll / HUD_*).
    #   SIDE_MODULE=2 + gc-sections strips them because nothing in-tree refs them.
    # - -Bsymbolic binds C++ methods locally so they are not GOT.func imports.
    #   Without it the engine call_indirects through null ("RuntimeError: null function").
    # - WASM_BIGINT keeps libc time() as (i32)->i64 like the engine export.
    cmake_snip = (
        "# EFW_OVERLAY\n"
        "if(EMSCRIPTEN)\n"
        "\tset_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)\n"
        "\tadd_compile_options(-fPIC)\n"
        "\tadd_link_options(-sSIDE_MODULE=1 -sWASM_BIGINT=1 -Wl,-Bsymbolic)\n"
        '\tset(CMAKE_SHARED_LIBRARY_SUFFIX ".wasm")\n'
        '\tset(CMAKE_STRIP "/bin/true" CACHE STRING "" FORCE)\n'
        "endif()\n"
    )
    text = cmake_root.read_text(encoding="utf-8", errors="replace")
    text = text.replace("/* EFW_OVERLAY */\n", "")
    # Drop any previous overlay EMSCRIPTEN block (late or early) and reinsert
    # immediately after project() so flags apply to all targets.
    start = text.find("# EFW_OVERLAY\n")
    while start != -1:
        end = text.find("endif()\n", start)
        if end == -1:
            break
        text = text[:start] + text[end + len("endif()\n") :]
        start = text.find("# EFW_OVERLAY\n")
    if "project (HLSDK-PORTABLE)\n" in text:
        text = text.replace(
            "project (HLSDK-PORTABLE)\n",
            "project (HLSDK-PORTABLE)\n\n" + cmake_snip,
            1,
        )
    cmake_root.write_text(text, encoding="utf-8")
    print(f"overlay applied to {sdk}")


if __name__ == "__main__":
    main()

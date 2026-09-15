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

    copies = {
        "efw.h": dlls / "efw.h",
        "efw_game.h": dlls / "efw_game.h",
        "efw_script.h": dlls / "efw_script.h",
        "efw_script.cpp": dlls / "efw_script.cpp",
        "efw_conversation.cpp": dlls / "efw_conversation.cpp",
        "efw_player.cpp": dlls / "efw_player.cpp",
        "efw_entities.cpp": dlls / "efw_entities.cpp",
        "efw_weapons.cpp": dlls / "efw_weapons.cpp",
        "hud_hope.cpp": cldll / "hud_hope.cpp",
        "hud_diary.cpp": cldll / "hud_diary.cpp",
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
        "\tefw_conversation.cpp\n"
        "\tefw_player.cpp\n"
        "\tefw_entities.cpp\n"
        "\tefw_weapons.cpp\n",
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
        f"\thud_hope.cpp # EFW_OVERLAY\n"
        "\thud_diary.cpp\n",
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
        '\tSET_MODEL( ENT( pev ), "models/barney.mdl" );\n'
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
        f"class CHudHope: public CHudBase {MARKER}\n"
        "{\n"
        "public:\n"
        "\tint Init( void );\n"
        "\tint VidInit( void );\n"
        "\tint Draw( float flTime );\n"
        "\tvoid Reset( void );\n"
        "\tint MsgFunc_Hope( const char *pszName, int iSize, void *pbuf );\n"
        "private:\n"
        "\tint m_iHope;\n"
        "};\n"
        "\n"
        "class CHudDiary: public CHudBase\n"
        "{\n"
        "public:\n"
        "\tint Init( void );\n"
        "\tint VidInit( void );\n"
        "\tint Draw( float flTime );\n"
        "\tvoid Reset( void );\n"
        "\tint MsgFunc_EfwDiary( const char *pszName, int iSize, void *pbuf );\n"
        "private:\n"
        "\tint m_iOpen;\n"
        "\tint m_iPage;\n"
        "\tint m_iUnlocked;\n"
        "\tint m_iLoadedPage;\n"
        "\tHSPRITE m_hSprite;\n"
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
        f"\tCHudHope\t\tm_Hope; {MARKER}\n"
        "\tCHudDiary\t\tm_Diary;\n",
    )

    hud_cpp = cldll / "hud.cpp"
    once(
        hud_cpp,
        "\tm_Flash.Init();\n",
        "\tm_Flash.Init();\n"
        f"\tm_Hope.Init(); {MARKER}\n"
        "\tm_Diary.Init();\n",
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

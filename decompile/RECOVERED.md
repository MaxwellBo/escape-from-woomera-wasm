# Recovered map from the original Win32 DLLs

Ghidra 12.1.3 headless + Capstone. Image base `0x10000000`. Compiled Mon Jan 19 2004, PDB `C:\Development\EscapeFromWoomera\Source\dlls\Releasehl\EscapeFromWoomera.pdb`, linked as `spirit.dll`.

Decompiled C is in `decompile/out/*_ghidra_efw.c`. Recovered names are in `decompile/recovered/NAMES.md`. This is **not** a drop-in translation yet: Ghidra C uses `FUN_*` / `DAT_*`, MSVC thiscall, and GoldSrc engine callbacks.

The playable port in `game-logic/` follows these recovered functions (hope, talk range, `EFWData`, refugee IdleThink, ClientCommand names) and is compiled into `public/hlsdk/{client,server}.wasm`. Raw Ghidra C is not a drop-in compile.

## Server `EscapeFromWoomera.dll`

2942 functions decompiled. EFW overlay is a small cluster on Spirit of Half-Life (~RVA `0xc0000`–`0xc84b0`) plus `ClientCommand` at `0x1001a550`.

### Conversation engine (`efwConversation`)

Not exported. `Squark` is only named in an error string.

| VA | Recovered | What it does |
|----|-----------|----------------|
| `0x100b8ff0` | `LoadAll` | `GetGameDir` + `FindFirstFileA("…\\Conversations\\*.txt")`, parse each file, then seed keywords `ESCAPE`, `GREET`, `GOODBYE` |
| `0x100c2660` | `ParseFile` | allocate a 0x74-byte parser object (`0x100c1dc0`) |
| `0x100ba040` | `Squark` | `UTIL_FindEntityByTargetname`; if missing, `OutputDebugStringA("WARNING -- efwConversation::Squark -- character (%s) is not found")` |
| `0x100b86b0` | `RegisterDefaults` | display names: Gate Guard, Electrician, Detainee, Detainee in queue |
| `0x100c6e60` | `efw_ShowMenu` | up to 7 menu lines → GoldSrc `ShowMenu`; logs `CONVERSATION   (%d messages)` / `<conversation inactive>` |
| `0x100c6c10` | `efw_ThinkConversation` | hide menu if partner distance ≥ float at `0x1011d130`; log `Conversation hidden, partner too far` |
| `0x100c7830` | `efw_TalkScan` | sphere search `monster_refugee` / `monster_barney`, send user message `EFW_CtPrv` |

### Hope

| VA | Recovered | What it does |
|----|-----------|----------------|
| `0x100c6ad0` | `efw_ThinkHope` | `hope -= dt * (1/12)`, clamp 0..100. At 0: fail code `0x4d` and debug `Run out of hope!` |
| `0x100c6b60` | `efw_SendHudState` | pack hope / diary page / conversation flags into `EFWData` bytes |
| `0x100c81a0` / `0x100c81b0` | hud byte set/get | client reads slot 1 as the hope bar |

### Diary, markers, NPCs, weapons

| VA | Symbol |
|----|--------|
| `0x100c6890` | `efw_AddDiary` — `Diary active item added    %d` |
| `0x100c3120` | `efw_Marker` — `LINK_ENTITY`, alloc `0x15c` |
| `0x100c5ea0` | `monster_refugee` — `CRefugee`, alloc `0x3a8`, vtable `0x100f93a4` |
| `0x100c5f10` | `monster_efw_guard` — same vtable |
| `0x100c6440` | `CRefugee::IdleThink` — hull, `queue`, `now walking %s`, `mad_scientist_entity` |
| `0x100c4700`+ | `weapon_efw_{Pliers,Lever,Branch,MobilePhone,IDTag,*PhoneCard,WashingPowder}` |
| `0x1000d1d0` | guard models: electrician → `tradesman.mdl`, others → `security.mdl` |
| `0x1007af20` | `REG_USER_MSG`: `ShowMenu`, `EFWShow`, `EFWData`, `EFW_Menu`, `EFW_Cntxt`, `EFW_CtPrv` |

### `ClientCommand` (`0x1001a550`)

Ghidra only decompiled the `say` / `say_team` prefix. Capstone recovered the later `stricmp` chain (`decompile/recovered/ClientCommand_dispatch.c`):

`give`, `drop`, `use`, `efw_diary`, `efw_diary_next`, `efw_diary_prev`, `efw_changelevel`, `efw_pause`, `efw_set_state`, `efw_spider`, `efw_UseWithMarker`, `efw_Pickup`, `efw_Talk`, `efw_Give`, `efw_ShowMenu`, `efw_HideUnderBuilding`, `efw_HelpScreen`, `efw_PickupPliers`.

`efw_Talk` looks up the named NPC (`UTIL_FindEntityByTargetname`) and debug-prints `>>> efw_Talk %s`. `efw_PickupPliers` calls `Squark` then the stock give-weapon helper.

## Client `client.dll`

1735 functions decompiled. HUD owns diary sprites, speech-bubble prompts, storyboards, and the hope bar.

| VA | Recovered | What it does |
|----|-----------|----------------|
| `0x10018c60` | `HUD_Init` | hook `EFW_Menu`, init EFW HUD |
| `0x10018c90` | `HUD_Redraw` | `FUN_1001db00` — hope fade, `sprites/efw_diary_%02d%s.spr`, menu text `Press %d %s` |
| `0x100419e0` | | `HOOK_MESSAGE(EFWShow)` / `HOOK_MESSAGE(EFWData)` |
| `0x10044f70` | | world-space Talk/Give/Hide: `sprites/efw_speech_bubble.spr`, `efw_Talk %c`, give/hide icons |
| `0x10047720` | | `HOOK_MESSAGE(EFW_Menu)` and command slots `efw_ShowMenu %i` |
| `0x10047830` | | VGUI storyboards (`Storyboard/EFW_Storyboards_*.spr`, endings, intro, hiding) |

Client issues `efw_Talk %c` / `efw_Give %d %c` / `efw_UseWithMarker %d %c` as `ClientCmd` strings; the server `ClientCommand` above is the matching half.

## Next port (not in this change)

Recompile these C dumps against hlsdk-portable with:

- `thiscall` → explicit `this` (already the Ghidra calling convention on MSVC)
- engine callbacks (`DAT_10121e*` / `DAT_100a4ff0`) → `g_engfuncs` / `gEngfuncs`
- `FindFirstFileA(Conversations\\*.txt)` → `LOAD_FILE_FOR_ME` / packaged `public/Conversations/`
- user messages `EFWData` / `EFW_Menu` / `EFW_CtPrv` (the playable port no longer uses `efw_js_pick`)

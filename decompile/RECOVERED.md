# Recovered map from the original Win32 DLLs

Ghidra 12.1.3 headless + Capstone. Image base `0x10000000`. Compiled Mon Jan 19 2004, PDB `C:\Development\EscapeFromWoomera\Source\dlls\Releasehl\EscapeFromWoomera.pdb`, linked as `spirit.dll`.

Decompiled C is in `decompile/out/*_ghidra_efw.c`. Recovered names are in `decompile/recovered/NAMES.md`. This is **not** a drop-in translation yet: Ghidra C uses `FUN_*` / `DAT_*`, MSVC thiscall, and GoldSrc engine callbacks.

The playable port in `game-logic/` follows these recovered functions (hope, talk range, `EFWData`, `EFW_Cntxt` scan slots, gate FSM, FailOrNarrate, refugee IdleThink, ClientCommand names) and is compiled into `public/hlsdk/{client,server}.wasm`. Raw Ghidra C is not a drop-in compile.

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
| `0x100c6910` | `efw_FlagDiary` — set `DAT_10134474[page]` |
| `0x100c30a0` | `CEfwMarker::Spawn` — SOLID_NOT, DROP_TO_FLOOR, EF_NODRAW unless `showtriggers` |
| `0x100bfbf0` | conversation `ServerCommand` — `efw_GetPackage` (powder+phone, menu 0x47, hope+10), `efw_EndMailPickupMessage` (`FUN_100c77e0`) |
| `0x100c77c0` | PA rarLock=1, play `Ann_RAR_124.wav`, timer=0 |
| `0x100c77e0` | PA rarLock=0 |
| `0x100c3120` | `efw_Marker` — `LINK_ENTITY`, alloc `0x15c` |
| `0x100c5ea0` | `monster_refugee` — `CRefugee`, alloc `0x3a8`, vtable `0x100f93a4` |
| `0x100c5f10` | `monster_efw_guard` — same vtable |
| `0x100c6000` | `CRefugee::Precache` — 13 models (`detaineeMaleT0`–`T7`, `FemaleT0`–`T2`, security, tradesman) |
| `0x100c6040` | `CRefugee::Spawn` — `efw_FStrEq` name table; unknown logs `Model not known for name: %s` and alternates MaleT6/T7; `movetype=STEP`, `solid=BBOX`, health 80, `FL_MONSTER` |
| `0x100c27f0` | maplevel 2 fence IDTag spawn, then `FUN_100c2a20` |
| `0x100c29f0` | IDTag AddToPlayer cooldown (`GetTickCount` vs `this+0x12c`) then clear `Player'sIDTagOnFence` |
| `0x100c2a20` | IDTag UseWithMarker: `"ID Tag has been placed on the wall"`, abs-center, Materialize, NODRAW+SOLID_NOT, Squark gate guard |
| `0x100c4af0` | player look-use: sphere 96 from `EyePosition`, `_CIacos` cone 0.1745 rad, `TraceLine` 0.97, `efw_Marker` then vtable+0x114 |
| `0x100c6320` | CRefugee studio sequence hull (`GET_MODEL_PTR`, seqdesc bbmin/bbmax) |
| `0x100c6440` | `CRefugee::IdleThink` — hull, `queue`, `now walking %s`, `mad_scientist_entity` |
| `0x100c4700`+ | `weapon_efw_{Pliers,Lever,Branch,MobilePhone,IDTag,*PhoneCard,WashingPowder}` |
| `0x1000d1d0` | guard models: electrician → `tradesman.mdl`, others → `security.mdl` |
| `0x1007af20` | `REG_USER_MSG`: `ShowMenu`, `EFWShow`, `EFWData`, `EFW_Menu`, `EFW_Cntxt`, `EFW_CtPrv` |

### `ClientCommand` (`0x1001a550`)

Ghidra only decompiled the `say` / `say_team` prefix. Capstone recovered the later `stricmp` chain (`decompile/recovered/ClientCommand_dispatch.c`):

`give`, `drop`, `use`, `efw_diary`, `efw_diary_next`, `efw_diary_prev`, `efw_changelevel`, `efw_pause`, `efw_set_state`, `efw_spider`, `efw_UseWithMarker`, `efw_Pickup`, `efw_Talk`, `efw_Give`, `efw_ShowMenu`, `efw_HideUnderBuilding`, `efw_HelpScreen`, `efw_PickupPliers`.

`efw_Talk` looks up the named NPC (`UTIL_FindEntityByTargetname`) and debug-prints `>>> efw_Talk %s`. `efw_PickupPliers` calls `Squark` then the stock give-weapon helper.

Give virtuals (Capstone of the gap Ghidra skipped between `0x100c4f30` and `0x100c53c0`):

- `FUN_100c4e30` pliers → Amir/Fashid/Nasir/Mouhtaz, else `FUN_100c4550`
- `FUN_100c5240` phone → Gholan+GotHintAboutHiding, else `FUN_100c4550`
- `FUN_100c5330` powder → Mouhtaz gives Lever, else `FUN_100c4550`
- `FUN_100c4550` → `FUN_100b95a0(npc, classname, 8)` finds the UNWANTED_ITEM (type-1) question and Squarks flag 8; fallback `"Thanks, but I don't need it."`

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

Remaining thinner surfaces are CRT/STL helpers, the Win32 VGUI Panel class, and Spirit-of-HL stock AI. Overlay look-use `FUN_100c4af0` is in `EFW_LookUse` (IN_USE + HostFwd `efw_inuse`). `FUN_100c69a0` GetAsyncKeyState('1'–'6') is `EFW_PollMenuKeys` from SendHudState (`FUN_100c6a60`) plus the client `EFW_ClientKey` / `menuselect` stand-in. Intro menus `0x49`–`0x4b` stay client-only.

Client VGUI CommandButtons (`FUN_10044f70`) are ported as HUD widgets plus an HTML overlay. Give buttons loop weapon ids 16–24 like `DAT_100a37a8`. TalkScan also writes `EFWVGUI` lines to MEMFS `/efwvgui.txt` (and the engine log) so the browser can click the original `efw_Talk` / `efw_Give %d` / `efw_UseWithMarker %d` ClientCmds when HUD_Redraw never runs. Patrol `FUN_100c54e0` + `FUN_100c5480` chase-all-guards is in `CPatrolGuard::PatrolThink`. Look-use (`FUN_100c4af0`) runs on IN_USE so markers and world weapons work without those clicks. `FUN_100c6320` trusts sequence hulls only for `IDST` studio headers; Spawn still hardcodes the PE `-16..72` box. Flex scanner `FUN_100c1f20`–`FUN_100c2640` is the Q/A language implemented in `EfwScript_Parse`.

# REA pass: conversation engine

[REA](https://github.com/morluto/rea) 6.4.0 (`rea-agents`) with Ghidra 12.1.4 decompiled the conversation cluster the EFW export skipped. One MCP session (`rea --mcp`, protocol `2025-06-18`) opened the DLL, waited out analysis on the first `procedure_pseudo_code`, then reused that program for the rest.

Binary: `decompile/binaries/EscapeFromWoomera.dll` extracted from `public/woomera.zip` (`EscapeFromWoomera_v084/dlls/EscapeFromWoomera.dll`, 1 302 621 bytes, SHA-256 `b2395aab1e2ec65fd4fef354425866935e72d22e42fadab96ae167b416fccf74`). Image base `0x10000000`, language taken from the PE header (x86).

`ExportEfwDecomp.java` always keeps `0x100c0000`–`0x100c84b0` and any function that references an EFW string. The conversation code sits just below that window. `out/EscapeFromWoomera_functions.txt` lists 128 functions from `0x100b8000` through `0x100bffff`; `out/EscapeFromWoomera_ghidra_efw.c` keeps four of them (`RegisterDefaults`, `LoadAll`, `Squark`, `ServerCommand`).

## Checked in

Raw `procedure_pseudo_code` output. Banners are still `FUN_*`; recovered names are in `NAMES.md`.

| File | VA | Recovered |
|------|-----|-----------|
| `100b89a0.c` | `0x100b89a0` | `DisplayName` |
| `100b8c10.c` | `0x100b8c10` | `InitSpeechMap` |
| `100b8e10.c` | `0x100b8e10` | `InitCursorMap` |
| `100b93f0.c` | `0x100b93f0` | `SpeechRecord` |
| `100b94e0.c` | `0x100b94e0` | `ClearSpeechMap` |
| `100b95a0.c` | `0x100b95a0` | `GiveUnwanted` |
| `100b9720.c` | `0x100b9720` | `ReplyFlagsPass` |
| `100b9770.c` | `0x100b9770` | `MatchScriptName` |
| `100b9990.c` | `0x100b9990` | `ShowTopics` |
| `100ba080.c` | `0x100ba080` | `SetSpeech` |
| `100b9cd0.c` | `0x100b9cd0` | `ChooseReply` |

`efw_Talk` in `ClientCommand_dispatch.c` calls `0x100c6420` (21 bytes, already in the EFW dump): `MapLevel` (`0x100c5b80`) then `ShowTopics`. `0x100c7430` is `ShowMenu` with null lines and hide float `0x43480000` (200). It clears `talkActive` and leaves the speech map.

## Speech map

`InitSpeechMap` builds `DAT_10132428`. `SpeechRecord` looks the NPC up by entity pointer and returns the node at `+0x10`. The first float is the `FUN_100c5b60` stamp (`gpGlobals->time`). The `std::string` sits at `+8`.

`SetSpeech` (`Squark`'s body once the NPC exists):

- If bit 8 is clear and `now - stamp <= 1.0`, return without writing.
- Otherwise stamp `now`, copy the text, call `0x100c7430`, then a title-only `ShowMenu`.
- Hide distance stored at `DAT_1011d130`: bit 2 → `0x44800000` (1024), bit 4 → `0x43800000` (256), else `0x43000000` (128). Those floats are the hide distance, not the 20s title window (`DAT_1011d128`).

`ShowTopics` titles the menu with that NPC's string while `now - stamp < 20`. A miss uses the empty global at `0x10121c38`. It does not clear the record. The menu walks questions at one depth, at most six lines, and filters with the flag walker `0x100bdc40`. `ShowMenu`'s last argument is `0x43480000` (200).

`ClearSpeechMap` has three near calls, found by scanning `E8 rel32` in `.text`:

- `0x100c6c3e` and `0x100c6cda`, both in `efw_ThinkConversation`, after `0x100c7430` (20s timeout, then partner too far)
- `0x100b9fb3` in `ChooseReply`, on the `Goodbye` arm

`LoadAll` erases the same map up front with `FUN_100bb5b0` on `DAT_10132428`.

## ChooseReply

`0x100b9cd0` is 838 bytes and ends at the `ret` before `Squark` (`0x100ba015`). The prologue is `sub esp, 38h` / `mov esi, ecx` with no `push ebp`, and Ghidra's auto-analysis did not create a function. REA `procedure_pseudo_code` returned `No procedure exists at the requested address`. A headless `createFunction` at that address (same Ghidra 12.1.4) produced `100b9cd0.c`.

The body filters replies with `ReplyFlagsPass`, appends the hits, and picks `rand() % count` (`0x100c84e4`). It builds a result `std::string` from the empty global at `0x10121c38` and passes that string to each action virtual. Then:

- equal to empty, or equal to neither name below: store the follow-up question as the cursor, `SetSpeech` flag 8 with the reply text, `ShowTopics`
- equal to `Top` (`0x1011c424`): store a null cursor, `SetSpeech` flag 8, `ShowTopics` (the depth-0 menu)
- equal to `Goodbye` (`0x1011c41c`): null cursor, `SetSpeech` flag 8 only when the reply length at `+0x10` is nonzero, then `0x100c7430`, `ClearSpeechMap`, and copy the empty global over `DAT_10134480`

Every shipped `Conversations/*.txt` ends a branch with `A:[Goodbye]` (no answer text). None of them use `Top`.

`GiveUnwanted` finds the script the same way `ShowTopics` does (`pev+0x1cc` through `MatchScriptName`), asks `0x100bdc80` for a type-1 `UNWANTED_ITEM` question, keeps replies whose flags all pass (`ReplyFlagsPass`; a null list passes), and Squarks `rand() % count` with the caller's flag byte. The fallback at `0x1011c3fc` is `Thanks, but I don't need it.`

`DisplayName` returns the `RegisterDefaults` string at node `+0x1c`. A miss copies the key and replaces `_` with space.

## Not checked in

The same session decompiled `0x100ba1f0` through `0x100bb180` and `0x100bf8e0`, `0x100bf960`, `0x100bfad0`, `0x100bfd60`, `0x100bfec0`, `0x100bff60`, `0x100bffe0`. Those are MSVC `std::string` / map helpers (refcount byte at `ptr-1`, assign, destructor). Script actions in `Conversations/*.txt` stay `AddDiary`, `AddTopic`, `DeleteTopic`, `Goodbye`, and `ServerCommand`. Bison `FUN_100be970` (3713 bytes) and the flag walkers `0x100bdc40` / `0x100bdc80` were not decompiled in this pass; the port already walks type-1 questions and `FirstTime` flags.

## Port

`game-logic/efw_conversation.cpp` keeps 32 speech slots keyed by edict.

- `EFW_RememberSpeech` is `SetSpeech`'s write. The stamp is `EFW_HostClock()`, because the listen server parks `gpGlobals->time` and `FUN_100c5b60` would not move.
- `EFW_CopySpeechTitle` is the `ShowTopics` title test. Age must be strictly under 20 seconds.
- `EFW_StartTalk` (`0x100c6420`) does not clear the slot.
- `EFW_CloseTalk` matches `0x100c7430`: the menu goes, the slots stay.
- `EFW_ThinkConversation` calls `EFW_ClearSpeechRecords` after the timeout close and after `Conversation hidden, partner too far`. `EFW_ChooseTalk` does the same on a `Goodbye` action. That wipe is the whole table.
- `EFW_ChooseTalk` runs bracket actions when the answer text is empty, so `A:[Goodbye]` closes. `Top` sets the cursor back to depth 0 and shows the menu again.
- `EFW_LoadAllConversations` clears the table where `LoadAll` erases `DAT_10132428`.

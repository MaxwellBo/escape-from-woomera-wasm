# Recovered function names

| VA | Recovered name | Role |
|----|----------------|------|
| 0x1000d1d0 | `efw_AssignGuardModel` | targetname -> models/security.mdl or tradesman.mdl |
| 0x1007af20 | `LinkUserMessages` | registers ShowMenu, EFWShow, EFWData, EFW_Menu, EFW_Cntxt, EFW_CtPrv |
| 0x100ba040 | `efwConversation::Squark` | look up NPC by targetname; warn if missing |
| 0x100b86b0 | `efwConversation::RegisterDefaults` | Gate Guard / Electrician / detainee display names |
| 0x100b8ff0 | `efwConversation::LoadAll` | FindFirstFile game dir\\Conversations\\*.txt, then seed ESCAPE/GREET/GOODBYE |
| 0x100c1dc0 | `efwConversationFile ctor` | 0x74-byte conversation parser object |
| 0x100c2660 | `efwConversation::ParseFile` | open one Conversations/*.txt |
| 0x100c3120 | `efw_Marker` | LINK_ENTITY_TO_CLASS, alloc 0x15c |
| 0x100c3430 | `efw_HasKeyword` | return unlocked flag at keyword node +0x1c |
| 0x100c3500 | `efwConversation::AddKeyword` | insert keyword and store unlocked flag (0 = locked topic) |
| 0x100bfbf0 | `efw_ServerCommand` | GetPackage gives powder+phone, FailOrNarrate 0x47, hope+10; EndMail unlocks PA |
| 0x100c30a0 | `CEfwMarker::Spawn` | solid=0, movetype=7, DROP_TO_FLOOR, NODRAW unless showtriggers |
| 0x100c77c0 | `efw_PALockRAR` | rarLock=1, play slot 0 (Ann_RAR_124), timer=0 |
| 0x100c77e0 | `efw_PAUnlock` | rarLock=0 |
| 0x100c6910 | `efw_FlagDiary` | `DAT_10134474[page] = 1` |
| 0x100c4e30 | `weapon Give to NPC` | Amir / Fashid / Nasir / Mouhtaz Squark + diary 0x4c |
| 0x100c59c0 | `efw_ElectricianSees` | dist < 256 or view cone ~35° |
| 0x100c7510 | `efw_pause` | hudInt[6], MOVETYPE_NONE freeze |
| 0x100c7830 | `efw_TalkScan` | sphere 123, up to 3×0x30 slots, EFW_CtPrv |
| 0x100c7d30 | `efw_SendCntxt` | WRITE_BYTE(count) + count×0x30 raw EFW_Cntxt |
| 0x100c7da0 | `efw_GateFSM` | kitchen door, 1st/2nd compound entry, approach_bin |
| 0x100c81d0 | `efw_FailOrNarrate` | hope±15 for 0x3f/0x43; ShowMenu strings 0x3c–0x45; else EFW_Menu |
| 0x100c4700 | `weapon_efw_Pliers` | w/v/p_Pliers.mdl |
| 0x100c53c0 | `monster_patrol_guard` | LINK_ENTITY_TO_CLASS |
| 0x100c5480 | `efw_PatrolAlertAll` | every patrol sets chase state 4 |
| 0x100c54e0 | `CPatrolGuard::Think` | sight/hear FSM; halt; isolation 0x46 |
| 0x100c5ea0 | `monster_refugee` | CRefugee, alloc 0x3a8, vtable 0x100f93a4 |
| 0x100c5f10 | `monster_efw_guard` | same CRefugee vtable |
| 0x100c6440 | `CRefugee::IdleThink` | queue / mad_scientist_entity / walk-to-player |
| 0x100c6880 | `efw_DiaryCount` | return DAT_10134870 |
| 0x100c6890 | `efw_AddDiary` | append diary page; logs 'Diary active item added' |
| 0x100c6980 | `efw_Player` | return DAT_10134888 (local CBasePlayer*) |
| 0x100c6ad0 | `efw_ThinkHope` | hope -= dt*(1/12); clamp 0..100; at 0 fire menu 0x4d 'Run out of hope!' |
| 0x100c6b60 | `efw_SendHudState` | pack hope / diary / conversation into EFWData slots |
| 0x100c6c10 | `efw_ThinkConversation` | timeout + 'Conversation hidden, partner too far' |
| 0x100c6e60 | `efw_ShowMenu` | GoldSrc ShowMenu with up to 7 lines; logs CONVERSATION (n messages) |
| 0x100c80d0 | `efw_DebugPrint` | vsprintf + OutputDebugStringA |
| 0x100c8160 | `efw_FStrEq` | case-sensitive entity-name compare |
| 0x1001a550 | `ClientCommand` | say/say_team plus EFW cmds; strcmp chain in ClientCommand_dispatch.c |

## ClientCommand strcmp hits

- `0x1001ac19` `say`
- `0x1001ac64` `say_team`
- `0x1001acb0` `give`
- `0x1001ad87` `drop`
- `0x1001af70` `use`
- `0x1001b1bd` `efw_diary`
- `0x1001b217` `efw_diary_next`
- `0x1001b287` `efw_diary_prev`
- `0x1001b2f3` `efw_changelevel`
- `0x1001b34a` `efw_pause`
- `0x1001b3d0` `efw_set_state`
- `0x1001b450` `efw_spider`
- `0x1001b4c7` `efw_UseWithMarker`
- `0x1001b5d7` `efw_Pickup`
- `0x1001b656` `efw_Talk`
- `0x1001b784` `efw_Give`
- `0x1001b908` `efw_ShowMenu`
- `0x1001b969` `efw_HideUnderBuilding`
- `0x1001baab` `efw_HelpScreen`
- `0x1001bafc` `efw_PickupPliers`

## Client `client.dll` (image base 0x10000000)

| VA | Recovered name | Role |
|----|----------------|------|
| 0x10018c00 | `Initialize` | copy `cl_enginefunc_t` (0x62 dwords), require interface 7 |
| 0x10018c60 | `HUD_Init` | hooks EFW_Menu (`FUN_10047720`), EFW HUD (`FUN_100436a0`) |
| 0x10018c90 | `HUD_Redraw` | `CHud::Redraw` → diary/hope/menu (`FUN_1001db00`) |
| 0x1001db00 | `CHud::RedrawEFW` | hope fade, `sprites/efw_diary_%02d%s.spr`, ShowMenu text |
| 0x100419e0 | `HookEFWUserMsgs` | `HOOK_MESSAGE(EFWShow)`, `HOOK_MESSAGE(EFWData)` |
| 0x10044f70 | `CHudEfwPrompts::Draw` | world-space Talk/Give/Hide; `efw_Talk %c`, speech-bubble sprite |
| 0x10047720 | `HookEFWMenu` | `HOOK_MESSAGE(EFW_Menu)` + slots `efw_ShowMenu %i` |
| 0x10047830 | `CEfwStoryboard` | VGUI panels for intro/hiding/isolation/ending storyboards |
| 0x100442f0 | `weapon_efw_Pliers` | client prediction stub |

Hope on the client is HUD slot 1 (`FUN_10047670(1)`), filled by the server `EFWData` bytes from `efw_SendHudState`.

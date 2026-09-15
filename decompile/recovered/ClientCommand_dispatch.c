/* Recovered ClientCommand dispatch from EscapeFromWoomera.dll
 * Ghidra function FUN_1001a550 (0x1001a550) only decompiled the say/say_team
 * prefix. The remaining strcmp chain is reconstructed from Capstone.
 * Image base 0x10000000. Thiscall/cdecl as on Win32 MSVC 2004.
 */

/* pfnClientCommand(edict_t *pEntity) — GoldSrc DLL_FUNCTIONS slot */
void ClientCommand(edict_t *pEntity)
{
    const char *cmd = CMD_ARGV(0);
    /* 0x1001a550: say / say_team (Ghidra FUN_1001a550) */

    /* 0x1001ac19 strcmp(cmd, 'say' @ 0x10105450) */
    if (!stricmp(cmd, "say")) {
        /* call 0x1001ac4d -> FUN_1001a550 */
    }

    /* 0x1001ac64 strcmp(cmd, 'say_team' @ 0x10105444) */
    if (!stricmp(cmd, "say_team")) {
        /* call 0x1001ac99 -> FUN_1001a550 */
    }

    /* 0x1001acb0 strcmp(cmd, 'give' @ 0x1010563c) */
    if (!stricmp(cmd, "give")) {
        /* call 0x1001ad51 -> FUN_1001d7d0 */
        /* call 0x1001ad73 -> FUN_1007f800 */
    }

    /* 0x1001ad87 strcmp(cmd, 'drop' @ 0x10105634) */
    if (!stricmp(cmd, "drop")) {
        /* call 0x1001adf7 -> FUN_1001d7d0 */
        /* call 0x1001ae17 -> FUN_10081f40 */
        /* call 0x1001aebd -> FUN_1001d7d0 */
        /* call 0x1001aed8 -> FUN_100c8388 */
        /* call 0x1001af2f -> FUN_1001d7d0 */
        /* call 0x1001af4f -> FUN_100affd0 */
    }

    /* 0x1001af70 strcmp(cmd, 'use' @ 0x1010561c) */
    if (!stricmp(cmd, "use")) {
        /* call 0x1001afe0 -> FUN_1001d7d0 */
        /* call 0x1001b000 -> FUN_1007f400 */
        /* call 0x1001b01a -> FUN_100c88c0 */
        /* call 0x1001b06c -> FUN_1001d7d0 */
        /* call 0x1001b081 -> FUN_1007f400 */
        /* call 0x1001b09b -> FUN_1007f400 */
        /* call 0x1001b11d -> FUN_1001d7d0 */
        /* call 0x1001b12d -> FUN_1007f410 */
        /* call 0x1001b142 -> FUN_1007f410 */
        /* call 0x1001b192 -> FUN_1001d7d0 */
    }

    /* 0x1001b1bd strcmp(cmd, 'efw_diary' @ 0x10105600) */
    if (!stricmp(cmd, "efw_diary")) {
        /* call 0x1001b1f3 -> efw_GetHudByte */
        /* call 0x1001b200 -> efw_SetHudByte */
    }

    /* 0x1001b217 strcmp(cmd, 'efw_diary_next' @ 0x101055f0) */
    if (!stricmp(cmd, "efw_diary_next")) {
        /* call 0x1001b24d -> efw_SetHudByte */
        /* call 0x1001b252 -> FUN_100c6880 */
        /* call 0x1001b25b -> efw_GetHudByte */
        /* call 0x1001b270 -> efw_SetHudByte */
    }

    /* 0x1001b287 strcmp(cmd, 'efw_diary_prev' @ 0x101055e0) */
    if (!stricmp(cmd, "efw_diary_prev")) {
        /* call 0x1001b2bd -> efw_SetHudByte */
        /* call 0x1001b2c2 -> FUN_100c6880 */
        /* call 0x1001b2cb -> efw_GetHudByte */
        /* call 0x1001b2dc -> efw_SetHudByte */
    }

    /* 0x1001b2f3 strcmp(cmd, 'efw_changelevel' @ 0x101055d0) */
    if (!stricmp(cmd, "efw_changelevel")) {
    }

    /* 0x1001b34a strcmp(cmd, 'efw_pause' @ 0x101055c4) */
    if (!stricmp(cmd, "efw_pause")) {
        /* call 0x1001b3a3 -> FUN_100c7510 */
        /* call 0x1001b3b9 -> FUN_100c7510 */
    }

    /* 0x1001b3d0 strcmp(cmd, 'efw_set_state' @ 0x101055b4) */
    if (!stricmp(cmd, "efw_set_state")) {
        /* call 0x1001b439 -> FUN_100c3500 */
    }

    /* 0x1001b450 strcmp(cmd, 'efw_spider' @ 0x101055a8) */
    if (!stricmp(cmd, "efw_spider")) {
        /* call 0x1001b482 -> FUN_100c7820 */
        /* call 0x1001b490 -> efw_DebugPrint */
        /* call 0x1001b4a3 -> FUN_100c7d30 */
        /* call 0x1001b4aa -> efw_Player */
        /* call 0x1001b4b0 -> efw_FailOrNarrate */
    }

    /* 0x1001b4c7 strcmp(cmd, 'efw_UseWithMarker' @ 0x10105580) */
    if (!stricmp(cmd, "efw_UseWithMarker")) {
        /* call 0x1001b506 -> FUN_100c8388 */
        /* call 0x1001b55a -> FUN_10002dc0 */
        /* call 0x1001b56f -> FUN_10002ec0 */
        /* call 0x1001b584 -> FUN_10002ec0 */
        /* call 0x1001b598 -> FUN_100c2ee0 */
        /* call 0x1001b5ad -> UTIL_FindEntityByTargetname */
    }

    /* 0x1001b5d7 strcmp(cmd, 'efw_Pickup' @ 0x10105574) */
    if (!stricmp(cmd, "efw_Pickup")) {
        /* call 0x1001b614 -> FUN_100aec40 */
        /* call 0x1001b639 -> efw_Player */
    }

    /* 0x1001b656 strcmp(cmd, 'efw_Talk' @ 0x10105568) */
    if (!stricmp(cmd, "efw_Talk")) {
        /* call 0x1001b696 -> efw_Player */
        /* call 0x1001b6c6 -> efw_DebugPrint */
        /* call 0x1001b6d5 -> UTIL_FindEntityByTargetname */
        /* call 0x1001b738 -> UTIL_FindEntityByTargetname */
        /* call 0x1001b74f -> efw_DebugPrint */
        /* call 0x1001b755 -> FUN_100c6420 */
        /* call 0x1001b76d -> efw_DebugPrint */
    }

    /* 0x1001b784 strcmp(cmd, 'efw_Give' @ 0x10105514) */
    if (!stricmp(cmd, "efw_Give")) {
        /* call 0x1001b7c3 -> FUN_100c8388 */
        /* call 0x1001b817 -> FUN_10002dc0 */
        /* call 0x1001b82c -> FUN_10002ec0 */
        /* call 0x1001b840 -> FUN_100c2ee0 */
        /* call 0x1001b85c -> efw_Player */
        /* call 0x1001b87d -> UTIL_FindEntityByTargetname */
        /* call 0x1001b8d8 -> UTIL_FindEntityByTargetname */
    }

    /* 0x1001b908 strcmp(cmd, 'efw_ShowMenu' @ 0x10105504) */
    if (!stricmp(cmd, "efw_ShowMenu")) {
        /* call 0x1001b943 -> FUN_100c8388 */
        /* call 0x1001b94c -> efw_Player */
        /* call 0x1001b952 -> efw_FailOrNarrate */
    }

    /* 0x1001b969 strcmp(cmd, 'efw_HideUnderBuilding' @ 0x101054ec) */
    if (!stricmp(cmd, "efw_HideUnderBuilding")) {
        /* call 0x1001b99f -> FUN_100c5b80 */
        /* call 0x1001b9b1 -> FUN_100c3430 */
        /* call 0x1001b9c2 -> FUN_100c3430 */
        /* call 0x1001b9d3 -> efw_Player */
        /* call 0x1001b9d9 -> FUN_100c2f70 */
        /* call 0x1001b9e7 -> efw_Player */
        /* call 0x1001b9ed -> efw_FailOrNarrate */
        /* call 0x1001ba02 -> efw_Player */
        /* call 0x1001ba08 -> efw_FailOrNarrate */
        /* call 0x1001ba1d -> efw_Player */
        /* call 0x1001ba23 -> efw_FailOrNarrate */
        /* call 0x1001ba38 -> efw_Player */
    }

    /* 0x1001baab strcmp(cmd, 'efw_HelpScreen' @ 0x10105494) */
    if (!stricmp(cmd, "efw_HelpScreen")) {
        /* call 0x1001badf -> efw_Player */
        /* call 0x1001bae5 -> efw_FailOrNarrate */
    }

    /* 0x1001bafc strcmp(cmd, 'efw_PickupPliers' @ 0x10105480) */
    if (!stricmp(cmd, "efw_PickupPliers")) {
        /* call 0x1001bb2e -> efw_Player */
        /* call 0x1001bb34 -> FUN_100c59c0 */
        /* call 0x1001bb4c -> efwConversation::Squark */
        /* call 0x1001bb64 -> efw_Player */
        /* call 0x1001bb6b -> FUN_1007f800 */
        /* call 0x1001bb87 -> FUN_100c87c0 */
        /* call 0x1001bba9 -> FUN_100affd0 */
        /* call 0x1001bbb5 -> FUN_100af9c0 */
    }

}


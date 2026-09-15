/* EFW-focused Ghidra decompile of EscapeFromWoomera.dll
 * Names in banners are recovered (see decompile/recovered/NAMES.md).
 * CRT after 0x100c84b0 was stripped. This is not yet recompilable.
 */

/* ==== 1000d1d0 efw_AssignGuardModel ==== */

void __fastcall efw_AssignGuardModel(int *param_1)

{
  int iVar1;
  bool bVar2;
  uint *puVar3;
  char *pcVar4;
  undefined4 uStack_18;
  undefined4 uStack_14;
  undefined4 uStack_10;
  undefined4 uStack_c;
  undefined4 uStack_8;
  undefined4 uStack_4;
  
  (**(code **)(*param_1 + 0x2c))();
  puVar3 = (uint *)(*(int *)(param_1[1] + 0x1cc) + *(int *)(DAT_10122044 + 0x98));
  bVar2 = efw_FStrEq(puVar3,s_Kitchen_Guard_101031e0);
  if (!bVar2) {
    bVar2 = efw_FStrEq(puVar3,s_efw_electrician_101031bc);
    if (bVar2) {
      (*DAT_10121e10)(*(undefined4 *)(param_1[1] + 0x208),s_models_tradesman_mdl_101031a4);
      goto LAB_1000d39b;
    }
    bVar2 = efw_FStrEq(puVar3,s_efw_compound_gate_guard_1010318c);
    if (bVar2) {
      (*DAT_10121e10)(*(undefined4 *)(param_1[1] + 0x208),s_models_security_mdl_101031cc);
      goto LAB_1000d39b;
    }
    bVar2 = efw_FStrEq(puVar3,s_Mail_Officer_1010317c);
    if (!bVar2) {
      bVar2 = efw_FStrEq(puVar3,s_Roster_Officer_1010316c);
      if (bVar2) {
        (*DAT_10121e10)(*(undefined4 *)(param_1[1] + 0x208),s_models_security_mdl_101031cc);
        goto LAB_1000d39b;
      }
      bVar2 = efw_FStrEq(puVar3,s_Patrol_Guard_2_1010315c);
      if (bVar2) {
        (*DAT_10121e10)(*(undefined4 *)(param_1[1] + 0x208),s_models_security_mdl_101031cc);
        goto LAB_1000d39b;
      }
      bVar2 = efw_FStrEq(puVar3,s_Patrol_Guard_3_1010314c);
      if (!bVar2) {
        bVar2 = efw_FStrEq(puVar3,s_Patrolling_Guard_1_10103138);
        if (bVar2) {
          (*DAT_10121e10)(*(undefined4 *)(param_1[1] + 0x208),s_models_security_mdl_101031cc);
          goto LAB_1000d39b;
        }
        bVar2 = efw_FStrEq(puVar3,s_Patrol_Guard_5_10103128);
        if (bVar2) {
          (*DAT_10121e10)(*(undefined4 *)(param_1[1] + 0x208),s_models_security_mdl_101031cc);
          goto LAB_1000d39b;
        }
        bVar2 = efw_FStrEq(puVar3,s_Patrol_Guard_4_10103118);
        if (!bVar2) {
          pcVar4 = s_Model_not_known_for_name___s_101030f8;
          efw_DebugPrint();
          (*DAT_10121e10)(*(undefined4 *)(param_1[1] + 0x208),s_models_security_mdl_101031cc,pcVar4,
                          puVar3);
          goto LAB_1000d39b;
        }
      }
    }
  }
  (*DAT_10121e10)(*(undefined4 *)(param_1[1] + 0x208),s_models_security_mdl_101031cc);
LAB_1000d39b:
  uStack_18 = 0x41800000;
  uStack_14 = 0x41800000;
  uStack_10 = 0x42900000;
  uStack_c = 0xc1800000;
  uStack_8 = 0xc1800000;
  uStack_4 = 0;
  FUN_100afdc0(param_1[1],&uStack_c,&uStack_18);
  *(undefined4 *)(param_1[1] + 0x10c) = 3;
  *(undefined4 *)(param_1[1] + 0x108) = 4;
  param_1[0xb4] = 0xf7;
  if (*(float *)(param_1[1] + 0x160) == 0.0) {
    *(undefined4 *)(param_1[1] + 0x160) = DAT_10126a60;
  }
  iVar1 = param_1[1];
  uStack_18 = 0;
  uStack_14 = 0;
  *(undefined4 *)(iVar1 + 0x174) = 0;
  uStack_10 = 0x42480000;
  *(undefined4 *)(iVar1 + 0x178) = 0;
  *(undefined4 *)(iVar1 + 0x17c) = 0x42480000;
  param_1[0x70] = -0x40cccccd;
  param_1[0x76] = 0;
  *(undefined4 *)(param_1[1] + 0x114) = 0;
  param_1[0xe1] = 0;
  param_1[0xae] = 0x7c0;
  (**(code **)(*param_1 + 0x134))();
  param_1[0x1f] = (int)CTalkMonster::FollowerUse;
  return;
}



/* ==== 1007af20 LinkUserMessages ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void LinkUserMessages(void)

{
  if (DAT_10124868 == 0) {
    DAT_10124868 = (*DAT_10121f34)(s_SelAmmo_10114ce4,4);
    DAT_10124884 = (*DAT_10121f34)(s_CurWeapon_10114cd8,3);
    DAT_101248ec = (*DAT_10121f34)(s_Geiger_10114cd0,1);
    DAT_1012486c = (*DAT_10121f34)(s_Flashlight_10114cc4,2);
    DAT_10124870 = (*DAT_10121f34)(s_FlashBat_10114cb8,1);
    DAT_10124888 = (*DAT_10121f34)(s_Health_10114cb0,1);
    DAT_1012488c = (*DAT_10121f34)(s_Damage_10114ca8,0xc);
    DAT_10124890 = (*DAT_10121f34)(s_Battery_10114ca0,2);
    DAT_10124894 = (*DAT_10121f34)(s_Train_10114c98,1);
    DAT_101248a8 = (*DAT_10121f34)(s_HudText_10114c90,0xffffffff);
    DAT_101248dc = (*DAT_10121f34)(s_SayText_10114c88,0xffffffff);
    DAT_101248e0 = (*DAT_10121f34)(s_TextMsg_10114c80,0xffffffff);
    DAT_101248a0 = (*DAT_10121f34)(s_WeaponList_10114c74,0xffffffff);
    DAT_10124874 = (*DAT_10121f34)(s_ResetHUD_10114c68,1);
    DAT_10124878 = (*DAT_10121f34)(s_InitHUD_10114c60,0);
    DAT_1012487c = (*DAT_10121f34)(s_SetFog_10114c58,9);
    DAT_10124880 = (*DAT_10121f34)(s_GameTitle_10114c4c,1);
    DAT_101248ac = (*DAT_10121f34)(s_DeathMsg_10114c40,0xffffffff);
    DAT_101248b0 = (*DAT_10121f34)(s_ScoreInfo_10114c34,9);
    DAT_101248b4 = (*DAT_10121f34)(s_TeamInfo_10114c28,0xffffffff);
    _DAT_101248b8 = (*DAT_10121f34)(s_TeamScore_10114c1c,0xffffffff);
    DAT_101248bc = (*DAT_10121f34)(s_GameMode_10114c10,1);
    DAT_101248c0 = (*DAT_10121f34)(&DAT_10114c08,0xffffffff);
    DAT_101248c4 = (*DAT_10121f34)(s_ServerName_10114bfc,0xffffffff);
    DAT_101248c8 = (*DAT_10121f34)(s_AmmoPickup_10114bf0,2);
    DAT_101248cc = (*DAT_10121f34)(s_WeapPickup_10114be4,1);
    DAT_101248d0 = (*DAT_10121f34)(s_ItemPickup_10114bd8,0xffffffff);
    DAT_101248d4 = (*DAT_10121f34)(s_HideWeapon_10114bcc,1);
    DAT_101248e4 = (*DAT_10121f34)(s_SetFOV_10114bc4,1);
    _DAT_101248e8 = (*DAT_10121f34)(s_ShowMenu_10114bb8,0xffffffff);
    DAT_10124860 = (*DAT_10121f34)(s_ScreenShake_10114bac,6);
    DAT_10124864 = (*DAT_10121f34)(s_ScreenFade_10114ba0,10);
    DAT_101248a4 = (*DAT_10121f34)(s_AmmoX_10114b98,2);
    DAT_101248f0 = (*DAT_10121f34)(s_TeamNames_10114b8c,0xffffffff);
    DAT_1012489c = (*DAT_10121f34)(s_HUDColor_10114b80,4);
    DAT_101248f4 = (*DAT_10121f34)(s_StatusIcon_10114b74,0xffffffff);
    DAT_101248f8 = (*DAT_10121f34)(s_EFWShow_10114b6c,0xffffffff);
    DAT_101248fc = (*DAT_10121f34)(s_EFWData_10114b64,0xffffffff);
    DAT_10124900 = (*DAT_10121f34)(s_EFW_Menu_10114b58,1);
    DAT_10124904 = (*DAT_10121f34)(s_EFW_Cntxt_10114b4c,0xffffffff);
    DAT_10124908 = (*DAT_10121f34)(s_EFW_CtPrv_10114b40,1);
  }
  return;
}



/* ==== 100b2f80 FUN_100b2f80 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_100b2f80(void)

{
  int iVar1;
  undefined4 *puVar2;
  
  puVar2 = &DAT_10131e18;
  for (iVar1 = 0x160; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  puVar2 = &DAT_10131d10;
  for (iVar1 = 0x40; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  DAT_1013239c = 0;
  FUN_100b0f60(0x1010fa24);
  FUN_100b0f60(0x10108be0);
  FUN_100b0f60(0x1010fa40);
  FUN_100b0f60(0x1010fa30);
  FUN_100b0f60(0x1011ba18);
  FUN_100b2e90(0x10108b84);
  FUN_100b0f60(0x10108b74);
  FUN_100b2e90(0x10111c64);
  FUN_100b2e90(0x1010337c);
  FUN_100b0f60(0x10108bc0);
  FUN_100b0f60(0x1011ba0c);
  FUN_100b2e90(0x10108bb0);
  FUN_100b0f60(0x10108ba4);
  FUN_100b0f60(0x10108b94);
  FUN_100b2e90(0x10103390);
  FUN_100b0f60(0x10108b48);
  FUN_100b2e90(0x101150a4);
  FUN_100b0f60(0x10108b1c);
  FUN_100b2e90(0x10108b3c);
  FUN_100b0f60(0x10108b2c);
  FUN_100b2e90(0x10108b64);
  FUN_100b0f60(0x10108b54);
  FUN_100b2e90(0x101150b4);
  FUN_100b2e90(0x10108af8);
  FUN_100b2e90(0x10108ae8);
  FUN_100b2e90(0x10108b08);
  FUN_100b2e90(0x10108ad8);
  FUN_100b2e90(0x10101b10);
  FUN_100b2e90(0x101054a4);
  FUN_100b2e90(0x101054b8);
  FUN_100b2e90(0x1011b9f8);
  FUN_100b2e90(0x1011b9e0);
  FUN_100b2e90(0x1011b9cc);
  FUN_100b2e90(0x1011b9b4);
  FUN_100b2e90(0x1011b998);
  FUN_100b2e90(0x1011b97c);
  FUN_100b2e90(0x1011b960);
  iVar1 = (**(code **)(*DAT_10121db8 + 0x1c))();
  if (iVar1 != 0) {
    FUN_100b0f60(0x10114db8);
  }
  DAT_10131d04 = (*DAT_10121e08)(s_sprites_zerogxplode_spr_1011b948);
  DAT_10131cf0 = (*DAT_10121e08)(s_sprites_WXplo1_spr_1011b934);
  DAT_10131cf2 = (*DAT_10121e08)(s_sprites_steam1_spr_1011b920);
  DAT_10132398 = (*DAT_10121e08)(s_sprites_bubble_spr_10108020);
  DAT_10131d08 = (*DAT_10121e08)(s_sprites_bloodspray_spr_1011b908);
  DAT_10131cf4 = (*DAT_10121e08)(s_sprites_blood_spr_1011b8f4);
  DAT_10131d06 = (*DAT_10121e08)(PTR_s_sprites_laserbeam_spr_1011b508);
  _DAT_10131e10 = (*DAT_10121e08)(s_sprites_laserdot_spr_10115480);
  (*DAT_10121e08)(s_models_grenade_mdl_10108710);
  (*DAT_10121e08)(s_sprites_explode1_spr_1011b8dc);
  (*DAT_10121e0c)(s_weapons_debris1_wav_1010d2f4);
  (*DAT_10121e0c)(s_weapons_debris2_wav_1010d308);
  (*DAT_10121e0c)(s_weapons_debris3_wav_1010d31c);
  (*DAT_10121e0c)(s_weapons_grenade_hit1_wav_1010d330);
  (*DAT_10121e0c)(s_weapons_grenade_hit2_wav_1010d34c);
  (*DAT_10121e0c)(s_weapons_grenade_hit3_wav_1010d368);
  (*DAT_10121e0c)(s_weapons_bullet_hit1_wav_101170a0);
  (*DAT_10121e0c)(s_weapons_bullet_hit2_wav_10117088);
  (*DAT_10121e0c)(s_items_weapondrop1_wav_1011b8c4);
  return;
}



/* ==== 100b86b0 efwConversation::RegisterDefaults ==== */

void efwConversation_RegisterDefaults(void)

{
  char cVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  uint uVar5;
  uint uVar6;
  void *this;
  char *pcVar7;
  char *pcVar8;
  undefined4 local_48 [2];
  undefined1 local_40 [4];
  char *local_3c;
  undefined1 local_30 [16];
  undefined1 local_20 [32];
  
  FUN_100bacf0(local_40,'\0');
  uVar5 = 0xffffffff;
  pcVar7 = s_efw_compound_gate_guard_1010318c;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  FUN_100bba70(local_40,(undefined4 *)s_efw_compound_gate_guard_1010318c,~uVar5 - 1);
  FUN_100bacf0(local_30,'\0');
  piVar3 = (int *)FUN_100baf70(local_20,local_40,local_30);
  piVar3 = (int *)FUN_100badb0(&DAT_10132418,local_48,piVar3);
  iVar2 = *piVar3;
  FUN_100ba8f0((int)local_20);
  FUN_100bacf0(local_30,'\x01');
  uVar5 = 0xffffffff;
  pcVar7 = s_Gate_Guard_1011c3a8;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  FUN_100bba70((void *)(iVar2 + 0x1c),(undefined4 *)s_Gate_Guard_1011c3a8,~uVar5 - 1);
  FUN_100bacf0(local_40,'\x01');
  FUN_100bacf0(local_40,'\0');
  uVar5 = 0xffffffff;
  pcVar7 = s_efw_electrician_101031bc;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  FUN_100bba70(local_40,(undefined4 *)s_efw_electrician_101031bc,~uVar5 - 1);
  FUN_100bacf0(local_30,'\0');
  piVar3 = (int *)FUN_100baf70(local_20,local_40,local_30);
  piVar3 = (int *)FUN_100badb0(&DAT_10132418,local_48,piVar3);
  iVar2 = *piVar3;
  FUN_100ba8f0((int)local_20);
  FUN_100bacf0(local_30,'\x01');
  uVar5 = 0xffffffff;
  pcVar7 = s_Electrician_1011c39c;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  FUN_100bba70((void *)(iVar2 + 0x1c),(undefined4 *)s_Electrician_1011c39c,~uVar5 - 1);
  FUN_100bacf0(local_40,'\x01');
  FUN_100bacf0(local_40,'\0');
  uVar5 = 0xffffffff;
  pcVar7 = s_detainee_1011c390;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  FUN_100bba70(local_40,(undefined4 *)s_detainee_1011c390,~uVar5 - 1);
  FUN_100bacf0(local_30,'\0');
  piVar3 = (int *)FUN_100baf70(local_20,local_40,local_30);
  piVar3 = (int *)FUN_100badb0(&DAT_10132418,local_48,piVar3);
  iVar2 = *piVar3;
  FUN_100ba8f0((int)local_20);
  FUN_100bacf0(local_30,'\x01');
  uVar5 = 0xffffffff;
  pcVar7 = s_Detainee_1011c384;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  FUN_100bba70((void *)(iVar2 + 0x1c),(undefined4 *)s_Detainee_1011c384,~uVar5 - 1);
  FUN_100bacf0(local_40,'\x01');
  FUN_100bacf0(local_40,'\0');
  uVar5 = 0xffffffff;
  pcVar7 = s_detainee_queue_1011c374;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  uVar5 = ~uVar5 - 1;
  uVar4 = FUN_100bbb30(local_40,uVar5,'\x01');
  if ((char)uVar4 != '\0') {
    pcVar7 = s_detainee_queue_1011c374;
    for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
      *(undefined4 *)local_3c = *(undefined4 *)pcVar7;
      pcVar7 = pcVar7 + 4;
      local_3c = local_3c + 4;
    }
    for (uVar6 = uVar5 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
      *local_3c = *pcVar7;
      pcVar7 = pcVar7 + 1;
      local_3c = local_3c + 1;
    }
    FUN_100bbb10(local_40,uVar5);
  }
  FUN_100bacf0(local_30,'\0');
  piVar3 = (int *)FUN_100baf70(local_20,local_40,local_30);
  piVar3 = (int *)FUN_100badb0(&DAT_10132418,local_48,piVar3);
  iVar2 = *piVar3;
  FUN_100ba8f0((int)local_20);
  FUN_100bacf0(local_30,'\x01');
  uVar5 = 0xffffffff;
  this = (void *)(iVar2 + 0x1c);
  pcVar7 = s_Detainee_in_queue_1011c360;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  uVar5 = ~uVar5 - 1;
  uVar4 = FUN_100bbb30(this,uVar5,'\x01');
  if ((char)uVar4 != '\0') {
    pcVar7 = s_Detainee_in_queue_1011c360;
    pcVar8 = *(char **)(iVar2 + 0x20);
    for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
      *(undefined4 *)pcVar8 = *(undefined4 *)pcVar7;
      pcVar7 = pcVar7 + 4;
      pcVar8 = pcVar8 + 4;
    }
    for (uVar6 = uVar5 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
      *pcVar8 = *pcVar7;
      pcVar7 = pcVar7 + 1;
      pcVar8 = pcVar8 + 1;
    }
    FUN_100bbb10(this,uVar5);
  }
  FUN_100bacf0(local_40,'\x01');
  return;
}



/* ==== 100b8ff0 efwConversation::LoadAll ==== */

void efwConversation_LoadAll(void)

{
  char cVar1;
  int iVar2;
  HANDLE hFindFile;
  undefined4 *puVar3;
  undefined1 *puVar4;
  LPCSTR lpFileName;
  BOOL BVar5;
  uint uVar6;
  int iVar7;
  char *pcVar8;
  _WIN32_FIND_DATAA *p_Var9;
  CHAR *pCVar10;
  char *pcVar11;
  int *piVar12;
  undefined1 uStack_2dd;
  undefined1 auStack_2dc [4];
  int iStack_2d8;
  undefined1 uStack_2c9;
  HANDLE pvStack_2c8;
  char acStack_2c4 [4];
  int iStack_2c0;
  undefined1 auStack_2bc [16];
  char acStack_2ac [4];
  undefined4 uStack_2a8;
  undefined1 auStack_2a4 [16];
  undefined1 auStack_294 [16];
  undefined1 auStack_284 [4];
  char *pcStack_280;
  undefined1 auStack_274 [4];
  LPCSTR pCStack_270;
  undefined1 auStack_264 [16];
  undefined1 auStack_254 [16];
  char local_244 [260];
  _WIN32_FIND_DATAA _Stack_140;
  
  (*DAT_10121f94)(local_244);
  FUN_100bb360(&DAT_101323f8,DAT_101323fc,DAT_10132400);
  piVar12 = DAT_1013242c;
  puVar3 = (undefined4 *)FUN_100bb5a0(&DAT_10132428,&pvStack_2c8);
  FUN_100bb5b0(&DAT_10132428,(undefined4 *)acStack_2c4,(int *)*puVar3,piVar12);
  piVar12 = DAT_1013240c;
  puVar3 = (undefined4 *)FUN_100bb760(&DAT_10132408,(undefined4 *)acStack_2c4);
  FUN_100bb770(&DAT_10132408,&pvStack_2c8,(int *)*puVar3,piVar12);
  auStack_2dc[0] = uStack_2dd;
  FUN_100bacf0(auStack_2dc,'\0');
  uVar6 = 0xffffffff;
  pcVar8 = local_244;
  do {
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    cVar1 = *pcVar8;
    pcVar8 = pcVar8 + 1;
  } while (cVar1 != '\0');
  FUN_100bba70(auStack_2dc,(undefined4 *)local_244,~uVar6 - 1);
  puVar4 = FUN_100ba1f0(auStack_294,auStack_2dc);
  uVar6 = 0xffffffff;
  pcVar8 = s__Conversations___txt_1011c3e4;
  do {
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    cVar1 = *pcVar8;
    pcVar8 = pcVar8 + 1;
  } while (cVar1 != '\0');
  puVar4 = FUN_100be420(puVar4,(undefined4 *)s__Conversations___txt_1011c3e4,~uVar6 - 1);
  FUN_100ba1f0(auStack_274,puVar4);
  FUN_100bacf0(auStack_294,'\x01');
  FUN_100bacf0(auStack_2dc,'\x01');
  p_Var9 = &_Stack_140;
  for (iVar7 = 0x50; iVar7 != 0; iVar7 = iVar7 + -1) {
    p_Var9->dwFileAttributes = 0;
    p_Var9 = (_WIN32_FIND_DATAA *)&p_Var9->ftCreationTime;
  }
  lpFileName = pCStack_270;
  if (pCStack_270 == (LPCSTR)0x0) {
    lpFileName = "";
  }
  pvStack_2c8 = FindFirstFileA(lpFileName,&_Stack_140);
  if (pvStack_2c8 != (HANDLE)0x0) {
    do {
      auStack_2a4[0] = uStack_2dd;
      FUN_100bacf0(auStack_2a4,'\0');
      uVar6 = 0xffffffff;
      pcVar8 = local_244;
      do {
        if (uVar6 == 0) break;
        uVar6 = uVar6 - 1;
        cVar1 = *pcVar8;
        pcVar8 = pcVar8 + 1;
      } while (cVar1 != '\0');
      FUN_100bba70(auStack_2a4,(undefined4 *)local_244,~uVar6 - 1);
      puVar4 = FUN_100ba1f0(auStack_264,auStack_2a4);
      uVar6 = 0xffffffff;
      pcVar8 = s__Conversations__1011c3d4;
      do {
        if (uVar6 == 0) break;
        uVar6 = uVar6 - 1;
        cVar1 = *pcVar8;
        pcVar8 = pcVar8 + 1;
      } while (cVar1 != '\0');
      puVar4 = FUN_100be420(puVar4,(undefined4 *)s__Conversations__1011c3d4,~uVar6 - 1);
      FUN_100ba1f0(auStack_294,puVar4);
      FUN_100bacf0(auStack_264,'\x01');
      puVar4 = FUN_100ba1f0(auStack_254,auStack_294);
      uVar6 = 0xffffffff;
      pCVar10 = _Stack_140.cFileName;
      do {
        if (uVar6 == 0) break;
        uVar6 = uVar6 - 1;
        cVar1 = *pCVar10;
        pCVar10 = pCVar10 + 1;
      } while (cVar1 != '\0');
      puVar4 = FUN_100be420(puVar4,(undefined4 *)_Stack_140.cFileName,~uVar6 - 1);
      FUN_100ba1f0(auStack_284,puVar4);
      FUN_100bacf0(auStack_254,'\x01');
      FUN_100bacf0(auStack_294,'\x01');
      FUN_100bacf0(auStack_2a4,'\x01');
      pcVar8 = pcStack_280;
      if (pcStack_280 == (char *)0x0) {
        pcVar8 = "";
      }
      FUN_100ba1d0(auStack_2bc,&uStack_2dd);
      acStack_2ac[0] = '\0';
      uStack_2a8 = 0;
      FUN_100ba310(auStack_2dc,pcVar8,&uStack_2c9);
      uVar6 = 0xffffffff;
      pcVar11 = &DAT_1011c3d0;
      do {
        if (uVar6 == 0) break;
        uVar6 = uVar6 - 1;
        cVar1 = *pcVar11;
        pcVar11 = pcVar11 + 1;
      } while (cVar1 != '\0');
      iVar7 = FUN_100bac00(auStack_2dc,&DAT_1011c3d0,0xffffffff,~uVar6 - 1);
      if (iVar7 != -1) {
        FUN_100bac70(auStack_2dc);
        iVar2 = iStack_2d8;
        FUN_100bac70(auStack_2dc);
        FUN_100ba3d0(auStack_2dc,iStack_2d8,iVar2 + 1 + iVar7);
      }
      uVar6 = 0xffffffff;
      pcVar11 = &DAT_1011c3cc;
      do {
        if (uVar6 == 0) break;
        uVar6 = uVar6 - 1;
        cVar1 = *pcVar11;
        pcVar11 = pcVar11 + 1;
      } while (cVar1 != '\0');
      iVar7 = FUN_100baba0(auStack_2dc,&DAT_1011c3cc,0,~uVar6 - 1);
      if (iVar7 != -1) {
        FUN_100bac70(auStack_2dc);
        iVar2 = iStack_2d8;
        uVar6 = FUN_100ba4c0(auStack_2dc);
        FUN_100ba3d0(auStack_2dc,iVar2 + iVar7,uVar6);
      }
      FUN_100ba9a0(auStack_2bc,auStack_2dc,0,0xffffffff);
      iStack_2c0 = efwConversation_ParseFile(pcVar8);
      acStack_2c4[0] = iStack_2c0 != 0;
      FUN_100ba550(acStack_2ac,acStack_2c4);
      FUN_100ba6b0(acStack_2c4);
      FUN_100bacf0(auStack_2dc,'\x01');
      FUN_100bb180(&DAT_101323f8,(undefined1 *)DAT_10132400,auStack_2bc);
      FUN_100ba6b0(acStack_2ac);
      FUN_100bacf0(auStack_2bc,'\x01');
      hFindFile = pvStack_2c8;
      BVar5 = FindNextFileA(pvStack_2c8,&_Stack_140);
      if (BVar5 == 0) goto LAB_100b9381;
      FUN_100bacf0(auStack_284,'\x01');
    } while( true );
  }
LAB_100b938f:
  efwConversation_AddKeyword(s_ESCAPE_1011c3c4,1);
  efwConversation_AddKeyword(s_GREET_1011c3bc,1);
  efwConversation_AddKeyword(s_GOODBYE_1011c3b4,1);
  efwConversation_RegisterDefaults();
  if (pCStack_270 != (LPCSTR)0x0) {
    cVar1 = pCStack_270[-1];
    if ((cVar1 != '\0') && (cVar1 != -1)) {
      pCStack_270[-1] = cVar1 + -1;
      return;
    }
    FUN_100c8ce6(pCStack_270 + -1);
  }
  return;
LAB_100b9381:
  FUN_100bacf0(auStack_284,'\x01');
  FindClose(hFindFile);
  goto LAB_100b938f;
}



/* ==== 100ba040 efwConversation::Squark ==== */

void __cdecl efwConversation_Squark(byte *param_1,char *param_2,byte param_3)

{
  uint uVar1;
  
  uVar1 = FUN_100af060(0,param_1,0);
  if (uVar1 == 0) {
    efw_DebugPrint();
    return;
  }
  FUN_100ba080(uVar1,param_2,param_3);
  return;
}



/* ==== 100bfbf0 FUN_100bfbf0 ==== */

void __fastcall FUN_100bfbf0(int param_1)

{
  char cVar1;
  uint uVar2;
  void *this;
  uint uVar3;
  uint uVar4;
  int iVar5;
  byte *pbVar6;
  char *pcVar7;
  byte *pbVar8;
  bool bVar9;
  bool bVar10;
  int iVar11;
  
  uVar3 = 0xffffffff;
  pcVar7 = s_efw_GetPackage_1011c4d8;
  do {
    if (uVar3 == 0) break;
    uVar3 = uVar3 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  uVar2 = *(uint *)(param_1 + 0xc);
  uVar4 = ~uVar3 - 1;
  uVar3 = uVar2;
  if (uVar4 <= uVar2) {
    uVar3 = uVar4;
  }
  bVar9 = false;
  iVar5 = 0;
  bVar10 = true;
  pbVar6 = *(byte **)(param_1 + 8);
  pbVar8 = (byte *)s_efw_GetPackage_1011c4d8;
  do {
    if (uVar3 == 0) break;
    uVar3 = uVar3 - 1;
    bVar9 = *pbVar6 < *pbVar8;
    bVar10 = *pbVar6 == *pbVar8;
    pbVar6 = pbVar6 + 1;
    pbVar8 = pbVar8 + 1;
  } while (bVar10);
  if (!bVar10) {
    iVar5 = (1 - (uint)bVar9) - (uint)(bVar9 != 0);
  }
  if (((iVar5 != 0) || (uVar2 < uVar4)) || (uVar2 != uVar4)) {
    uVar3 = 0xffffffff;
    pcVar7 = s_efw_GetPackage__1011c4c8;
    do {
      if (uVar3 == 0) break;
      uVar3 = uVar3 - 1;
      cVar1 = *pcVar7;
      pcVar7 = pcVar7 + 1;
    } while (cVar1 != '\0');
    uVar2 = *(uint *)(param_1 + 0xc);
    uVar4 = ~uVar3 - 1;
    uVar3 = uVar2;
    if (uVar4 <= uVar2) {
      uVar3 = uVar4;
    }
    bVar9 = false;
    iVar5 = 0;
    bVar10 = true;
    pbVar6 = *(byte **)(param_1 + 8);
    pbVar8 = (byte *)s_efw_GetPackage__1011c4c8;
    do {
      if (uVar3 == 0) break;
      uVar3 = uVar3 - 1;
      bVar9 = *pbVar6 < *pbVar8;
      bVar10 = *pbVar6 == *pbVar8;
      pbVar6 = pbVar6 + 1;
      pbVar8 = pbVar8 + 1;
    } while (bVar10);
    if (!bVar10) {
      iVar5 = (1 - (uint)bVar9) - (uint)(bVar9 != 0);
    }
    if (((iVar5 != 0) || (uVar2 < uVar4)) || (uVar2 != uVar4)) {
      uVar3 = 0xffffffff;
      pcVar7 = s_efw_EndMailPickupMessage_1011c4ac;
      do {
        if (uVar3 == 0) break;
        uVar3 = uVar3 - 1;
        cVar1 = *pcVar7;
        pcVar7 = pcVar7 + 1;
      } while (cVar1 != '\0');
      uVar2 = *(uint *)(param_1 + 0xc);
      uVar4 = ~uVar3 - 1;
      uVar3 = uVar2;
      if (uVar4 <= uVar2) {
        uVar3 = uVar4;
      }
      bVar9 = false;
      iVar5 = 0;
      bVar10 = true;
      pbVar6 = *(byte **)(param_1 + 8);
      pbVar8 = (byte *)s_efw_EndMailPickupMessage_1011c4ac;
      do {
        if (uVar3 == 0) break;
        uVar3 = uVar3 - 1;
        bVar9 = *pbVar6 < *pbVar8;
        bVar10 = *pbVar6 == *pbVar8;
        pbVar6 = pbVar6 + 1;
        pbVar8 = pbVar8 + 1;
      } while (bVar10);
      if (!bVar10) {
        iVar5 = (1 - (uint)bVar9) - (uint)(bVar9 != 0);
      }
      if (((iVar5 != 0) || (uVar2 < uVar4)) || (uVar2 != uVar4)) {
        uVar3 = 0xffffffff;
        pcVar7 = s_efw_EndMailPickupMessage__1011c490;
        do {
          if (uVar3 == 0) break;
          uVar3 = uVar3 - 1;
          cVar1 = *pcVar7;
          pcVar7 = pcVar7 + 1;
        } while (cVar1 != '\0');
        uVar2 = *(uint *)(param_1 + 0xc);
        uVar4 = ~uVar3 - 1;
        uVar3 = uVar2;
        if (uVar4 <= uVar2) {
          uVar3 = uVar4;
        }
        bVar9 = false;
        iVar5 = 0;
        bVar10 = true;
        pbVar6 = *(byte **)(param_1 + 8);
        pbVar8 = (byte *)s_efw_EndMailPickupMessage__1011c490;
        do {
          if (uVar3 == 0) break;
          uVar3 = uVar3 - 1;
          bVar9 = *pbVar6 < *pbVar8;
          bVar10 = *pbVar6 == *pbVar8;
          pbVar6 = pbVar6 + 1;
          pbVar8 = pbVar8 + 1;
        } while (bVar10);
        if (!bVar10) {
          iVar5 = (1 - (uint)bVar9) - (uint)(bVar9 != 0);
        }
        if (iVar5 != 0) {
          return;
        }
        if (uVar2 < uVar4) {
          return;
        }
        if (uVar2 != uVar4) {
          return;
        }
      }
      if (DAT_10132cc4 == 0) {
        return;
      }
      FUN_100c77e0(DAT_10132cc4);
      return;
    }
  }
  this = (void *)efw_Player();
  FUN_1007f800(this,0x1011b960);
  FUN_1007f800(this,0x1011b9e0);
  iVar11 = 0x47;
  iVar5 = efw_Player();
  efw_FailOrNarrate(iVar5,iVar11);
  FUN_100c4d70(10.0);
  return;
}



/* ==== 100c00c0 FUN_100c00c0 ==== */

undefined4 * __thiscall FUN_100c00c0(void *this,uint param_1)

{
  char cVar1;
  uint uVar2;
  undefined1 *puVar3;
  
  uVar2 = *(uint *)((int)this + 8);
  if ((uVar2 < param_1) || (*(int *)((int)this + 4) == 0)) {
    return &DAT_100f7058;
  }
  cVar1 = *(char *)(*(int *)((int)this + 4) + -1);
  if ((cVar1 == '\0') || (cVar1 == -1)) goto LAB_100c0131;
  if (0xfffffffd < uVar2) {
    FUN_100d9d61();
  }
  puVar3 = *(undefined1 **)((int)this + 4);
  if (((puVar3 == (undefined1 *)0x0) || (cVar1 = puVar3[-1], cVar1 == '\0')) || (cVar1 == -1)) {
    if (uVar2 == 0) {
      if (puVar3 != (undefined1 *)0x0) {
        *(undefined4 *)((int)this + 8) = 0;
        *puVar3 = 0;
      }
      goto LAB_100c0131;
    }
    if (uVar2 <= *(uint *)((int)this + 0xc)) goto LAB_100c0131;
  }
  else if (uVar2 == 0) {
    puVar3[-1] = cVar1 + -1;
    *(undefined4 *)((int)this + 4) = 0;
    *(undefined4 *)((int)this + 8) = 0;
    *(undefined4 *)((int)this + 0xc) = 0;
    goto LAB_100c0131;
  }
  FUN_100bcd30(uVar2);
LAB_100c0131:
  if (*(int *)((int)this + 4) != 0) {
    *(undefined1 *)(*(int *)((int)this + 4) + -1) = 0xff;
  }
  return (undefined4 *)(param_1 + *(int *)((int)this + 4));
}



/* ==== 100c0160 FUN_100c0160 ==== */

void __thiscall FUN_100c0160(void *this,uint param_1)

{
  undefined4 *puVar1;
  int iVar2;
  uint uVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  
  if (*(int *)((int)this + 4) == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = *(int *)((int)this + 0xc) - *(int *)((int)this + 4) >> 2;
  }
  if (uVar3 < param_1) {
    uVar3 = param_1;
    if ((int)param_1 < 0) {
      uVar3 = 0;
    }
    puVar4 = operator_new(uVar3 << 2);
    puVar1 = *(undefined4 **)((int)this + 8);
    puVar6 = puVar4;
    for (puVar5 = *(undefined4 **)((int)this + 4); puVar5 != puVar1; puVar5 = puVar5 + 1) {
      if (puVar6 != (undefined4 *)0x0) {
        *puVar6 = *puVar5;
      }
      puVar6 = puVar6 + 1;
    }
    FUN_100c8ce6(*(undefined **)((int)this + 4));
    *(undefined4 **)((int)this + 0xc) = puVar4 + param_1;
    iVar2 = *(int *)((int)this + 4);
    if (iVar2 == 0) {
      *(undefined4 **)((int)this + 4) = puVar4;
      *(undefined4 **)((int)this + 8) = puVar4;
      return;
    }
    *(undefined4 **)((int)this + 4) = puVar4;
    *(undefined4 **)((int)this + 8) = puVar4 + (*(int *)((int)this + 8) - iVar2 >> 2);
  }
  return;
}



/* ==== 100c0200 FUN_100c0200 ==== */

/* WARNING: Removing unreachable block (ram,0x100c0215) */

void __thiscall FUN_100c0200(void *this,undefined4 *param_1,undefined4 *param_2)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  uint uVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  uint uVar7;
  undefined4 *puVar8;
  
  puVar6 = *(undefined4 **)((int)this + 4);
  puVar8 = *(undefined4 **)((int)this + 4);
  uVar7 = (int)param_2 - (int)param_1 >> 2;
  *(undefined4 **)((int)this + 8) = puVar6;
  if ((uint)(*(int *)((int)this + 0xc) - (int)puVar6 >> 2) < uVar7) {
    if ((puVar8 == (undefined4 *)0x0) || (uVar4 = (int)puVar6 - (int)puVar8 >> 2, uVar4 <= uVar7)) {
      uVar4 = uVar7;
    }
    if (puVar8 == (undefined4 *)0x0) {
      iVar1 = 0;
    }
    else {
      iVar1 = (int)puVar6 - (int)puVar8 >> 2;
    }
    iVar1 = iVar1 + uVar4;
    iVar2 = iVar1;
    if (iVar1 < 0) {
      iVar2 = 0;
    }
    puVar3 = operator_new(iVar2 << 2);
    puVar5 = puVar3;
    for (puVar6 = *(undefined4 **)((int)this + 4); puVar6 != puVar8; puVar6 = puVar6 + 1) {
      FUN_100c17b0(puVar5,puVar6);
      puVar5 = puVar5 + 1;
    }
    for (; param_1 != param_2; param_1 = param_1 + 1) {
      FUN_100c17b0(puVar5,param_1);
      puVar5 = puVar5 + 1;
    }
    puVar6 = *(undefined4 **)((int)this + 8);
    if (puVar8 != puVar6) {
      iVar2 = (int)puVar5 - (int)puVar8;
      do {
        FUN_100c17b0((undefined4 *)(iVar2 + (int)puVar8),puVar8);
        puVar8 = puVar8 + 1;
      } while (puVar8 != puVar6);
    }
    FUN_100c1030();
    FUN_100c8ce6(*(undefined **)((int)this + 4));
    *(undefined4 **)((int)this + 0xc) = puVar3 + iVar1;
    iVar1 = FUN_100c03f0((int)this);
    *(undefined4 **)((int)this + 4) = puVar3;
    *(undefined4 **)((int)this + 8) = puVar3 + iVar1 + uVar7;
    return;
  }
  if ((uint)((int)puVar6 - (int)puVar8 >> 2) < uVar7) {
    FUN_100c1040(puVar8,puVar6,puVar8 + uVar7);
    FUN_100c1040(param_1 + ((int)*(undefined4 **)((int)this + 8) - (int)puVar8 >> 2),param_2,
                 *(undefined4 **)((int)this + 8));
    param_2 = param_1 + (*(int *)((int)this + 8) - (int)puVar8 >> 2);
  }
  else {
    if (uVar7 == 0) {
      return;
    }
    FUN_100c1040(puVar6 + -uVar7,puVar6,puVar6);
    puVar6 = *(undefined4 **)((int)this + 8);
    for (puVar5 = puVar6 + -uVar7; puVar8 != puVar5; puVar5 = puVar5 + -1) {
      puVar6 = puVar6 + -1;
      *puVar6 = puVar5[-1];
    }
  }
  FUN_100c16c0(param_1,param_2,puVar8);
  *(uint *)((int)this + 8) = *(int *)((int)this + 8) + uVar7 * 4;
  return;
}



/* ==== 100c03f0 FUN_100c03f0 ==== */

int __fastcall FUN_100c03f0(int param_1)

{
  if (*(int *)(param_1 + 4) == 0) {
    return 0;
  }
  return *(int *)(param_1 + 8) - *(int *)(param_1 + 4) >> 2;
}



/* ==== 100c0410 FUN_100c0410 ==== */

void __thiscall FUN_100c0410(void *this,uint param_1)

{
  undefined4 *puVar1;
  int iVar2;
  uint uVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  
  if (*(int *)((int)this + 4) == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = *(int *)((int)this + 0xc) - *(int *)((int)this + 4) >> 2;
  }
  if (uVar3 < param_1) {
    uVar3 = param_1;
    if ((int)param_1 < 0) {
      uVar3 = 0;
    }
    puVar4 = operator_new(uVar3 << 2);
    puVar1 = *(undefined4 **)((int)this + 8);
    puVar6 = puVar4;
    for (puVar5 = *(undefined4 **)((int)this + 4); puVar5 != puVar1; puVar5 = puVar5 + 1) {
      if (puVar6 != (undefined4 *)0x0) {
        *puVar6 = *puVar5;
      }
      puVar6 = puVar6 + 1;
    }
    FUN_100c8ce6(*(undefined **)((int)this + 4));
    *(undefined4 **)((int)this + 0xc) = puVar4 + param_1;
    iVar2 = *(int *)((int)this + 4);
    if (iVar2 == 0) {
      *(undefined4 **)((int)this + 4) = puVar4;
      *(undefined4 **)((int)this + 8) = puVar4;
      return;
    }
    *(undefined4 **)((int)this + 4) = puVar4;
    *(undefined4 **)((int)this + 8) = puVar4 + (*(int *)((int)this + 8) - iVar2 >> 2);
  }
  return;
}



/* ==== 100c04b0 FUN_100c04b0 ==== */

/* WARNING: Removing unreachable block (ram,0x100c04c5) */

void __thiscall FUN_100c04b0(void *this,undefined4 *param_1,undefined4 *param_2)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  uint uVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  uint uVar7;
  undefined4 *puVar8;
  
  puVar6 = *(undefined4 **)((int)this + 4);
  puVar8 = *(undefined4 **)((int)this + 4);
  uVar7 = (int)param_2 - (int)param_1 >> 2;
  *(undefined4 **)((int)this + 8) = puVar6;
  if ((uint)(*(int *)((int)this + 0xc) - (int)puVar6 >> 2) < uVar7) {
    if ((puVar8 == (undefined4 *)0x0) || (uVar4 = (int)puVar6 - (int)puVar8 >> 2, uVar4 <= uVar7)) {
      uVar4 = uVar7;
    }
    if (puVar8 == (undefined4 *)0x0) {
      iVar1 = 0;
    }
    else {
      iVar1 = (int)puVar6 - (int)puVar8 >> 2;
    }
    iVar1 = iVar1 + uVar4;
    iVar2 = iVar1;
    if (iVar1 < 0) {
      iVar2 = 0;
    }
    puVar3 = operator_new(iVar2 << 2);
    puVar5 = puVar3;
    for (puVar6 = *(undefined4 **)((int)this + 4); puVar6 != puVar8; puVar6 = puVar6 + 1) {
      FUN_100be920(puVar5,puVar6);
      puVar5 = puVar5 + 1;
    }
    for (; param_1 != param_2; param_1 = param_1 + 1) {
      FUN_100be920(puVar5,param_1);
      puVar5 = puVar5 + 1;
    }
    puVar6 = *(undefined4 **)((int)this + 8);
    if (puVar8 != puVar6) {
      iVar2 = (int)puVar5 - (int)puVar8;
      do {
        FUN_100be920((undefined4 *)(iVar2 + (int)puVar8),puVar8);
        puVar8 = puVar8 + 1;
      } while (puVar8 != puVar6);
    }
    FUN_100be6a0();
    FUN_100c8ce6(*(undefined **)((int)this + 4));
    *(undefined4 **)((int)this + 0xc) = puVar3 + iVar1;
    iVar1 = FUN_100be680((int)this);
    *(undefined4 **)((int)this + 4) = puVar3;
    *(undefined4 **)((int)this + 8) = puVar3 + iVar1 + uVar7;
    return;
  }
  if ((uint)((int)puVar6 - (int)puVar8 >> 2) < uVar7) {
    FUN_100be8c0(puVar8,puVar6,puVar8 + uVar7);
    FUN_100be8c0(param_1 + ((int)*(undefined4 **)((int)this + 8) - (int)puVar8 >> 2),param_2,
                 *(undefined4 **)((int)this + 8));
    param_2 = param_1 + (*(int *)((int)this + 8) - (int)puVar8 >> 2);
  }
  else {
    if (uVar7 == 0) {
      return;
    }
    FUN_100be8c0(puVar6 + -uVar7,puVar6,puVar6);
    puVar6 = *(undefined4 **)((int)this + 8);
    for (puVar5 = puVar6 + -uVar7; puVar8 != puVar5; puVar5 = puVar5 + -1) {
      puVar6 = puVar6 + -1;
      *puVar6 = puVar5[-1];
    }
  }
  FUN_100c16f0(param_1,param_2,puVar8);
  *(uint *)((int)this + 8) = *(int *)((int)this + 8) + uVar7 * 4;
  return;
}



/* ==== 100c06a0 FUN_100c06a0 ==== */

void __thiscall FUN_100c06a0(void *this,uint param_1)

{
  undefined4 *puVar1;
  int iVar2;
  uint uVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  
  if (*(int *)((int)this + 4) == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = *(int *)((int)this + 0xc) - *(int *)((int)this + 4) >> 2;
  }
  if (uVar3 < param_1) {
    uVar3 = param_1;
    if ((int)param_1 < 0) {
      uVar3 = 0;
    }
    puVar4 = operator_new(uVar3 << 2);
    puVar1 = *(undefined4 **)((int)this + 8);
    puVar6 = puVar4;
    for (puVar5 = *(undefined4 **)((int)this + 4); puVar5 != puVar1; puVar5 = puVar5 + 1) {
      if (puVar6 != (undefined4 *)0x0) {
        *puVar6 = *puVar5;
      }
      puVar6 = puVar6 + 1;
    }
    FUN_100c8ce6(*(undefined **)((int)this + 4));
    *(undefined4 **)((int)this + 0xc) = puVar4 + param_1;
    iVar2 = *(int *)((int)this + 4);
    if (iVar2 == 0) {
      *(undefined4 **)((int)this + 4) = puVar4;
      *(undefined4 **)((int)this + 8) = puVar4;
      return;
    }
    *(undefined4 **)((int)this + 4) = puVar4;
    *(undefined4 **)((int)this + 8) = puVar4 + (*(int *)((int)this + 8) - iVar2 >> 2);
  }
  return;
}



/* ==== 100c0740 FUN_100c0740 ==== */

/* WARNING: Removing unreachable block (ram,0x100c0755) */

void __thiscall FUN_100c0740(void *this,undefined4 *param_1,undefined4 *param_2)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  uint uVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  uint uVar7;
  undefined4 *puVar8;
  
  puVar6 = *(undefined4 **)((int)this + 4);
  puVar8 = *(undefined4 **)((int)this + 4);
  uVar7 = (int)param_2 - (int)param_1 >> 2;
  *(undefined4 **)((int)this + 8) = puVar6;
  if ((uint)(*(int *)((int)this + 0xc) - (int)puVar6 >> 2) < uVar7) {
    if ((puVar8 == (undefined4 *)0x0) || (uVar4 = (int)puVar6 - (int)puVar8 >> 2, uVar4 <= uVar7)) {
      uVar4 = uVar7;
    }
    if (puVar8 == (undefined4 *)0x0) {
      iVar1 = 0;
    }
    else {
      iVar1 = (int)puVar6 - (int)puVar8 >> 2;
    }
    iVar1 = iVar1 + uVar4;
    iVar2 = iVar1;
    if (iVar1 < 0) {
      iVar2 = 0;
    }
    puVar3 = operator_new(iVar2 << 2);
    puVar5 = puVar3;
    for (puVar6 = *(undefined4 **)((int)this + 4); puVar6 != puVar8; puVar6 = puVar6 + 1) {
      FUN_100c17d0(puVar5,puVar6);
      puVar5 = puVar5 + 1;
    }
    for (; param_1 != param_2; param_1 = param_1 + 1) {
      FUN_100c17d0(puVar5,param_1);
      puVar5 = puVar5 + 1;
    }
    puVar6 = *(undefined4 **)((int)this + 8);
    if (puVar8 != puVar6) {
      iVar2 = (int)puVar5 - (int)puVar8;
      do {
        FUN_100c17d0((undefined4 *)(iVar2 + (int)puVar8),puVar8);
        puVar8 = puVar8 + 1;
      } while (puVar8 != puVar6);
    }
    FUN_100c1210();
    FUN_100c8ce6(*(undefined **)((int)this + 4));
    *(undefined4 **)((int)this + 0xc) = puVar3 + iVar1;
    iVar1 = FUN_100bad90((int)this);
    *(undefined4 **)((int)this + 4) = puVar3;
    *(undefined4 **)((int)this + 8) = puVar3 + iVar1 + uVar7;
    return;
  }
  if ((uint)((int)puVar6 - (int)puVar8 >> 2) < uVar7) {
    FUN_100c1220(puVar8,puVar6,puVar8 + uVar7);
    FUN_100c1220(param_1 + ((int)*(undefined4 **)((int)this + 8) - (int)puVar8 >> 2),param_2,
                 *(undefined4 **)((int)this + 8));
    param_2 = param_1 + (*(int *)((int)this + 8) - (int)puVar8 >> 2);
  }
  else {
    if (uVar7 == 0) {
      return;
    }
    FUN_100c1220(puVar6 + -uVar7,puVar6,puVar6);
    puVar6 = *(undefined4 **)((int)this + 8);
    for (puVar5 = puVar6 + -uVar7; puVar8 != puVar5; puVar5 = puVar5 + -1) {
      puVar6 = puVar6 + -1;
      *puVar6 = puVar5[-1];
    }
  }
  FUN_100c1720(param_1,param_2,puVar8);
  *(uint *)((int)this + 8) = *(int *)((int)this + 8) + uVar7 * 4;
  return;
}



/* ==== 100c0930 FUN_100c0930 ==== */

void __thiscall FUN_100c0930(void *this,uint param_1)

{
  undefined4 *puVar1;
  int iVar2;
  uint uVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  
  if (*(int *)((int)this + 4) == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = *(int *)((int)this + 0xc) - *(int *)((int)this + 4) >> 2;
  }
  if (uVar3 < param_1) {
    uVar3 = param_1;
    if ((int)param_1 < 0) {
      uVar3 = 0;
    }
    puVar4 = operator_new(uVar3 << 2);
    puVar1 = *(undefined4 **)((int)this + 8);
    puVar6 = puVar4;
    for (puVar5 = *(undefined4 **)((int)this + 4); puVar5 != puVar1; puVar5 = puVar5 + 1) {
      if (puVar6 != (undefined4 *)0x0) {
        *puVar6 = *puVar5;
      }
      puVar6 = puVar6 + 1;
    }
    FUN_100c8ce6(*(undefined **)((int)this + 4));
    *(undefined4 **)((int)this + 0xc) = puVar4 + param_1;
    iVar2 = *(int *)((int)this + 4);
    if (iVar2 == 0) {
      *(undefined4 **)((int)this + 4) = puVar4;
      *(undefined4 **)((int)this + 8) = puVar4;
      return;
    }
    *(undefined4 **)((int)this + 4) = puVar4;
    *(undefined4 **)((int)this + 8) = puVar4 + (*(int *)((int)this + 8) - iVar2 >> 2);
  }
  return;
}



/* ==== 100c09d0 FUN_100c09d0 ==== */

/* WARNING: Removing unreachable block (ram,0x100c09e5) */

void __thiscall FUN_100c09d0(void *this,undefined4 *param_1,undefined4 *param_2)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  uint uVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  uint uVar7;
  undefined4 *puVar8;
  
  puVar6 = *(undefined4 **)((int)this + 4);
  puVar8 = *(undefined4 **)((int)this + 4);
  uVar7 = (int)param_2 - (int)param_1 >> 2;
  *(undefined4 **)((int)this + 8) = puVar6;
  if ((uint)(*(int *)((int)this + 0xc) - (int)puVar6 >> 2) < uVar7) {
    if ((puVar8 == (undefined4 *)0x0) || (uVar4 = (int)puVar6 - (int)puVar8 >> 2, uVar4 <= uVar7)) {
      uVar4 = uVar7;
    }
    if (puVar8 == (undefined4 *)0x0) {
      iVar1 = 0;
    }
    else {
      iVar1 = (int)puVar6 - (int)puVar8 >> 2;
    }
    iVar1 = iVar1 + uVar4;
    iVar2 = iVar1;
    if (iVar1 < 0) {
      iVar2 = 0;
    }
    puVar3 = operator_new(iVar2 << 2);
    puVar5 = puVar3;
    for (puVar6 = *(undefined4 **)((int)this + 4); puVar6 != puVar8; puVar6 = puVar6 + 1) {
      FUN_100c17f0(puVar5,puVar6);
      puVar5 = puVar5 + 1;
    }
    for (; param_1 != param_2; param_1 = param_1 + 1) {
      FUN_100c17f0(puVar5,param_1);
      puVar5 = puVar5 + 1;
    }
    puVar6 = *(undefined4 **)((int)this + 8);
    if (puVar8 != puVar6) {
      iVar2 = (int)puVar5 - (int)puVar8;
      do {
        FUN_100c17f0((undefined4 *)(iVar2 + (int)puVar8),puVar8);
        puVar8 = puVar8 + 1;
      } while (puVar8 != puVar6);
    }
    FUN_100bda20();
    FUN_100c8ce6(*(undefined **)((int)this + 4));
    *(undefined4 **)((int)this + 0xc) = puVar3 + iVar1;
    iVar1 = FUN_100c0bc0((int)this);
    *(undefined4 **)((int)this + 4) = puVar3;
    *(undefined4 **)((int)this + 8) = puVar3 + iVar1 + uVar7;
    return;
  }
  if ((uint)((int)puVar6 - (int)puVar8 >> 2) < uVar7) {
    FUN_100c13f0(puVar8,puVar6,puVar8 + uVar7);
    FUN_100c13f0(param_1 + ((int)*(undefined4 **)((int)this + 8) - (int)puVar8 >> 2),param_2,
                 *(undefined4 **)((int)this + 8));
    param_2 = param_1 + (*(int *)((int)this + 8) - (int)puVar8 >> 2);
  }
  else {
    if (uVar7 == 0) {
      return;
    }
    FUN_100c13f0(puVar6 + -uVar7,puVar6,puVar6);
    puVar6 = *(undefined4 **)((int)this + 8);
    for (puVar5 = puVar6 + -uVar7; puVar8 != puVar5; puVar5 = puVar5 + -1) {
      puVar6 = puVar6 + -1;
      *puVar6 = puVar5[-1];
    }
  }
  FUN_100c1750(param_1,param_2,puVar8);
  *(uint *)((int)this + 8) = *(int *)((int)this + 8) + uVar7 * 4;
  return;
}



/* ==== 100c0bc0 FUN_100c0bc0 ==== */

int __fastcall FUN_100c0bc0(int param_1)

{
  if (*(int *)(param_1 + 4) == 0) {
    return 0;
  }
  return *(int *)(param_1 + 8) - *(int *)(param_1 + 4) >> 2;
}



/* ==== 100c0be0 FUN_100c0be0 ==== */

void __thiscall FUN_100c0be0(void *this,uint param_1)

{
  undefined4 *puVar1;
  int iVar2;
  uint uVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  
  if (*(int *)((int)this + 4) == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = *(int *)((int)this + 0xc) - *(int *)((int)this + 4) >> 2;
  }
  if (uVar3 < param_1) {
    uVar3 = param_1;
    if ((int)param_1 < 0) {
      uVar3 = 0;
    }
    puVar4 = operator_new(uVar3 << 2);
    puVar1 = *(undefined4 **)((int)this + 8);
    puVar6 = puVar4;
    for (puVar5 = *(undefined4 **)((int)this + 4); puVar5 != puVar1; puVar5 = puVar5 + 1) {
      if (puVar6 != (undefined4 *)0x0) {
        *puVar6 = *puVar5;
      }
      puVar6 = puVar6 + 1;
    }
    FUN_100c8ce6(*(undefined **)((int)this + 4));
    *(undefined4 **)((int)this + 0xc) = puVar4 + param_1;
    iVar2 = *(int *)((int)this + 4);
    if (iVar2 == 0) {
      *(undefined4 **)((int)this + 4) = puVar4;
      *(undefined4 **)((int)this + 8) = puVar4;
      return;
    }
    *(undefined4 **)((int)this + 4) = puVar4;
    *(undefined4 **)((int)this + 8) = puVar4 + (*(int *)((int)this + 8) - iVar2 >> 2);
  }
  return;
}



/* ==== 100c0c80 FUN_100c0c80 ==== */

/* WARNING: Removing unreachable block (ram,0x100c0c95) */

void __thiscall FUN_100c0c80(void *this,undefined4 *param_1,undefined4 *param_2)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  uint uVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  uint uVar7;
  undefined4 *puVar8;
  
  puVar6 = *(undefined4 **)((int)this + 4);
  puVar8 = *(undefined4 **)((int)this + 4);
  uVar7 = (int)param_2 - (int)param_1 >> 2;
  *(undefined4 **)((int)this + 8) = puVar6;
  if ((uint)(*(int *)((int)this + 0xc) - (int)puVar6 >> 2) < uVar7) {
    if ((puVar8 == (undefined4 *)0x0) || (uVar4 = (int)puVar6 - (int)puVar8 >> 2, uVar4 <= uVar7)) {
      uVar4 = uVar7;
    }
    if (puVar8 == (undefined4 *)0x0) {
      iVar1 = 0;
    }
    else {
      iVar1 = (int)puVar6 - (int)puVar8 >> 2;
    }
    iVar1 = iVar1 + uVar4;
    iVar2 = iVar1;
    if (iVar1 < 0) {
      iVar2 = 0;
    }
    puVar3 = operator_new(iVar2 << 2);
    puVar5 = puVar3;
    for (puVar6 = *(undefined4 **)((int)this + 4); puVar6 != puVar8; puVar6 = puVar6 + 1) {
      FUN_100c1810(puVar5,puVar6);
      puVar5 = puVar5 + 1;
    }
    for (; param_1 != param_2; param_1 = param_1 + 1) {
      FUN_100c1810(puVar5,param_1);
      puVar5 = puVar5 + 1;
    }
    puVar6 = *(undefined4 **)((int)this + 8);
    if (puVar8 != puVar6) {
      iVar2 = (int)puVar5 - (int)puVar8;
      do {
        FUN_100c1810((undefined4 *)(iVar2 + (int)puVar8),puVar8);
        puVar8 = puVar8 + 1;
      } while (puVar8 != puVar6);
    }
    FUN_100c15c0();
    FUN_100c8ce6(*(undefined **)((int)this + 4));
    *(undefined4 **)((int)this + 0xc) = puVar3 + iVar1;
    iVar1 = FUN_100c0e70((int)this);
    *(undefined4 **)((int)this + 4) = puVar3;
    *(undefined4 **)((int)this + 8) = puVar3 + iVar1 + uVar7;
    return;
  }
  if ((uint)((int)puVar6 - (int)puVar8 >> 2) < uVar7) {
    FUN_100c15d0(puVar8,puVar6,puVar8 + uVar7);
    FUN_100c15d0(param_1 + ((int)*(undefined4 **)((int)this + 8) - (int)puVar8 >> 2),param_2,
                 *(undefined4 **)((int)this + 8));
    param_2 = param_1 + (*(int *)((int)this + 8) - (int)puVar8 >> 2);
  }
  else {
    if (uVar7 == 0) {
      return;
    }
    FUN_100c15d0(puVar6 + -uVar7,puVar6,puVar6);
    puVar6 = *(undefined4 **)((int)this + 8);
    for (puVar5 = puVar6 + -uVar7; puVar8 != puVar5; puVar5 = puVar5 + -1) {
      puVar6 = puVar6 + -1;
      *puVar6 = puVar5[-1];
    }
  }
  FUN_100c1780(param_1,param_2,puVar8);
  *(uint *)((int)this + 8) = *(int *)((int)this + 8) + uVar7 * 4;
  return;
}



/* ==== 100c0e70 FUN_100c0e70 ==== */

int __fastcall FUN_100c0e70(int param_1)

{
  if (*(int *)(param_1 + 4) == 0) {
    return 0;
  }
  return *(int *)(param_1 + 8) - *(int *)(param_1 + 4) >> 2;
}



/* ==== 100c0e90 FUN_100c0e90 ==== */

undefined4 * __thiscall FUN_100c0e90(void *this,undefined4 *param_1,undefined4 *param_2)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  uint uVar4;
  undefined4 *puVar5;
  int iVar6;
  undefined4 *puVar7;
  
  puVar5 = *(undefined4 **)((int)this + 8);
  iVar1 = *(int *)((int)this + 4);
  iVar6 = (int)param_1 - iVar1 >> 2;
  if (*(int *)((int)this + 0xc) - (int)puVar5 >> 2 != 0) {
    if ((int)puVar5 - (int)param_1 >> 2 == 0) {
      FUN_100c1040(param_1,puVar5,param_1 + 1);
      FUN_100c1600(*(undefined4 **)((int)this + 8),
                   1 - ((int)*(undefined4 **)((int)this + 8) - (int)param_1 >> 2),param_2);
      puVar5 = *(undefined4 **)((int)this + 8);
      for (; param_1 != puVar5; param_1 = param_1 + 1) {
        *param_1 = *param_2;
      }
    }
    else {
      FUN_100c1040(puVar5 + -1,puVar5,puVar5);
      puVar5 = *(undefined4 **)((int)this + 8);
      puVar7 = puVar5;
      while (param_1 != puVar7 + -1) {
        puVar5 = puVar5 + -1;
        *puVar5 = puVar7[-2];
        puVar7 = puVar7 + -1;
      }
      puVar5 = param_1 + 1;
      for (; param_1 != puVar5; param_1 = param_1 + 1) {
        *param_1 = *param_2;
      }
    }
    *(int *)((int)this + 8) = *(int *)((int)this + 8) + 4;
    return (undefined4 *)(*(int *)((int)this + 4) + iVar6 * 4);
  }
  if ((iVar1 == 0) || (uVar4 = (int)puVar5 - iVar1 >> 2, uVar4 < 2)) {
    uVar4 = 1;
  }
  if (iVar1 == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = (int)puVar5 - iVar1 >> 2;
  }
  iVar1 = iVar1 + uVar4;
  iVar2 = iVar1;
  if (iVar1 < 0) {
    iVar2 = 0;
  }
  puVar3 = operator_new(iVar2 * 4);
  puVar7 = puVar3;
  for (puVar5 = *(undefined4 **)((int)this + 4); puVar5 != param_1; puVar5 = puVar5 + 1) {
    FUN_100c17b0(puVar7,puVar5);
    puVar7 = puVar7 + 1;
  }
  FUN_100c17b0(puVar7,param_2);
  FUN_100c1040(param_1,*(undefined4 **)((int)this + 8),puVar7 + 1);
  FUN_100c1030();
  FUN_100c8ce6(*(undefined **)((int)this + 4));
  *(undefined4 **)((int)this + 0xc) = puVar3 + iVar1;
  iVar1 = FUN_100c03f0((int)this);
  *(undefined4 **)((int)this + 4) = puVar3;
  *(undefined4 **)((int)this + 8) = puVar3 + iVar1 + 1;
  return puVar3 + iVar6;
}



/* ==== 100c1030 FUN_100c1030 ==== */

void FUN_100c1030(void)

{
  return;
}



/* ==== 100c1040 FUN_100c1040 ==== */

undefined4 * FUN_100c1040(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3)

{
  if (param_1 == param_2) {
    return param_3;
  }
  do {
    if (param_3 != (undefined4 *)0x0) {
      *param_3 = *param_1;
    }
    param_1 = param_1 + 1;
    param_3 = param_3 + 1;
  } while (param_1 != param_2);
  return param_3;
}



/* ==== 100c1070 FUN_100c1070 ==== */

undefined4 * __thiscall FUN_100c1070(void *this,undefined4 *param_1,undefined4 *param_2)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  uint uVar4;
  undefined4 *puVar5;
  int iVar6;
  undefined4 *puVar7;
  
  puVar5 = *(undefined4 **)((int)this + 8);
  iVar1 = *(int *)((int)this + 4);
  iVar6 = (int)param_1 - iVar1 >> 2;
  if (*(int *)((int)this + 0xc) - (int)puVar5 >> 2 != 0) {
    if ((int)puVar5 - (int)param_1 >> 2 == 0) {
      FUN_100c1220(param_1,puVar5,param_1 + 1);
      FUN_100c1630(*(undefined4 **)((int)this + 8),
                   1 - ((int)*(undefined4 **)((int)this + 8) - (int)param_1 >> 2),param_2);
      puVar5 = *(undefined4 **)((int)this + 8);
      for (; param_1 != puVar5; param_1 = param_1 + 1) {
        *param_1 = *param_2;
      }
    }
    else {
      FUN_100c1220(puVar5 + -1,puVar5,puVar5);
      puVar5 = *(undefined4 **)((int)this + 8);
      puVar7 = puVar5;
      while (param_1 != puVar7 + -1) {
        puVar5 = puVar5 + -1;
        *puVar5 = puVar7[-2];
        puVar7 = puVar7 + -1;
      }
      puVar5 = param_1 + 1;
      for (; param_1 != puVar5; param_1 = param_1 + 1) {
        *param_1 = *param_2;
      }
    }
    *(int *)((int)this + 8) = *(int *)((int)this + 8) + 4;
    return (undefined4 *)(*(int *)((int)this + 4) + iVar6 * 4);
  }
  if ((iVar1 == 0) || (uVar4 = (int)puVar5 - iVar1 >> 2, uVar4 < 2)) {
    uVar4 = 1;
  }
  if (iVar1 == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = (int)puVar5 - iVar1 >> 2;
  }
  iVar1 = iVar1 + uVar4;
  iVar2 = iVar1;
  if (iVar1 < 0) {
    iVar2 = 0;
  }
  puVar3 = operator_new(iVar2 * 4);
  puVar7 = puVar3;
  for (puVar5 = *(undefined4 **)((int)this + 4); puVar5 != param_1; puVar5 = puVar5 + 1) {
    FUN_100c17d0(puVar7,puVar5);
    puVar7 = puVar7 + 1;
  }
  FUN_100c17d0(puVar7,param_2);
  FUN_100c1220(param_1,*(undefined4 **)((int)this + 8),puVar7 + 1);
  FUN_100c1210();
  FUN_100c8ce6(*(undefined **)((int)this + 4));
  *(undefined4 **)((int)this + 0xc) = puVar3 + iVar1;
  iVar1 = FUN_100bad90((int)this);
  *(undefined4 **)((int)this + 4) = puVar3;
  *(undefined4 **)((int)this + 8) = puVar3 + iVar1 + 1;
  return puVar3 + iVar6;
}



/* ==== 100c1210 FUN_100c1210 ==== */

void FUN_100c1210(void)

{
  return;
}



/* ==== 100c1220 FUN_100c1220 ==== */

undefined4 * FUN_100c1220(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3)

{
  if (param_1 == param_2) {
    return param_3;
  }
  do {
    if (param_3 != (undefined4 *)0x0) {
      *param_3 = *param_1;
    }
    param_1 = param_1 + 1;
    param_3 = param_3 + 1;
  } while (param_1 != param_2);
  return param_3;
}



/* ==== 100c1250 FUN_100c1250 ==== */

undefined4 * __thiscall FUN_100c1250(void *this,undefined4 *param_1,undefined4 *param_2)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  uint uVar4;
  undefined4 *puVar5;
  int iVar6;
  undefined4 *puVar7;
  
  puVar5 = *(undefined4 **)((int)this + 8);
  iVar1 = *(int *)((int)this + 4);
  iVar6 = (int)param_1 - iVar1 >> 2;
  if (*(int *)((int)this + 0xc) - (int)puVar5 >> 2 != 0) {
    if ((int)puVar5 - (int)param_1 >> 2 == 0) {
      FUN_100c13f0(param_1,puVar5,param_1 + 1);
      FUN_100c1660(*(undefined4 **)((int)this + 8),
                   1 - ((int)*(undefined4 **)((int)this + 8) - (int)param_1 >> 2),param_2);
      puVar5 = *(undefined4 **)((int)this + 8);
      for (; param_1 != puVar5; param_1 = param_1 + 1) {
        *param_1 = *param_2;
      }
    }
    else {
      FUN_100c13f0(puVar5 + -1,puVar5,puVar5);
      puVar5 = *(undefined4 **)((int)this + 8);
      puVar7 = puVar5;
      while (param_1 != puVar7 + -1) {
        puVar5 = puVar5 + -1;
        *puVar5 = puVar7[-2];
        puVar7 = puVar7 + -1;
      }
      puVar5 = param_1 + 1;
      for (; param_1 != puVar5; param_1 = param_1 + 1) {
        *param_1 = *param_2;
      }
    }
    *(int *)((int)this + 8) = *(int *)((int)this + 8) + 4;
    return (undefined4 *)(*(int *)((int)this + 4) + iVar6 * 4);
  }
  if ((iVar1 == 0) || (uVar4 = (int)puVar5 - iVar1 >> 2, uVar4 < 2)) {
    uVar4 = 1;
  }
  if (iVar1 == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = (int)puVar5 - iVar1 >> 2;
  }
  iVar1 = iVar1 + uVar4;
  iVar2 = iVar1;
  if (iVar1 < 0) {
    iVar2 = 0;
  }
  puVar3 = operator_new(iVar2 * 4);
  puVar7 = puVar3;
  for (puVar5 = *(undefined4 **)((int)this + 4); puVar5 != param_1; puVar5 = puVar5 + 1) {
    FUN_100c17f0(puVar7,puVar5);
    puVar7 = puVar7 + 1;
  }
  FUN_100c17f0(puVar7,param_2);
  FUN_100c13f0(param_1,*(undefined4 **)((int)this + 8),puVar7 + 1);
  FUN_100bda20();
  FUN_100c8ce6(*(undefined **)((int)this + 4));
  *(undefined4 **)((int)this + 0xc) = puVar3 + iVar1;
  iVar1 = FUN_100c0bc0((int)this);
  *(undefined4 **)((int)this + 4) = puVar3;
  *(undefined4 **)((int)this + 8) = puVar3 + iVar1 + 1;
  return puVar3 + iVar6;
}



/* ==== 100c13f0 FUN_100c13f0 ==== */

undefined4 * FUN_100c13f0(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3)

{
  if (param_1 == param_2) {
    return param_3;
  }
  do {
    if (param_3 != (undefined4 *)0x0) {
      *param_3 = *param_1;
    }
    param_1 = param_1 + 1;
    param_3 = param_3 + 1;
  } while (param_1 != param_2);
  return param_3;
}



/* ==== 100c1420 FUN_100c1420 ==== */

undefined4 * __thiscall FUN_100c1420(void *this,undefined4 *param_1,undefined4 *param_2)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  uint uVar4;
  undefined4 *puVar5;
  int iVar6;
  undefined4 *puVar7;
  
  puVar5 = *(undefined4 **)((int)this + 8);
  iVar1 = *(int *)((int)this + 4);
  iVar6 = (int)param_1 - iVar1 >> 2;
  if (*(int *)((int)this + 0xc) - (int)puVar5 >> 2 != 0) {
    if ((int)puVar5 - (int)param_1 >> 2 == 0) {
      FUN_100c15d0(param_1,puVar5,param_1 + 1);
      FUN_100c1690(*(undefined4 **)((int)this + 8),
                   1 - ((int)*(undefined4 **)((int)this + 8) - (int)param_1 >> 2),param_2);
      puVar5 = *(undefined4 **)((int)this + 8);
      for (; param_1 != puVar5; param_1 = param_1 + 1) {
        *param_1 = *param_2;
      }
    }
    else {
      FUN_100c15d0(puVar5 + -1,puVar5,puVar5);
      puVar5 = *(undefined4 **)((int)this + 8);
      puVar7 = puVar5;
      while (param_1 != puVar7 + -1) {
        puVar5 = puVar5 + -1;
        *puVar5 = puVar7[-2];
        puVar7 = puVar7 + -1;
      }
      puVar5 = param_1 + 1;
      for (; param_1 != puVar5; param_1 = param_1 + 1) {
        *param_1 = *param_2;
      }
    }
    *(int *)((int)this + 8) = *(int *)((int)this + 8) + 4;
    return (undefined4 *)(*(int *)((int)this + 4) + iVar6 * 4);
  }
  if ((iVar1 == 0) || (uVar4 = (int)puVar5 - iVar1 >> 2, uVar4 < 2)) {
    uVar4 = 1;
  }
  if (iVar1 == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = (int)puVar5 - iVar1 >> 2;
  }
  iVar1 = iVar1 + uVar4;
  iVar2 = iVar1;
  if (iVar1 < 0) {
    iVar2 = 0;
  }
  puVar3 = operator_new(iVar2 * 4);
  puVar7 = puVar3;
  for (puVar5 = *(undefined4 **)((int)this + 4); puVar5 != param_1; puVar5 = puVar5 + 1) {
    FUN_100c1810(puVar7,puVar5);
    puVar7 = puVar7 + 1;
  }
  FUN_100c1810(puVar7,param_2);
  FUN_100c15d0(param_1,*(undefined4 **)((int)this + 8),puVar7 + 1);
  FUN_100c15c0();
  FUN_100c8ce6(*(undefined **)((int)this + 4));
  *(undefined4 **)((int)this + 0xc) = puVar3 + iVar1;
  iVar1 = FUN_100c0e70((int)this);
  *(undefined4 **)((int)this + 4) = puVar3;
  *(undefined4 **)((int)this + 8) = puVar3 + iVar1 + 1;
  return puVar3 + iVar6;
}



/* ==== 100c15c0 FUN_100c15c0 ==== */

void FUN_100c15c0(void)

{
  return;
}



/* ==== 100c15d0 FUN_100c15d0 ==== */

undefined4 * FUN_100c15d0(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3)

{
  if (param_1 == param_2) {
    return param_3;
  }
  do {
    if (param_3 != (undefined4 *)0x0) {
      *param_3 = *param_1;
    }
    param_1 = param_1 + 1;
    param_3 = param_3 + 1;
  } while (param_1 != param_2);
  return param_3;
}



/* ==== 100c1600 FUN_100c1600 ==== */

void FUN_100c1600(undefined4 *param_1,int param_2,undefined4 *param_3)

{
  for (; param_2 != 0; param_2 = param_2 + -1) {
    if (param_1 != (undefined4 *)0x0) {
      *param_1 = *param_3;
    }
    param_1 = param_1 + 1;
  }
  return;
}



/* ==== 100c1630 FUN_100c1630 ==== */

void FUN_100c1630(undefined4 *param_1,int param_2,undefined4 *param_3)

{
  for (; param_2 != 0; param_2 = param_2 + -1) {
    if (param_1 != (undefined4 *)0x0) {
      *param_1 = *param_3;
    }
    param_1 = param_1 + 1;
  }
  return;
}



/* ==== 100c1660 FUN_100c1660 ==== */

void FUN_100c1660(undefined4 *param_1,int param_2,undefined4 *param_3)

{
  for (; param_2 != 0; param_2 = param_2 + -1) {
    if (param_1 != (undefined4 *)0x0) {
      *param_1 = *param_3;
    }
    param_1 = param_1 + 1;
  }
  return;
}



/* ==== 100c1690 FUN_100c1690 ==== */

void FUN_100c1690(undefined4 *param_1,int param_2,undefined4 *param_3)

{
  for (; param_2 != 0; param_2 = param_2 + -1) {
    if (param_1 != (undefined4 *)0x0) {
      *param_1 = *param_3;
    }
    param_1 = param_1 + 1;
  }
  return;
}



/* ==== 100c16c0 FUN_100c16c0 ==== */

undefined4 * __cdecl FUN_100c16c0(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3)

{
  undefined4 uVar1;
  
  if (param_1 == param_2) {
    return param_3;
  }
  do {
    uVar1 = *param_1;
    param_1 = param_1 + 1;
    *param_3 = uVar1;
    param_3 = param_3 + 1;
  } while (param_1 != param_2);
  return param_3;
}



/* ==== 100c16f0 FUN_100c16f0 ==== */

undefined4 * __cdecl FUN_100c16f0(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3)

{
  undefined4 uVar1;
  
  if (param_1 == param_2) {
    return param_3;
  }
  do {
    uVar1 = *param_1;
    param_1 = param_1 + 1;
    *param_3 = uVar1;
    param_3 = param_3 + 1;
  } while (param_1 != param_2);
  return param_3;
}



/* ==== 100c1720 FUN_100c1720 ==== */

undefined4 * __cdecl FUN_100c1720(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3)

{
  undefined4 uVar1;
  
  if (param_1 == param_2) {
    return param_3;
  }
  do {
    uVar1 = *param_1;
    param_1 = param_1 + 1;
    *param_3 = uVar1;
    param_3 = param_3 + 1;
  } while (param_1 != param_2);
  return param_3;
}



/* ==== 100c1750 FUN_100c1750 ==== */

undefined4 * __cdecl FUN_100c1750(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3)

{
  undefined4 uVar1;
  
  if (param_1 == param_2) {
    return param_3;
  }
  do {
    uVar1 = *param_1;
    param_1 = param_1 + 1;
    *param_3 = uVar1;
    param_3 = param_3 + 1;
  } while (param_1 != param_2);
  return param_3;
}



/* ==== 100c1780 FUN_100c1780 ==== */

undefined4 * __cdecl FUN_100c1780(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3)

{
  undefined4 uVar1;
  
  if (param_1 == param_2) {
    return param_3;
  }
  do {
    uVar1 = *param_1;
    param_1 = param_1 + 1;
    *param_3 = uVar1;
    param_3 = param_3 + 1;
  } while (param_1 != param_2);
  return param_3;
}



/* ==== 100c17b0 FUN_100c17b0 ==== */

void __cdecl FUN_100c17b0(undefined4 *param_1,undefined4 *param_2)

{
  if (param_1 != (undefined4 *)0x0) {
    *param_1 = *param_2;
  }
  return;
}



/* ==== 100c17d0 FUN_100c17d0 ==== */

void __cdecl FUN_100c17d0(undefined4 *param_1,undefined4 *param_2)

{
  if (param_1 != (undefined4 *)0x0) {
    *param_1 = *param_2;
  }
  return;
}



/* ==== 100c17f0 FUN_100c17f0 ==== */

void __cdecl FUN_100c17f0(undefined4 *param_1,undefined4 *param_2)

{
  if (param_1 != (undefined4 *)0x0) {
    *param_1 = *param_2;
  }
  return;
}



/* ==== 100c1810 FUN_100c1810 ==== */

void __cdecl FUN_100c1810(undefined4 *param_1,undefined4 *param_2)

{
  if (param_1 != (undefined4 *)0x0) {
    *param_1 = *param_2;
  }
  return;
}



/* ==== 100c1dc0 efwConversationFile::efwConversationFile ==== */

void __thiscall efwConversationFile_efwConversationFile(void *this,undefined4 param_1,undefined4 param_2)

{
  *(undefined4 *)((int)this + 0x24) = param_1;
  *(undefined4 *)((int)this + 0x28) = param_2;
  *(undefined ***)this = &PTR_FUN_100f7ed0;
  *(undefined4 *)((int)this + 0x38) = 0;
  *(undefined4 *)((int)this + 0x3c) = 1;
  *(undefined4 *)((int)this + 0x40) = 0;
  *(undefined4 *)((int)this + 0x10) = 0;
  *(undefined4 *)((int)this + 0xc) = 1;
  *(undefined4 *)((int)this + 0x44) = 0;
  *(undefined4 *)((int)this + 0x68) = 0;
  *(undefined4 *)((int)this + 0x6c) = 0;
  *(undefined4 *)((int)this + 0x70) = 0;
  *(undefined4 *)((int)this + 0x1c) = 0;
  *(undefined4 *)((int)this + 0x18) = 0;
  *(undefined4 *)((int)this + 0x20) = 0;
  *(undefined4 *)((int)this + 0x2c) = 0;
  *(undefined4 *)((int)this + 0x50) = 0;
  return;
}



/* ==== 100c1e10 FUN_100c1e10 ==== */

undefined4 * __thiscall FUN_100c1e10(void *this,byte param_1)

{
  FUN_100c1e30(this);
  if ((param_1 & 1) != 0) {
    FUN_100c8ce6(this);
  }
  return this;
}



/* ==== 100c1e30 FUN_100c1e30 ==== */

void __fastcall FUN_100c1e30(undefined4 *param_1)

{
  *param_1 = &PTR_FUN_100f7ed0;
  FUN_100c8ce6((undefined *)param_1[0x14]);
  FUN_100c2480(param_1,(undefined *)param_1[0xb]);
  *param_1 = &PTR_LAB_100f7ef8;
  return;
}



/* ==== 100c1e80 FUN_100c1e80 ==== */

void __thiscall FUN_100c1e80(void *this,int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  if (param_1 != 0) {
    (**(code **)(*(int *)this + 0xc))(*(undefined4 *)((int)this + 0x2c));
    iVar1 = *(int *)this;
    uVar2 = (**(code **)(iVar1 + 8))(param_1,0x4000);
    (**(code **)(iVar1 + 4))(uVar2);
  }
  if (param_2 != 0) {
    *(int *)((int)this + 0x28) = param_2;
  }
  return;
}



/* ==== 100c1ec0 FUN_100c1ec0 ==== */

int __thiscall FUN_100c1ec0(void *this,undefined1 *param_1)

{
  int *this_00;
  uint uVar1;
  
  this_00 = *(int **)((int)this + 0x24);
  if ((*(byte *)(*(int *)(*this_00 + 4) + 8 + (int)this_00) & 7) == 0) {
    FUN_100da305(this_00,param_1);
    uVar1 = *(uint *)(*(int *)(**(int **)((int)this + 0x24) + 4) + 8 +
                     (int)*(int **)((int)this + 0x24));
    if ((uVar1 & 1) == 0) {
      return (-(uint)((uVar1 & 4) != 0) & 0xfffffffe) + 1;
    }
  }
  return 0;
}



/* ==== 100c1f20 FUN_100c1f20 ==== */

char __fastcall FUN_100c1f20(int *param_1)

{
  int *piVar1;
  byte *pbVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  int iVar7;
  int iVar8;
  undefined1 *puVar9;
  char cVar10;
  undefined1 *puVar11;
  
  puVar9 = (undefined1 *)param_1[1];
  puVar11 = *(undefined1 **)(param_1[0xb] + 4);
  if (puVar11 + param_1[0xd] + 1 < (undefined1 *)param_1[0xe]) {
    (**(code **)(*param_1 + 0x24))(s_fatal_flex_scanner_internal_erro_1011c568);
  }
  if (*(int *)(param_1[0xb] + 0x20) == 0) {
    return (param_1[0xe] - param_1[1] != 1) + '\x01';
  }
  iVar8 = (param_1[0xe] - param_1[1]) + -1;
  iVar4 = iVar8;
  if (0 < iVar8) {
    do {
      *puVar11 = *puVar9;
      puVar11 = puVar11 + 1;
      puVar9 = puVar9 + 1;
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
  }
  iVar4 = param_1[0xb];
  if (*(int *)(iVar4 + 0x24) == 2) {
    param_1[0xd] = 0;
  }
  else {
    iVar5 = *(int *)(iVar4 + 0xc) - iVar8;
    while (iVar5 = iVar5 + -1, iVar5 < 1) {
      pbVar2 = *(byte **)(iVar4 + 4);
      iVar5 = param_1[0xe];
      if (*(int *)(iVar4 + 0x14) == 0) {
        *(undefined4 *)(iVar4 + 4) = 0;
      }
      else {
        uVar3 = *(uint *)(iVar4 + 0xc);
        iVar7 = uVar3 * 2;
        if (iVar7 < 1) {
          iVar7 = (uVar3 >> 3) + uVar3;
        }
        *(int *)(iVar4 + 0xc) = iVar7;
        uVar6 = FUN_100c2560(pbVar2,(uint *)(iVar7 + 2));
        *(undefined4 *)(iVar4 + 4) = uVar6;
      }
      if (*(int *)(iVar4 + 4) == 0) {
        (**(code **)(*param_1 + 0x24))(s_fatal_error___scanner_input_buff_1011c53c);
      }
      piVar1 = (int *)(iVar4 + 4);
      iVar4 = param_1[0xb];
      param_1[0xe] = (iVar5 - (int)pbVar2) + *piVar1;
      iVar5 = *(int *)(iVar4 + 0xc) - iVar8;
    }
    if (0x2000 < iVar5) {
      iVar5 = 0x2000;
    }
    iVar4 = (**(code **)(*param_1 + 0x1c))(*(int *)(param_1[0xb] + 4) + iVar8,iVar5);
    param_1[0xd] = iVar4;
    if (iVar4 < 0) {
      (**(code **)(*param_1 + 0x24))(s_input_in_flex_scanner_failed_1011c51c);
    }
  }
  if (param_1[0xd] == 0) {
    if (iVar8 == 0) {
      cVar10 = '\x01';
      (**(code **)(*param_1 + 0x10))(param_1[9]);
    }
    else {
      cVar10 = '\x02';
      *(undefined4 *)(param_1[0xb] + 0x24) = 2;
    }
  }
  else {
    cVar10 = '\0';
  }
  iVar4 = param_1[0xd];
  param_1[0xd] = iVar4 + iVar8;
  *(undefined1 *)(*(int *)(param_1[0xb] + 4) + iVar4 + iVar8) = 0;
  *(undefined1 *)(*(int *)(param_1[0xb] + 4) + 1 + param_1[0xd]) = 0;
  param_1[1] = *(int *)(param_1[0xb] + 4);
  return cVar10;
}



/* ==== 100c20a0 FUN_100c20a0 ==== */

void __fastcall FUN_100c20a0(int param_1)

{
  int iVar1;
  byte *pbVar2;
  int iVar3;
  uint local_4;
  
  pbVar2 = *(byte **)(param_1 + 4);
  iVar1 = *(int *)(param_1 + 0x40);
  if (pbVar2 < *(byte **)(param_1 + 0x38)) {
    do {
      if (*pbVar2 == 0) {
        local_4 = 1;
      }
      else {
        local_4 = (uint)(byte)(&DAT_100f75b8)[(uint)*pbVar2 * 4];
      }
      iVar3 = iVar1 * 2;
      if (*(short *)(&DAT_100f74f4 + iVar3) != 0) {
        *(int *)(param_1 + 0x48) = iVar1;
        *(byte **)(param_1 + 0x4c) = pbVar2;
      }
      if (*(short *)(&DAT_100f7d64 + ((int)*(short *)(&DAT_100f7a68 + iVar3) + local_4) * 2) !=
          iVar1) {
        do {
          iVar1 = (int)*(short *)(&DAT_100f7b30 + iVar3);
          if (0x61 < iVar1) {
            local_4 = (uint)(byte)(&DAT_100f79b8)[local_4 * 4];
          }
          iVar3 = iVar1 * 2;
        } while (*(short *)(&DAT_100f7d64 + ((int)*(short *)(&DAT_100f7a68 + iVar3) + local_4) * 2)
                 != iVar1);
      }
      pbVar2 = pbVar2 + 1;
      iVar1 = (int)*(short *)(&DAT_100f7bf8 +
                             ((int)*(short *)(&DAT_100f7a68 + iVar1 * 2) + local_4) * 2);
    } while (pbVar2 < *(byte **)(param_1 + 0x38));
  }
  return;
}



/* ==== 100c2170 FUN_100c2170 ==== */

uint __thiscall FUN_100c2170(void *this,int param_1)

{
  int iVar1;
  uint local_4;
  
  iVar1 = param_1 * 2;
  if (*(short *)(&DAT_100f74f4 + iVar1) != 0) {
    *(int *)((int)this + 0x48) = param_1;
    *(undefined4 *)((int)this + 0x4c) = *(undefined4 *)((int)this + 0x38);
  }
  if (*(short *)(&DAT_100f7d66 + *(short *)(&DAT_100f7a68 + iVar1) * 2) == param_1) {
    local_4 = 1;
  }
  else {
    local_4 = 1;
    do {
      param_1 = (int)*(short *)(&DAT_100f7b30 + iVar1);
      if (0x61 < param_1) {
        local_4 = (uint)(byte)(&DAT_100f79b8)[local_4 * 4];
      }
      iVar1 = param_1 * 2;
    } while (*(short *)(&DAT_100f7d64 + ((int)*(short *)(&DAT_100f7a68 + iVar1) + local_4) * 2) !=
             param_1);
  }
  return -(uint)((int)*(short *)(&DAT_100f7bf8 +
                                ((int)*(short *)(&DAT_100f7a68 + param_1 * 2) + local_4) * 2) !=
                0x61) &
         (int)*(short *)(&DAT_100f7bf8 +
                        ((int)*(short *)(&DAT_100f7a68 + param_1 * 2) + local_4) * 2);
}



/* ==== 100c2220 FUN_100c2220 ==== */

void __thiscall FUN_100c2220(void *this,undefined1 param_1,int param_2)

{
  undefined1 *puVar1;
  undefined1 uVar2;
  uint uVar3;
  uint uVar4;
  undefined1 *puVar5;
  undefined1 *puVar6;
  
  puVar6 = *(undefined1 **)((int)this + 0x38);
  *puVar6 = *(undefined1 *)((int)this + 0x30);
  uVar3 = *(uint *)(*(int *)((int)this + 0x2c) + 4);
  if (puVar6 < (undefined1 *)(uVar3 + 2)) {
    puVar5 = (undefined1 *)(*(int *)(*(int *)((int)this + 0x2c) + 0xc) + 2 + uVar3);
    uVar4 = uVar3 + 2 + *(int *)((int)this + 0x34);
    if (uVar3 < uVar4) {
      do {
        puVar1 = (undefined1 *)(uVar4 - 1);
        uVar4 = uVar4 - 1;
        puVar5 = puVar5 + -1;
        *puVar5 = *puVar1;
      } while (*(uint *)(*(int *)((int)this + 0x2c) + 4) < uVar4);
    }
    puVar6 = puVar6 + ((int)puVar5 - uVar4);
    param_2 = param_2 + ((int)puVar5 - uVar4);
    *(undefined4 *)((int)this + 0x34) = *(undefined4 *)(*(int *)((int)this + 0x2c) + 0xc);
    if (puVar6 < (undefined1 *)(*(int *)(*(int *)((int)this + 0x2c) + 4) + 2)) {
      (**(code **)(*(int *)this + 0x24))(s_flex_scanner_push_back_overflow_1011c5a0);
    }
  }
  puVar6 = puVar6 + -1;
  *puVar6 = param_1;
  *(int *)((int)this + 4) = param_2;
  uVar2 = *puVar6;
  *(undefined1 **)((int)this + 0x38) = puVar6;
  *(undefined1 *)((int)this + 0x30) = uVar2;
  return;
}



/* ==== 100c22b0 FUN_100c22b0 ==== */

uint __fastcall FUN_100c22b0(int *param_1)

{
  byte bVar1;
  char *pcVar2;
  char cVar3;
  undefined3 extraout_var;
  int iVar4;
  
  *(char *)param_1[0xe] = (char)param_1[0xc];
  pcVar2 = (char *)param_1[0xe];
  cVar3 = *pcVar2;
  do {
    if (cVar3 != '\0') {
LAB_100c2341:
      bVar1 = *(byte *)param_1[0xe];
      *(byte *)param_1[0xe] = 0;
      iVar4 = param_1[0xe];
      param_1[0xe] = iVar4 + 1;
      *(undefined1 *)(param_1 + 0xc) = *(undefined1 *)(iVar4 + 1);
      return (uint)bVar1;
    }
    if (pcVar2 < (char *)(*(int *)(param_1[0xb] + 4) + param_1[0xd])) {
      *(undefined1 *)param_1[0xe] = 0;
      goto LAB_100c2341;
    }
    param_1[1] = (int)pcVar2;
    param_1[0xe] = (int)(pcVar2 + 1);
    cVar3 = FUN_100c1f20(param_1);
    iVar4 = CONCAT31(extraout_var,cVar3);
    if (iVar4 == 0) {
      param_1[0xe] = param_1[1];
      goto LAB_100c2341;
    }
    if (iVar4 != 1) {
      if (iVar4 == 2) {
        (**(code **)(*param_1 + 0x24))(s_unexpected_last_match_in_yyinput_1011c5c0);
      }
      goto LAB_100c2341;
    }
    iVar4 = FUN_100c27b0();
    if (iVar4 != 0) {
      param_1[0xe] = param_1[1];
      return 0xffffffff;
    }
    if (param_1[0x11] == 0) {
      (**(code **)(*param_1 + 0x10))(param_1[9]);
    }
    *(char *)param_1[0xe] = (char)param_1[0xc];
    pcVar2 = (char *)param_1[0xe];
    cVar3 = *pcVar2;
  } while( true );
}



/* ==== 100c2360 FUN_100c2360 ==== */

void __thiscall FUN_100c2360(void *this,undefined4 param_1)

{
  undefined4 uVar1;
  
  if (*(int *)((int)this + 0x2c) == 0) {
    uVar1 = (**(code **)(*(int *)this + 8))(*(undefined4 *)((int)this + 0x24),0x4000);
    *(undefined4 *)((int)this + 0x2c) = uVar1;
  }
  FUN_100c24c0(this,*(undefined4 **)((int)this + 0x2c),param_1);
  FUN_100c23f0((int)this);
  return;
}



/* ==== 100c23a0 FUN_100c23a0 ==== */

void __thiscall FUN_100c23a0(void *this,int param_1)

{
  if (*(int *)((int)this + 0x2c) != param_1) {
    if (*(int *)((int)this + 0x2c) != 0) {
      **(undefined1 **)((int)this + 0x38) = *(undefined1 *)((int)this + 0x30);
      *(undefined4 *)(*(int *)((int)this + 0x2c) + 8) = *(undefined4 *)((int)this + 0x38);
      *(undefined4 *)(*(int *)((int)this + 0x2c) + 0x10) = *(undefined4 *)((int)this + 0x34);
    }
    *(int *)((int)this + 0x2c) = param_1;
    FUN_100c23f0((int)this);
    *(undefined4 *)((int)this + 0x44) = 1;
  }
  return;
}



/* ==== 100c23f0 FUN_100c23f0 ==== */

void __fastcall FUN_100c23f0(int param_1)

{
  undefined4 *puVar1;
  undefined1 *puVar2;
  
  puVar1 = *(undefined4 **)(param_1 + 0x2c);
  *(undefined4 *)(param_1 + 0x34) = puVar1[4];
  puVar2 = (undefined1 *)puVar1[2];
  *(undefined1 **)(param_1 + 0x38) = puVar2;
  *(undefined1 **)(param_1 + 4) = puVar2;
  *(undefined4 *)(param_1 + 0x24) = *puVar1;
  *(undefined1 *)(param_1 + 0x30) = *puVar2;
  return;
}



/* ==== 100c2410 FUN_100c2410 ==== */

undefined4 * __thiscall FUN_100c2410(void *this,undefined4 param_1,int param_2)

{
  undefined4 *puVar1;
  int iVar2;
  
  puVar1 = (undefined4 *)FUN_100c2550(0x28);
  if (puVar1 == (undefined4 *)0x0) {
    (**(code **)(*(int *)this + 0x24))(s_out_of_dynamic_memory_in_yy_crea_1011c5e4);
  }
  puVar1[3] = param_2;
  iVar2 = FUN_100c2550(param_2 + 2);
  puVar1[1] = iVar2;
  if (iVar2 == 0) {
    (**(code **)(*(int *)this + 0x24))(s_out_of_dynamic_memory_in_yy_crea_1011c5e4);
  }
  puVar1[5] = 1;
  FUN_100c24c0(this,puVar1,param_1);
  return puVar1;
}



/* ==== 100c2480 FUN_100c2480 ==== */

void __thiscall FUN_100c2480(void *this,undefined *param_1)

{
  if (param_1 != (undefined *)0x0) {
    if (param_1 == *(undefined **)((int)this + 0x2c)) {
      *(undefined4 *)((int)this + 0x2c) = 0;
    }
    if (*(int *)(param_1 + 0x14) != 0) {
      FUN_100c2580(*(undefined **)(param_1 + 4));
    }
    FUN_100c2580(param_1);
  }
  return;
}



/* ==== 100c24c0 FUN_100c24c0 ==== */

void __thiscall FUN_100c24c0(void *this,undefined4 *param_1,undefined4 param_2)

{
  FUN_100c24f0(this,(int)param_1);
  param_1[8] = 1;
  *param_1 = param_2;
  param_1[6] = 0;
  return;
}



/* ==== 100c24f0 FUN_100c24f0 ==== */

void __thiscall FUN_100c24f0(void *this,int param_1)

{
  *(undefined4 *)(param_1 + 0x10) = 0;
  **(undefined1 **)(param_1 + 4) = 0;
  *(undefined1 *)(*(int *)(param_1 + 4) + 1) = 0;
  *(undefined4 *)(param_1 + 8) = *(undefined4 *)(param_1 + 4);
  *(undefined4 *)(param_1 + 0x1c) = 1;
  *(undefined4 *)(param_1 + 0x24) = 0;
  if (param_1 == *(int *)((int)this + 0x2c)) {
    FUN_100c23f0((int)this);
  }
  return;
}



/* ==== 100c2550 FUN_100c2550 ==== */

void __cdecl FUN_100c2550(size_t param_1)

{
  _malloc(param_1);
  return;
}



/* ==== 100c2560 FUN_100c2560 ==== */

void __cdecl FUN_100c2560(byte *param_1,uint *param_2)

{
  FUN_100ca1f2(param_1,param_2);
  return;
}



/* ==== 100c2580 FUN_100c2580 ==== */

void __cdecl FUN_100c2580(undefined *param_1)

{
  FUN_100c8cf1(param_1);
  return;
}



/* ==== 100c25a0 FUN_100c25a0 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_100c25a0(undefined4 param_1)

{
  undefined1 local_1;
  
  local_1 = (undefined1)((uint)param_1 >> 0x18);
  DAT_10132450 = local_1;
  DAT_10132454 = 0;
  _DAT_10132458 = 0;
  DAT_1013245c = 0;
  return;
}



/* ==== 100c25c0 FUN_100c25c0 ==== */

void FUN_100c25c0(void)

{
  FUN_100c89be(&LAB_100c25d0);
  return;
}



/* ==== 100c2620 FUN_100c2620 ==== */

void FUN_100c2620(void)

{
  FUN_100c9014((byte *)s_ERROR___s__line___i__1011c610);
  return;
}



/* ==== 100c2640 FUN_100c2640 ==== */

void FUN_100c2640(int param_1)

{
  DAT_10132468[5] = param_1;
                    /* WARNING: Could not recover jumptable at 0x100c2654. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*DAT_10132468 + 0x14))();
  return;
}



/* ==== 100c2660 efwConversation::ParseFile ==== */

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __cdecl efwConversation_ParseFile(char *param_1)

{
  char cVar1;
  undefined4 uVar2;
  void *this;
  uint uVar3;
  uint uVar4;
  char *pcVar5;
  int local_5c [2];
  int iStack_54;
  undefined1 local_50 [80];
  
  FUN_100dae51();
  if (*(int *)((int)&iStack_54 + *(int *)(local_5c[0] + 4)) != 0) {
    FUN_100db020((int)local_50);
    FUN_100dac95((undefined4 *)local_50);
    return 0;
  }
  uVar3 = 0xffffffff;
  DAT_10132464 = 0;
  pcVar5 = param_1;
  do {
    if (uVar3 == 0) break;
    uVar3 = uVar3 - 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar5 + 1;
  } while (cVar1 != '\0');
  uVar4 = ~uVar3 - 1;
  uVar3 = FUN_100bcd20();
  if (uVar3 < uVar4) {
    FUN_100d9d61();
  }
  if (((DAT_10132454 == (char *)0x0) || (cVar1 = DAT_10132454[-1], cVar1 == '\0')) || (cVar1 == -1))
  {
    if (uVar4 == 0) {
      FUN_100bacf0(&DAT_10132450,'\x01');
      goto LAB_100c2759;
    }
    if ((0x1f < DAT_1013245c) || (DAT_1013245c < uVar4)) {
      FUN_100bacf0(&DAT_10132450,'\x01');
      goto LAB_100c2728;
    }
  }
  else {
    if (uVar4 == 0) {
      DAT_10132454[-1] = cVar1 + -1;
      FUN_100bacf0(&DAT_10132450,'\0');
      goto LAB_100c2759;
    }
LAB_100c2728:
    FUN_100bcd30(uVar4);
  }
  pcVar5 = DAT_10132454;
  for (uVar3 = uVar4 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
    *(undefined4 *)pcVar5 = *(undefined4 *)param_1;
    param_1 = param_1 + 4;
    pcVar5 = pcVar5 + 4;
  }
  for (uVar3 = uVar4 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
    *pcVar5 = *param_1;
    param_1 = param_1 + 1;
    pcVar5 = pcVar5 + 1;
  }
  _DAT_10132458 = uVar4;
  DAT_10132454[uVar4] = '\0';
LAB_100c2759:
  this = operator_new(0x74);
  if (this == (void *)0x0) {
    DAT_10132468 = 0;
  }
  else {
    DAT_10132468 = FUN_100c1dc0(this,local_5c,0);
  }
  FUN_100be970();
  uVar2 = DAT_10132460;
  FUN_100db020((int)local_50);
  FUN_100dac95((undefined4 *)local_50);
  return uVar2;
}



/* ==== 100c27b0 FUN_100c27b0 ==== */

undefined4 FUN_100c27b0(void)

{
  return 1;
}



/* ==== 100c27f0 FUN_100c27f0 ==== */

void FUN_100c27f0(void)

{
  undefined1 uVar1;
  undefined3 extraout_var;
  int iVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  
  uVar1 = FUN_100c5b80();
  iVar2 = CONCAT31(extraout_var,uVar1);
  if (((iVar2 != 0) && (iVar2 != 1)) && (iVar2 == 2)) {
    iVar2 = FUN_100af030(0,(byte *)s_efw_IDTag_Position_1011c664);
    if (iVar2 != 0) {
      iVar3 = (*DAT_10121e64)((int)s_weapon_efw_IDTag_1011b9cc - *(int *)(DAT_10122044 + 0x98));
      if (iVar3 != 0) {
        iVar4 = (*DAT_10121f20)(iVar3);
        if (iVar4 != 0) {
          iVar3 = iVar3 + 0x80;
          if (iVar3 == 0) {
            iVar4 = (*DAT_10121e5c)();
            if (iVar4 != 0) {
              iVar3 = iVar4 + 0x80;
            }
          }
          iVar4 = *(int *)(iVar3 + 0x208);
          if ((iVar4 == 0) || (piVar5 = *(int **)(iVar4 + 0x7c), piVar5 == (int *)0x0)) {
            piVar5 = (int *)(*DAT_10121f04)(iVar4,0x130);
            if (piVar5 != (int *)0x0) {
              FUN_100c43f0(piVar5,s_models_w_IDTag_mdl_1011c628,s_models_v_IDTag_mdl_1011c63c,
                           s_models_p_IDTag_mdl_1011c650);
              *piVar5 = (int)&PTR_LAB_100f7f14;
              piVar5[1] = iVar3;
              (**(code **)(*piVar5 + 0x1a8))(iVar2);
              return;
            }
            piVar5 = (int *)0x0;
            iRam00000004 = iVar3;
          }
          (**(code **)(*piVar5 + 0x1a8))(iVar2);
        }
      }
    }
  }
  return;
}



/* ==== 100c29f0 FUN_100c29f0 ==== */

void __thiscall FUN_100c29f0(void *this,int *param_1)

{
  DWORD DVar1;
  
  DVar1 = GetTickCount();
  if (*(uint *)((int)this + 300) < DVar1) {
    FUN_100c46a0(this,param_1);
    efwConversation_AddKeyword(s_Player_sIDTagOnFence_1011c678,0);
  }
  return;
}



/* ==== 100c2a20 FUN_100c2a20 ==== */

void __thiscall FUN_100c2a20(void *this,int param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  byte bVar5;
  int iVar6;
  byte *pbVar7;
  DWORD DVar8;
  int iVar9;
  char *pcVar10;
  bool bVar11;
  
  iVar9 = *(int *)(*(int *)(param_1 + 4) + 0x208);
  if (iVar9 == 0) {
    iVar9 = 0;
  }
  else {
    iVar9 = iVar9 + 0x80;
  }
  pcVar10 = s_efw_IDTag_Position_1011c664;
  pbVar7 = (byte *)(*(int *)(DAT_10122044 + 0x98) + *(int *)(iVar9 + 0x1cc));
  do {
    bVar5 = *pbVar7;
    bVar11 = bVar5 < (byte)*pcVar10;
    if (bVar5 != *pcVar10) {
LAB_100c2a83:
      iVar9 = (1 - (uint)bVar11) - (uint)(bVar11 != 0);
      goto LAB_100c2a88;
    }
    if (bVar5 == 0) break;
    bVar5 = pbVar7[1];
    bVar11 = bVar5 < (byte)pcVar10[1];
    if (bVar5 != pcVar10[1]) goto LAB_100c2a83;
    pbVar7 = pbVar7 + 2;
    pcVar10 = pcVar10 + 2;
  } while (bVar5 != 0);
  iVar9 = 0;
LAB_100c2a88:
  if (iVar9 == 0) {
    FUN_100b0fe0((byte *)s_ID_Tag_has_been_placed_on_the_wa_1011c6ac);
    iVar9 = *(int *)(param_1 + 4);
    fVar1 = *(float *)(iVar9 + 0xd4);
    fVar2 = *(float *)(iVar9 + 200);
    fVar3 = *(float *)(iVar9 + 0xd8);
    fVar4 = *(float *)(iVar9 + 0xcc);
    iVar6 = *(int *)((int)this + 4);
    *(float *)(iVar6 + 8) = (*(float *)(iVar9 + 0xd0) + *(float *)(iVar9 + 0xc4)) * 0.5;
    *(float *)(iVar6 + 0xc) = (fVar1 + fVar2) * 0.5;
    *(float *)(iVar6 + 0x10) = (fVar3 + fVar4) * 0.5;
    *(undefined4 *)(*(int *)((int)this + 4) + 0x50) = 0;
    *(undefined4 *)(*(int *)((int)this + 4) + 0x54) = 0;
    *(undefined4 *)(*(int *)((int)this + 4) + 0x58) = 0;
    *(uint *)(*(int *)((int)this + 4) + 0x118) =
         *(uint *)(*(int *)((int)this + 4) + 0x118) & 0xffffff7f;
    *(undefined4 *)(*(int *)((int)this + 4) + 0x108) = 0;
    *(undefined4 *)(*(int *)((int)this + 4) + 0x198) = 0;
    *(undefined4 *)(*(int *)((int)this + 4) + 0x194) = 0;
    *(undefined4 *)(*(int *)((int)this + 4) + 0xb4) = 0;
    (*DAT_10121e10)(*(undefined4 *)(*(int *)((int)this + 4) + 0x208),
                    *(undefined4 *)((int)this + 0x120));
    FUN_100afe10((int)this,*(int *)((int)this + 4) + 8);
    if (*(int **)((int)this + 0xdc) != (int *)0x0) {
      FUN_100c2e20(*(int **)((int)this + 0xdc),this);
    }
    CBasePlayerItem::Materialize(this);
    *(undefined4 *)(*(int *)((int)this + 4) + 0x10c) = 0;
    *(uint *)(*(int *)((int)this + 4) + 0x118) = *(uint *)(*(int *)((int)this + 4) + 0x118) | 0x80;
    DVar8 = GetTickCount();
    *(DWORD *)((int)this + 300) = DVar8 + 500;
    efwConversation_AddKeyword(s_Player_sIDTagOnFence_1011c678,1);
    efwConversation_Squark((byte *)s_efw_compound_gate_guard_1010318c,s_Okay_RAR_124__you_can_pass__1011c690,4
                );
  }
  return;
}



/* ==== 100c2bf0 FUN_100c2bf0 ==== */

void FUN_100c2bf0(void)

{
  DAT_10132c70 = 0;
  return;
}



/* ==== 100c2c00 FUN_100c2c00 ==== */

undefined4 * __cdecl FUN_100c2c00(int param_1)

{
  int iVar1;
  int *piVar2;
  
  iVar1 = 0;
  if (0 < DAT_10132c70) {
    piVar2 = &DAT_10132470;
    do {
      if (*piVar2 == param_1) {
        return &DAT_10132470 + iVar1 * 2;
      }
      iVar1 = iVar1 + 1;
      piVar2 = piVar2 + 2;
    } while (iVar1 < DAT_10132c70);
  }
  return (undefined4 *)0x0;
}



/* ==== 100c2c30 FUN_100c2c30 ==== */

void __cdecl FUN_100c2c30(int *param_1,int *param_2)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int local_4;
  
  piVar2 = param_1;
  local_4 = 6;
  param_1 = param_1 + 0x144;
  do {
    piVar1 = (int *)*param_1;
    if (piVar1 != (int *)0x0) {
      for (piVar3 = (int *)piVar1[0x38]; piVar3 != (int *)0x0; piVar3 = (int *)piVar3[0x38]) {
        if (piVar3 == param_2) {
          (**(code **)(*piVar3 + 0x144))();
          if (piVar2[0x14c] == piVar1[0x38]) {
            piVar2[0x14c] = 0;
          }
          piVar1[0x38] = *(int *)(piVar1[0x38] + 0xe0);
          param_2[0x38] = 0;
          piVar3 = piVar1;
        }
        piVar1 = piVar3;
      }
      if ((int *)*param_1 == param_2) {
        (**(code **)(*(int *)*param_1 + 0x144))();
        if (piVar2[0x14c] == *param_1) {
          piVar2[0x14c] = 0;
        }
        *param_1 = *(int *)(*param_1 + 0xe0);
      }
    }
    param_1 = param_1 + 1;
    local_4 = local_4 + -1;
  } while (local_4 != 0);
  *(uint *)(piVar2[1] + 0x168) = *(uint *)(piVar2[1] + 0x168) & ~(1 << ((byte)param_2[0x39] & 0x1f))
  ;
  if ((int *)piVar2[0x14a] == param_2) {
    FUN_10081e70((int)piVar2);
    (**(code **)(*(int *)piVar2[0x14a] + 0x134))(0);
    *(undefined4 *)(piVar2[1] + 0xbc) = 0;
    *(undefined4 *)(piVar2[1] + 0xc0) = 0;
    piVar2[0x14a] = 0;
    (**(code **)(*piVar2 + 0x23c))();
    (*DAT_10121ec0)(1,DAT_10124884,0,*(undefined4 *)(piVar2[1] + 0x208));
    (*DAT_10121ec8)(0);
    (*DAT_10121ec8)(0);
    (*DAT_10121ec8)(0);
    (*DAT_10121ec4)();
    param_2[0x37] = 0;
    return;
  }
  (**(code **)(*piVar2 + 0x23c))();
  param_2[0x37] = 0;
  return;
}



/* ==== 100c2dc0 FUN_100c2dc0 ==== */

uint __cdecl FUN_100c2dc0(int param_1,byte *param_2)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  
  iVar2 = 0;
  piVar3 = (int *)(param_1 + 0x510);
  do {
    for (iVar1 = *piVar3; iVar1 != 0; iVar1 = *(int *)(iVar1 + 0xe0)) {
      param_1 = FUN_100d8b20(*(void **)(DAT_10122044 + 0x98),
                             (byte *)(**(int **)(iVar1 + 4) + (int)*(void **)(DAT_10122044 + 0x98)),
                             param_2);
      if (param_1 == 0) {
        return 1;
      }
    }
    iVar2 = iVar2 + 1;
    piVar3 = piVar3 + 1;
  } while (iVar2 < 6);
  return param_1 & 0xffffff00;
}



/* ==== 100c2e20 FUN_100c2e20 ==== */

void __cdecl FUN_100c2e20(int *param_1,int *param_2)

{
  undefined4 *puVar1;
  int iVar2;
  
  puVar1 = FUN_100c2c00((int)param_2);
  if (puVar1 != (undefined4 *)0x0) {
    FUN_100c97a0(puVar1,puVar1 + 2,((int)&DAT_10132468 + (DAT_10132c70 * 8 - (int)puVar1) >> 3) << 3
                );
    DAT_10132c70 = DAT_10132c70 + -1;
  }
  iVar2 = (**(code **)(*param_1 + 0xcc))();
  if (iVar2 != 0) {
    FUN_100c2c30(param_1,param_2);
  }
  return;
}



/* ==== 100c2e90 FUN_100c2e90 ==== */

void __cdecl FUN_100c2e90(undefined4 param_1,undefined4 param_2)

{
  (&DAT_10132470)[DAT_10132c70 * 2] = param_2;
  (&DAT_10132474)[DAT_10132c70 * 2] = param_1;
  DAT_10132c70 = DAT_10132c70 + 1;
  return;
}



/* ==== 100c2ee0 FUN_100c2ee0 ==== */

int __cdecl FUN_100c2ee0(int *param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  
  iVar2 = (**(code **)(*param_1 + 0xcc))();
  if (iVar2 == 0) {
    iVar2 = 0;
    if (0 < DAT_10132c70) {
      piVar3 = &DAT_10132470;
      while (((int *)piVar3[1] != param_1 || (*(int *)(*piVar3 + 0xe4) != param_2))) {
        iVar2 = iVar2 + 1;
        piVar3 = piVar3 + 2;
        if (DAT_10132c70 <= iVar2) {
          return 0;
        }
      }
      return (&DAT_10132470)[iVar2 * 2];
    }
  }
  else {
    piVar3 = param_1 + 0x144;
    iVar2 = 0;
    do {
      for (iVar1 = *piVar3; iVar1 != 0; iVar1 = *(int *)(iVar1 + 0xe0)) {
        if (*(int *)(iVar1 + 0xe4) == param_2) {
          return iVar1;
        }
      }
      iVar2 = iVar2 + 1;
      piVar3 = piVar3 + 1;
    } while (iVar2 < 6);
  }
  return 0;
}



/* ==== 100c2f70 FUN_100c2f70 ==== */

uint __cdecl FUN_100c2f70(int *param_1,byte *param_2)

{
  byte bVar1;
  int iVar2;
  uint uVar3;
  byte *pbVar4;
  int iVar5;
  byte *pbVar6;
  int *piVar7;
  bool bVar8;
  
  iVar2 = (**(code **)(*param_1 + 0xcc))();
  if (iVar2 != 0) {
    uVar3 = FUN_100c2dc0((int)param_1,param_2);
    return uVar3;
  }
  iVar2 = 0;
  if (0 < (int)DAT_10132c70) {
    piVar7 = &DAT_10132470;
    do {
      if ((int *)piVar7[1] == param_1) {
        pbVar4 = (byte *)(**(int **)(*piVar7 + 4) + *(int *)(DAT_10122044 + 0x98));
        pbVar6 = param_2;
        do {
          bVar1 = *pbVar4;
          bVar8 = bVar1 < *pbVar6;
          if (bVar1 != *pbVar6) {
LAB_100c2feb:
            iVar5 = (1 - (uint)bVar8) - (uint)(bVar8 != 0);
            goto LAB_100c2ff0;
          }
          if (bVar1 == 0) break;
          bVar1 = pbVar4[1];
          bVar8 = bVar1 < pbVar6[1];
          if (bVar1 != pbVar6[1]) goto LAB_100c2feb;
          pbVar4 = pbVar4 + 2;
          pbVar6 = pbVar6 + 2;
        } while (bVar1 != 0);
        iVar5 = 0;
LAB_100c2ff0:
        if (iVar5 == 0) {
          return 1;
        }
      }
      iVar2 = iVar2 + 1;
      piVar7 = piVar7 + 2;
    } while (iVar2 < (int)DAT_10132c70);
  }
  return DAT_10132c70 & 0xffffff00;
}



/* ==== 100c3020 FUN_100c3020 ==== */

void __cdecl FUN_100c3020(void *param_1)

{
  undefined1 uVar1;
  undefined3 extraout_var;
  int iVar2;
  
  uVar1 = FUN_100c5b80();
  iVar2 = CONCAT31(extraout_var,uVar1);
  if (iVar2 == 0) {
    FUN_1007f800(param_1,0x1011b9cc);
    FUN_1007f800(param_1,0x1011b9b4);
    return;
  }
  if (iVar2 == 1) {
    FUN_1007f800(param_1,0x101054b8);
    return;
  }
  if (iVar2 == 2) {
    FUN_1007f800(param_1,0x101054a4);
  }
  return;
}



/* ==== 100c30a0 FUN_100c30a0 ==== */

void __fastcall FUN_100c30a0(int param_1)

{
  float10 fVar1;
  
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x10c) = 0;
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x108) = 7;
  (*DAT_10121e10)(*(undefined4 *)(*(int *)(param_1 + 4) + 0x208),
                  *(int *)(*(int *)(param_1 + 4) + 0xb8) + *(int *)(DAT_10122044 + 0x98));
  FUN_100afe10(param_1,*(int *)(param_1 + 4) + 8);
  fVar1 = (float10)(*DAT_10121eec)(s_showtriggers_10103c24);
  if (fVar1 == (float10)0.0) {
    *(uint *)(*(int *)(param_1 + 4) + 0x118) = *(uint *)(*(int *)(param_1 + 4) + 0x118) | 0x80;
  }
  return;
}



/* ==== 100c3120 efw_Marker ==== */

void __cdecl efw_Marker(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  
                    /* 0xc3120  317  efw_Marker */
  if ((param_1 == 0) && (iVar2 = (*DAT_10121e5c)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = iRam00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (puVar3 = (undefined4 *)(*DAT_10121f04)(iVar2,0x15c), iVar1 = param_1,
     puVar3 != (undefined4 *)0x0)) {
    *puVar3 = &PTR_LAB_100f80c4;
    puVar3[1] = param_1;
    return;
  }
  iRam00000004 = iVar1;
  return;
}



/* ==== 100c3180 FUN_100c3180 ==== */

void FUN_100c3180(void)

{
  undefined4 *puVar1;
  undefined1 local_5;
  undefined1 local_4 [4];
  
  DAT_10132c80 = local_5;
  DAT_10132c81 = local_5;
  DAT_10132c88 = 0;
  puVar1 = operator_new(0x24);
  puVar1[1] = 0;
  puVar1[8] = 1;
  FUN_100d9cb0(local_4);
  if (DAT_10132c78 == (undefined4 *)0x0) {
    DAT_10132c78 = puVar1;
    *puVar1 = 0;
    puVar1 = (undefined4 *)0x0;
    DAT_10132c78[2] = 0;
  }
  DAT_10132c74 = DAT_10132c74 + 1;
  FUN_100d9d4c();
  if (puVar1 != (undefined4 *)0x0) {
    FUN_100c8ce6((undefined *)puVar1);
  }
  puVar1 = DAT_10132c78;
  DAT_10132c84 = operator_new(0x24);
  *(undefined4 **)((int)DAT_10132c84 + 4) = puVar1;
  *(undefined4 *)((int)DAT_10132c84 + 0x20) = 0;
  DAT_10132c8c = 0;
  *(void **)DAT_10132c84 = DAT_10132c84;
  *(void **)((int)DAT_10132c84 + 8) = DAT_10132c84;
  return;
}



/* ==== 100c3240 FUN_100c3240 ==== */

void FUN_100c3240(void)

{
  FUN_100c89be(&LAB_100c3250);
  return;
}



/* ==== 100c3370 FUN_100c3370 ==== */

void FUN_100c3370(void)

{
  int *piVar1;
  int *piVar2;
  int *local_8;
  undefined4 local_4;
  
  piVar2 = DAT_10132c84;
  local_8 = (int *)*DAT_10132c84;
  if (DAT_10132c8c == 0) {
    DAT_10132c8c = 0;
    piVar1 = local_8;
    while (local_8 = piVar1, piVar1 != piVar2) {
      FUN_100c36b0((int *)&local_8);
      FUN_100c3960(&DAT_10132c80,&local_4,piVar1);
      piVar1 = local_8;
    }
    FUN_100c35e0();
    return;
  }
  piVar2 = (int *)DAT_10132c84[1];
  if ((int *)DAT_10132c84[1] != DAT_10132c78) {
    do {
      FUN_100c3d60((int *)piVar2[2]);
      piVar1 = (int *)*piVar2;
      FUN_100c3660((int)(piVar2 + 3));
      FUN_100c8ce6((undefined *)piVar2);
      piVar2 = piVar1;
    } while (piVar1 != DAT_10132c78);
  }
  DAT_10132c84[1] = (int)DAT_10132c78;
  DAT_10132c8c = 0;
  *DAT_10132c84 = (int)DAT_10132c84;
  DAT_10132c84[2] = (int)DAT_10132c84;
  FUN_100c35e0();
  return;
}



/* ==== 100c3430 FUN_100c3430 ==== */

undefined1 __cdecl FUN_100c3430(char *param_1)

{
  char cVar1;
  undefined1 uVar2;
  undefined4 uVar3;
  uint uVar4;
  uint uVar5;
  char *pcVar6;
  int local_2c [2];
  undefined1 local_24 [4];
  char *local_20;
  uint local_1c;
  undefined4 local_18;
  int local_14 [4];
  undefined1 local_4;
  
  local_24[0] = param_1._0_1_;
  uVar4 = 0xffffffff;
  pcVar6 = param_1;
  do {
    if (uVar4 == 0) break;
    uVar4 = uVar4 - 1;
    cVar1 = *pcVar6;
    pcVar6 = pcVar6 + 1;
  } while (cVar1 != '\0');
  uVar4 = ~uVar4 - 1;
  local_20 = (char *)0x0;
  local_1c = 0;
  local_18 = 0;
  uVar3 = FUN_100bbb30(local_24,uVar4,'\x01');
  if ((char)uVar3 != '\0') {
    pcVar6 = local_20;
    for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)param_1;
      param_1 = param_1 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar5 = uVar4 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
      *pcVar6 = *param_1;
      param_1 = param_1 + 1;
      pcVar6 = pcVar6 + 1;
    }
    local_20[uVar4] = '\0';
    local_1c = uVar4;
  }
  FUN_100ba1f0(local_14,local_24);
  local_4 = 0;
  FUN_100c3700(&DAT_10132c80,local_2c,local_14);
  FUN_100bacf0(local_14,'\x01');
  uVar2 = *(undefined1 *)(local_2c[0] + 0x1c);
  if (local_20 != (char *)0x0) {
    cVar1 = local_20[-1];
    if ((cVar1 != '\0') && (cVar1 != -1)) {
      local_20[-1] = cVar1 + -1;
      return uVar2;
    }
    FUN_100c8ce6(local_20 + -1);
  }
  return uVar2;
}



/* ==== 100c3500 efwConversation::AddKeyword ==== */

void __cdecl efwConversation_AddKeyword(char *param_1,undefined1 param_2)

{
  char cVar1;
  undefined4 uVar2;
  uint uVar3;
  uint uVar4;
  char *pcVar5;
  int local_2c [2];
  undefined1 local_24 [4];
  char *local_20;
  uint local_1c;
  undefined4 local_18;
  int local_14 [4];
  undefined1 local_4;
  
  local_24[0] = param_2;
  uVar3 = 0xffffffff;
  pcVar5 = param_1;
  do {
    if (uVar3 == 0) break;
    uVar3 = uVar3 - 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar5 + 1;
  } while (cVar1 != '\0');
  uVar3 = ~uVar3 - 1;
  local_20 = (char *)0x0;
  local_1c = 0;
  local_18 = 0;
  uVar2 = FUN_100bbb30(local_24,uVar3,'\x01');
  if ((char)uVar2 != '\0') {
    pcVar5 = local_20;
    for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)param_1;
      param_1 = param_1 + 4;
      pcVar5 = pcVar5 + 4;
    }
    for (uVar4 = uVar3 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
      *pcVar5 = *param_1;
      param_1 = param_1 + 1;
      pcVar5 = pcVar5 + 1;
    }
    local_20[uVar3] = '\0';
    local_1c = uVar3;
  }
  FUN_100ba1f0(local_14,local_24);
  local_4 = 0;
  FUN_100c3700(&DAT_10132c80,local_2c,local_14);
  FUN_100bacf0(local_14,'\x01');
  *(undefined1 *)(local_2c[0] + 0x1c) = param_2;
  if (local_20 != (char *)0x0) {
    cVar1 = local_20[-1];
    if ((cVar1 != '\0') && (cVar1 != -1)) {
      local_20[-1] = cVar1 + -1;
      FUN_100c35e0();
      return;
    }
    FUN_100c8ce6(local_20 + -1);
  }
  FUN_100c35e0();
  return;
}



/* ==== 100c35e0 FUN_100c35e0 ==== */

void FUN_100c35e0(void)

{
  int *piVar1;
  int *piVar2;
  
  efw_DebugPrint();
  piVar2 = (int *)*DAT_10132c84;
  if (piVar2 != DAT_10132c84) {
    do {
      efw_DebugPrint();
      if ((undefined4 *)piVar2[2] == DAT_10132c78) {
        piVar1 = (int *)piVar2[1];
        if (piVar2 == (int *)piVar1[2]) {
          do {
            piVar2 = piVar1;
            piVar1 = (int *)piVar2[1];
          } while (piVar2 == (int *)piVar1[2]);
        }
        if ((int *)piVar2[2] != piVar1) goto LAB_100c3648;
      }
      else {
        piVar1 = (int *)FUN_100c3dd0((undefined4 *)piVar2[2]);
LAB_100c3648:
        piVar2 = piVar1;
      }
    } while (piVar2 != DAT_10132c84);
  }
  return;
}



/* ==== 100c3660 FUN_100c3660 ==== */

void __fastcall FUN_100c3660(int param_1)

{
  char cVar1;
  int iVar2;
  
  iVar2 = *(int *)(param_1 + 4);
  if (iVar2 != 0) {
    cVar1 = *(char *)(iVar2 + -1);
    if ((cVar1 == '\0') || (cVar1 == -1)) {
      FUN_100c8ce6((char *)(iVar2 + -1));
    }
    else {
      *(char *)(iVar2 + -1) = cVar1 + -1;
    }
  }
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 8) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0;
  return;
}



/* ==== 100c36a0 FUN_100c36a0 ==== */

void __thiscall FUN_100c36a0(void *this,undefined4 *param_1)

{
  *param_1 = **(undefined4 **)((int)this + 4);
  return;
}



/* ==== 100c36b0 FUN_100c36b0 ==== */

void __fastcall FUN_100c36b0(int *param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  int iVar3;
  
  puVar1 = *(undefined4 **)(*param_1 + 8);
  if (puVar1 == DAT_10132c78) {
    iVar3 = *(int *)(*param_1 + 4);
    if (*param_1 == *(int *)(iVar3 + 8)) {
      do {
        *param_1 = iVar3;
        iVar3 = *(int *)(iVar3 + 4);
      } while (*param_1 == *(int *)(iVar3 + 8));
    }
    if (*(int *)(*param_1 + 8) != iVar3) {
      *param_1 = iVar3;
    }
    return;
  }
  for (puVar2 = (undefined4 *)*puVar1; puVar2 != DAT_10132c78; puVar2 = (undefined4 *)*puVar2) {
    puVar1 = puVar2;
  }
  *param_1 = (int)puVar1;
  return;
}



/* ==== 100c3700 FUN_100c3700 ==== */

undefined4 * __thiscall FUN_100c3700(void *this,undefined4 *param_1,int *param_2)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  byte *pbVar4;
  uint uVar5;
  undefined4 *puVar6;
  int *piVar7;
  int *piVar8;
  bool bVar9;
  int *local_4;
  
  piVar1 = param_2;
  bVar9 = true;
  piVar7 = *(int **)((int)this + 4);
  piVar8 = (int *)piVar7[1];
  local_4 = this;
  if (piVar8 != DAT_10132c78) {
    do {
      piVar7 = piVar8;
      pbVar4 = (byte *)piVar7[4];
      if (pbVar4 == (byte *)0x0) {
        pbVar4 = (byte *)&DAT_100f7058;
      }
      uVar5 = FUN_100be3b0(piVar1,0,piVar1[2],pbVar4,piVar7[5]);
      bVar9 = (int)uVar5 < 0;
      if (bVar9) {
        piVar8 = (int *)*piVar7;
      }
      else {
        piVar8 = (int *)piVar7[2];
      }
    } while (piVar8 != DAT_10132c78);
  }
  if (*(char *)((int)this + 8) != '\0') {
    param_2 = (int *)FUN_100c4140(piVar7,0);
    *param_2 = (int)DAT_10132c78;
    param_2[2] = (int)DAT_10132c78;
    FUN_100c41c0((undefined1 *)(param_2 + 3),(undefined1 *)piVar1);
    *(int *)((int)this + 0xc) = *(int *)((int)this + 0xc) + 1;
    if (((piVar7 == *(int **)((int)this + 4)) || (piVar8 != DAT_10132c78)) ||
       (uVar5 = FUN_100be360(piVar1,(int)(piVar7 + 3)), (int)uVar5 < 0)) {
      *piVar7 = (int)param_2;
      piVar8 = *(int **)((int)this + 4);
      if (piVar7 == piVar8) {
        piVar8[1] = (int)param_2;
        *(int **)(*(int *)((int)this + 4) + 8) = param_2;
      }
      else if (piVar7 == (int *)*piVar8) {
        *piVar8 = (int)param_2;
      }
    }
    else {
      piVar7[2] = (int)param_2;
      if (piVar7 == *(int **)(*(int *)((int)this + 4) + 8)) {
        *(int **)(*(int *)((int)this + 4) + 8) = param_2;
      }
    }
    piVar8 = param_2;
    piVar7 = param_2;
    if (param_2 != *(int **)(*(int *)((int)this + 4) + 4)) {
      do {
        piVar1 = (int *)piVar7[1];
        if (piVar1[8] != 0) break;
        piVar2 = *(int **)piVar1[1];
        if (piVar1 == piVar2) {
          iVar3 = ((undefined4 *)piVar1[1])[2];
          if (*(int *)(iVar3 + 0x20) == 0) {
            piVar1[8] = 1;
            *(undefined4 *)(iVar3 + 0x20) = 1;
            *(undefined4 *)(*(int *)(piVar7[1] + 4) + 0x20) = 0;
            piVar7 = *(int **)(piVar7[1] + 4);
          }
          else {
            if (piVar7 == (int *)piVar1[2]) {
              FUN_100c4080(this,(int)piVar1);
              piVar7 = piVar1;
            }
            *(undefined4 *)(piVar7[1] + 0x20) = 1;
            *(undefined4 *)(*(int *)(piVar7[1] + 4) + 0x20) = 0;
            FUN_100c40e0(this,*(int **)(piVar7[1] + 4));
          }
        }
        else if (piVar2[8] == 0) {
          piVar1[8] = 1;
          piVar2[8] = 1;
          *(undefined4 *)(*(int *)(piVar7[1] + 4) + 0x20) = 0;
          piVar7 = *(int **)(piVar7[1] + 4);
        }
        else {
          if (piVar7 == (int *)*piVar1) {
            FUN_100c40e0(this,piVar1);
            piVar7 = piVar1;
          }
          *(undefined4 *)(piVar7[1] + 0x20) = 1;
          *(undefined4 *)(*(int *)(piVar7[1] + 4) + 0x20) = 0;
          FUN_100c4080(this,*(int *)(piVar7[1] + 4));
        }
      } while (piVar7 != *(int **)(*(int *)((int)this + 4) + 4));
    }
    *(undefined4 *)(*(int *)(*(int *)((int)this + 4) + 4) + 0x20) = 1;
    *param_1 = piVar8;
    *(undefined1 *)(param_1 + 1) = 1;
    return param_1;
  }
  local_4 = piVar7;
  if (bVar9) {
    if (piVar7 == (int *)**(int **)((int)this + 4)) goto LAB_100c3920;
    FUN_100c4160((int *)&local_4);
  }
  uVar5 = FUN_100be360(local_4 + 3,(int)piVar1);
  if (-1 < (int)uVar5) {
    param_2 = (int *)((uint)param_2 & 0xffffff00);
    FUN_100c3df0(param_1,&local_4,(undefined1 *)&param_2);
    return param_1;
  }
LAB_100c3920:
  puVar6 = (undefined4 *)FUN_100c3e10(this,&param_2,(int)piVar8,piVar7,(undefined1 *)piVar1);
  *param_1 = *puVar6;
  *(undefined1 *)(param_1 + 1) = 1;
  return param_1;
}



/* ==== 100c3960 FUN_100c3960 ==== */

void __thiscall FUN_100c3960(void *this,undefined4 *param_1,int *param_2)

{
  char cVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  undefined4 *puVar5;
  int *piVar6;
  int *piVar7;
  int *piVar8;
  undefined4 *puVar9;
  int *piVar10;
  undefined4 *puVar11;
  int *local_4;
  
  piVar3 = param_2;
  FUN_100c36b0((int *)&param_2);
  piVar8 = (int *)*piVar3;
  local_4 = piVar3;
  if (piVar8 == DAT_10132c78) {
    piVar10 = (int *)piVar3[2];
  }
  else {
    piVar4 = (int *)piVar3[2];
    piVar10 = piVar8;
    if (piVar4 != DAT_10132c78) {
      for (piVar10 = (int *)*piVar4; piVar10 != DAT_10132c78; piVar10 = (int *)*piVar10) {
        piVar4 = piVar10;
      }
      piVar10 = (int *)piVar4[2];
      local_4 = piVar4;
      if (piVar4 != piVar3) {
        piVar8[1] = (int)piVar4;
        *piVar4 = *piVar3;
        if (piVar4 == (int *)piVar3[2]) {
          piVar10[1] = (int)piVar4;
        }
        else {
          piVar10[1] = piVar4[1];
          *(int **)piVar4[1] = piVar10;
          piVar4[2] = piVar3[2];
          *(int **)(piVar3[2] + 4) = piVar4;
        }
        if (*(int **)(*(int *)((int)this + 4) + 4) == piVar3) {
          *(int **)(*(int *)((int)this + 4) + 4) = piVar4;
        }
        else {
          piVar8 = (int *)piVar3[1];
          if ((int *)*piVar8 == piVar3) {
            *piVar8 = (int)piVar4;
          }
          else {
            piVar8[2] = (int)piVar4;
          }
        }
        local_4 = piVar3;
        piVar4[1] = piVar3[1];
        iVar2 = piVar4[8];
        piVar4[8] = piVar3[8];
        piVar3[8] = iVar2;
        goto LAB_100c3a8f;
      }
    }
  }
  piVar10[1] = local_4[1];
  if (*(int **)(*(int *)((int)this + 4) + 4) == piVar3) {
    *(int **)(*(int *)((int)this + 4) + 4) = piVar10;
  }
  else {
    piVar8 = (int *)piVar3[1];
    if ((int *)*piVar8 == piVar3) {
      *piVar8 = (int)piVar10;
    }
    else {
      piVar8[2] = (int)piVar10;
    }
  }
  piVar8 = *(int **)((int)this + 4);
  if ((int *)*piVar8 == piVar3) {
    if ((int *)piVar3[2] == DAT_10132c78) {
      *piVar8 = piVar3[1];
    }
    else {
      piVar7 = (int *)*piVar10;
      piVar4 = piVar10;
      while (piVar6 = piVar7, piVar6 != DAT_10132c78) {
        piVar4 = piVar6;
        piVar7 = (int *)*piVar6;
      }
      *piVar8 = (int)piVar4;
    }
  }
  if (*(int **)(*(int *)((int)this + 4) + 8) == piVar3) {
    if ((int *)*piVar3 == DAT_10132c78) {
      piVar8 = (int *)piVar3[1];
    }
    else {
      piVar3 = (int *)piVar10[2];
      piVar8 = piVar10;
      while (piVar4 = piVar3, piVar4 != DAT_10132c78) {
        piVar8 = piVar4;
        piVar3 = (int *)piVar4[2];
      }
    }
    *(int **)(*(int *)((int)this + 4) + 8) = piVar8;
  }
LAB_100c3a8f:
  if (local_4[8] == 1) {
    if (piVar10 != *(int **)(*(int *)((int)this + 4) + 4)) {
      do {
        if (piVar10[8] != 1) break;
        piVar8 = *(int **)piVar10[1];
        if (piVar10 == piVar8) {
          piVar8 = (int *)((undefined4 *)piVar10[1])[2];
          if (piVar8[8] == 0) {
            piVar8[8] = 1;
            *(undefined4 *)(piVar10[1] + 0x20) = 0;
            iVar2 = piVar10[1];
            piVar8 = *(int **)(iVar2 + 8);
            *(int *)(iVar2 + 8) = *piVar8;
            if ((int *)*piVar8 != DAT_10132c78) {
              ((int *)*piVar8)[1] = iVar2;
            }
            piVar8[1] = *(int *)(iVar2 + 4);
            if (iVar2 == *(int *)(*(int *)((int)this + 4) + 4)) {
              *(int **)(*(int *)((int)this + 4) + 4) = piVar8;
            }
            else {
              piVar3 = *(int **)(iVar2 + 4);
              if (iVar2 == *piVar3) {
                *piVar3 = (int)piVar8;
              }
              else {
                piVar3[2] = (int)piVar8;
              }
            }
            *piVar8 = iVar2;
            *(int **)(iVar2 + 4) = piVar8;
            piVar8 = *(int **)(piVar10[1] + 8);
          }
          if ((*(int *)(*piVar8 + 0x20) != 1) || (*(int *)(piVar8[2] + 0x20) != 1)) {
            if (*(int *)(piVar8[2] + 0x20) == 1) {
              *(undefined4 *)(*piVar8 + 0x20) = 1;
              iVar2 = *piVar8;
              piVar8[8] = 0;
              *piVar8 = *(int *)(iVar2 + 8);
              if (*(int **)(iVar2 + 8) != DAT_10132c78) {
                (*(int **)(iVar2 + 8))[1] = (int)piVar8;
              }
              *(int *)(iVar2 + 4) = piVar8[1];
              if (piVar8 == *(int **)(*(int *)((int)this + 4) + 4)) {
                *(int *)(*(int *)((int)this + 4) + 4) = iVar2;
              }
              else {
                piVar3 = (int *)piVar8[1];
                if (piVar8 == (int *)piVar3[2]) {
                  piVar3[2] = iVar2;
                }
                else {
                  *piVar3 = iVar2;
                }
              }
              *(int **)(iVar2 + 8) = piVar8;
              piVar8[1] = iVar2;
              piVar8 = *(int **)(piVar10[1] + 8);
            }
            piVar8[8] = *(int *)(piVar10[1] + 0x20);
            *(undefined4 *)(piVar10[1] + 0x20) = 1;
            *(undefined4 *)(piVar8[2] + 0x20) = 1;
            puVar9 = (undefined4 *)piVar10[1];
            puVar11 = (undefined4 *)puVar9[2];
            puVar9[2] = *puVar11;
            if ((int *)*puVar11 != DAT_10132c78) {
              ((int *)*puVar11)[1] = (int)puVar9;
            }
            puVar11[1] = puVar9[1];
            if (puVar9 == *(undefined4 **)(*(int *)((int)this + 4) + 4)) {
              *(undefined4 **)(*(int *)((int)this + 4) + 4) = puVar11;
              *puVar11 = puVar9;
            }
            else {
              piVar8 = (int *)puVar9[1];
              if (puVar9 == (undefined4 *)*piVar8) {
                *piVar8 = (int)puVar11;
                *puVar11 = puVar9;
              }
              else {
                piVar8[2] = (int)puVar11;
                *puVar11 = puVar9;
              }
            }
LAB_100c3d03:
            puVar9[1] = puVar11;
            break;
          }
        }
        else {
          if (piVar8[8] == 0) {
            piVar8[8] = 1;
            *(undefined4 *)(piVar10[1] + 0x20) = 0;
            piVar8 = (int *)piVar10[1];
            iVar2 = *piVar8;
            *piVar8 = *(int *)(iVar2 + 8);
            if (*(int **)(iVar2 + 8) != DAT_10132c78) {
              (*(int **)(iVar2 + 8))[1] = (int)piVar8;
            }
            *(int *)(iVar2 + 4) = piVar8[1];
            if (piVar8 == *(int **)(*(int *)((int)this + 4) + 4)) {
              *(int *)(*(int *)((int)this + 4) + 4) = iVar2;
            }
            else {
              piVar3 = (int *)piVar8[1];
              if (piVar8 == (int *)piVar3[2]) {
                piVar3[2] = iVar2;
              }
              else {
                *piVar3 = iVar2;
              }
            }
            *(int **)(iVar2 + 8) = piVar8;
            piVar8[1] = iVar2;
            piVar8 = *(int **)piVar10[1];
          }
          if ((*(int *)(piVar8[2] + 0x20) != 1) || (*(int *)(*piVar8 + 0x20) != 1)) {
            if (*(int *)(*piVar8 + 0x20) == 1) {
              *(undefined4 *)(piVar8[2] + 0x20) = 1;
              piVar3 = (int *)piVar8[2];
              piVar8[8] = 0;
              piVar8[2] = *piVar3;
              if ((int *)*piVar3 != DAT_10132c78) {
                ((int *)*piVar3)[1] = (int)piVar8;
              }
              piVar3[1] = piVar8[1];
              if (piVar8 == *(int **)(*(int *)((int)this + 4) + 4)) {
                *(int **)(*(int *)((int)this + 4) + 4) = piVar3;
              }
              else {
                piVar4 = (int *)piVar8[1];
                if (piVar8 == (int *)*piVar4) {
                  *piVar4 = (int)piVar3;
                }
                else {
                  piVar4[2] = (int)piVar3;
                }
              }
              *piVar3 = (int)piVar8;
              piVar8[1] = (int)piVar3;
              piVar8 = *(int **)piVar10[1];
            }
            piVar8[8] = *(int *)(piVar10[1] + 0x20);
            *(undefined4 *)(piVar10[1] + 0x20) = 1;
            *(undefined4 *)(*piVar8 + 0x20) = 1;
            puVar9 = (undefined4 *)piVar10[1];
            puVar11 = (undefined4 *)*puVar9;
            *puVar9 = puVar11[2];
            if ((int *)puVar11[2] != DAT_10132c78) {
              ((int *)puVar11[2])[1] = (int)puVar9;
            }
            puVar11[1] = puVar9[1];
            if (puVar9 == *(undefined4 **)(*(int *)((int)this + 4) + 4)) {
              *(undefined4 **)(*(int *)((int)this + 4) + 4) = puVar11;
            }
            else {
              puVar5 = (undefined4 *)puVar9[1];
              if (puVar9 == (undefined4 *)puVar5[2]) {
                puVar5[2] = puVar11;
              }
              else {
                *puVar5 = puVar11;
              }
            }
            puVar11[2] = puVar9;
            goto LAB_100c3d03;
          }
        }
        piVar8[8] = 0;
        piVar10 = (int *)piVar10[1];
      } while (piVar10 != *(int **)(*(int *)((int)this + 4) + 4));
    }
    piVar10[8] = 1;
  }
  iVar2 = local_4[4];
  if (iVar2 != 0) {
    cVar1 = *(char *)(iVar2 + -1);
    if ((cVar1 == '\0') || (cVar1 == -1)) {
      FUN_100c8ce6((char *)(iVar2 + -1));
    }
    else {
      *(char *)(iVar2 + -1) = cVar1 + -1;
    }
  }
  local_4[4] = 0;
  local_4[5] = 0;
  local_4[6] = 0;
  FUN_100c8ce6((undefined *)local_4);
  *(int *)((int)this + 0xc) = *(int *)((int)this + 0xc) + -1;
  *param_1 = param_2;
  return;
}



/* ==== 100c3d60 FUN_100c3d60 ==== */

void FUN_100c3d60(int *param_1)

{
  char cVar1;
  int iVar2;
  int *piVar3;
  
  if (param_1 != DAT_10132c78) {
    do {
      FUN_100c3d60((int *)param_1[2]);
      iVar2 = param_1[4];
      piVar3 = (int *)*param_1;
      if (iVar2 != 0) {
        cVar1 = *(char *)(iVar2 + -1);
        if ((cVar1 == '\0') || (cVar1 == -1)) {
          FUN_100c8ce6((char *)(iVar2 + -1));
        }
        else {
          *(char *)(iVar2 + -1) = cVar1 + -1;
        }
      }
      param_1[4] = 0;
      param_1[5] = 0;
      param_1[6] = 0;
      FUN_100c8ce6((undefined *)param_1);
      param_1 = piVar3;
    } while (piVar3 != DAT_10132c78);
  }
  return;
}



/* ==== 100c3dd0 FUN_100c3dd0 ==== */

void __cdecl FUN_100c3dd0(undefined4 *param_1)

{
  undefined4 *puVar1;
  
  for (puVar1 = (undefined4 *)*param_1; puVar1 != DAT_10132c78; puVar1 = (undefined4 *)*puVar1) {
  }
  return;
}



/* ==== 100c3df0 FUN_100c3df0 ==== */

void __thiscall FUN_100c3df0(void *this,undefined4 *param_1,undefined1 *param_2)

{
  *(undefined4 *)this = *param_1;
  *(undefined1 *)((int)this + 4) = *param_2;
  return;
}



/* ==== 100c3e10 FUN_100c3e10 ==== */

void __thiscall
FUN_100c3e10(void *this,undefined4 *param_1,int param_2,int *param_3,undefined1 *param_4)

{
  int iVar1;
  int *piVar2;
  int *piVar3;
  uint uVar4;
  int *piVar5;
  int *piVar6;
  int *piVar7;
  
  piVar3 = operator_new(0x24);
  piVar3[1] = (int)param_3;
  piVar3[8] = 0;
  *piVar3 = DAT_10132c78;
  piVar3[2] = DAT_10132c78;
  FUN_100c41c0((undefined1 *)(piVar3 + 3),param_4);
  *(int *)((int)this + 0xc) = *(int *)((int)this + 0xc) + 1;
  if ((param_3 != *(int **)((int)this + 4)) && (param_2 == DAT_10132c78)) {
    uVar4 = FUN_100be360(param_4,(int)(param_3 + 3));
    if (-1 < (int)uVar4) {
      param_3[2] = (int)piVar3;
      if (param_3 == *(int **)(*(int *)((int)this + 4) + 8)) {
        *(int **)(*(int *)((int)this + 4) + 8) = piVar3;
      }
      goto LAB_100c3ea4;
    }
  }
  *param_3 = (int)piVar3;
  piVar5 = *(int **)((int)this + 4);
  if (param_3 == piVar5) {
    piVar5[1] = (int)piVar3;
    *(int **)(*(int *)((int)this + 4) + 8) = piVar3;
  }
  else if (param_3 == (int *)*piVar5) {
    *piVar5 = (int)piVar3;
  }
LAB_100c3ea4:
  piVar5 = piVar3;
  if (piVar3 != *(int **)(*(int *)((int)this + 4) + 4)) {
    do {
      piVar6 = (int *)piVar5[1];
      if (piVar6[8] != 0) break;
      piVar7 = *(int **)piVar6[1];
      if (piVar6 == piVar7) {
        iVar1 = ((undefined4 *)piVar6[1])[2];
        if (*(int *)(iVar1 + 0x20) == 0) {
          piVar6[8] = 1;
          *(undefined4 *)(iVar1 + 0x20) = 1;
          *(undefined4 *)(*(int *)(piVar5[1] + 4) + 0x20) = 0;
          piVar5 = *(int **)(piVar5[1] + 4);
        }
        else {
          if (piVar5 == (int *)piVar6[2]) {
            piVar5 = (int *)piVar6[2];
            piVar6[2] = *piVar5;
            if (*piVar5 != DAT_10132c78) {
              *(int **)(*piVar5 + 4) = piVar6;
            }
            piVar5[1] = piVar6[1];
            if (piVar6 == *(int **)(*(int *)((int)this + 4) + 4)) {
              *(int **)(*(int *)((int)this + 4) + 4) = piVar5;
            }
            else {
              piVar7 = (int *)piVar6[1];
              if (piVar6 == (int *)*piVar7) {
                *piVar7 = (int)piVar5;
              }
              else {
                piVar7[2] = (int)piVar5;
              }
            }
            *piVar5 = (int)piVar6;
            piVar6[1] = (int)piVar5;
            piVar5 = piVar6;
          }
          *(undefined4 *)(piVar5[1] + 0x20) = 1;
          *(undefined4 *)(*(int *)(piVar5[1] + 4) + 0x20) = 0;
          piVar6 = *(int **)(piVar5[1] + 4);
          piVar7 = (int *)*piVar6;
          *piVar6 = piVar7[2];
          if (piVar7[2] != DAT_10132c78) {
            *(int **)(piVar7[2] + 4) = piVar6;
          }
          piVar7[1] = piVar6[1];
          if (piVar6 == *(int **)(*(int *)((int)this + 4) + 4)) {
            *(int **)(*(int *)((int)this + 4) + 4) = piVar7;
            piVar7[2] = (int)piVar6;
          }
          else {
            piVar2 = (int *)piVar6[1];
            if (piVar6 == (int *)piVar2[2]) {
              piVar2[2] = (int)piVar7;
              piVar7[2] = (int)piVar6;
            }
            else {
              *piVar2 = (int)piVar7;
              piVar7[2] = (int)piVar6;
            }
          }
LAB_100c4052:
          piVar6[1] = (int)piVar7;
        }
      }
      else {
        if (piVar7[8] != 0) {
          if (piVar5 == (int *)*piVar6) {
            iVar1 = *piVar6;
            *piVar6 = *(int *)(iVar1 + 8);
            if (*(int *)(iVar1 + 8) != DAT_10132c78) {
              *(int **)(*(int *)(iVar1 + 8) + 4) = piVar6;
            }
            *(int *)(iVar1 + 4) = piVar6[1];
            if (piVar6 == *(int **)(*(int *)((int)this + 4) + 4)) {
              *(int *)(*(int *)((int)this + 4) + 4) = iVar1;
            }
            else {
              piVar5 = (int *)piVar6[1];
              if (piVar6 == (int *)piVar5[2]) {
                piVar5[2] = iVar1;
              }
              else {
                *piVar5 = iVar1;
              }
            }
            *(int **)(iVar1 + 8) = piVar6;
            piVar6[1] = iVar1;
            piVar5 = piVar6;
          }
          *(undefined4 *)(piVar5[1] + 0x20) = 1;
          *(undefined4 *)(*(int *)(piVar5[1] + 4) + 0x20) = 0;
          piVar6 = *(int **)(piVar5[1] + 4);
          piVar7 = (int *)piVar6[2];
          piVar6[2] = *piVar7;
          if (*piVar7 != DAT_10132c78) {
            *(int **)(*piVar7 + 4) = piVar6;
          }
          piVar7[1] = piVar6[1];
          if (piVar6 == *(int **)(*(int *)((int)this + 4) + 4)) {
            *(int **)(*(int *)((int)this + 4) + 4) = piVar7;
          }
          else {
            piVar2 = (int *)piVar6[1];
            if (piVar6 == (int *)*piVar2) {
              *piVar2 = (int)piVar7;
            }
            else {
              piVar2[2] = (int)piVar7;
            }
          }
          *piVar7 = (int)piVar6;
          goto LAB_100c4052;
        }
        piVar6[8] = 1;
        piVar7[8] = 1;
        *(undefined4 *)(*(int *)(piVar5[1] + 4) + 0x20) = 0;
        piVar5 = *(int **)(piVar5[1] + 4);
      }
    } while (piVar5 != *(int **)(*(int *)((int)this + 4) + 4));
  }
  *(undefined4 *)(*(int *)(*(int *)((int)this + 4) + 4) + 0x20) = 1;
  *param_1 = piVar3;
  return;
}



/* ==== 100c4080 FUN_100c4080 ==== */

void __thiscall FUN_100c4080(void *this,int param_1)

{
  int *piVar1;
  int *piVar2;
  
  piVar1 = *(int **)(param_1 + 8);
  *(int *)(param_1 + 8) = *piVar1;
  if (*piVar1 != DAT_10132c78) {
    *(int *)(*piVar1 + 4) = param_1;
  }
  piVar1[1] = *(int *)(param_1 + 4);
  if (param_1 == *(int *)(*(int *)((int)this + 4) + 4)) {
    *(int **)(*(int *)((int)this + 4) + 4) = piVar1;
    *piVar1 = param_1;
    *(int **)(param_1 + 4) = piVar1;
    return;
  }
  piVar2 = *(int **)(param_1 + 4);
  if (param_1 == *piVar2) {
    *piVar2 = (int)piVar1;
    *piVar1 = param_1;
    *(int **)(param_1 + 4) = piVar1;
    return;
  }
  piVar2[2] = (int)piVar1;
  *piVar1 = param_1;
  *(int **)(param_1 + 4) = piVar1;
  return;
}



/* ==== 100c40e0 FUN_100c40e0 ==== */

void __thiscall FUN_100c40e0(void *this,int *param_1)

{
  int iVar1;
  int *piVar2;
  
  iVar1 = *param_1;
  *param_1 = *(int *)(iVar1 + 8);
  if (*(int *)(iVar1 + 8) != DAT_10132c78) {
    *(int **)(*(int *)(iVar1 + 8) + 4) = param_1;
  }
  *(int *)(iVar1 + 4) = param_1[1];
  if (param_1 == *(int **)(*(int *)((int)this + 4) + 4)) {
    *(int *)(*(int *)((int)this + 4) + 4) = iVar1;
    *(int **)(iVar1 + 8) = param_1;
    param_1[1] = iVar1;
    return;
  }
  piVar2 = (int *)param_1[1];
  if (param_1 == (int *)piVar2[2]) {
    piVar2[2] = iVar1;
    *(int **)(iVar1 + 8) = param_1;
    param_1[1] = iVar1;
    return;
  }
  *piVar2 = iVar1;
  *(int **)(iVar1 + 8) = param_1;
  param_1[1] = iVar1;
  return;
}



/* ==== 100c4140 FUN_100c4140 ==== */

void FUN_100c4140(undefined4 param_1,undefined4 param_2)

{
  void *pvVar1;
  
  pvVar1 = operator_new(0x24);
  *(undefined4 *)((int)pvVar1 + 4) = param_1;
  *(undefined4 *)((int)pvVar1 + 0x20) = param_2;
  return;
}



/* ==== 100c4160 FUN_100c4160 ==== */

void __fastcall FUN_100c4160(int *param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  
  piVar3 = (int *)*param_1;
  if ((piVar3[8] == 0) && (*(int **)(piVar3[1] + 4) == piVar3)) {
    *param_1 = piVar3[2];
    return;
  }
  iVar1 = *piVar3;
  if (iVar1 == DAT_10132c78) {
    piVar3 = (int *)piVar3[1];
    if (*param_1 == *piVar3) {
      do {
        *param_1 = (int)piVar3;
        piVar3 = (int *)piVar3[1];
      } while (*param_1 == *piVar3);
    }
    *param_1 = (int)piVar3;
    return;
  }
  for (iVar2 = *(int *)(iVar1 + 8); iVar2 != DAT_10132c78; iVar2 = *(int *)(iVar2 + 8)) {
    iVar1 = iVar2;
  }
  *param_1 = iVar1;
  return;
}



/* ==== 100c41c0 FUN_100c41c0 ==== */

/* WARNING: Removing unreachable block (ram,0x100c41f3) */

void __cdecl FUN_100c41c0(undefined1 *param_1,undefined1 *param_2)

{
  undefined1 uVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  uint uVar6;
  int iVar7;
  undefined4 *puVar8;
  
  if (param_1 != (undefined1 *)0x0) {
    uVar1 = *param_2;
    *(undefined4 *)(param_1 + 4) = 0;
    *param_1 = uVar1;
    *(undefined4 *)(param_1 + 8) = 0;
    *(undefined4 *)(param_1 + 0xc) = 0;
    uVar2 = *(uint *)(param_2 + 8);
    if (param_1 == param_2) {
      if (uVar2 != 0) {
        FUN_100d9f8d();
      }
      FUN_100bbc60(param_1);
      iVar3 = *(int *)(param_1 + 8) - uVar2;
      iVar7 = -1;
      if (iVar3 != -1) {
        iVar7 = iVar3;
      }
      if (iVar7 != 0) {
        FUN_100c97a0((undefined4 *)(uVar2 + *(int *)(param_1 + 4)),
                     (undefined4 *)((int)(uVar2 + *(int *)(param_1 + 4)) + iVar7),iVar3 - iVar7);
        iVar3 = *(int *)(param_1 + 8);
        uVar4 = FUN_100bbb30(param_1,iVar3 - iVar7,'\0');
        if ((char)uVar4 != '\0') {
          FUN_100bbb10(param_1,iVar3 - iVar7);
        }
      }
      FUN_100bbc60(param_1);
      param_1[0x10] = param_2[0x10];
      return;
    }
    if (uVar2 != 0) {
      puVar5 = *(undefined4 **)(param_2 + 4);
      if (puVar5 == (undefined4 *)0x0) {
        puVar5 = &DAT_100f7058;
      }
      if (*(byte *)((int)puVar5 + -1) < 0xfe) {
        FUN_100bacf0(param_1,'\x01');
        puVar5 = *(undefined4 **)(param_2 + 4);
        if (puVar5 == (undefined4 *)0x0) {
          puVar5 = &DAT_100f7058;
        }
        *(undefined4 **)(param_1 + 4) = puVar5;
        *(undefined4 *)(param_1 + 8) = *(undefined4 *)(param_2 + 8);
        *(undefined4 *)(param_1 + 0xc) = *(undefined4 *)(param_2 + 0xc);
        *(char *)((int)puVar5 + -1) = *(char *)((int)puVar5 + -1) + '\x01';
        param_1[0x10] = param_2[0x10];
        return;
      }
    }
    uVar4 = FUN_100bbb30(param_1,uVar2,'\x01');
    if ((char)uVar4 != '\0') {
      puVar5 = *(undefined4 **)(param_2 + 4);
      if (puVar5 == (undefined4 *)0x0) {
        puVar5 = &DAT_100f7058;
      }
      puVar8 = *(undefined4 **)(param_1 + 4);
      for (uVar6 = uVar2 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
        *puVar8 = *puVar5;
        puVar5 = puVar5 + 1;
        puVar8 = puVar8 + 1;
      }
      for (uVar6 = uVar2 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
        *(undefined1 *)puVar8 = *(undefined1 *)puVar5;
        puVar5 = (undefined4 *)((int)puVar5 + 1);
        puVar8 = (undefined4 *)((int)puVar8 + 1);
      }
      *(uint *)(param_1 + 8) = uVar2;
      *(undefined1 *)(*(int *)(param_1 + 4) + uVar2) = 0;
    }
    param_1[0x10] = param_2[0x10];
  }
  return;
}



/* ==== 100c4300 FUN_100c4300 ==== */

undefined * __thiscall FUN_100c4300(void *this,byte param_1)

{
  char cVar1;
  int iVar2;
  
  iVar2 = *(int *)((int)this + 4);
  if (iVar2 != 0) {
    cVar1 = *(char *)(iVar2 + -1);
    if ((cVar1 == '\0') || (cVar1 == -1)) {
      FUN_100c8ce6((char *)(iVar2 + -1));
    }
    else {
      *(char *)(iVar2 + -1) = cVar1 + -1;
    }
  }
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined4 *)((int)this + 0xc) = 0;
  if ((param_1 & 1) != 0) {
    FUN_100c8ce6(this);
  }
  return this;
}



/* ==== 100c43b0 FUN_100c43b0 ==== */

int __cdecl FUN_100c43b0(byte *param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined **ppuVar3;
  int iVar4;
  
  iVar2 = -1;
  iVar4 = 0;
  ppuVar3 = &PTR_s_weapon_efw_Pliers_100f81e0;
  do {
    uVar1 = FUN_100c8110(*ppuVar3,param_1);
    if ((char)uVar1 != '\0') {
      iVar2 = iVar4 + 0x10;
    }
    ppuVar3 = ppuVar3 + 1;
    iVar4 = iVar4 + 1;
  } while ((int)ppuVar3 < 0x100f8204);
  return iVar2;
}



/* ==== 100c43f0 FUN_100c43f0 ==== */

void __thiscall FUN_100c43f0(void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  *(undefined4 *)((int)this + 0x120) = param_1;
  *(undefined4 *)((int)this + 0x124) = param_2;
  *(undefined4 *)((int)this + 0x128) = param_3;
  *(undefined ***)this = &PTR_LAB_100f8228;
  return;
}



/* ==== 100c4550 FUN_100c4550 ==== */

void __thiscall FUN_100c4550(void *this,uint param_1)

{
  FUN_100b95a0(param_1,(int *)(**(int **)((int)this + 4) + *(int *)(DAT_10122044 + 0x98)),8);
  return;
}



/* ==== 100c4580 FUN_100c4580 ==== */

void __fastcall FUN_100c4580(int *param_1)

{
  undefined4 unaff_retaddr;
  
  (**(code **)(*param_1 + 0x134))(0);
  FUN_100c2e20((int *)param_1[0x37],param_1);
  FUN_100c2e90(unaff_retaddr,param_1);
  return;
}



/* ==== 100c45b0 FUN_100c45b0 ==== */

void __fastcall FUN_100c45b0(int *param_1)

{
  int iVar1;
  
  (**(code **)(*param_1 + 0x2c))();
  param_1[0x43] = -1;
  iVar1 = (**(code **)(*param_1 + 0x1ac))();
  param_1[0x39] = iVar1;
  (*DAT_10121e10)(*(undefined4 *)(param_1[1] + 0x208),param_1[0x48]);
  FUN_100b3430(param_1);
  param_1[0x1e] = (int)CBasePlayerItem::DefaultTouch;
  return;
}



/* ==== 100c4600 FUN_100c4600 ==== */

void __fastcall FUN_100c4600(int param_1)

{
  (*DAT_10121e08)(*(undefined4 *)(param_1 + 0x120));
  (*DAT_10121e08)(*(undefined4 *)(param_1 + 0x124));
  (*DAT_10121e08)(*(undefined4 *)(param_1 + 0x128));
  return;
}



/* ==== 100c46a0 FUN_100c46a0 ==== */

void __thiscall FUN_100c46a0(void *this,int *param_1)

{
  int iVar1;
  
  iVar1 = (**(code **)(*param_1 + 0xcc))();
  if (iVar1 != 0) {
    iVar1 = (**(code **)(*param_1 + 0x88))(this);
    if (iVar1 != 0) {
      (**(code **)(*(int *)this + 0x14c))(param_1);
      FUN_100b0fe0((byte *)s_You_just_picked_up_the__s__1011c738);
    }
    FUN_10098e40(this,param_1,3,0);
  }
  return;
}



/* ==== 100c4700 weapon_efw_Pliers ==== */

void __cdecl weapon_efw_Pliers(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *this;
  
                    /* 0xc4700  572  weapon_efw_Pliers */
  if ((param_1 == 0) && (iVar2 = (*DAT_10121e5c)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = iRam00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (this = (undefined4 *)(*DAT_10121f04)(iVar2,0x130), iVar1 = param_1, this != (undefined4 *)0x0)
     ) {
    FUN_100c43f0(this,s_models_w_Pliers_mdl_1011c754,s_models_v_Pliers_mdl_1011c768,
                 s_models_p_Pliers_mdl_1011c77c);
    *this = &PTR_LAB_100f83d8;
    this[1] = param_1;
    return;
  }
  iRam00000004 = iVar1;
  return;
}



/* ==== 100c4770 weapon_efw_Lever ==== */

void __cdecl weapon_efw_Lever(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *this;
  
                    /* 0xc4770  570  weapon_efw_Lever */
  if ((param_1 == 0) && (iVar2 = (*DAT_10121e5c)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = iRam00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (this = (undefined4 *)(*DAT_10121f04)(iVar2,300), iVar1 = param_1, this != (undefined4 *)0x0))
  {
    FUN_100c43f0(this,s_models_w_Lever_mdl_1011c790,s_models_v_Lever_mdl_1011c7a4,
                 s_models_p_Lever_mdl_1011c7b8);
    *this = &PTR_LAB_100f8588;
    this[1] = param_1;
    return;
  }
  iRam00000004 = iVar1;
  return;
}



/* ==== 100c47e0 weapon_efw_Branch ==== */

void __cdecl weapon_efw_Branch(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *this;
  
                    /* 0xc47e0  567  weapon_efw_Branch */
  if ((param_1 == 0) && (iVar2 = (*DAT_10121e5c)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = iRam00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (this = (undefined4 *)(*DAT_10121f04)(iVar2,300), iVar1 = param_1, this != (undefined4 *)0x0))
  {
    FUN_100c43f0(this,s_models_w_Branch_mdl_1011c7cc,s_models_v_Branch_mdl_1011c7e0,
                 s_models_p_Branch_mdl_1011c7f4);
    *this = &PTR_LAB_100f8738;
    this[1] = param_1;
    return;
  }
  iRam00000004 = iVar1;
  return;
}



/* ==== 100c4850 weapon_efw_MobilePhone ==== */

void __cdecl weapon_efw_MobilePhone(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *this;
  
                    /* 0xc4850  571  weapon_efw_MobilePhone */
  if ((param_1 == 0) && (iVar2 = (*DAT_10121e5c)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = iRam00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (this = (undefined4 *)(*DAT_10121f04)(iVar2,300), iVar1 = param_1, this != (undefined4 *)0x0))
  {
    FUN_100c43f0(this,s_models_w_MobilePhone_mdl_1011c808,s_models_v_MobilePhone_mdl_1011c824,
                 s_models_p_MobilePhone_mdl_1011c840);
    *this = &PTR_LAB_100f88e8;
    this[1] = param_1;
    return;
  }
  iRam00000004 = iVar1;
  return;
}



/* ==== 100c48c0 weapon_efw_IDTag ==== */

void __cdecl weapon_efw_IDTag(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *this;
  
                    /* 0xc48c0  569  weapon_efw_IDTag */
  if ((param_1 == 0) && (iVar2 = (*DAT_10121e5c)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = iRam00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (this = (undefined4 *)(*DAT_10121f04)(iVar2,0x130), iVar1 = param_1, this != (undefined4 *)0x0)
     ) {
    FUN_100c43f0(this,s_models_w_IDTag_mdl_1011c628,s_models_v_IDTag_mdl_1011c63c,
                 s_models_p_IDTag_mdl_1011c650);
    *this = &PTR_LAB_100f7f14;
    this[1] = param_1;
    return;
  }
  iRam00000004 = iVar1;
  return;
}



/* ==== 100c4930 weapon_efw_RedPhoneCard ==== */

void __cdecl weapon_efw_RedPhoneCard(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *this;
  
                    /* 0xc4930  573  weapon_efw_RedPhoneCard */
  if ((param_1 == 0) && (iVar2 = (*DAT_10121e5c)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = iRam00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (this = (undefined4 *)(*DAT_10121f04)(iVar2,300), iVar1 = param_1, this != (undefined4 *)0x0))
  {
    FUN_100c43f0(this,s_models_w_RedPhoneCard_mdl_1011c85c,s_models_v_RedPhoneCard_mdl_1011c878,
                 s_models_p_RedPhoneCard_mdl_1011c894);
    *this = &PTR_LAB_100f8a98;
    this[1] = param_1;
    return;
  }
  iRam00000004 = iVar1;
  return;
}



/* ==== 100c49a0 weapon_efw_GreenPhoneCard ==== */

void __cdecl weapon_efw_GreenPhoneCard(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *this;
  
                    /* 0xc49a0  568  weapon_efw_GreenPhoneCard */
  if ((param_1 == 0) && (iVar2 = (*DAT_10121e5c)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = iRam00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (this = (undefined4 *)(*DAT_10121f04)(iVar2,300), iVar1 = param_1, this != (undefined4 *)0x0))
  {
    FUN_100c43f0(this,s_models_w_GreenPhoneCard_mdl_1011c8b0,s_models_v_GreenPhoneCard_mdl_1011c8cc,
                 s_models_p_GreenPhoneCard_mdl_1011c8e8);
    *this = &PTR_LAB_100f8c48;
    this[1] = param_1;
    return;
  }
  iRam00000004 = iVar1;
  return;
}



/* ==== 100c4a10 weapon_efw_BluePhoneCard ==== */

void __cdecl weapon_efw_BluePhoneCard(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *this;
  
                    /* 0xc4a10  566  weapon_efw_BluePhoneCard */
  if ((param_1 == 0) && (iVar2 = (*DAT_10121e5c)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = iRam00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (this = (undefined4 *)(*DAT_10121f04)(iVar2,300), iVar1 = param_1, this != (undefined4 *)0x0))
  {
    FUN_100c43f0(this,s_models_w_BluePhoneCard_mdl_1011c904,s_models_v_BluePhoneCard_mdl_1011c920,
                 s_models_p_BluePhoneCard_mdl_1011c93c);
    *this = &PTR_LAB_100f8df8;
    this[1] = param_1;
    return;
  }
  iRam00000004 = iVar1;
  return;
}



/* ==== 100c4a80 weapon_efw_WashingPowder ==== */

void __cdecl weapon_efw_WashingPowder(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *this;
  
                    /* 0xc4a80  574  weapon_efw_WashingPowder */
  if ((param_1 == 0) && (iVar2 = (*DAT_10121e5c)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = iRam00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (this = (undefined4 *)(*DAT_10121f04)(iVar2,300), iVar1 = param_1, this != (undefined4 *)0x0))
  {
    FUN_100c43f0(this,s_models_w_WashingPowder_mdl_1011c958,s_models_v_WashingPowder_mdl_1011c974,
                 s_models_p_WashingPowder_mdl_1011c990);
    *this = &PTR_LAB_100f8fa8;
    this[1] = param_1;
    return;
  }
  iRam00000004 = iVar1;
  return;
}



/* ==== 100c4d10 FUN_100c4d10 ==== */

void __cdecl FUN_100c4d10(float param_1)

{
  float10 fVar1;
  
  fVar1 = FUN_100c8190(1);
  fVar1 = fVar1 - (float10)param_1;
  if (fVar1 < (float10)0.0) {
    FUN_100c8180(1,0);
    return;
  }
  if ((float10)100.0 < fVar1) {
    fVar1 = (float10)100.0;
  }
  FUN_100c8180(1,(float)fVar1);
  return;
}



/* ==== 100c4d70 FUN_100c4d70 ==== */

void __cdecl FUN_100c4d70(float param_1)

{
  float10 fVar1;
  
  fVar1 = FUN_100c8190(1);
  fVar1 = fVar1 + (float10)param_1;
  if (fVar1 < (float10)0.0) {
    FUN_100c8180(1,0);
    return;
  }
  if ((float10)100.0 < fVar1) {
    fVar1 = (float10)100.0;
  }
  FUN_100c8180(1,(float)fVar1);
  return;
}



/* ==== 100c4e30 FUN_100c4e30 ==== */

void __thiscall FUN_100c4e30(void *this,uint param_1)

{
  bool bVar1;
  int iVar2;
  uint *puVar3;
  int iVar4;
  
  puVar3 = (uint *)(*(int *)(*(int *)(param_1 + 4) + 0x1cc) + *(int *)(DAT_10122044 + 0x98));
  bVar1 = efw_FStrEq(puVar3,&DAT_1011cba4);
  if (bVar1) {
    FUN_100c4580(this);
    efwConversation_Squark(&DAT_1011cba4,s_Well_done__Your_bravery_and_clev_1011cb48,10);
    FUN_100c6910(2);
    iVar4 = 0x4c;
    iVar2 = efw_Player();
    efw_FailOrNarrate(iVar2,iVar4);
    return;
  }
  bVar1 = efw_FStrEq(puVar3,s_Fashid_1011cb40);
  if (bVar1) {
    efwConversation_Squark((byte *)s_Fashid_1011cb40,s_Thanks__but_I_don_t_want_them__A_1011ca9c,8);
    return;
  }
  bVar1 = efw_FStrEq(puVar3,s_Nasir_1011ca94);
  if (bVar1) {
    efwConversation_Squark((byte *)s_Nasir_1011ca94,s_Well_done__but_you_ll_have_to_hi_1011ca20,8);
    return;
  }
  bVar1 = efw_FStrEq(puVar3,s_Mouhtaz_1011ca18);
  if (bVar1) {
    efwConversation_Squark((byte *)s_Mouhtaz_1011ca18,s_Are_you_crazy__Whatever_you_do__d_1011c9b8,8);
    return;
  }
  FUN_100c4550(this,param_1);
  return;
}



/* ==== 100c4f30 FUN_100c4f30 ==== */

void __thiscall FUN_100c4f30(void *this,int *param_1)

{
  undefined1 uVar1;
  undefined3 extraout_var;
  int iVar2;
  
  FUN_100c46a0(this,param_1);
  uVar1 = FUN_100c5b80();
  if (CONCAT31(extraout_var,uVar1) == 0) {
    iVar2 = (**(code **)(*param_1 + 0xcc))();
    if ((iVar2 != 0) && (param_1 != (int *)0x0)) {
      efw_FailOrNarrate((int)param_1,0x3d);
    }
  }
  efwConversation_AddKeyword(s_PLIERS_1011cbcc,0);
  efwConversation_AddKeyword(s_PLIERS_GOT_PLIERS_1011cbb8,1);
  efwConversation_AddKeyword(s_ELECTRICIAN_1011cbac,0);
  return;
}



/* ==== 100c5220 FUN_100c5220 ==== */

void __cdecl FUN_100c5220(void *param_1,int param_2)

{
  FUN_1007f800(param_1,param_2);
  return;
}



/* ==== 100c53c0 monster_patrol_guard ==== */

void __cdecl monster_patrol_guard(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  
                    /* 0xc53c0  480  monster_patrol_guard */
  if ((param_1 == 0) && (iVar2 = (*DAT_10121e5c)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = iRam00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (puVar3 = (undefined4 *)(*DAT_10121f04)(iVar2,0x3b0), iVar1 = param_1,
     puVar3 != (undefined4 *)0x0)) {
    FUN_10002f30(puVar3);
    *puVar3 = &PTR_LAB_100f9160;
    puVar3[1] = param_1;
    return;
  }
  iRam00000004 = iVar1;
  return;
}



/* ==== 100c5420 FUN_100c5420 ==== */

void __fastcall FUN_100c5420(int *param_1)

{
  FUN_1000d1d0(param_1);
  (*DAT_10121e10)(*(undefined4 *)(param_1[1] + 0x208),s_models_security_mdl_101031cc);
  return;
}



/* ==== 100c5480 FUN_100c5480 ==== */

void FUN_100c5480(void)

{
  void *this;
  int iVar1;
  
  for (this = (void *)FUN_100aec40(0,s_monster_patrol_guard_1011ce50); this != (void *)0x0;
      this = (void *)FUN_100aec40((int)this,s_monster_patrol_guard_1011ce50)) {
    iVar1 = efw_Player();
    FUN_10019420((void *)((int)this + 0x168),iVar1);
    FUN_1005d500(this,4,0);
    *(undefined4 *)((int)this + 0x398) = 4;
  }
  return;
}



/* ==== 100c59c0 FUN_100c59c0 ==== */

undefined4 __cdecl FUN_100c59c0(int param_1)

{
  int iVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float10 fVar8;
  float fVar9;
  float fVar10;
  int iVar11;
  undefined2 uVar13;
  undefined2 extraout_var;
  undefined2 extraout_var_00;
  undefined4 uVar12;
  float10 fVar14;
  undefined4 local_1c;
  float local_18;
  float local_14;
  byte local_10 [4];
  char local_c [4];
  char local_8 [4];
  char local_4 [4];
  
  local_c = (char  [4])s_efw_electrician_101031bc._4_4_;
  local_10 = (byte  [4])s_efw_electrician_101031bc._0_4_;
  local_8 = (char  [4])s_efw_electrician_101031bc._8_4_;
  local_4 = (char  [4])s_efw_electrician_101031bc._12_4_;
  iVar11 = FUN_100af030(0,local_10);
  uVar12 = 0;
  if (iVar11 != 0) {
    iVar1 = *(int *)(iVar11 + 4);
    fVar2 = *(float *)(iVar1 + 8);
    fVar3 = *(float *)(iVar1 + 0xc);
    fVar4 = *(float *)(iVar1 + 0x10);
    iVar1 = *(int *)(param_1 + 4);
    fVar5 = *(float *)(iVar1 + 8);
    fVar6 = *(float *)(iVar1 + 0xc);
    fVar7 = *(float *)(iVar1 + 0x10);
    fVar9 = fVar5 - fVar2;
    local_18 = fVar6 - fVar3;
    local_14 = fVar7 - fVar4;
    fVar10 = SQRT(fVar9 * fVar9 + local_18 * local_18 + local_14 * local_14);
    uVar13 = (undefined2)((uint)fVar7 >> 0x10);
    if (fVar10 == 0.0) {
      local_1c = 0;
      local_18 = 0.0;
      local_14 = 1.0;
    }
    else {
      fVar10 = 1.0 / fVar10;
      FUN_10001d60(&local_1c,fVar9 * fVar10,local_18 * fVar10,local_14 * fVar10);
      uVar13 = extraout_var;
    }
    fVar2 = fVar2 - fVar5;
    fVar3 = fVar3 - fVar6;
    fVar4 = fVar4 - fVar7;
    fVar2 = SQRT(fVar2 * fVar2 + fVar3 * fVar3 + fVar4 * fVar4);
    if (fVar2 < 256.0 != 0) {
      return CONCAT31((int3)(CONCAT22(uVar13,(ushort)(fVar2 < 256.0) << 8 | (ushort)NAN(fVar2) << 10
                                             | (ushort)(fVar2 == 256.0) << 0xe) >> 8),1);
    }
    FUN_100af200(*(int *)(iVar11 + 4) + 0x50);
    fVar14 = (float10)FUN_100c9190();
    fVar8 = (float10)0.61086476;
    uVar12 = CONCAT22(extraout_var_00,
                      (ushort)(fVar14 < fVar8) << 8 | (ushort)(NAN(fVar14) || NAN(fVar8)) << 10 |
                      (ushort)(fVar14 == fVar8) << 0xe);
    if (fVar14 < fVar8) {
      return CONCAT31((int3)((uint)uVar12 >> 8),1);
    }
  }
  return uVar12;
}



/* ==== 100c5b60 FUN_100c5b60 ==== */

float10 FUN_100c5b60(void)

{
  return (float10)*DAT_10122044;
}



/* ==== 100c5b70 FUN_100c5b70 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_100c5b70(void)

{
  return (float10)_DAT_10134874;
}



/* ==== 100c5b80 FUN_100c5b80 ==== */

undefined1 FUN_100c5b80(void)

{
  byte bVar1;
  char *pcVar2;
  int iVar3;
  byte *pbVar4;
  byte *pbVar5;
  bool bVar6;
  
  pcVar2 = s_efw_prototype_level3_1011cf28;
  pbVar5 = (byte *)(*(int *)(DAT_10122044 + 0x98) + *(int *)(DAT_10122044 + 0xc));
  pbVar4 = pbVar5;
  do {
    bVar1 = *pcVar2;
    bVar6 = bVar1 < *pbVar4;
    if (bVar1 != *pbVar4) {
LAB_100c5bc3:
      iVar3 = (1 - (uint)bVar6) - (uint)(bVar6 != 0);
      goto LAB_100c5bc8;
    }
    if (bVar1 == 0) break;
    bVar1 = pcVar2[1];
    bVar6 = bVar1 < pbVar4[1];
    if (bVar1 != pbVar4[1]) goto LAB_100c5bc3;
    pcVar2 = pcVar2 + 2;
    pbVar4 = pbVar4 + 2;
  } while (bVar1 != 0);
  iVar3 = 0;
LAB_100c5bc8:
  if (iVar3 == 0) {
    return 2;
  }
  pcVar2 = s_efw_prototype_level2_1011cf10;
  while( true ) {
    bVar1 = *pcVar2;
    bVar6 = bVar1 < *pbVar5;
    if (bVar1 != *pbVar5) break;
    if (bVar1 == 0) {
      return 1;
    }
    bVar1 = pcVar2[1];
    bVar6 = bVar1 < pbVar5[1];
    if (bVar1 != pbVar5[1]) break;
    pcVar2 = pcVar2 + 2;
    pbVar5 = pbVar5 + 2;
    if (bVar1 == 0) {
      return 1;
    }
  }
  return 1 - bVar6 == (uint)(bVar6 != 0);
}



/* ==== 100c5c30 FUN_100c5c30 ==== */

void __cdecl FUN_100c5c30(undefined4 *param_1,int param_2)

{
  int iVar1;
  
  iVar1 = *(int *)(param_2 + 4);
  *param_1 = *(undefined4 *)(iVar1 + 8);
  param_1[1] = *(undefined4 *)(iVar1 + 0xc);
  param_1[2] = *(undefined4 *)(iVar1 + 0x10);
  return;
}



/* ==== 100c5c50 FUN_100c5c50 ==== */

undefined4 __cdecl FUN_100c5c50(int param_1,int param_2)

{
  float10 fVar1;
  float fVar2;
  undefined2 extraout_var;
  undefined4 uVar3;
  undefined2 extraout_var_00;
  undefined2 extraout_var_01;
  undefined2 extraout_var_02;
  float10 fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  undefined1 local_50 [12];
  float local_44;
  float local_40;
  float local_3c;
  undefined1 local_38 [16];
  float local_28;
  
  FUN_100c5c30(&local_74,param_1);
  FUN_100c5c30(&local_68,param_2);
  fVar5 = local_68 - local_74;
  local_58 = local_64 - local_70;
  local_54 = local_60 - local_6c;
  fVar2 = SQRT(fVar5 * fVar5 + local_58 * local_58 + local_54 * local_54);
  if (fVar2 == 0.0) {
    fVar7 = 1.0;
    fVar6 = 0.0;
    fVar5 = 0.0;
  }
  else {
    fVar2 = 1.0 / fVar2;
    fVar7 = local_54 * fVar2;
    fVar6 = local_58 * fVar2;
    fVar5 = fVar5 * fVar2;
  }
  FUN_10001d60(local_50,fVar5,fVar6,fVar7);
  fVar5 = SQRT((local_74 - local_68) * (local_74 - local_68) +
               (local_70 - local_64) * (local_70 - local_64) +
               (local_6c - local_60) * (local_6c - local_60));
  uVar3 = CONCAT22(extraout_var,
                   (ushort)(fVar5 < 512.0) << 8 | (ushort)NAN(fVar5) << 10 |
                   (ushort)(fVar5 == 512.0) << 0xe);
  if (fVar5 < 512.0 || (fVar5 == 512.0) != 0) {
    FUN_100af200(*(int *)(param_1 + 4) + 0x50);
    fVar4 = (float10)FUN_100c9190();
    fVar1 = (float10)1.0471967;
    uVar3 = CONCAT22(extraout_var_00,
                     (ushort)(fVar4 < fVar1) << 8 | (ushort)(NAN(fVar4) || NAN(fVar1)) << 10 |
                     (ushort)(fVar4 == fVar1) << 0xe);
    if (fVar4 < fVar1) {
      FUN_100afc60(&local_74,&local_68,1,*(undefined4 *)(*(int *)(param_1 + 4) + 0x208),local_38);
      uVar3 = CONCAT22(extraout_var_01,
                       (ushort)(local_28 < 1.0) << 8 | (ushort)NAN(local_28) << 10 |
                       (ushort)(local_28 == 1.0) << 0xe);
      if (local_28 < 1.0 != 0) {
        local_3c = local_6c + 40.0;
        local_44 = local_74;
        local_5c = local_68;
        local_54 = local_60 + 40.0;
        local_40 = local_70;
        local_58 = local_64;
        FUN_100afc60(&local_44,&local_5c,1,*(undefined4 *)(*(int *)(param_1 + 4) + 0x208),local_38);
        uVar3 = CONCAT22(extraout_var_02,
                         (ushort)(local_28 < 1.0) << 8 | (ushort)NAN(local_28) << 10 |
                         (ushort)(local_28 == 1.0) << 0xe);
        if (local_28 < 1.0) {
          return uVar3;
        }
      }
      return CONCAT31((int3)((uint)uVar3 >> 8),1);
    }
  }
  return uVar3;
}



/* ==== 100c5e30 FUN_100c5e30 ==== */

undefined4 __cdecl FUN_100c5e30(int param_1)

{
  uint3 uVar1;
  
  uVar1 = (uint3)((uint)param_1 >> 8);
  if (*(int *)(param_1 + 0x2e4) == 8) {
    *(undefined4 *)(param_1 + 0x2e4) = 0;
    return CONCAT31(uVar1,1);
  }
  return (uint)uVar1 << 8;
}



/* ==== 100c5e50 FUN_100c5e50 ==== */

float10 __cdecl FUN_100c5e50(int param_1,int param_2)

{
  float *pfVar1;
  float *pfVar2;
  undefined4 local_18 [3];
  undefined4 local_c [3];
  
  pfVar1 = (float *)FUN_100c5c30(local_18,param_2);
  pfVar2 = (float *)FUN_100c5c30(local_c,param_1);
  return SQRT(((float10)*pfVar2 - (float10)*pfVar1) * ((float10)*pfVar2 - (float10)*pfVar1) +
              ((float10)pfVar2[1] - (float10)pfVar1[1]) * ((float10)pfVar2[1] - (float10)pfVar1[1]))
  ;
}



/* ==== 100c5ea0 monster_refugee ==== */

void __cdecl monster_refugee(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  
                    /* 0xc5ea0  482  monster_refugee */
  if ((param_1 == 0) && (iVar2 = (*DAT_10121e5c)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = iRam00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (puVar3 = (undefined4 *)(*DAT_10121f04)(iVar2,0x3a8), iVar1 = param_1,
     puVar3 != (undefined4 *)0x0)) {
    FUN_10002f30(puVar3);
    *puVar3 = &PTR_LAB_100f93a4;
    puVar3[1] = param_1;
    return;
  }
  iRam00000004 = iVar1;
  return;
}



/* ==== 100c5f10 monster_efw_guard ==== */

void __cdecl monster_efw_guard(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  
                    /* 0xc5f10  459  monster_efw_guard */
  if ((param_1 == 0) && (iVar2 = (*DAT_10121e5c)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = iRam00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (puVar3 = (undefined4 *)(*DAT_10121f04)(iVar2,0x3a8), iVar1 = param_1,
     puVar3 != (undefined4 *)0x0)) {
    FUN_10002f30(puVar3);
    *puVar3 = &PTR_LAB_100f93a4;
    puVar3[1] = param_1;
    return;
  }
  iRam00000004 = iVar1;
  return;
}



/* ==== 100c5f80 FUN_100c5f80 ==== */

void FUN_100c5f80(void)

{
  FUN_100c7590((undefined4 *)&DAT_10132c98);
  return;
}



/* ==== 100c5f90 FUN_100c5f90 ==== */

void FUN_100c5f90(void)

{
  FUN_100c89be(&LAB_100c5fa0);
  return;
}



/* ==== 100c5fb0 FUN_100c5fb0 ==== */

void FUN_100c5fb0(void)

{
  undefined4 *puVar1;
  
  puVar1 = operator_new(100);
  if (puVar1 != (undefined4 *)0x0) {
    DAT_10132cc4 = FUN_100c7670(puVar1);
    FUN_100c75c0(&DAT_10132c98,s_Dingaling_wav_1011d0ac);
    return;
  }
  DAT_10132cc4 = (undefined4 *)0x0;
  FUN_100c75c0(&DAT_10132c98,s_Dingaling_wav_1011d0ac);
  return;
}



/* ==== 100c6000 FUN_100c6000 ==== */

void FUN_100c6000(void)

{
  int iVar1;
  undefined **ppuVar2;
  
  iVar1 = 0;
  if (0 < DAT_1011cf74) {
    ppuVar2 = &PTR_s_models_detaineeMaleT0_mdl_1011cf40;
    do {
      (*DAT_10121e08)(*ppuVar2);
      iVar1 = iVar1 + 1;
      ppuVar2 = ppuVar2 + 1;
    } while (iVar1 < DAT_1011cf74);
  }
  FUN_100c5fb0();
  return;
}



/* ==== 100c6420 FUN_100c6420 ==== */

undefined4 __cdecl FUN_100c6420(uint param_1)

{
  undefined4 uVar1;
  
  FUN_100c5b80();
  uVar1 = FUN_100b9990(param_1);
  return CONCAT31((int3)((uint)uVar1 >> 8),1);
}



/* ==== 100c6440 CRefugee::IdleThink ==== */

/* public: void __thiscall CRefugee::IdleThink(void) */

void __thiscall CRefugee::IdleThink(CRefugee *this)

{
  float fVar1;
  float fVar2;
  float fVar3;
  bool bVar4;
  int iVar5;
  int iVar6;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  undefined4 local_4;
  
                    /* 0xc6440  129  ?IdleThink@CRefugee@@QAEXXZ */
  *(undefined4 *)(*(int *)(this + 4) + 0x138) = 0x3f800000;
  if (*(int *)(*(int *)(this + 4) + 0x170) == 2) {
    local_18 = 0;
    local_14 = 0;
    local_10 = 0;
    local_c = 0;
    local_8 = 0;
    local_4 = 0;
    FUN_100afdc0(*(int *)(this + 4),&local_c,&local_18);
  }
  else {
    FUN_100aec40(0,s_mad_scientist_entity_1011d110);
    FUN_100af1c0(0);
    efw_Player();
  }
  local_c = 0x41800000;
  local_8 = 0x41800000;
  local_4 = 0x42900000;
  local_18 = 0xc1800000;
  local_14 = 0xc1800000;
  local_10 = 0;
  FUN_100afdc0(*(int *)(this + 4),&local_18,&local_c);
  bVar4 = efw_FStrEq((uint *)(*(int *)(DAT_10122044 + 0x98) + *(int *)(*(int *)(this + 4) + 0x1cc)
                               ),s_queue_1011d108);
  iVar5 = efw_Player();
  if ((iVar5 != 0) && (!bVar4)) {
    iVar5 = *(int *)(this + 4);
    iVar6 = efw_Player();
    iVar6 = *(int *)(iVar6 + 4);
    DAT_10132ca8 = DAT_10132ca8 + 1;
    fVar1 = *(float *)(iVar6 + 8) - *(float *)(iVar5 + 8);
    fVar3 = *(float *)(iVar6 + 0xc) - *(float *)(iVar5 + 0xc);
    fVar2 = *(float *)(iVar6 + 0x10) - *(float *)(iVar5 + 0x10);
    fVar1 = SQRT(fVar1 * fVar1 + fVar3 * fVar3 + fVar2 * fVar2);
    (**(code **)(*(int *)this + 8))(0x40400000);
    *(undefined4 *)(*(int *)(this + 4) + 0x108) = 4;
    if ((DAT_10132ca8 % 0x52 == 0) && ((100.0 < fVar1 && (fVar1 < 300.0)))) {
      (**(code **)(*(int *)this + 0x1a8))(3);
      FUN_1005d290((int)this);
      *(undefined4 *)(this + 0x26c) = 1;
      *(undefined4 *)(this + 0x284) = 3;
      *(uint *)(this + 0x2a0) = *(uint *)(this + 0x2a0) & 0xffffffdf;
      iVar5 = efw_Player();
      FUN_10019420(this + 0x168,iVar5);
      FUN_1005d500(this,3,0);
      efw_DebugPrint();
    }
    if ((*(int *)(this + 0x284) == 3) && (fVar1 < 100.0)) {
      (**(code **)(*(int *)this + 0x1a8))(1);
    }
  }
  FUN_1005d160((int *)this);
  return;
}



/* ==== 100c66d0 FUN_100c66d0 ==== */

void FUN_100c66d0(void)

{
  efw_DebugPrint();
  efw_DebugPrint();
  efw_DebugPrint();
  return;
}



/* ==== 100c6700 FUN_100c6700 ==== */

void FUN_100c6700(void)

{
  FUN_100c89be(&LAB_100c6710);
  return;
}



/* ==== 100c6740 FUN_100c6740 ==== */

void FUN_100c6740(void)

{
  efw_DebugPrint();
  efw_DebugPrint();
  FUN_100c8180(0,0x42c80000);
  FUN_100c8180(1,0x42a00000);
  FUN_100c2bf0();
  FUN_100c3370();
  return;
}



/* ==== 100c6780 FUN_100c6780 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_100c6780(void)

{
  undefined1 uVar1;
  undefined3 extraout_var;
  undefined3 extraout_var_00;
  
  uVar1 = FUN_100c5b80();
  if (CONCAT31(extraout_var,uVar1) == 0) {
    DAT_1011d14c = '\x01';
  }
  else if (DAT_1011d14c == '\0') goto LAB_100c67a7;
  FUN_100c6740();
  DAT_1011d14c = '\0';
LAB_100c67a7:
  efw_DebugPrint();
  efw_DebugPrint();
  FUN_100c35e0();
  efwConversation_LoadAll();
  FUN_100c6950();
  _DAT_10134474 = 0;
  DAT_10134868 = 0xffffffff;
  _DAT_10134478 = 0;
  _DAT_1013447c = 0;
  DAT_1013447e = 0;
  DAT_10134870 = 0;
  efw_AddDiary(0,2);
  efw_AddDiary(1,2);
  uVar1 = FUN_100c5b80();
  if ((CONCAT31(extraout_var_00,uVar1) == 1) || (CONCAT31(extraout_var_00,uVar1) == 2)) {
    efw_AddDiary(2,2);
    efw_AddDiary(3,2);
    efw_AddDiary(4,2);
    efw_AddDiary(5,2);
    efw_AddDiary(6,2);
    efw_AddDiary(7,2);
    efw_AddDiary(8,2);
    efw_AddDiary(9,2);
    efw_AddDiary(10,2);
  }
  FUN_100c81a0(0,1);
  FUN_100c7810();
  return;
}



/* ==== 100c6880 efw_DiaryCount ==== */

undefined4 efw_DiaryCount(void)

{
  return DAT_10134870;
}



/* ==== 100c6890 efw_AddDiary ==== */

void __cdecl efw_AddDiary(int param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  
  piVar1 = &DAT_10134444 + DAT_10134870;
  for (piVar2 = &DAT_10134444; (piVar1 != piVar2 && (*piVar2 != param_1)); piVar2 = piVar2 + 1) {
  }
  if (piVar2 == piVar1) {
    *piVar1 = param_1;
    FUN_100c75e0((undefined4 *)&DAT_10132c98);
    if (param_2 == 0) {
      FUN_100c81a0(0,DAT_10134870);
    }
    else if (param_2 == 1) {
      FUN_100c6920(DAT_10134870);
    }
    DAT_10134870 = DAT_10134870 + 1;
    efw_DebugPrint();
  }
  return;
}



/* ==== 100c6910 FUN_100c6910 ==== */

void __cdecl FUN_100c6910(int param_1)

{
  (&DAT_10134474)[param_1] = 1;
  return;
}



/* ==== 100c6920 FUN_100c6920 ==== */

void __cdecl FUN_100c6920(undefined4 param_1)

{
  char cVar1;
  
  cVar1 = FUN_100c7450();
  if (cVar1 != '\0') {
    DAT_10134868 = param_1;
    return;
  }
  FUN_100c81a0(0,param_1);
  DAT_10134868 = 0xffffffff;
  return;
}



/* ==== 100c6950 FUN_100c6950 ==== */

void FUN_100c6950(void)

{
  DAT_1013488c = 0;
  DAT_10134880 = 0;
  FUN_100c7430();
  return;
}



/* ==== 100c6970 FUN_100c6970 ==== */

void __cdecl FUN_100c6970(undefined4 param_1)

{
  DAT_10134888 = param_1;
  return;
}



/* ==== 100c6980 efw_Player ==== */

undefined4 efw_Player(void)

{
  return DAT_10134888;
}



/* ==== 100c69a0 FUN_100c69a0 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_100c69a0(void)

{
  char cVar1;
  ushort uVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  char *pcVar6;
  int iVar7;
  char *pcVar8;
  char *pcVar9;
  
  iVar7 = 0;
  pcVar6 = &DAT_10132cd0;
  do {
    uVar2 = FUN_100c6a50((&DAT_1011d134)[iVar7]);
    if ((char)uVar2 != '\0') {
      efw_DebugPrint();
      efw_DebugPrint();
      uVar4 = 0xffffffff;
      pcVar8 = pcVar6;
      do {
        pcVar9 = pcVar8;
        if (uVar4 == 0) break;
        uVar4 = uVar4 - 1;
        pcVar9 = pcVar8 + 1;
        cVar1 = *pcVar8;
        pcVar8 = pcVar9;
      } while (cVar1 != '\0');
      uVar4 = ~uVar4;
      pcVar8 = pcVar9 + -uVar4;
      pcVar9 = (char *)&DAT_10134480;
      for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
        *(undefined4 *)pcVar9 = *(undefined4 *)pcVar8;
        pcVar8 = pcVar8 + 4;
        pcVar9 = pcVar9 + 4;
      }
      for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        *pcVar9 = *pcVar8;
        pcVar8 = pcVar8 + 1;
        pcVar9 = pcVar9 + 1;
      }
      iVar3 = FUN_100c7420();
      if (iVar3 != 0) {
        _DAT_10134470 = FUN_100c7420();
      }
      if ((int *)(&DAT_10134894)[iVar7] == (int *)0x0) {
        FUN_100c7430();
      }
      else {
        (**(code **)(*(int *)(&DAT_10134894)[iVar7] + 4))();
      }
    }
    pcVar6 = pcVar6 + 1000;
    iVar7 = iVar7 + 1;
    if (0x1013443f < (int)pcVar6) {
      return;
    }
  } while( true );
}



/* ==== 100c6a50 FUN_100c6a50 ==== */

ushort __cdecl FUN_100c6a50(int param_1)

{
  ushort uVar1;
  
  uVar1 = GetAsyncKeyState(param_1);
  return uVar1 & 1;
}



/* ==== 100c6a60 FUN_100c6a60 ==== */

void FUN_100c6a60(void)

{
  char cVar1;
  
  cVar1 = FUN_100c7450();
  if (cVar1 != '\0') {
    FUN_100c69a0();
    return;
  }
  return;
}



/* ==== 100c6a70 FUN_100c6a70 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_100c6a70(void)

{
  float10 fVar1;
  float10 fVar2;
  
  fVar1 = FUN_100c5b60();
  fVar2 = fVar1 - (float10)_DAT_1013486c;
  if (fVar2 < (float10)0.0) {
    _DAT_10134874 = 0.0;
    _DAT_1013486c = (float)fVar1;
    return;
  }
  if ((float10)0.2 < fVar2) {
    fVar2 = (float10)0.2;
  }
  _DAT_10134874 = (float)fVar2;
  _DAT_1013486c = (float)fVar1;
  return;
}



/* ==== 100c6ad0 efw_ThinkHope ==== */

void efw_ThinkHope(void)

{
  int iVar1;
  float10 fVar2;
  float10 fVar3;
  int iVar4;
  
  fVar2 = FUN_100c5b70();
  fVar3 = FUN_100c8190(1);
  fVar3 = fVar3 - (float10)(float)fVar2 * (float10)0.083333336;
  if ((float10)0.0 <= fVar3) {
    if ((float10)100.0 < fVar3) {
      fVar3 = (float10)100.0;
    }
  }
  else {
    fVar3 = (float10)0.0;
  }
  FUN_100c8180(1,(float)fVar3);
  fVar2 = FUN_100c8190(1);
  if (fVar2 <= (float10)0.0) {
    iVar4 = 0x4d;
    iVar1 = efw_Player();
    efw_FailOrNarrate(iVar1,iVar4);
    efw_DebugPrint();
  }
  return;
}



/* ==== 100c6b60 efw_SendHudState ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void efw_SendHudState(void)

{
  undefined1 uVar1;
  int iVar2;
  undefined3 extraout_var;
  int iVar3;
  
  efw_ThinkHope();
  iVar2 = FUN_100c81b0(0);
  iVar2 = (&DAT_10134444)[iVar2];
  FUN_100c81a0(1,iVar2);
  FUN_100c81a0(2,(uint)(byte)(&DAT_10134474)[iVar2]);
  uVar1 = FUN_100c5b80();
  FUN_100c81a0(3,CONCAT31(extraout_var,uVar1));
  FUN_100c6dd0();
  _DAT_10134878 = _DAT_10134878 + 1;
  FUN_100c6a70();
  FUN_100c6a60();
  iVar2 = efw_Player();
  if (iVar2 != 0) {
    iVar3 = -1;
    if (*(int *)(iVar2 + 0x528) != 0) {
      iVar3 = FUN_100c43b0((byte *)(**(int **)(*(int *)(iVar2 + 0x528) + 4) +
                                   *(int *)(DAT_10122044 + 0x98)));
    }
    FUN_100c81a0(4,iVar3);
  }
  FUN_100c6c10();
  efw_TalkScan();
  FUN_100c7d30();
  return;
}



/* ==== 100c6c10 efw_ThinkConversation ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void efw_ThinkConversation(void)

{
  char cVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  undefined1 uVar5;
  int iVar6;
  int iVar7;
  undefined3 extraout_var;
  uint uVar8;
  uint uVar9;
  char *pcVar10;
  char *pcVar11;
  float10 fVar12;
  
  if (DAT_10134880 == '\0') {
    DAT_10134884 = DAT_10134884 + 1;
    if ((0x3c < DAT_10134884) && (DAT_10134868 != -1)) {
      FUN_100c81a0(0,DAT_10134868);
      FUN_100c81a0(5,1);
      DAT_10134868 = -1;
    }
  }
  else {
    fVar12 = FUN_100c5b60();
    if ((float10)_DAT_1011d128 + (float10)_DAT_1013487c <= fVar12) {
      FUN_100c7430();
      FUN_100b94e0();
      uVar8 = 0xffffffff;
      pcVar10 = &DAT_10121c38;
      do {
        pcVar11 = pcVar10;
        if (uVar8 == 0) break;
        uVar8 = uVar8 - 1;
        pcVar11 = pcVar10 + 1;
        cVar1 = *pcVar10;
        pcVar10 = pcVar11;
      } while (cVar1 != '\0');
      uVar8 = ~uVar8;
      pcVar10 = pcVar11 + -uVar8;
      pcVar11 = (char *)&DAT_10134480;
      for (uVar9 = uVar8 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
        *(undefined4 *)pcVar11 = *(undefined4 *)pcVar10;
        pcVar10 = pcVar10 + 4;
        pcVar11 = pcVar11 + 4;
      }
      for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
        *pcVar11 = *pcVar10;
        pcVar10 = pcVar10 + 1;
        pcVar11 = pcVar11 + 1;
      }
    }
    iVar6 = efw_Player();
    if ((iVar6 != 0) && (DAT_1013488c != 0)) {
      iVar6 = *(int *)(DAT_1013488c + 4);
      iVar7 = efw_Player();
      iVar7 = *(int *)(iVar7 + 4);
      fVar2 = *(float *)(iVar7 + 8) - *(float *)(iVar6 + 8);
      fVar3 = *(float *)(iVar7 + 0xc) - *(float *)(iVar6 + 0xc);
      fVar4 = *(float *)(iVar7 + 0x10) - *(float *)(iVar6 + 0x10);
      if (_DAT_1011d130 <= SQRT(fVar3 * fVar3 + fVar2 * fVar2 + fVar4 * fVar4)) {
        efw_DebugPrint();
        FUN_100c7430();
        FUN_100b94e0();
        uVar8 = 0xffffffff;
        pcVar10 = &DAT_10121c38;
        do {
          pcVar11 = pcVar10;
          if (uVar8 == 0) break;
          uVar8 = uVar8 - 1;
          pcVar11 = pcVar10 + 1;
          cVar1 = *pcVar10;
          pcVar10 = pcVar11;
        } while (cVar1 != '\0');
        uVar8 = ~uVar8;
        pcVar10 = pcVar11 + -uVar8;
        pcVar11 = (char *)&DAT_10134480;
        for (uVar9 = uVar8 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
          *(undefined4 *)pcVar11 = *(undefined4 *)pcVar10;
          pcVar10 = pcVar10 + 4;
          pcVar11 = pcVar11 + 4;
        }
        for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
          *pcVar11 = *pcVar10;
          pcVar10 = pcVar10 + 1;
          pcVar11 = pcVar11 + 1;
        }
      }
    }
    DAT_10134884 = 0;
  }
  uVar5 = FUN_100c5b80();
  if (CONCAT31(extraout_var,uVar5) == 0) {
    fVar12 = FUN_100c5b60();
    FUN_100c7740(DAT_10132cc4,(float)fVar12);
  }
  return;
}



/* ==== 100c6d70 FUN_100c6d70 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl
FUN_100c6d70(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6)

{
  int *piVar1;
  
  piVar1 = &DAT_10134894;
  do {
    if ((undefined4 *)*piVar1 != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)*piVar1)(1);
    }
    *piVar1 = 0;
    piVar1 = piVar1 + 1;
  } while ((int)piVar1 < 0x101348ac);
  DAT_10134894 = param_1;
  DAT_10134898 = param_2;
  _DAT_1013489c = param_3;
  _DAT_101348a0 = param_4;
  _DAT_101348a4 = param_5;
  _DAT_101348a8 = param_6;
  return;
}



/* ==== 100c6dd0 FUN_100c6dd0 ==== */

void FUN_100c6dd0(void)

{
  uint uVar1;
  
  (*DAT_10121ec0)(2,DAT_101248fc,0,0);
  (*DAT_10121ec8)(1);
  uVar1 = 0;
  do {
    (*DAT_10121ec8)((int)(char)(&DAT_10134948)[uVar1]);
    uVar1 = uVar1 + 1;
  } while (uVar1 < 0x24);
  (*DAT_10121ec4)();
  return;
}



/* ==== 100c6e20 FUN_100c6e20 ==== */

void __cdecl FUN_100c6e20(char *param_1,char *param_2)

{
  char cVar1;
  uint uVar2;
  uint uVar3;
  char *pcVar4;
  
  if (param_2 == (char *)0x0) {
    *param_1 = '\0';
    return;
  }
  uVar2 = 0xffffffff;
  do {
    pcVar4 = param_2;
    if (uVar2 == 0) break;
    uVar2 = uVar2 - 1;
    pcVar4 = param_2 + 1;
    cVar1 = *param_2;
    param_2 = pcVar4;
  } while (cVar1 != '\0');
  uVar2 = ~uVar2;
  pcVar4 = pcVar4 + -uVar2;
  for (uVar3 = uVar2 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
    *(undefined4 *)param_1 = *(undefined4 *)pcVar4;
    pcVar4 = pcVar4 + 4;
    param_1 = param_1 + 4;
  }
  for (uVar2 = uVar2 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
    *param_1 = *pcVar4;
    pcVar4 = pcVar4 + 1;
    param_1 = param_1 + 1;
  }
  return;
}



/* ==== 100c6e60 efw_ShowMenu ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl
efw_ShowMenu(int param_1,char *param_2,char *param_3,char *param_4,char *param_5,char *param_6,
            char *param_7,char *param_8,undefined4 param_9)

{
  char cVar1;
  undefined4 uVar2;
  undefined1 *puVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  char *pcVar7;
  char *pcVar8;
  char *pcVar9;
  char *pcVar10;
  float10 fVar11;
  uint local_fc8;
  undefined1 auStack_fc0 [4];
  char *pcStack_fbc;
  uint uStack_fb8;
  undefined4 uStack_fb4;
  undefined1 auStack_fb0 [4];
  int iStack_fac;
  undefined4 uStack_fa8;
  undefined4 uStack_fa4;
  char acStack_fa0 [1000];
  char acStack_bb8 [1000];
  char local_7d0 [2000];
  
  local_fc8 = (uint)(param_2 != (char *)0x0);
  if (param_3 != (char *)0x0) {
    local_fc8 = (param_2 != (char *)0x0) + 1;
  }
  if (param_4 != (char *)0x0) {
    local_fc8 = local_fc8 + 1;
  }
  if (param_5 != (char *)0x0) {
    local_fc8 = local_fc8 + 1;
  }
  if (param_6 != (char *)0x0) {
    local_fc8 = local_fc8 + 1;
  }
  if (param_7 != (char *)0x0) {
    local_fc8 = local_fc8 + 1;
  }
  if (param_8 != (char *)0x0) {
    local_fc8 = local_fc8 + 1;
  }
  FUN_100c6e20(&DAT_10132cd0,param_3);
  FUN_100c6e20(&DAT_101330b8,param_4);
  FUN_100c6e20(&DAT_101334a0,param_5);
  FUN_100c6e20(&DAT_10133888,param_6);
  FUN_100c6e20(&DAT_10133c70,param_7);
  FUN_100c6e20(&DAT_10134058,param_8);
  pcVar7 = &DAT_10121c38;
  FUN_100c6d70(0,0,0,0,0,0);
  if (local_fc8 == 0) {
    param_1 = 0;
    efw_DebugPrint();
    DAT_10134880 = 0;
  }
  else {
    if (param_1 != 0) {
      pcVar7 = (char *)(*(int *)(*(int *)(param_1 + 4) + 0x1cc) + *(int *)(DAT_10122044 + 0x98));
    }
    efw_DebugPrint();
    efw_DebugPrint();
    efw_DebugPrint();
    efw_DebugPrint();
    if (DAT_10134890 != param_1) {
      efw_DebugPrint();
      uVar4 = 0xffffffff;
      pcVar10 = &DAT_10121c38;
      do {
        pcVar9 = pcVar10;
        if (uVar4 == 0) break;
        uVar4 = uVar4 - 1;
        pcVar9 = pcVar10 + 1;
        cVar1 = *pcVar10;
        pcVar10 = pcVar9;
      } while (cVar1 != '\0');
      uVar4 = ~uVar4;
      pcVar10 = pcVar9 + -uVar4;
      pcVar9 = (char *)&DAT_10134480;
      for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
        *(undefined4 *)pcVar9 = *(undefined4 *)pcVar10;
        pcVar10 = pcVar10 + 4;
        pcVar9 = pcVar9 + 4;
      }
      for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        *pcVar9 = *pcVar10;
        pcVar10 = pcVar10 + 1;
        pcVar9 = pcVar9 + 1;
      }
    }
    DAT_10134880 = 1;
  }
  if (param_1 != 0) {
    DAT_10134890 = param_1;
  }
  DAT_1013488c = param_1;
  fVar11 = FUN_100c5b60();
  _DAT_1013487c = (float)fVar11;
  _DAT_1011d130 = param_9;
  if (param_2 != (char *)0x0) {
    uVar4 = 0xffffffff;
    do {
      pcVar10 = param_2;
      if (uVar4 == 0) break;
      uVar4 = uVar4 - 1;
      pcVar10 = param_2 + 1;
      cVar1 = *param_2;
      param_2 = pcVar10;
    } while (cVar1 != '\0');
    uVar4 = ~uVar4;
    pcVar10 = pcVar10 + -uVar4;
    pcVar9 = local_7d0;
    for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar10;
      pcVar10 = pcVar10 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
      *pcVar9 = *pcVar10;
      pcVar10 = pcVar10 + 1;
      pcVar9 = pcVar9 + 1;
    }
    iVar6 = -1;
    pcVar10 = (char *)&DAT_10134480;
    do {
      if (iVar6 == 0) break;
      iVar6 = iVar6 + -1;
      cVar1 = *pcVar10;
      pcVar10 = pcVar10 + 1;
    } while (cVar1 != '\0');
    if (iVar6 != -2) {
      uVar4 = 0xffffffff;
      pcVar10 = s_____PREVIOUS_QUESTION__1011c348;
      do {
        pcVar9 = pcVar10;
        if (uVar4 == 0) break;
        uVar4 = uVar4 - 1;
        pcVar9 = pcVar10 + 1;
        cVar1 = *pcVar10;
        pcVar10 = pcVar9;
      } while (cVar1 != '\0');
      uVar4 = ~uVar4;
      iVar6 = -1;
      pcVar10 = local_7d0;
      do {
        pcVar8 = pcVar10;
        if (iVar6 == 0) break;
        iVar6 = iVar6 + -1;
        pcVar8 = pcVar10 + 1;
        cVar1 = *pcVar10;
        pcVar10 = pcVar8;
      } while (cVar1 != '\0');
      pcVar10 = pcVar9 + -uVar4;
      pcVar9 = pcVar8 + -1;
      for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
        *(undefined4 *)pcVar9 = *(undefined4 *)pcVar10;
        pcVar10 = pcVar10 + 4;
        pcVar9 = pcVar9 + 4;
      }
      for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        *pcVar9 = *pcVar10;
        pcVar10 = pcVar10 + 1;
        pcVar9 = pcVar9 + 1;
      }
      uVar4 = 0xffffffff;
      pcVar10 = (char *)&DAT_10134480;
      do {
        pcVar9 = pcVar10;
        if (uVar4 == 0) break;
        uVar4 = uVar4 - 1;
        pcVar9 = pcVar10 + 1;
        cVar1 = *pcVar10;
        pcVar10 = pcVar9;
      } while (cVar1 != '\0');
      uVar4 = ~uVar4;
      iVar6 = -1;
      pcVar10 = local_7d0;
      do {
        pcVar8 = pcVar10;
        if (iVar6 == 0) break;
        iVar6 = iVar6 + -1;
        pcVar8 = pcVar10 + 1;
        cVar1 = *pcVar10;
        pcVar10 = pcVar8;
      } while (cVar1 != '\0');
      pcVar10 = pcVar9 + -uVar4;
      pcVar9 = pcVar8 + -1;
      for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
        *(undefined4 *)pcVar9 = *(undefined4 *)pcVar10;
        pcVar10 = pcVar10 + 4;
        pcVar9 = pcVar9 + 4;
      }
      for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        *pcVar9 = *pcVar10;
        pcVar10 = pcVar10 + 1;
        pcVar9 = pcVar9 + 1;
      }
    }
    param_2 = local_7d0;
  }
  (*DAT_10121ec0)(2,DAT_101248f8,0,0);
  (*DAT_10121ec8)(0xff);
  (*DAT_10121ec4)();
  uVar4 = 0xffffffff;
  pcVar10 = pcVar7;
  do {
    if (uVar4 == 0) break;
    uVar4 = uVar4 - 1;
    cVar1 = *pcVar10;
    pcVar10 = pcVar10 + 1;
  } while (cVar1 != '\0');
  uVar4 = ~uVar4 - 1;
  pcStack_fbc = (char *)0x0;
  uStack_fb8 = 0;
  uStack_fb4 = 0;
  uVar2 = FUN_100bbb30(auStack_fc0,uVar4,'\x01');
  if ((char)uVar2 != '\0') {
    pcVar10 = pcVar7;
    pcVar9 = pcStack_fbc;
    for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar10;
      pcVar10 = pcVar10 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar5 = uVar4 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
      *pcVar9 = *pcVar10;
      pcVar10 = pcVar10 + 1;
      pcVar9 = pcVar9 + 1;
    }
    pcStack_fbc[uVar4] = '\0';
    uStack_fb8 = uVar4;
  }
  puVar3 = FUN_100b89a0(auStack_fb0,auStack_fc0);
  pcVar10 = *(char **)(puVar3 + 4);
  if (pcVar10 == (char *)0x0) {
    pcVar10 = "";
  }
  uVar4 = 0xffffffff;
  do {
    pcVar9 = pcVar10;
    if (uVar4 == 0) break;
    uVar4 = uVar4 - 1;
    pcVar9 = pcVar10 + 1;
    cVar1 = *pcVar10;
    pcVar10 = pcVar9;
  } while (cVar1 != '\0');
  uVar4 = ~uVar4;
  iVar6 = 0;
  pcVar10 = pcVar9 + -uVar4;
  pcVar9 = acStack_bb8;
  for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined4 *)pcVar9 = *(undefined4 *)pcVar10;
    pcVar10 = pcVar10 + 4;
    pcVar9 = pcVar9 + 4;
  }
  for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
    *pcVar9 = *pcVar10;
    pcVar10 = pcVar10 + 1;
    pcVar9 = pcVar9 + 1;
  }
  if (iStack_fac != 0) {
    cVar1 = *(char *)(iStack_fac + -1);
    if ((cVar1 == '\0') || (cVar1 == -1)) {
      FUN_100c8ce6((char *)(iStack_fac + -1));
    }
    else {
      *(char *)(iStack_fac + -1) = cVar1 + -1;
    }
  }
  iStack_fac = 0;
  uStack_fa8 = 0;
  uStack_fa4 = 0;
  if (pcStack_fbc != (char *)0x0) {
    cVar1 = pcStack_fbc[-1];
    if ((cVar1 == '\0') || (cVar1 == -1)) {
      FUN_100c8ce6(pcStack_fbc + -1);
    }
    else {
      pcStack_fbc[-1] = cVar1 + -1;
    }
  }
  if (local_fc8 != 0) {
    do {
      if ((pcVar7 == &DAT_10121c38) || (*param_2 == '\0')) {
        FUN_100c8506(acStack_fa0,&DAT_10105438);
      }
      else {
        FUN_100c8506(acStack_fa0,(byte *)s__s___s_1011d2e0);
      }
      if (param_2 == &DAT_10121c38) {
        uVar4 = 0xffffffff;
        pcVar10 = &DAT_10121c38;
        do {
          pcVar9 = pcVar10;
          if (uVar4 == 0) break;
          uVar4 = uVar4 - 1;
          pcVar9 = pcVar10 + 1;
          cVar1 = *pcVar10;
          pcVar10 = pcVar9;
        } while (cVar1 != '\0');
        uVar4 = ~uVar4;
        pcVar10 = pcVar9 + -uVar4;
        pcVar9 = acStack_fa0;
        for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
          *(undefined4 *)pcVar9 = *(undefined4 *)pcVar10;
          pcVar10 = pcVar10 + 4;
          pcVar9 = pcVar9 + 4;
        }
        for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
          *pcVar9 = *pcVar10;
          pcVar10 = pcVar10 + 1;
          pcVar9 = pcVar9 + 1;
        }
      }
      if (iVar6 == 0) {
        FUN_100c7380(0,acStack_fa0);
      }
      else {
        if (iVar6 == 1) {
          FUN_100c7380(1,param_3);
        }
        else if (iVar6 == 2) {
          FUN_100c7380(2,param_4);
        }
        else if (iVar6 == 3) {
          FUN_100c7380(3,param_5);
        }
        else if (iVar6 == 4) {
          FUN_100c7380(4,param_6);
        }
        else if (iVar6 == 5) {
          FUN_100c7380(5,param_7);
        }
        else {
          if (iVar6 != 6) goto LAB_100c7367;
          FUN_100c7380(6,param_8);
        }
        efw_DebugPrint();
      }
LAB_100c7367:
      iVar6 = iVar6 + 1;
    } while (iVar6 < (int)local_fc8);
  }
  return;
}



/* ==== 100c7380 FUN_100c7380 ==== */

void __cdecl FUN_100c7380(undefined4 param_1,char *param_2)

{
  char cVar1;
  char *pcVar2;
  uint uVar3;
  size_t sVar4;
  size_t _Count;
  char local_20 [32];
  
  uVar3 = 0xffffffff;
  pcVar2 = param_2;
  do {
    if (uVar3 == 0) break;
    uVar3 = uVar3 - 1;
    cVar1 = *pcVar2;
    pcVar2 = pcVar2 + 1;
  } while (cVar1 != '\0');
  for (sVar4 = ~uVar3 - 1; 0 < (int)sVar4; sVar4 = sVar4 - _Count) {
    _Count = sVar4;
    if (0x1e < (int)sVar4) {
      _Count = 0x1e;
    }
    _strncpy(local_20,param_2,_Count);
    local_20[_Count] = '\0';
    for (pcVar2 = local_20; pcVar2 != local_20 + _Count; pcVar2 = pcVar2 + 1) {
      if (*pcVar2 == -0x6e) {
        *pcVar2 = '\'';
      }
    }
    (*DAT_10121ec0)(2,DAT_101248f8,0,0);
    (*DAT_10121ec8)(param_1);
    (*DAT_10121ee0)(local_20);
    (*DAT_10121ec4)();
    param_2 = param_2 + _Count;
  }
  return;
}



/* ==== 100c7420 FUN_100c7420 ==== */

undefined4 FUN_100c7420(void)

{
  return DAT_1013488c;
}



/* ==== 100c7430 FUN_100c7430 ==== */

void FUN_100c7430(void)

{
  efw_ShowMenu(0,(char *)0x0,(char *)0x0,(char *)0x0,(char *)0x0,(char *)0x0,(char *)0x0,(char *)0x0
               ,0x43480000);
  return;
}



/* ==== 100c7450 FUN_100c7450 ==== */

undefined1 FUN_100c7450(void)

{
  return DAT_10134880;
}



/* ==== 100c7490 FUN_100c7490 ==== */

undefined1 FUN_100c7490(void)

{
  return DAT_101348ac;
}



/* ==== 100c74a0 FUN_100c74a0 ==== */

void __cdecl FUN_100c74a0(undefined4 param_1)

{
  int iVar1;
  
  for (iVar1 = FUN_100aec40(0,param_1); iVar1 != 0; iVar1 = FUN_100aec40(iVar1,param_1)) {
    *(undefined4 *)(*(int *)(iVar1 + 4) + 0x138) = 0;
    *(float *)(*(int *)(iVar1 + 4) + 0x104) = *DAT_10122044 + 0.05;
    *(undefined4 *)(*(int *)(iVar1 + 4) + 0x108) = 0;
    FUN_100afe10(iVar1,*(int *)(iVar1 + 4) + 8);
  }
  return;
}



/* ==== 100c7510 FUN_100c7510 ==== */

void __cdecl FUN_100c7510(byte param_1)

{
  int iVar1;
  
  FUN_100c81a0(6,(uint)param_1);
  DAT_101348ac = param_1;
  iVar1 = efw_Player();
  *(uint *)(*(int *)(iVar1 + 4) + 0x108) = (-(uint)(param_1 != 0) & 0xfffffffd) + 3;
  if (param_1 != 0) {
    FUN_100c74a0(s_monster_patrol_guard_1011ce50);
  }
  return;
}



/* ==== 100c7590 FUN_100c7590 ==== */

void __fastcall FUN_100c7590(undefined4 *param_1)

{
  *param_1 = 0;
  *(undefined1 *)(param_1 + 1) = 0;
  param_1[2] = 100;
  param_1[3] = 10;
  return;
}



/* ==== 100c75c0 FUN_100c75c0 ==== */

void __thiscall FUN_100c75c0(void *this,undefined4 param_1)

{
  *(undefined4 *)this = param_1;
  (*DAT_10121e0c)(param_1);
  return;
}



/* ==== 100c75e0 FUN_100c75e0 ==== */

void __fastcall FUN_100c75e0(undefined4 *param_1)

{
  int iVar1;
  int iVar2;
  
  if ((byte *)*param_1 == (byte *)0x0) {
    efw_DebugPrint();
    return;
  }
  iVar1 = FUN_100af030(0,(byte *)*param_1);
  iVar2 = _rand();
  if (iVar1 != 0) {
    FUN_10094b20((void *)*param_1,*(undefined4 *)(*(int *)(iVar1 + 4) + 0x208),3,(void *)*param_1,
                 0x3f800000,0x3fa00000,0,100 - iVar2 % 0x14);
    efw_DebugPrint();
    return;
  }
  iVar1 = efw_Player();
  FUN_10094bb0(*(undefined4 *)(*(int *)(iVar1 + 4) + 0x208),(void *)*param_1);
  return;
}



/* ==== 100c7670 FUN_100c7670 ==== */

undefined4 * __fastcall FUN_100c7670(undefined4 *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  FUN_100c7590(param_1);
  iVar1 = 4;
  puVar2 = param_1 + 4;
  do {
    FUN_100c7590(puVar2);
    puVar2 = puVar2 + 4;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  param_1[0x14] = 0x42f00000;
  param_1[0x15] = 0x40a00000;
  param_1[0x16] = 0;
  param_1[0x17] = 0;
  FUN_100c75c0(param_1,s_Ann_RAR_124_wav_1011d410);
  FUN_100c75c0(param_1 + 4,s_Ann_HAM_103_wav_1011d400);
  FUN_100c75c0(param_1 + 8,s_Ann_TRE_046_wav_1011d3f0);
  FUN_100c75c0(param_1 + 0xc,s_Ann_PHA_216_wav_1011d3e0);
  FUN_100c75c0(param_1 + 0x10,s_callToPrayer_wav_1011d3cc);
  *(undefined1 *)(param_1 + 0x18) = 0;
  return param_1;
}



/* ==== 100c7740 FUN_100c7740 ==== */

void __thiscall FUN_100c7740(void *this,float param_1)

{
  float fVar1;
  undefined4 *puVar2;
  int iVar3;
  float10 fVar4;
  
  fVar4 = (float10)FUN_100c8a0a(this);
  if ((fVar4 <= (float10)0.5) && (5.0 < *(float *)((int)this + 0x54))) {
    puVar2 = this;
    if (*(char *)((int)this + 0x60) != '\x01') {
      puVar2 = (undefined4 *)((*(int *)((int)this + 0x5c) + 1) * 0x10 + (int)this);
    }
    FUN_100c75e0(puVar2);
    efw_DebugPrint();
    iVar3 = *(int *)((int)this + 0x5c) + 1;
    *(undefined4 *)((int)this + 0x54) = 0;
    *(int *)((int)this + 0x5c) = iVar3;
    if (3 < iVar3) {
      *(undefined4 *)((int)this + 0x5c) = 0;
    }
  }
  fVar1 = *(float *)((int)this + 0x58);
  *(float *)((int)this + 0x58) = param_1;
  *(float *)((int)this + 0x54) = (param_1 - fVar1) + *(float *)((int)this + 0x54);
  return;
}



/* ==== 100c77c0 FUN_100c77c0 ==== */

void __fastcall FUN_100c77c0(undefined4 *param_1)

{
  *(undefined1 *)(param_1 + 0x18) = 1;
  FUN_100c75e0(param_1);
  param_1[0x15] = 0;
  return;
}



/* ==== 100c77e0 FUN_100c77e0 ==== */

void __fastcall FUN_100c77e0(int param_1)

{
  *(undefined1 *)(param_1 + 0x60) = 0;
  return;
}



/* ==== 100c7810 FUN_100c7810 ==== */

void FUN_100c7810(void)

{
  DAT_10134940 = 0;
  return;
}



/* ==== 100c7820 FUN_100c7820 ==== */

bool FUN_100c7820(void)

{
  return 0 < DAT_10134940;
}



/* ==== 100c7830 efw_TalkScan ==== */

void efw_TalkScan(void)

{
  byte bVar1;
  int iVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  int iVar6;
  int *piVar7;
  int *piVar8;
  uint uVar9;
  int *piVar10;
  int iVar11;
  int iVar12;
  void *this;
  void *this_00;
  void *this_01;
  void *this_02;
  byte *pbVar13;
  char *pcVar14;
  bool bVar15;
  undefined4 local_ec;
  undefined4 local_e8;
  undefined4 local_e4;
  int *local_e0;
  float fStack_dc;
  float fStack_d8;
  float fStack_d4;
  undefined4 uStack_d0;
  undefined4 uStack_cc;
  undefined4 uStack_c8;
  undefined4 uStack_c4;
  undefined4 uStack_c0;
  undefined4 uStack_bc;
  float fStack_b4;
  float fStack_b0;
  undefined4 uStack_ac;
  undefined4 uStack_a8;
  undefined4 uStack_a4;
  undefined4 local_a0;
  undefined4 uStack_9c;
  undefined4 uStack_98;
  float fStack_90;
  float fStack_8c;
  float fStack_88;
  float fStack_84;
  float fStack_80;
  undefined4 local_7c;
  undefined4 uStack_78;
  undefined4 uStack_74;
  undefined1 auStack_70 [16];
  float fStack_60;
  undefined1 auStack_38 [16];
  float fStack_28;
  
  DAT_10134940 = 0;
  piVar7 = (int *)efw_Player();
  iVar12 = piVar7[1];
  local_ec = *(undefined4 *)(iVar12 + 8);
  local_e8 = *(undefined4 *)(iVar12 + 0xc);
  local_e4 = *(undefined4 *)(iVar12 + 0x10);
  local_e0 = piVar7;
  piVar8 = (int *)FUN_100aeba0(0,&local_ec,0x42f60000);
  do {
    if ((piVar8 == (int *)0x0) || (2 < DAT_10134940)) {
      iVar12 = efw_Player();
      (*DAT_10121ec0)(1,DAT_10124908,0,*(undefined4 *)(*(int *)(iVar12 + 4) + 0x208));
      (*DAT_10121ec8)(DAT_10134940 != 0);
      (*DAT_10121ec4)();
      return;
    }
    if (piVar8 != piVar7) {
      pbVar13 = (byte *)(*(int *)((int)DAT_10122044 + 0x98) + (int)*(void **)piVar8[1]);
      uVar9 = FUN_100d8b20(*(void **)piVar8[1],pbVar13,(byte *)s_monster_refugee_10114fb4);
      if ((uVar9 == 0) ||
         (uVar9 = FUN_100d8b20(this,pbVar13,(byte *)s_monster_barney_10115c10), uVar9 == 0)) {
        iVar12 = piVar7[1];
        iVar2 = piVar8[1];
        fVar3 = *(float *)(iVar12 + 8) - *(float *)(iVar2 + 8);
        fVar5 = *(float *)(iVar12 + 0xc) - *(float *)(iVar2 + 0xc);
        fVar4 = *(float *)(iVar12 + 0x10) - *(float *)(iVar2 + 0x10);
        if (SQRT(fVar3 * fVar3 + fVar5 * fVar5 + fVar4 * fVar4) < 123.0) {
          iVar12 = 0;
LAB_100c7b80:
          _strncpy(&DAT_101348b4 + DAT_10134940 * 0x30,
                   (char *)(*(int *)(piVar8[1] + 0x1cc) + *(int *)((int)DAT_10122044 + 0x98)),0x1d);
          iVar6 = DAT_10134940;
          iVar11 = DAT_10134940 * 0x30;
          (&DAT_101348d1)[iVar11] = 0;
          iVar2 = piVar8[1];
          (&DAT_101348d4)[iVar6 * 0xc] = *(float *)(iVar2 + 8);
          (&DAT_101348d8)[iVar6 * 0xc] = *(undefined4 *)(iVar2 + 0xc);
          (&DAT_101348dc)[iVar6 * 0xc] = *(undefined4 *)(iVar2 + 0x10);
          if (iVar12 == 0) {
            (&DAT_101348dc)[iVar6 * 0xc] = (float)(&DAT_101348dc)[iVar6 * 0xc] + 64.0;
          }
          else if (iVar12 == 1) {
            iVar2 = piVar8[1];
            fStack_90 = *(float *)(iVar2 + 0xd4) + *(float *)(iVar2 + 200);
            fStack_8c = *(float *)(iVar2 + 0xd8) + *(float *)(iVar2 + 0xcc);
            fStack_dc = (*(float *)(iVar2 + 0xc4) + *(float *)(iVar2 + 0xd0)) * 0.5;
            fStack_d8 = fStack_90 * 0.5;
            (&DAT_101348d4)[iVar6 * 0xc] = fStack_dc;
            fStack_d4 = fStack_8c * 0.5;
            (&DAT_101348d8)[iVar6 * 0xc] = fStack_d8;
            (&DAT_101348dc)[iVar6 * 0xc] = fStack_d4;
          }
          *(int *)(&DAT_101348b0 + iVar11) = iVar12;
          DAT_10134940 = DAT_10134940 + 1;
          piVar7 = local_e0;
        }
      }
      else {
        uVar9 = FUN_100d8b20(this_00,pbVar13,(byte *)s_efw_Marker_1011c9ac);
        if (uVar9 == 0) {
          uVar9 = FUN_100d8b20(*(void **)((int)DAT_10122044 + 0x98),
                               (byte *)(*(int *)(piVar8[1] + 0x1cc) +
                                       (int)*(void **)((int)DAT_10122044 + 0x98)),
                               (byte *)s_efw_hiding_place_1011d458);
          if (uVar9 == 0) {
            (**(code **)(*piVar7 + 0x214))(&local_a0);
            uStack_d0 = local_a0;
            uStack_cc = uStack_9c;
            uStack_c8 = uStack_98;
            iVar12 = piVar8[1];
            fStack_b4 = *(float *)(iVar12 + 200) + *(float *)(iVar12 + 0xd4);
            fStack_b0 = *(float *)(iVar12 + 0xcc) + *(float *)(iVar12 + 0xd8);
            fStack_88 = (*(float *)(iVar12 + 0xc4) + *(float *)(iVar12 + 0xd0)) * 0.5;
            fStack_84 = fStack_b4 * 0.5;
            fStack_80 = fStack_b0 * 0.5;
            FUN_100afc60(&uStack_d0,&fStack_88,0,*(undefined4 *)(piVar7[1] + 0x208),auStack_70);
            if (fStack_60 < 0.97) goto LAB_100c7cd2;
          }
          pcVar14 = s_efw_PliersMarker_1011d444;
          pbVar13 = (byte *)(*(int *)(piVar8[1] + 0x1c8) + *(int *)((int)DAT_10122044 + 0x98));
          do {
            bVar1 = *pbVar13;
            bVar15 = bVar1 < (byte)*pcVar14;
            if (bVar1 != *pcVar14) {
LAB_100c7a2a:
              iVar12 = (1 - (uint)bVar15) - (uint)(bVar15 != 0);
              goto LAB_100c7a2f;
            }
            if (bVar1 == 0) break;
            bVar1 = pbVar13[1];
            bVar15 = bVar1 < (byte)pcVar14[1];
            if (bVar1 != pcVar14[1]) goto LAB_100c7a2a;
            pbVar13 = pbVar13 + 2;
            pcVar14 = pcVar14 + 2;
          } while (bVar1 != 0);
          iVar12 = 0;
LAB_100c7a2f:
          if (iVar12 != 0) {
LAB_100c7a97:
            iVar12 = 1;
            goto LAB_100c7b80;
          }
          pcVar14 = s_weapon_efw_Pliers_101054a4;
          piVar10 = (int *)efw_Player();
          uVar9 = FUN_100c2f70(piVar10,(byte *)pcVar14);
          if ((char)uVar9 == '\0') {
            iVar12 = 1;
            goto LAB_100c7b80;
          }
        }
        else {
          uVar9 = FUN_100d8b20(this_01,pbVar13,(byte *)s_func_door_rotating_101076a8);
          if (uVar9 == 0) {
            uVar9 = FUN_100d8b20(DAT_10122044,
                                 (byte *)(*(int *)(piVar8[1] + 0x1cc) +
                                         *(int *)((int)DAT_10122044 + 0x98)),
                                 (byte *)s_efw_cage_door_1011cc78);
            if (uVar9 == 0) goto LAB_100c7a97;
          }
          else {
            iVar12 = FUN_100d8bf0(pbVar13,s_weapon_efw_1011d438,&DAT_0000000a);
            if (iVar12 == 0) {
              uVar9 = FUN_100d8b20(this_02,pbVar13,(byte *)s_weapon_efw_Pliers_101054a4);
              if (uVar9 == 0) {
                (**(code **)(*piVar7 + 0x214))(&local_7c);
                uStack_ac = local_7c;
                uStack_a8 = uStack_78;
                uStack_a4 = uStack_74;
                iVar12 = piVar8[1];
                uStack_c4 = *(undefined4 *)(iVar12 + 8);
                uStack_c0 = *(undefined4 *)(iVar12 + 0xc);
                uStack_bc = *(undefined4 *)(iVar12 + 0x10);
                FUN_100afc60(&uStack_ac,&uStack_c4,0,*(undefined4 *)(piVar7[1] + 0x208),auStack_38);
                if (fStack_28 < 0.97) goto LAB_100c7cd2;
              }
              if (piVar8[0x37] == 0) {
                iVar12 = FUN_100c43b0(pbVar13);
                iVar12 = iVar12 + 100;
                if (iVar12 != -1) goto LAB_100c7b80;
              }
            }
          }
        }
      }
    }
LAB_100c7cd2:
    piVar8 = (int *)FUN_100aeba0((int)piVar8,&local_ec,0x42f60000);
  } while( true );
}



/* ==== 100c7d30 FUN_100c7d30 ==== */

void FUN_100c7d30(void)

{
  int iVar1;
  
  iVar1 = efw_Player();
  (*DAT_10121ec0)(1,DAT_10124904,0,*(undefined4 *)(*(int *)(iVar1 + 4) + 0x208));
  (*DAT_10121ec8)(DAT_10134940);
  iVar1 = 0;
  if (0 < DAT_10134940 * 0x30) {
    do {
      (*DAT_10121ec8)((int)(char)(&DAT_101348b0)[iVar1]);
      iVar1 = iVar1 + 1;
    } while (iVar1 < DAT_10134940 * 0x30);
  }
  (*DAT_10121ec4)();
  return;
}



/* ==== 100c7da0 FUN_100c7da0 ==== */

void __cdecl FUN_100c7da0(uint *param_1,int *param_2,undefined4 param_3)

{
  bool bVar1;
  char cVar2;
  undefined1 uVar3;
  int iVar4;
  uint uVar5;
  undefined3 extraout_var;
  int *piVar6;
  uint *puVar7;
  uint *puVar8;
  uint uStack_18;
  char acStack_14 [4];
  char acStack_10 [4];
  char acStack_c [4];
  char acStack_8 [8];
  
  iVar4 = (**(code **)(*param_2 + 0xcc))();
  if (iVar4 != 0) {
    bVar1 = efw_FStrEq(param_1,s_efw_kitchen_door_1011d718);
    if (bVar1) {
      uVar5 = FUN_100c2f70(param_2,(byte *)s_weapon_efw_Pliers_101054a4);
      if ((char)uVar5 == '\0') {
        cVar2 = FUN_100c3430(s_OnKitchenRoster_1011d708);
        if (cVar2 == '\0') {
          efwConversation_Squark((byte *)s_Kitchen_Guard_101031e0,s_Where_do_you_think_you_re_going__1011d618,
                       4);
          return;
        }
        uVar3 = FUN_100c5b80();
        if (CONCAT31(extraout_var,uVar3) == 0) {
          cVar2 = FUN_100c3430(s_HasEnteredTheKitchenForTheFirstT_1011d6e4);
          if (cVar2 == '\0') {
            efwConversation_AddKeyword(s_HasEnteredTheKitchenForTheFirstT_1011d6e4,1);
            efwConversation_Squark((byte *)s_Kitchen_Guard_101031e0,
                         s_RAR_124__Go_in__you_re_on_the_ro_1011d6bc,4);
          }
          FUN_10098c10((uint *)s_efw_actual_kitchen_door_1011d6a4,param_2,param_3,1,0);
          return;
        }
        if (CONCAT31(extraout_var,uVar3) == 2) {
          cVar2 = FUN_100c3430(s_HasEnteredTheKitchenInChapterTwo_1011d680);
          if (cVar2 == '\0') {
            efwConversation_AddKeyword(s_HasEnteredTheKitchenInChapterTwo_1011d680,1);
            efwConversation_Squark((byte *)s_Kitchen_Guard_101031e0,s_Back_again__OK__go_on_in__1011d664,4);
            FUN_10098c10((uint *)s_efw_actual_kitchen_door_1011d6a4,param_2,param_3,1,0);
            return;
          }
        }
      }
      else {
        iVar4 = (**(code **)(*param_2 + 0xcc))();
        if ((iVar4 != 0) && (param_2 != (int *)0x0)) {
          efw_FailOrNarrate((int)param_2,0x3c);
          return;
        }
      }
    }
    else {
      bVar1 = efw_FStrEq(param_1,s_efw_2ndcompound_entry_1011d600);
      if (bVar1) {
        cVar2 = FUN_100c3430(s_Player_sIDTagOnFence_1011c678);
        if (cVar2 == '\0') {
          cVar2 = FUN_100c3430(s_HeardCompoundGateGuardOnce_1011d5d0);
          if (cVar2 == '\0') {
            efwConversation_Squark((byte *)s_efw_compound_gate_guard_1010318c,
                         s_Hold_it__To_enter_the_main_compo_1011d53c,4);
            efwConversation_AddKeyword(s_HeardCompoundGateGuardOnce_1011d5d0,1);
            return;
          }
          efwConversation_Squark((byte *)s_efw_compound_gate_guard_1010318c,
                       s_No_tag_on_the_board__no_entry__L_1011d4f4,4);
          return;
        }
        acStack_10[0] = s_efw_compound_gate_1011d5ec[8];
        acStack_10[1] = s_efw_compound_gate_1011d5ec[9];
        acStack_10[2] = s_efw_compound_gate_1011d5ec[10];
        acStack_10[3] = s_efw_compound_gate_1011d5ec[0xb];
        uStack_18._0_1_ = s_efw_compound_gate_1011d5ec[0];
        uStack_18._1_1_ = s_efw_compound_gate_1011d5ec[1];
        uStack_18._2_1_ = s_efw_compound_gate_1011d5ec[2];
        uStack_18._3_1_ = s_efw_compound_gate_1011d5ec[3];
        acStack_14[0] = s_efw_compound_gate_1011d5ec[4];
        acStack_14[1] = s_efw_compound_gate_1011d5ec[5];
        acStack_14[2] = s_efw_compound_gate_1011d5ec[6];
        acStack_14[3] = s_efw_compound_gate_1011d5ec[7];
        acStack_c[0] = s_efw_compound_gate_1011d5ec[0xc];
        acStack_c[1] = s_efw_compound_gate_1011d5ec[0xd];
        acStack_c[2] = s_efw_compound_gate_1011d5ec[0xe];
        acStack_c[3] = s_efw_compound_gate_1011d5ec[0xf];
        acStack_8[0] = s_efw_compound_gate_1011d5ec[0x10];
        acStack_8[1] = s_efw_compound_gate_1011d5ec[0x11];
        piVar6 = (int *)FUN_100af060(0,(byte *)&uStack_18,0);
        iVar4 = (**(code **)(*piVar6 + 0x7c))();
        if (iVar4 != 0) {
          FUN_10098c10(&uStack_18,param_2,param_3,1,0);
          return;
        }
      }
      else {
        bVar1 = efw_FStrEq(param_1,s_efw_1stcompound_entry_1011d4dc);
        if (bVar1) {
          cVar2 = FUN_100c3430(s_Player_sIDTagOnFence_1011c678);
          if (cVar2 == '\0') {
            puVar7 = (uint *)s_efw_1stcompound_gate_1011d4c4;
            puVar8 = &uStack_18;
            for (iVar4 = 5; iVar4 != 0; iVar4 = iVar4 + -1) {
              *puVar8 = *puVar7;
              puVar7 = puVar7 + 1;
              puVar8 = puVar8 + 1;
            }
            *(char *)puVar8 = (char)*puVar7;
            FUN_10098c10(&uStack_18,param_2,param_3,1,0);
            return;
          }
          efwConversation_Squark((byte *)s_efw_compound_gate_guard_1010318c,
                       s_The_gate_doesn_t_open_til_you_co_1011d480,4);
          return;
        }
        bVar1 = efw_FStrEq(param_1,s_efw_approach_bin_1011d46c);
        if (bVar1) {
          iVar4 = (**(code **)(*param_2 + 0xcc))();
          if ((iVar4 != 0) && (param_2 != (int *)0x0)) {
            uVar5 = FUN_100c2f70(param_2,(byte *)s_weapon_efw_Pliers_101054a4);
            if ((char)uVar5 == '\0') {
              FUN_1007f800(param_2,0x101054a4);
              efw_FailOrNarrate((int)param_2,0x45);
              FUN_100c4d70(15.0);
            }
          }
        }
      }
    }
  }
  return;
}



/* ==== 100c80d0 efw_DebugPrint ==== */

void efw_DebugPrint(void)

{
  byte *in_stack_00001004;
  
  FUN_100c9600();
  FUN_100c96fb(&stack0x00000000,in_stack_00001004,(undefined4 *)&stack0x00001008);
  OutputDebugStringA(&stack0x00000000);
  return;
}



/* ==== 100c8110 FUN_100c8110 ==== */

undefined4 __cdecl FUN_100c8110(byte *param_1,byte *param_2)

{
  byte bVar1;
  int iVar2;
  bool bVar3;
  
  while( true ) {
    bVar1 = *param_1;
    bVar3 = bVar1 < *param_2;
    if (bVar1 != *param_2) break;
    if (bVar1 == 0) {
      return 1;
    }
    bVar1 = param_1[1];
    bVar3 = bVar1 < param_2[1];
    if (bVar1 != param_2[1]) break;
    param_1 = param_1 + 2;
    param_2 = param_2 + 2;
    if (bVar1 == 0) {
      return 1;
    }
  }
  iVar2 = (1 - (uint)bVar3) - (uint)(bVar3 != 0);
  return CONCAT31((int3)((uint)iVar2 >> 8),iVar2 == 0);
}



/* ==== 100c8160 efw_FStrEq ==== */

bool __cdecl efw_FStrEq(uint *param_1,char *param_2)

{
  uint *puVar1;
  
  puVar1 = FUN_100c88c0(param_1,param_2);
  return puVar1 != (uint *)0x0;
}



/* ==== 100c8180 FUN_100c8180 ==== */

void __cdecl FUN_100c8180(int param_1,undefined4 param_2)

{
  *(undefined4 *)(&DAT_10134948 + param_1 * 4) = param_2;
  return;
}



/* ==== 100c8190 FUN_100c8190 ==== */

float10 __cdecl FUN_100c8190(int param_1)

{
  return (float10)*(float *)(&DAT_10134948 + param_1 * 4);
}



/* ==== 100c81a0 FUN_100c81a0 ==== */

void __cdecl FUN_100c81a0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(&DAT_10134950 + param_1 * 4) = param_2;
  return;
}



/* ==== 100c81b0 FUN_100c81b0 ==== */

undefined4 __cdecl FUN_100c81b0(int param_1)

{
  return *(undefined4 *)(&DAT_10134950 + param_1 * 4);
}



/* ==== 100c81d0 efw_FailOrNarrate ==== */

void __cdecl efw_FailOrNarrate(int param_1,int param_2)

{
  char *pcVar1;
  
  if (param_2 == 0x43) {
    FUN_100c4d70(15.0);
  }
  else if (param_2 == 0x3f) {
    FUN_100c4d70(15.0);
  }
  else {
    if (((((param_2 == 0x45) || (param_2 == 0x3d)) || (param_2 == 0x3e)) ||
        ((param_2 == 0x40 || (param_2 == 0x41)))) ||
       ((param_2 == 0x42 || ((param_2 == 0x3c || (param_2 == 0x44)))))) {
      pcVar1 = s_<error>_1011df9c;
      if ((0x3b < param_2) && (param_2 - 0x3cU < 0x16)) {
        pcVar1 = *(char **)(s_Where_do_you_think_you_re_going__1011d618 + param_2 * 4 + 0x24);
      }
      efw_ShowMenu(0,pcVar1,(char *)0x0,(char *)0x0,(char *)0x0,(char *)0x0,(char *)0x0,(char *)0x0,
                   0x43480000);
      return;
    }
    if (param_2 == 0x47) goto LAB_100c8266;
  }
  FUN_100c7430();
LAB_100c8266:
  (*DAT_10121ec0)(1,DAT_10124900,0,*(undefined4 *)(*(int *)(param_1 + 4) + 0x208));
  (*DAT_10121ec8)(param_2);
  (*DAT_10121ec4)();
  return;
}



/* ==== 100c82a6 FUN_100c82a6 ==== */

float10 __thiscall FUN_100c82a6(undefined1 *param_1,byte *param_2)

{
  uint uVar1;
  int iVar2;
  byte *this;
  undefined1 *puVar3;
  uint local_1c [6];
  
  while( true ) {
    if (DAT_1011e234 < 2) {
      uVar1 = (byte)PTR_DAT_1011e028[(uint)*param_2 * 2] & 8;
      param_1 = PTR_DAT_1011e028;
    }
    else {
      puVar3 = &DAT_00000008;
      uVar1 = FUN_100c939e(param_1,(uint)*param_2,8);
      param_1 = puVar3;
    }
    if (uVar1 == 0) break;
    param_2 = param_2 + 1;
  }
  this = param_2;
  _strlen((char *)param_2);
  iVar2 = FUN_100ca6ca(this,local_1c,param_2);
  return (float10)*(double *)(iVar2 + 0x10);
}



/* ==== 100c82fd FUN_100c82fd ==== */

int __thiscall FUN_100c82fd(void *this,byte *param_1)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  byte *pbVar5;
  undefined1 *puVar6;
  
  while( true ) {
    if (DAT_1011e234 < 2) {
      uVar1 = (byte)PTR_DAT_1011e028[(uint)*param_1 * 2] & 8;
      this = PTR_DAT_1011e028;
    }
    else {
      puVar6 = &DAT_00000008;
      uVar1 = FUN_100c939e(this,(uint)*param_1,8);
      this = puVar6;
    }
    if (uVar1 == 0) break;
    param_1 = param_1 + 1;
  }
  uVar1 = (uint)*param_1;
  pbVar5 = param_1 + 1;
  if ((uVar1 == 0x2d) || (uVar4 = uVar1, uVar1 == 0x2b)) {
    uVar4 = (uint)*pbVar5;
    pbVar5 = param_1 + 2;
  }
  iVar3 = 0;
  while( true ) {
    if (DAT_1011e234 < 2) {
      uVar2 = (byte)PTR_DAT_1011e028[uVar4 * 2] & 4;
    }
    else {
      puVar6 = (undefined1 *)0x4;
      uVar2 = FUN_100c939e(this,uVar4,4);
      this = puVar6;
    }
    if (uVar2 == 0) break;
    iVar3 = (uVar4 - 0x30) + iVar3 * 10;
    uVar4 = (uint)*pbVar5;
    pbVar5 = pbVar5 + 1;
  }
  if (uVar1 == 0x2d) {
    iVar3 = -iVar3;
  }
  return iVar3;
}



/* ==== 100c8388 FUN_100c8388 ==== */

void __thiscall FUN_100c8388(void *this,byte *param_1)

{
  FUN_100c82fd(this,param_1);
  return;
}



/* ==== 100c844d FUN_100c844d ==== */

void FUN_100c844d(void)

{
  void *extraout_ECX;
  
  FUN_100c8465();
  DAT_10134974 = FUN_100ca854();
  FUN_100ca804(extraout_ECX);
  return;
}



/* ==== 100c8465 FUN_100c8465 ==== */

void FUN_100c8465(void)

{
  PTR___fptrap_1011e274 = &LAB_100ca8d7;
  PTR___fptrap_1011e270 = __cfltcvt;
  PTR___fptrap_1011e278 = __fassign;
  PTR___fptrap_1011e27c = FUN_100ca87d;
  PTR___fptrap_1011e280 = &LAB_100ca925;
  PTR___fptrap_1011e284 = __cfltcvt;
  return;
}




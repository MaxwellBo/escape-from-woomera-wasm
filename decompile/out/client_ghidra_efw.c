/* EFW-focused Ghidra decompile of client.dll
 * HUD / diary / storyboard / talk-prompt slice. Not recompilable as-is.
 * See decompile/recovered/NAMES.md and decompile/RECOVERED.md.
 */

/* ==== 10008f10 HUD_GetUserEntity ==== */

undefined4 HUD_GetUserEntity(void)

{
                    /* 0x8f10  486  HUD_GetUserEntity */
  return 0;
}



/* ==== 100099e0 FUN_100099e0 ==== */

void __thiscall FUN_100099e0(void *this,CBaseEntity *param_1)

{
  undefined1 uVar1;
  undefined3 extraout_var;
  int iVar2;
  CBasePlayerItem *extraout_ECX;
  CBasePlayerItem *this_00;
  CBasePlayerItem *extraout_ECX_00;
  CBasePlayerItem *this_01;
  CBasePlayerItem *this_02;
  CBasePlayerItem *this_03;
  
  CBasePlayerAmmo::DefaultTouch(this,param_1);
  uVar1 = FUN_10044880();
  this_01 = extraout_ECX;
  if (CONCAT31(extraout_var,uVar1) == 0) {
    iVar2 = (**(code **)(*(int *)param_1 + 0xcc))();
    this_01 = this_00;
    if ((iVar2 != 0) && (param_1 != (CBaseEntity *)0x0)) {
      CBasePlayerItem::AttemptToMaterialize(this_00);
      this_01 = extraout_ECX_00;
    }
  }
  CBasePlayerItem::AttemptToMaterialize(this_01);
  CBasePlayerItem::AttemptToMaterialize(this_02);
  CBasePlayerItem::AttemptToMaterialize(this_03);
  return;
}



/* ==== 10009e60 FUN_10009e60 ==== */

void __thiscall FUN_10009e60(void *this,CBaseEntity *param_1)

{
  DWORD DVar1;
  CBasePlayerItem *this_00;
  
  DVar1 = GetTickCount();
  if (*(uint *)((int)this + 300) < DVar1) {
    CBasePlayerAmmo::DefaultTouch(this,param_1);
    CBasePlayerItem::AttemptToMaterialize(this_00);
  }
  return;
}



/* ==== 10009e90 FUN_10009e90 ==== */

void __thiscall FUN_10009e90(void *this,int param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  byte bVar5;
  int iVar6;
  byte *pbVar7;
  int iVar8;
  DWORD DVar9;
  CBasePlayerItem *this_00;
  undefined3 uVar10;
  CBasePlayerItem *this_01;
  CBasePlayerItem *this_02;
  CBasePlayerItem *this_03;
  char *pcVar11;
  bool bVar12;
  
  iVar8 = *(int *)(*(int *)(param_1 + 4) + 0x208);
  if (iVar8 == 0) {
    this_00 = (CBasePlayerItem *)0x0;
  }
  else {
    this_00 = (CBasePlayerItem *)(iVar8 + 0x80);
  }
  pcVar11 = s_efw_IDTag_Position_100771d4;
  pbVar7 = (byte *)(*(int *)(DAT_1007f7f8 + 0x98) + *(int *)(this_00 + 0x1cc));
  do {
    bVar5 = *pbVar7;
    uVar10 = (undefined3)((uint)this_00 >> 8);
    this_00 = (CBasePlayerItem *)CONCAT31(uVar10,bVar5);
    bVar12 = bVar5 < (byte)*pcVar11;
    if (bVar5 != *pcVar11) {
LAB_10009ef3:
      iVar8 = (1 - (uint)bVar12) - (uint)(bVar12 != 0);
      goto LAB_10009ef8;
    }
    if (bVar5 == 0) break;
    bVar5 = pbVar7[1];
    this_00 = (CBasePlayerItem *)CONCAT31(uVar10,bVar5);
    bVar12 = bVar5 < (byte)pcVar11[1];
    if (bVar5 != pcVar11[1]) goto LAB_10009ef3;
    pbVar7 = pbVar7 + 2;
    pcVar11 = pcVar11 + 2;
  } while (bVar5 != 0);
  iVar8 = 0;
LAB_10009ef8:
  if (iVar8 == 0) {
    pcVar11 = s_ID_Tag_has_been_placed_on_the_wa_100771ac;
    CBasePlayerItem::AttemptToMaterialize(this_00);
    iVar8 = *(int *)(param_1 + 4);
    fVar1 = *(float *)(iVar8 + 0xd4);
    fVar2 = *(float *)(iVar8 + 200);
    fVar3 = *(float *)(iVar8 + 0xd8);
    fVar4 = *(float *)(iVar8 + 0xcc);
    iVar6 = *(int *)((int)this + 4);
    *(float *)(iVar6 + 8) = (*(float *)(iVar8 + 0xd0) + *(float *)(iVar8 + 0xc4)) * 0.5;
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
    (*DAT_1007eed8)(*(undefined4 *)(*(int *)((int)this + 4) + 0x208),
                    *(undefined4 *)((int)this + 0x120),pcVar11);
    CBasePlayerItem::AttemptToMaterialize(this_01);
    if (*(int **)((int)this + 0xdc) != (int *)0x0) {
      FUN_10043fa0(*(int **)((int)this + 0xdc),(int)this);
    }
    CBasePlayerItem::AttemptToMaterialize(this);
    *(undefined4 *)(*(int *)((int)this + 4) + 0x10c) = 0;
    *(uint *)(*(int *)((int)this + 4) + 0x118) = *(uint *)(*(int *)((int)this + 4) + 0x118) | 0x80;
    DVar9 = GetTickCount();
    *(DWORD *)((int)this + 300) = DVar9 + 500;
    CBasePlayerItem::AttemptToMaterialize(this_02);
    CBasePlayerItem::AttemptToMaterialize(this_03);
  }
  return;
}



/* ==== 1000b490 HUD_PostRunCmd ==== */

void __cdecl
HUD_PostRunCmd(int *param_1,int param_2,int param_3,undefined4 param_4,undefined4 param_5,
              undefined4 param_6,undefined4 param_7)

{
                    /* 0xb490  492  HUD_PostRunCmd */
  DAT_100a5180 = param_4;
  if ((DAT_100a9584 != 0) && (*(float *)(DAT_100a9584 + 0xc) != 0.0)) {
    FUN_1000aae0(param_1,param_2,param_3,(double)CONCAT44(param_6,param_5),param_7);
    DAT_100a9588 = *(undefined4 *)(param_2 + 0x1b4);
    return;
  }
  *(undefined4 *)(param_2 + 0x1b4) = DAT_100a9588;
  DAT_100a9588 = *(undefined4 *)(param_2 + 0x1b4);
  return;
}



/* ==== 10018b90 HUD_GetHullBounds ==== */

undefined4 __cdecl HUD_GetHullBounds(int param_1)

{
  undefined4 uVar1;
  
                    /* 0x18b90  484  HUD_GetHullBounds */
  uVar1 = 0;
  if (((param_1 == 0) || (param_1 == 1)) || (param_1 == 2)) {
    uVar1 = 1;
  }
  return uVar1;
}



/* ==== 10018bb0 HUD_ConnectionlessPacket ==== */

undefined4 HUD_ConnectionlessPacket(void)

{
  undefined4 *in_stack_00000010;
  
                    /* 0x18bb0  478  HUD_ConnectionlessPacket */
  *in_stack_00000010 = 0;
  return 0;
}



/* ==== 10018bc0 HUD_PlayerMoveInit ==== */

void __cdecl HUD_PlayerMoveInit(undefined4 param_1)

{
                    /* 0x18bc0  490  HUD_PlayerMoveInit */
  FUN_1002bed0(param_1);
  return;
}



/* ==== 10018bd0 HUD_PlayerMoveTexture ==== */

void __cdecl HUD_PlayerMoveTexture(byte *param_1)

{
                    /* 0x18bd0  491  HUD_PlayerMoveTexture */
  FUN_10026910(param_1);
  return;
}



/* ==== 10018be0 HUD_PlayerMove ==== */

void __cdecl HUD_PlayerMove(int param_1,int param_2)

{
                    /* 0x18be0  489  HUD_PlayerMove */
  FUN_1002bd60(param_1,param_2);
  return;
}



/* ==== 10018c00 Initialize ==== */

undefined4 __cdecl Initialize(undefined4 *param_1,int param_2)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  puVar2 = param_1;
  puVar3 = &DAT_100a4ff0;
                    /* 0x18c00  509  Initialize */
  for (iVar1 = 0x62; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = *puVar2;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  }
  if (param_2 != 7) {
    return 0;
  }
  puVar2 = &DAT_100a4ff0;
  for (iVar1 = 0x62; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = *param_1;
    param_1 = param_1 + 1;
    puVar2 = puVar2 + 1;
  }
  thunk_FUN_100090d0();
  return 1;
}



/* ==== 10018c40 HUD_VidInit ==== */

undefined4 HUD_VidInit(void)

{
                    /* 0x18c40  502  HUD_VidInit */
  FUN_1001c0e0(&DAT_100a3a40);
  FUN_10034300();
  return 1;
}



/* ==== 10018c60 HUD_Init ==== */

undefined4 HUD_Init(void)

{
                    /* 0x18c60  487  HUD_Init */
  FUN_100238c0();
  FUN_1001bc20((int *)&DAT_100a3a40);
  FUN_10035280();
  FUN_10047720();
  FUN_100436a0();
  return 1;
}



/* ==== 10018c90 HUD_Redraw ==== */

undefined4 HUD_Redraw(void)

{
                    /* 0x18c90  494  HUD_Redraw */
  FUN_1001db00();
  FUN_10043a10();
  return 1;
}



/* ==== 10018cb0 HUD_UpdateClientData ==== */

void __cdecl HUD_UpdateClientData(undefined4 *param_1)

{
                    /* 0x18cb0  501  HUD_UpdateClientData */
  FUN_100246a0();
  FUN_10021cd0(&DAT_100a3a40,param_1);
  return;
}



/* ==== 10018cd0 HUD_Reset ==== */

undefined4 HUD_Reset(void)

{
                    /* 0x18cd0  495  HUD_Reset */
  FUN_1001c0e0(&DAT_100a3a40);
  return 1;
}



/* ==== 10018ce0 HUD_Frame ==== */

void __cdecl HUD_Frame(undefined4 param_1,undefined4 param_2)

{
  undefined *this;
  
                    /* 0x18ce0  483  HUD_Frame */
  FUN_1001fa10(param_1,param_2);
  this = FUN_10012170();
  FUN_100129d0(this,(double)CONCAT44(param_2,param_1));
  return;
}



/* ==== 10018d10 HUD_VoiceStatus ==== */

void __cdecl HUD_VoiceStatus(int param_1,int param_2)

{
  undefined *this;
  
                    /* 0x18d10  503  HUD_VoiceStatus */
  this = FUN_10012170();
  FUN_10012c10(this,param_1,param_2);
  return;
}



/* ==== 10018d30 HUD_DirectorEvent ==== */

void __cdecl
HUD_DirectorEvent(char param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
                    /* 0x18d30  480  HUD_DirectorEvent */
  FUN_100201f0(&DAT_100a4118,param_1,param_2,param_3,param_4);
  return;
}



/* ==== 10019950 HUD_AddEntity ==== */

undefined4 __cdecl HUD_AddEntity(int param_1,int *param_2)

{
  int iVar1;
  
                    /* 0x19950  477  HUD_AddEntity */
  iVar1 = (*DAT_100a5150)();
  if (iVar1 != 0) {
    FUN_10021bb0(&DAT_100a4118,param_1,param_2);
    if (((DAT_100a412c == 4) || (DAT_100a4130 == 2)) && (*param_2 == DAT_100a4a68)) {
      return 0;
    }
  }
  return 1;
}



/* ==== 100199a0 HUD_TxferLocalOverrides ==== */

void __cdecl HUD_TxferLocalOverrides(int param_1,undefined4 *param_2)

{
                    /* 0x199a0  499  HUD_TxferLocalOverrides */
  *(undefined4 *)(param_1 + 0x10) = *param_2;
  *(undefined4 *)(param_1 + 0x14) = param_2[1];
  *(undefined4 *)(param_1 + 0x18) = param_2[2];
  *(undefined4 *)(param_1 + 0x104) = param_2[99];
  *(undefined4 *)(param_1 + 0x108) = param_2[100];
  *(undefined4 *)(param_1 + 0x10c) = param_2[0x65];
  *(undefined4 *)(param_1 + 0x110) = param_2[0x66];
  return;
}



/* ==== 100199f0 HUD_ProcessPlayerState ==== */

void __cdecl HUD_ProcessPlayerState(int param_1,int param_2)

{
  int *piVar1;
  
                    /* 0x199f0  493  HUD_ProcessPlayerState */
  *(undefined4 *)(param_1 + 0x10) = *(undefined4 *)(param_2 + 0x10);
  *(undefined4 *)(param_1 + 0x14) = *(undefined4 *)(param_2 + 0x14);
  *(undefined4 *)(param_1 + 0x18) = *(undefined4 *)(param_2 + 0x18);
  *(undefined4 *)(param_1 + 0x1c) = *(undefined4 *)(param_2 + 0x1c);
  *(undefined4 *)(param_1 + 0x20) = *(undefined4 *)(param_2 + 0x20);
  *(undefined4 *)(param_1 + 0x24) = *(undefined4 *)(param_2 + 0x24);
  *(undefined4 *)(param_1 + 0x70) = *(undefined4 *)(param_2 + 0x70);
  *(undefined4 *)(param_1 + 0x74) = *(undefined4 *)(param_2 + 0x74);
  *(undefined4 *)(param_1 + 0x78) = *(undefined4 *)(param_2 + 0x78);
  *(undefined4 *)(param_1 + 0x30) = *(undefined4 *)(param_2 + 0x30);
  *(undefined4 *)(param_1 + 0x28) = *(undefined4 *)(param_2 + 0x28);
  *(undefined2 *)(param_1 + 0x38) = *(undefined2 *)(param_2 + 0x38);
  *(undefined4 *)(param_1 + 0x3c) = *(undefined4 *)(param_2 + 0x3c);
  *(undefined4 *)(param_1 + 0xb4) = *(undefined4 *)(param_2 + 0xb4);
  *(undefined4 *)(param_1 + 0x58) = *(undefined4 *)(param_2 + 0x58);
  *(undefined4 *)(param_1 + 0x2c) = *(undefined4 *)(param_2 + 0x2c);
  *(undefined4 *)(param_1 + 0x5c) = *(undefined4 *)(param_2 + 0x5c);
  *(undefined2 *)(param_1 + 0x3a) = *(undefined2 *)(param_2 + 0x3a);
  *(undefined4 *)(param_1 + 0x48) = *(undefined4 *)(param_2 + 0x48);
  *(undefined4 *)(param_1 + 0x4c) = *(undefined4 *)(param_2 + 0x4c);
  *(undefined1 *)(param_1 + 0x50) = *(undefined1 *)(param_2 + 0x50);
  *(undefined1 *)(param_1 + 0x51) = *(undefined1 *)(param_2 + 0x51);
  *(undefined1 *)(param_1 + 0x52) = *(undefined1 *)(param_2 + 0x52);
  *(undefined4 *)(param_1 + 0x54) = *(undefined4 *)(param_2 + 0x54);
  *(undefined4 *)(param_1 + 0x60) = *(undefined4 *)(param_2 + 0x60);
  *(undefined4 *)(param_1 + 100) = *(undefined4 *)(param_2 + 100);
  *(undefined4 *)(param_1 + 0x68) = *(undefined4 *)(param_2 + 0x68);
  *(undefined2 *)(param_1 + 0x6c) = *(undefined2 *)(param_2 + 0x6c);
  *(undefined4 *)(param_1 + 0xbc) = *(undefined4 *)(param_2 + 0xbc);
  *(undefined4 *)(param_1 + 0xc0) = *(undefined4 *)(param_2 + 0xc0);
  *(undefined4 *)(param_1 + 0xc4) = *(undefined4 *)(param_2 + 0xc4);
  *(undefined4 *)(param_1 + 0x9c) = *(undefined4 *)(param_2 + 0x9c);
  *(undefined4 *)(param_1 + 0xa0) = *(undefined4 *)(param_2 + 0xa0);
  *(undefined4 *)(param_1 + 0xb8) = *(undefined4 *)(param_2 + 0xb8);
  *(undefined4 *)(param_1 + 0xb0) = *(undefined4 *)(param_2 + 0xb0);
  *(undefined4 *)(param_1 + 200) = *(undefined4 *)(param_2 + 200);
  *(undefined4 *)(param_1 + 0xa8) = *(undefined4 *)(param_2 + 0xa8);
  *(undefined4 *)(param_1 + 0xa4) = *(undefined4 *)(param_2 + 0xa4);
  *(undefined4 *)(param_1 + 0x34) = *(undefined4 *)(param_2 + 0x34);
  piVar1 = (int *)(*DAT_100a50bc)();
  if (*(int *)(param_1 + 4) == *piVar1) {
    DAT_100b6620 = *(undefined4 *)(param_1 + 0xa8);
    DAT_100b6730 = *(undefined4 *)(param_1 + 0xa4);
    DAT_100b6724 = *(undefined4 *)(param_2 + 0x104);
    DAT_100b672c = *(undefined4 *)(param_2 + 0x108);
    DAT_100b6728 = *(undefined4 *)(param_2 + 0x10c);
  }
  return;
}



/* ==== 10019b80 HUD_TxferPredictionData ==== */

void __cdecl
HUD_TxferPredictionData
          (int param_1,int param_2,int param_3,int param_4,undefined4 *param_5,undefined4 *param_6)

{
  int iVar1;
  
                    /* 0x19b80  500  HUD_TxferPredictionData */
  *(undefined4 *)(param_1 + 0xcc) = *(undefined4 *)(param_2 + 0xcc);
  *(undefined4 *)(param_1 + 0xd8) = *(undefined4 *)(param_2 + 0xd8);
  *(undefined4 *)(param_1 + 0xd4) = *(undefined4 *)(param_2 + 0xd4);
  *(undefined4 *)(param_1 + 0xa8) = *(undefined4 *)(param_2 + 0xa8);
  *(undefined4 *)(param_3 + 0x18) = *(undefined4 *)(param_4 + 0x18);
  *(undefined4 *)(param_3 + 0x68) = *(undefined4 *)(param_4 + 0x68);
  *(undefined4 *)(param_3 + 0x6c) = *(undefined4 *)(param_4 + 0x6c);
  *(undefined4 *)(param_3 + 0x70) = *(undefined4 *)(param_4 + 0x70);
  *(undefined4 *)(param_3 + 0x74) = *(undefined4 *)(param_4 + 0x74);
  *(undefined4 *)(param_3 + 0x78) = *(undefined4 *)(param_4 + 0x78);
  *(undefined4 *)(param_3 + 0x7c) = *(undefined4 *)(param_4 + 0x7c);
  *(undefined4 *)(param_3 + 0x60) = *(undefined4 *)(param_4 + 0x60);
  *(undefined4 *)(param_3 + 100) = *(undefined4 *)(param_4 + 100);
  *(undefined4 *)(param_3 + 0x80) = *(undefined4 *)(param_4 + 0x80);
  *(undefined4 *)(param_3 + 0x5c) = *(undefined4 *)(param_4 + 0x5c);
  iVar1 = *(int *)(param_4 + 0x88);
  *(int *)(param_3 + 0x88) = iVar1;
  if ((*(int *)(param_4 + 0x18c) != 0) || (DAT_10078530 = 0, iVar1 == 0)) {
    DAT_10078530 = 1;
  }
  *(undefined4 *)(param_3 + 0x18c) = *(undefined4 *)(param_4 + 0x18c);
  *(undefined4 *)(param_3 + 400) = *(undefined4 *)(param_4 + 400);
  *(undefined4 *)(param_3 + 0x194) = *(undefined4 *)(param_4 + 0x194);
  iVar1 = (*DAT_100a5150)();
  if (iVar1 != 0) {
    *(undefined4 *)(param_3 + 0x18c) = DAT_100b6724;
    *(undefined4 *)(param_3 + 400) = DAT_100b672c;
    *(undefined4 *)(param_3 + 0x194) = DAT_100b6728;
  }
  *(undefined4 *)(param_3 + 0x198) = *(undefined4 *)(param_4 + 0x198);
  *(undefined4 *)(param_3 + 0x1a0) = *(undefined4 *)(param_4 + 0x1a0);
  *(undefined4 *)(param_3 + 0x1a4) = *(undefined4 *)(param_4 + 0x1a4);
  *(undefined4 *)(param_3 + 0x1ac) = *(undefined4 *)(param_4 + 0x1ac);
  *(undefined4 *)(param_3 + 0x1b0) = *(undefined4 *)(param_4 + 0x1b0);
  *(undefined4 *)(param_3 + 0x1b4) = *(undefined4 *)(param_4 + 0x1b4);
  *(undefined4 *)(param_3 + 0x1b8) = *(undefined4 *)(param_4 + 0x1b8);
  *(undefined4 *)(param_3 + 0x1bc) = *(undefined4 *)(param_4 + 0x1bc);
  *(undefined4 *)(param_3 + 0x1c0) = *(undefined4 *)(param_4 + 0x1c0);
  *(undefined4 *)(param_3 + 0x1c4) = *(undefined4 *)(param_4 + 0x1c4);
  *(undefined4 *)(param_3 + 0x1c8) = *(undefined4 *)(param_4 + 0x1c8);
  *(undefined4 *)(param_3 + 0x1cc) = *(undefined4 *)(param_4 + 0x1cc);
  *(undefined4 *)(param_3 + 0x1d0) = *(undefined4 *)(param_4 + 0x1d0);
  *(undefined4 *)(param_3 + 0x1d4) = *(undefined4 *)(param_4 + 0x1d4);
  *(undefined4 *)(param_3 + 0x1d8) = *(undefined4 *)(param_4 + 0x1d8);
  for (iVar1 = 0x2c0; iVar1 != 0; iVar1 = iVar1 + -1) {
    *param_5 = *param_6;
    param_6 = param_6 + 1;
    param_5 = param_5 + 1;
  }
  return;
}



/* ==== 10019d50 HUD_CreateEntities ==== */

void HUD_CreateEntities(void)

{
  undefined *puVar1;
  
                    /* 0x19d50  479  HUD_CreateEntities */
  FUN_10009350();
  puVar1 = FUN_10012170();
  FUN_10012a60((int)puVar1);
  return;
}



/* ==== 10019d70 HUD_StudioEvent ==== */

void __cdecl HUD_StudioEvent(int param_1,int param_2)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
                    /* 0x19d70  497  HUD_StudioEvent */
  switch(*(undefined4 *)(param_1 + 4)) {
  case 0x1389:
    puVar1 = (undefined4 *)((int)DAT_100a513c + 0x4c);
    uVar2 = FUN_100499e5(DAT_100a513c,(byte *)(param_1 + 0xc));
    (*(code *)*puVar1)(param_2 + 0xb60,uVar2);
    return;
  case 0x138a:
    uVar4 = 100;
    uVar3 = 0xffffff9c;
    puVar1 = (undefined4 *)((int)DAT_100a513c + 0x84);
    uVar2 = FUN_100499e5(DAT_100a513c,(byte *)(param_1 + 0xc));
    (*(code *)*puVar1)(param_2 + 0xb60,uVar2,uVar3,uVar4);
    return;
  case 0x138c:
    (*DAT_100a50ec)(param_1 + 0xc,0x3f800000,param_2 + 0xb60);
    break;
  case 0x1393:
    puVar1 = (undefined4 *)((int)DAT_100a513c + 0x4c);
    uVar2 = FUN_100499e5(DAT_100a513c,(byte *)(param_1 + 0xc));
    (*(code *)*puVar1)(param_2 + 0xb6c,uVar2);
    return;
  case 0x139d:
    puVar1 = (undefined4 *)((int)DAT_100a513c + 0x4c);
    uVar2 = FUN_100499e5(DAT_100a513c,(byte *)(param_1 + 0xc));
    (*(code *)*puVar1)(param_2 + 0xb78,uVar2);
    return;
  case 0x13a7:
    puVar1 = (undefined4 *)((int)DAT_100a513c + 0x4c);
    uVar2 = FUN_100499e5(DAT_100a513c,(byte *)(param_1 + 0xc));
    (*(code *)*puVar1)(param_2 + 0xb84,uVar2);
    return;
  }
  return;
}



/* ==== 10019ec0 HUD_TempEntUpdate ==== */

void __cdecl
HUD_TempEntUpdate(double param_1,double param_2,double param_3,uint *param_4,int *param_5,
                 undefined *param_6,undefined *param_7)

{
  float *pfVar1;
  float fVar2;
  float fVar3;
  uint uVar4;
  uint *puVar5;
  uint *puVar6;
  int iVar7;
  code *pcVar8;
  float *pfVar9;
  undefined1 *puVar10;
  uint *puVar11;
  float10 fVar12;
  float10 fVar13;
  longlong lVar14;
  float fStack_b4;
  float fStack_a4;
  float fStack_a0;
  float fStack_9c;
  float fStack_98;
  uint *puStack_94;
  float fStack_90;
  float fStack_8c;
  undefined1 auStack_88 [16];
  float fStack_78;
  float fStack_68;
  float fStack_64;
  float fStack_60;
  int iStack_58;
  undefined1 auStack_44 [16];
  float fStack_34;
  float fStack_24;
  float fStack_20;
  float fStack_1c;
  
                    /* 0x19ec0  498  HUD_TempEntUpdate */
  if (*param_5 != 0) {
    (**(code **)(DAT_100a5140 + 0x28))(0,1);
    (**(code **)(DAT_100a5140 + 0x2c))();
    (**(code **)(DAT_100a5140 + 0x34))(0xffffffff);
    DAT_100a54c0 = DAT_100a54c0 + 1 & 0x1f;
    puVar11 = (uint *)*param_5;
    if (0.0 < param_1) {
      puStack_94 = (uint *)0x0;
      fStack_90 = (float)((float10)param_2 * (float10)5.5);
      fVar2 = (float)-((float10)param_1 * (float10)param_3);
      fStack_8c = (float)(-((float10)param_1 * (float10)param_3) * (float10)0.5);
      puVar6 = puStack_94;
      while (puVar5 = puVar11, puStack_94 = puVar6, puVar5 != (uint *)0x0) {
        puVar11 = (uint *)puVar5[0xb];
        if (0.0 <= (float)puVar5[1] - (float)param_2) {
LAB_1001a02e:
          pfVar1 = (float *)(puVar5 + 0x2e3);
          puVar5[0x6c] = puVar5[0x2e3];
          puVar5[0x6d] = puVar5[0x2e4];
          puVar5[0x6e] = puVar5[0x2e5];
          puStack_94 = puVar5;
          if ((*puVar5 & 0x20000) == 0) {
            uVar4 = *puVar5;
            if ((uVar4 & 0x8000) == 0) {
              if ((uVar4 & 1) == 0) {
                if ((uVar4 & 0x400) == 0) {
                  iVar7 = 3;
                  pfVar9 = pfVar1;
                  do {
                    iVar7 = iVar7 + -1;
                    *pfVar9 = pfVar9[-0x2cc] * (float)param_1 + *pfVar9;
                    pfVar9 = pfVar9 + 1;
                  } while (iVar7 != 0);
                  goto LAB_1001a24b;
                }
                fVar13 = (float10)fsin((float10)param_2 * (float10)20.0 + (float10)(int)puVar5);
                *pfVar1 = (float)((float10)(float)puVar5[0x17] * (float10)param_1 +
                                  fVar13 * (float10)8.0 + (float10)*pfVar1);
                fVar13 = (float10)fsin((float10)param_2 * (float10)30.0 + (float10)(int)puVar5);
                puVar5[0x2e4] =
                     (uint)(float)((float10)(float)puVar5[0x18] * (float10)param_1 +
                                   fVar13 * (float10)4.0 + (float10)(float)puVar5[0x2e4]);
              }
              else {
                fVar13 = (float10)(float)puVar5[0x17] * (float10)param_1 + (float10)(float)puVar5[3]
                ;
                puVar5[3] = (uint)(float)fVar13;
                puVar5[4] = (uint)((float)puVar5[0x18] * (float)param_1 + (float)puVar5[4]);
                fVar12 = (float10)fsin((float10)(float)puVar5[0x74] * (float10)param_2 +
                                       (float10)(float)puVar5[0x19]);
                *pfVar1 = (float)(fVar13 + (float10)(float)puVar5[0xd5] * (float10)10.0 * fVar12);
                fVar13 = (float10)fsin((float10)fStack_90 + (float10)(float)puVar5[0x19] +
                                       (float10)0.7);
                puVar5[0x2e4] =
                     (uint)(float)((float10)(float)puVar5[0xd5] * (float10)8.0 * fVar13 +
                                  (float10)(float)puVar5[4]);
              }
              puVar5[0x2e5] = (uint)((float)puVar5[0x19] * (float)param_1 + (float)puVar5[0x2e5]);
            }
            else {
              iVar7 = (*DAT_100a50c4)((int)(short)puVar5[0xd]);
              *pfVar1 = *(float *)(iVar7 + 0xb48) + (float)puVar5[0xe];
              puVar5[0x2e4] = (uint)(*(float *)(iVar7 + 0xb4c) + (float)puVar5[0xf]);
              puVar5[0x2e5] = (uint)(*(float *)(iVar7 + 0xb50) + (float)puVar5[0x10]);
            }
          }
          else if ((float)puVar5[0x23] < (float)param_2) {
            (**(code **)(DAT_100a513c + 0x84))(pfVar1,8,0xffffff38,200);
            fVar3 = (float)puVar5[0x2b];
            puVar5[0x2b] = (uint)(fVar3 - 0.1);
            if (0.0 < fVar3 - 0.1) {
              puVar5[1] = (uint)(float)((float10)param_2 + (float10)0.5);
              puVar5[0x23] = (uint)(float)((float10)param_2 + (float10)0.1);
            }
            else {
              puVar5[1] = (uint)(float)param_2;
            }
          }
LAB_1001a24b:
          if ((*puVar5 & 0x100) == 0) {
            if (((*puVar5 & 0x800) != 0) &&
               (fVar13 = (float10)param_1 * (float10)10.0 + (float10)(float)puVar5[0xc9],
               puVar5[0xc9] = (uint)(float)fVar13, (float10)(float)puVar5[2] <= fVar13)) {
              lVar14 = __ftol();
              puVar5[0xc9] = (uint)((float)puVar5[0xc9] - (float)(int)lVar14);
            }
LAB_1001a2f3:
            if ((*puVar5 & 4) != 0) {
              puVar5[0x2e6] = (uint)((float)puVar5[0x1a] * (float)param_1 + (float)puVar5[0x2e6]);
              puVar5[0x2e7] = (uint)((float)puVar5[0x1b] * (float)param_1 + (float)puVar5[0x2e7]);
              puVar5[0x2e8] = (uint)((float)puVar5[0x1c] * (float)param_1 + (float)puVar5[0x2e8]);
              puVar5[0x2db] = puVar5[0x2e6];
              puVar5[0x2dc] = puVar5[0x2e7];
              puVar5[0x2dd] = puVar5[0x2e8];
            }
            uVar4 = *puVar5;
            if ((uVar4 & 0x1020) != 0) {
              if ((uVar4 & 0x1000) == 0) {
                if ((uVar4 & 0x20) != 0) {
                  (**(code **)(DAT_100a5140 + 0x38))(2);
                  (**(code **)(DAT_100a5140 + 0x3c))(puVar5 + 0x6c,pfVar1,10,0xffffffff,auStack_44);
                  if (fStack_34 != 1.0) {
                    fStack_a0 = fStack_24;
                    fStack_a4 = fStack_34;
                    fStack_9c = fStack_20;
                    fStack_98 = fStack_1c;
                    if ((*puVar5 & 0x20000) != 0) {
                      pfVar9 = (float *)(puVar5 + 0x17);
                      FUN_10031290(pfVar9,0.6,pfVar9);
                      fVar13 = FUN_100264c0(pfVar9);
                      if (fVar13 < (float10)10.0) {
                        puVar5[0x2b] = 0;
                      }
                    }
                    pcVar8 = (code *)puVar5[9];
                    if (pcVar8 != (code *)0x0) {
                      puVar10 = auStack_44;
                      goto LAB_1001a4c7;
                    }
                    goto LAB_1001a4ce;
                  }
                }
              }
              else {
                (**(code **)(DAT_100a5140 + 0x38))(2);
                (**(code **)(DAT_100a5140 + 0x3c))(puVar5 + 0x6c,pfVar1,2,0xffffffff,auStack_88);
                if ((fStack_78 != 1.0) &&
                   ((iVar7 = (**(code **)(DAT_100a5140 + 0x24))(iStack_58), iStack_58 == 0 ||
                    (*(int *)(iVar7 + 0x50) != (int)(short)puVar5[0xd])))) {
                  fStack_a0 = fStack_68;
                  pcVar8 = (code *)puVar5[9];
                  fStack_a4 = fStack_78;
                  fStack_9c = fStack_64;
                  fStack_98 = fStack_60;
                  if (pcVar8 != (code *)0x0) {
                    puVar10 = auStack_88;
LAB_1001a4c7:
                    (*pcVar8)(puVar5,puVar10);
                  }
LAB_1001a4ce:
                  if (fStack_a4 != 1.0) {
                    pfVar9 = (float *)(puVar5 + 0x17);
                    FUN_100312c0((float *)(puVar5 + 0x6c),fStack_a4 * (float)param_1,pfVar9,pfVar1);
                    fStack_b4 = (float)puVar5[7];
                    if (((((*puVar5 & 10) != 0) && (fStack_b4 = fStack_b4 * 0.5, 0.9 < fStack_98))
                        && ((float)puVar5[0x19] <= 0.0)) && (fVar2 * 3.0 <= (float)puVar5[0x19])) {
                      fStack_b4 = 0.0;
                      *puVar5 = *puVar5 & 0xffffffc1;
                      puVar5[0x2e6] = 0;
                      puVar5[0x2e8] = 0;
                    }
                    if (puVar5[8] != 0) {
                      (*(code *)param_7)(puVar5,fStack_b4);
                    }
                    if ((*puVar5 & 0x4000) == 0) {
                      if (fStack_b4 != 0.0) {
                        FUN_100312c0(pfVar9,(fStack_a0 * *pfVar9 +
                                            fStack_98 * (float)puVar5[0x19] +
                                            fStack_9c * (float)puVar5[0x18]) * -2.0,&fStack_a0,
                                     pfVar9);
                        puVar5[0x2e7] = (uint)-(float)puVar5[0x2e7];
                      }
                      if (fStack_b4 != 1.0) {
                        FUN_10031290(pfVar9,fStack_b4,pfVar9);
                        FUN_10031290((float *)(puVar5 + 0x2e6),0.9,(float *)(puVar5 + 0x2e6));
                      }
                    }
                    else {
                      puVar5[1] = (uint)(float)param_2;
                      *puVar5 = *puVar5 & 0xffffff7f;
                    }
                  }
                }
              }
            }
            if (((*puVar5 & 0x40) != 0) && (DAT_100a54c0 == puVar5[0xcc])) {
              pfVar9 = (float *)(**(code **)(DAT_100a513c + 0x100))(0);
              *pfVar9 = *pfVar1;
              pfVar9[1] = (float)puVar5[0x2e4];
              pfVar9[2] = (float)puVar5[0x2e5];
              pfVar9[3] = 60.0;
              pfVar9[5] = (float)((float10)param_2 + (float10)0.01);
              *(undefined1 *)(pfVar9 + 4) = 0xff;
              *(undefined1 *)((int)pfVar9 + 0x11) = 0x78;
              *(undefined1 *)((int)pfVar9 + 0x12) = 0;
            }
            if ((*puVar5 & 0x10) != 0) {
              (**(code **)(DAT_100a513c + 0x78))(puVar5 + 0x6c,pfVar1,1);
            }
            fVar3 = fVar2;
            if (((*puVar5 & 2) != 0) || (fVar3 = fStack_8c, (*puVar5 & 8) != 0)) {
              puVar5[0x19] = (uint)(fVar3 + (float)puVar5[0x19]);
            }
            if (((*puVar5 & 0x80000) != 0) && ((code *)puVar5[10] != (code *)0x0)) {
              (*(code *)puVar5[10])(puVar5,(float)param_1,(float)param_2);
            }
            puVar6 = puStack_94;
            if ((((*puVar5 & 0x40000) == 0) &&
                (iVar7 = (*(code *)param_6)(puVar5 + 0x11), puVar6 = puStack_94, iVar7 == 0)) &&
               ((*puVar5 & 0x2000) == 0)) {
              puVar5[1] = (uint)(float)param_2;
              *puVar5 = *puVar5 & 0xffffff7f;
            }
          }
          else {
            fVar3 = (float)puVar5[0xd5] * (float)param_1 + (float)puVar5[0xc9];
            puVar5[0xc9] = (uint)fVar3;
            if (fVar3 < (float)puVar5[2]) goto LAB_1001a2f3;
            lVar14 = __ftol();
            puVar5[0xc9] = (uint)((float)puVar5[0xc9] - (float)(int)lVar14);
            if ((*puVar5 & 0x10000) != 0) goto LAB_1001a2f3;
            puVar5[1] = (uint)(float)param_2;
            puVar6 = puStack_94;
          }
        }
        else {
          if ((*puVar5 & 0x80) != 0) {
            if (puVar5[0xcf] == 0) {
              puVar5[0xcf] = 2;
            }
            lVar14 = __ftol();
            puVar5[0xd0] = (uint)lVar14;
            if (0 < (int)(uint)lVar14) goto LAB_1001a02e;
          }
          puVar5[0xb] = *param_4;
          *param_4 = (uint)puVar5;
          if (puVar6 == (uint *)0x0) {
            *param_5 = (int)puVar11;
            puVar6 = puStack_94;
          }
          else {
            puVar6[0xb] = (uint)puVar11;
            puVar6 = puStack_94;
          }
        }
      }
    }
    else if (puVar11 != (uint *)0x0) {
      do {
        if ((*puVar11 & 0x40000) == 0) {
          (*(code *)param_6)(puVar11 + 0x11);
        }
        puVar11 = (uint *)puVar11[0xb];
      } while (puVar11 != (uint *)0x0);
      (**(code **)(DAT_100a5140 + 0x30))();
      return;
    }
    (**(code **)(DAT_100a5140 + 0x30))();
  }
  return;
}



/* ==== 1001ae60 HUD_GetStudioModelInterface ==== */

undefined4 __cdecl HUD_GetStudioModelInterface(int param_1,undefined4 *param_2,undefined4 *param_3)

{
  int iVar1;
  undefined4 *puVar2;
  
                    /* 0x1ae60  485  HUD_GetStudioModelInterface */
  if (param_1 != 1) {
    return 0;
  }
  *param_2 = &DAT_10078570;
  puVar2 = &DAT_100b20f8;
  for (iVar1 = 0x2b; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = *param_3;
    param_3 = param_3 + 1;
    puVar2 = puVar2 + 1;
  }
  FUN_1001ae50();
  return 1;
}



/* ==== 1001db00 FUN_1001db00 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_1001db00(void)

{
  float fVar1;
  int *piVar2;
  int *piVar3;
  bool bVar4;
  char cVar5;
  uint *puVar6;
  int iVar7;
  undefined4 uVar8;
  undefined4 *extraout_ECX;
  uint uVar9;
  uint uVar10;
  char *pcVar11;
  int iVar12;
  int iVar13;
  char *pcVar14;
  int iVar15;
  longlong lVar16;
  char *pcStack00000004;
  float fStack00000008;
  undefined4 *puStack00000010;
  float in_stack_00001468;
  int in_stack_0000146c;
  
  FUN_1004a6a0();
  _DAT_100a95ac = in_stack_00001468;
  puStack00000010 = extraout_ECX;
  FUN_10041a20();
  FUN_1001e4c0(1.0);
  if (DAT_100baee0 < 1) {
LAB_1001db43:
    bVar4 = false;
  }
  else {
    cVar5 = FUN_10046990();
    bVar4 = true;
    if (cVar5 != '\0') goto LAB_1001db43;
  }
  _DAT_100a95b8 = ((((float)bVar4 - _DAT_100a95b8) * 0.2 + _DAT_100a95b8) - 0.5) * 1.03 + 0.5;
  if (0.0 <= _DAT_100a95b8) {
    if (1.0 < _DAT_100a95b8) {
      _DAT_100a95b8 = 1.0;
    }
  }
  else {
    _DAT_100a95b8 = 0.0;
  }
  iVar12 = DAT_100a4dd0 + -0x140;
  lVar16 = __ftol();
  iVar7 = (int)lVar16;
  pcStack00000004 = (char *)(float)iVar12;
  __ftol();
  __ftol();
  (*DAT_100a501c)();
  FUN_1001d750(0,(char *)(float)iVar12,(float)DAT_100a4dcc * _DAT_100a95b8 * 1.1,(float)DAT_100a4dd0
               ,0,0,0x3e4ccccd,0x3f800000,0);
  if (bVar4) {
    iVar12 = FUN_10041a30();
    fVar1 = (float)iVar12 * 0.1;
    if (0.0 <= fVar1) {
      if (1.0 < fVar1) {
        fVar1 = 1.0;
      }
    }
    else {
      fVar1 = 0.0;
    }
    FUN_1001e4c0(1.0 - fVar1 * 0.35000002);
    lVar16 = __ftol();
    iVar13 = (int)lVar16;
    puVar6 = FUN_1004aff0((uint *)&DAT_100b7824,s_____PREVIOUS_QUESTION__1007aaf0);
    uVar9 = 0xffffffff;
    pcStack00000004 = &stack0x00000c94;
    pcVar14 = &DAT_100b7824;
    do {
      pcVar11 = pcVar14;
      if (uVar9 == 0) break;
      uVar9 = uVar9 - 1;
      pcVar11 = pcVar14 + 1;
      cVar5 = *pcVar14;
      pcVar14 = pcVar11;
    } while (cVar5 != '\0');
    uVar9 = ~uVar9;
    pcVar14 = pcVar11 + -uVar9;
    pcVar11 = pcStack00000004;
    for (uVar10 = uVar9 >> 2; uVar10 != 0; uVar10 = uVar10 - 1) {
      *(undefined4 *)pcVar11 = *(undefined4 *)pcVar14;
      pcVar14 = pcVar14 + 4;
      pcVar11 = pcVar11 + 4;
    }
    for (uVar9 = uVar9 & 3; uVar9 != 0; uVar9 = uVar9 - 1) {
      *pcVar11 = *pcVar14;
      pcVar14 = pcVar14 + 1;
      pcVar11 = pcVar11 + 1;
    }
    if (puVar6 != (uint *)0x0) {
      uVar9 = 0xffffffff;
      pcStack00000004 = &stack0x000004c4;
      pcVar14 = s_____PREVIOUS_QUESTION__1007aaf0;
      do {
        if (uVar9 == 0) break;
        uVar9 = uVar9 - 1;
        cVar5 = *pcVar14;
        pcVar14 = pcVar14 + 1;
      } while (cVar5 != '\0');
      uVar10 = 0xffffffff;
      pcVar14 = (char *)((~uVar9 - 1) + (int)puVar6);
      do {
        pcVar11 = pcVar14;
        if (uVar10 == 0) break;
        uVar10 = uVar10 - 1;
        pcVar11 = pcVar14 + 1;
        cVar5 = *pcVar14;
        pcVar14 = pcVar11;
      } while (cVar5 != '\0');
      uVar10 = ~uVar10;
      pcVar14 = pcVar11 + -uVar10;
      pcVar11 = pcStack00000004;
      for (uVar9 = uVar10 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
        *(undefined4 *)pcVar11 = *(undefined4 *)pcVar14;
        pcVar14 = pcVar14 + 4;
        pcVar11 = pcVar11 + 4;
      }
      for (uVar10 = uVar10 & 3; uVar10 != 0; uVar10 = uVar10 - 1) {
        *pcVar11 = *pcVar14;
        pcVar14 = pcVar14 + 1;
        pcVar11 = pcVar11 + 1;
      }
      (&stack0x00000000)[(int)(puVar6 + -0x402dae4)] = 0;
    }
    iVar15 = 0;
    if (0 < DAT_100baee0) {
      pcStack00000004 = (char *)(iVar12 * 3);
      do {
        uVar9 = 0xffffffff;
        pcVar14 = &DAT_100b7824;
        do {
          if (uVar9 == 0) break;
          uVar9 = uVar9 - 1;
          cVar5 = *pcVar14;
          pcVar14 = pcVar14 + 1;
        } while (cVar5 != '\0');
        pcVar11 = (char *)(~uVar9 - 1);
        pcVar14 = pcStack00000004;
        if (DAT_100baed4 != '\0') {
          pcVar14 = pcVar11;
        }
        if (iVar15 == 0) {
          FUN_100499f0(&stack0x000000dc,&DAT_10078410);
          if ((-1 < (int)pcVar14) && ((int)pcVar14 < (int)pcVar11)) {
            (&stack0x000000dc)[(int)pcVar14] = 0;
          }
          iVar12 = -1;
          DAT_100a95b4._0_1_ = 0;
          pcVar14 = &stack0x000004c4;
          do {
            if (iVar12 == 0) break;
            iVar12 = iVar12 + -1;
            cVar5 = *pcVar14;
            pcVar14 = pcVar14 + 1;
          } while (cVar5 != '\0');
          if (iVar12 != -2) {
            FUN_1001e7d0(0x19,iVar13,iVar7,&stack0x000004c4,100,200,100);
            iVar13 = iVar13 + 0x23;
          }
          FUN_1001e7d0(0x19,iVar13,iVar7,&stack0x000000dc,200,200,0);
          iVar13 = DAT_100a95a8 + 0x23;
        }
        else if ((int)pcVar11 <= (int)pcVar14) {
          FUN_100499f0(&stack0x000000dc,(byte *)s_Press___d___s_100789e0);
          DAT_100a95b4._0_1_ = 1;
          FUN_1001e7d0(0x19,iVar13,iVar7,&stack0x000000dc,100,200,100);
          DAT_100a95b4._0_1_ = 0;
          iVar13 = DAT_100a95a8 + 10;
        }
        iVar15 = iVar15 + 1;
      } while (iVar15 < DAT_100baee0);
    }
  }
  cVar5 = FUN_10046990();
  if ((cVar5 == '\0') && (!bVar4)) {
    iVar7 = FUN_10047670(1);
    FUN_10047670(2);
    iVar12 = FUN_10047670(5);
    pcStack00000004 = (char *)(uint)(iVar12 != 0);
    fVar1 = (float)(int)pcStack00000004;
    fStack00000008 =
         ((((float)(int)pcStack00000004 - _DAT_100a95bc) * 0.2 + _DAT_100a95bc) - 0.5) * 1.03 + 0.5;
    if (0.0 <= fStack00000008) {
      if (1.0 < fStack00000008) {
        fStack00000008 = 1.0;
      }
    }
    else {
      fStack00000008 = 0.0;
    }
    _DAT_100a95bc = fStack00000008;
    if (0.0 < fStack00000008) {
      FUN_10043dd0(fStack00000008);
    }
    _DAT_100a95c4 = (((fVar1 - _DAT_100a95c4) * 0.2 + _DAT_100a95c4) - 0.5) * 1.03 + 0.5;
    if (0.0 <= _DAT_100a95c4) {
      if (1.0 < _DAT_100a95c4) {
        _DAT_100a95c4 = 1.0;
      }
    }
    else {
      _DAT_100a95c4 = 0.0;
    }
    if ((((DAT_100a95c0 < 1) || (iVar7 < 1)) || (DAT_100a95c0 == iVar7)) || (_DAT_100a95c4 != 1.0))
    {
      if ((0.99 < _DAT_100a95c4) && (0 < iVar7)) {
        DAT_100a95c0 = iVar7;
      }
      if (_DAT_100a95c4 < 0.01) goto LAB_1001e02e;
    }
    else {
      _DAT_100a95c4 = 0.0;
LAB_1001e02e:
      if (iVar7 == 0) {
        DAT_100a95c0 = 0;
      }
    }
    if (0.0 < _DAT_100a95c4) {
      iVar12 = DAT_100a95c0;
      if (iVar7 == 0) {
        iVar12 = 0;
        iVar7 = DAT_100a95c0;
      }
      if ((iVar12 != iVar7) && (0 < iVar12)) {
        FUN_100499f0(&stack0x00000014,(byte *)s_sprites_efw_diary__02d_s_spr_100789c0);
        iVar7 = FUN_10031300(&stack0x00000014);
        iVar12 = (*DAT_100a4ffc)();
        iVar13 = (*DAT_100a4ff8)();
        pcStack00000004 = (char *)((DAT_100a4dcc - iVar12) + iVar12);
        FUN_1001d750((float)(DAT_100a4dcc - iVar12),(float)(DAT_100a4dd0 - iVar12),
                     (float)(int)pcStack00000004,(float)((DAT_100a4dd0 - iVar12) + iVar13),
                     0x3f800000,0x3f800000,0x3f800000,0x3f800000,iVar7);
      }
      FUN_100499f0(&stack0x00000014,(byte *)s_sprites_efw_diary__02d_s_spr_100789c0);
      iVar7 = FUN_10031300(&stack0x00000014);
      iVar12 = (*DAT_100a4ffc)();
      iVar13 = (*DAT_100a4ff8)();
      pcStack00000004 = (char *)((DAT_100a4dcc - iVar12) + iVar12);
      FUN_1001d750((float)(DAT_100a4dcc - iVar12),(float)(DAT_100a4dd0 - iVar12),
                   (float)(int)pcStack00000004,
                   (float)iVar13 * _DAT_100a95c4 + (float)(DAT_100a4dd0 - iVar12),0x3f800000,
                   0x3f800000,0x3f800000,0x3f800000,iVar7);
    }
  }
  FUN_10047660(1);
  __ftol();
  lVar16 = __ftol();
  FUN_1001daa0(0x14,0x78,(int)lVar16);
  FUN_10046590();
  FUN_10043bb0();
  uVar8 = puStack00000010[9];
  puStack00000010[9] = in_stack_00001468;
  puStack00000010[10] = uVar8;
  *(double *)(puStack00000010 + 0xc) = (double)(in_stack_00001468 - (float)puStack00000010[10]);
  if (_DAT_100a95a0 != 0.0) {
    DAT_100a95a4 = DAT_100a95a4 -
                   (float)(30000 - DAT_100a959c) *
                   ((in_stack_00001468 - (float)puStack00000010[10]) / _DAT_100a95a0);
    if (30000.0 < DAT_100a95a4) {
      DAT_100a95a4 = 30000.0;
    }
    if (DAT_100a95a4 < (float)DAT_100a959c) {
      DAT_100a95a4 = (float)DAT_100a959c;
    }
  }
  if (*(double *)(puStack00000010 + 0xc) < 0.0) {
    puStack00000010[0xc] = 0;
    puStack00000010[0xd] = 0;
  }
  if (DAT_100a5178 == (void *)0x0) goto LAB_1001e2fe;
  if (puStack00000010[0x569] != 0) {
    if (in_stack_0000146c == 0) {
      puStack00000010[0x569] = 0;
      FUN_1003c9a0(DAT_100a5178);
      FUN_1003ca30((int)DAT_100a5178);
      goto LAB_1001e2fe;
    }
    if (puStack00000010[0x569] != 0) goto LAB_1001e2fe;
  }
  if (in_stack_0000146c != 0) {
    FUN_1003c9a0(DAT_100a5178);
    FUN_1003d490((int)DAT_100a5178);
    FUN_1003c9e0((int)DAT_100a5178);
  }
LAB_1001e2fe:
  if ((DAT_100a95c8 != 0) && ((float)DAT_100a95c8 < in_stack_00001468)) {
    (*DAT_100a5040)();
    DAT_100a95c8 = 0;
  }
  piVar2 = (int *)*puStack00000010;
  puStack00000010[0x569] = in_stack_0000146c;
  for (; piVar2 != (int *)0x0; piVar2 = (int *)piVar2[1]) {
    piVar3 = (int *)*piVar2;
    if (in_stack_0000146c == 0) {
      if (((*(byte *)(piVar3 + 4) & 1) != 0) && ((*(byte *)(puStack00000010 + 0x15) & 4) == 0)) {
        (**(code **)(*piVar3 + 0xc))();
      }
    }
    else if ((*(byte *)(piVar3 + 4) & 2) != 0) {
      (**(code **)(*piVar3 + 0xc))();
    }
  }
  __ftol();
  FUN_10047670(3);
  FUN_100499f0(&stack0x00000078,(byte *)s__2d__02d__s_100789a0);
  if (puStack00000010[1] == 0) {
    uVar8 = FUN_10031300((byte *)s_sprites_efw_artslogo_spr_10078984);
    puStack00000010[1] = uVar8;
  }
  (*DAT_100a4ffc)();
  (*DAT_100a4ff8)();
  __ftol();
  (*DAT_100a5000)();
  (*DAT_100a500c)();
  return 1;
}



/* ==== 10022d00 HUD_Key_Event ==== */

undefined4 __cdecl HUD_Key_Event(int param_1,int param_2,byte *param_3)

{
  undefined4 uVar1;
  
                    /* 0x22d00  488  HUD_Key_Event */
  if (DAT_100a5178 != (void *)0x0) {
    uVar1 = FUN_1003df40(DAT_100a5178,param_1,param_2,param_3);
    return uVar1;
  }
  return 1;
}



/* ==== 10023de0 HUD_Shutdown ==== */

void HUD_Shutdown(void)

{
  IN_DeactivateMouse();
  FUN_10022bf0();
  return;
                    /* 0x23de0  496  HUD_Shutdown */
}



/* ==== 10031140 HUD_DrawNormalTriangles ==== */

void HUD_DrawNormalTriangles(void)

{
                    /* 0x31140  481  HUD_DrawNormalTriangles */
  FUN_10021b30(0x100a4118);
  return;
}



/* ==== 10031150 HUD_DrawTransparentTriangles ==== */

void HUD_DrawTransparentTriangles(void)

{
  undefined4 uStack_4;
  
  if (((1 < DAT_1008656c) || (DAT_100a958c <= 0.0)) || (uStack_4 = 1, DAT_100a95a4 <= 0.0)) {
    uStack_4 = 0;
  }
  (**(code **)(DAT_100a5138 + 0x34))(&DAT_100a9590,DAT_100a958c,DAT_100a95a4,uStack_4);
  return;
}



/* ==== 100352e0 FUN_100352e0 ==== */

undefined4 * __thiscall FUN_100352e0(void *this,undefined4 param_1)

{
  byte bVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  int *piVar6;
  Font *this_00;
  undefined4 uVar7;
  int iVar8;
  void *this_01;
  void *extraout_ECX;
  void *extraout_ECX_00;
  void *this_02;
  void *this_03;
  void *this_04;
  void *this_05;
  void *this_06;
  void *this_07;
  void *this_08;
  void *this_09;
  void *this_10;
  uint uVar9;
  uint uVar10;
  char *pcVar11;
  char *pcVar12;
  char *pcVar13;
  undefined4 *puVar14;
  char *pcVar15;
  byte *pbVar16;
  int *piVar17;
  void *pvVar18;
  undefined1 *puVar19;
  char cStack_6a2;
  char cStack_6a1;
  char cStack_6a0;
  char cStack_69f;
  char cStack_69e;
  char cStack_69d;
  int iStack_69c;
  int iStack_698;
  int iStack_690;
  byte abStack_68c [63];
  undefined1 uStack_64d;
  byte abStack_64c [63];
  undefined1 uStack_60d;
  byte abStack_60c [1024];
  undefined1 auStack_20c [512];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_10057bdc;
  pvStack_c = ExceptionList;
  pcVar13 = (char *)0x0;
  ExceptionList = &pvStack_c;
  *(undefined ***)this = &PTR_FUN_1006228c;
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  iVar4 = (*DAT_100a512c)(s_efw_textscheme_txt_10079e74,5,0);
  *(undefined4 *)((int)this + 0xc) = param_1;
  if ((DAT_100b30b8 & 1) == 0) {
    DAT_100b30b8 = DAT_100b30b8 | 1;
    FUN_1004b7bf(&DAT_100b30c0,0x7c,0x40,&LAB_100352a0);
    FUN_10049c8a(&LAB_10035b20);
  }
  iStack_69c = -1;
  puVar14 = &DAT_100b30c0;
  for (iVar8 = 0x7c0; iVar8 != 0; iVar8 = iVar8 + -1) {
    *puVar14 = 0;
    puVar14 = puVar14 + 1;
  }
  if (iVar4 == 0) {
    (*DAT_100a5094)(s_Unable_to_find___textscheme_txt_10079e50);
  }
  else {
    iStack_698 = (*DAT_100a5130)(iVar4,abStack_60c);
    iVar8 = -1;
    pbVar16 = abStack_60c;
    do {
      if (iVar8 == 0) break;
      iVar8 = iVar8 + -1;
      bVar1 = *pbVar16;
      pbVar16 = pbVar16 + 1;
    } while (bVar1 != 0);
    if (iVar8 != -2) {
      pcVar11 = (char *)0x100b3044;
      while ((int)pcVar11 < 0x100b4fc0) {
        _strncpy((char *)abStack_68c,(char *)abStack_60c,0x40);
        uStack_64d = 0;
        uVar7 = (*DAT_100a5130)(iStack_698,abStack_60c);
        uVar9 = FUN_10056910(this_01,abStack_60c,&DAT_10079e4c);
        if (uVar9 != 0) {
          if (iStack_69c < 0) {
            (*DAT_100a5090)(s_error_parsing_font_scheme_text_f_10079d60,abStack_60c);
          }
          else {
            (*DAT_100a5090)(s_error_parsing_font_scheme_text_f_10079d10,
                            &DAT_100b30c0 + iStack_69c * 0x1f,abStack_60c);
          }
          break;
        }
        uVar7 = (*DAT_100a5130)(uVar7,abStack_60c);
        _strncpy((char *)abStack_64c,(char *)abStack_60c,0x40);
        uStack_60d = 0;
        uVar9 = FUN_10056910(abStack_68c,abStack_68c,(byte *)s_SchemeName_10079e40);
        pvVar18 = extraout_ECX;
        pcVar12 = pcVar11;
        if (uVar9 == 0) {
          if (pcVar13 != (char *)0x0) {
            if (cStack_69e == '\0') {
              pcVar13[99] = -1;
              pcVar13[0x62] = -1;
              pcVar13[0x61] = -1;
              pcVar13[0x60] = -1;
            }
            if (cStack_6a0 == '\0') {
              *(undefined4 *)(pcVar13 + 0x68) = *(undefined4 *)(pcVar13 + 0x60);
            }
            if (cStack_69d == '\0') {
              *(undefined4 *)(pcVar13 + 0x70) = *(undefined4 *)(pcVar13 + 0x68);
            }
            if (cStack_6a1 == '\0') {
              pcVar13[0x67] = '\0';
              pcVar13[0x66] = '\0';
              pcVar13[0x65] = '\0';
              pcVar13[100] = '\0';
            }
            if (cStack_69f == '\0') {
              *(undefined4 *)(pcVar13 + 0x6c) = *(undefined4 *)(pcVar13 + 100);
            }
            if (cStack_6a2 == '\0') {
              *(undefined4 *)(pcVar13 + 0x74) = *(undefined4 *)(pcVar13 + 0x6c);
            }
            if (*(int *)(pcVar13 + 0x50) == 0) {
              pcVar13[0x50] = '\x11';
              pcVar13[0x51] = '\0';
              pcVar13[0x52] = '\0';
              pcVar13[0x53] = '\0';
            }
            if (pcVar13[0x20] == '\0') {
              uVar9 = 0xffffffff;
              pcVar12 = s_Arial_10079e38;
              do {
                pcVar15 = pcVar12;
                if (uVar9 == 0) break;
                uVar9 = uVar9 - 1;
                pcVar15 = pcVar12 + 1;
                cVar2 = *pcVar12;
                pcVar12 = pcVar15;
              } while (cVar2 != '\0');
              uVar9 = ~uVar9;
              pcVar12 = pcVar15 + -uVar9;
              pcVar13 = pcVar13 + 0x20;
              for (uVar10 = uVar9 >> 2; uVar10 != 0; uVar10 = uVar10 - 1) {
                *(undefined4 *)pcVar13 = *(undefined4 *)pcVar12;
                pcVar12 = pcVar12 + 4;
                pcVar13 = pcVar13 + 4;
              }
              for (uVar9 = uVar9 & 3; uVar9 != 0; uVar9 = uVar9 - 1) {
                *pcVar13 = *pcVar12;
                pcVar12 = pcVar12 + 1;
                pcVar13 = pcVar13 + 1;
              }
            }
          }
          pcVar12 = pcVar11 + 0x7c;
          iStack_69c = iStack_69c + 1;
          cStack_6a2 = '\0';
          cStack_69d = '\0';
          cStack_69f = '\0';
          cStack_6a0 = '\0';
          cStack_6a1 = '\0';
          cStack_69e = '\0';
          _strncpy(pcVar12,(char *)abStack_64c,0x20);
          pcVar11[0x9b] = '\0';
          pvVar18 = extraout_ECX_00;
          pcVar13 = pcVar12;
        }
        if (pcVar13 == (char *)0x0) {
          (*DAT_100a5090)(s_font_scheme_text_file_MUST_start_10079cd8);
          break;
        }
        uVar9 = FUN_10056910(pvVar18,abStack_68c,(byte *)s_FontName_10079e2c);
        if (uVar9 == 0) {
          _strncpy(pcVar13 + 0x20,(char *)abStack_64c,0x30);
          pcVar13[0x4f] = '\0';
        }
        else {
          uVar9 = FUN_10056910(this_02,abStack_68c,(byte *)s_FontSize_10079e20);
          if (uVar9 == 0) {
            uVar5 = FUN_100499e5(this_03,abStack_64c);
            *(undefined4 *)(pcVar13 + 0x50) = uVar5;
          }
          else {
            uVar9 = FUN_10056910(abStack_68c,abStack_68c,(byte *)s_FontWeight_10079e14);
            if (uVar9 == 0) {
              uVar5 = FUN_100499e5(this_04,abStack_64c);
              *(undefined4 *)(pcVar13 + 0x54) = uVar5;
            }
            else {
              uVar9 = FUN_10056910(this_04,abStack_68c,(byte *)s_FgColor_10079e0c);
              if (uVar9 == 0) {
                FUN_10035ad0(pcVar13 + 0x60,(char *)abStack_64c);
                cStack_69e = '\x01';
              }
              else {
                uVar9 = FUN_10056910(this_05,abStack_68c,(byte *)s_BgColor_10079e04);
                if (uVar9 == 0) {
                  FUN_10035ad0(pcVar13 + 100,(char *)abStack_64c);
                  cStack_6a1 = '\x01';
                }
                else {
                  uVar9 = FUN_10056910(this_06,abStack_68c,(byte *)s_FgColorArmed_10079df4);
                  if (uVar9 == 0) {
                    FUN_10035ad0(pcVar13 + 0x68,(char *)abStack_64c);
                    cStack_6a0 = '\x01';
                  }
                  else {
                    uVar9 = FUN_10056910(this_07,abStack_68c,(byte *)s_BgColorArmed_10079de4);
                    if (uVar9 == 0) {
                      FUN_10035ad0(pcVar13 + 0x6c,(char *)abStack_64c);
                      cStack_69f = '\x01';
                    }
                    else {
                      uVar9 = FUN_10056910(this_08,abStack_68c,(byte *)s_FgColorMousedown_10079dd0);
                      if (uVar9 == 0) {
                        FUN_10035ad0(pcVar13 + 0x70,(char *)abStack_64c);
                        cStack_69d = '\x01';
                      }
                      else {
                        uVar9 = FUN_10056910(this_09,abStack_68c,(byte *)s_BgColorMousedown_10079dbc
                                            );
                        if (uVar9 == 0) {
                          pcVar11 = pcVar13 + 0x74;
                        }
                        else {
                          uVar9 = FUN_10056910(this_10,abStack_68c,(byte *)s_BorderColor_10079db0);
                          if (uVar9 != 0) goto LAB_1003571d;
                          pcVar11 = pcVar13 + 0x78;
                        }
                        FUN_10035ad0(pcVar11,(char *)abStack_64c);
                        cStack_6a2 = '\x01';
                      }
                    }
                  }
                }
              }
            }
          }
        }
LAB_1003571d:
        iStack_698 = (*DAT_100a5130)(uVar7,abStack_60c);
        iVar8 = -1;
        pbVar16 = abStack_60c;
        do {
          if (iVar8 == 0) break;
          iVar8 = iVar8 + -1;
          bVar1 = *pbVar16;
          pbVar16 = pbVar16 + 1;
        } while (bVar1 != 0);
        pcVar11 = pcVar12;
        if (iVar8 == -2) break;
      }
    }
    (*DAT_100a5134)(iVar4);
    if (-1 < iStack_69c) goto LAB_1003586d;
  }
  uVar9 = 0xffffffff;
  iStack_69c = 0;
  pcVar13 = s_Default_Scheme_10079cc8;
  do {
    pcVar11 = pcVar13;
    if (uVar9 == 0) break;
    uVar9 = uVar9 - 1;
    pcVar11 = pcVar13 + 1;
    cVar2 = *pcVar13;
    pcVar13 = pcVar11;
  } while (cVar2 != '\0');
  uVar9 = ~uVar9;
  pcVar13 = pcVar11 + -uVar9;
  pcVar11 = (char *)&DAT_100b30c0;
  for (uVar10 = uVar9 >> 2; uVar10 != 0; uVar10 = uVar10 - 1) {
    *(undefined4 *)pcVar11 = *(undefined4 *)pcVar13;
    pcVar13 = pcVar13 + 4;
    pcVar11 = pcVar11 + 4;
  }
  for (uVar9 = uVar9 & 3; uVar9 != 0; uVar9 = uVar9 - 1) {
    *pcVar11 = *pcVar13;
    pcVar13 = pcVar13 + 1;
    pcVar11 = pcVar11 + 1;
  }
  uVar9 = 0xffffffff;
  pcVar13 = s_Arial_10079e38;
  do {
    pcVar11 = pcVar13;
    if (uVar9 == 0) break;
    uVar9 = uVar9 - 1;
    pcVar11 = pcVar13 + 1;
    cVar2 = *pcVar13;
    pcVar13 = pcVar11;
  } while (cVar2 != '\0');
  uVar9 = ~uVar9;
  pcVar13 = pcVar11 + -uVar9;
  pcVar11 = (char *)&DAT_100b30e0;
  for (uVar10 = uVar9 >> 2; uVar10 != 0; uVar10 = uVar10 - 1) {
    *(undefined4 *)pcVar11 = *(undefined4 *)pcVar13;
    pcVar13 = pcVar13 + 4;
    pcVar11 = pcVar11 + 4;
  }
  for (uVar9 = uVar9 & 3; uVar9 != 0; uVar9 = uVar9 - 1) {
    *pcVar11 = *pcVar13;
    pcVar13 = pcVar13 + 1;
    pcVar11 = pcVar11 + 1;
  }
  DAT_100b3110 = 0;
  DAT_100b3123 = 0xff;
  DAT_100b3122 = 0xff;
  DAT_100b3121 = 0xff;
  DAT_100b3120 = 0xff;
  DAT_100b312b = 0xff;
  DAT_100b312a = 0xff;
  DAT_100b3129 = 0xff;
  DAT_100b3128 = 0xff;
  DAT_100b3133 = 0xff;
  DAT_100b3132 = 0xff;
  DAT_100b3131 = 0xff;
  DAT_100b3130 = 0xff;
LAB_1003586d:
  iStack_69c = iStack_69c + 1;
  *(int *)((int)this + 8) = iStack_69c;
  piVar6 = operator_new(iStack_69c * 0x7c + 4);
  uStack_4 = 0;
  if (piVar6 == (int *)0x0) {
    piVar17 = (int *)0x0;
  }
  else {
    piVar17 = piVar6 + 1;
    *piVar6 = iStack_69c;
    FUN_1004b7bf(piVar17,0x7c,iStack_69c,&LAB_100352a0);
  }
  *(int **)((int)this + 4) = piVar17;
  uStack_4 = 0xffffffff;
  iStack_69c = 0;
  piVar6 = &DAT_100b30c0;
  for (uVar9 = *(int *)((int)this + 8) * 0x1f & 0x3fffffff; uVar9 != 0; uVar9 = uVar9 - 1) {
    *piVar17 = *piVar6;
    piVar6 = piVar6 + 1;
    piVar17 = piVar17 + 1;
  }
  for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
    *(char *)piVar17 = (char)*piVar6;
    piVar6 = (int *)((int)piVar6 + 1);
    piVar17 = (int *)((int)piVar17 + 1);
  }
  if (0 < *(int *)((int)this + 8)) {
    iVar4 = 0;
    do {
      iVar8 = 0;
      *(undefined4 *)(iVar4 + 0x58 + *(int *)((int)this + 4)) = 0;
      iStack_698 = iStack_69c;
      if (0 < iVar4) {
        do {
          pvVar18 = (void *)(iVar8 + 0x20 + *(int *)((int)this + 4));
          uVar9 = FUN_10056910(pvVar18,(byte *)(iVar4 + 0x20 + *(int *)((int)this + 4)),pvVar18);
          if (((uVar9 == 0) &&
              (iVar3 = *(int *)((int)this + 4),
              *(int *)(iVar4 + 0x50 + iVar3) == *(int *)(iVar8 + 0x50 + iVar3))) &&
             (*(int *)(iVar4 + 0x54 + iVar3) == *(int *)(iVar8 + 0x54 + iVar3))) {
            *(undefined4 *)(iVar4 + 0x58 + iVar3) = *(undefined4 *)(iVar8 + 0x58 + iVar3);
            *(undefined4 *)(iVar4 + 0x5c + *(int *)((int)this + 4)) = 0;
          }
          iVar8 = iVar8 + 0x7c;
          iStack_698 = iStack_698 + -1;
        } while (iStack_698 != 0);
      }
      iVar8 = iVar4 + *(int *)((int)this + 4);
      if (*(int *)(iVar4 + 0x58 + *(int *)((int)this + 4)) == 0) {
        pvVar18 = (void *)0x0;
        iStack_690 = -1;
        if ((DAT_100b4fc0 != 0) && (*(float *)(DAT_100b4fc0 + 0xc) != 0.0)) {
          uVar7 = *(undefined4 *)((int)this + 0xc);
          puVar19 = auStack_20c;
          pcVar13 = s_gfx_vgui_fonts__d__s_tga_10079cac;
          FUN_100499f0(puVar19,(byte *)s_gfx_vgui_fonts__d__s_tga_10079cac);
          pvVar18 = (void *)(*DAT_100a512c)(auStack_20c,5,&iStack_690,puVar19,pcVar13,uVar7,iVar8);
        }
        this_00 = operator_new(0x10);
        uStack_4 = 1;
        if (this_00 == (Font *)0x0) {
          uVar7 = 0;
        }
        else {
          iVar8 = *(int *)((int)this + 4) + iVar4;
          uVar7 = vgui::Font::Font(this_00,(char *)(iVar8 + 0x20),pvVar18,iStack_690,
                                   *(int *)(iVar8 + 0x50),0,0.0,*(int *)(iVar8 + 0x54),false,false,
                                   false,false);
        }
        uStack_4 = 0xffffffff;
        *(undefined4 *)(iVar4 + 0x58 + *(int *)((int)this + 4)) = uVar7;
        *(undefined4 *)(iVar4 + 0x5c + *(int *)((int)this + 4)) = 1;
      }
      *(char *)(iVar4 + 99 + *(int *)((int)this + 4)) =
           -1 - *(char *)(iVar4 + 99 + *(int *)((int)this + 4));
      *(char *)(iVar4 + 0x67 + *(int *)((int)this + 4)) =
           -1 - *(char *)(iVar4 + 0x67 + *(int *)((int)this + 4));
      *(char *)(iVar4 + 0x6b + *(int *)((int)this + 4)) =
           -1 - *(char *)(iVar4 + 0x6b + *(int *)((int)this + 4));
      *(char *)(iVar4 + 0x6f + *(int *)((int)this + 4)) =
           -1 - *(char *)(iVar4 + 0x6f + *(int *)((int)this + 4));
      *(char *)(iVar4 + 0x73 + *(int *)((int)this + 4)) =
           -1 - *(char *)(iVar4 + 0x73 + *(int *)((int)this + 4));
      pcVar13 = (char *)(iVar4 + 0x77 + *(int *)((int)this + 4));
      iVar4 = iVar4 + 0x7c;
      iStack_69c = iStack_69c + 1;
      *pcVar13 = -1 - *pcVar13;
    } while (iStack_69c < *(int *)((int)this + 8));
  }
  ExceptionList = pvStack_c;
  return this;
}



/* ==== 100419e0 FUN_100419e0 ==== */

void FUN_100419e0(void)

{
  (*DAT_100a5038)(s_EFWShow_1007aae8,&LAB_10041930);
  (*DAT_100a5038)(s_EFWData_1007aae0,&LAB_100418f0);
  DAT_100baed8 = 0;
  DAT_100baedc = 0;
  FUN_10042140();
  return;
}



/* ==== 10042140 FUN_10042140 ==== */

void FUN_10042140(void)

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
  undefined1 auStack_9c [16];
  undefined1 auStack_8c [16];
  uint auStack_7c [2];
  undefined1 auStack_74 [4];
  char *pcStack_70;
  undefined1 auStack_64 [16];
  undefined1 auStack_54 [32];
  undefined4 auStack_34 [2];
  undefined1 auStack_2c [32];
  void *pvStack_c;
  undefined1 *puStack_8;
  int iStack_4;
  
  iStack_4 = 0xffffffff;
  puStack_8 = &LAB_10058182;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  FUN_100424b0(auStack_9c,'\0');
  uVar5 = 0xffffffff;
  pcVar7 = s_efw_compound_gate_guard_10077178;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  FUN_10042900(auStack_9c,(undefined4 *)s_efw_compound_gate_guard_10077178,~uVar5 - 1);
  iStack_4 = 0;
  FUN_100424b0(auStack_8c,'\0');
  iStack_4._0_1_ = 1;
  piVar3 = (int *)FUN_100426c0(auStack_54,auStack_9c,auStack_8c);
  iStack_4._0_1_ = 2;
  piVar3 = (int *)FUN_10042500(&DAT_100baee8,auStack_7c,piVar3);
  iVar2 = *piVar3;
  iStack_4._0_1_ = 1;
  FUN_10042240((int)auStack_54);
  iStack_4 = (uint)iStack_4._1_3_ << 8;
  FUN_100424b0(auStack_8c,'\x01');
  uVar5 = 0xffffffff;
  pcVar7 = s_Gate_Guard_1007ab50;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  FUN_10042900((void *)(iVar2 + 0x1c),(undefined4 *)s_Gate_Guard_1007ab50,~uVar5 - 1);
  iStack_4 = 0xffffffff;
  FUN_100424b0(auStack_9c,'\x01');
  FUN_100424b0(auStack_9c,'\0');
  uVar5 = 0xffffffff;
  pcVar7 = s_efw_electrician_10076e4c;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  FUN_10042900(auStack_9c,(undefined4 *)s_efw_electrician_10076e4c,~uVar5 - 1);
  iStack_4 = 3;
  FUN_100424b0(auStack_8c,'\0');
  iStack_4._0_1_ = 4;
  piVar3 = (int *)FUN_100426c0(auStack_54,auStack_9c,auStack_8c);
  iStack_4._0_1_ = 5;
  piVar3 = (int *)FUN_10042500(&DAT_100baee8,auStack_7c,piVar3);
  iVar2 = *piVar3;
  iStack_4._0_1_ = 4;
  FUN_10042240((int)auStack_54);
  iStack_4 = CONCAT31(iStack_4._1_3_,3);
  FUN_100424b0(auStack_8c,'\x01');
  uVar5 = 0xffffffff;
  pcVar7 = s_Electrician_1007ab44;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  FUN_10042900((void *)(iVar2 + 0x1c),(undefined4 *)s_Electrician_1007ab44,~uVar5 - 1);
  iStack_4 = 0xffffffff;
  FUN_100424b0(auStack_9c,'\x01');
  FUN_100424b0(auStack_9c,'\0');
  uVar5 = 0xffffffff;
  pcVar7 = s_detainee_1007ab38;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  FUN_10042900(auStack_9c,(undefined4 *)s_detainee_1007ab38,~uVar5 - 1);
  iStack_4 = 6;
  FUN_100424b0(auStack_8c,'\0');
  iStack_4._0_1_ = 7;
  piVar3 = (int *)FUN_100426c0(auStack_54,auStack_9c,auStack_8c);
  iStack_4._0_1_ = 8;
  piVar3 = (int *)FUN_10042500(&DAT_100baee8,auStack_7c,piVar3);
  iVar2 = *piVar3;
  iStack_4._0_1_ = 7;
  FUN_10042240((int)auStack_54);
  iStack_4 = CONCAT31(iStack_4._1_3_,6);
  FUN_100424b0(auStack_8c,'\x01');
  uVar5 = 0xffffffff;
  pcVar7 = s_Detainee_1007ab2c;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  FUN_10042900((void *)(iVar2 + 0x1c),(undefined4 *)s_Detainee_1007ab2c,~uVar5 - 1);
  iStack_4 = 0xffffffff;
  FUN_100424b0(auStack_9c,'\x01');
  FUN_100424b0(auStack_74,'\0');
  uVar5 = 0xffffffff;
  pcVar7 = s_detainee_queue_1007ab1c;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  uVar5 = ~uVar5 - 1;
  uVar4 = FUN_100429c0(auStack_74,uVar5,'\x01');
  if ((char)uVar4 != '\0') {
    pcVar7 = s_detainee_queue_1007ab1c;
    for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
      *(undefined4 *)pcStack_70 = *(undefined4 *)pcVar7;
      pcVar7 = pcVar7 + 4;
      pcStack_70 = pcStack_70 + 4;
    }
    for (uVar6 = uVar5 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
      *pcStack_70 = *pcVar7;
      pcVar7 = pcVar7 + 1;
      pcStack_70 = pcStack_70 + 1;
    }
    FUN_100429a0(auStack_74,uVar5);
  }
  iStack_4 = 9;
  FUN_100424b0(auStack_64,'\0');
  iStack_4._0_1_ = 10;
  piVar3 = (int *)FUN_100426c0(auStack_2c,auStack_74,auStack_64);
  iStack_4._0_1_ = 0xb;
  piVar3 = (int *)FUN_10042500(&DAT_100baee8,auStack_34,piVar3);
  iVar2 = *piVar3;
  iStack_4._0_1_ = 10;
  FUN_10042240((int)auStack_2c);
  iStack_4 = CONCAT31(iStack_4._1_3_,9);
  FUN_100424b0(auStack_64,'\x01');
  uVar5 = 0xffffffff;
  this = (void *)(iVar2 + 0x1c);
  pcVar7 = s_Detainee_in_queue_1007ab08;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar7 + 1;
  } while (cVar1 != '\0');
  uVar5 = ~uVar5 - 1;
  auStack_7c[0] = uVar5;
  uVar4 = FUN_100429c0(this,uVar5,'\x01');
  if ((char)uVar4 != '\0') {
    pcVar7 = s_Detainee_in_queue_1007ab08;
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
    FUN_100429a0(this,uVar5);
  }
  iStack_4 = 0xffffffff;
  FUN_100424b0(auStack_74,'\x01');
  ExceptionList = pvStack_c;
  return;
}



/* ==== 100436c0 FUN_100436c0 ==== */

void FUN_100436c0(void)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  
  iVar2 = DAT_100a4dd0;
  iVar1 = DAT_100a4dcc;
  uVar3 = (*DAT_100a4ff0)(s_sprites_efw_grey_spr_1007ab64);
  iVar4 = (*DAT_100a4ffc)(uVar3,0);
  iVar5 = (*DAT_100a4ff8)(uVar3,0);
  (*DAT_100a5000)(uVar3,0,0,0);
  iVar7 = 0;
  if (0 < iVar1) {
    do {
      iVar6 = 0;
      if (0 < iVar2) {
        do {
          (*DAT_100a5004)(0,iVar7,iVar6,0);
          iVar6 = iVar6 + iVar5;
        } while (iVar6 < iVar2);
      }
      iVar7 = iVar7 + iVar4;
    } while (iVar7 < iVar1);
  }
  return;
}



/* ==== 100442f0 weapon_efw_Pliers ==== */

void __cdecl weapon_efw_Pliers(int param_1)

{
                    /* 0x442f0  537  weapon_efw_Pliers */
  FUN_10044440(param_1);
  return;
}



/* ==== 10044300 weapon_efw_Lever ==== */

void __cdecl weapon_efw_Lever(int param_1)

{
                    /* 0x44300  535  weapon_efw_Lever */
  FUN_100444b0(param_1);
  return;
}



/* ==== 10044310 weapon_efw_Branch ==== */

void __cdecl weapon_efw_Branch(int param_1)

{
                    /* 0x44310  532  weapon_efw_Branch */
  FUN_10044520(param_1);
  return;
}



/* ==== 10044320 weapon_efw_MobilePhone ==== */

void __cdecl weapon_efw_MobilePhone(int param_1)

{
                    /* 0x44320  536  weapon_efw_MobilePhone */
  FUN_10044590(param_1);
  return;
}



/* ==== 10044330 weapon_efw_IDTag ==== */

void __cdecl weapon_efw_IDTag(int param_1)

{
                    /* 0x44330  534  weapon_efw_IDTag */
  FUN_10044600(param_1);
  return;
}



/* ==== 10044340 weapon_efw_RedPhoneCard ==== */

void __cdecl weapon_efw_RedPhoneCard(int param_1)

{
                    /* 0x44340  538  weapon_efw_RedPhoneCard */
  FUN_10044670(param_1);
  return;
}



/* ==== 10044350 weapon_efw_GreenPhoneCard ==== */

void __cdecl weapon_efw_GreenPhoneCard(int param_1)

{
                    /* 0x44350  533  weapon_efw_GreenPhoneCard */
  FUN_100446e0(param_1);
  return;
}



/* ==== 10044360 weapon_efw_BluePhoneCard ==== */

void __cdecl weapon_efw_BluePhoneCard(int param_1)

{
                    /* 0x44360  531  weapon_efw_BluePhoneCard */
  FUN_10044750(param_1);
  return;
}



/* ==== 10044370 weapon_efw_WashingPowder ==== */

void __cdecl weapon_efw_WashingPowder(int param_1)

{
                    /* 0x44370  539  weapon_efw_WashingPowder */
  FUN_100447c0(param_1);
  return;
}



/* ==== 10044440 FUN_10044440 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl FUN_10044440(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *this;
  
  if ((param_1 == 0) && (iVar2 = (*DAT_1007ef24)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = _DAT_00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (this = (undefined4 *)(*DAT_1007efcc)(iVar2,0x130), iVar1 = param_1, this != (undefined4 *)0x0)
     ) {
    FUN_100440c0(this,s_models_w_Pliers_mdl_10076be8,s_models_v_Pliers_mdl_10076bfc,
                 s_models_p_Pliers_mdl_10076c10);
    *this = &PTR_FUN_1005c374;
    this[1] = param_1;
    return;
  }
  _DAT_00000004 = iVar1;
  return;
}



/* ==== 10044600 FUN_10044600 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl FUN_10044600(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *this;
  
  if ((param_1 == 0) && (iVar2 = (*DAT_1007ef24)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = _DAT_00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (this = (undefined4 *)(*DAT_1007efcc)(iVar2,0x130), iVar1 = param_1, this != (undefined4 *)0x0)
     ) {
    FUN_100440c0(this,s_models_w_IDTag_mdl_10077124,s_models_v_IDTag_mdl_10077138,
                 s_models_p_IDTag_mdl_1007714c);
    *this = &PTR_FUN_1005ca34;
    this[1] = param_1;
    return;
  }
  _DAT_00000004 = iVar1;
  return;
}



/* ==== 100447c0 FUN_100447c0 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl FUN_100447c0(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *this;
  
  if ((param_1 == 0) && (iVar2 = (*DAT_1007ef24)(), iVar2 != 0)) {
    param_1 = iVar2 + 0x80;
  }
  iVar2 = *(int *)(param_1 + 0x208);
  if (((iVar2 == 0) || (iVar1 = _DAT_00000004, *(int *)(iVar2 + 0x7c) == 0)) &&
     (this = (undefined4 *)(*DAT_1007efcc)(iVar2,300), iVar1 = param_1, this != (undefined4 *)0x0))
  {
    FUN_100440c0(this,s_models_w_WashingPowder_mdl_100772e4,s_models_v_WashingPowder_mdl_10077300,
                 s_models_p_WashingPowder_mdl_1007731c);
    *this = &PTR_FUN_1005d0f4;
    this[1] = param_1;
    return;
  }
  _DAT_00000004 = iVar1;
  return;
}



/* ==== 10044880 FUN_10044880 ==== */

undefined1 FUN_10044880(void)

{
  byte bVar1;
  char *pcVar2;
  int iVar3;
  byte *pbVar4;
  byte *pbVar5;
  bool bVar6;
  
  pcVar2 = s_efw_prototype_level3_1007acc0;
  pbVar5 = (byte *)(*(int *)(DAT_1007f7f8 + 0x98) + *(int *)(DAT_1007f7f8 + 0xc));
  pbVar4 = pbVar5;
  do {
    bVar1 = *pcVar2;
    bVar6 = bVar1 < *pbVar4;
    if (bVar1 != *pbVar4) {
LAB_100448c3:
      iVar3 = (1 - (uint)bVar6) - (uint)(bVar6 != 0);
      goto LAB_100448c8;
    }
    if (bVar1 == 0) break;
    bVar1 = pcVar2[1];
    bVar6 = bVar1 < pbVar4[1];
    if (bVar1 != pbVar4[1]) goto LAB_100448c3;
    pcVar2 = pcVar2 + 2;
    pbVar4 = pbVar4 + 2;
  } while (bVar1 != 0);
  iVar3 = 0;
LAB_100448c8:
  if (iVar3 == 0) {
    return 2;
  }
  pcVar2 = s_efw_prototype_level2_1007aca8;
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



/* ==== 10044e30 FUN_10044e30 ==== */

/* WARNING: Removing unreachable block (ram,0x10044ebc) */
/* WARNING: Removing unreachable block (ram,0x10044eca) */
/* WARNING: Removing unreachable block (ram,0x10044eea) */
/* WARNING: Removing unreachable block (ram,0x10044eeb) */
/* WARNING: Removing unreachable block (ram,0x10044f09) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_10044e30(void)

{
  int *piVar1;
  int *piVar2;
  
  (*DAT_100a5038)(s_EFW_Cntxt_1007ace4,&LAB_10044950);
  (*DAT_100a5038)(s_EFW_CtPrv_1007acd8,&LAB_10044bd0);
  _DAT_100bc358 = (*DAT_100a4ff0)(s__sprites_ascale_spr_1007896f + 1);
  piVar1 = DAT_100bc330;
  DAT_100bc338 = 0;
  DAT_100bc350 = 0;
  _DAT_100bc354 = 0;
  for (piVar2 = DAT_100bc32c; piVar2 != piVar1; piVar2 = piVar2 + 1) {
    if ((undefined4 *)*piVar2 != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)*piVar2)(1);
    }
    *piVar2 = 0;
  }
  DAT_100bc330 = DAT_100bc32c;
  DAT_100bc348 = DAT_100bc344;
  DAT_100bc384 = 0;
  return;
}



/* ==== 10044f70 FUN_10044f70 ==== */

/* WARNING: Removing unreachable block (ram,0x10045966) */
/* WARNING: Removing unreachable block (ram,0x10045985) */
/* WARNING: Removing unreachable block (ram,0x10045994) */

void __cdecl FUN_10044f70(int *param_1)

{
  byte bVar1;
  undefined4 uVar2;
  float fVar3;
  float fVar4;
  int *piVar5;
  int *piVar6;
  int *piVar7;
  int iVar8;
  undefined4 *puVar9;
  undefined4 uVar10;
  undefined4 *puVar11;
  byte *pbVar12;
  byte *pbVar13;
  int iVar14;
  uint uVar15;
  uint uVar16;
  undefined4 *puVar17;
  char *pcVar18;
  uint uVar19;
  bool bVar20;
  undefined1 uStack_4e;
  undefined1 uStack_4d;
  undefined4 *puStack_4c;
  undefined4 *puStack_48;
  undefined4 *puStack_44;
  undefined4 *local_40;
  undefined4 *puStack_3c;
  int local_34;
  int local_30;
  int local_2c;
  undefined4 auStack_28 [3];
  undefined1 auStack_1c [4];
  int *piStack_18;
  int *piStack_14;
  undefined4 uStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  uint uStack_4;
  
  piVar6 = param_1;
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_10058380;
  pvStack_c = ExceptionList;
  local_34 = param_1[9];
  local_30 = param_1[10];
  local_2c = param_1[0xb];
  ExceptionList = &pvStack_c;
  iVar8 = (**(code **)(DAT_100a5138 + 0x30))(&local_34,&local_40);
  if (iVar8 == 1) {
    ExceptionList = pvStack_c;
    return;
  }
  fVar3 = (float)DAT_100a4dcc;
  local_40 = (undefined4 *)(((float)local_40 + 1.0) * fVar3 * 0.5);
  fVar4 = (float)DAT_100a4dd0;
  puStack_3c = (undefined4 *)((1.0 - (float)puStack_3c) * fVar4 * 0.5);
  if ((float)local_40 <= 90.0) {
    ExceptionList = pvStack_c;
    return;
  }
  if ((float)puStack_3c <= 90.0) {
    ExceptionList = pvStack_c;
    return;
  }
  if (fVar3 - 90.0 <= (float)local_40) {
    ExceptionList = pvStack_c;
    return;
  }
  if (fVar4 - 90.0 <= (float)puStack_3c) {
    ExceptionList = pvStack_c;
    return;
  }
  puStack_4c = (undefined4 *)(fVar3 - 180.0);
  if ((float)local_40 < 180.0) {
    local_40 = (undefined4 *)0x43340000;
  }
  if ((float)puStack_4c < (float)local_40) {
    local_40 = puStack_4c;
  }
  puStack_48 = (undefined4 *)(fVar4 - 180.0);
  if ((float)puStack_3c < 180.0) {
    puStack_3c = (undefined4 *)0x43340000;
  }
  if ((float)puStack_48 < (float)puStack_3c) {
    puStack_3c = puStack_48;
  }
  auStack_1c[0] = param_1._0_1_;
  piStack_18 = (int *)0x0;
  piStack_14 = (int *)0x0;
  uStack_10 = 0;
  iVar8 = *piVar6;
  uStack_4 = 0;
  if (iVar8 == 0) {
    puVar9 = operator_new(0x44);
    uStack_4._0_1_ = 1;
    puStack_4c = puVar9;
    if (puVar9 == (undefined4 *)0x0) {
      puStack_48 = (undefined4 *)0x0;
    }
    else {
      uVar10 = (*DAT_100a4ff0)(s_sprites_efw_speech_bubble_spr_1007ae6c);
      puVar9[1] = 0;
      puVar9[2] = 0;
      puVar9[3] = uVar10;
      *puVar9 = &PTR_LAB_10064f60;
      uStack_4._0_1_ = 2;
      puVar9[4] = piVar6;
      puVar9[5] = 0xffffffff;
      FUN_100469a0(puVar9 + 6,s_Talk_to__C_1007ae60,&uStack_4e);
      uStack_4._0_1_ = 3;
      FUN_100469a0(puVar9 + 10,s_efw_Talk__c_1007ae54,&uStack_4d);
      uStack_4._0_1_ = 4;
      FUN_100083d0(puVar9 + 0xe,&local_40);
      *puVar9 = &PTR_FUN_10064f48;
      puStack_48 = puVar9;
    }
    uStack_4 = (uint)uStack_4._1_3_ << 8;
    FUN_10046e20(auStack_1c,piStack_14,1,&puStack_48);
    iVar8 = 0;
    puStack_48 = (undefined4 *)0x0;
    puVar11 = (undefined4 *)0x0;
    do {
      iVar14 = (&DAT_100a37a8)[(int)puVar11 + iVar8];
      if ((iVar14 != 0) && (*(int *)(iVar14 + 0x9c) != 0)) {
        puVar9 = operator_new(0x44);
        puStack_44 = puVar9;
        if (puVar9 == (undefined4 *)0x0) {
          puStack_4c = (undefined4 *)0x0;
        }
        else {
          uVar10 = *(undefined4 *)(iVar14 + 0xbc);
          uVar2 = *(undefined4 *)(iVar14 + 0x9c);
          puVar9[1] = 0;
          puVar9[2] = 0;
          puVar9[3] = uVar10;
          *puVar9 = &PTR_LAB_10064f60;
          uStack_4._0_1_ = 6;
          puVar9[4] = param_1;
          puVar9[5] = uVar2;
          FUN_100469a0(puVar9 + 6,s_Give__i_to__C_1007ae44,&uStack_4d);
          uStack_4._0_1_ = 7;
          FUN_100469a0(puVar9 + 10,s_efw_Give__d__c_1007ae34,&uStack_4e);
          uStack_4 = CONCAT31(uStack_4._1_3_,8);
          FUN_100083d0(puVar9 + 0xe,&local_40);
          *puVar9 = &PTR_FUN_10064f48;
          puStack_4c = puVar9;
        }
        uStack_4 = uStack_4 & 0xffffff00;
        FUN_10046e20(auStack_1c,piStack_14,1,&puStack_4c);
        puVar11 = puStack_48;
      }
      iVar8 = iVar8 + 1;
      if (9 < iVar8) {
        iVar8 = 0;
        puVar11 = (undefined4 *)((int)puVar11 + 0xb);
        puStack_48 = puVar11;
      }
      puVar9 = DAT_100bc330;
      piVar5 = piStack_18;
    } while ((int)puVar11 < 0x37);
    goto joined_r0x100457f5;
  }
  puVar9 = DAT_100bc330;
  piVar5 = piStack_18;
  if (iVar8 == 1) {
    pbVar13 = (byte *)(piVar6 + 1);
    pcVar18 = s_efw_IDTag_Position_100771d4;
    pbVar12 = pbVar13;
    do {
      bVar1 = *pbVar12;
      bVar20 = bVar1 < (byte)*pcVar18;
      if (bVar1 != *pcVar18) {
LAB_100452cf:
        iVar8 = (1 - (uint)bVar20) - (uint)(bVar20 != 0);
        goto LAB_100452d4;
      }
      if (bVar1 == 0) break;
      bVar1 = pbVar12[1];
      bVar20 = bVar1 < (byte)pcVar18[1];
      if (bVar1 != pcVar18[1]) goto LAB_100452cf;
      pbVar12 = pbVar12 + 2;
      pcVar18 = pcVar18 + 2;
    } while (bVar1 != 0);
    iVar8 = 0;
LAB_100452d4:
    if (iVar8 == 0) {
      iVar8 = FUN_10044f30(0x14);
      puVar9 = DAT_100bc330;
      piVar5 = piStack_18;
      if (iVar8 != 0) {
        puVar9 = operator_new(0x44);
        puStack_44 = puVar9;
        if (puVar9 == (undefined4 *)0x0) {
          puStack_48 = (undefined4 *)0x0;
        }
        else {
          uVar10 = *(undefined4 *)(iVar8 + 0xbc);
          uVar2 = *(undefined4 *)(iVar8 + 0x9c);
          puVar9[1] = 0;
          puVar9[2] = 0;
          puVar9[3] = uVar10;
          *puVar9 = &PTR_LAB_10064f60;
          uStack_4._0_1_ = 10;
          puVar9[4] = param_1;
          puVar9[5] = uVar2;
          FUN_100469a0(puVar9 + 6,s_Place__i_on_fence_1007ae20,(undefined1 *)&param_1);
          uStack_4._0_1_ = 0xb;
          FUN_100469a0(puVar9 + 10,s_efw_UseWithMarker__d__c_1007ae08,&uStack_4d);
          uStack_4 = CONCAT31(uStack_4._1_3_,0xc);
          FUN_100083d0(puVar9 + 0xe,&local_40);
          *puVar9 = &PTR_FUN_10064f48;
          puStack_48 = puVar9;
        }
        uStack_4 = uStack_4 & 0xffffff00;
        FUN_10046e20(auStack_1c,piStack_14,1,&puStack_48);
        puVar9 = DAT_100bc330;
        piVar5 = piStack_18;
      }
      goto joined_r0x100457f5;
    }
    pcVar18 = s_efw_kitchen_bin_10076eb8;
    pbVar12 = pbVar13;
    do {
      bVar1 = *pbVar12;
      bVar20 = bVar1 < (byte)*pcVar18;
      if (bVar1 != *pcVar18) {
LAB_100453c1:
        iVar8 = (1 - (uint)bVar20) - (uint)(bVar20 != 0);
        goto LAB_100453c6;
      }
      if (bVar1 == 0) break;
      bVar1 = pbVar12[1];
      bVar20 = bVar1 < (byte)pcVar18[1];
      if (bVar1 != pcVar18[1]) goto LAB_100453c1;
      pbVar12 = pbVar12 + 2;
      pcVar18 = pcVar18 + 2;
    } while (bVar1 != 0);
    iVar8 = 0;
LAB_100453c6:
    if (iVar8 == 0) {
      iVar8 = FUN_10044f30(0x10);
      puVar9 = DAT_100bc330;
      piVar5 = piStack_18;
      if (iVar8 != 0) {
        puVar9 = operator_new(0x44);
        puStack_44 = puVar9;
        if (puVar9 == (undefined4 *)0x0) {
          puStack_48 = (undefined4 *)0x0;
        }
        else {
          uVar10 = *(undefined4 *)(iVar8 + 0xbc);
          uVar2 = *(undefined4 *)(iVar8 + 0x9c);
          puVar9[1] = 0;
          puVar9[2] = 0;
          puVar9[3] = uVar10;
          *puVar9 = &PTR_LAB_10064f60;
          uStack_4._0_1_ = 0xe;
          puVar9[4] = param_1;
          puVar9[5] = uVar2;
          FUN_100469a0(puVar9 + 6,s_Hide__i_in_bin_1007adf8,(undefined1 *)&param_1);
          uStack_4._0_1_ = 0xf;
          FUN_100469a0(puVar9 + 10,s_efw_UseWithMarker__d__c_1007ae08,&uStack_4d);
          uStack_4 = CONCAT31(uStack_4._1_3_,0x10);
          FUN_100083d0(puVar9 + 0xe,&local_40);
          *puVar9 = &PTR_FUN_10064f48;
          puStack_48 = puVar9;
        }
        uStack_4 = uStack_4 & 0xffffff00;
        FUN_10046e20(auStack_1c,piStack_14,1,&puStack_48);
        puVar9 = DAT_100bc330;
        piVar5 = piStack_18;
      }
      goto joined_r0x100457f5;
    }
    pcVar18 = s_efw_hiding_place_1007ade4;
    pbVar12 = pbVar13;
    do {
      bVar1 = *pbVar12;
      bVar20 = bVar1 < (byte)*pcVar18;
      if (bVar1 != *pcVar18) {
LAB_100454b3:
        iVar8 = (1 - (uint)bVar20) - (uint)(bVar20 != 0);
        goto LAB_100454b8;
      }
      if (bVar1 == 0) break;
      bVar1 = pbVar12[1];
      bVar20 = bVar1 < (byte)pcVar18[1];
      if (bVar1 != pcVar18[1]) goto LAB_100454b3;
      pbVar12 = pbVar12 + 2;
      pcVar18 = pcVar18 + 2;
    } while (bVar1 != 0);
    iVar8 = 0;
LAB_100454b8:
    if (iVar8 == 0) {
      puVar9 = operator_new(0x44);
      uStack_4._0_1_ = 0x11;
      puStack_44 = puVar9;
      if (puVar9 == (undefined4 *)0x0) {
        puStack_48 = (undefined4 *)0x0;
      }
      else {
        uVar10 = (*DAT_100a4ff0)(s_sprites_efw_hide_icon_spr_1007adc8);
        puVar9[1] = 0;
        puVar9[2] = 0;
        puVar9[3] = uVar10;
        *puVar9 = &PTR_LAB_10064f60;
        uStack_4._0_1_ = 0x12;
        puVar9[4] = piVar6;
        puVar9[5] = 0xffffffff;
        FUN_100469a0(puVar9 + 6,s_Hide_under_the_building_1007adb0,(undefined1 *)&param_1);
        uStack_4._0_1_ = 0x13;
        FUN_100469a0(puVar9 + 10,s_efw_HideUnderBuilding_1007ad98,&uStack_4d);
        uStack_4._0_1_ = 0x14;
        FUN_100083d0(puVar9 + 0xe,&local_40);
        *puVar9 = &PTR_FUN_10064f48;
        puStack_48 = puVar9;
      }
      uStack_4 = (uint)uStack_4._1_3_ << 8;
      FUN_10046e20(auStack_1c,piStack_14,1,&puStack_48);
      puVar9 = DAT_100bc330;
      piVar5 = piStack_18;
      goto joined_r0x100457f5;
    }
    pcVar18 = s_efw_cage_door_10076f48;
    pbVar12 = pbVar13;
    do {
      bVar1 = *pbVar12;
      bVar20 = bVar1 < (byte)*pcVar18;
      if (bVar1 != *pcVar18) {
LAB_10045598:
        iVar8 = (1 - (uint)bVar20) - (uint)(bVar20 != 0);
        goto LAB_1004559d;
      }
      if (bVar1 == 0) break;
      bVar1 = pbVar12[1];
      bVar20 = bVar1 < (byte)pcVar18[1];
      if (bVar1 != pcVar18[1]) goto LAB_10045598;
      pbVar12 = pbVar12 + 2;
      pcVar18 = pcVar18 + 2;
    } while (bVar1 != 0);
    iVar8 = 0;
LAB_1004559d:
    if (iVar8 != 0) {
      pcVar18 = s_efw_PliersMarker_1007ad64;
      do {
        bVar1 = *pbVar13;
        bVar20 = bVar1 < (byte)*pcVar18;
        if (bVar1 != *pcVar18) {
LAB_10045683:
          iVar8 = (1 - (uint)bVar20) - (uint)(bVar20 != 0);
          goto LAB_10045688;
        }
        if (bVar1 == 0) break;
        bVar1 = pbVar13[1];
        bVar20 = bVar1 < (byte)pcVar18[1];
        if (bVar1 != pcVar18[1]) goto LAB_10045683;
        pbVar13 = pbVar13 + 2;
        pcVar18 = pcVar18 + 2;
      } while (bVar1 != 0);
      iVar8 = 0;
LAB_10045688:
      if (iVar8 != 0) goto joined_r0x100457f5;
      puVar9 = operator_new(0x44);
      uStack_4._0_1_ = 0x19;
      puStack_44 = puVar9;
      if (puVar9 == (undefined4 *)0x0) {
        puStack_48 = (undefined4 *)0x0;
      }
      else {
        uVar10 = (*DAT_100a4ff0)(s_sprites_efw_item_pliers_spr_1007ad48);
        puVar9[1] = 0;
        puVar9[2] = 0;
        puVar9[3] = uVar10;
        *puVar9 = &PTR_LAB_10064f60;
        uStack_4._0_1_ = 0x1a;
        puVar9[4] = piVar6;
        puVar9[5] = 0xffffffff;
        FUN_100469a0(puVar9 + 6,s_Take_pliers_1007ad3c,(undefined1 *)&param_1);
        uStack_4._0_1_ = 0x1b;
        FUN_100469a0(puVar9 + 10,s_efw_PickupPliers_1007ad28,&uStack_4d);
        uStack_4._0_1_ = 0x1c;
        FUN_100083d0(puVar9 + 0xe,&local_40);
        *puVar9 = &PTR_FUN_10064f48;
        puStack_48 = puVar9;
      }
      goto LAB_100457dc;
    }
    iVar8 = FUN_10044f30(0x11);
    puVar9 = DAT_100bc330;
    piVar5 = piStack_18;
    if (iVar8 == 0) goto joined_r0x100457f5;
    puVar9 = operator_new(0x44);
    puStack_44 = puVar9;
    if (puVar9 == (undefined4 *)0x0) {
      puStack_48 = (undefined4 *)0x0;
    }
    else {
      uVar10 = *(undefined4 *)(iVar8 + 0xbc);
      uVar2 = *(undefined4 *)(iVar8 + 0x9c);
      puVar9[1] = 0;
      puVar9[2] = 0;
      puVar9[3] = uVar10;
      *puVar9 = &PTR_LAB_10064f60;
      puVar9[4] = param_1;
      uStack_4._0_1_ = 0x16;
      puVar9[5] = uVar2;
      FUN_100469a0(puVar9 + 6,s_Force_open_cage_door_with__i_1007ad78,(undefined1 *)&param_1);
      uStack_4._0_1_ = 0x17;
      FUN_100469a0(puVar9 + 10,s_efw_UseWithMarker__d__c_1007ae08,&uStack_4d);
      uStack_4 = CONCAT31(uStack_4._1_3_,0x18);
      FUN_100083d0(puVar9 + 0xe,&local_40);
      *puVar9 = &PTR_FUN_10064f48;
      puStack_48 = puVar9;
    }
    uStack_4 = uStack_4 >> 8;
  }
  else {
    if (iVar8 < 100) goto joined_r0x100457f5;
    iVar14 = FUN_10044f30(iVar8 + -100);
    puVar9 = operator_new(0x44);
    uStack_4._0_1_ = 0x1d;
    puStack_44 = puVar9;
    if (puVar9 == (undefined4 *)0x0) {
      puStack_48 = (undefined4 *)0x0;
    }
    else {
      if (iVar14 == 0) {
        uVar10 = (*DAT_100a4ff0)(s_sprites_efw_give_icon_spr_1007ad0c);
      }
      else {
        uVar10 = *(undefined4 *)(iVar14 + 0xbc);
      }
      FUN_10045a30(puVar9,uVar10);
      uStack_4._0_1_ = 0x1e;
      puVar9[4] = param_1;
      puVar9[5] = iVar8 + -100;
      FUN_100469a0(puVar9 + 6,s_Pick_up__i_1007ad00,(undefined1 *)&param_1);
      uStack_4._0_1_ = 0x1f;
      FUN_100469a0(puVar9 + 10,s_efw_Pickup__u_1007acf0,&uStack_4d);
      uStack_4._0_1_ = 0x20;
      FUN_100083d0(puVar9 + 0xe,&local_40);
      *puVar9 = &PTR_FUN_10064f48;
      puStack_48 = puVar9;
    }
LAB_100457dc:
    uStack_4 = (uint)uStack_4._1_3_;
  }
  uStack_4 = uStack_4 << 8;
  FUN_10046ab0(auStack_1c,piStack_14,&puStack_48);
  puVar9 = DAT_100bc330;
  piVar5 = piStack_18;
joined_r0x100457f5:
  for (; piVar7 = piStack_14, piVar6 = piStack_18, DAT_100bc330 = puVar9, piVar5 != piStack_14;
      piVar5 = piVar5 + 1) {
    if (piStack_18 == (int *)0x0) {
      iVar8 = 0;
    }
    else {
      iVar8 = (int)piStack_14 - (int)piStack_18 >> 2;
    }
    iVar14 = *piVar5;
    *(int *)(iVar14 + 4) = (int)piVar5 - (int)piStack_18 >> 2;
    *(int *)(iVar14 + 8) = iVar8;
    puVar9 = DAT_100bc330;
  }
  uVar19 = (int)piStack_14 - (int)piStack_18 >> 2;
  param_1 = piStack_14;
  if ((uint)((int)DAT_100bc334 - (int)puVar9 >> 2) < uVar19) {
    uVar15 = FUN_10046a90(0x100bc328);
    uVar16 = uVar19;
    if (uVar19 < uVar15) {
      uVar16 = FUN_10046a90(0x100bc328);
    }
    iVar8 = FUN_10046a90(0x100bc328);
    puStack_48 = (undefined4 *)(iVar8 + uVar16);
    puVar11 = puStack_48;
    if ((int)puStack_48 < 0) {
      puVar11 = (undefined4 *)0x0;
    }
    puVar11 = operator_new((int)puVar11 << 2);
    puVar17 = FUN_10046c50(DAT_100bc32c,puVar9,puVar11);
    puVar17 = FUN_10046c50(piVar6,param_1,puVar17);
    FUN_10046c50(puVar9,DAT_100bc330,puVar17);
    FUN_10001ad0();
    FUN_10049a73((undefined *)DAT_100bc32c);
    DAT_100bc334 = puVar11 + (int)puStack_48;
    iVar8 = FUN_10046a90(0x100bc328);
    DAT_100bc330 = puVar11 + iVar8 + uVar19;
    DAT_100bc32c = puVar11;
  }
  else if (uVar19 != 0) {
    FUN_10046c50(puVar9,puVar9,puVar9 + uVar19);
    FUN_10046c50(piVar6 + ((int)DAT_100bc330 - (int)puVar9 >> 2),piVar7,DAT_100bc330);
    FUN_10047590(piVar6,piVar6 + ((int)DAT_100bc330 - (int)puVar9 >> 2),puVar9);
    DAT_100bc330 = DAT_100bc330 + uVar19;
  }
  if ((piStack_18 != (int *)0x0) && (((int)piStack_14 - (int)piStack_18 & 0xfffffffcU) != 0)) {
    FUN_100083d0(auStack_28,&local_40);
    FUN_10047030(&DAT_100bc340,DAT_100bc348,1,auStack_28);
  }
  uStack_4 = 0xffffffff;
  FUN_10001ad0();
  FUN_10049a73((undefined *)piStack_18);
  ExceptionList = pvStack_c;
  return;
}



/* ==== 10047720 FUN_10047720 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_10047720(void)

{
  char cVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  int *piVar5;
  char *pcVar6;
  char *pcVar7;
  char acStack_100 [256];
  
  (*DAT_100a5038)(s_EFW_Menu_1007b78c,&LAB_10047680);
  _DAT_100bc9f4 = 0;
  piVar5 = &DAT_100bc884;
  do {
    FUN_100499f0(acStack_100,(byte *)s_efw_ShowMenu__i_1007b77c);
    uVar3 = 0xffffffff;
    pcVar6 = acStack_100;
    do {
      if (uVar3 == 0) break;
      uVar3 = uVar3 - 1;
      cVar1 = *pcVar6;
      pcVar6 = pcVar6 + 1;
    } while (cVar1 != '\0');
    uVar3 = ~uVar3 - 1;
    if (0xfffffffd < uVar3) {
      FUN_10057251();
    }
    iVar2 = *piVar5;
    if (((iVar2 == 0) || (cVar1 = *(char *)(iVar2 + -1), cVar1 == '\0')) || (cVar1 == -1)) {
      if (uVar3 != 0) {
        if ((0x1f < (uint)piVar5[2]) || ((uint)piVar5[2] < uVar3)) {
          FUN_100424b0(piVar5 + -1,'\x01');
          goto LAB_100477de;
        }
        goto LAB_100477e3;
      }
      FUN_100424b0(piVar5 + -1,'\x01');
    }
    else if (uVar3 == 0) {
      *(char *)(iVar2 + -1) = cVar1 + -1;
      *piVar5 = 0;
      piVar5[1] = 0;
      piVar5[2] = 0;
    }
    else {
LAB_100477de:
      FUN_10042ff0(uVar3);
LAB_100477e3:
      pcVar6 = acStack_100;
      pcVar7 = (char *)*piVar5;
      for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
        *(undefined4 *)pcVar7 = *(undefined4 *)pcVar6;
        pcVar6 = pcVar6 + 4;
        pcVar7 = pcVar7 + 4;
      }
      for (uVar4 = uVar3 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        *pcVar7 = *pcVar6;
        pcVar6 = pcVar6 + 1;
        pcVar7 = pcVar7 + 1;
      }
      piVar5[1] = uVar3;
      *(undefined1 *)(*piVar5 + uVar3) = 0;
    }
    piVar5 = piVar5 + 4;
    if (0x100bc9f3 < (int)piVar5) {
      return;
    }
  } while( true );
}



/* ==== 10047830 FUN_10047830 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

Panel * __cdecl FUN_10047830(Panel *param_1,undefined4 param_2)

{
  char cVar1;
  bool bVar2;
  bool bVar3;
  bool bVar4;
  bool bVar5;
  bool bVar6;
  bool bVar7;
  Panel *pPVar8;
  undefined4 uVar9;
  InputSignal *pIVar10;
  void *this;
  int iVar11;
  uint uVar12;
  uint uVar13;
  bool bVar14;
  char *pcVar15;
  undefined4 *puVar16;
  float10 fVar17;
  undefined1 local_40 [4];
  char *local_3c;
  Panel local_30 [16];
  undefined4 local_20 [4];
  void *pvStack_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_10058649;
  local_c = ExceptionList;
  bVar7 = false;
  bVar6 = false;
  bVar5 = false;
  bVar4 = false;
  bVar3 = false;
  bVar2 = false;
  bVar14 = false;
  if ((int)param_1 < 0x3c) {
    return (Panel *)0x0;
  }
  pcVar15 = s_Escape_From_Woomera_1007ba28;
  puVar16 = local_20;
  for (iVar11 = 5; iVar11 != 0; iVar11 = iVar11 + -1) {
    *puVar16 = *(undefined4 *)pcVar15;
    pcVar15 = pcVar15 + 4;
    puVar16 = puVar16 + 1;
  }
  pcVar15 = s_<error>_1007ba20;
  if (param_1 + -0x3c < (Panel *)0x16) {
    pcVar15 = *(char **)(s_efw_UseWithMarker__d__c_1007ae08 + (int)param_1 * 4 + 0x14);
  }
  if (param_1 == (Panel *)0x3f) {
    ExceptionList = &local_c;
    pPVar8 = operator_new(0xe8);
    local_4 = 0;
    param_1 = pPVar8;
    if (pPVar8 == (Panel *)0x0) goto LAB_1004793b;
    FUN_10048a70(pPVar8,0,0,0,DAT_100a4dcc,DAT_100a4dd0);
    *(undefined ***)pPVar8 = &PTR_setPos_100651dc;
    FUN_10048ad0(pPVar8 + 0xd8,&DAT_100bc9b0);
    local_4 = CONCAT31(local_4._1_3_,1);
    *(undefined ***)pPVar8 = &PTR_setPos_10064fcc;
    pcVar15 = s_Storyboard_EFW_Storyboards_Hidin_1007b9f4;
  }
  else {
    if (param_1 == (Panel *)0x4f) {
      ExceptionList = &local_c;
      pPVar8 = operator_new(0xe8);
      local_4 = 2;
      param_1 = pPVar8;
      if (pPVar8 == (Panel *)0x0) {
        pPVar8 = (Panel *)0x0;
      }
      else {
        param_1._0_1_ = SUB41(pPVar8,0);
        local_40[0] = param_1._0_1_;
        FUN_100424b0(local_40,'\0');
        uVar12 = 0xffffffff;
        pcVar15 = s_efw_changelevel_efw_prototype_le_1007b9cc;
        do {
          if (uVar12 == 0) break;
          uVar12 = uVar12 - 1;
          cVar1 = *pcVar15;
          pcVar15 = pcVar15 + 1;
        } while (cVar1 != '\0');
        uVar12 = ~uVar12 - 1;
        uVar9 = FUN_100429c0(local_40,uVar12,'\x01');
        if ((char)uVar9 != '\0') {
          pcVar15 = s_efw_changelevel_efw_prototype_le_1007b9cc;
          for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
            *(undefined4 *)local_3c = *(undefined4 *)pcVar15;
            pcVar15 = pcVar15 + 4;
            local_3c = local_3c + 4;
          }
          for (uVar13 = uVar12 & 3; uVar13 != 0; uVar13 = uVar13 - 1) {
            *local_3c = *pcVar15;
            pcVar15 = pcVar15 + 1;
            local_3c = local_3c + 1;
          }
          FUN_100429a0(local_40,uVar12);
        }
        bVar14 = true;
        local_4 = CONCAT31(local_4._1_3_,3);
        FUN_10048a70(pPVar8,0,0,0,DAT_100a4dcc,DAT_100a4dd0);
        *(undefined ***)pPVar8 = &PTR_setPos_100651dc;
        FUN_10048ad0(pPVar8 + 0xd8,local_40);
        local_4 = 4;
        *(undefined ***)pPVar8 = &PTR_setPos_10064fcc;
        uVar9 = (*DAT_100a4ff0)(s_Storyboard_EFW_Storyboards_Decoy_1007b9a0);
        *(undefined4 *)(pPVar8 + 0xd4) = uVar9;
        pIVar10 = operator_new(4);
        if (pIVar10 == (InputSignal *)0x0) {
          vgui::Panel::addInputSignal(pPVar8,(InputSignal *)0x0);
        }
        else {
          *(undefined ***)pIVar10 = &PTR_LAB_10064fa0;
          vgui::Panel::addInputSignal(pPVar8,pIVar10);
        }
      }
      local_4 = 0xffffffff;
      if (bVar14) {
        FUN_100424b0(local_40,'\x01');
      }
      goto LAB_1004838c;
    }
    if (param_1 == (Panel *)0x43) {
      ExceptionList = &local_c;
      pPVar8 = operator_new(0xe8);
      local_4 = 6;
      param_1 = pPVar8;
      if (pPVar8 == (Panel *)0x0) goto LAB_1004793b;
      FUN_10048a70(pPVar8,0,0,0,DAT_100a4dcc,DAT_100a4dd0);
      *(undefined ***)pPVar8 = &PTR_setPos_100651dc;
      FUN_10048ad0(pPVar8 + 0xd8,&DAT_100bc9c0);
      local_4 = CONCAT31(local_4._1_3_,7);
      *(undefined ***)pPVar8 = &PTR_setPos_10064fcc;
      pcVar15 = s_Storyboard_EFW_Storyboards_Hidin_1007b974;
    }
    else {
      if (param_1 == (Panel *)0x50) {
        ExceptionList = &local_c;
        pPVar8 = operator_new(0xe8);
        local_4 = 8;
        param_1 = pPVar8;
        if (pPVar8 == (Panel *)0x0) {
          pPVar8 = (Panel *)0x0;
        }
        else {
          param_1._0_1_ = SUB41(pPVar8,0);
          local_40[0] = param_1._0_1_;
          FUN_100424b0(local_40,'\0');
          uVar12 = 0xffffffff;
          pcVar15 = s_efw_changelevel_efw_prototype_le_1007b94c;
          do {
            if (uVar12 == 0) break;
            uVar12 = uVar12 - 1;
            cVar1 = *pcVar15;
            pcVar15 = pcVar15 + 1;
          } while (cVar1 != '\0');
          uVar12 = ~uVar12 - 1;
          uVar9 = FUN_100429c0(local_40,uVar12,'\x01');
          if ((char)uVar9 != '\0') {
            pcVar15 = s_efw_changelevel_efw_prototype_le_1007b94c;
            for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
              *(undefined4 *)local_3c = *(undefined4 *)pcVar15;
              pcVar15 = pcVar15 + 4;
              local_3c = local_3c + 4;
            }
            for (uVar13 = uVar12 & 3; uVar13 != 0; uVar13 = uVar13 - 1) {
              *local_3c = *pcVar15;
              pcVar15 = pcVar15 + 1;
              local_3c = local_3c + 1;
            }
            FUN_100429a0(local_40,uVar12);
          }
          bVar2 = true;
          local_4 = CONCAT31(local_4._1_3_,9);
          FUN_10048a70(pPVar8,0,0,0,DAT_100a4dcc,DAT_100a4dd0);
          *(undefined ***)pPVar8 = &PTR_setPos_100651dc;
          FUN_10048ad0(pPVar8 + 0xd8,local_40);
          local_4 = 10;
          *(undefined ***)pPVar8 = &PTR_setPos_10064fcc;
          uVar9 = (*DAT_100a4ff0)(s_Storyboard_EFW_Storyboards_Dec_R_1007b920);
          *(undefined4 *)(pPVar8 + 0xd4) = uVar9;
          pIVar10 = operator_new(4);
          if (pIVar10 == (InputSignal *)0x0) {
            vgui::Panel::addInputSignal(pPVar8,(InputSignal *)0x0);
          }
          else {
            *(undefined ***)pIVar10 = &PTR_LAB_10064fa0;
            vgui::Panel::addInputSignal(pPVar8,pIVar10);
          }
        }
        local_4 = 0xffffffff;
        if (bVar2) {
          FUN_100424b0(local_40,'\x01');
        }
        goto LAB_1004838c;
      }
      if (param_1 == (Panel *)0x52) {
        ExceptionList = &local_c;
        pPVar8 = operator_new(0xe8);
        local_4 = 0xc;
        param_1 = pPVar8;
        if (pPVar8 == (Panel *)0x0) {
          pPVar8 = (Panel *)0x0;
        }
        else {
          param_1._0_1_ = SUB41(pPVar8,0);
          local_40[0] = param_1._0_1_;
          FUN_100424b0(local_40,'\0');
          uVar12 = 0xffffffff;
          pcVar15 = s_;
          do {
            if (uVar12 == 0) break;
            uVar12 = uVar12 - 1;
            cVar1 = *pcVar15;
            pcVar15 = pcVar15 + 1;
          } while (cVar1 != '\0');
          uVar12 = ~uVar12 - 1;
          uVar9 = FUN_100429c0(local_40,uVar12,'\x01');
          if ((char)uVar9 != '\0') {
            pcVar15 = s_;
            for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
              *(undefined4 *)local_3c = *(undefined4 *)pcVar15;
              pcVar15 = pcVar15 + 4;
              local_3c = local_3c + 4;
            }
            for (uVar13 = uVar12 & 3; uVar13 != 0; uVar13 = uVar13 - 1) {
              *local_3c = *pcVar15;
              pcVar15 = pcVar15 + 1;
              local_3c = local_3c + 1;
            }
            FUN_100429a0(local_40,uVar12);
          }
          bVar3 = true;
          local_4 = CONCAT31(local_4._1_3_,0xd);
          FUN_10048a70(pPVar8,0,0,0,DAT_100a4dcc,DAT_100a4dd0);
          *(undefined ***)pPVar8 = &PTR_setPos_100651dc;
          FUN_10048ad0(pPVar8 + 0xd8,local_40);
          local_4 = 0xe;
          *(undefined ***)pPVar8 = &PTR_setPos_10064fcc;
          uVar9 = (*DAT_100a4ff0)(s_Storyboard_EFW_Storyboards_Help__1007b8f4);
          *(undefined4 *)(pPVar8 + 0xd4) = uVar9;
          pIVar10 = operator_new(4);
          if (pIVar10 == (InputSignal *)0x0) {
            vgui::Panel::addInputSignal(pPVar8,(InputSignal *)0x0);
          }
          else {
            *(undefined ***)pIVar10 = &PTR_LAB_10064fa0;
            vgui::Panel::addInputSignal(pPVar8,pIVar10);
          }
        }
        local_4 = 0xffffffff;
        if (bVar3) {
          FUN_100424b0(local_40,'\x01');
        }
        goto LAB_1004838c;
      }
      if (param_1 == (Panel *)0x51) {
        ExceptionList = &local_c;
        pPVar8 = operator_new(0xe8);
        local_4 = 0x10;
        param_1 = pPVar8;
        if (pPVar8 == (Panel *)0x0) {
          pPVar8 = (Panel *)0x0;
        }
        else {
          param_1._0_1_ = SUB41(pPVar8,0);
          local_40[0] = param_1._0_1_;
          FUN_100424b0(local_40,'\0');
          uVar12 = 0xffffffff;
          pcVar15 = s_;
          do {
            if (uVar12 == 0) break;
            uVar12 = uVar12 - 1;
            cVar1 = *pcVar15;
            pcVar15 = pcVar15 + 1;
          } while (cVar1 != '\0');
          FUN_10042900(local_40,(undefined4 *)s_,~uVar12 - 1);
          bVar4 = true;
          local_4 = CONCAT31(local_4._1_3_,0x11);
          FUN_10048a70(pPVar8,0,0,0,DAT_100a4dcc,DAT_100a4dd0);
          *(undefined ***)pPVar8 = &PTR_setPos_100651dc;
          FUN_10048ad0(pPVar8 + 0xd8,local_40);
          local_4 = 0x12;
          *(undefined ***)pPVar8 = &PTR_setPos_10064fcc;
          uVar9 = (*DAT_100a4ff0)(s_Storyboard_EFW_Storyboards_Isola_1007b8c8);
          *(undefined4 *)(pPVar8 + 0xd4) = uVar9;
          pIVar10 = operator_new(4);
          if (pIVar10 == (InputSignal *)0x0) {
            vgui::Panel::addInputSignal(pPVar8,(InputSignal *)0x0);
          }
          else {
            *(undefined ***)pIVar10 = &PTR_LAB_10064fa0;
            vgui::Panel::addInputSignal(pPVar8,pIVar10);
          }
        }
        local_4 = 0xffffffff;
        if (bVar4) {
          FUN_100424b0(local_40,'\x01');
        }
        goto LAB_1004838c;
      }
      if (param_1 == (Panel *)0x46) {
        ExceptionList = &local_c;
        pPVar8 = operator_new(0xe8);
        local_4 = 0x14;
        param_1 = pPVar8;
        if (pPVar8 == (Panel *)0x0) {
          pPVar8 = (Panel *)0x0;
        }
        else {
          param_1._0_1_ = SUB41(pPVar8,0);
          local_40[0] = param_1._0_1_;
          FUN_100424b0(local_40,'\0');
          uVar12 = 0xffffffff;
          pcVar15 = s_efw_changelevel_efw_prototype_le_1007b9cc;
          do {
            if (uVar12 == 0) break;
            uVar12 = uVar12 - 1;
            cVar1 = *pcVar15;
            pcVar15 = pcVar15 + 1;
          } while (cVar1 != '\0');
          FUN_10042900(local_40,(undefined4 *)s_efw_changelevel_efw_prototype_le_1007b9cc,
                       ~uVar12 - 1);
          bVar5 = true;
          local_4 = CONCAT31(local_4._1_3_,0x15);
          FUN_100483d0(pPVar8,0,0,0,DAT_100a4dcc,DAT_100a4dd0);
          FUN_10048ad0(pPVar8 + 0xd8,local_40);
          local_4 = 0x16;
          *(undefined ***)pPVar8 = &PTR_setPos_10064fcc;
          uVar9 = (*DAT_100a4ff0)(s_Storyboard_EFW_Storyboards_Isola_1007b8c8);
          *(undefined4 *)(pPVar8 + 0xd4) = uVar9;
          pIVar10 = operator_new(4);
          if (pIVar10 == (InputSignal *)0x0) {
            vgui::Panel::addInputSignal(pPVar8,(InputSignal *)0x0);
          }
          else {
            *(undefined ***)pIVar10 = &PTR_LAB_10064fa0;
            vgui::Panel::addInputSignal(pPVar8,pIVar10);
          }
        }
        local_4 = 0xffffffff;
        if (bVar5) {
          FUN_100424b0(local_40,'\x01');
        }
        goto LAB_1004838c;
      }
      if (param_1 == (Panel *)0x49) {
        ExceptionList = &local_c;
        pPVar8 = operator_new(0xe8);
        local_4 = 0x18;
        param_1 = pPVar8;
        if (pPVar8 == (Panel *)0x0) {
LAB_1004793b:
          pPVar8 = (Panel *)0x0;
          local_4 = 0xffffffff;
          goto LAB_1004838c;
        }
        FUN_100483d0(pPVar8,0,0,0,DAT_100a4dcc,DAT_100a4dd0);
        FUN_10048ad0(pPVar8 + 0xd8,&DAT_100bc960);
        local_4 = CONCAT31(local_4._1_3_,0x19);
        *(undefined ***)pPVar8 = &PTR_setPos_10064fcc;
        pcVar15 = s_Storyboard_EFW_Storyboards_Intro_1007b898;
      }
      else {
        if (param_1 != (Panel *)0x4a) {
          if (param_1 == (Panel *)0x4b) {
            ExceptionList = &local_c;
            pPVar8 = operator_new(0xe8);
            local_4 = 0x1c;
            param_1 = pPVar8;
            if (pPVar8 == (Panel *)0x0) {
              pPVar8 = (Panel *)0x0;
            }
            else {
              param_1._0_1_ = SUB41(pPVar8,0);
              local_40[0] = param_1._0_1_;
              FUN_100424b0(local_40,'\0');
              uVar12 = 0xffffffff;
              pcVar15 = s_;
              do {
                if (uVar12 == 0) break;
                uVar12 = uVar12 - 1;
                cVar1 = *pcVar15;
                pcVar15 = pcVar15 + 1;
              } while (cVar1 != '\0');
              FUN_10042900(local_40,(undefined4 *)s_,~uVar12 - 1);
              bVar6 = true;
              local_4 = CONCAT31(local_4._1_3_,0x1d);
              FUN_100483d0(pPVar8,0,0,0,DAT_100a4dcc,DAT_100a4dd0);
              FUN_10048ad0(pPVar8 + 0xd8,local_40);
              local_4 = 0x1e;
              *(undefined ***)pPVar8 = &PTR_setPos_10064fcc;
              uVar9 = (*DAT_100a4ff0)(s_Storyboard_EFW_Storyboards_Intro_1007b838);
              *(undefined4 *)(pPVar8 + 0xd4) = uVar9;
              pIVar10 = operator_new(4);
              if (pIVar10 == (InputSignal *)0x0) {
                vgui::Panel::addInputSignal(pPVar8,(InputSignal *)0x0);
              }
              else {
                *(undefined ***)pIVar10 = &PTR_LAB_10064fa0;
                vgui::Panel::addInputSignal(pPVar8,pIVar10);
              }
            }
            local_4 = 0xffffffff;
            if (bVar6) {
              FUN_100424b0(local_40,'\x01');
            }
            goto LAB_1004838c;
          }
          if (param_1 == (Panel *)0x4c) {
            ExceptionList = &local_c;
            pPVar8 = operator_new(0xe8);
            local_4 = 0x20;
            param_1 = pPVar8;
            if (pPVar8 == (Panel *)0x0) {
              pPVar8 = (Panel *)0x0;
            }
            else {
              param_1._0_1_ = SUB41(pPVar8,0);
              local_40[0] = param_1._0_1_;
              FUN_100424b0(local_40,'\0');
              uVar12 = 0xffffffff;
              pcVar15 = s_efw_changelevel_efw_prototype_le_1007b810;
              do {
                if (uVar12 == 0) break;
                uVar12 = uVar12 - 1;
                cVar1 = *pcVar15;
                pcVar15 = pcVar15 + 1;
              } while (cVar1 != '\0');
              FUN_10042900(local_40,(undefined4 *)s_efw_changelevel_efw_prototype_le_1007b810,
                           ~uVar12 - 1);
              bVar7 = true;
              local_4 = CONCAT31(local_4._1_3_,0x21);
              FUN_100483d0(pPVar8,0,0,0,DAT_100a4dcc,DAT_100a4dd0);
              FUN_10048ad0(pPVar8 + 0xd8,local_40);
              local_4 = 0x22;
              *(undefined ***)pPVar8 = &PTR_setPos_10064fcc;
              uVar9 = (*DAT_100a4ff0)(s_Storyboard_EFW_Storyboards_Endin_1007b7e0);
              *(undefined4 *)(pPVar8 + 0xd4) = uVar9;
              pIVar10 = operator_new(4);
              if (pIVar10 == (InputSignal *)0x0) {
                vgui::Panel::addInputSignal(pPVar8,(InputSignal *)0x0);
              }
              else {
                *(undefined ***)pIVar10 = &PTR_LAB_10064fa0;
                vgui::Panel::addInputSignal(pPVar8,pIVar10);
              }
            }
            local_4 = 0xffffffff;
            if (bVar7) {
              FUN_100424b0(local_40,'\x01');
            }
            goto LAB_1004838c;
          }
          if (param_1 == (Panel *)0x4d) {
            ExceptionList = &local_c;
            param_1 = operator_new(0xe8);
            local_4 = 0x24;
            if (param_1 == (Panel *)0x0) goto LAB_10048380;
            pPVar8 = FUN_100484b0(param_1,s_Storyboard_EFW_Sb_Ending_Iso_spr_1007b7bc,
                                  (Panel *)&DAT_100bc9a0);
            goto LAB_10048382;
          }
          if (param_1 == (Panel *)0x4e) {
            ExceptionList = &local_c;
            this = operator_new(0xe8);
            local_4 = 0x25;
            if (this == (void *)0x0) {
              pPVar8 = (Panel *)0x0;
            }
            else {
              FUN_100469a0(local_30,s_efw_changelevel_efw_prototype_le_1007b810,
                           (undefined1 *)&param_1);
              local_4 = CONCAT31(local_4._1_3_,0x26);
              pPVar8 = FUN_100484b0(this,s_Storyboard_EFW_Sb_Ending_Dep_spr_1007b798,local_30);
            }
            local_4 = 0xffffffff;
            if (this != (void *)0x0) {
              FUN_100424b0(local_30,'\x01');
            }
            goto LAB_1004838c;
          }
          if (param_1 == (Panel *)0x48) {
            ExceptionList = &local_c;
            fVar17 = FUN_10044870();
            if ((fVar17 - (float10)_DAT_100bc9f4 <= (float10)0.4) &&
               ((float10)0.0 <= fVar17 - (float10)_DAT_100bc9f4)) {
              ExceptionList = local_c;
              return (Panel *)0x0;
            }
            param_1 = operator_new(0xd4);
            local_4 = 0x28;
            if (param_1 == (Panel *)0x0) {
LAB_10048380:
              pPVar8 = (Panel *)0x0;
            }
            else {
              pPVar8 = FUN_10048650(param_1);
            }
          }
          else {
            ExceptionList = &local_c;
            param_1 = operator_new(0xe4);
            local_4 = 0x29;
            if (param_1 == (Panel *)0x0) goto LAB_10048380;
            pPVar8 = FUN_10048790(param_1,pcVar15,local_20,1,0,0,DAT_100a4dcc,DAT_100a4dd0,0,0);
          }
LAB_10048382:
          local_4 = 0xffffffff;
          goto LAB_1004838c;
        }
        ExceptionList = &local_c;
        pPVar8 = operator_new(0xe8);
        local_4 = 0x1a;
        param_1 = pPVar8;
        if (pPVar8 == (Panel *)0x0) goto LAB_1004793b;
        FUN_100483d0(pPVar8,0,0,0,DAT_100a4dcc,DAT_100a4dd0);
        FUN_10048ad0(pPVar8 + 0xd8,&DAT_100bc970);
        local_4 = CONCAT31(local_4._1_3_,0x1b);
        *(undefined ***)pPVar8 = &PTR_setPos_10064fcc;
        pcVar15 = s_Storyboard_EFW_Storyboards_Intro_1007b868;
      }
    }
  }
  uVar9 = (*DAT_100a4ff0)(pcVar15);
  *(undefined4 *)(pPVar8 + 0xd4) = uVar9;
  pIVar10 = operator_new(4);
  if (pIVar10 == (InputSignal *)0x0) {
    vgui::Panel::addInputSignal(pPVar8,(InputSignal *)0x0);
    local_4 = 0xffffffff;
  }
  else {
    *(undefined ***)pIVar10 = &PTR_LAB_10064fa0;
    vgui::Panel::addInputSignal(pPVar8,pIVar10);
    local_4 = 0xffffffff;
  }
LAB_1004838c:
  if (pPVar8 == (Panel *)0x0) {
    ExceptionList = local_c;
    return (Panel *)0x0;
  }
  (**(code **)(*(int *)pPVar8 + 0x40))(param_2);
  ExceptionList = pvStack_10;
  return pPVar8;
}



/* ==== 10048430 FUN_10048430 ==== */

void __fastcall FUN_10048430(int *param_1)

{
  (**(code **)(*param_1 + 0x24))(1);
  param_1[0x34] = DAT_100a3a64;
  (*DAT_100a503c)(s_efw_pause_1_1007ba3c);
  return;
}



/* ==== 10048460 FUN_10048460 ==== */

void __fastcall FUN_10048460(int *param_1)

{
  (*DAT_100a503c)(s_efw_pause_0_1007ba48);
  (**(code **)(*param_1 + 0x24))(0);
  param_1[0x33] = 0;
  if (param_1[0x32] != 0) {
    (**(code **)(*DAT_100a5178 + 0x4c))(param_1);
  }
  return;
}



/* ==== 10048590 FUN_10048590 ==== */

void __fastcall FUN_10048590(int *param_1)

{
  (**(code **)(*param_1 + 0x24))(1);
  param_1[0x34] = DAT_100a3a64;
  (*DAT_100a503c)(s_efw_pause_1_1007ba3c);
  DAT_1007ab5c = param_1[0x35];
  DAT_100baf10 = 0;
  return;
}



/* ==== 100485d0 FUN_100485d0 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_100485d0(int *param_1)

{
  undefined4 *puVar1;
  float10 fVar2;
  
  (*DAT_100a503c)(s_efw_pause_0_1007ba48);
  (**(code **)(*param_1 + 0x24))(0);
  param_1[0x33] = 0;
  if (param_1[0x32] != 0) {
    (**(code **)(*DAT_100a5178 + 0x4c))(param_1);
  }
  _DAT_1007ab60 = DAT_1007ab5c;
  DAT_1007ab5c = 0xffffffff;
  fVar2 = FUN_10044870();
  _DAT_100bc9f4 = (float)fVar2;
  puVar1 = (undefined4 *)param_1[0x37];
  if (puVar1 == (undefined4 *)0x0) {
    puVar1 = &DAT_10064d30;
  }
  (*DAT_100a503c)(puVar1);
  return;
}



/* ==== 10048710 FUN_10048710 ==== */

void __fastcall FUN_10048710(int *param_1)

{
  (**(code **)(*param_1 + 0x24))(1);
  param_1[0x34] = DAT_100a3a64;
  (*DAT_100a503c)(s_efw_pause_1_1007ba3c);
  FUN_10046370();
  return;
}



/* ==== 10048740 FUN_10048740 ==== */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_10048740(int *param_1)

{
  float10 fVar1;
  
  (*DAT_100a503c)(s_efw_pause_0_1007ba48);
  (**(code **)(*param_1 + 0x24))(0);
  param_1[0x33] = 0;
  if (param_1[0x32] != 0) {
    (**(code **)(*DAT_100a5178 + 0x4c))(param_1);
  }
  FUN_100463c0();
  fVar1 = FUN_10044870();
  _DAT_100bc9f4 = (float)fVar1;
  return;
}



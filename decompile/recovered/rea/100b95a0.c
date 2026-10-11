/* REA procedure_pseudo_code 0x100b95a0 */

void __cdecl FUN_100b95a0(uint param_1,int *param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 *puVar2;
  undefined1 *puVar3;
  undefined *puVar4;
  char cVar5;
  undefined4 uVar6;
  int *piVar7;
  int *piVar8;
  uint uVar9;
  char *pcVar10;
  undefined1 *puVar11;
  undefined4 *puVar12;
  uint uVar13;
  undefined4 in_stack_ffffffd0;
  int in_stack_ffffffd4;
  int local_14;
  undefined1 local_10 [4];
  undefined *local_c;
  undefined4 *local_8;
  undefined4 local_4;
  
  puVar3 = DAT_10132400;
  local_14 = *(int *)(*(int *)(param_1 + 4) + 0x1cc) + *(int *)(DAT_10122044 + 0x98);
  puVar11 = DAT_101323fc;
  while ((puVar11 != puVar3 && (uVar6 = FUN_100b9770(&local_14,puVar11), (char)uVar6 == '\0'))) {
    puVar11 = puVar11 + 0x18;
  }
  if ((puVar11 != (undefined1 *)0x0) && (puVar11 != DAT_10132400)) {
    iVar1 = *(int *)(puVar11 + 0x14);
    FUN_100ba310(&stack0xffffffd0,(char *)param_2,(undefined1 *)&param_2);
    piVar7 = (int *)FUN_100ba7e0(iVar1);
    piVar8 = (int *)FUN_100ba7d0(iVar1);
    piVar7 = FUN_100bdc80(piVar8,piVar7,in_stack_ffffffd0,in_stack_ffffffd4);
    if ((piVar7 != (int *)0x0) && (piVar7 != *(int **)(iVar1 + 8))) {
      piVar7 = (int *)*piVar7;
      local_10[0] = (undefined1)param_1;
      local_c = (undefined *)0x0;
      local_8 = (undefined4 *)0x0;
      local_4 = 0;
      puVar2 = *(undefined4 **)(*piVar7 + 8);
      puVar12 = *(undefined4 **)(*piVar7 + 4);
      param_2 = piVar7;
      if (puVar12 != puVar2) {
        do {
          cVar5 = FUN_100b9720(&param_2,(int *)*puVar12);
          if (cVar5 != '\0') {
            FUN_100be6b0(local_10,local_8,1,puVar12);
          }
          puVar4 = local_c;
          puVar12 = puVar12 + 1;
        } while (puVar12 != puVar2);
        if ((local_c != (undefined *)0x0) &&
           (uVar13 = (int)local_8 - (int)local_c >> 2, uVar13 != 0)) {
          uVar9 = _rand();
          iVar1 = *(int *)(puVar4 + (uVar9 % uVar13) * 4);
          if (iVar1 != 0) {
            *(undefined1 *)(piVar7 + 2) = 1;
            pcVar10 = *(char **)(iVar1 + 0xc);
            if (pcVar10 == (char *)0x0) {
              pcVar10 = "";
            }
            FUN_100ba080(param_1,pcVar10,(byte)param_3);
            FUN_100c8ce6(local_c);
            return;
          }
        }
      }
      FUN_100c8ce6(local_c);
    }
  }
  FUN_100ba080(param_1,s_Thanks__but_I_don_t_need_it__1011c3fc,(byte)param_3);
  return;
}



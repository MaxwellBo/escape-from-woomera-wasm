/* REA procedure_pseudo_code 0x100b9990 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __cdecl FUN_100b9990(uint param_1)

{
  int iVar1;
  undefined1 *puVar2;
  char cVar3;
  undefined4 uVar4;
  int *piVar5;
  int iVar6;
  undefined4 *puVar7;
  float *pfVar8;
  char *pcVar9;
  int iVar10;
  char *pcVar11;
  char *pcVar12;
  char *pcVar13;
  char *pcVar14;
  undefined1 *puVar15;
  int *piVar16;
  undefined4 *puVar17;
  char *pcVar18;
  char *pcVar19;
  float10 fVar20;
  int *local_4c;
  int local_48;
  int local_44;
  uint local_40;
  undefined4 local_3c;
  uint local_38 [2];
  int local_30 [10];
  undefined4 local_8;
  undefined4 local_4;
  
  puVar2 = DAT_10132400;
  local_44 = *(int *)(*(int *)(param_1 + 4) + 0x1cc) + *(int *)(DAT_10122044 + 0x98);
  puVar15 = DAT_101323fc;
  while ((puVar15 != puVar2 && (uVar4 = FUN_100b9770(&local_44,puVar15), (char)uVar4 == '\0'))) {
    puVar15 = puVar15 + 0x18;
  }
  if ((puVar15 == (undefined1 *)0x0) || (puVar15 == DAT_10132400)) {
    return (uint)DAT_10132400 & 0xffffff00;
  }
  local_3c = 0;
  piVar5 = local_30;
  for (iVar10 = 6; iVar10 != 0; iVar10 = iVar10 + -1) {
    *piVar5 = 0;
    piVar5 = piVar5 + 1;
  }
  piVar5 = local_30 + 6;
  for (iVar10 = 6; iVar10 != 0; iVar10 = iVar10 + -1) {
    *piVar5 = 0;
    piVar5 = piVar5 + 1;
  }
  iVar10 = *(int *)(puVar15 + 0x14);
  local_40 = param_1;
  local_44 = iVar10;
  piVar5 = (int *)FUN_100bb670(&DAT_10132408,local_38,&local_40);
  local_4c = *(int **)(iVar10 + 4);
  local_48 = 0;
  iVar1 = *(int *)(*piVar5 + 0x10);
  if ((iVar1 != 0) && (piVar5 = *(int **)(iVar10 + 8), piVar16 = local_4c, local_4c != piVar5)) {
    do {
      if (*piVar16 == iVar1) break;
      piVar16 = piVar16 + 1;
    } while (piVar16 != piVar5);
    if (piVar16 != piVar5) {
      if ((*(int *)(iVar1 + 4) == 0) || (iVar6 = FUN_100bad90(*(int *)(iVar1 + 4)), iVar6 == 0)) {
        local_4c = piVar16 + 1;
        local_48 = 1;
      }
      else {
        local_48 = *(int *)(**(int **)(*(int *)(iVar1 + 4) + 4) + 0x14) + 1;
        local_4c = piVar16 + 1;
      }
    }
  }
  if (local_4c != *(int **)(iVar10 + 8)) {
    iVar10 = 0;
    do {
      if (0x17 < iVar10) break;
      iVar1 = *local_4c;
      if ((*(int *)(iVar1 + 4) == 0) || (iVar6 = FUN_100bad90(*(int *)(iVar1 + 4)), iVar6 == 0)) {
        iVar6 = 0;
      }
      else {
        iVar6 = *(int *)(**(int **)(*(int *)(iVar1 + 4) + 4) + 0x14);
      }
      if (iVar6 < local_48) break;
      if ((*(int *)(iVar1 + 4) == 0) || (iVar6 = FUN_100bad90(*(int *)(iVar1 + 4)), iVar6 == 0)) {
        iVar6 = 0;
      }
      else {
        iVar6 = *(int *)(**(int **)(*(int *)(iVar1 + 4) + 4) + 0x14);
      }
      if ((iVar6 == local_48) && (iVar6 = *(int *)(iVar1 + 4), iVar6 != 0)) {
        puVar7 = *(undefined4 **)(iVar6 + 8);
        puVar17 = *(undefined4 **)(iVar6 + 4);
        while (((puVar17 != puVar7 && (iVar6 = *(int *)*puVar17, iVar6 != 0)) &&
               (cVar3 = FUN_100bdc40(*(undefined4 **)(iVar6 + 4),*(undefined4 **)(iVar6 + 8),'\x01',
                                     iVar1), cVar3 == '\0'))) {
          puVar17 = puVar17 + 1;
        }
        if (puVar17 != *(undefined4 **)(*(int *)(iVar1 + 4) + 8)) {
          *(undefined4 *)((int)local_30 + iVar10) = *puVar17;
          puVar7 = operator_new(0x14);
          if (puVar7 == (undefined4 *)0x0) {
            puVar7 = (undefined4 *)0x0;
          }
          else {
            *puVar7 = &DAT_100f705c;
            puVar7[2] = 0;
            puVar7[1] = 0;
            puVar7[3] = 0;
            puVar7[4] = 0;
          }
          *(undefined4 **)((int)local_30 + iVar10 + 0x18) = puVar7;
          puVar7[1] = param_1;
          puVar7[2] = *puVar17;
          puVar7[3] = iVar1;
          puVar7[4] = local_44;
          iVar10 = iVar10 + 4;
        }
      }
      local_4c = local_4c + 1;
    } while (local_4c != *(int **)(local_44 + 8));
  }
  pfVar8 = (float *)FUN_100b93f0(param_1);
  fVar20 = FUN_100c5b60();
  pcVar13 = &DAT_10121c38;
  if (fVar20 - (float10)*pfVar8 < (float10)_DAT_1011d128) {
    pcVar13 = "";
    if ((char *)pfVar8[2] != (char *)0x0) {
      pcVar13 = (char *)pfVar8[2];
    }
  }
  if (local_30[5] == 0) {
    pcVar14 = (char *)0x0;
  }
  else {
    pcVar14 = *(char **)(local_30[5] + 8);
    if (pcVar14 == (char *)0x0) {
      pcVar14 = "";
    }
  }
  if (local_30[4] == 0) {
    pcVar19 = (char *)0x0;
  }
  else {
    pcVar19 = *(char **)(local_30[4] + 8);
    if (pcVar19 == (char *)0x0) {
      pcVar19 = "";
    }
  }
  if (local_30[3] == 0) {
    pcVar18 = (char *)0x0;
  }
  else {
    pcVar18 = *(char **)(local_30[3] + 8);
    if (pcVar18 == (char *)0x0) {
      pcVar18 = "";
    }
  }
  if (local_30[2] == 0) {
    pcVar12 = (char *)0x0;
  }
  else {
    pcVar12 = *(char **)(local_30[2] + 8);
    if (pcVar12 == (char *)0x0) {
      pcVar12 = "";
    }
  }
  if (local_30[1] == 0) {
    pcVar11 = (char *)0x0;
  }
  else {
    pcVar11 = *(char **)(local_30[1] + 8);
    if (pcVar11 == (char *)0x0) {
      pcVar11 = "";
    }
  }
  if (local_30[0] == 0) {
    pcVar9 = (char *)0x0;
  }
  else {
    pcVar9 = *(char **)(local_30[0] + 8);
    if (pcVar9 == (char *)0x0) {
      pcVar9 = "";
    }
  }
  FUN_100c6e60(param_1,pcVar13,pcVar9,pcVar11,pcVar12,pcVar18,pcVar19,pcVar14,0x43480000);
  uVar4 = FUN_100c6d70(local_30[6],local_30[7],local_30[8],local_30[9],local_8,local_4);
  return CONCAT31((int3)((uint)uVar4 >> 8),1);
}



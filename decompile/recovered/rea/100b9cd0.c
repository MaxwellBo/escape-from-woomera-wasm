/* Ghidra createFunction 0x100b9cd0 — auto-analysis left no procedure,
   so REA procedure_pseudo_code rejected the address. Body ends at the
   ret before Squark (0x100ba015). */

void __fastcall FUN_100b9cd0(int param_1)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  char cVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  undefined *puVar10;
  int iVar11;
  int iVar12;
  int *unaff_EBP;
  int unaff_ESI;
  int *piVar13;
  int *piVar14;
  char *pcVar15;
  undefined4 *puVar16;
  uint uVar17;
  char *pcVar18;
  undefined4 *puStack_4c;
  undefined1 auStack_2c [4];
  int *piStack_28;
  undefined4 uStack_24;
  int iStack_1c;
  undefined4 *puStack_c;
  int iStack_8;
  undefined4 uStack_4;
  
  piStack_28 = *(int **)(param_1 + 0xc);
  if (piStack_28 == (int *)0x0) {
    return;
  }
  if (*(int *)(param_1 + 8) == 0) {
    return;
  }
  puStack_c = (undefined4 *)0x0;
  iStack_8 = 0;
  uStack_4 = 0;
  puVar1 = *(undefined4 **)(*piStack_28 + 8);
  puVar16 = *(undefined4 **)(*piStack_28 + 4);
  if (puVar16 == puVar1) goto LAB_100ba001;
  do {
    puStack_4c = (undefined4 *)*puVar16;
    cVar4 = func_0x100b9720();
    if (cVar4 != '\0') {
      puStack_4c = puVar16;
      func_0x100be4e0(iStack_8);
    }
    puVar3 = puStack_c;
    puVar16 = puVar16 + 1;
  } while (puVar16 != puVar1);
  if ((puStack_c == (undefined4 *)0x0) || (uVar17 = iStack_8 - (int)puStack_c >> 2, uVar17 == 0))
  goto LAB_100ba001;
  puStack_4c = (undefined4 *)0x100b9d5d;
  uVar5 = func_0x100c84e4();
  piVar9 = (int *)puVar3[uVar5 % uVar17];
  if (piVar9 == (int *)0x0) goto LAB_100ba001;
  iVar12 = *(int *)(param_1 + 0xc);
  uVar2 = *(undefined4 *)(param_1 + 8);
  piVar14 = *(int **)(param_1 + 4);
  puStack_4c = (undefined4 *)0x0;
  piStack_28 = piVar14;
  func_0x100bacf0();
  uVar17 = 0xffffffff;
  pcVar15 = (char *)0x10121c38;
  do {
    if (uVar17 == 0) break;
    uVar17 = uVar17 - 1;
    cVar4 = *pcVar15;
    pcVar15 = pcVar15 + 1;
  } while (cVar4 != '\0');
  func_0x100bba70(0x10121c38,~uVar17 - 1);
  iVar6 = *(int *)(unaff_ESI + 4);
  if (iVar6 != 0) {
    puVar1 = *(undefined4 **)(iVar6 + 8);
    for (puVar16 = *(undefined4 **)(iVar6 + 4); piVar14 = piVar9, puVar16 != puVar1;
        puVar16 = puVar16 + 1) {
      (**(code **)(*(int *)*puVar16 + 4))(uVar2,unaff_ESI,auStack_2c);
    }
  }
  uVar17 = 0xffffffff;
  *(undefined1 *)(iVar12 + 8) = 1;
  pcVar15 = (char *)0x10121c38;
  do {
    if (uVar17 == 0) break;
    uVar17 = uVar17 - 1;
    cVar4 = *pcVar15;
    pcVar15 = pcVar15 + 1;
  } while (cVar4 != '\0');
  iVar11 = ~uVar17 - 1;
  iVar6 = func_0x100be3b0(0,uStack_24,0x10121c38);
  if (iVar6 == 0) {
LAB_100b9e72:
    if (iVar12 != 0) {
      iVar6 = 0;
      piVar9 = (int *)puStack_4c[1];
      piVar14 = (int *)puStack_4c[2];
      piVar13 = piVar9;
      if (piVar9 != piVar14) {
        do {
          if (*piVar13 == iVar12) break;
          piVar13 = piVar13 + 1;
        } while (piVar13 != piVar14);
        if (piVar13 != piVar14) {
          if ((*(int *)(iVar12 + 4) == 0) || (iVar6 = func_0x100bad90(), iVar6 == 0)) {
            iVar6 = 0;
          }
          else {
            iVar6 = *(int *)(**(int **)(*(int *)(iVar12 + 4) + 4) + 0x14);
          }
          iVar6 = iVar6 + 1;
          piVar9 = piVar13 + 1;
        }
      }
      piVar14 = unaff_EBP;
      if (piVar9 != (int *)puStack_4c[2]) {
        do {
          iVar8 = *piVar9;
          if ((*(int *)(iVar8 + 4) == 0) || (iVar7 = func_0x100bad90(), iVar7 == 0)) {
            iVar7 = 0;
          }
          else {
            iVar7 = *(int *)(**(int **)(*(int *)(iVar8 + 4) + 4) + 0x14);
          }
          if (iVar7 < iVar6) break;
          if ((*(int *)(iVar8 + 4) == 0) || (iVar7 = func_0x100bad90(), iVar7 == 0)) {
            iVar8 = 0;
          }
          else {
            iVar8 = *(int *)(**(int **)(*(int *)(iVar8 + 4) + 4) + 0x14);
          }
          if (iVar8 == iVar6) goto LAB_100b9f23;
          piVar9 = piVar9 + 1;
        } while (piVar9 != (int *)puStack_4c[2]);
      }
      iVar12 = 0;
    }
LAB_100b9f23:
    piVar9 = (int *)func_0x100bb670(&puStack_4c,&stack0xffffffbc);
    *(int *)(*piVar9 + 0x10) = iVar12;
    puVar10 = *(undefined **)(iVar11 + 0xc);
    if (puVar10 == (undefined *)0x0) {
      puVar10 = &UNK_100f7058;
    }
    func_0x100ba080(piVar14,puVar10,8);
    func_0x100b9990(piVar14);
  }
  else {
    uVar17 = 0xffffffff;
    pcVar15 = (char *)0x1011c424;
    do {
      if (uVar17 == 0) break;
      uVar17 = uVar17 - 1;
      cVar4 = *pcVar15;
      pcVar15 = pcVar15 + 1;
    } while (cVar4 != '\0');
    iVar6 = func_0x100be3b0(0,piVar9,0x1011c424,~uVar17 - 1);
    if (iVar6 == 0) {
      iVar12 = 0;
      goto LAB_100b9f23;
    }
    uVar17 = 0xffffffff;
    pcVar15 = (char *)0x1011c41c;
    do {
      if (uVar17 == 0) break;
      uVar17 = uVar17 - 1;
      cVar4 = *pcVar15;
      pcVar15 = pcVar15 + 1;
    } while (cVar4 != '\0');
    iVar6 = func_0x100be3b0(0,piVar9,0x1011c41c,~uVar17 - 1);
    if (iVar6 != 0) goto LAB_100b9e72;
    piVar9 = (int *)func_0x100bb670(&puStack_4c,&stack0xffffffbc);
    *(undefined4 *)(*piVar9 + 0x10) = 0;
    if (*(int *)(iVar11 + 0x10) != 0) {
      puVar10 = *(undefined **)(iVar11 + 0xc);
      if (puVar10 == (undefined *)0x0) {
        puVar10 = &UNK_100f7058;
      }
      func_0x100ba080(piVar14,puVar10,8);
    }
    func_0x100c7430();
    func_0x100b94e0();
    uVar17 = 0xffffffff;
    pcVar15 = (char *)0x10121c38;
    do {
      pcVar18 = pcVar15;
      if (uVar17 == 0) break;
      uVar17 = uVar17 - 1;
      pcVar18 = pcVar15 + 1;
      cVar4 = *pcVar15;
      pcVar15 = pcVar18;
    } while (cVar4 != '\0');
    uVar17 = ~uVar17;
    pcVar15 = pcVar18 + -uVar17;
    pcVar18 = (char *)0x10134480;
    for (uVar5 = uVar17 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
      *(undefined4 *)pcVar18 = *(undefined4 *)pcVar15;
      pcVar15 = pcVar15 + 4;
      pcVar18 = pcVar18 + 4;
    }
    for (uVar17 = uVar17 & 3; uVar17 != 0; uVar17 = uVar17 - 1) {
      *pcVar18 = *pcVar15;
      pcVar15 = pcVar15 + 1;
      pcVar18 = pcVar18 + 1;
    }
  }
  if (iStack_1c != 0) {
    cVar4 = *(char *)(iStack_1c + -1);
    if ((cVar4 == '\0') || (cVar4 == -1)) {
      puStack_4c = (undefined4 *)(iStack_1c + -1);
      func_0x100c8ce6();
    }
    else {
      *(char *)(iStack_1c + -1) = cVar4 + -1;
    }
  }
LAB_100ba001:
  puStack_4c = puStack_c;
  func_0x100c8ce6();
  return;
}



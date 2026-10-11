/* REA procedure_pseudo_code 0x100ba080 */

void __cdecl FUN_100ba080(uint param_1,char *param_2,byte param_3)

{
  char cVar1;
  float fVar2;
  float *pfVar3;
  uint uVar4;
  float fVar5;
  char *pcVar6;
  char *pcVar7;
  float10 fVar8;
  
  fVar8 = FUN_100c5b60();
  pfVar3 = (float *)FUN_100b93f0(param_1);
  if (((param_3 & 8) == 0) && ((float)fVar8 - *pfVar3 <= 1.0)) {
    return;
  }
  *pfVar3 = (float)fVar8;
  uVar4 = 0xffffffff;
  pcVar6 = param_2;
  do {
    if (uVar4 == 0) break;
    uVar4 = uVar4 - 1;
    cVar1 = *pcVar6;
    pcVar6 = pcVar6 + 1;
  } while (cVar1 != '\0');
  fVar5 = (float)(~uVar4 - 1);
  if (0xfffffffd < (uint)fVar5) {
    FUN_100d9d61();
  }
  fVar2 = pfVar3[2];
  if (((fVar2 == 0.0) || (cVar1 = *(char *)((int)fVar2 + -1), cVar1 == '\0')) || (cVar1 == -1)) {
    if (fVar5 == 0.0) {
      FUN_100bacf0(pfVar3 + 1,'\x01');
      goto LAB_100ba15a;
    }
    if ((0x1f < (uint)pfVar3[4]) || ((uint)pfVar3[4] < (uint)fVar5)) {
      FUN_100bacf0(pfVar3 + 1,'\x01');
      goto LAB_100ba12f;
    }
  }
  else {
    if (fVar5 == 0.0) {
      *(char *)((int)fVar2 + -1) = cVar1 + -1;
      pfVar3[2] = 0.0;
      pfVar3[3] = 0.0;
      pfVar3[4] = 0.0;
      goto LAB_100ba15a;
    }
LAB_100ba12f:
    FUN_100bcd30((uint)fVar5);
  }
  pcVar6 = param_2;
  pcVar7 = (char *)pfVar3[2];
  for (uVar4 = (uint)fVar5 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
    *(undefined4 *)pcVar7 = *(undefined4 *)pcVar6;
    pcVar6 = pcVar6 + 4;
    pcVar7 = pcVar7 + 4;
  }
  for (uVar4 = (uint)fVar5 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
    *pcVar7 = *pcVar6;
    pcVar6 = pcVar6 + 1;
    pcVar7 = pcVar7 + 1;
  }
  pfVar3[3] = fVar5;
  *(undefined1 *)((int)pfVar3[2] + (int)fVar5) = 0;
LAB_100ba15a:
  FUN_100c7430();
  if ((param_3 & 2) != 0) {
    FUN_100c6e60(param_1,param_2,(char *)0x0,(char *)0x0,(char *)0x0,(char *)0x0,(char *)0x0,
                 (char *)0x0,0x44800000);
    return;
  }
  if ((param_3 & 4) == 0) {
    FUN_100c6e60(param_1,param_2,(char *)0x0,(char *)0x0,(char *)0x0,(char *)0x0,(char *)0x0,
                 (char *)0x0,0x43000000);
    return;
  }
  FUN_100c6e60(param_1,param_2,(char *)0x0,(char *)0x0,(char *)0x0,(char *)0x0,(char *)0x0,
               (char *)0x0,0x43800000);
  return;
}



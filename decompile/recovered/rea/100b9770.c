/* REA procedure_pseudo_code 0x100b9770 */

undefined4 __thiscall FUN_100b9770(void *this,undefined1 *param_1)

{
  char cVar1;
  bool bVar2;
  undefined3 extraout_var;
  undefined3 extraout_var_00;
  undefined3 uVar3;
  undefined3 extraout_var_01;
  undefined3 extraout_var_02;
  uint uVar4;
  char *pcVar5;
  char *pcVar6;
  uint *puVar7;
  undefined1 local_40 [4];
  char *local_3c;
  int local_38;
  undefined4 local_34;
  undefined1 local_30 [4];
  char *local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined1 local_20 [4];
  char *local_1c;
  int local_18;
  undefined1 local_10 [4];
  uint *local_c;
  
  pcVar5 = *(char **)this;
  local_10[0] = param_1._0_1_;
  FUN_100bacf0(local_10,'\0');
  uVar4 = 0xffffffff;
  pcVar6 = pcVar5;
  do {
    if (uVar4 == 0) break;
    uVar4 = uVar4 - 1;
    cVar1 = *pcVar6;
    pcVar6 = pcVar6 + 1;
  } while (cVar1 != '\0');
  FUN_100bba70(local_10,(undefined4 *)pcVar5,~uVar4 - 1);
  local_30[0] = *param_1;
  FUN_100bacf0(local_30,'\0');
  FUN_100ba9a0(local_30,param_1,0,0xffffffff);
  pcVar5 = *(char **)this;
  local_40[0] = param_1._0_1_;
  FUN_100bacf0(local_40,'\0');
  uVar4 = 0xffffffff;
  pcVar6 = pcVar5;
  do {
    if (uVar4 == 0) break;
    uVar4 = uVar4 - 1;
    cVar1 = *pcVar6;
    pcVar6 = pcVar6 + 1;
  } while (cVar1 != '\0');
  FUN_100bba70(local_40,(undefined4 *)pcVar5,~uVar4 - 1);
  local_20[0] = *param_1;
  FUN_100bacf0(local_20,'\0');
  FUN_100ba9a0(local_20,param_1,0,0xffffffff);
  FUN_100bac70(local_10);
  puVar7 = local_c;
  FUN_100bac70(local_40);
  if (local_3c == (char *)0x0) {
    pcVar5 = (char *)0x0;
  }
  else {
    pcVar5 = local_3c + local_38;
  }
  FUN_100bac70(local_40);
  for (pcVar6 = local_3c; pcVar6 != pcVar5; pcVar6 = pcVar6 + 1) {
    uVar4 = FUN_100c94bf((int)*pcVar6);
    *(char *)puVar7 = (char)uVar4;
    puVar7 = (uint *)((int)puVar7 + 1);
  }
  FUN_100bac70(local_30);
  pcVar5 = local_2c;
  FUN_100bac70(local_20);
  if (local_1c == (char *)0x0) {
    pcVar6 = (char *)0x0;
  }
  else {
    pcVar6 = local_1c + local_18;
  }
  FUN_100bac70(local_20);
  for (; local_1c != pcVar6; local_1c = local_1c + 1) {
    uVar4 = FUN_100c94bf((int)*local_1c);
    *pcVar5 = (char)uVar4;
    pcVar5 = pcVar5 + 1;
  }
  pcVar5 = local_2c;
  if (local_2c == (char *)0x0) {
    pcVar5 = "";
  }
  puVar7 = local_c;
  if (local_c == (uint *)0x0) {
    puVar7 = &DAT_100f7058;
  }
  bVar2 = FUN_100c8160(puVar7,pcVar5);
  FUN_100bacf0(local_20,'\x01');
  uVar3 = extraout_var;
  if (local_3c != (char *)0x0) {
    cVar1 = local_3c[-1];
    if ((cVar1 == '\0') || (cVar1 == -1)) {
      FUN_100c8ce6(local_3c + -1);
      uVar3 = extraout_var_00;
    }
    else {
      local_3c[-1] = cVar1 + -1;
    }
  }
  local_3c = (char *)0x0;
  local_38 = 0;
  local_34 = 0;
  if (local_2c != (char *)0x0) {
    cVar1 = local_2c[-1];
    if ((cVar1 == '\0') || (cVar1 == -1)) {
      FUN_100c8ce6(local_2c + -1);
      uVar3 = extraout_var_01;
    }
    else {
      local_2c[-1] = cVar1 + -1;
    }
  }
  local_2c = (char *)0x0;
  local_28 = 0;
  local_24 = 0;
  if (local_c != (uint *)0x0) {
    cVar1 = *(char *)((int)local_c + -1);
    if ((cVar1 != '\0') && (cVar1 != -1)) {
      *(char *)((int)local_c + -1) = cVar1 + -1;
      return CONCAT31(uVar3,bVar2);
    }
    FUN_100c8ce6((undefined *)((int)local_c + -1));
    uVar3 = extraout_var_02;
  }
  return CONCAT31(uVar3,bVar2);
}



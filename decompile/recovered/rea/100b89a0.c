/* REA procedure_pseudo_code 0x100b89a0 */

undefined1 * __cdecl FUN_100b89a0(undefined1 *param_1,undefined1 *param_2)

{
  undefined1 *puVar1;
  char cVar2;
  char *pcVar3;
  char *pcVar4;
  undefined1 local_10 [4];
  char *local_c;
  uint local_8;
  
  puVar1 = param_2;
  FUN_100baec0(&DAT_10132418,&param_2,(int)param_2);
  if (param_2 != DAT_1013241c) {
    puVar1 = param_2 + 0x1c;
    *param_1 = param_2[0x1c];
    FUN_100bacf0(param_1,'\0');
    FUN_100ba9a0(param_1,puVar1,0,0xffffffff);
    return param_1;
  }
  local_10[0] = *puVar1;
  FUN_100bacf0(local_10,'\0');
  FUN_100ba9a0(local_10,puVar1,0,0xffffffff);
  if (local_c != (char *)0x0) {
    if ((local_c[-1] != '\0') && (local_c[-1] != -1)) {
      FUN_100bbb30(local_10,local_8,'\0');
    }
    if ((local_c != (char *)0x0) && (local_c[-1] = -1, local_c != (char *)0x0)) {
      pcVar4 = local_c + local_8;
      if ((local_c[-1] != '\0') && (local_c[-1] != -1)) {
        FUN_100bbb30(local_10,local_8,'\0');
      }
      pcVar3 = local_c;
      if (local_c != (char *)0x0) {
        local_c[-1] = -1;
      }
      goto joined_r0x100b8a86;
    }
  }
  pcVar4 = (char *)0x0;
  pcVar3 = local_c;
joined_r0x100b8a86:
  for (; pcVar3 != pcVar4; pcVar3 = pcVar3 + 1) {
    if (*pcVar3 == '_') {
      *pcVar3 = ' ';
    }
  }
  *param_1 = local_10[0];
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 8) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0;
  FUN_100ba9a0(param_1,local_10,0,0xffffffff);
  if (local_c != (char *)0x0) {
    cVar2 = local_c[-1];
    if ((cVar2 != '\0') && (cVar2 != -1)) {
      local_c[-1] = cVar2 + -1;
      return param_1;
    }
    FUN_100c8ce6(local_c + -1);
  }
  return param_1;
}



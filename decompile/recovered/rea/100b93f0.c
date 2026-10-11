/* REA procedure_pseudo_code 0x100b93f0 */

int __cdecl FUN_100b93f0(uint param_1)

{
  char cVar1;
  int local_34 [2];
  undefined4 local_2c;
  undefined1 local_28 [4];
  int local_24;
  uint local_18;
  undefined4 local_14;
  undefined1 local_10 [4];
  int local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  local_2c = 0xff7fffff;
  local_28[0] = (undefined1)param_1;
  FUN_100bacf0(local_28,'\0');
  local_18 = param_1;
  local_14 = local_2c;
  local_10[0] = local_28[0];
  FUN_100bacf0(local_10,'\0');
  FUN_100ba9a0(local_10,local_28,0,0xffffffff);
  FUN_100bc220(&DAT_10132428,local_34,&local_18);
  if (local_c != 0) {
    cVar1 = *(char *)(local_c + -1);
    if ((cVar1 == '\0') || (cVar1 == -1)) {
      FUN_100c8ce6((char *)(local_c + -1));
    }
    else {
      *(char *)(local_c + -1) = cVar1 + -1;
    }
  }
  local_c = 0;
  local_8 = 0;
  local_4 = 0;
  if (local_24 != 0) {
    cVar1 = *(char *)(local_24 + -1);
    if ((cVar1 != '\0') && (cVar1 != -1)) {
      *(char *)(local_24 + -1) = cVar1 + -1;
      return local_34[0] + 0x10;
    }
    FUN_100c8ce6((char *)(local_24 + -1));
  }
  return local_34[0] + 0x10;
}



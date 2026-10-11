/* REA procedure_pseudo_code 0x100b94e0 */

void FUN_100b94e0(void)

{
  int *piVar1;
  int *piVar2;
  int *local_8;
  undefined4 local_4;
  
  piVar2 = DAT_1013242c;
  local_8 = (int *)*DAT_1013242c;
  if (DAT_10132434 == 0) {
    DAT_10132434 = 0;
    piVar1 = local_8;
    while (piVar1 != piVar2) {
      local_8 = piVar1;
      FUN_100bdb40((int *)&local_8);
      FUN_100bc460(&DAT_10132428,&local_4,piVar1);
      piVar1 = local_8;
    }
    return;
  }
  piVar2 = (int *)DAT_1013242c[1];
  if ((int *)DAT_1013242c[1] != DAT_10132444) {
    do {
      FUN_100bc860((int *)piVar2[2]);
      piVar1 = (int *)*piVar2;
      FUN_100ba960((int)(piVar2 + 4));
      FUN_100c8ce6((undefined *)piVar2);
      piVar2 = piVar1;
    } while (piVar1 != DAT_10132444);
  }
  DAT_1013242c[1] = (int)DAT_10132444;
  DAT_10132434 = 0;
  *DAT_1013242c = (int)DAT_1013242c;
  DAT_1013242c[2] = (int)DAT_1013242c;
  return;
}



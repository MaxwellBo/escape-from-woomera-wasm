/* REA procedure_pseudo_code 0x100b8c10 */

void FUN_100b8c10(void)

{
  undefined4 *puVar1;
  undefined1 local_5;
  undefined1 local_4 [4];
  
  DAT_10132428 = local_5;
  DAT_10132429 = local_5;
  DAT_10132430 = 0;
  puVar1 = operator_new(0x28);
  puVar1[1] = 0;
  puVar1[9] = 1;
  FUN_100d9cb0(local_4);
  if (DAT_10132444 == (undefined4 *)0x0) {
    DAT_10132444 = puVar1;
    *puVar1 = 0;
    puVar1 = (undefined4 *)0x0;
    DAT_10132444[2] = 0;
  }
  DAT_10132440 = DAT_10132440 + 1;
  FUN_100d9d4c();
  if (puVar1 != (undefined4 *)0x0) {
    FUN_100c8ce6((undefined *)puVar1);
  }
  puVar1 = DAT_10132444;
  DAT_1013242c = operator_new(0x28);
  *(undefined4 **)((int)DAT_1013242c + 4) = puVar1;
  *(undefined4 *)((int)DAT_1013242c + 0x24) = 0;
  DAT_10132434 = 0;
  *(void **)DAT_1013242c = DAT_1013242c;
  *(void **)((int)DAT_1013242c + 8) = DAT_1013242c;
  return;
}



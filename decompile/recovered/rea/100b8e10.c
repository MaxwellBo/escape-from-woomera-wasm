/* REA procedure_pseudo_code 0x100b8e10 */

void FUN_100b8e10(void)

{
  undefined4 *puVar1;
  undefined1 local_5;
  undefined1 local_4 [4];
  
  DAT_10132408 = local_5;
  DAT_10132409 = local_5;
  DAT_10132410 = 0;
  puVar1 = operator_new(0x18);
  puVar1[1] = 0;
  puVar1[5] = 1;
  FUN_100d9cb0(local_4);
  if (DAT_1013244c == (undefined4 *)0x0) {
    DAT_1013244c = puVar1;
    *puVar1 = 0;
    puVar1 = (undefined4 *)0x0;
    DAT_1013244c[2] = 0;
  }
  DAT_10132448 = DAT_10132448 + 1;
  FUN_100d9d4c();
  if (puVar1 != (undefined4 *)0x0) {
    FUN_100c8ce6((undefined *)puVar1);
  }
  puVar1 = DAT_1013244c;
  DAT_1013240c = operator_new(0x18);
  *(undefined4 **)((int)DAT_1013240c + 4) = puVar1;
  *(undefined4 *)((int)DAT_1013240c + 0x14) = 0;
  DAT_10132414 = 0;
  *(void **)DAT_1013240c = DAT_1013240c;
  *(void **)((int)DAT_1013240c + 8) = DAT_1013240c;
  return;
}



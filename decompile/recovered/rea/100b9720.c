/* REA procedure_pseudo_code 0x100b9720 */

char __thiscall FUN_100b9720(void *this,int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  char cVar4;
  undefined4 *puVar5;
  
  iVar1 = *param_1;
  if (iVar1 != 0) {
    uVar2 = *(undefined4 *)this;
    puVar3 = *(undefined4 **)(iVar1 + 8);
    cVar4 = '\x01';
    for (puVar5 = *(undefined4 **)(iVar1 + 4); puVar5 != puVar3; puVar5 = puVar5 + 1) {
      if ((cVar4 == '\0') || (cVar4 = (**(code **)(*(int *)*puVar5 + 4))(uVar2), cVar4 == '\0')) {
        cVar4 = '\0';
      }
      else {
        cVar4 = '\x01';
      }
    }
    return cVar4;
  }
  return '\x01';
}



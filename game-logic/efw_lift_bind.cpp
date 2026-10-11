/* Server-only bridge: the lifted overlay calls enginefuncs_t by slot index.
 * GiveFnptrsToDll in the original DLL copied 143 pointers (0x8f dwords)
 * starting at g_engfuncs. pfnSetModel is slot 2. The hlsdk struct starts
 * with the same fields, so the address of g_engfuncs is the slot array.
 */
#include "extdll.h"
#include "util.h"

extern "C" {

unsigned int *EFW_EngSlots(void)
{
	return (unsigned int *)&g_engfuncs;
}

void EFW_LiftKeep(void);

void EFW_LiftAnchor(void)
{
	EFW_LiftKeep();
}

}

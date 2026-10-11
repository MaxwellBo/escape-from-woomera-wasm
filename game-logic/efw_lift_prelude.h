/* Types and shims for the mechanical Ghidra lift of the EFW overlay.
 * Compile this as 32-bit C (wasm32 or gcc -m32). The decompiler treats
 * every absolute address as a 32-bit integer; EFW_VA relocates those into
 * efw_mem, which is a copy of the original DLL image.
 */
#ifndef EFW_LIFT_PRELUDE_H
#define EFW_LIFT_PRELUDE_H

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "efw_lift_size.h"

typedef uint8_t undefined;
typedef uint8_t undefined1;
typedef uint16_t undefined2;
typedef uint32_t undefined3;
typedef uint32_t undefined4;
typedef uint32_t uint3;
typedef uint32_t int3;
typedef uint8_t byte;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint32_t dword;
typedef long double float10;
/* Ghidra emits (**(code **)(slot))(). code is a function pointer, so a
 * code ** is one pointer further out and the double deref yields the fn. */
typedef uint32_t (*code)();
typedef uint32_t (*efw_fn)();

typedef void *HANDLE;
typedef char *LPCSTR;
typedef char *LPSTR;
typedef char CHAR;
typedef unsigned char BYTE;
typedef uint16_t WORD;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef void *LPVOID;
typedef int BOOL;
typedef uint32_t DWORD;

typedef struct {
	uint32_t dwFileAttributes;
	uint32_t ftCreationTime[2];
	uint32_t ftLastAccessTime[2];
	uint32_t ftLastWriteTime[2];
	uint32_t nFileSizeHigh;
	uint32_t nFileSizeLow;
	uint32_t dwReserved0;
	uint32_t dwReserved1;
	char cFileName[260];
	char cAlternateFileName[14];
	uint16_t pad;
} WIN32_FIND_DATAA;
typedef WIN32_FIND_DATAA _WIN32_FIND_DATAA;

#define CONCAT22(a, b) ((uint32_t)(((uint32_t)(uint16_t)(a) << 16) | (uint16_t)(b)))
#define CONCAT31(a, b) ((uint32_t)(((uint32_t)(a) << 8) | (uint8_t)(b)))
#define SQRT(x) sqrtl((long double)(x))
/* Ghidra's NAN(x) is the unordered x87 compare, not the math.h constant. */
#ifdef NAN
#undef NAN
#endif
static inline int NAN(long double x)
{
	return isnan(x);
}

/* Pass a float's bits through an undefined4 parameter. Ghidra's (float)
 * cast at such a call is not a conversion in the original binary. */
static inline uint32_t efw_fbits(float f)
{
	uint32_t u;
	memcpy(&u, &f, sizeof u);
	return u;
}

extern uint8_t efw_mem[EFW_MEM_SIZE];

static inline uint8_t *EFW_VA(uint32_t va)
{
	return efw_mem + (va - EFW_IMAGE_BASE);
}

void EFW_InstallImage(const uint8_t *src, uint32_t n);

/* Slot i of the GoldSrc enginefuncs_t copied by GiveFnptrsToDll.
 * The wasm build returns &g_engfuncs. The native test returns a dummy table. */
uint32_t *EFW_EngSlots(void);

HANDLE EFW_FindFirstFileA(const char *name, WIN32_FIND_DATAA *data);
int EFW_FindNextFileA(HANDLE find, WIN32_FIND_DATAA *data);
int EFW_FindClose(HANDLE find);
uint32_t EFW_GetTickCount(void);
void EFW_OutputDebugStringA(const char *text);
short EFW_GetAsyncKeyState(int vk);

#endif

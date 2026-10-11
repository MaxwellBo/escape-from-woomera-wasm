#include "efw_lift_prelude.h"

void EFW_InstallImage(const uint8_t *src, uint32_t n)
{
	uint32_t i;
	if (n > EFW_MEM_SIZE)
		n = EFW_MEM_SIZE;
	memcpy(efw_mem, src, n);
	if (n < EFW_MEM_SIZE)
		memset(efw_mem + n, 0, EFW_MEM_SIZE - n);
	/* Rebase dwords in rdata/data that point into the original image.
	 * Floats and small integers sit outside that address range. */
	for (i = (EFW_RELOC_LO - EFW_IMAGE_BASE) / 4; i < n / 4; i++) {
		uint32_t v = ((uint32_t *)efw_mem)[i];
		if (v >= EFW_IMAGE_BASE && v < EFW_IMAGE_BASE + EFW_MEM_SIZE)
			((uint32_t *)efw_mem)[i] =
				(uint32_t)(uintptr_t)(efw_mem + (v - EFW_IMAGE_BASE));
	}
}

HANDLE EFW_FindFirstFileA(const char *name, WIN32_FIND_DATAA *data)
{
	(void)name;
	if (data)
		memset(data, 0, sizeof(*data));
	return (HANDLE)0;
}

int EFW_FindNextFileA(HANDLE find, WIN32_FIND_DATAA *data)
{
	(void)find;
	(void)data;
	return 0;
}

int EFW_FindClose(HANDLE find)
{
	(void)find;
	return 1;
}

uint32_t EFW_GetTickCount(void)
{
	return 0;
}

void EFW_OutputDebugStringA(const char *text)
{
	if (text && text[0])
		fputs(text, stderr);
}

short EFW_GetAsyncKeyState(int vk)
{
	(void)vk;
	return 0;
}

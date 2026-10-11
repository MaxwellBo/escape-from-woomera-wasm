/* Wine 6's web filesystem ships a placeholder psapi.dll and no psapi.dll.so.
   xash.dll imports EnumProcessModules, GetModuleBaseNameA and
   GetModuleInformation (crash handler). Implement those with toolhelp. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>

typedef struct MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD SizeOfImage;
    LPVOID EntryPoint;
} MODULEINFO, *LPMODULEINFO;

static DWORD pid_of(HANDLE process)
{
    typedef DWORD(WINAPI *GetProcessIdFn)(HANDLE);
    GetProcessIdFn get_id;

    if (process == NULL || process == (HANDLE)(LONG_PTR)-1)
        return GetCurrentProcessId();
    get_id = (GetProcessIdFn)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetProcessId");
    if (get_id) {
        DWORD id = get_id(process);
        if (id)
            return id;
    }
    return GetCurrentProcessId();
}

static LPVOID entry_of(BYTE *base)
{
    IMAGE_DOS_HEADER *dos;
    IMAGE_NT_HEADERS *nt;
    MEMORY_BASIC_INFORMATION info;

    if (!base || !VirtualQuery(base, &info, sizeof(info)))
        return NULL;
    dos = (IMAGE_DOS_HEADER *)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew < 0 || dos->e_lfanew > 0x1000)
        return NULL;
    nt = (IMAGE_NT_HEADERS *)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE)
        return NULL;
    if (!nt->OptionalHeader.AddressOfEntryPoint)
        return NULL;
    return base + nt->OptionalHeader.AddressOfEntryPoint;
}

BOOL WINAPI EnumProcessModules(HANDLE process, HMODULE *modules, DWORD cb, DWORD *needed)
{
    HANDLE snap;
    MODULEENTRY32 me;
    DWORD count = 0;
    DWORD pid = pid_of(process);

    if (!needed)
        return FALSE;
    snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (snap == INVALID_HANDLE_VALUE)
        return FALSE;
    me.dwSize = sizeof(me);
    if (Module32First(snap, &me)) {
        do {
            if (modules && (count + 1) * sizeof(HMODULE) <= cb)
                modules[count] = me.hModule;
            count++;
        } while (Module32Next(snap, &me));
    }
    CloseHandle(snap);
    *needed = count * sizeof(HMODULE);
    return TRUE;
}

DWORD WINAPI GetModuleBaseNameA(HANDLE process, HMODULE module, LPSTR name, DWORD size)
{
    HANDLE snap;
    MODULEENTRY32 me;
    DWORD pid = pid_of(process);
    DWORD written = 0;

    if (!name || !size)
        return 0;
    name[0] = 0;
    snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (snap == INVALID_HANDLE_VALUE)
        return 0;
    me.dwSize = sizeof(me);
    if (Module32First(snap, &me)) {
        do {
            if (module == NULL || me.hModule == module) {
                lstrcpynA(name, me.szModule, size);
                written = lstrlenA(name);
                break;
            }
        } while (Module32Next(snap, &me));
    }
    CloseHandle(snap);
    return written;
}

BOOL WINAPI GetModuleInformation(HANDLE process, HMODULE module, LPMODULEINFO info, DWORD cb)
{
    HANDLE snap;
    MODULEENTRY32 me;
    DWORD pid = pid_of(process);
    BOOL found = FALSE;

    if (!info || cb < sizeof(MODULEINFO))
        return FALSE;
    snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (snap == INVALID_HANDLE_VALUE)
        return FALSE;
    me.dwSize = sizeof(me);
    if (Module32First(snap, &me)) {
        do {
            if (module == NULL || me.hModule == module) {
                info->lpBaseOfDll = me.modBaseAddr;
                info->SizeOfImage = me.modBaseSize;
                info->EntryPoint = entry_of(me.modBaseAddr);
                found = TRUE;
                break;
            }
        } while (Module32Next(snap, &me));
    }
    CloseHandle(snap);
    return found;
}

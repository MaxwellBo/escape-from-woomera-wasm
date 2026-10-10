/* CRT pieces the MSVC-ABI freevgui objects reference. Wine 6's msvcrt
   covers stdio; these fill the C++ bits clang emits for i686-windows-msvc. */
extern "C" int _fltused = 0;
extern "C" int __cdecl _purecall(void) { return 0; }
extern "C" void __cdecl _chkstk(void) {}
extern "C" void *__cdecl __RTDynamicCast(void *, int, void *, void *, int) { return 0; }
extern "C" void *__cdecl __RTCastToVoid(void *p) { return p; }
extern "C" int __cdecl atexit(void (*fn)(void))
{
    (void)fn;
    return 0;
}

extern "C" {
struct _iobuf {
    char pad[32];
};
struct _iobuf _iob[20];
void *ti_vftable[1] = {0};
}

extern "C" int __stdcall DllMain(void *inst, unsigned long reason, void *reserved)
{
    (void)inst;
    (void)reason;
    (void)reserved;
    return 1;
}

/* Classic msvcrt exports _iob. Newer MinGW headers route stdin/stdout
   through __acrt_iob_func, which this Wine build does not provide. */
#define _STDSTREAM_DEFINED
struct _iobuf;
typedef struct _iobuf FILE;
extern "C" FILE _iob[];
#define stdin (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])

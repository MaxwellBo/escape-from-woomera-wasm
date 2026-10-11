#include <stdio.h>

/* Wine 6 ships wmic.exe as a builtin placeholder and omits wmic.exe.so.
   Loading that placeholder aborts Boxedwine. The engine also reads wmic's
   stdout from a pipe and blocks until the child writes something, so an
   empty process is not enough. */
int main(void) {
	fputs("SerialNumber\r\nNONE\r\n", stdout);
	fflush(stdout);
	return 0;
}

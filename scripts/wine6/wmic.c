/* Wine 6 ships wmic.exe as a builtin placeholder and omits wmic.exe.so.
   Loading that placeholder aborts Boxedwine. A native exe that exits 0 is
   enough for the monitor queries SDL makes while creating the game window. */
int main(void) {
    return 0;
}

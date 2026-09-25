#include "efw_persist.h"
#include <stdio.h>

int main( void )
{
	int fail = 0;

	/* Fresh WASM boot always resets, including +map level2. */
	if( !EFW_ShouldResetHope( 0, 0, 0 ) )
	{
		printf( "first boot level1 should reset\n" );
		fail++;
	}
	if( !EFW_ShouldResetHope( 1, 0, 0 ) )
	{
		printf( "first boot level2 should reset\n" );
		fail++;
	}

	/* FUN_100c6780: level1 always FUN_100c6740. */
	if( !EFW_ShouldResetHope( 0, 0, 1 ) )
	{
		printf( "level1 inited should reset\n" );
		fail++;
	}
	if( !EFW_ShouldResetHope( 0, 1, 1 ) )
	{
		printf( "level1 latch should reset\n" );
		fail++;
	}

	/* Chapter 2/3 after chapter 1: skip 6740, keep keywords/items/hope. */
	if( EFW_ShouldResetHope( 1, 0, 1 ) )
	{
		printf( "level2 after chapter 1 must persist\n" );
		fail++;
	}
	if( EFW_ShouldResetHope( 2, 0, 1 ) )
	{
		printf( "level3 after chapter 1 must persist\n" );
		fail++;
	}

	/* Latch set (DAT_1011d14c != 0) still resets on a later map. */
	if( !EFW_ShouldResetHope( 1, 1, 1 ) )
	{
		printf( "level2 with latch should reset\n" );
		fail++;
	}

	if( EFW_LoadoutBits( 0 ) != ( ( 1 << 5 ) | ( 1 << 8 ) ) )
	{
		printf( "FUN_100c3020 level1 loadout\n" );
		fail++;
	}
	if( EFW_LoadoutBits( 1 ) != ( 1 << 2 ) )
	{
		printf( "FUN_100c3020 level2 loadout\n" );
		fail++;
	}
	if( EFW_LoadoutBits( 2 ) != ( 1 << 0 ) )
	{
		printf( "FUN_100c3020 level3 loadout\n" );
		fail++;
	}

	if( fail )
	{
		printf( "FAIL %d\n", fail );
		return 1;
	}
	printf( "FUN_100c6780 persist ok\n" );
	return 0;
}

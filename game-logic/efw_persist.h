#pragma once

/* FUN_100c6780 DAT_1011d14c: hope/keyword reset (FUN_100c6740) runs on
   level1, or when the persist latch is already set. First WASM boot
   (!inited) also resets — PE BSS hope is 0, but +map level2 must still
   be playable. Changelevel from chapter 1 keeps hope, keywords, items. */
static inline int EFW_ShouldResetHope( int mapLevel, int persistLatch, int inited )
{
	if( !inited )
		return 1;
	if( mapLevel == 0 )
		return 1;
	return persistLatch != 0;
}

/* FUN_100c3020 starting loadout by maplevel (0=level1, 1=level2, 2=level3).
   Bits match EFW_ITEM_* in efw.h (IDTag=1<<5, RedCard=1<<8, Lever=1<<2, Pliers=1<<0). */
static inline int EFW_LoadoutBits( int mapLevel )
{
	if( mapLevel == 0 )
		return ( 1 << 5 ) | ( 1 << 8 );
	if( mapLevel == 1 )
		return ( 1 << 2 );
	if( mapLevel == 2 )
		return ( 1 << 0 );
	return 0;
}

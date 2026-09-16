/* CRefugee::Spawn recovered from EscapeFromWoomera.dll
 * VA 0x100c6040. Ghidra left a gap between FUN_100c6000 (Precache, 51 bytes)
 * and FUN_100c6420. Capstone of the PE, image base 0x10000000.
 *
 * FUN_100c6000 PRECACHE_MODEL on PTR 0x1011cf40[0..DAT_1011cf74)
 * (13 entries: detaineeMaleT0–T7, detaineeFemaleT0–T2, security, tradesman),
 * then FUN_100c5fb0 (Dingaling.wav).
 *
 * FUN_100c8160 is efw_FStrEq (case-sensitive). SET_MODEL is DAT_10121e10.
 * Unknown names: DebugPrint "Model not known for name: %s", DAT_10132ccc++,
 * odd → MaleT6, even → MaleT7.
 *
 * WASM uses zip casing (DetaineeMaleT0.mdl) because the MEMFS is case-sensitive.
 */
void CRefugee::Spawn( void )
{
	Precache(); /* vtable+0x2c → FUN_100c6000 */
	pev->movetype = 4; /* MOVETYPE_STEP */
	pev->solid = 2;    /* SOLID_BBOX */
	pev->takedamage = 1.0f;
	pev->flags |= 0x20; /* FL_MONSTER */
	pev->health = 80.0f;
	pev->gravity = 1.0f;

	if( efw_FStrEq( STRING( pev->targetname ), "Shala" ) )
		SET_MODEL( edict(), "models/detaineeFemaleT0.mdl" );
	else if( efw_FStrEq( STRING( pev->targetname ), "Amir" ) )
		SET_MODEL( edict(), "models/detaineeMaleT0.mdl" );
	else if( efw_FStrEq( STRING( pev->targetname ), "Hassan" ) )
		SET_MODEL( edict(), "models/detaineeMaleT1.mdl" );
	else if( efw_FStrEq( STRING( pev->targetname ), "Laleh" ) )
		SET_MODEL( edict(), "models/detaineeFemaleT1.mdl" );
	else if( efw_FStrEq( STRING( pev->targetname ), "Elika" ) )
		SET_MODEL( edict(), "models/detaineeFemaleT2.mdl" );
	else if( efw_FStrEq( STRING( pev->targetname ), "Mouhtaz" ) )
		SET_MODEL( edict(), "models/detaineeMaleT2.mdl" );
	else if( efw_FStrEq( STRING( pev->targetname ), "Fashid" ) )
		SET_MODEL( edict(), "models/detaineeMaleT3.mdl" );
	else if( efw_FStrEq( STRING( pev->targetname ), "Nasir" ) )
		SET_MODEL( edict(), "models/detaineeMaleT4.mdl" );
	else if( efw_FStrEq( STRING( pev->targetname ), "Gholan" ) )
		SET_MODEL( edict(), "models/detaineeMaleT5.mdl" );
	else
	{
		efw_DebugPrint( "Model not known for name: %s\n", STRING( pev->targetname ) );
		DAT_10132ccc++;
		if( DAT_10132ccc % 2 )
			SET_MODEL( edict(), "models/detaineeMaleT6.mdl" );
		else
			SET_MODEL( edict(), "models/detaineeMaleT7.mdl" );
	}

	UTIL_SetSize( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 72 ) );
	/* vtable+0x1a8(1); FUN_100085d0 MonsterInit */
	SetThink( &CRefugee::IdleThink ); /* this+0x74 = 0x100c6440 */
	pev->nextthink = gpGlobals->time + 0.1f;
	pev->framerate = 1.0f;
	pev->movetype = 4;
}

int CRefugee::Classify( void ) /* 0x100c6310 */
{
	return 3; /* CLASS_HUMAN_PASSIVE */
}

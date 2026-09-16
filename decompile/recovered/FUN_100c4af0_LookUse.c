/* FUN_100c4af0 — player look-use. Capstone of EscapeFromWoomera.dll.
 * Ghidra left a gap between weapon_efw_WashingPowder (0x100c4a80, 106 bytes)
 * and FUN_100c4d10. Image base 0x10000000. cdecl, one argument: CBaseEntity*
 * (the player). No E8/E9/pointer callers in the PE — vtable-only candidate
 * that the 2004 link never stored. WASM attaches it to IN_USE.
 *
 * 0x100af200 UTIL_MakeVectors(pev->v_angle)
 * vtable+0x214 EyePosition()  → hidden Vector* out
 * 0x100aeba0 UTIL_FindEntityInSphere(start, eye, 96.0)   ; 0x42c00000
 * dir = ent.origin - eye; if |dir|==0 use (0,0,1) else normalize
 * 0x100c9190 _CIacos (fld1/fadd/fsub/fmul/fsqrt/fpatan)
 * keep if acos(dot(dir, gpGlobals->v_forward)) < 0.17453278  (~10°)
 * 0x100afc60 UTIL_TraceLine(eye, ent.origin, dont_ignore_monsters=0, player)
 * if tr.flFraction >= 0.97:  vtable+0x114(player) on the sphere entity
 * else if Instance(tr.pHit)->classname == "efw_Marker" @ 0x1011c9ac:
 *     vtable+0x114(player) on the hit entity
 * else next sphere entity
 *
 * vtable+0x114 (thiscall, 1 stack arg, ret 4):
 *   CEfwWeapon  0x100c4690 → FUN_100c46a0 AddToPlayer
 *   CEfwMarker  0x100031b0 → empty stub (ret 4)
 *   CRefugee    0x100031b0 → empty stub
 * The efw_Marker classname branch is the unfinished marker path; the WASM
 * port calls EFW_UseMarker so kitchen_bin / IDTag_Position work without
 * VGUI CommandButtons.
 */
void FUN_100c4af0( CBaseEntity *player )
{
	Vector eye;
	CBaseEntity *ent;
	Vector dir;
	float len;
	float ang;
	TraceResult tr;
	CBaseEntity *hit;

	UTIL_MakeVectors( player->pev->v_angle );
	eye = player->EyePosition(); /* vtable+0x214 */

	ent = UTIL_FindEntityInSphere( NULL, eye, 96.0f );
	while( ent )
	{
		if( ent != player )
		{
			dir = ent->pev->origin - eye;
			len = dir.Length();
			if( len == 0.0f )
				dir = Vector( 0, 0, 1 );
			else
				dir = dir * ( 1.0f / len );

			ang = acos( DotProduct( dir, gpGlobals->v_forward ) );
			if( ang < 0.17453278f )
			{
				UTIL_TraceLine( eye, ent->pev->origin, dont_ignore_monsters,
					player->edict(), &tr );
				if( tr.flFraction >= 0.97f )
				{
					/* vtable+0x114(player) */
					return;
				}
				hit = CBaseEntity::Instance( tr.pHit );
				if( hit && !strcmp( STRING( hit->pev->classname ), "efw_Marker" ) )
				{
					/* vtable+0x114(player) on marker */
					return;
				}
			}
		}
		ent = UTIL_FindEntityInSphere( ent, eye, 96.0f );
	}
}

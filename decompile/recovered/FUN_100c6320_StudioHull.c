/* FUN_100c6320 — CRefugee studio sequence hull.
 * Capstone of the gap after Classify (0x100c6310) in EscapeFromWoomera.dll.
 * thiscall. No vtable pointer in the PE (Spawn still hardcodes
 * UTIL_SetSize -16/-16/0 .. 16/16/72); WASM overrides SetObjectCollisionBox
 * so DispatchObjectCollsionBox uses the model sequence bbox.
 *
 * GET_MODEL_PTR = engine pfn at DAT_10121f30.
 * studiohdr_t.seqindex at +0xa8; mstudioseqdesc_t size 176 (0xb0).
 * pev->sequence at entvars +0x128.
 * bbmin at seqdesc+0x60, bbmax at +0x6c → UTIL_SetSize (0x100afdc0).
 * Null model: ALERT(1, "Invalid model ptr! FUCK\n") @ 0x1011d0dc.
 */
void CRefugee::SetObjectCollisionBox( void )
{
	void *pmodel = GET_MODEL_PTR( edict() );
	studiohdr_t *hdr;
	mstudioseqdesc_t *seq;

	if( !pmodel )
		ALERT( at_console, "Invalid model ptr! FUCK\n" );
	hdr = (studiohdr_t *)pmodel;
	seq = (mstudioseqdesc_t *)( (byte *)hdr + hdr->seqindex ) + pev->sequence;
	UTIL_SetSize( pev, seq->bbmin, seq->bbmax );
}

/* Runs the lifted hope drain without booting the game.
 * efw_ThinkHope is FUN_100c6ad0: hope -= dt * (1/12), clamp 0..100.
 * dt is the float at 0x10134874 (FUN_100c5b70). Hope slot 1 is the float
 * at 0x10134948 + 4 (FUN_100c8180 / FUN_100c8190).
 */
#include "efw_lift_prelude.h"

/* lift_ prefix keeps these off the hlsdk export names (monster_refugee, …). */
extern void lift_FUN_100c8180(int param_1, undefined4 param_2);
extern float10 lift_FUN_100c8190(int param_1);
extern void lift_efw_ThinkHope(void);

/* The wasm bridge provides the real table. The native test does not call it. */
static uint32_t g_dummy_slots[143];

uint32_t *EFW_EngSlots(void)
{
	return g_dummy_slots;
}

static uint32_t bits_of(float v)
{
	uint32_t bits;
	memcpy(&bits, &v, 4);
	return bits;
}

static int near(long double got, double want)
{
	long double d = got - (long double)want;
	if (d < 0)
		d = -d;
	return d < 0.02L;
}

int main(void)
{
	long double got;

	memcpy(EFW_VA(0x10134874), &(float){12.f}, 4);
	lift_FUN_100c8180(1, bits_of(80.f));
	lift_efw_ThinkHope();
	got = lift_FUN_100c8190(1);
	if (!near(got, 79.0)) {
		fprintf(stderr, "hope drain got %Lf, want 79\n", got);
		return 1;
	}

	/* A full second at dt=12 drains exactly 1. Twelve of those from 10
	 * lands on 0 and must clamp, which also fires the fail path. Stay
	 * above zero here: 10 - 1 = 9. */
	lift_FUN_100c8180(1, bits_of(10.f));
	lift_efw_ThinkHope();
	got = lift_FUN_100c8190(1);
	if (!near(got, 9.0)) {
		fprintf(stderr, "hope drain got %Lf, want 9\n", got);
		return 1;
	}

	/* Clamp at the top. */
	memcpy(EFW_VA(0x10134874), &(float){-1200.f}, 4);
	lift_FUN_100c8180(1, bits_of(50.f));
	lift_efw_ThinkHope();
	got = lift_FUN_100c8190(1);
	if (!near(got, 100.0)) {
		fprintf(stderr, "hope clamp got %Lf, want 100\n", got);
		return 1;
	}

	printf("lifted efw_ThinkHope ok\n");
	return 0;
}

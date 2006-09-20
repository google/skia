#include "SkFilterProc.h"

/*	[1-x 1-y] [x 1-y]
	[1-x   y] [x   y]
*/

static unsigned bilerp00(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return a00; }
static unsigned bilerp01(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (3 * a00 + a01) >> 2; }
static unsigned bilerp02(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (a00 + a01) >> 1; }
static unsigned bilerp03(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (a00 + 3 * a01) >> 2; }

static unsigned bilerp10(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (3 * a00 + a10) >> 2; }
static unsigned bilerp11(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (9 * a00 + 3 * (a01 + a10) + a11) >> 4; }
static unsigned bilerp12(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (3 * (a00 + a01) + a10 + a11) >> 3; }
static unsigned bilerp13(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (9 * a01 + 3 * (a00 + a11) + a10) >> 4; }

static unsigned bilerp20(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (a00 + a10) >> 1; }
static unsigned bilerp21(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (3 * (a00 + a10) + a01 + a11) >> 3; }
static unsigned bilerp22(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (a00 + a01 + a10 + a11) >> 2; }
static unsigned bilerp23(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (3 * (a01 + a11) + a00 + a10) >> 3; }

static unsigned bilerp30(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (a00 + 3 * a10) >> 2; }
static unsigned bilerp31(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (9 * a10 + 3 * (a00 + a11) + a01) >> 4; }
static unsigned bilerp32(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (3 * (a10 + a11) + a00 + a01) >> 3; }
static unsigned bilerp33(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (9 * a11 + 3 * (a01 + a10) + a00) >> 4; }

static const SkFilterProc gBilerpProcs[4 * 4] = {
	bilerp00, bilerp01, bilerp02, bilerp03,
	bilerp10, bilerp11, bilerp12, bilerp13,
	bilerp20, bilerp21, bilerp22, bilerp23,
	bilerp30, bilerp31, bilerp32, bilerp33
};

const SkFilterProc* SkGetBilinearFilterProcTable()
{
	return gBilerpProcs;
}


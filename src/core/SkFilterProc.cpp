
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkFilterProc.h"

/*  [1-x 1-y] [x 1-y]
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define MASK            0xFF00FF
#define LO_PAIR(x)      ((x) & MASK)
#define HI_PAIR(x)      (((x) >> 8) & MASK)
#define COMBINE(lo, hi) (((lo) & ~0xFF00) | (((hi) & ~0xFF00) << 8))

///////////////////////////////////////////////////////////////////////////////

static unsigned bilerp4_00(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    return c00;
}
static unsigned bilerp4_01(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (3 * LO_PAIR(c00) + LO_PAIR(c01)) >> 2;
    uint32_t hi = (3 * HI_PAIR(c00) + HI_PAIR(c01)) >> 2;
    return COMBINE(lo, hi);
}
static unsigned bilerp4_02(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (LO_PAIR(c00) + LO_PAIR(c01)) >> 1;
    uint32_t hi = (HI_PAIR(c00) + HI_PAIR(c01)) >> 1;
    return COMBINE(lo, hi);
}
static unsigned bilerp4_03(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (LO_PAIR(c00) + 3 * LO_PAIR(c01)) >> 2;
    uint32_t hi = (HI_PAIR(c00) + 3 * HI_PAIR(c01)) >> 2;
    return COMBINE(lo, hi);
}

static unsigned bilerp4_10(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (3 * LO_PAIR(c00) + LO_PAIR(c10)) >> 2;
    uint32_t hi = (3 * HI_PAIR(c00) + HI_PAIR(c10)) >> 2;
    return COMBINE(lo, hi);
}
static unsigned bilerp4_11(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (9 * LO_PAIR(c00) + 3 * (LO_PAIR(c01) + LO_PAIR(c10)) + LO_PAIR(c11)) >> 4;
    uint32_t hi = (9 * HI_PAIR(c00) + 3 * (HI_PAIR(c01) + HI_PAIR(c10)) + HI_PAIR(c11)) >> 4;
    return COMBINE(lo, hi);
}
static unsigned bilerp4_12(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (3 * (LO_PAIR(c00) + LO_PAIR(c01)) + LO_PAIR(c10) + LO_PAIR(c11)) >> 3;
    uint32_t hi = (3 * (HI_PAIR(c00) + HI_PAIR(c01)) + HI_PAIR(c10) + HI_PAIR(c11)) >> 3;
    return COMBINE(lo, hi);
}
static unsigned bilerp4_13(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (9 * LO_PAIR(c01) + 3 * (LO_PAIR(c00) + LO_PAIR(c11)) + LO_PAIR(c10)) >> 4;
    uint32_t hi = (9 * HI_PAIR(c01) + 3 * (HI_PAIR(c00) + HI_PAIR(c11)) + HI_PAIR(c10)) >> 4;
    return COMBINE(lo, hi);
}

static unsigned bilerp4_20(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (LO_PAIR(c00) + LO_PAIR(c10)) >> 1;
    uint32_t hi = (HI_PAIR(c00) + HI_PAIR(c10)) >> 1;
    return COMBINE(lo, hi);
}
static unsigned bilerp4_21(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (3 * (LO_PAIR(c00) + LO_PAIR(c10)) + LO_PAIR(c01) + LO_PAIR(c11)) >> 3;
    uint32_t hi = (3 * (HI_PAIR(c00) + HI_PAIR(c10)) + HI_PAIR(c01) + HI_PAIR(c11)) >> 3;
    return COMBINE(lo, hi);
}
static unsigned bilerp4_22(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (LO_PAIR(c00) + LO_PAIR(c01) + LO_PAIR(c10) + LO_PAIR(c11)) >> 2;
    uint32_t hi = (HI_PAIR(c00) + HI_PAIR(c01) + HI_PAIR(c10) + HI_PAIR(c11)) >> 2;
    return COMBINE(lo, hi);
}
static unsigned bilerp4_23(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (3 * (LO_PAIR(c01) + LO_PAIR(c11)) + LO_PAIR(c00) + LO_PAIR(c10)) >> 3;
    uint32_t hi = (3 * (HI_PAIR(c01) + HI_PAIR(c11)) + HI_PAIR(c00) + HI_PAIR(c10)) >> 3;
    return COMBINE(lo, hi);
}

static unsigned bilerp4_30(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (LO_PAIR(c00) + 3 * LO_PAIR(c10)) >> 2;
    uint32_t hi = (HI_PAIR(c00) + 3 * HI_PAIR(c10)) >> 2;
    return COMBINE(lo, hi);
}
static unsigned bilerp4_31(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (9 * LO_PAIR(c10) + 3 * (LO_PAIR(c00) + LO_PAIR(c11)) + LO_PAIR(c01)) >> 4;
    uint32_t hi = (9 * HI_PAIR(c10) + 3 * (HI_PAIR(c00) + HI_PAIR(c11)) + HI_PAIR(c01)) >> 4;
    return COMBINE(lo, hi);
}
static unsigned bilerp4_32(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (3 * (LO_PAIR(c10) + LO_PAIR(c11)) + LO_PAIR(c00) + LO_PAIR(c01)) >> 3;
    uint32_t hi = (3 * (HI_PAIR(c10) + HI_PAIR(c11)) + HI_PAIR(c00) + HI_PAIR(c01)) >> 3;
    return COMBINE(lo, hi);
}
static unsigned bilerp4_33(uint32_t c00, uint32_t c01, uint32_t c10, uint32_t c11) {
    uint32_t lo = (9 * LO_PAIR(c11) + 3 * (LO_PAIR(c01) + LO_PAIR(c10)) + LO_PAIR(c00)) >> 4;
    uint32_t hi = (9 * HI_PAIR(c11) + 3 * (HI_PAIR(c01) + HI_PAIR(c10)) + HI_PAIR(c00)) >> 4;
    return COMBINE(lo, hi);
}

static const SkFilter32Proc gBilerp32Procs[4 * 4] = {
    bilerp4_00, bilerp4_01, bilerp4_02, bilerp4_03,
    bilerp4_10, bilerp4_11, bilerp4_12, bilerp4_13,
    bilerp4_20, bilerp4_21, bilerp4_22, bilerp4_23,
    bilerp4_30, bilerp4_31, bilerp4_32, bilerp4_33
};

const SkFilter32Proc* SkGetFilter32ProcTable()
{
    return gBilerp32Procs;
}

///////////////////////////////////////////////////////////////////////////////

static uint32_t bilerptr00(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    return *a00;
}
static uint32_t bilerptr01(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;   
    uint32_t lo = (3 * LO_PAIR(c00) + LO_PAIR(c01)) >> 2;
    uint32_t hi = (3 * HI_PAIR(c00) + HI_PAIR(c01)) >> 2;
    return COMBINE(lo, hi);
}
static uint32_t bilerptr02(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;   
    uint32_t lo = (LO_PAIR(c00) + LO_PAIR(c01)) >> 1;
    uint32_t hi = (HI_PAIR(c00) + HI_PAIR(c01)) >> 1;
    return COMBINE(lo, hi);
}
static uint32_t bilerptr03(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t lo = (LO_PAIR(c00) + 3 * LO_PAIR(c01)) >> 2;
    uint32_t hi = (HI_PAIR(c00) + 3 * HI_PAIR(c01)) >> 2;
    return COMBINE(lo, hi);
}

static uint32_t bilerptr10(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c10 = *a10;
    uint32_t lo = (3 * LO_PAIR(c00) + LO_PAIR(c10)) >> 2;
    uint32_t hi = (3 * HI_PAIR(c00) + HI_PAIR(c10)) >> 2;
    return COMBINE(lo, hi);
}
static uint32_t bilerptr11(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (9 * LO_PAIR(c00) + 3 * (LO_PAIR(c01) + LO_PAIR(c10)) + LO_PAIR(c11)) >> 4;
    uint32_t hi = (9 * HI_PAIR(c00) + 3 * (HI_PAIR(c01) + HI_PAIR(c10)) + HI_PAIR(c11)) >> 4;
    return COMBINE(lo, hi);
}
static uint32_t bilerptr12(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (3 * (LO_PAIR(c00) + LO_PAIR(c01)) + LO_PAIR(c10) + LO_PAIR(c11)) >> 3;
    uint32_t hi = (3 * (HI_PAIR(c00) + HI_PAIR(c01)) + HI_PAIR(c10) + HI_PAIR(c11)) >> 3;
    return COMBINE(lo, hi);
}
static uint32_t bilerptr13(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (9 * LO_PAIR(c01) + 3 * (LO_PAIR(c00) + LO_PAIR(c11)) + LO_PAIR(c10)) >> 4;
    uint32_t hi = (9 * HI_PAIR(c01) + 3 * (HI_PAIR(c00) + HI_PAIR(c11)) + HI_PAIR(c10)) >> 4;
    return COMBINE(lo, hi);
}

static uint32_t bilerptr20(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c10 = *a10;
    uint32_t lo = (LO_PAIR(c00) + LO_PAIR(c10)) >> 1;
    uint32_t hi = (HI_PAIR(c00) + HI_PAIR(c10)) >> 1;
    return COMBINE(lo, hi);
}
static uint32_t bilerptr21(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (3 * (LO_PAIR(c00) + LO_PAIR(c10)) + LO_PAIR(c01) + LO_PAIR(c11)) >> 3;
    uint32_t hi = (3 * (HI_PAIR(c00) + HI_PAIR(c10)) + HI_PAIR(c01) + HI_PAIR(c11)) >> 3;
    return COMBINE(lo, hi);
}
static uint32_t bilerptr22(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (LO_PAIR(c00) + LO_PAIR(c01) + LO_PAIR(c10) + LO_PAIR(c11)) >> 2;
    uint32_t hi = (HI_PAIR(c00) + HI_PAIR(c01) + HI_PAIR(c10) + HI_PAIR(c11)) >> 2;
    return COMBINE(lo, hi);
}
static uint32_t bilerptr23(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (3 * (LO_PAIR(c01) + LO_PAIR(c11)) + LO_PAIR(c00) + LO_PAIR(c10)) >> 3;
    uint32_t hi = (3 * (HI_PAIR(c01) + HI_PAIR(c11)) + HI_PAIR(c00) + HI_PAIR(c10)) >> 3;
    return COMBINE(lo, hi);
}

static uint32_t bilerptr30(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c10 = *a10;
    uint32_t lo = (LO_PAIR(c00) + 3 * LO_PAIR(c10)) >> 2;
    uint32_t hi = (HI_PAIR(c00) + 3 * HI_PAIR(c10)) >> 2;
    return COMBINE(lo, hi);
}
static uint32_t bilerptr31(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (9 * LO_PAIR(c10) + 3 * (LO_PAIR(c00) + LO_PAIR(c11)) + LO_PAIR(c01)) >> 4;
    uint32_t hi = (9 * HI_PAIR(c10) + 3 * (HI_PAIR(c00) + HI_PAIR(c11)) + HI_PAIR(c01)) >> 4;
    return COMBINE(lo, hi);
}
static uint32_t bilerptr32(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (3 * (LO_PAIR(c10) + LO_PAIR(c11)) + LO_PAIR(c00) + LO_PAIR(c01)) >> 3;
    uint32_t hi = (3 * (HI_PAIR(c10) + HI_PAIR(c11)) + HI_PAIR(c00) + HI_PAIR(c01)) >> 3;
    return COMBINE(lo, hi);
}
static uint32_t bilerptr33(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (9 * LO_PAIR(c11) + 3 * (LO_PAIR(c01) + LO_PAIR(c10)) + LO_PAIR(c00)) >> 4;
    uint32_t hi = (9 * HI_PAIR(c11) + 3 * (HI_PAIR(c01) + HI_PAIR(c10)) + HI_PAIR(c00)) >> 4;
    return COMBINE(lo, hi);
}

static const SkFilterPtrProc gBilerpPtrProcs[4 * 4] = {
    bilerptr00, bilerptr01, bilerptr02, bilerptr03,
    bilerptr10, bilerptr11, bilerptr12, bilerptr13,
    bilerptr20, bilerptr21, bilerptr22, bilerptr23,
    bilerptr30, bilerptr31, bilerptr32, bilerptr33
};

const SkFilterPtrProc* SkGetBilinearFilterPtrProcTable()
{
    return gBilerpPtrProcs;
}

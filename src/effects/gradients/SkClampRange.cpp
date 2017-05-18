/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkClampRange.h"
#include "SkMathPriv.h"

static int SkCLZ64(uint64_t value) {
    int count = 0;
    if (value >> 32) {
        value >>= 32;
    } else {
        count += 32;
    }
    return count + SkCLZ(SkToU32(value));
}

static bool sk_64_smul_check(int64_t count, int64_t dx, int64_t* result) {
    // Do it the slow way until we have some assembly.
    if (dx == std::numeric_limits<int64_t>::min()) {
        return false; // SkTAbs overflow
    }

    SkASSERT(count >= 0);
    uint64_t ucount = static_cast<uint64_t>(count);
    uint64_t udx = static_cast<uint64_t>(SkTAbs(dx));
    int zeros = SkCLZ64(ucount) + SkCLZ64(udx);
    // this is a conservative check: it may return false when in fact it would not have overflowed.
    // Hackers Delight uses 34 as its convervative check, but that is for 32x32 multiplies.
    // Since we are looking at 64x64 muls, we add 32 to the check.
    if (zeros < (32 + 34)) {
        return false;
    }
    *result = count * dx;
    return true;
}

static bool sk_64_sadd_check(int64_t a, int64_t b, int64_t* result) {
    if (a > 0) {
        if (b > std::numeric_limits<int64_t>::max() - a) {
            return false;
        }
    } else {
        if (b < std::numeric_limits<int64_t>::min() - a) {
            return false;
        }
    }

    *result = a + b;
    return true;
}


/*
 *  returns [0..count] for the number of steps (<= count) for which x0 <= edge
 *  given each step is followed by x0 += dx
 */
static int chop(int64_t x0, SkGradFixed edge, int64_t x1, int64_t dx, int count) {
    SkASSERT(dx > 0);
    SkASSERT(count >= 0);

    if (x0 >= edge) {
        return 0;
    }
    if (x1 <= edge) {
        return count;
    }
    int64_t n = (edge - x0 + dx - 1) / dx;
    SkASSERT(n >= 0);
    SkASSERT(n <= count);
    return (int)n;
}

void SkClampRange::initFor1(SkGradFixed fx) {
    fCount0 = fCount1 = fCount2 = 0;
    if (fx <= 0) {
        fCount0 = 1;
    } else if (fx < kFracMax_SkGradFixed) {
        fCount1 = 1;
        fFx1 = fx;
    } else {
        fCount2 = 1;
    }
}

void SkClampRange::init(SkGradFixed fx0, SkGradFixed dx0, int count, int v0, int v1) {
    SkASSERT(count > 0);

    fV0 = v0;
    fV1 = v1;

    // special case 1 == count, as it is slightly common for skia
    // and avoids us ever calling divide or 64bit multiply
    if (1 == count) {
        this->initFor1(fx0);
        return;
    }

    int64_t fx = fx0;
    int64_t dx = dx0;

    // start with ex equal to the last computed value
    int64_t count_times_dx, ex;
    if (!sk_64_smul_check(count - 1, dx, &count_times_dx) ||
        !sk_64_sadd_check(fx, count_times_dx, &ex)) {
        // we can't represent the computed end in 32.32, so just draw something (first color)
        fCount1 = fCount2 = 0;
        fCount0 = count;
        return;
    }

    if ((uint64_t)(fx | ex) <= kFracMax_SkGradFixed) {
        fCount0 = fCount2 = 0;
        fCount1 = count;
        fFx1 = fx0;
        return;
    }
    if (fx <= 0 && ex <= 0) {
        fCount1 = fCount2 = 0;
        fCount0 = count;
        return;
    }
    if (fx >= kFracMax_SkGradFixed && ex >= kFracMax_SkGradFixed) {
        fCount0 = fCount1 = 0;
        fCount2 = count;
        return;
    }

    // now make ex be 1 past the last computed value
    ex += dx;

    bool doSwap = dx < 0;

    if (doSwap) {
        ex -= dx;
        fx -= dx;
        SkTSwap(fx, ex);
        dx = -dx;
    }


    fCount0 = chop(fx, 0, ex, dx, count);
    SkASSERT(fCount0 >= 0);
    SkASSERT(fCount0 <= count);
    count -= fCount0;
    fx += fCount0 * dx;
    SkASSERT(fx >= 0);
    SkASSERT(fCount0 == 0 || (fx - dx) < 0);
    fCount1 = chop(fx, kFracMax_SkGradFixed, ex, dx, count);
    SkASSERT(fCount1 >= 0);
    SkASSERT(fCount1 <= count);
    count -= fCount1;
    fCount2 = count;

#ifdef SK_DEBUG
    fx += fCount1 * dx;
    SkASSERT(fx <= ex);
    if (fCount2 > 0) {
        SkASSERT(fx >= kFracMax_SkGradFixed);
        if (fCount1 > 0) {
            SkASSERT(fx - dx < kFracMax_SkGradFixed);
        }
    }
#endif

    if (doSwap) {
        SkTSwap(fCount0, fCount2);
        SkTSwap(fV0, fV1);
        dx = -dx;
    }

    if (fCount1 > 0) {
        fFx1 = fx0 + fCount0 * dx;
    }
}

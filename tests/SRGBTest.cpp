/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSRGB.h"
#include "SkTypes.h"
#include "Test.h"
#include <math.h>

static uint8_t linear_to_srgb(float l) {
    // Round float to int, truncate that to uint8_t.
    return (uint8_t)Sk4f_round( sk_linear_to_srgb(Sk4f{l}) )[0];
}

DEF_TEST(sk_linear_to_srgb, r) {
    // Should map 0 -> 0 and 1 -> 1.
    REPORTER_ASSERT(r,   0 == linear_to_srgb(0.0f));
    REPORTER_ASSERT(r, 255 == linear_to_srgb(1.0f));

    // Should be monotonic between 0 and 1.
    // We don't bother checking denorm values.
    int tolerated_regressions = 0;
#if defined(SK_ARM_HAS_NEON)
    // Values around 0.166016 are usually 72 but drop briefly (41 floats) down to 71.
    tolerated_regressions = 1;
#endif
    uint8_t prev = 0;
    for (float f = FLT_MIN; f <= 1.0f; ) {
        uint8_t srgb = linear_to_srgb(f);

        REPORTER_ASSERT(r, srgb >= prev || tolerated_regressions > 0);
        if (srgb < prev) { tolerated_regressions--; }
        prev = srgb;

        union { float flt; uint32_t bits; } pun = { f };
        pun.bits++;
        f = pun.flt;
    }
}

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
    return (uint8_t)sk_linear_to_srgb(Sk4f{l})[0];
}

DEF_TEST(sk_linear_to_srgb, r) {
    // All bytes should round trip.
    for (int i = 0; i < 256; i++) {
        int actual = linear_to_srgb(sk_linear_from_srgb[i]);
        if (i != actual) {
            ERRORF(r, "%d -> %d\n", i, actual);
        }
    }

    // Should be monotonic between 0 and 1.
    uint8_t prev = 0;
    for (float f = FLT_MIN; f <= 1.0f; ) {  // We don't bother checking denorm values.
        uint8_t srgb = linear_to_srgb(f);

        REPORTER_ASSERT(r, srgb >= prev);
        prev = srgb;

        union { float flt; uint32_t bits; } pun = { f };
        pun.bits++;
        SkDEBUGCODE(pun.bits += 127);
        f = pun.flt;
    }
}

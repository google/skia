/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkTypes.h"
#include "include/private/SkColorData.h"
#include "tests/Test.h"

#define ASSERT(expr) REPORTER_ASSERT(r, expr)

DEF_TEST(Splay, r) {
    const SkPMColor color = 0xA1B2C3D4;

    uint32_t ag, rb;
    SkSplay(color, &ag, &rb);
    ASSERT(ag == 0x00A100C3);
    ASSERT(rb == 0x00B200D4);
    ASSERT(SkUnsplay(ag << 8, rb << 8) == color);

    const uint64_t agrb = SkSplay(color);
    ASSERT(agrb == 0x00A100C300B200D4ULL);
    ASSERT(SkUnsplay(agrb<<8) == color);
}

DEF_TEST(FourByteInterp, r) {
    const SkPMColor src = 0xAB998877, dst = 0x66334455;
    for (unsigned scale = 0; scale <= 256; scale++) {
        ASSERT(SkFourByteInterp256(src, dst, scale) == SkFastFourByteInterp256(src, dst, scale));
    }

    for (unsigned scale = 0; scale < 256; scale++) {
        // SkFourByteInterp and SkFastFourByteInterp convert from [0, 255] to [0, 256] differently.
        // In particular, slow may end up a little too high (weirdly, fast is more accurate).
        const SkPMColor slow = SkFourByteInterp(src, dst, scale);
        const SkPMColor fast = SkFastFourByteInterp(src, dst, scale);

        const int deltaA = SkGetPackedA32(slow) - SkGetPackedA32(fast);
        const int deltaR = SkGetPackedR32(slow) - SkGetPackedR32(fast);
        const int deltaG = SkGetPackedG32(slow) - SkGetPackedG32(fast);
        const int deltaB = SkGetPackedB32(slow) - SkGetPackedB32(fast);

        ASSERT(deltaA == 0 || deltaA == 1);
        ASSERT(deltaR == 0 || deltaR == 1);
        ASSERT(deltaG == 0 || deltaG == 1);
        ASSERT(deltaB == 0 || deltaB == 1);
    }
}

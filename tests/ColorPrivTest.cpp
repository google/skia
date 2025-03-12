/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkTypes.h"
#include "src/core/SkColorData.h"
#include "src/core/SkColorPriv.h"
#include "tests/Test.h"

#include <cstdint>

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

DEF_TEST(SkBlendARGB32_SameOpaqueSrcAndDstWithAnyAlpha_ProducesSameColor, r) {
    SkColor colors[] = {
            SK_ColorWHITE, SK_ColorBLACK, SK_ColorGRAY,
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
            SK_ColorYELLOW, SK_ColorCYAN, SK_ColorMAGENTA,
            SkColorSetARGB(255, 255 / 4, 255 / 3, 255 / 2),
             // https://bugzilla.mozilla.org/show_bug.cgi?id=1200684 and https://codereview.chromium.org/2097883002
            0xFF2E3338,
    };

    for (SkColor c: colors) {
        SkASSERTF(SkColorGetA(c) == 0xFF, "this is only true for opaque colors");

        const SkPMColor srcAndDst = SkPreMultiplyColor(c);

        for (unsigned scale = 0; scale < 256; scale++) {
            U8CPU alpha = static_cast<U8CPU>(scale);

            SkPMColor output = SkBlendARGB32(srcAndDst, srcAndDst, alpha);
            REPORTER_ASSERT(r,
                            srcAndDst == output,
                            "SkBlendARGB32(0x%08x, 0x%08x, %02x) = %08x instead of the original",
                            srcAndDst, srcAndDst, alpha, output);
        }
    }
}

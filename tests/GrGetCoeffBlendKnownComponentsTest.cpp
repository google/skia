/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrBlend.h"
#include "SkGr.h"
#include "SkGrPriv.h"
#include "SkRandom.h"

static GrColor make_baseline_color(GrColor src, GrColor dst, const SkXfermode* xm) {
    SkPMColor skSrc = GrColorToSkPMColor(src);
    SkPMColor skDst = GrColorToSkPMColor(dst);
    if (xm) {
        xm->xfer32(&skDst, &skSrc, 1, nullptr);
    } else {
        // null means src-over
        skDst = SkPMSrcOver(skSrc, skDst);
    }
    return SkPMColorToGrColor(skDst);
}

DEF_TEST(GrGetCoeffBlendKnownComponents, reporter) {
    SkRandom random;
    for (int i = 0; i < SkXfermode::kLastCoeffMode; ++i) {
        SkXfermode::Mode mode = (SkXfermode::Mode)i;
        auto xm(SkXfermode::Make(mode));
        SkXfermode::Coeff srcCoeff, dstCoeff;
        SkAssertResult(SkXfermode::ModeAsCoeff(mode, &srcCoeff, &dstCoeff));
        for (int j = 0; j < 1000; ++j) {
            GrColor src = GrPremulColor(random.nextU());
            GrColor dst = GrPremulColor(random.nextU());
            GrColor outColor;
            GrColorComponentFlags outFlags;
            GrGetCoeffBlendKnownComponents(SkXfermodeCoeffToGrBlendCoeff(srcCoeff),
                                           SkXfermodeCoeffToGrBlendCoeff(dstCoeff),
                                           src, kRGBA_GrColorComponentFlags,
                                           dst, kRGBA_GrColorComponentFlags,
                                           &outColor, &outFlags);
            GrColor baselineColor = make_baseline_color(src, dst, xm.get());
            if (SkAbs32(GrColorUnpackA(baselineColor) - GrColorUnpackA(outColor)) > 1 ||
                SkAbs32(GrColorUnpackR(baselineColor) - GrColorUnpackR(outColor)) > 1 ||
                SkAbs32(GrColorUnpackG(baselineColor) - GrColorUnpackG(outColor)) > 1 ||
                SkAbs32(GrColorUnpackB(baselineColor) - GrColorUnpackB(outColor)) > 1) {
                ERRORF(reporter, "Blended color is 0x%08x, expected 0x%08x", outColor,
                       baselineColor);
            }
            GrColorIsPMAssert(outColor);
        }
    }
    GrColor outColor;
    GrColorComponentFlags outFlags;
    GrGetCoeffBlendKnownComponents(kZero_GrBlendCoeff, kZero_GrBlendCoeff,
                                   0xFFFFFFFF, kNone_GrColorComponentFlags,
                                   0xFFFFFFFF, kNone_GrColorComponentFlags,
                                   &outColor, &outFlags);
    REPORTER_ASSERT(reporter, GrColor_TRANSPARENT_BLACK == outColor &&
                              kRGBA_GrColorComponentFlags == outFlags);
    GrGetCoeffBlendKnownComponents(
        kOne_GrBlendCoeff, kOne_GrBlendCoeff,
        0x80FF0100, (kG_GrColorComponentFlag | kB_GrColorComponentFlag | kA_GrColorComponentFlag),
        0x7F00FFFF, (kR_GrColorComponentFlag | kG_GrColorComponentFlag | kA_GrColorComponentFlag),
        &outColor, &outFlags);
    REPORTER_ASSERT(reporter, GrColor_WHITE == outColor && kRGBA_GrColorComponentFlags == outFlags);

    GrGetCoeffBlendKnownComponents(
        kOne_GrBlendCoeff, kISA_GrBlendCoeff,
        0x0000000, kRGBA_GrColorComponentFlags,
        0x80010203, kRGBA_GrColorComponentFlags,
        &outColor, &outFlags);
    REPORTER_ASSERT(reporter, 0x80010203 == outColor && kRGBA_GrColorComponentFlags == outFlags);

    GrGetCoeffBlendKnownComponents(kZero_GrBlendCoeff, kISA_GrBlendCoeff,
                                   0x0000000, kA_GrColorComponentFlag,
                                   0x80010203, kRGBA_GrColorComponentFlags,
                                   &outColor, &outFlags);
    REPORTER_ASSERT(reporter, 0x80010203 == outColor && kRGBA_GrColorComponentFlags == outFlags);

    GrGetCoeffBlendKnownComponents(
        kIDC_GrBlendCoeff, kSC_GrBlendCoeff,
        0x0, kNone_GrColorComponentFlags,
        0x0, kRGBA_GrColorComponentFlags,
        &outColor, &outFlags);
    REPORTER_ASSERT(reporter, kNone_GrColorComponentFlags == outFlags);

    GrGetCoeffBlendKnownComponents(
        kOne_GrBlendCoeff, kISA_GrBlendCoeff,
        0xFF808080, (kG_GrColorComponentFlag | kB_GrColorComponentFlag | kA_GrColorComponentFlag),
        0xFF606060, kRGBA_GrColorComponentFlags,
        &outColor, &outFlags);
    REPORTER_ASSERT(reporter,
                    (kG_GrColorComponentFlag | kB_GrColorComponentFlag | kA_GrColorComponentFlag) == outFlags &&
                    (outColor & 0xFFFFFF00) == 0xFF808000);
}

#endif

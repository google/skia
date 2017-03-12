/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "SkGr.h"

static GrColor filterColor(const GrColor& color, uint32_t flags)  {
    uint32_t mask = 0;
    if (flags & kR_GrColorComponentFlag) {
        mask = 0xFF << GrColor_SHIFT_R;
    }
    if (flags & kG_GrColorComponentFlag) {
        mask |= 0xFF << GrColor_SHIFT_G;
    }
    if (flags & kB_GrColorComponentFlag) {
        mask |= 0xFF << GrColor_SHIFT_B;
    }
    if (flags & kA_GrColorComponentFlag) {
        mask |= 0xFF << GrColor_SHIFT_A;
    }
    return color & mask;
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(GpuColorFilter, reporter, ctxInfo) {
    struct GetConstantComponentTestCase {
        // "Shape drawn with"
        uint32_t inputComponents; // "rgb of", "red of", "alpha of", ...
        GrColor inputColor;       // "[color]"

        SkColor filterColor;      // "with filter color [color]"
        SkBlendMode filterMode; // "in mode [mode]"

        // "produces"
        uint32_t outputComponents; // "rgb of", "red of", "alpha of", ...
        GrColor outputColor;       // "[color]"
    };

    // Shorthands.
    enum {
        kR = kR_GrColorComponentFlag,
        kG = kG_GrColorComponentFlag,
        kB = kB_GrColorComponentFlag,
        kA = kA_GrColorComponentFlag,
        kRGB = kRGB_GrColorComponentFlags,
        kRGBA = kRGBA_GrColorComponentFlags
    };

    // Note: below, SkColors are non-premultiplied, where as GrColors are premultiplied.

    const SkColor c1 = SkColorSetARGB(200, 200, 200, 200);
    const SkColor c2 = SkColorSetARGB(60, 60, 60, 60);
    const GrColor gr_c1 = SkColorToPremulGrColor(c1);
    const GrColor gr_c2 = SkColorToPremulGrColor(c2);

    const GrColor gr_black = GrColorPackA4(0);
    const GrColor gr_white = GrColorPackA4(255);
    const GrColor gr_whiteTrans = GrColorPackA4(128);

    GetConstantComponentTestCase filterTests[] = {
        // A color filtered with Clear produces black.
        { kRGBA, gr_white, SK_ColorBLACK, SkBlendMode::kClear, kRGBA, gr_black },
        { kRGBA, gr_c1,    SK_ColorWHITE, SkBlendMode::kClear, kRGBA, gr_black },
        { kR,    gr_white, c1,            SkBlendMode::kClear, kRGBA, gr_black },

        // A color filtered with a color in mode Src, produces the filter color.
        { kRGBA, gr_c2, c1, SkBlendMode::kSrc, kRGBA, gr_c1 },
        { kA,    gr_c1, c1, SkBlendMode::kSrc, kRGBA, gr_c1 },

        // A color filtered with SrcOver produces a color.
        { kRGBA, gr_whiteTrans, SkColorSetARGB(128, 200, 200, 200), SkBlendMode::kSrcOver, kRGBA, GrColorPackRGBA(164, 164, 164, 192)},
        // An unknown color with known alpha filtered with SrcOver produces an unknown color with known alpha.
        { kA   , gr_whiteTrans, SkColorSetARGB(128, 200, 200, 200), SkBlendMode::kSrcOver, kA   , GrColorPackRGBA(0, 0, 0, 192)},
        // A color with unknown alpha filtered with SrcOver produces a color with unknown alpha.
        { kRGB , gr_whiteTrans, SkColorSetARGB(128, 200, 200, 200), SkBlendMode::kSrcOver, kRGB, GrColorPackRGBA(164, 164, 164, 0)},

        // A color filtered with DstOver produces a color.
        { kRGBA, gr_whiteTrans, SkColorSetARGB(128, 200, 200, 200), SkBlendMode::kDstOver, kRGBA, GrColorPackRGBA(178, 178, 178, 192)},
        // An unknown color with known alpha filtered with DstOver produces an unknown color with known alpha.
        { kA   , gr_whiteTrans, SkColorSetARGB(128, 200, 200, 200), SkBlendMode::kDstOver, kA   , GrColorPackRGBA(0, 0, 0, 192)},
        // A color with unknown alpha filtered with DstOver produces an unknown color.
        { kRGB , gr_whiteTrans, SkColorSetARGB(128, 200, 200, 200), SkBlendMode::kDstOver, 0    , gr_black},

        // An unknown color with known alpha and red component filtered with Multiply produces an unknown color with known red and alpha.
        { kR|kA , gr_whiteTrans, SkColorSetARGB(128, 200, 200, 200), SkBlendMode::kModulate, kR|kA, GrColorPackRGBA(50, 0, 0, 64) }
    };

    GrPaint paint;
    for (size_t i = 0; i < SK_ARRAY_COUNT(filterTests); ++i) {
        const GetConstantComponentTestCase& test = filterTests[i];
        auto cf(SkColorFilter::MakeModeFilter(test.filterColor, test.filterMode));
        // TODO: Test other color spaces
        sk_sp<GrFragmentProcessor> fp(cf->asFragmentProcessor(ctxInfo.grContext(), nullptr));
        REPORTER_ASSERT(reporter, fp);
        GrInvariantOutput inout(test.inputColor,
                                static_cast<GrColorComponentFlags>(test.inputComponents));
        fp->computeInvariantOutput(&inout);
        REPORTER_ASSERT(reporter, filterColor(inout.color(), inout.validFlags()) ==
                                  test.outputColor);
        REPORTER_ASSERT(reporter, test.outputComponents == inout.validFlags());
    }
}

#endif

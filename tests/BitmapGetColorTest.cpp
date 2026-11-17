/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "tests/Test.h"

#include <array>
#include <cstddef>
#include <cstdint>

DEF_TEST(GetColor, reporter) {
    struct Rec {
        SkColorType fColorType;
        SkColor     fInColor;
        SkColor     fOutColor;
    };

    static constexpr auto gRec = std::to_array<Rec>({
            // todo: add some tests that involve alpha, so we exercise the
            // unpremultiply aspect of getColor()
            Rec{   kAlpha_8_SkColorType,   0xFF000000,     0xFF000000  },
            Rec{   kAlpha_8_SkColorType,   0,              0           },
            Rec{   kRGB_565_SkColorType,   0xFF00FF00,     0xFF00FF00  },
            Rec{   kRGB_565_SkColorType,   0xFFFF00FF,     0xFFFF00FF  },
            Rec{   kRGB_888x_SkColorType,  0xFF224466,     0xFF224466  },
            Rec{   kRGB_888x_SkColorType,  0,              0xFF000000  },
            Rec{   kBGR_888x_SkColorType,  0xFF224466,     0xFF224466  },
            Rec{   kBGR_888x_SkColorType,  0,              0xFF000000  },
            Rec{   kN32_SkColorType,       0xFFFFFFFF,     0xFFFFFFFF  },
            Rec{   kN32_SkColorType,       0,              0           },
            Rec{   kN32_SkColorType,       0xFF224466,     0xFF224466  },
    });

    // specify an area that doesn't touch (0,0) and may extend beyond the
    // bitmap bounds (to test that we catch that in eraseArea
    const SkColor initColor = 0xFF0000FF;
    const SkIRect area = { 1, 1, 3, 3 };

    for (size_t i = 0; i < std::size(gRec); i++) {
        SkImageInfo info = SkImageInfo::Make(2, 2, gRec[i].fColorType,
                                             kPremul_SkAlphaType);
        SkBitmap bm;
        uint32_t storage[4];
        bm.installPixels(info, storage, info.minRowBytes());

        bm.eraseColor(initColor);
        bm.eraseArea(area, gRec[i].fInColor);

        SkColor c = bm.getColor(1, 1);
        REPORTER_ASSERT(reporter, c == gRec[i].fOutColor);
    }

    // Verify that BGR_888x stores B,G,R,x in memory and ignores the 4th byte on read.
    {
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(1, 1, kBGR_888x_SkColorType, kOpaque_SkAlphaType));
        uint8_t* p = static_cast<uint8_t*>(bm.getPixels());
        p[0] = 0x33;  // B
        p[1] = 0x22;  // G
        p[2] = 0x11;  // R
        p[3] = 0x00;  // x (ignored)
        constexpr SkColor kExpected = SkColorSetARGB(0xFF, 0x11, 0x22, 0x33);
        REPORTER_ASSERT(reporter, bm.getColor(0, 0) == kExpected);
        REPORTER_ASSERT(reporter, bm.getColor4f(0, 0) == SkColor4f::FromColor(kExpected));
    }

    // Verify that getColor() and getColor4f() agree on R/B channel order for 1010102/101010x.
    for (SkColorType ct : {kRGBA_1010102_SkColorType,
                           kBGRA_1010102_SkColorType,
                           kRGB_101010x_SkColorType,
                           kBGR_101010x_SkColorType}) {
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(1, 1, ct, kUnpremul_SkAlphaType));
        bm.eraseColor(SK_ColorRED);
        REPORTER_ASSERT(reporter, bm.getColor(0, 0) == SK_ColorRED);
        REPORTER_ASSERT(reporter, bm.getColor4f(0, 0) == SkColors::kRed);
    }
}

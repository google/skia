/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "src/core/SkColorPriv.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkScalerContext.h"
#include "tests/Test.h"

#include <array>
#include <cstdint>
#include <initializer_list>

namespace {
class PathMaskScalerContext : public SkScalerContext {
public:
    using SkScalerContext::GenerateImageFromPath;
};
}  // namespace

DEF_TEST(ScalerContext_LCDMaskPreBlend, reporter) {
    constexpr int kSize = 8;
    // Distinct constant LUTs make channel selection observable after RGB565 packing.
    std::array<uint8_t, 256> identity, red, green, blue;
    for (int i = 0; i < 256; ++i) {
        identity[i] = i;
        red[i] = 64;
        green[i] = 128;
        blue[i] = 192;
    }

    SkMaskGamma::PreBlend identityBlend;
    identityBlend.fR = identityBlend.fG = identityBlend.fB = identity.data();
    SkMaskGamma::PreBlend colorBlend;
    colorBlend.fR = red.data();
    colorBlend.fG = green.data();
    colorBlend.fB = blue.data();

    const SkPath path = SkPath::Rect(SkRect::MakeLTRB(2.25f, 2.25f, 5.75f, 5.75f));
    for (bool vertical : {false, true}) {
        for (bool bgr : {false, true}) {
            std::array<uint16_t, kSize * kSize> raw, unchanged, corrected;
            auto render = [&](auto& pixels, const SkMaskGamma::PreBlend& blend) {
                SkMaskBuilder mask(reinterpret_cast<uint8_t*>(pixels.data()),
                                   SkIRect::MakeWH(kSize, kSize),
                                   kSize * sizeof(uint16_t),
                                   SkMask::kLCD16_Format);
                PathMaskScalerContext::GenerateImageFromPath(
                        mask, path, blend, bgr, vertical, false, false);
            };
            render(raw, SkMaskGamma::PreBlend());
            render(unchanged, identityBlend);
            render(corrected, colorBlend);
            REPORTER_ASSERT(reporter, raw == unchanged);
            REPORTER_ASSERT(reporter, raw != corrected);
            for (uint16_t pixel : corrected) {
                REPORTER_ASSERT(reporter, pixel == SkPack888ToRGB16(64, 128, 192));
            }
        }
    }
}

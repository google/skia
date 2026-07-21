/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GradientBitmap.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/private/SkAssert.h"
#include "src/core/SkHalf.h"

#include <cmath>

namespace skgpu {

bool EncodeGradientStopToHalf(float offset, float* mantissa, float* exponent) {
    int exp;
    float mant = std::frexp(offset, &exp);

    SkHalf halfE = SkFloatToHalf(static_cast<float>(exp));
    if (static_cast<int>(SkHalfToFloat(halfE)) != exp) {
        return false;
    }

#if defined(SK_DEBUG)
    SkHalf halfM = SkFloatToHalf(mant);
    float restored = std::ldexp(SkHalfToFloat(halfM), exp);
    SkASSERT(std::abs(restored - offset) < 0.001f);
#endif

    *mantissa = mant;
    *exponent = static_cast<float>(exp);
    return true;
}

SkBitmap CreateGradientColorAndOffsetBitmap(int numStops,
                                            const SkPMColor4f* colors,
                                            const float* offsets) {
    SkBitmap colorsAndOffsetsBitmap;
    if (!colorsAndOffsetsBitmap.tryAllocPixels(
                SkImageInfo::Make(numStops, 2, kRGBA_F16_SkColorType, kPremul_SkAlphaType))) {
        return {};
    }

    for (int i = 0; i < numStops; i++) {
        // TODO: there should be a way to directly set a premul pixel in a bitmap with
        // a premul color.
        SkColor4f unpremulColor = colors[i].unpremul();
        colorsAndOffsetsBitmap.erase(unpremulColor, SkIRect::MakeXYWH(i, 0, 1, 1));

        float offset = offsets ? offsets[i] : SkIntToFloat(i) / (numStops - 1);
        SkASSERT(offset >= 0.0f && offset <= 1.0f);

        float mantissa, exponent;
        if (!EncodeGradientStopToHalf(offset, &mantissa, &exponent)) {
            return {};
        }

        // TODO: we're only using 2 of the f16s here. The encoding could be altered to better
        // preserve precision. This encoding yields < 0.001f error for 2^20 evenly spaced stops.
        colorsAndOffsetsBitmap.erase(SkColor4f{mantissa, exponent, 0, 1},
                                     SkIRect::MakeXYWH(i, 1, 1, 1));
    }

    return colorsAndOffsetsBitmap;
}

}  // namespace skgpu

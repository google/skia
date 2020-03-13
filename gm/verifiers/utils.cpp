/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/private/SkHalf.h"

namespace skiagm {
namespace verifiers {
namespace utils {

SkColor4f getColor(const SkBitmap& bmp, int x, int y) {
    // Check for expected pixel format (expected to be same as GMVerifier::VerifierColorInfo())
    SkASSERT(bmp.colorType() == kRGBA_F16_SkColorType);
    SkASSERT(bmp.alphaType() == kPremul_SkAlphaType);
    SkASSERT(SkColorSpace::Equals(
            bmp.colorSpace(),
            SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020).get()));

    const bool needsUnpremul = kPremul_SkAlphaType == bmp.alphaType();
    const SkPixmap& pm = bmp.pixmap();
    const uint16_t* pixAddr = pm.addrF16(x, y);

    Sk4f p4 = SkHalfToFloat_finite_ftz(*pixAddr);
    if (p4[3] != 0 && needsUnpremul) {
        const float inva = 1 / p4[3];
        p4 = p4 * Sk4f(inva, inva, inva, 1);
    }

    SkColor4f color;
    p4.store(color.vec());
    return color;
}

float maxChannelDiff(SkColor4f a, SkColor4f b) {
    const Sk4f fa = Sk4f::Load(a.vec()), fb = Sk4f::Load(b.vec());
    const Sk4f delta = fa - fb;
    const Sk4f absDelta = delta.abs();
    return absDelta.max();
}

bool colorInNeighborhood(const SkBitmap& bitmap, int x, int y, SkColor4f color, int n, float dist) {
    const SkIRect bounds = bitmap.bounds();
    const int minX = std::max(x - n, bounds.fLeft), maxX = std::min(x + n, bounds.fRight - 1),
              minY = std::max(y - n, bounds.fTop), maxY = std::min(y + n, bounds.fBottom - 1);

    for (int i = minY; i <= maxY; i++) {
        for (int j = minX; j <= maxX; j++) {
            if (maxChannelDiff(color, utils::getColor(bitmap, x, y)) <= dist) {
                return true;
            }
        }
    }

    return false;
}

}  // namespace utils
}  // namespace verifiers
}  // namespace skiagm

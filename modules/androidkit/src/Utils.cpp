/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/androidkit/src/Utils.h"

namespace androidkit {
namespace utils {

SkSamplingOptions SamplingOptions(jint desc, jfloat coeffB, jfloat coeffC) {
    if (desc & 0x01) {
        return SkSamplingOptions(SkCubicResampler{coeffB, coeffC});
    }

    const auto fm = static_cast<SkFilterMode>((desc >> 1) & 0x01);
    SkASSERT(fm <= SkFilterMode::kLast);
    const auto mm = static_cast<SkMipmapMode>((desc >> 2) & 0x03);
    SkASSERT(mm <= SkMipmapMode::kLast);

    return SkSamplingOptions(fm, mm);
}

SkTileMode TileMode(jint tm) {
    // to catch Skia API changes
    static_assert(static_cast<int>(SkTileMode::kClamp ) == 0);
    static_assert(static_cast<int>(SkTileMode::kRepeat) == 1);
    static_assert(static_cast<int>(SkTileMode::kMirror) == 2);
    static_assert(static_cast<int>(SkTileMode::kDecal ) == 3);

    return static_cast<SkTileMode>(tm);
}

SkBlendMode BlendMode(jint bm) {
    // catch API changes
    static_assert(0 == static_cast<int>(SkBlendMode::kClear));
    static_assert(1 == static_cast<int>(SkBlendMode::kSrc));
    static_assert(2 == static_cast<int>(SkBlendMode::kDst));
    static_assert(3 == static_cast<int>(SkBlendMode::kSrcOver));
    static_assert(4 == static_cast<int>(SkBlendMode::kDstOver));
    static_assert(5 == static_cast<int>(SkBlendMode::kSrcIn));
    static_assert(6 == static_cast<int>(SkBlendMode::kDstIn));
    static_assert(7 == static_cast<int>(SkBlendMode::kSrcOut));
    static_assert(8 == static_cast<int>(SkBlendMode::kDstOut));
    static_assert(9 == static_cast<int>(SkBlendMode::kSrcATop));
    static_assert(10 == static_cast<int>(SkBlendMode::kDstATop));
    static_assert(11 == static_cast<int>(SkBlendMode::kXor));
    static_assert(12 == static_cast<int>(SkBlendMode::kPlus));
    static_assert(13 == static_cast<int>(SkBlendMode::kModulate));
    static_assert(14 == static_cast<int>(SkBlendMode::kScreen));

    static_assert(15 == static_cast<int>(SkBlendMode::kOverlay));
    static_assert(16 == static_cast<int>(SkBlendMode::kDarken));
    static_assert(17 == static_cast<int>(SkBlendMode::kLighten));
    static_assert(18 == static_cast<int>(SkBlendMode::kColorDodge));
    static_assert(19 == static_cast<int>(SkBlendMode::kColorBurn));
    static_assert(20 == static_cast<int>(SkBlendMode::kHardLight));
    static_assert(21 == static_cast<int>(SkBlendMode::kSoftLight));
    static_assert(22 == static_cast<int>(SkBlendMode::kDifference));
    static_assert(23 == static_cast<int>(SkBlendMode::kExclusion));
    static_assert(24 == static_cast<int>(SkBlendMode::kMultiply));

    static_assert(25 == static_cast<int>(SkBlendMode::kHue));
    static_assert(26 == static_cast<int>(SkBlendMode::kSaturation));
    static_assert(27 == static_cast<int>(SkBlendMode::kColor));
    static_assert(28 == static_cast<int>(SkBlendMode::kLuminosity));


    return static_cast<SkBlendMode>(bm);
}

} // namespace utils
} // namespace androidkit

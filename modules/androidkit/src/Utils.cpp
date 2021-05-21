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

} // namespace utils
} // namespace androidkit

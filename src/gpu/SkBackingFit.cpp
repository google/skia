/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/SkBackingFit.h"

#include "include/private/base/SkMath.h"
#include "src/base/SkMathPriv.h"

#include <algorithm>

namespace skgpu {

SkISize GetApproxSize(SkISize size) {
    // Map 'value' to a larger multiple of 2. Values <= 'kMagicTol' will pop up to
    // the next power of 2. Those above 'kMagicTol' will only go up half the floor power of 2.
    auto adjust = [](int value) {
        constexpr int kMinApproxSize = 16;
        constexpr int kMagicTol = 1024;

        value = std::max(kMinApproxSize, value);

        if (SkIsPow2(value)) {
            return value;
        }

        int ceilPow2 = SkNextPow2(value);
        if (value <= kMagicTol) {
            return ceilPow2;
        }

        int floorPow2 = ceilPow2 >> 1;
        int mid = floorPow2 + (floorPow2 >> 1);

        if (value <= mid) {
            return mid;
        }
        return ceilPow2;
    };

    return {adjust(size.width()), adjust(size.height())};
}

}  // namespace skgpu

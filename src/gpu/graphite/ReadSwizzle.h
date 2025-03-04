/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ReadSwizzle_DEFINED
#define skgpu_graphite_ReadSwizzle_DEFINED

#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/Log.h"

namespace skgpu::graphite {
/**
 * Enumerate the few possible read and write swizzle options for smaller storage.
*/
enum class ReadSwizzle {
    kRGBA, // Default
    kRGB1,
    kRRR1,
    kBGRA,
    k000R,
};

inline skgpu::graphite::ReadSwizzle SwizzleClassToReadEnum(const skgpu::Swizzle& swizzle) {
    if (swizzle == skgpu::Swizzle::RGBA()) {
        return skgpu::graphite::ReadSwizzle::kRGBA;
    } else if (swizzle == skgpu::Swizzle::RGB1()) {
        return skgpu::graphite::ReadSwizzle::kRGB1;
    } else if (swizzle == skgpu::Swizzle("rrr1")) {
        return skgpu::graphite::ReadSwizzle::kRRR1;
    } else if (swizzle == skgpu::Swizzle::BGRA()) {
        return skgpu::graphite::ReadSwizzle::kBGRA;
    } else if (swizzle == skgpu::Swizzle("000r")) {
        return skgpu::graphite::ReadSwizzle::k000R;
    } else {
        SKGPU_LOG_W("%s is an unsupported read swizzle. Defaulting to RGBA.\n",
                    swizzle.asString().data());
        return skgpu::graphite::ReadSwizzle::kRGBA;
    }
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_ReadSwizzle_DEFINED

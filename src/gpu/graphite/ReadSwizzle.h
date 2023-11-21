/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ReadSwizzle_DEFINED
#define skgpu_graphite_ReadSwizzle_DEFINED

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

} // namespace skgpu::graphite

#endif // skgpu_graphite_ReadSwizzle_DEFINED

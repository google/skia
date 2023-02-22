/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ReadWriteSwizzle_DEFINED
#define skgpu_graphite_ReadWriteSwizzle_DEFINED

namespace skgpu::graphite {
/**
 * Enumerate the few possible read and write swizzle options for smaller storage.
*/
enum class ReadSwizzle {
    kRGBA, // Default
    kRGB1,
    /* 000r is a possible read swizzle, but it is currently only expected for use with alpha-only
       color types. As a result, the swizzle is concatenated with "aaaa", resulting in "rrrr". */
    kRRRR,
    kRRR1,
    kBGRA
};

enum class WriteSwizzle {
    kRGBA, // Default
    kA000,
    kBGRA,
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ReadWriteSwizzle_DEFINED

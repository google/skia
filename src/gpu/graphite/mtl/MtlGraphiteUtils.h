/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlGraphiteTypesPriv_DEFINED
#define skgpu_graphite_MtlGraphiteTypesPriv_DEFINED

#include "include/core/SkString.h"
#include "include/ports/SkCFObject.h"
#include "src/base/SkEnumBitMask.h"

class SkStream;
class SkWStream;

///////////////////////////////////////////////////////////////////////////////

#include <TargetConditionals.h>

// We're using the MSL version as shorthand for the Metal SDK version here
#if defined(SK_BUILD_FOR_MAC)
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 130000
#define SKGPU_GRAPHITE_METAL_SDK_VERSION 300
#elif __MAC_OS_X_VERSION_MAX_ALLOWED >= 120000
#define SKGPU_GRAPHITE_METAL_SDK_VERSION 240
#elif __MAC_OS_X_VERSION_MAX_ALLOWED >= 110000
#define SKGPU_GRAPHITE_METAL_SDK_VERSION 230
#else
#error Must use at least 11.00 SDK to build Metal backend for MacOS
#endif
#else
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 160000 || __TV_OS_VERSION_MAX_ALLOWED >= 160000
#define SKGPU_GRAPHITE_METAL_SDK_VERSION 300
#elif __IPHONE_OS_VERSION_MAX_ALLOWED >= 150000 || __TV_OS_VERSION_MAX_ALLOWED >= 150000
#define SKGPU_GRAPHITE_METAL_SDK_VERSION 240
#elif __IPHONE_OS_VERSION_MAX_ALLOWED >= 140000 || __TV_OS_VERSION_MAX_ALLOWED >= 140000
#define SKGPU_GRAPHITE_METAL_SDK_VERSION 230
#else
#error Must use at least 14.00 SDK to build Metal backend for iOS
#endif
#endif

#import <Metal/Metal.h>

namespace skgpu {
class ShaderErrorHandler;
}

namespace skgpu::graphite {

class MtlSharedContext;
enum class TextureFormat : uint8_t;

sk_cfp<id<MTLLibrary>> MtlCompileShaderLibrary(const MtlSharedContext* sharedContext,
                                               std::string_view label,
                                               std::string_view msl,
                                               ShaderErrorHandler* errorHandler);

TextureFormat MTLPixelFormatToTextureFormat(MTLPixelFormat);
MTLPixelFormat TextureFormatToMTLPixelFormat(TextureFormat);

// Reproduction of "Texture capabalities by pixel format" from
//     https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
// NOTE: All graphics and compute kernels can read or sample a texture with any pixel format.
// This intentionally does not follow Skia's normal naming convention in order to better match
// the format table from the document.
enum class MTLFeatureFlag : uint8_t {
    NotAvailable = 0x0,  // Not available.
    Atomic       = 0x1,  // The GPU can use atomic operations on textures with the pixel format.
    Filter       = 0x2,  // The GPU can filter a texture with the pixel format during sampling.
    Write        = 0x4,  // The GPU can write to a texture on a per-pixel basis with the format.
    Color        = 0x8,  // The GPU can use a texture with the format as a color render target.
    Blend        = 0x10, // The GPU can blend a texture with the pixel format.
    MSAA         = 0x20, // The GPU can use a texture with the pixel format as a destination for
                          // multisample antialias (MSAA) data.
    Sparse       = 0x40, // The GPU supports sparse-texture allocations for textures in this format
    Resolve      = 0x80, // The GPU can use a texture with the pixel format as a source for
                          // multisample antialias (MSAA) resolve operations.

    // Sparse isn’t included in All since the Mac2, Metal3, and Apple2 through Apple5 family GPUs,
    // don’t support the sparse texture feature. Apple6+ references to All in the table implicitly
    // include Sparse.
    // ERRATA: The PDF claims Apple6 does not support Sparse, but the column for Apple6 adds Sparse
    // to rows that weren't already listed as "All", and
    // https://developer.apple.com/documentation/metal/managing-sparse-texture-memory
    // suggests Apple6 is when sparse texture support was added.
    All = Filter | Write | Color | Blend | MSAA | Resolve,
};
SK_MAKE_BITMASK_OPS(MTLFeatureFlag)

// This returns the static features for a format and given GPU family. It does not apply any
// MTLDevice-specific checks required for some formats.
SkEnumBitMask<MTLFeatureFlag> MTLPixelFormatSupport(MTLGPUFamily, MTLPixelFormat);

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_MtlGraphiteTypesPriv_DEFINED

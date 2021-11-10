/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlTypesPriv_DEFINED
#define skgpu_MtlTypesPriv_DEFINED

#include "experimental/graphite/include/GraphiteTypes.h"
#include "experimental/graphite/include/mtl/MtlTypes.h"

///////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__

#include <TargetConditionals.h>

// We're using the MSL version as shorthand for the Metal SDK version here
#if defined(SK_BUILD_FOR_MAC)
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 110000
#define GR_METAL_SDK_VERSION 230
#elif __MAC_OS_X_VERSION_MAX_ALLOWED >= 120000
#define GR_METAL_SDK_VERSION 240
#else
#error Must use at least 11.00 SDK to build Metal backend for MacOS
#endif
#else
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 140000 || __TV_OS_VERSION_MAX_ALLOWED >= 140000
#define GR_METAL_SDK_VERSION 230
#elif __IPHONE_OS_VERSION_MAX_ALLOWED >= 150000 || __TV_OS_VERSION_MAX_ALLOWED >= 150000
#define GR_METAL_SDK_VERSION 240
#else
#error Must use at least 14.00 SDK to build Metal backend for iOS
#endif
#endif

#endif  // __APPLE__

namespace skgpu::mtl {

struct TextureSpec {
    TextureSpec()
            : fFormat(0)
            , fUsage(0)
            , fStorageMode(0)
            , fFramebufferOnly(false) {}
    TextureSpec(const TextureInfo& info)
            : fFormat(info.fFormat)
            , fUsage(info.fUsage)
            , fStorageMode(info.fStorageMode)
            , fFramebufferOnly(info.fFramebufferOnly) {}

    bool operator==(const TextureSpec& that) const {
        return fFormat == that.fFormat &&
               fUsage == that.fUsage &&
               fStorageMode == that.fStorageMode &&
               fFramebufferOnly == that.fFramebufferOnly;
    }

    PixelFormat fFormat;
    TextureUsage fUsage;
    StorageMode fStorageMode;
    bool fFramebufferOnly;
};

TextureInfo TextureSpecToTextureInfo(const TextureSpec& mtlSpec,
                                     uint32_t sampleCount,
                                     uint32_t levelCount);

}  // namespace skgpu::mtl

#endif  // skgpu_MtlTypesPriv_DEFINED

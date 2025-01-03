/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlGraphiteTypes_DEFINED
#define skgpu_graphite_MtlGraphiteTypes_DEFINED

#if __OBJC__  // <Metal/Metal.h> only works when compiled for Objective C

#include "include/core/SkTypes.h"

#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypesUtils.h"
#include "include/private/base/SkAPI.h"

#import <CoreFoundation/CoreFoundation.h>
#import <Metal/Metal.h>
#import <TargetConditionals.h>

namespace skgpu::graphite {
struct SK_API MtlTextureInfo {
    uint32_t fSampleCount = 1;
    skgpu::Mipmapped fMipmapped = skgpu::Mipmapped::kNo;

    MTLPixelFormat fFormat = MTLPixelFormatInvalid;
    MTLTextureUsage fUsage = MTLTextureUsageUnknown;
    MTLStorageMode fStorageMode = MTLStorageModeShared;
    bool fFramebufferOnly = false;

    MtlTextureInfo() = default;
    MtlTextureInfo(CFTypeRef mtlTexture);
    MtlTextureInfo(uint32_t sampleCount,
                   skgpu::Mipmapped mipmapped,
                   MTLPixelFormat format,
                   MTLTextureUsage usage,
                   MTLStorageMode storageMode,
                   bool framebufferOnly)
            : fSampleCount(sampleCount)
            , fMipmapped(mipmapped)
            , fFormat(format)
            , fUsage(usage)
            , fStorageMode(storageMode)
            , fFramebufferOnly(framebufferOnly) {}
};
}  // namespace skgpu::graphite

#endif  // __OBJC__

#endif // skgpu_graphite_MtlGraphiteTypes_DEFINED

/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlGraphiteTypes_DEFINED
#define skgpu_graphite_MtlGraphiteTypes_DEFINED

#include "include/core/SkTypes.h"

#if __OBJC__  // <Metal/Metal.h> only works when compiled for Objective C
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/ports/SkCFObject.h"
#include "include/private/base/SkAPI.h"

#import <CoreFoundation/CoreFoundation.h>
#import <Metal/Metal.h>
#import <TargetConditionals.h>

#if TARGET_OS_SIMULATOR
#define SK_API_AVAILABLE_CA_METAL_LAYER SK_API_AVAILABLE(macos(10.11), ios(13.0), tvos(13.0))
#else  // TARGET_OS_SIMULATOR
#define SK_API_AVAILABLE_CA_METAL_LAYER SK_API_AVAILABLE(macos(10.11), ios(8.0), tvos(9.0))
#endif  // TARGET_OS_SIMULATOR

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

namespace TextureInfos {
SK_API TextureInfo MakeMetal(const MtlTextureInfo&);
SK_API TextureInfo MakeMetal(CFTypeRef mtlTexture);

SK_API bool GetMtlTextureInfo(const TextureInfo&, MtlTextureInfo*);
}  // namespace TextureInfos

namespace BackendTextures {
// The BackendTexture will not call retain or release on the passed in CFTypeRef. Thus the
// client must keep the CFTypeRef valid until they are no longer using the BackendTexture.
SK_API BackendTexture MakeMetal(SkISize dimensions, CFTypeRef mtlTexture);

SK_API CFTypeRef GetMtlTexture(const BackendTexture&);
}  // namespace BackendTextures

namespace BackendSemaphores {
// TODO(b/286088355) Determine creator's responsibility for setting refcnt.
SK_API BackendSemaphore MakeMetal(CFTypeRef mtlEvent, uint64_t value);

SK_API CFTypeRef GetMtlEvent(const BackendSemaphore&);
SK_API uint64_t GetMtlValue(const BackendSemaphore&);
}  // namespace BackendSemaphores

} // namespace skgpu::graphite

#endif  // __OBJC__

#endif // skgpu_graphite_MtlGraphiteTypes_DEFINED

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
#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes_cpp.h"
#include "include/private/base/SkAPI.h"

#import <CoreFoundation/CoreFoundation.h>
#import <Metal/Metal.h>
#import <TargetConditionals.h>

class SkStream;
class SkWStream;

namespace skgpu::graphite {

class SK_API MtlTextureInfo final : public TextureInfo::Data {
public:
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
            : Data(sampleCount, mipmapped)
            , fFormat(format)
            , fUsage(usage)
            , fStorageMode(storageMode)
            , fFramebufferOnly(framebufferOnly) {}

private:
    friend class TextureInfo;
    friend class TextureInfoPriv;

    // Non-virtual template API for TextureInfo::Data accessed directly when backend type is known.
    static constexpr skgpu::BackendApi kBackend = skgpu::BackendApi::kMetal;

    Protected isProtected() const { return Protected::kNo; }
    TextureFormat viewFormat() const;

    bool serialize(SkWStream*) const;
    bool deserialize(SkStream*);

    // Virtual API when the specific backend type is not available.
    SkString toBackendString() const override;

    void copyTo(TextureInfo::AnyTextureInfoData& dstData) const override {
        dstData.emplace<MtlTextureInfo>(*this);
    }
    bool isCompatible(const TextureInfo& that, bool requireExact) const override;
};

}  // namespace skgpu::graphite

#endif  // __OBJC__

#endif // skgpu_graphite_MtlGraphiteTypes_DEFINED

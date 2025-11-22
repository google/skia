/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtils.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#include <cstdint>

#import <Metal/Metal.h>

class SkStream;

namespace skgpu::graphite {

MtlTextureInfo::MtlTextureInfo(CFTypeRef texture) {
    SkASSERT(texture);
    id<MTLTexture> mtlTex = (id<MTLTexture>)texture;

    fSampleCount = mtlTex.sampleCount;
    fMipmapped = mtlTex.mipmapLevelCount > 1 ? Mipmapped::kYes : Mipmapped::kNo;

    fFormat = mtlTex.pixelFormat;
    fUsage = mtlTex.usage;
    fStorageMode = mtlTex.storageMode;
    fFramebufferOnly = mtlTex.framebufferOnly;
}

TextureFormat MtlTextureInfo::viewFormat() const {
    return MTLPixelFormatToTextureFormat(fFormat);
}

SkString MtlTextureInfo::toBackendString() const {
    return SkStringPrintf("usage=0x%04X,storageMode=%u,framebufferOnly=%d",
                          (uint32_t)fUsage,
                          (uint32_t)fStorageMode,
                          fFramebufferOnly);
}

bool MtlTextureInfo::isCompatible(const TextureInfo& that, bool requireExact) const {
    const auto& mt = TextureInfoPriv::Get<MtlTextureInfo>(that);
    // The usages may match or the usage passed in may be a superset of the usage stored within.
    const auto usageMask = requireExact ? mt.fUsage : fUsage;
    return fFormat == mt.fFormat &&
           fStorageMode == mt.fStorageMode &&
           fFramebufferOnly == mt.fFramebufferOnly &&
           (usageMask & mt.fUsage) == fUsage;
}

namespace TextureInfos {

skgpu::graphite::TextureInfo MakeMetal(CFTypeRef mtlTexture) {
    return MakeMetal(MtlTextureInfo(mtlTexture));
}

skgpu::graphite::TextureInfo MakeMetal(const MtlTextureInfo& mtlInfo) {
    return TextureInfoPriv::Make(mtlInfo);
}

bool GetMtlTextureInfo(const TextureInfo& info, MtlTextureInfo* out) {
    return TextureInfoPriv::Copy(info, out);
}

}  // namespace TextureInfos

}  // namespace skgpu::graphite

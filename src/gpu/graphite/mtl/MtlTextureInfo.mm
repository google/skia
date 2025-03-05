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

bool MtlTextureInfo::serialize(SkWStream* stream) const {
    SkASSERT(fFormat                                    < (1u << 24));
    SkASSERT(fUsage                                     < (1u << 5));
    SkASSERT(fStorageMode                               < (1u << 2));
    SkASSERT(static_cast<uint32_t>(fFramebufferOnly)    < (1u << 1));

    // TODO(robertphillips): not densely packed (see above asserts)
    if (!stream->write32(static_cast<uint32_t>(fFormat)))        { return false; }
    if (!stream->write16(static_cast<uint16_t>(fUsage)))         { return false; }
    if (!stream->write8(static_cast<uint8_t>(fStorageMode)))     { return false; }
    if (!stream->write8(static_cast<uint8_t>(fFramebufferOnly))) { return false; }
    return true;
}

bool MtlTextureInfo::deserialize(SkStream* stream) {
    uint32_t tmp32;

    if (!stream->readU32(&tmp32)) {
        return false;
    }
    // TODO(robertphillips): add validity checks to deserialized values
    fFormat = static_cast<MTLPixelFormat>(tmp32);

    uint16_t tmp16;
    if (!stream->readU16(&tmp16)) {
        return false;
    }
    fUsage = static_cast<MTLTextureUsage>(tmp16);

    uint8_t tmp8;
    if (!stream->readU8(&tmp8)) {
        return false;
    }
    fStorageMode = static_cast<MTLStorageMode>(tmp8);

    if (!stream->readU8(&tmp8)) {
        return false;
    }
    fFramebufferOnly = SkToBool(tmp8);
    return true;
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

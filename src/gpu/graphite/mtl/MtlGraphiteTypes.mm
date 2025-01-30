/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "src/gpu/graphite/mtl/MtlGraphiteTypesPriv.h"

#import <Metal/Metal.h>

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

MtlTextureInfo MtlTextureSpecToTextureInfo(const MtlTextureSpec& mtlSpec,
                                           uint32_t sampleCount,
                                           Mipmapped mipmapped) {
    MtlTextureInfo info;
    // Shared info
    info.fSampleCount = sampleCount;
    info.fMipmapped = mipmapped;

    // Mtl info
    info.fFormat = mtlSpec.fFormat;
    info.fUsage = mtlSpec.fUsage;
    info.fStorageMode = mtlSpec.fStorageMode;
    info.fFramebufferOnly = mtlSpec.fFramebufferOnly;

    return info;
}

bool MtlTextureSpec::serialize(SkWStream* stream) const {

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

bool MtlTextureSpec::Deserialize(SkStream* stream, MtlTextureSpec* out) {
    uint32_t tmp32;

    if (!stream->readU32(&tmp32)) {
        return false;
    }
    // TODO(robertphillips): add validity checks to deserialized values
    out->fFormat = static_cast<MTLPixelFormat>(tmp32);

    uint16_t tmp16;
    if (!stream->readU16(&tmp16)) {
        return false;
    }
    out->fUsage = static_cast<MTLTextureUsage>(tmp16);

    uint8_t tmp8;
    if (!stream->readU8(&tmp8)) {
        return false;
    }
    out->fStorageMode = static_cast<MTLStorageMode>(tmp8);

    if (!stream->readU8(&tmp8)) {
        return false;
    }
    out->fFramebufferOnly = SkToBool(tmp8);
    return true;
}

}  // namespace skgpu::graphite

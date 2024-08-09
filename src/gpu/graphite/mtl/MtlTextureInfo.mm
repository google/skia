/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/mtl/MtlGraphiteTypesPriv.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#include <cstdint>

#import <Metal/Metal.h>

namespace skgpu::graphite {

class MtlTextureInfoData final : public TextureInfoData {
public:
    MtlTextureInfoData(MtlTextureSpec m) : fMtlSpec(m) {}

#if defined(SK_DEBUG)
    skgpu::BackendApi type() const override { return skgpu::BackendApi::kMetal; }
#endif

    MtlTextureSpec spec() const { return fMtlSpec; }

private:
    MtlTextureSpec fMtlSpec;

    size_t bytesPerPixel() const override {
        return MtlFormatBytesPerBlock(fMtlSpec.fFormat);
    }

    SkTextureCompressionType compressionType() const override {
        return MtlFormatToCompressionType(fMtlSpec.fFormat);
    }

    bool isMemoryless() const override {
        if (@available(macOS 11.0, iOS 10.0, tvOS 10.0, *)) {
            return fMtlSpec.fStorageMode == MTLStorageModeMemoryless;
        }
        return false;
    }

    SkString toString() const override {
        return SkStringPrintf("Metal(%s,", fMtlSpec.toString().c_str());
    }

    SkString toRPAttachmentString(uint32_t sampleCount) const override {
        return SkStringPrintf(
                "Metal(f=%u,s=%u)", static_cast<unsigned int>(fMtlSpec.fFormat), sampleCount);
    }

    void copyTo(AnyTextureInfoData& dstData) const override {
        // Don't assert that dstData is a metal type because it could be
        // uninitialized and that assert would fail.
        dstData.emplace<MtlTextureInfoData>(fMtlSpec);
    }

    bool equal(const TextureInfoData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kMetal);
        if (auto otherMtl = static_cast<const MtlTextureInfoData*>(that)) {
            return fMtlSpec == otherMtl->fMtlSpec;
        }
        return false;
    }

    bool isCompatible(const TextureInfoData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kMetal);
        if (auto otherMtl = static_cast<const MtlTextureInfoData*>(that)) {
            return fMtlSpec.isCompatible(otherMtl->fMtlSpec);
        }
        return false;
    }
};

static const MtlTextureInfoData* get_and_cast_data(const TextureInfo& info) {
    auto data = TextureInfoPriv::GetData(info);
    SkASSERT(!data || data->type() == skgpu::BackendApi::kMetal);
    return static_cast<const MtlTextureInfoData*>(data);
}

namespace TextureInfos {
skgpu::graphite::TextureInfo MakeMetal(CFTypeRef mtlTexture) {
    return MakeMetal(MtlTextureInfo(mtlTexture));
}

skgpu::graphite::TextureInfo MakeMetal(const MtlTextureInfo& mtlInfo) {
    return TextureInfoPriv::Make(skgpu::BackendApi::kMetal,
                                 mtlInfo.fSampleCount,
                                 mtlInfo.fMipmapped,
                                 Protected::kNo,
                                 MtlTextureInfoData(mtlInfo));
}

bool GetMtlTextureInfo(const TextureInfo& info, MtlTextureInfo* out) {
    if (!info.isValid() || info.backend() != skgpu::BackendApi::kMetal) {
        return false;
    }
    SkASSERT(out);
    const MtlTextureInfoData* mtlData = get_and_cast_data(info);
    SkASSERT(mtlData);
    *out = MtlTextureSpecToTextureInfo(mtlData->spec(), info.numSamples(), info.mipmapped());
    return true;
}

// This cannot return a const reference or we get a warning about returning
// a reference to a temporary local variable.
MtlTextureSpec GetMtlTextureSpec(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kMetal);
    const MtlTextureInfoData* mtlData = get_and_cast_data(info);
    SkASSERT(mtlData);
    return mtlData->spec();
}

MTLPixelFormat GetMTLPixelFormat(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kMetal);
    const MtlTextureInfoData* mtlData = get_and_cast_data(info);
    SkASSERT(mtlData);
    return mtlData->spec().fFormat;
}

MTLTextureUsage GetMTLTextureUsage(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kMetal);
    const MtlTextureInfoData* mtlData = get_and_cast_data(info);
    SkASSERT(mtlData);
    return mtlData->spec().fUsage;
}

bool GetMtlFramebufferOnly(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kMetal);
    const MtlTextureInfoData* mtlData = get_and_cast_data(info);
    SkASSERT(mtlData);
    return mtlData->spec().fFramebufferOnly;
}

}  // namespace TextureInfos

}  // namespace skgpu::graphite

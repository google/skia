/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/dawn/DawnGraphiteTypesPriv.h"
#include "src/gpu/graphite/dawn/DawnUtilsPriv.h"

#include <cstdint>

namespace skgpu::graphite {

class DawnTextureInfoData final : public TextureInfoData {
public:
    DawnTextureInfoData(DawnTextureSpec v) : fDawnSpec(v) {}

#if defined(SK_DEBUG)
    skgpu::BackendApi type() const override { return skgpu::BackendApi::kDawn; }
#endif

    DawnTextureSpec spec() const { return fDawnSpec; }

private:
    DawnTextureSpec fDawnSpec;

    size_t bytesPerPixel() const override {
        return DawnFormatBytesPerBlock(fDawnSpec.getViewFormat());
    }

    SkTextureCompressionType compressionType() const override {
        return DawnFormatToCompressionType(fDawnSpec.getViewFormat());
    }

    bool isMemoryless() const override {
#if !defined(__EMSCRIPTEN__)
        return fDawnSpec.fUsage & wgpu::TextureUsage::TransientAttachment;
#else
        return false;
#endif
    }

    SkString toString() const override {
        return SkStringPrintf("Dawn(%s,", fDawnSpec.toString().c_str());
    }

    SkString toRPAttachmentString(uint32_t sampleCount) const override {
        return SkStringPrintf(
                "Dawn(f=%u,s=%u)", static_cast<unsigned int>(fDawnSpec.fFormat), sampleCount);
    }

    void copyTo(AnyTextureInfoData& dstData) const override {
        // Don't assert that dstData has a Dawn type() because it could be
        // uninitialized and that assert would fail.
        dstData.emplace<DawnTextureInfoData>(fDawnSpec);
    }

    bool equal(const TextureInfoData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kDawn);
        if (auto otherDawn = static_cast<const DawnTextureInfoData*>(that)) {
            return fDawnSpec == otherDawn->fDawnSpec;
        }
        return false;
    }

    bool isCompatible(const TextureInfoData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kDawn);
        if (auto otherDawn = static_cast<const DawnTextureInfoData*>(that)) {
            return fDawnSpec.isCompatible(otherDawn->fDawnSpec);
        }
        return false;
    }
};

static const DawnTextureInfoData* get_and_cast_data(const TextureInfo& info) {
    auto data = TextureInfoPriv::GetData(info);
    SkASSERT(!data || data->type() == skgpu::BackendApi::kDawn);
    return static_cast<const DawnTextureInfoData*>(data);
}

namespace TextureInfos {
TextureInfo MakeDawn(const DawnTextureInfo& dawnInfo) {
    return TextureInfoPriv::Make(skgpu::BackendApi::kDawn,
                                 dawnInfo.fSampleCount,
                                 dawnInfo.fMipmapped,
                                 Protected::kNo,
                                 DawnTextureInfoData(dawnInfo));
}

bool GetDawnTextureInfo(const TextureInfo& info, DawnTextureInfo* out) {
    if (!info.isValid() || info.backend() != skgpu::BackendApi::kDawn) {
        return false;
    }
    SkASSERT(out);
    const DawnTextureInfoData* dawnData = get_and_cast_data(info);
    SkASSERT(dawnData);
    *out = DawnTextureSpecToTextureInfo(dawnData->spec(), info.numSamples(), info.mipmapped());
    return true;
}

// This cannot return a const reference or we get a warning about returning
// a reference to a temporary local variable.
DawnTextureSpec GetDawnTextureSpec(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kDawn);
    const DawnTextureInfoData* dawnData = get_and_cast_data(info);
    SkASSERT(dawnData);
    return dawnData->spec();
}

wgpu::TextureFormat GetDawnViewFormat(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kDawn);
    const DawnTextureInfoData* dawnData = get_and_cast_data(info);
    SkASSERT(dawnData);
    return dawnData->spec().getViewFormat();
}

wgpu::TextureAspect GetDawnAspect(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kDawn);
    const DawnTextureInfoData* dawnData = get_and_cast_data(info);
    SkASSERT(dawnData);
    return dawnData->spec().fAspect;
}

}  // namespace TextureInfos

}  // namespace skgpu::graphite

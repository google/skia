/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/gpu/MutableTextureState.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes_cpp.h"
#include "src/gpu/graphite/BackendTexturePriv.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtils.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#include <cstdint>

#import <Metal/Metal.h>

namespace skgpu::graphite {

class MtlBackendTextureData final : public BackendTextureData {
public:
    MtlBackendTextureData(CFTypeRef tex) : fMtlTexture(tex) {}

#if defined(SK_DEBUG)
    skgpu::BackendApi type() const override { return skgpu::BackendApi::kMetal; }
#endif

    CFTypeRef texture() const { return fMtlTexture; }

private:
    CFTypeRef fMtlTexture;

    void copyTo(AnyBackendTextureData& dstData) const override {
        // Don't assert that dstData is a metal type because it could be
        // uninitialized and that assert would fail.
        dstData.emplace<MtlBackendTextureData>(fMtlTexture);
    }

    bool equal(const BackendTextureData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kMetal);
        if (auto otherMtl = static_cast<const MtlBackendTextureData*>(that)) {
            return fMtlTexture == otherMtl->fMtlTexture;
        }
        return false;
    }
};

static const MtlBackendTextureData* get_and_cast_data(const BackendTexture& tex) {
    auto data = BackendTexturePriv::GetData(tex);
    SkASSERT(!data || data->type() == skgpu::BackendApi::kMetal);
    return static_cast<const MtlBackendTextureData*>(data);
}

namespace BackendTextures {
BackendTexture MakeMetal(SkISize dimensions, CFTypeRef mtlTexture) {
    return BackendTexturePriv::Make(
            dimensions, TextureInfos::MakeMetal(mtlTexture), MtlBackendTextureData(mtlTexture));
}

CFTypeRef GetMtlTexture(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kMetal) {
        return nullptr;
    }
    const MtlBackendTextureData* mtlData = get_and_cast_data(tex);
    SkASSERT(mtlData);
    return mtlData->texture();
}

}  // namespace BackendTextures

}  // namespace skgpu::graphite

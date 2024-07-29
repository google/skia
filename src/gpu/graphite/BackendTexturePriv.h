/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_BackendTexturePriv_DEFINED
#define skgpu_graphite_BackendTexturePriv_DEFINED

#include "include/core/SkString.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/private/base/SkAssert.h"

#include <cstdint>

namespace skgpu::graphite {

class BackendTextureData {
public:
    virtual ~BackendTextureData();

#if defined(SK_DEBUG)
    virtual skgpu::BackendApi type() const = 0;
#endif
protected:
    BackendTextureData() = default;
    BackendTextureData(const BackendTextureData&) = default;

    using AnyBackendTextureData = BackendTexture::AnyBackendTextureData;

private:
    friend class BackendTexture;

    virtual void copyTo(AnyBackendTextureData& dstData) const = 0;
    virtual bool equal(const BackendTextureData* that) const = 0;
};

class BackendTexturePriv {
public:
    template <typename SomeBackendTextureData>
    static BackendTexture Make(SkISize dimensions,
                               TextureInfo info,
                               const SomeBackendTextureData& textureData) {
        return BackendTexture(dimensions, info, textureData);
    }

    static const BackendTextureData* GetData(const BackendTexture& info) {
        return info.fTextureData.get();
    }

    static BackendTextureData* GetData(BackendTexture* info) {
        SkASSERT(info);
        return info->fTextureData.get();
    }
};

}  // namespace skgpu::graphite

#endif

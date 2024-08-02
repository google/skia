/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_BackendTexture_DEFINED
#define skgpu_graphite_BackendTexture_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkAnySubclass.h"

namespace skgpu::graphite {

class BackendTextureData;
struct VulkanTextureInfo;

class SK_API BackendTexture {
public:
    BackendTexture();
    BackendTexture(const BackendTexture&);

    ~BackendTexture();

    BackendTexture& operator=(const BackendTexture&);

    bool operator==(const BackendTexture&) const;
    bool operator!=(const BackendTexture& that) const { return !(*this == that); }

    bool isValid() const { return fInfo.isValid(); }
    BackendApi backend() const { return fInfo.backend(); }

    SkISize dimensions() const { return fDimensions; }

    const TextureInfo& info() const { return fInfo; }

private:
    friend class BackendTextureData;
    friend class BackendTexturePriv;

    // Size determined by looking at the BackendTextureData subclasses, then guessing-and-checking.
    // Compiler will complain if this is too small - in that case, just increase the number.
    inline constexpr static size_t kMaxSubclassSize = 72;
    using AnyBackendTextureData = SkAnySubclass<BackendTextureData, kMaxSubclassSize>;

    template <typename SomeBackendTextureData>
    BackendTexture(SkISize dimensions, TextureInfo info, const SomeBackendTextureData& textureData)
            : fDimensions(dimensions), fInfo(info) {
        fTextureData.emplace<SomeBackendTextureData>(textureData);
    }

    SkISize fDimensions;
    TextureInfo fInfo;
    AnyBackendTextureData fTextureData;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_BackendTexture_DEFINED


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

#if defined(SK_DAWN) && !defined(SK_DISABLE_LEGACY_DAWN_BACKEND_TEXTURE_FUNCS)
#include "include/gpu/graphite/dawn/DawnTypes.h"
#endif

#if defined(SK_VULKAN) && !defined(SK_DISABLE_LEGACY_VK_BACKEND_TEXTURE_FUNCS)
#include "include/gpu/vk/VulkanTypes.h"
#endif

namespace skgpu::graphite {

class BackendTextureData;
struct VulkanTextureInfo;

class SK_API BackendTexture {
public:
    BackendTexture();
#if defined(SK_DAWN) && !defined(SK_DISABLE_LEGACY_DAWN_BACKEND_TEXTURE_FUNCS)
    // Create a BackendTexture from a WGPUTexture. Texture info will be queried from the texture.
    //
    // This is the recommended way of specifying a BackendTexture for Dawn. See the note below on
    // the constructor that takes a WGPUTextureView for a fuller explanation.
    //
    // The BackendTexture will not call retain or release on the passed in WGPUTexture. Thus, the
    // client must keep the WGPUTexture valid until they are no longer using the BackendTexture.
    // However, any SkImage or SkSurface that wraps the BackendTexture *will* retain and release
    // the WGPUTexture.
    BackendTexture(WGPUTexture texture);

    // Create a BackendTexture from a WGPUTexture. Texture planeDimensions, plane aspect and
    // info have to be provided. This is intended to be used only when accessing a plane
    // of a WGPUTexture.
    //
    // The BackendTexture will not call retain or release on the passed in WGPUTexture. Thus, the
    // client must keep the WGPUTexture valid until they are no longer using the BackendTexture.
    // However, any SkImage or SkSurface that wraps the BackendTexture *will* retain and release
    // the WGPUTexture.
    BackendTexture(SkISize planeDimensions, const DawnTextureInfo& info, WGPUTexture texture);

    // Create a BackendTexture from a WGPUTextureView. Texture dimensions and
    // info have to be provided.
    //
    // Using a WGPUTextureView rather than a WGPUTexture is less effecient for operations that
    // require buffer transfers to or from the texture (e.g. methods on graphite::Context that read
    // pixels or SkSurface::writePixels). In such cases an intermediate copy to or from a
    // WGPUTexture is required. Thus, it is recommended to use this functionality only for cases
    // where a WGPUTexture is unavailable, in particular when using wgpu::SwapChain.
    //
    // The BackendTexture will not call retain or release on the passed in WGPUTextureView. Thus,
    // the client must keep the WGPUTextureView valid until they are no longer using the
    // BackendTexture. However, any SkImage or SkSurface that wraps the BackendTexture *will* retain
    // and release the WGPUTextureView.
    BackendTexture(SkISize dimensions, const DawnTextureInfo& info, WGPUTextureView textureView);
#endif

#if defined(SK_VULKAN) && !defined(SK_DISABLE_LEGACY_VK_BACKEND_TEXTURE_FUNCS)
    BackendTexture(SkISize dimensions,
                   const VulkanTextureInfo&,
                   VkImageLayout,
                   uint32_t queueFamilyIndex,
                   VkImage,
                   VulkanAlloc);
#endif

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


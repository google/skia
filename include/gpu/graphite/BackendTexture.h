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

#ifdef SK_DAWN
#include "include/gpu/graphite/dawn/DawnTypes.h"
#endif

#ifdef SK_METAL
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#endif

#ifdef SK_VULKAN
#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#endif

namespace skgpu {
class MutableTextureState;
}

namespace skgpu::graphite {

class SK_API BackendTexture {
public:
    BackendTexture();
#ifdef SK_DAWN
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
#ifdef SK_METAL
    // The BackendTexture will not call retain or release on the passed in CFTypeRef. Thus the
    // client must keep the CFTypeRef valid until they are no longer using the BackendTexture.
    BackendTexture(SkISize dimensions, CFTypeRef mtlTexture);
#endif

#ifdef SK_VULKAN
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

    // If the client changes any of the mutable backend of the GrBackendTexture they should call
    // this function to inform Skia that those values have changed. The backend API specific state
    // that can be set from this function are:
    //
    // Vulkan: VkImageLayout and QueueFamilyIndex
    void setMutableState(const skgpu::MutableTextureState&);

#ifdef SK_DAWN
    WGPUTexture getDawnTexturePtr() const;
    WGPUTextureView getDawnTextureViewPtr() const;
#endif
#ifdef SK_METAL
    CFTypeRef getMtlTexture() const;
#endif

#ifdef SK_VULKAN
    VkImage getVkImage() const;
    VkImageLayout getVkImageLayout() const;
    uint32_t getVkQueueFamilyIndex() const;
    const VulkanAlloc* getMemoryAlloc() const;
#endif

private:
    friend class VulkanResourceProvider;    // for getMutableState
    sk_sp<MutableTextureState> getMutableState() const;

    SkISize fDimensions;
    TextureInfo fInfo;

    sk_sp<MutableTextureState> fMutableState;

#ifdef SK_VULKAN
    // fMemoryAlloc == VulkanAlloc() if the client has already created their own VkImage and
    // will destroy it themselves as opposed to having Skia create/destroy it via
    // Recorder::createBackendTexture and Context::deleteBackendTexture.
    VulkanAlloc fMemoryAlloc = VulkanAlloc();
#endif

    union {
#ifdef SK_DAWN
        struct {
            WGPUTexture fDawnTexture;
            WGPUTextureView fDawnTextureView;
        };
#endif
#ifdef SK_METAL
        CFTypeRef fMtlTexture;
#endif
#ifdef SK_VULKAN
        VkImage fVkImage = VK_NULL_HANDLE;
#endif
        void* fEnsureUnionNonEmpty;
    };
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_BackendTexture_DEFINED


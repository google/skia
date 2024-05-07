/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VkYcbcrSamplerHelper_DEFINED
#define VkYcbcrSamplerHelper_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_VULKAN

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/BackendTexture.h"

namespace skgpu::graphite {
class Recorder;
class VulkanSharedContext;
}
#endif


#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/vk/GrVkTypes.h"

class GrDirectContext;
class GrVkGpu;

// This helper will create and hold data for a Vulkan YCbCr backend texture. This format is
// particularly interesting because its sampler is immutable.
class VkYcbcrSamplerHelper {
public:
#if defined(SK_GRAPHITE)
    VkYcbcrSamplerHelper(const skgpu::graphite::VulkanSharedContext* ctxt,
                         VkPhysicalDevice physDev)
            : fSharedCtxt(ctxt)
            , fPhysDev(physDev) {
        SkASSERT(ctxt);
        fDContext = nullptr;
        fGrTexture = {};
    }

    const skgpu::graphite::BackendTexture& backendTexture() const { return fTexture; }

    bool createBackendTexture(uint32_t width, uint32_t height);
#endif

    VkYcbcrSamplerHelper(GrDirectContext*);

    const GrBackendTexture& grBackendTexture() const { return fGrTexture; }

    ~VkYcbcrSamplerHelper();

    bool isYCbCrSupported();

    bool createGrBackendTexture(uint32_t width, uint32_t height);

    static int GetExpectedY(int x, int y, int width, int height);
    static std::pair<int, int> GetExpectedUV(int x, int y, int width, int height);

private:
#if defined(SK_GRAPHITE)
    skgpu::graphite::BackendTexture             fTexture;
    const skgpu::graphite::VulkanSharedContext* fSharedCtxt;
    // Needed to query PhysicalDeviceFormatProperties for relevant VkFormat(s)
    VkPhysicalDevice                            fPhysDev;
#endif

    GrVkGpu* vkGpu();

    GrDirectContext* fDContext;
    GrBackendTexture fGrTexture;

    VkImage fImage = VK_NULL_HANDLE;
    VkDeviceMemory fImageMemory = VK_NULL_HANDLE;
};

#endif // SK_VULKAN
#endif // VkYcbcrSamplerHelper_DEFINED

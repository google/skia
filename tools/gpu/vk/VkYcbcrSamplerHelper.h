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
#endif

#if defined (SK_GANESH)
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/vk/GrVkTypes.h"
#endif

#include "include/gpu/vk/VulkanTypes.h"

namespace skgpu::graphite {
    class Recorder;
    class VulkanSharedContext;
}

class GrDirectContext;
class GrVkGpu;
class GrBackendTexture;

// This helper will create and hold data for a Vulkan YCbCr backend texture. This format is
// particularly interesting because its sampler is immutable.
class VkYcbcrSamplerHelper {
public:
#if defined(SK_GRAPHITE)
    VkYcbcrSamplerHelper(const skgpu::graphite::VulkanSharedContext* ctxt)
            : fSharedCtxt(ctxt) {
        SkASSERT(ctxt);
    }

    const skgpu::graphite::BackendTexture& backendTexture() const { return fTexture; }

    bool createBackendTexture(uint32_t width, uint32_t height);
#endif

#if defined(SK_GANESH)
    VkYcbcrSamplerHelper(GrDirectContext*);
#endif

    ~VkYcbcrSamplerHelper();

    bool isYCbCrSupported();

#if defined(SK_GANESH)
    const GrBackendTexture& grBackendTexture() const { return fGrTexture; }
    bool createGrBackendTexture(uint32_t width, uint32_t height);
#endif

    static int GetExpectedY(int x, int y, int width, int height);
    static std::pair<int, int> GetExpectedUV(int x, int y, int width, int height);

private:
#if defined(SK_GRAPHITE)
    skgpu::graphite::BackendTexture             fTexture;
    const skgpu::graphite::VulkanSharedContext* fSharedCtxt = nullptr;
#endif

#if defined(SK_GANESH)
    GrVkGpu* vkGpu();

    GrDirectContext* fDContext = nullptr;
    GrBackendTexture fGrTexture;
#endif

    VkImage fImage = VK_NULL_HANDLE;
    VkDeviceMemory fImageMemory = VK_NULL_HANDLE;
};

#endif // SK_VULKAN
#endif // VkYcbcrSamplerHelper_DEFINED

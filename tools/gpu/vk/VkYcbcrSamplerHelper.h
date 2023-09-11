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

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/vk/GrVkTypes.h"

class GrDirectContext;
class GrVkGpu;

// This helper will create and hold data for a Vulkan YCbCr backend texture. This format is
// particularly interesting because its sampler is immutable.
class VkYcbcrSamplerHelper {
public:
    VkYcbcrSamplerHelper(GrDirectContext*);
    ~VkYcbcrSamplerHelper();

    bool isYCbCrSupported();

    bool createBackendTexture(uint32_t width, uint32_t height);

    const GrBackendTexture& backendTexture() const { return fTexture; }

    static int GetExpectedY(int x, int y, int width, int height);
    static std::pair<int, int> GetExpectedUV(int x, int y, int width, int height);

private:
    GrVkGpu* vkGpu();

    GrDirectContext* fDContext;

    VkImage fImage = VK_NULL_HANDLE;
    VkDeviceMemory fImageMemory = VK_NULL_HANDLE;
    GrBackendTexture fTexture;
};

#endif // SK_VULKAN

#endif // VkYcbcrSamplerHelper_DEFINED

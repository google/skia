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

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrBackendSurface.h"

class GrContext;
class GrVkGpu;
class SkImage;

// This helper will create a YCbCr-backed SkImage for Vulkan. Images of this format are
// particularly interesting because their samplers are immutable.
class VkYcbcrSamplerHelper {
public:
    VkYcbcrSamplerHelper(GrContext*);
    ~VkYcbcrSamplerHelper();

    // This is wrt the supplied GrContext
    bool isYCbCrSupported();

    sk_sp<SkImage> createI420Image(uint32_t width, uint32_t height);

    static int GetExpectedY(int x, int y, int width, int height);
    static std::pair<int, int> GetExpectedUV(int x, int y, int width, int height);

private:
    GrVkGpu* vkGpu();

    GrContext* fContext;

    VkImage fImage = VK_NULL_HANDLE;
    VkDeviceMemory fImageMemory = VK_NULL_HANDLE;
    GrBackendTexture fTexture;
};

#endif // SK_VULKAN

#endif // VkYcbcrSamplerHelper_DEFINED

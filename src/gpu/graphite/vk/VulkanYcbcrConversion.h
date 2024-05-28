/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanYcbcrConversion_DEFINED
#define skgpu_graphite_VulkanYcbcrConversion_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/core/SkChecksum.h"

#include <cinttypes>

namespace skgpu {
struct VulkanYcbcrConversionInfo;
}

namespace skgpu::graphite {

class VulkanSharedContext;

class VulkanYcbcrConversion : public Resource {
public:
    static sk_sp<VulkanYcbcrConversion> Make(const VulkanSharedContext*,
                                             const VulkanYcbcrConversionInfo&);

    static sk_sp<VulkanYcbcrConversion> Make(const VulkanSharedContext*,
                                             uint32_t nonFormatInfo,
                                             uint64_t format);

    static GraphiteResourceKey MakeYcbcrConversionKey(const VulkanSharedContext*,
                                                      const VulkanYcbcrConversionInfo&);

    // Return a fully-formed GraphiteResourceKey that represents a YCbCr conversion by extracting
    // relevant information from a SamplerDesc.
    static GraphiteResourceKey GetKeyFromSamplerDesc(const SamplerDesc& samplerDesc);

    VkSamplerYcbcrConversion ycbcrConversion() const { return fYcbcrConversion; }

    const char* getResourceType() const override { return "Vulkan YCbCr Conversion"; }

private:
    VulkanYcbcrConversion(const VulkanSharedContext*, VkSamplerYcbcrConversion);

    void freeGpuData() override;

    VkSamplerYcbcrConversion fYcbcrConversion;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanYcbcrConversion_DEFINED


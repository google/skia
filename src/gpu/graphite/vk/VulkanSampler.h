/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanSampler_DEFINED
#define skgpu_graphite_VulkanSampler_DEFINED

#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanYcbcrConversion.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkTileMode.h"

struct SkSamplingOptions;

namespace skgpu::graphite {

class VulkanSampler : public Sampler {
public:
    static sk_sp<VulkanSampler> Make(const VulkanSharedContext*,
                                     const SamplerDesc&,
                                     sk_sp<VulkanYcbcrConversion> ycbcrConversion = nullptr);
    ~VulkanSampler() override {}

    VkSampler vkSampler() const { return fSampler; }

    const VulkanYcbcrConversion* ycbcrConversion() const { return fYcbcrConversion.get(); }

    const SamplerDesc& samplerDesc() const { return fDesc; }

    const VkSampler* constVkSamplerPtr() const { return &fSampler; }

private:
    VulkanSampler(const VulkanSharedContext*,
                  const SamplerDesc&,
                  VkSampler,
                  sk_sp<VulkanYcbcrConversion>);

    void freeGpuData() override;

    // It's helpful to store the sampler desc such that when we create a descriptor for this sampler
    // we can easily access the numerical sampler representation.
    const SamplerDesc fDesc;

    VkSampler fSampler;
    sk_sp<VulkanYcbcrConversion> fYcbcrConversion;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_VulkanSampler_DEFINED

/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanSampler_DEFINED
#define skgpu_graphite_VulkanSampler_DEFINED

#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/vk/VulkanSamplerYcbcrConversion.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkTileMode.h"

struct SkSamplingOptions;

namespace skgpu::graphite {

class VulkanSampler : public Sampler {
public:
    static sk_sp<VulkanSampler> Make(const VulkanSharedContext*,
                                     const SkSamplingOptions&,
                                     SkTileMode xTileMode,
                                     SkTileMode yTileMode,
                                     sk_sp<VulkanSamplerYcbcrConversion> ycbcrConversion = nullptr);

    ~VulkanSampler() override {}

    VkSampler vkSampler() const { return fSampler; }

    const VulkanSamplerYcbcrConversion* ycbcrConversion() const { return fYcbcrConversion.get(); }

private:
    VulkanSampler(const VulkanSharedContext*, VkSampler, sk_sp<VulkanSamplerYcbcrConversion>);

    void freeGpuData() override;

    VkSampler fSampler;
    sk_sp<VulkanSamplerYcbcrConversion> fYcbcrConversion;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_VulkanSampler_DEFINED

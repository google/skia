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
                                     const SkSamplingOptions& samplingOptions,
                                     SkTileMode xTileMode,
                                     SkTileMode yTileMode);

    ~VulkanSampler() override {}

    VkSampler vkSampler() const { return fSampler; }

private:
    VulkanSampler(const VulkanSharedContext*, VkSampler);

    void freeGpuData() override;

    VkSampler fSampler;
    // TODO: Add YCbCr conversion information to this class.
    //sk_sp<VulkanSamplerYcbcrConversion> fYcbcrConversion;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_VulkanSampler_DEFINED

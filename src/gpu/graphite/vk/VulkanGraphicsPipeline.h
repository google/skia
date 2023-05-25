/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanGraphicsPipeline_DEFINED
#define skgpu_graphite_VulkanGraphicsPipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/graphite/GraphicsPipeline.h"

namespace skgpu::graphite {

class VulkanSharedContext;

class VulkanGraphicsPipeline final : public GraphicsPipeline {
public:
    inline static constexpr unsigned int kIntrinsicUniformBufferIndex = 0;
    inline static constexpr unsigned int kRenderStepUniformBufferIndex = 1;
    inline static constexpr unsigned int kPaintUniformBufferIndex = 2;

    // For now, rigidly assign all uniform buffer descriptors to be in one descriptor set in binding
    // 0 and all texture/samplers to be in binding 1.
    // TODO(b/274762935): Make the bindings and descriptor set organization more flexible.
    inline static constexpr unsigned int kUniformBufferDescSetIndex = 0;
    inline static constexpr unsigned int kTextureBindDescSetIndex = 1;


    static sk_sp<VulkanGraphicsPipeline> Make(const VulkanSharedContext*
                                              /* TODO: fill out argument list */);

    ~VulkanGraphicsPipeline() override {}

    VkPipelineLayout layout() const {
        SkASSERT(fPipelineLayout != VK_NULL_HANDLE);
        return fPipelineLayout;
    }

    // TODO: Implement.
    bool hasStepUniforms() const { return false; }
    bool hasFragment() const { return false; }

private:
    VulkanGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext
                           /* TODO: fill out argument list */)
        : GraphicsPipeline(sharedContext) { }

    void freeGpuData() override;

    VkPipelineLayout  fPipelineLayout;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlGraphicsPipeline_DEFINED

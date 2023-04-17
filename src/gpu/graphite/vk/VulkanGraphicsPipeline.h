/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanGraphicsPipeline_DEFINED
#define skgpu_graphite_VulkanGraphicsPipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/graphite/GraphicsPipeline.h"

namespace skgpu::graphite {

class VulkanSharedContext;

class VulkanGraphicsPipeline final : public GraphicsPipeline {
public:
    static sk_sp<VulkanGraphicsPipeline> Make(const VulkanSharedContext*
                                              /* TODO: fill out argument list */);

    ~VulkanGraphicsPipeline() override {}

private:
    VulkanGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext
                           /* TODO: fill out argument list */)
        : GraphicsPipeline(sharedContext) { }

    void freeGpuData() override;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlGraphicsPipeline_DEFINED

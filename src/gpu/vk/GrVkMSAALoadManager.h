/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkMSAALoadManager_DEFINED
#define GrVkMSAALoadManager_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/GrNativeRect.h"
#include "src/gpu/GrPrimitiveProcessor.h"
#include "src/gpu/vk/GrVkDescriptorSetManager.h"

class GrSurface;
class GrVkCommandBuffer;
class GrVkGpu;
class GrVkRenderPass;
struct SkIRect;

class GrVkMSAALoadManager {
public:
    GrVkMSAALoadManager();

    ~GrVkMSAALoadManager();

    bool loadMSAAFromResolve(GrVkGpu* gpu,
                             GrVkCommandBuffer* commandBuffer,
                             const GrVkRenderPass& renderPass,
                             GrSurface* dst,
                             GrSurface* src,
                             const SkIRect& srcRect);

    void destroyResources(GrVkGpu* gpu);

private:
    bool createMSAALoadProgram(GrVkGpu* gpu);

    // Everything below is only created once and shared by all msaa load pipelines
    VkShaderModule fVertShaderModule;
    VkShaderModule fFragShaderModule;
    VkPipelineShaderStageCreateInfo fShaderStageInfo[2];

    // All pipelines used by this class use the same VkPipelineLayout. Therefore, unlike regular
    // GrVkPipelines, we have the manager own the layout instead of the GrVkPipeline.
    VkPipelineLayout fPipelineLayout;
};

#endif


/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkResourceProvider_DEFINED
#define GrVkResourceProvider_DEFINED

#include "GrVkDescriptorPool.h"
#include "GrVkResource.h"
#include "GrVkUtil.h"
#include "SkTArray.h"

#include "vulkan/vulkan.h"

class GrPipeline;
class GrPrimitiveProcessor;
class GrVkCommandBuffer;
class GrVkGpu;
class GrVkPipeline;
class GrVkRenderPass;
class GrVkRenderTarget;

class GrVkResourceProvider {
public:
    GrVkResourceProvider(GrVkGpu* gpu);
    ~GrVkResourceProvider();

    GrVkPipeline* createPipeline(const GrPipeline& pipeline,
                                 const GrPrimitiveProcessor& primProc,
                                 VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                 int shaderStageCount,
                                 GrPrimitiveType primitiveType,
                                 const GrVkRenderPass& renderPass,
                                 VkPipelineLayout layout);

    // Finds or creates a simple render pass that matches the target, increments the refcount,
    // and returns.
    const GrVkRenderPass* findOrCreateCompatibleRenderPass(const GrVkRenderTarget& target);

    GrVkCommandBuffer* createCommandBuffer();
    void checkCommandBuffers();

    // Finds or creates a compatible GrVkDescriptorPool for the requested DescriptorTypeCount.
    // The refcount is incremented and a pointer returned.
    // TODO: Currently this will just create a descriptor pool without holding onto a ref itself
    //       so we currently do not reuse them. Rquires knowing if another draw is currently using
    //       the GrVkDescriptorPool, the ability to reset pools, and the ability to purge pools out
    //       of our cache of GrVkDescriptorPools.
    GrVkDescriptorPool* findOrCreateCompatibleDescriptorPool(
                                        const GrVkDescriptorPool::DescriptorTypeCounts& typeCounts);

    // Destroy any cached resources. To be called before destroying the VkDevice.
    // The assumption is that all queues are idle and all command buffers are finished.
    // For resource tracing to work properly, this should be called after unrefing all other
    // resource usages.
    void destroyResources();

    // Abandon any cached resources. To be used when the context/VkDevice is lost.
    // For resource tracing to work properly, this should be called after unrefing all other
    // resource usages.
    void abandonResources();

private:
    GrVkGpu* fGpu;

    // Array of RenderPasses that only have a single color attachment, optional stencil attachment,
    // optional resolve attachment, and only one subpass
    SkSTArray<4, GrVkRenderPass*> fSimpleRenderPasses;

    // Array of CommandBuffers that are currently in flight
    SkSTArray<4, GrVkCommandBuffer*> fActiveCommandBuffers;
};

#endif

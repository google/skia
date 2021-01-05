/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipeline_DEFINED
#define GrVkPipeline_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrPrimitiveProcessor.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/vk/GrVkManagedResource.h"

class GrPipeline;
class GrProgramInfo;
class GrRenderTarget;
class GrStencilSettings;
class GrSwizzle;
class GrVkCommandBuffer;
class GrVkGpu;
class GrVkRenderPass;
struct SkIRect;

class GrVkPipeline : public GrVkManagedResource {
public:
    static sk_sp<GrVkPipeline> Make(GrVkGpu*,
                                    const GrPrimitiveProcessor::AttributeSet& vertexAttribs,
                                    const GrPrimitiveProcessor::AttributeSet& instanceAttribs,
                                    GrPrimitiveType,
                                    GrSurfaceOrigin,
                                    const GrStencilSettings&,
                                    int numRasterSamples,
                                    bool isHWAntialiasState,
                                    bool isMixedSampled,
                                    const GrXferProcessor::BlendInfo&,
                                    bool isWireframe,
                                    bool useConservativeRaster,
                                    uint32_t subpass,
                                    VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                    int shaderStageCount,
                                    VkRenderPass compatibleRenderPass,
                                    VkPipelineLayout layout,
                                    bool ownsLayout,
                                    VkPipelineCache cache);

    static sk_sp<GrVkPipeline> Make(GrVkGpu*,
                                    const GrProgramInfo&,
                                    VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                    int shaderStageCount,
                                    VkRenderPass compatibleRenderPass,
                                    VkPipelineLayout layout,
                                    VkPipelineCache cache,
                                    uint32_t subpass);

    VkPipeline pipeline() const { return fPipeline; }
    VkPipelineLayout layout() const {
        SkASSERT(fPipelineLayout != VK_NULL_HANDLE);
        return fPipelineLayout;
    }

    static void SetDynamicScissorRectState(GrVkGpu*, GrVkCommandBuffer*, const GrRenderTarget*,
                                           GrSurfaceOrigin, const SkIRect& scissorRect);
    static void SetDynamicViewportState(GrVkGpu*, GrVkCommandBuffer*, const GrRenderTarget*);
    static void SetDynamicBlendConstantState(GrVkGpu*,
                                             GrVkCommandBuffer*,
                                             const GrSwizzle& writeSwizzle,
                                             const GrXferProcessor&);

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkPipeline: %d (%d refs)\n", fPipeline, this->getRefCnt());
    }
#endif

protected:
    GrVkPipeline(const GrVkGpu* gpu, VkPipeline pipeline, VkPipelineLayout layout)
            : INHERITED(gpu), fPipeline(pipeline), fPipelineLayout(layout) {}

    VkPipeline  fPipeline;
    VkPipelineLayout  fPipelineLayout;

private:
    void freeGPUData() const override;

    using INHERITED = GrVkManagedResource;
};

#endif

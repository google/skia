/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipeline_DEFINED
#define GrVkPipeline_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/Blend.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/vk/GrVkManagedResource.h"

#include <cinttypes>

class GrPipeline;
class GrProgramInfo;
class GrRenderTarget;
class GrStencilSettings;
class GrVkCommandBuffer;
class GrVkGpu;
class GrVkRenderPass;
class GrXferProcessor;
struct SkIRect;

namespace skgpu {
class Swizzle;
}

class GrVkPipeline : public GrVkManagedResource {
public:
    static sk_sp<GrVkPipeline> Make(GrVkGpu*,
                                    const GrGeometryProcessor::AttributeSet& vertexAttribs,
                                    const GrGeometryProcessor::AttributeSet& instanceAttribs,
                                    GrPrimitiveType,
                                    GrSurfaceOrigin,
                                    const GrStencilSettings&,
                                    int numSamples,
                                    bool isHWAntialiasState,
                                    const skgpu::BlendInfo&,
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

    static void SetDynamicScissorRectState(GrVkGpu*,
                                           GrVkCommandBuffer*,
                                           SkISize colorAttachmentDimensions,
                                           GrSurfaceOrigin, const SkIRect& scissorRect);
    static void SetDynamicViewportState(GrVkGpu*,
                                        GrVkCommandBuffer*,
                                        SkISize colorAttachmentDimensions);
    static void SetDynamicBlendConstantState(GrVkGpu*,
                                             GrVkCommandBuffer*,
                                             const skgpu::Swizzle& writeSwizzle,
                                             const GrXferProcessor&);

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkPipeline: %" PRIdPTR " (%d refs)\n", (intptr_t)fPipeline, this->getRefCnt());
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

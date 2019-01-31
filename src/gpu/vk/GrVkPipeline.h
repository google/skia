/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipeline_DEFINED
#define GrVkPipeline_DEFINED

#include "GrTypesPriv.h"
#include "GrVkResource.h"
#include "vk/GrVkTypes.h"

class GrPipeline;
class GrPrimitiveProcessor;
class GrRenderTarget;
class GrXferProcessor;
class GrStencilSettings;
class GrVkCommandBuffer;
class GrVkGpu;
class GrVkRenderPass;
struct SkIRect;

class GrVkPipeline : public GrVkResource {
public:
    static GrVkPipeline* Create(GrVkGpu*,
                                int numColorSamples,
                                const GrPrimitiveProcessor&,
                                const GrPipeline& pipeline,
                                const GrStencilSettings&,
                                VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                int shaderStageCount,
                                GrPrimitiveType primitiveType,
                                VkRenderPass compatibleRenderPass,
                                VkPipelineLayout layout,
                                VkPipelineCache cache);

    VkPipeline pipeline() const { return fPipeline; }

    static void SetDynamicScissorRectState(GrVkGpu*, GrVkCommandBuffer*, const GrRenderTarget*,
                                           GrSurfaceOrigin, SkIRect);
    static void SetDynamicViewportState(GrVkGpu*, GrVkCommandBuffer*, const GrRenderTarget*);
    static void SetDynamicBlendConstantState(GrVkGpu*, GrVkCommandBuffer*, GrPixelConfig,
                                             const GrXferProcessor&);

#ifdef SK_TRACE_VK_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkPipeline: %d (%d refs)\n", fPipeline, this->getRefCnt());
    }
#endif

protected:
    GrVkPipeline(VkPipeline pipeline) : INHERITED(), fPipeline(pipeline) {}

    VkPipeline  fPipeline;

private:
    void freeGPUData(GrVkGpu* gpu) const override;

    typedef GrVkResource INHERITED;
};

#endif

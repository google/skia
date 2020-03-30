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
#include "src/gpu/vk/GrVkManagedResource.h"

class GrPipeline;
class GrPrimitiveProcessor;
class GrRenderTarget;
class GrXferProcessor;
class GrStencilSettings;
class GrVkCommandBuffer;
class GrVkGpu;
class GrVkRenderPass;
struct SkIRect;

class GrVkPipeline : public GrVkManagedResource {
public:
    static GrVkPipeline* Create(GrVkGpu*,
                                const GrProgramInfo&,
                                VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                int shaderStageCount,
                                VkRenderPass compatibleRenderPass,
                                VkPipelineLayout layout,
                                VkPipelineCache cache);

    VkPipeline pipeline() const { return fPipeline; }
    VkPipelineLayout layout() const { return fPipelineLayout; }

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

    typedef GrVkManagedResource INHERITED;
};

#endif

/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipeline_DEFINED
#define GrVkPipeline_DEFINED

#include "GrTypes.h"

#include "GrVkResource.h"

#include "vk/GrVkDefines.h"

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
    static GrVkPipeline* Create(GrVkGpu* gpu,
                                const GrPipeline& pipeline,
                                const GrStencilSettings&,
                                const GrPrimitiveProcessor& primProc,
                                VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                int shaderStageCount,
                                GrPrimitiveType primitiveType,
                                const GrVkRenderPass& renderPass,
                                VkPipelineLayout layout,
                                VkPipelineCache cache);

    VkPipeline pipeline() const { return fPipeline; }

    static void SetDynamicScissorRectState(GrVkGpu*, GrVkCommandBuffer*, const GrRenderTarget*,
                                           SkIRect);
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
    void freeGPUData(const GrVkGpu* gpu) const override;

    typedef GrVkResource INHERITED;
};

#endif

/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipeline_DEFINED
#define GrVkPipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrManagedResource.h"
#include "src/gpu/ganesh/vk/GrVkManagedResource.h"

#include <cinttypes>
#include <cstdint>

class GrProgramInfo;
class GrStencilSettings;
class GrVkCommandBuffer;
class GrVkGpu;
class GrXferProcessor;
enum GrSurfaceOrigin : int;
enum class GrPrimitiveType : uint8_t;
struct SkIRect;
struct SkISize;

namespace skgpu {
class Swizzle;
struct BlendInfo;
}  // namespace skgpu

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

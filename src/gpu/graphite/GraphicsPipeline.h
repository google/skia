/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GraphicsPipeline_DEFINED
#define skgpu_graphite_GraphicsPipeline_DEFINED

#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"

namespace skgpu::graphite {

struct FragSkSLInfo;
struct VertSkSLInfo;
class RenderStep;

/**
 * GraphicsPipeline corresponds to a backend specific pipeline used for rendering (vs. compute),
 * e.g. MTLRenderPipelineState (Metal),
 *      CreateRenderPipeline (Dawn),
 *      CreateGraphicsPipelineState (D3D12),
 *   or VkGraphicsPipelineCreateInfo (Vulkan).
 *
 * A GraphicsPipeline is created from the combination of a GraphicsPipelineDesc (representing draw
 * specific configuration) and a RenderPassDesc (representing the target of the draw).
 */
class GraphicsPipeline : public Resource {
public:
    ~GraphicsPipeline() override;

    const char* getResourceType() const override { return "Graphics Pipeline"; }

    DstReadRequirement dstReadRequirement() const { return fPipelineInfo.fDstReadReq; }

    int  numFragTexturesAndSamplers() const { return fPipelineInfo.fNumFragTexturesAndSamplers; }
    bool hasPaintUniforms()           const { return fPipelineInfo.fHasPaintUniforms;           }
    bool hasStepUniforms()            const { return fPipelineInfo.fHasStepUniforms;            }
    bool hasGradientBuffer()          const { return fPipelineInfo.fHasGradientBuffer;          }

    struct PipelineInfo {
        PipelineInfo() = default;

        // NOTE: Subclasses must manually fill in native shader code in GPU_TEST_UTILS builds.
        PipelineInfo(const VertSkSLInfo&, const FragSkSLInfo&);

        DstReadRequirement fDstReadReq = DstReadRequirement::kNone;
        int  fNumFragTexturesAndSamplers = 0;
        bool fHasPaintUniforms  = false;
        bool fHasStepUniforms   = false;
        bool fHasGradientBuffer = false;

        // In test-enabled builds, we preserve the generated shader code to display in the viewer
        // slide UI. This is not quite enough information to fully recreate the pipeline, as the
        // RenderPassDesc used to make the pipeline is not preserved.
#if defined(GPU_TEST_UTILS)
        std::string fLabel;

        std::string fSkSLVertexShader;
        std::string fSkSLFragmentShader;
        std::string fNativeVertexShader;
        std::string fNativeFragmentShader;
#endif
    };

#if defined(GPU_TEST_UTILS)
    const PipelineInfo& getPipelineInfo() const {
        return fPipelineInfo;
    }
#endif

protected:
    GraphicsPipeline(const SharedContext*, const PipelineInfo&);

private:
    PipelineInfo fPipelineInfo;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_GraphicsPipeline_DEFINED

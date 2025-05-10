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

class ShaderInfo;
class RenderStep;

enum class PipelineCreationFlags : uint8_t {
    kNone             = 0b000,
    // For Dawn, this flag overrides the DawnCaps::fUseAsyncPipelineCreation
    // parameter and forces Synchronous Pipeline creation.
    kForPrecompilation = 0b001,
};

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

    DstReadStrategy dstReadStrategy() const { return fPipelineInfo.fDstReadStrategy; }

    int  numFragTexturesAndSamplers() const { return fPipelineInfo.fNumFragTexturesAndSamplers; }
    bool hasPaintUniforms()           const { return fPipelineInfo.fHasPaintUniforms;           }
    bool hasStepUniforms()            const { return fPipelineInfo.fHasStepUniforms;            }
    bool hasGradientBuffer()          const { return fPipelineInfo.fHasGradientBuffer;          }

    struct PipelineInfo {
        PipelineInfo() = default;

        // NOTE: Subclasses must manually fill in native shader code in GPU_TEST_UTILS builds.
        PipelineInfo(const ShaderInfo&, SkEnumBitMask<PipelineCreationFlags>,
                     uint32_t uniqueKeyHash, uint32_t compilationID);

        DstReadStrategy fDstReadStrategy = DstReadStrategy::kNoneRequired;
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
        const uint32_t fUniqueKeyHash = 0;
        // The compilation ID is used to distinguish between different compilations/instantiations
        // of the same unique key. If, for example, two versions were created due to threading.
        const uint32_t fCompilationID = 0;
        const bool fFromPrecompile = false;
        bool fWasUsed = false;
        uint16_t fEpoch = 0;   // the last epoch in which this Pipeline was touched
    };

    const PipelineInfo& getPipelineInfo() const {
        return fPipelineInfo;
    }
    bool fromPrecompile() const { return fPipelineInfo.fFromPrecompile; }

    void markUsed() { fPipelineInfo.fWasUsed = true; }
    bool wasUsed() const { return fPipelineInfo.fWasUsed; }

    void markEpoch(uint16_t epoch) { fPipelineInfo.fEpoch = epoch; }
    uint16_t epoch() const { return fPipelineInfo.fEpoch; }

    // GraphicsPipeline compiles can take a while. If the underlying compilation is performed
    // asynchronously, we may create a GraphicsPipeline object that later "fails" and need to remove
    // it from the GlobalCache.
    virtual bool didAsyncCompilationFail() const { return false; }

protected:
    GraphicsPipeline(const SharedContext*, const PipelineInfo&);

private:
    PipelineInfo fPipelineInfo;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_GraphicsPipeline_DEFINED

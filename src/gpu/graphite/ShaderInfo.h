/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ShaderInfo_DEFINED
#define skgpu_graphite_ShaderInfo_DEFINED

#include "include/private/base/SkTArray.h"
#include "src/base/SkArenaAlloc.h"
#include "src/gpu/Blend.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"

namespace skgpu::graphite {

class RenderStep;
class RuntimeEffectDictionary;
class ShaderCodeDictionary;
class ShaderNode;

// ShaderInfo holds all root ShaderNodes defined for a PaintParams as well as the extracted fixed
// function blending parameters and other aggregate requirements for the effect trees that have
// been linked into a single fragment program (sans any RenderStep fragment work and fixed SkSL
// logic required for all rendering in Graphite).
class ShaderInfo {
public:
    // Accepts a real or, by default, an invalid/nullptr pointer to a container of SamplerDescs.
    // Backend implementations which may utilize static / immutable samplers should pass in a real
    // pointer to indicate that shader node data must be analyzed to determine whether
    // immutable samplers are used, and if so, ascertain SamplerDescs for them.
    // TODO(b/366220690): Actually perform this analysis.
    //
    // If provided a valid container ptr, this function will delegate the addition of SamplerDescs
    // for each sampler the nodes utilize (dynamic and immutable). This way, a SamplerDesc's index
    // within the container can inform its binding order. Each SamplerDesc will be either:
    // 1) a default-constructed SamplerDesc, indicating the use of a "regular" dynamic sampler which
    //    requires no special handling OR
    // 2) a real SamplerDesc describing an immutable sampler. Backend pipelines can then use the
    //    desc to obtain a real immutable sampler pointer (which typically must be included in
    //    pipeline layouts)
    static std::unique_ptr<ShaderInfo> Make(const Caps*,
                                            const ShaderCodeDictionary*,
                                            const RuntimeEffectDictionary*,
                                            const RenderStep*,
                                            UniquePaintParamsID,
                                            bool useStorageBuffers,
                                            skgpu::Swizzle writeSwizzle,
                                            DstReadStrategy dstReadStrategy,
                                            skia_private::TArray<SamplerDesc>* outDescs = nullptr);

    const ShaderCodeDictionary* shaderCodeDictionary() const {
        return fShaderCodeDictionary;
    }
    const RuntimeEffectDictionary* runtimeEffectDictionary() const {
        return fRuntimeEffectDictionary;
    }

    const char* shadingSsboIndex() const { return fShadingSsboIndex; }

    DstReadStrategy dstReadStrategy() const { return fDstReadStrategy; }
    const skgpu::BlendInfo& blendInfo() const { return fBlendInfo; }

    const skia_private::TArray<uint32_t>& data() const { return fData; }

    const std::string& vertexSkSL() const { return fVertexSkSL; }
    const std::string& fragmentSkSL() const { return fFragmentSkSL; }
    const std::string& vsLabel() const { return fVSLabel; }
    const std::string& fsLabel() const { return fFSLabel; }

    int numFragmentTexturesAndSamplers() const { return fNumFragmentTexturesAndSamplers; }
    bool hasStepUniforms() const { return fHasStepUniforms; }
    bool hasPaintUniforms() const { return fHasPaintUniforms; }
    bool hasGradientBuffer() const { return fHasGradientBuffer; }

    // Name used in-shader for gradient buffer uniform.
    static constexpr char kGradientBufferName[] = "fsGradientBuffer";

private:
    ShaderInfo(const ShaderCodeDictionary*,
               const RuntimeEffectDictionary*,
               const char* ssboIndex,
               DstReadStrategy);

    void generateVertexSkSL(const Caps*,
                            const RenderStep*,
                            bool useStorageBuffers);

    // Determines fNumFragmentTexturesAndSamplers, fHasPaintUniforms, fHasGradientBuffer,
    // fHasSsboIndicesVarying, and if a valid SamplerDesc ptr is passed in, any immutable
    // sampler SamplerDescs.
    void generateFragmentSkSL(const Caps*,
                              const ShaderCodeDictionary*,
                              const RenderStep*,
                              UniquePaintParamsID,
                              bool useStorageBuffers,
                              skgpu::Swizzle writeSwizzle,
                              skia_private::TArray<SamplerDesc>* outDescs);

    bool needsLocalCoords() const;

    // Recursive method which traverses ShaderNodes in a depth-first manner to aggregate all
    // ShaderNode data (not owned by ShaderNode) into ShaderInfo's owned fData.
    // TODO(b/347072931): Ideally, this method could go away and each snippet's data could remain
    // tied to its ID instead of accumulating it all here.
    void aggregateSnippetData(const ShaderNode*);

    // All shader nodes and arrays of children pointers are held in this arena
    SkArenaAlloc fShaderNodeAlloc{256};

    const ShaderCodeDictionary* fShaderCodeDictionary;
    const RuntimeEffectDictionary* fRuntimeEffectDictionary;
    const char* fShadingSsboIndex;

    // De-compressed shader tree from a PaintParamsKey. There can be 1 or 2 root nodes, the first
    // being the paint effects (rooted with a BlendCompose for the final paint blend) and the
    // optional second being any analytic clip effect (geometric or shader treated as coverage).
    SkSpan<const ShaderNode*> fRootNodes;
    // The blendInfo represents the actual GPU blend operations, which may or may not completely
    // implement the paint and coverage blending defined by the root nodes.
    skgpu::BlendInfo fBlendInfo;
    DstReadStrategy fDstReadStrategy = DstReadStrategy::kNoneRequired;

    // Note that fData is currently only used to store SamplerDesc information for shaders that have
    // the option of using immutable samplers. However, other snippets could leverage this field to
    // convey other information once data can be tied to snippetIDs (b/347072931).
    skia_private::TArray<uint32_t> fData;

    std::string fVertexSkSL;
    std::string fFragmentSkSL;
    std::string fVSLabel;
    std::string fFSLabel;

    int fNumFragmentTexturesAndSamplers = 0;
    bool fHasStepUniforms = false;
    bool fHasPaintUniforms = false;
    bool fHasLiftedPaintUniforms = false;
    bool fHasGradientBuffer = false;
    bool fHasSsboIndicesVarying = false;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ShaderInfo_DEFINED

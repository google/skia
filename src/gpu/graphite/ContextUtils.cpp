/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ContextUtils.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkM44.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkSpan_impl.h"
#include "src/base/SkEnumBitMask.h"
#include "src/gpu/BlendFormula.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ComputeTypes.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLUtil.h"

#include <string>

namespace skgpu::graphite {

UniquePaintParamsID ExtractPaintData(Recorder* recorder,
                                     PipelineDataGatherer* gatherer,
                                     PaintParamsKeyBuilder* builder,
                                     const Layout layout,
                                     const SkM44& local2Dev,
                                     const PaintParams& p,
                                     const Geometry& geometry,
                                     const SkColorInfo& targetColorInfo) {
    SkDEBUGCODE(builder->checkReset());

    gatherer->resetWithNewLayout(layout);

    KeyContext keyContext(recorder,
                          local2Dev,
                          targetColorInfo,
                          geometry.isShape() || geometry.isEdgeAAQuad()
                                  ? KeyGenFlags::kDefault
                                  : KeyGenFlags::kDisableSamplingOptimization,
                          p.color());
    p.toKey(keyContext, builder, gatherer);

    return recorder->priv().shaderCodeDictionary()->findOrCreate(builder);
}

bool CanUseHardwareBlending(const Caps* caps,
                            std::optional<SkBlendMode> blendMode,
                            Coverage coverage) {
    // If the blend mode is absent, this is assumed to be for a runtime blender, for which we always
    // do a dst read.
    if (!blendMode.has_value()) {
        return false;
    }

    // Check for special cases that would prevent the usage of direct hardware blending and
    // require us to fall back to using shader-based blending.
    const SkBlendMode bm = blendMode.value();
    const bool hasCoverage = coverage != Coverage::kNone;
    if (// Using LCD coverage (which must be applied after the blend equation) with any blend mode
        // besides SkBlendMode::kSrcOver
        // TODO(b/414597217): Add support to use dual-source blending with LCD coverage.
        (coverage == Coverage::kLCD && bm != SkBlendMode::kSrcOver) ||

        // SkBlendMode::kPlus always clamps its output to [0,1], but we can't rely on hardware
        // blending to do that for all texture formats.
        // NOTE: We could check the draw dst properties to only do in-shader blending with plus when
        // necessary, but we can't detect that during shader precompilation.
        bm == SkBlendMode::kPlus ||

        // Using an advanced blend mode but the hardware does not support them
        (bm > SkBlendMode::kLastCoeffMode && !caps->supportsHardwareAdvancedBlending()) ||

        // The blend formula requires dual-source blending, but it is not supported by hardware
        (bm <= SkBlendMode::kLastCoeffMode &&
         (coverage == Coverage::kLCD ? skgpu::GetLCDBlendFormula(bm).hasSecondaryOutput()
                                     : skgpu::GetBlendFormula(/*isOpaque=*/false,
                                                              hasCoverage,
                                                              bm).hasSecondaryOutput()) &&
         !caps->shaderCaps()->fDualSourceBlendingSupport)) {
        return false;
    }

    // In all other cases (which are more commonly encountered; e.g. using a simple blend mode),
    // we can use direct HW blending.
    return true;
}

void CollectIntrinsicUniforms(const Caps* caps,
                              SkIRect viewport,
                              SkIRect dstReadBounds,
                              UniformManager* uniforms) {
    SkDEBUGCODE(uniforms->setExpectedUniforms(kIntrinsicUniforms, /*isSubstruct=*/false);)

    // viewport
    {
        // The vertex shader needs to divide by the dimension and then multiply by 2, so do this
        // once on the CPU. This is because viewport normalization wants to range from -1 to 1, and
        // not 0 to 1. If any other user of the viewport uniform requires the true reciprocal or
        // original dimensions, this can be adjusted.
        SkASSERT(!viewport.isEmpty());
        float invTwoW = 2.f / viewport.width();
        float invTwoH = 2.f / viewport.height();

        // If the NDC Y axis points up (opposite normal skia convention and the underlying view
        // convention), upload the inverse height as a negative value. See ShaderInfo::Make
        // for how this is used.
        if (!caps->ndcYAxisPointsDown()) {
            invTwoH *= -1.f;
        }
        uniforms->write(SkV4{(float) viewport.left(), (float) viewport.top(), invTwoW, invTwoH});
    }

    // dstReadBounds
    {
        // Unlike viewport, dstReadBounds can be empty so check for 0 dimensions and set the
        // reciprocal to 0. It is also not doubled since its purpose is to normalize texture coords
        // to 0 to 1, and not -1 to 1.
        int width = dstReadBounds.width();
        int height = dstReadBounds.height();
        uniforms->write(SkV4{(float) dstReadBounds.left(), (float) dstReadBounds.top(),
                             width ? 1.f / width : 0.f, height ? 1.f / height : 0.f});
    }

    SkDEBUGCODE(uniforms->doneWithExpectedUniforms());
}

std::string EmitSamplerLayout(const ResourceBindingRequirements& bindingReqs, int* binding) {
    std::string result;

    if (bindingReqs.fSeparateTextureAndSamplerBinding) {
        int samplerIndex = (*binding)++;
        int textureIndex = (*binding)++;
        result = SkSL::String::printf("layout(webgpu, set=%d, sampler=%d, texture=%d)",
                                      bindingReqs.fTextureSamplerSetIdx,
                                      samplerIndex,
                                      textureIndex);
    } else {
        int samplerIndex = (*binding)++;
        result = SkSL::String::printf("layout(set=%d, binding=%d)",
                                      bindingReqs.fTextureSamplerSetIdx,
                                      samplerIndex);
    }
    return result;
}

std::string GetPipelineLabel(const ShaderCodeDictionary* dict,
                             const RenderPassDesc& renderPassDesc,
                             const RenderStep* renderStep,
                             UniquePaintParamsID paintID) {
    std::string label = renderPassDesc.toPipelineLabel().c_str(); // includes the write swizzle
    label += " + ";
    label += renderStep->name();
    label += " + ";
    // the shader portion will be "(empty)" for depth-only draws
    label += dict->idToString(paintID).c_str();
    return label;
}

std::string BuildComputeSkSL(const Caps* caps, const ComputeStep* step, BackendApi backend) {
    std::string sksl =
            SkSL::String::printf("layout(local_size_x=%u, local_size_y=%u, local_size_z=%u) in;\n",
                                 step->localDispatchSize().fWidth,
                                 step->localDispatchSize().fHeight,
                                 step->localDispatchSize().fDepth);

    const auto& bindingReqs = caps->resourceBindingRequirements();
    const bool texturesUseDistinctIdxRanges = bindingReqs.fComputeUsesDistinctIdxRangesForTextures;
    int index = 0;
    // NOTE: SkSL Metal codegen always assigns the same binding index to a texture and its sampler.
    // TODO: This could cause sampler indices to not be tightly packed if the sampler2D declaration
    // comes after 1 or more storage texture declarations (which don't have samplers). An optional
    // "layout(msl, sampler=T, texture=T)" syntax to count them separately (like we do for WGSL)
    // could come in handy here but it's not supported in MSL codegen yet.
    int texIdx = 0;
    for (const ComputeStep::ResourceDesc& r : step->resources()) {
        using Type = ComputeStep::ResourceType;
        switch (r.fType) {
            case Type::kUniformBuffer:
                SkSL::String::appendf(&sksl, "layout(binding=%d) uniform ", index++);
                sksl += r.fSkSL;
                break;
            case Type::kStorageBuffer:
            case Type::kIndirectBuffer:
                SkSL::String::appendf(&sksl, "layout(binding=%d) buffer ", index++);
                sksl += r.fSkSL;
                break;
            case Type::kReadOnlyStorageBuffer:
                SkSL::String::appendf(&sksl, "layout(binding=%d) readonly buffer ", index++);
                sksl += r.fSkSL;
                break;
            case Type::kWriteOnlyStorageTexture:
                SkSL::String::appendf(&sksl, "layout(binding=%d, rgba8) writeonly texture2D ",
                                      texturesUseDistinctIdxRanges ? texIdx++ : index++);
                sksl += r.fSkSL;
                break;
            case Type::kReadOnlyTexture:
                SkSL::String::appendf(&sksl, "layout(binding=%d, rgba8) readonly texture2D ",
                                      texturesUseDistinctIdxRanges ? texIdx++ : index++);
                sksl += r.fSkSL;
                break;
            case Type::kSampledTexture:
                // The following SkSL expects specific backends to have certain resource binding
                // requirements. Before appending the SkSL, assert that these assumptions hold true.
                // TODO(b/396420770): Have this method be more backend-agnostic.
                if (backend == BackendApi::kMetal) {
                     // Metal is expected to use combined texture/samplers.
                    SkASSERT(!bindingReqs.fSeparateTextureAndSamplerBinding);
                    SkSL::String::appendf(&sksl,
                                          "layout(metal, binding=%d) ",
                                          texturesUseDistinctIdxRanges ? texIdx++ : index++);
                } else if (backend == BackendApi::kDawn) {
                    // Dawn is expected to use separate texture/samplers and not use distinct
                    // index ranges for texture resources.
                    SkASSERT(bindingReqs.fSeparateTextureAndSamplerBinding &&
                             !texturesUseDistinctIdxRanges);
                    SkSL::String::appendf(
                        &sksl, "layout(webgpu, sampler=%d, texture=%d) ", index, index + 1);
                    index += 2;
                } else {
                    // This SkSL depends upon the assumption that we are using combined texture/
                    // samplers and that we are not using separate resource indices for textures.
                    SkASSERT(!bindingReqs.fSeparateTextureAndSamplerBinding &&
                             !texturesUseDistinctIdxRanges);
                    SkSL::String::appendf(&sksl, "layout(binding=%d) ", index++);
                }
                sksl += "sampler2D ";
                sksl += r.fSkSL;
                break;
        }
        sksl += ";\n";
    }

    sksl += step->computeSkSL();
    return sksl;
}

} // namespace skgpu::graphite

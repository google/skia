/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ContextUtils.h"

#include "src/gpu/BlendFormula.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PaintParams.h"
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
                                  ? KeyContext::OptimizeSampling::kYes
                                  : KeyContext::OptimizeSampling::kNo,
                          p.color());
    p.toKey(keyContext, builder, gatherer);

    return recorder->priv().shaderCodeDictionary()->findOrCreate(builder);
}

DstReadRequirement GetDstReadRequirement(const Caps* caps,
                                         std::optional<SkBlendMode> blendMode,
                                         Coverage coverage) {
    // If the blend mode is absent, this is assumed to be for a runtime blender, for which we always
    // do a dst read.
    // If the blend mode is plus, always do in-shader blending since we may be drawing to an
    // unsaturated surface (e.g. F16) and we don't want to let the hardware clamp the color output
    // in that case. We could check the draw dst properties to only do in-shader blending with plus
    // when necessary, but we can't detect that during shader precompilation.
    if (!blendMode || *blendMode > SkBlendMode::kLastCoeffMode ||
        *blendMode == SkBlendMode::kPlus) {
        return caps->getDstReadRequirement();
    }

    const bool isLCD = coverage == Coverage::kLCD;
    const bool hasCoverage = coverage != Coverage::kNone;
    BlendFormula blendFormula = isLCD ? skgpu::GetLCDBlendFormula(*blendMode)
                                      : skgpu::GetBlendFormula(false, hasCoverage, *blendMode);
    if ((blendFormula.hasSecondaryOutput() && !caps->shaderCaps()->fDualSourceBlendingSupport) ||
        (coverage == Coverage::kLCD && blendMode != SkBlendMode::kSrcOver)) {
        return caps->getDstReadRequirement();
    }

    return DstReadRequirement::kNone;
}

void CollectIntrinsicUniforms(const Caps* caps,
                              SkIRect viewport,
                              SkIRect dstCopyBounds,
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

    // dstCopyBounds
    {
        // Unlike viewport, dstCopyBounds can be empty so check for 0 dimensions and set the
        // reciprocal to 0. It is also not doubled since its purpose is to normalize texture coords
        // to 0 to 1, and not -1 to 1.
        int width = dstCopyBounds.width();
        int height = dstCopyBounds.height();
        uniforms->write(SkV4{(float) dstCopyBounds.left(), (float) dstCopyBounds.top(),
                             width ? 1.f / width : 0.f, height ? 1.f / height : 0.f});
    }

    SkDEBUGCODE(uniforms->doneWithExpectedUniforms());
}

std::string EmitSamplerLayout(const ResourceBindingRequirements& bindingReqs, int* binding) {
    std::string result;

    // If fDistinctIndexRanges is false, then texture and sampler indices may clash with other
    // resource indices. Graphite assumes that they will be placed in descriptor set (Vulkan) and
    // bind group (Dawn) index 1.
    const char* distinctIndexRange = bindingReqs.fDistinctIndexRanges ? "" : "set=1, ";

    if (bindingReqs.fSeparateTextureAndSamplerBinding) {
        int samplerIndex = (*binding)++;
        int textureIndex = (*binding)++;
        result = SkSL::String::printf("layout(webgpu, %ssampler=%d, texture=%d)",
                                      distinctIndexRange,
                                      samplerIndex,
                                      textureIndex);
    } else {
        int samplerIndex = (*binding)++;
        result = SkSL::String::printf("layout(%sbinding=%d)",
                                      distinctIndexRange,
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
    label += dict->idToString(paintID).c_str(); // will be "(empty)" for depth-only draws
    return label;
}

std::string BuildComputeSkSL(const Caps* caps, const ComputeStep* step) {
    std::string sksl =
            SkSL::String::printf("layout(local_size_x=%u, local_size_y=%u, local_size_z=%u) in;\n",
                                 step->localDispatchSize().fWidth,
                                 step->localDispatchSize().fHeight,
                                 step->localDispatchSize().fDepth);

    const auto& bindingReqs = caps->resourceBindingRequirements();
    bool distinctRanges = bindingReqs.fDistinctIndexRanges;
    bool separateSampler = bindingReqs.fSeparateTextureAndSamplerBinding;

    int index = 0;
    int texIdx = 0;
    // NOTE: SkSL Metal codegen always assigns the same binding index to a texture and its sampler.
    // TODO: This could cause sampler indices to not be tightly packed if the sampler2D declaration
    // comes after 1 or more storage texture declarations (which don't have samplers). An optional
    // "layout(msl, sampler=T, texture=T)" syntax to count them separately (like we do for WGSL)
    // could come in handy here but it's not supported in MSL codegen yet.

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
                                      distinctRanges ? texIdx++ : index++);
                sksl += r.fSkSL;
                break;
            case Type::kReadOnlyTexture:
                SkSL::String::appendf(&sksl, "layout(binding=%d, rgba8) readonly texture2D ",
                                      distinctRanges ? texIdx++ : index++);
                sksl += r.fSkSL;
                break;
            case Type::kSampledTexture:
                if (distinctRanges) {
                    SkSL::String::appendf(&sksl, "layout(metal, binding=%d) ", texIdx++);
                } else if (separateSampler) {
                    SkSL::String::appendf(
                            &sksl, "layout(webgpu, sampler=%d, texture=%d) ", index, index + 1);
                    index += 2;
                } else {
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

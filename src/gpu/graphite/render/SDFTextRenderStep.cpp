/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/SDFTextRenderStep.h"

#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/SubRunData.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"
#include "src/sksl/SkSLString.h"
#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/VertexFiller.h"

#if defined(SK_GAMMA_APPLY_TO_A8)
#include "include/private/base/SkCPUTypes.h"
#include "src/core/SkMaskGamma.h"
#include "src/text/gpu/DistanceFieldAdjustTable.h"
#endif

namespace skgpu::graphite {

namespace {

// We are expecting to sample from up to 4 textures
constexpr int kNumSDFAtlasTextures = 4;

}  // namespace

SDFTextRenderStep::SDFTextRenderStep()
        : RenderStep(RenderStepID::kSDFText,
                     Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage |
                     Flags::kAppendInstances,
                     /*uniforms=*/{{"subRunDeviceMatrix", SkSLType::kFloat4x4},
                                   {"deviceToLocal", SkSLType::kFloat4x4},
                                   {"atlasSizeInv", SkSLType::kFloat2},
                                   {"gammaParams", SkSLType::kHalf2}},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthLEqualPass,
                     /*staticAttrs=*/ {},
                     /*appendAttrs=*/
                     {{"size", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"uvPos", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"xyPos", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"indexAndFlags", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"strikeToSourceScale", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndices", VertexAttribType::kUInt2, SkSLType::kUInt2}},
                     /*varyings=*/
                     {{"unormTexCoords", SkSLType::kFloat2},
                      {"textureCoords", SkSLType::kFloat2},
                      {"texIndex", SkSLType::kFloat}}) {}

SDFTextRenderStep::~SDFTextRenderStep() {}

std::string SDFTextRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "texIndex = half(indexAndFlags.x);"
           "float4 devPosition = text_vertex_fn(float2(sk_VertexID >> 1, sk_VertexID & 1), "
                                               "subRunDeviceMatrix, "
                                               "deviceToLocal, "
                                               "atlasSizeInv, "
                                               "float2(size), "
                                               "float2(uvPos), "
                                               "xyPos, "
                                               "strikeToSourceScale, "
                                               "depth, "
                                               "textureCoords, "
                                               "unormTexCoords, "
                                               "stepLocalCoords);";
}

std::string SDFTextRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    std::string result;

    for (unsigned int i = 0; i < kNumSDFAtlasTextures; ++i) {
        result += EmitSamplerLayout(bindingReqs, nextBindingIndex);
        SkSL::String::appendf(&result, " sampler2D sdf_atlas_%u;\n", i);
    }

    return result;
}

const char* SDFTextRenderStep::fragmentCoverageSkSL() const {
    // The returned SkSL must write its coverage into a 'half4 outputCoverage' variable (defined in
    // the calling code) with the actual coverage splatted out into all four channels.

    // TODO: To minimize the number of shaders generated this is the full affine shader.
    // For best performance it may be worth creating the uniform scale shader as well,
    // as that's the most common case.
    // TODO: Need to add 565 support.
    // TODO: Need aliased and possibly sRGB support.
    static_assert(kNumSDFAtlasTextures == 4);
    return "outputCoverage = sdf_text_coverage_fn(sample_indexed_atlas(textureCoords, "
                                                                      "int(texIndex), "
                                                                      "sdf_atlas_0, "
                                                                      "sdf_atlas_1, "
                                                                      "sdf_atlas_2, "
                                                                      "sdf_atlas_3).r, "
                                                 "gammaParams, "
                                                 "unormTexCoords);";
}

void SDFTextRenderStep::writeVertices(DrawWriter* dw,
                                      const DrawParams& params,
                                      skvx::uint2 ssboIndices) const {
    const SubRunData& subRunData = params.geometry().subRunData();
    subRunData.subRun()->vertexFiller().fillInstanceData(dw,
                                                         subRunData.startGlyphIndex(),
                                                         subRunData.glyphCount(),
                                                         subRunData.subRun()->instanceFlags(),
                                                         ssboIndices,
                                                         subRunData.subRun()->glyphs(),
                                                         params.order().depthAsFloat());
}

void SDFTextRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                 PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(gatherer->checkRewind());
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    const SubRunData& subRunData = params.geometry().subRunData();
    unsigned int numProxies;
    Recorder* recorder = subRunData.recorder();
    const sk_sp<TextureProxy>* proxies =
            recorder->priv().atlasProvider()->textAtlasManager()->getProxies(
                    subRunData.subRun()->maskFormat(), &numProxies);
    SkASSERT(proxies && numProxies > 0);

    // write uniforms
    gatherer->write(params.transform().matrix());  // subRunDeviceMatrix
    gatherer->write(subRunData.deviceToLocal());
    SkV2 atlasDimensionsInverse = {1.f/proxies[0]->dimensions().width(),
                                   1.f/proxies[0]->dimensions().height()};
    gatherer->write(atlasDimensionsInverse);

    float gammaAdjustment = 0;
    // TODO: generate LCD adjustment
#if defined(SK_GAMMA_APPLY_TO_A8)
    auto dfAdjustTable = sktext::gpu::DistanceFieldAdjustTable::Get();
    // TODO: don't do this for aliased text
    U8CPU lum = SkColorSpaceLuminance::computeLuminance(SK_GAMMA_EXPONENT,
                                                        subRunData.luminanceColor());
    gammaAdjustment = dfAdjustTable->getAdjustment(lum, subRunData.useGammaCorrectDistanceTable());
#endif
    SkV2 gammaParams = {gammaAdjustment, subRunData.useGammaCorrectDistanceTable() ? 1.f : 0.f};
    gatherer->writeHalf(gammaParams);

    // write textures and samplers
    for (unsigned int i = 0; i < numProxies; ++i) {
        gatherer->add(proxies[i], {SkFilterMode::kLinear, SkTileMode::kClamp});
    }
    // If the atlas has less than 4 active proxies we still need to set up samplers for the shader.
    for (unsigned int i = numProxies; i < kNumSDFAtlasTextures; ++i) {
        gatherer->add(proxies[0], {SkFilterMode::kLinear, SkTileMode::kClamp});
    }
}

}  // namespace skgpu::graphite

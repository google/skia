/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/BitmapTextRenderStep.h"

#include "include/core/SkM44.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"
#include "src/sksl/SkSLString.h"
#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/VertexFiller.h"

using AtlasSubRun = sktext::gpu::AtlasSubRun;

namespace skgpu::graphite {

namespace {

// We are expecting to sample from up to 4 textures
constexpr int kNumTextAtlasTextures = 4;

}  // namespace

BitmapTextRenderStep::BitmapTextRenderStep(bool isLCD)
        : RenderStep("BitmapTextRenderStep",
                     "",
                     isLCD ? Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage |
                             Flags::kLCDCoverage
                           : Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage,
                     /*uniforms=*/{{"subRunDeviceMatrix", SkSLType::kFloat4x4},
                                   {"deviceToLocal"     , SkSLType::kFloat4x4},
                                   {"atlasSizeInv"      , SkSLType::kFloat2}},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGEqualPass,
                     /*vertexAttrs=*/ {},
                     /*instanceAttrs=*/
                     {{"size", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"uvPos", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"xyPos", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"indexAndFlags", VertexAttribType::kUShort2, SkSLType::kUShort2},
                      {"strikeToSourceScale", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndex", VertexAttribType::kInt, SkSLType::kInt}},
                     /*varyings=*/
                     {{"textureCoords", SkSLType::kFloat2},
                      {"texIndex", SkSLType::kHalf},
                      {"maskFormat", SkSLType::kHalf}}) {}

BitmapTextRenderStep::~BitmapTextRenderStep() {}

std::string BitmapTextRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "texIndex = half(indexAndFlags.x);"
           "maskFormat = half(indexAndFlags.y);"
           "float2 unormTexCoords;"
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

std::string BitmapTextRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    std::string result;

    for (unsigned int i = 0; i < kNumTextAtlasTextures; ++i) {
        result += EmitSamplerLayout(bindingReqs, nextBindingIndex);
        SkSL::String::appendf(&result, " sampler2D text_atlas_%d;\n", i);
    }

    return result;
}

const char* BitmapTextRenderStep::fragmentCoverageSkSL() const {
    // The returned SkSL must write its coverage into a 'half4 outputCoverage' variable (defined in
    // the calling code) with the actual coverage splatted out into all four channels.
    static_assert(kNumTextAtlasTextures == 4);
    return "outputCoverage = bitmap_text_coverage_fn(sample_indexed_atlas(textureCoords, "
                                                                         "int(texIndex), "
                                                                         "text_atlas_0, "
                                                                         "text_atlas_1, "
                                                                         "text_atlas_2, "
                                                                         "text_atlas_3), "
                                                    "int(maskFormat));";
}

void BitmapTextRenderStep::writeVertices(DrawWriter* dw,
                                         const DrawParams& params,
                                         int ssboIndex) const {
    const SubRunData& subRunData = params.geometry().subRunData();

    subRunData.subRun()->vertexFiller().fillInstanceData(dw,
                                                         subRunData.startGlyphIndex(),
                                                         subRunData.glyphCount(),
                                                         subRunData.subRun()->instanceFlags(),
                                                         ssboIndex,
                                                         subRunData.subRun()->glyphs(),
                                                         params.order().depthAsFloat());
}

void BitmapTextRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                    PipelineDataGatherer* gatherer) const {
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

    // write textures and samplers
    const SkSamplingOptions kSamplingOptions(SkFilterMode::kNearest);
    constexpr SkTileMode kTileModes[2] = { SkTileMode::kClamp, SkTileMode::kClamp };
    for (unsigned int i = 0; i < numProxies; ++i) {
        gatherer->add(kSamplingOptions, kTileModes, proxies[i]);
    }
    // If the atlas has less than 4 active proxies we still need to set up samplers for the shader.
    for (unsigned int i = numProxies; i < kNumTextAtlasTextures; ++i) {
        gatherer->add(kSamplingOptions, kTileModes, proxies[0]);
    }
}

}  // namespace skgpu::graphite

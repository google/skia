/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/BitmapTextRenderStep.h"

#include "src/core/SkPipelineData.h"

#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkSLString.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/text/AtlasManager.h"
#include "src/text/gpu/SubRunContainer.h"

using AtlasSubRun = sktext::gpu::AtlasSubRun;

namespace skgpu::graphite {

namespace {

// We are expecting to sample from up to 4 textures
constexpr int kNumTextAtlasTextures = 4;

}  // namespace

BitmapTextRenderStep::BitmapTextRenderStep(bool isA8)
        : RenderStep("BitmapTextRenderStep",
                     isA8 ? "A8" : "ARGB",
                     Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage,
                     /*uniforms=*/{{"deviceMatrix", SkSLType::kFloat4x4},
                                   {"atlasSizeInv", SkSLType::kFloat2}},
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
                      {"texIndex", SkSLType::kFloat}})
        , fIsA8(isA8) {}

BitmapTextRenderStep::~BitmapTextRenderStep() {}

const char* BitmapTextRenderStep::vertexSkSL() const {
    return R"(
        float2 baseCoords = float2(float(sk_VertexID >> 1), float(sk_VertexID & 1));
        baseCoords.xy *= float2(size);

        stepLocalCoords = strikeToSourceScale*baseCoords + float2(xyPos);
        float4 position = deviceMatrix*float4(stepLocalCoords, 0, 1);

        float2 unormTexCoords = baseCoords + float2(uvPos);
        textureCoords = unormTexCoords * atlasSizeInv;
        texIndex = float(indexAndFlags.x);

        float4 devPosition = float4(position.xy, depth, position.w);
    )";
}

std::string BitmapTextRenderStep::texturesAndSamplersSkSL(int binding) const {
    std::string result;

    for (unsigned int i = 0; i < kNumTextAtlasTextures; ++i) {
        SkSL::String::appendf(&result,
                              "layout(binding=%d) uniform sampler2D text_atlas_%d;\n", binding, i);
        binding++;
    }

    return result;
}

const char* BitmapTextRenderStep::fragmentCoverageSkSL() const {
    if (fIsA8) {
        return R"(
            half4 texColor;
            if (texIndex == 0) {
               texColor = sample(text_atlas_0, textureCoords).rrrr;
            } else if (texIndex == 1) {
               texColor = sample(text_atlas_1, textureCoords).rrrr;
            } else if (texIndex == 2) {
               texColor = sample(text_atlas_2, textureCoords).rrrr;
            } else if (texIndex == 3) {
               texColor = sample(text_atlas_3, textureCoords).rrrr;
            } else {
               texColor = sample(text_atlas_0, textureCoords).rrrr;
            }
            outputCoverage = texColor;
        )";
    } else {
        return R"(
            half4 texColor;
            if (texIndex == 0) {
               texColor = sample(text_atlas_0, textureCoords);
            } else if (texIndex == 1) {
               texColor = sample(text_atlas_1, textureCoords);
            } else if (texIndex == 2) {
               texColor = sample(text_atlas_2, textureCoords);
            } else if (texIndex == 3) {
               texColor = sample(text_atlas_3, textureCoords);
            } else {
               texColor = sample(text_atlas_0, textureCoords);
            }
            outputCoverage = texColor;
        )";
    }
}

void BitmapTextRenderStep::writeVertices(DrawWriter* dw,
                                         const DrawParams& params,
                                         int ssboIndex) const {
    const SubRunData& subRunData = params.geometry().subRunData();

    subRunData.subRun()->fillInstanceData(dw, subRunData.startGlyphIndex(), subRunData.glyphCount(),
                                          ssboIndex, params.order().depthAsFloat());
}

void BitmapTextRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                    SkPipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    const SubRunData& subRunData = params.geometry().subRunData();
    unsigned int numProxies;
    Recorder* recorder = subRunData.recorder();
    const sk_sp<TextureProxy>* proxies =
            recorder->priv().atlasManager()->getProxies(subRunData.subRun()->maskFormat(),
                                                        &numProxies);
    SkASSERT(proxies && numProxies > 0);

    // write uniforms
    gatherer->write(params.transform());
    skvx::float2 atlasDimensionsInverse = {1.f/proxies[0]->dimensions().width(),
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

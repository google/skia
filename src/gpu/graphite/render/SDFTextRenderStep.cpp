/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/SDFTextRenderStep.h"

#include "include/core/SkM44.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkSLString.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/text/AtlasManager.h"
#include "src/text/gpu/SubRunContainer.h"

namespace skgpu::graphite {

namespace {

// We are expecting to sample from up to 4 textures
constexpr int kNumSDFAtlasTextures = 4;

}  // namespace

SDFTextRenderStep::SDFTextRenderStep(bool isA8)
        : RenderStep("SDFTextRenderStep",
                     isA8 ? "A8" : "565",
                     Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage,
                     /*uniforms=*/{{"deviceMatrix", SkSLType::kFloat4x4},
                                   {"atlasSizeInv", SkSLType::kFloat2},
                                   {"distanceAdjust", SkSLType::kFloat}},
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
                     {{"unormTexCoords", SkSLType::kFloat2},
                      {"textureCoords", SkSLType::kFloat2},
                      {"texIndex", SkSLType::kFloat}}) {
    // TODO: store if it's A8 and adjust shader
}

SDFTextRenderStep::~SDFTextRenderStep() {}

std::string SDFTextRenderStep::vertexSkSL() const {
    return
        "float2 baseCoords = float2(float(sk_VertexID >> 1), float(sk_VertexID & 1));"
        "baseCoords.xy *= float2(size);"

        "stepLocalCoords = strikeToSourceScale*baseCoords + float2(xyPos);"
        "float4 position = deviceMatrix*float4(stepLocalCoords, 0, 1);"

        "unormTexCoords = baseCoords + float2(uvPos);"
        "textureCoords = unormTexCoords * atlasSizeInv;"
        "texIndex = float(indexAndFlags.x);"

        "float4 devPosition = float4(position.xy, depth, position.w);";
}

std::string SDFTextRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    std::string result;

    for (unsigned int i = 0; i < kNumSDFAtlasTextures; ++i) {
        result += EmitSamplerLayout(bindingReqs, nextBindingIndex);
        SkSL::String::appendf(&result, " uniform sampler2D sdf_atlas_%d;\n", i);
    }

    return result;
}

const char* SDFTextRenderStep::fragmentCoverageSkSL() const {
    // TODO: To minimize the number of shaders generated this is the full affine shader.
    // For best performance it may be worth creating the uniform scale shader as well,
    // as that's the most common case.
    // TODO: Need to add 565 support.
    // TODO: Need aliased and possibly sRGB support.
    return
        "half texColor;"
        "if (texIndex == 0) {"
           "texColor = sample(sdf_atlas_0, textureCoords).r;"
        "} else if (texIndex == 1) {"
           "texColor = sample(sdf_atlas_1, textureCoords).r;"
        "} else if (texIndex == 2) {"
           "texColor = sample(sdf_atlas_2, textureCoords).r;"
        "} else if (texIndex == 3) {"
           "texColor = sample(sdf_atlas_3, textureCoords).r;"
        "} else {"
           "texColor = sample(sdf_atlas_0, textureCoords).r;"
        "}"
        // The distance field is constructed as uchar8_t values, so that the zero value is at 128,
        // and the supported range of distances is [-4 * 127/128, 4].
        // Hence to convert to floats our multiplier (width of the range) is 4 * 255/128 = 7.96875
        // and zero threshold is 128/255 = 0.50196078431.
        "half distance = 7.96875*(texColor - 0.50196078431);"

        // We may further adjust the distance for gamma correction.
        "distance -= half(distanceAdjust);"

        // After the distance is unpacked, we need to correct it by a factor dependent on the
        // current transformation. For general transforms, to determine the amount of correction
        // we multiply a unit vector pointing along the SDF gradient direction by the Jacobian of
        // unormTexCoords (which is the inverse transform for this fragment) and take the length of
        // the result.
        "half2 dist_grad = half2(float2(dFdx(distance), dFdy(distance)));"
            "half dg_len2 = dot(dist_grad, dist_grad);"

        // The length of the gradient may be near 0, so we need to check for that. This also
        // compensates for the Adreno, which likes to drop tiles on division by 0
        "if (dg_len2 < 0.0001) {"
            "dist_grad = half2(0.7071, 0.7071);"
        "} else {"
            "dist_grad = dist_grad*half(inversesqrt(dg_len2));"
        "}"

        // Computing the Jacobian and multiplying by the gradient.
        "half2 Jdx = half2(dFdx(unormTexCoords));"
        "half2 Jdy = half2(dFdy(unormTexCoords));"
        "half2 grad = half2(dist_grad.x*Jdx.x + dist_grad.y*Jdy.x,"
                           "dist_grad.x*Jdx.y + dist_grad.y*Jdy.y);"

        // This gives us a smooth step across approximately one fragment.
        "half afwidth = 0.65*length(grad);"
        // TODO: handle aliased and sRGB rendering
        "half val = smoothstep(-afwidth, afwidth, distance);"
        "outputCoverage = half4(val);";
}

void SDFTextRenderStep::writeVertices(DrawWriter* dw,
                                      const DrawParams& params,
                                      int ssboIndex) const {
    const SubRunData& subRunData = params.geometry().subRunData();
    subRunData.subRun()->fillInstanceData(dw, subRunData.startGlyphIndex(), subRunData.glyphCount(),
                                          ssboIndex, params.order().depthAsFloat());
}

void SDFTextRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                 PipelineDataGatherer* gatherer) const {
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
    SkV2 atlasDimensionsInverse = {1.f/proxies[0]->dimensions().width(),
                                   1.f/proxies[0]->dimensions().height()};
    gatherer->write(atlasDimensionsInverse);

    // TODO: get this from DistanceFieldAdjustTable and luminance color (set in SubRunData?)
    float gammaCorrection = 0.f;
    gatherer->write(gammaCorrection);

    // write textures and samplers
    const SkSamplingOptions kSamplingOptions(SkFilterMode::kLinear);
    constexpr SkTileMode kTileModes[2] = { SkTileMode::kClamp, SkTileMode::kClamp };
    for (unsigned int i = 0; i < numProxies; ++i) {
        gatherer->add(kSamplingOptions, kTileModes, proxies[i]);
    }
    // If the atlas has less than 4 active proxies we still need to set up samplers for the shader.
    for (unsigned int i = numProxies; i < kNumSDFAtlasTextures; ++i) {
        gatherer->add(kSamplingOptions, kTileModes, proxies[0]);
    }
}

}  // namespace skgpu::graphite

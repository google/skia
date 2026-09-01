/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/EndCapRenderStep.h"

#include "include/core/SkPath.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkDebug.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/sksl/SkSLString.h"

#include <algorithm>

namespace skgpu::graphite {

EndCapRenderStep::EndCapRenderStep(Layout layout)
        : RenderStep(layout,
                     RenderStepID::kEndCap,
                     Flags::kAppendInstances | Flags::kEmitsCoverage | Flags::kPerformsShading |
                             Flags::kHasTextures,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthLessPass,
                     /*vertexAttrs=*/{},
                     /*instanceAttrs=*/
                     {{"tileBounds", VertexAttribType::kFloat4, SkSLType::kFloat4},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt},
                      {"mask", VertexAttribType::kUInt, SkSLType::kUInt},
                      {"texPage", VertexAttribType::kUInt, SkSLType::kUInt},
                      {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3}},
                     /*storageUniforms=*/{},
                     /*varyings=*/
                     {{{"rectCoord", SkSLType::kFloat2},
                       {"maskIndex", SkSLType::kUInt},
                       {"texPageVarying", SkSLType::kInt}}}) {}

EndCapRenderStep::~EndCapRenderStep() {}

std::string EndCapRenderStep::vertexSkSL(const RootNodesInfo&) const {
    return "float2 position = float2(sk_VertexID < 2 ? tileBounds.x : tileBounds.z,"
                                    "(sk_VertexID % 2) == 0 ? tileBounds.y : tileBounds.w);"
           "float2 rectCoordAttr = float2(sk_VertexID < 2 ? 0.0 : (tileBounds.z - tileBounds.x),"
                                        "(sk_VertexID % 2) == 0 ? 0.0 : (tileBounds.w - "
                                        "tileBounds.y));"
           "float4 devPosition = endcap_vertex_fn(position, rectCoordAttr, depth, mask, texPage, "
                                                 "maskIndex, texPageVarying, rectCoord);"
           "float3x3 invM = float3x3(mat0, mat1, mat2);"
           "float3 localCoords = invM * float3(position, 1.0);"
           "stepLocalCoords = localCoords.xy / localCoords.z;";
}

std::string EndCapRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    std::string result;
    for (int i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        result += EmitSamplerLayout(bindingReqs, nextBindingIndex);
        SkSL::String::appendf(&result, " sampler2D alpha_text_%d;", i);
    }
    return result;
}

const char* EndCapRenderStep::fragmentCoverageSkSL() const {
    return "outputCoverage = endcap_coverage_fn(rectCoord,"
                                               "alpha_text_0,"
                                               "alpha_text_1,"
                                               "maskIndex,"
                                               "texPageVarying);";
}

void EndCapRenderStep::writeVertices(DrawWriter* writer,
                                     StorageContext* /*storageContext*/,
                                     const DrawParams& params,
                                     uint32_t ssboIndex) const {
    SkASSERT(params.geometry().isEndCaps());
    const auto& caps = params.geometry().endCaps().caps();
    if (caps.empty()) {
        return;
    }

    SkM44 invM;
    if (!params.transform().matrix().invert(&invM)) {
        return;
    }

    DrawWriter::Instances instances{*writer, {}, {}, 4};
    instances.reserve(caps.size());

    float depth = params.order().depthAsFloat();
    float height = static_cast<float>(SparseStripConfig::kTileHeight);

    for (const auto& cap : caps) {
        float l = static_cast<float>(cap.fX);
        float t = static_cast<float>(cap.fY);
        float width = static_cast<float>(cap.fWidth);

        float r = l + width;
        float b = t + height;

        instances.append(1) << l << t << r << b << depth << ssboIndex << cap.fAlphaIndex
                            << static_cast<uint32_t>(cap.fTexPage)
                            << invM.rc(0, 0) << invM.rc(1, 0) << invM.rc(3, 0)  // mat0
                            << invM.rc(0, 1) << invM.rc(1, 1) << invM.rc(3, 1)  // mat1
                            << invM.rc(0, 3) << invM.rc(1, 3) << invM.rc(3, 3); // mat2
    }
}

void EndCapRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(gatherer->checkRewind());
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    SkASSERT(params.geometry().isEndCaps());
    const auto& proxies = params.geometry().endCaps().proxies();
    SkASSERT(!proxies.empty());

    for (int i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        int proxyIdx = std::min(i, static_cast<int>(proxies.size()) - 1);
        gatherer->add(proxies[proxyIdx], {SkFilterMode::kNearest, SkTileMode::kClamp});
    }
}

}  // namespace skgpu::graphite

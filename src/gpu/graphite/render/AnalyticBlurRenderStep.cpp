/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/AnalyticBlurRenderStep.h"

#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

namespace skgpu::graphite {

AnalyticBlurRenderStep::AnalyticBlurRenderStep()
        : RenderStep("AnalyticBlurRenderStep",
                     "",
                     Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage,
                     /*uniforms=*/
                     {{"localToDevice", SkSLType::kFloat4x4},
                      {"deviceToScaledShape", SkSLType::kFloat3x3},
                      {"shapeData", SkSLType::kFloat4},
                      {"blurData", SkSLType::kHalf2},
                      {"shapeType", SkSLType::kInt},
                      {"depth", SkSLType::kFloat}},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGreaterPass,
                     /*vertexAttrs=*/
                     {{"position", VertexAttribType::kFloat2, SkSLType::kFloat2},
                      {"ssboIndices", VertexAttribType::kUShort2, SkSLType::kUShort2}},
                     /*instanceAttrs=*/{},
                     /*varyings=*/
                     // scaledShapeCoords are the fragment coordinates in local shape space, where
                     // the shape has been scaled to device space but not translated or rotated.
                     {{"scaledShapeCoords", SkSLType::kFloat2}}) {}

std::string AnalyticBlurRenderStep::vertexSkSL() const {
    return R"(
        float4 devPosition = localToDevice * float4(position, depth, 1.0);
        stepLocalCoords = position;
        scaledShapeCoords = (deviceToScaledShape * devPosition.xy1).xy;
    )";
}

std::string AnalyticBlurRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    return EmitSamplerLayout(bindingReqs, nextBindingIndex) + " sampler2D s;";
}

const char* AnalyticBlurRenderStep::fragmentCoverageSkSL() const {
    return "outputCoverage = blur_coverage_fn(scaledShapeCoords, "
                                             "shapeData, "
                                             "blurData, "
                                             "shapeType, "
                                             "s);";
}

void AnalyticBlurRenderStep::writeVertices(DrawWriter* writer,
                                           const DrawParams& params,
                                           skvx::ushort2 ssboIndices) const {
    const Rect& r = params.geometry().analyticBlurMask().drawBounds();
    DrawWriter::Vertices verts{*writer};
    verts.append(4) << skvx::float2(r.left(), r.top()) << ssboIndices
                    << skvx::float2(r.right(), r.top()) << ssboIndices
                    << skvx::float2(r.left(), r.bot()) << ssboIndices
                    << skvx::float2(r.right(), r.bot()) << ssboIndices;
}

void AnalyticBlurRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                      PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    gatherer->write(params.transform().matrix());

    const AnalyticBlurMask& blur = params.geometry().analyticBlurMask();
    gatherer->write(blur.deviceToScaledShape().asM33());
    gatherer->write(blur.shapeData().asSkRect());
    gatherer->writeHalf(blur.blurData());
    gatherer->write(static_cast<int>(blur.shapeType()));
    gatherer->write(params.order().depthAsFloat());

    SkSamplingOptions samplingOptions = blur.shapeType() == AnalyticBlurMask::ShapeType::kRect
                                                ? SkFilterMode::kLinear
                                                : SkFilterMode::kNearest;
    constexpr SkTileMode kTileModes[2] = {SkTileMode::kClamp, SkTileMode::kClamp};
    gatherer->add(samplingOptions, kTileModes, blur.refProxy());
}

}  // namespace skgpu::graphite

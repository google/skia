/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/TessellateWedgesRenderStep.h"

#include "src/sksl/SkSLString.h"

#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/render/DynamicInstancesPatchAllocator.h"

#include "src/gpu/tessellate/FixedCountBufferUtils.h"
#include "src/gpu/tessellate/MidpointContourParser.h"
#include "src/gpu/tessellate/PatchWriter.h"

namespace skgpu::graphite {

namespace {

using namespace skgpu::tess;

// Only kFanPoint, no stroke params, since this is for filled wedges.
// No color or wide color attribs, since it might always be part of the PaintParams
// or we'll add a color-only fast path to RenderStep later.
// No explicit curve type on platforms that support infinity.
static constexpr PatchAttribs kAttribs = PatchAttribs::kFanPoint |
                                         PatchAttribs::kPaintDepth |
                                         PatchAttribs::kSsboIndex;
static constexpr PatchAttribs kAttribsWithCurveType = kAttribs | PatchAttribs::kExplicitCurveType;

using Writer = PatchWriter<DynamicInstancesPatchAllocator<FixedCountWedges>,
                           Required<PatchAttribs::kFanPoint>,
                           Required<PatchAttribs::kPaintDepth>,
                           Required<PatchAttribs::kSsboIndex>,
                           Optional<PatchAttribs::kExplicitCurveType>>;

// The order of the attribute declarations must match the order used by
// PatchWriter::emitPatchAttribs, i.e.:
//     join << fanPoint << stroke << color << depth << curveType << ssboIndices
static constexpr Attribute kBaseAttributes[] = {
        {"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"fanPointAttrib", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
        {"ssboIndices", VertexAttribType::kUShort2, SkSLType::kUShort2}};

static constexpr Attribute kAttributesWithCurveType[] = {
        {"p01", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"p23", VertexAttribType::kFloat4, SkSLType::kFloat4},
        {"fanPointAttrib", VertexAttribType::kFloat2, SkSLType::kFloat2},
        {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
        {"curveType", VertexAttribType::kFloat, SkSLType::kFloat},
        {"ssboIndices", VertexAttribType::kUShort2, SkSLType::kUShort2}};

static constexpr SkSpan<const Attribute> kAttributes[2] = {kAttributesWithCurveType,
                                                           kBaseAttributes};

}  // namespace

TessellateWedgesRenderStep::TessellateWedgesRenderStep(std::string_view variantName,
                                                       bool infinitySupport,
                                                       DepthStencilSettings depthStencilSettings,
                                                       StaticBufferManager* bufferManager)
        : RenderStep("TessellateWedgesRenderStep",
                     variantName,
                     Flags::kRequiresMSAA |
                     (depthStencilSettings.fDepthWriteEnabled ? Flags::kPerformsShading
                                                              : Flags::kNone),
                     /*uniforms=*/{{"localToDevice", SkSLType::kFloat4x4}},
                     PrimitiveType::kTriangles,
                     depthStencilSettings,
                     /*vertexAttrs=*/  {{"resolveLevel_and_idx",
                                         VertexAttribType::kFloat2, SkSLType::kFloat2}},
                     /*instanceAttrs=*/kAttributes[infinitySupport])
        , fInfinitySupport(infinitySupport) {
    SkASSERT(this->instanceStride() ==
             PatchStride(infinitySupport ? kAttribs : kAttribsWithCurveType));

    // Initialize the static buffers we'll use when recording draw calls.
    // NOTE: Each instance of this RenderStep gets its own copy of the data. If this ends up causing
    // problems, we can modify StaticBufferManager to de-duplicate requests.
    const size_t vertexSize = FixedCountWedges::VertexBufferSize();
    auto vertexData = bufferManager->getVertexWriter(vertexSize, &fVertexBuffer);
    FixedCountWedges::WriteVertexBuffer(std::move(vertexData), vertexSize);

    const size_t indexSize = FixedCountWedges::IndexBufferSize();
    auto indexData = bufferManager->getIndexWriter(indexSize, &fIndexBuffer);
    FixedCountWedges::WriteIndexBuffer(std::move(indexData), indexSize);
}

TessellateWedgesRenderStep::~TessellateWedgesRenderStep() {}

std::string TessellateWedgesRenderStep::vertexSkSL() const {
    return SkSL::String::printf(
            R"(
                float2 localCoord;
                if (resolveLevel_and_idx.x < 0) {
                    // A negative resolve level means this is the fan point.
                    localCoord = fanPointAttrib;
                } else {
                    // TODO: Approximate perspective scaling to match how PatchWriter is configured
                    // (or provide explicit tessellation level in instance data instead of
                    // replicating work)
                    float2x2 vectorXform = float2x2(localToDevice[0].xy, localToDevice[1].xy);
                    localCoord = tessellate_filled_curve(
                        vectorXform, resolveLevel_and_idx.x, resolveLevel_and_idx.y, p01, p23, %s);
                }
                float4 devPosition = localToDevice * float4(localCoord, 0.0, 1.0);
                devPosition.z = depth;
                stepLocalCoords = localCoord;
            )",
            fInfinitySupport ? "curve_type_using_inf_support(p23)" : "curveType");
}

void TessellateWedgesRenderStep::writeVertices(DrawWriter* dw,
                                               const DrawParams& params,
                                               skvx::ushort2 ssboIndices) const {
    SkPath path = params.geometry().shape().asPath(); // TODO: Iterate the Shape directly

    int patchReserveCount = FixedCountWedges::PreallocCount(path.countVerbs());
    Writer writer{fInfinitySupport ? kAttribs : kAttribsWithCurveType,
                  *dw,
                  fVertexBuffer,
                  fIndexBuffer,
                  patchReserveCount};
    writer.updatePaintDepthAttrib(params.order().depthAsFloat());
    writer.updateSsboIndexAttrib(ssboIndices);

    // The vector xform approximates how the control points are transformed by the shader to
    // more accurately compute how many *parametric* segments are needed.
    // TODO: This doesn't account for perspective division yet, which will require updating the
    // approximate transform based on each verb's control points' bounding box.
    SkASSERT(params.transform().type() < Transform::Type::kPerspective);
    writer.setShaderTransform(wangs_formula::VectorXform{params.transform().matrix()},
                              params.transform().maxScaleFactor());

    // TODO: Essentially the same as PathWedgeTessellator::write_patches but with a different
    // PatchWriter template.
    // For wedges, we iterate over each contour explicitly, using a fan point position that is in
    // the midpoint of the current contour.
    MidpointContourParser parser{path};
    while (parser.parseNextContour()) {
        writer.updateFanPointAttrib(parser.currentMidpoint());
        SkPoint lastPoint = {0, 0};
        SkPoint startPoint = {0, 0};
        for (auto [verb, pts, w] : parser.currentContour()) {
            switch (verb) {
                case SkPathVerb::kMove:
                    startPoint = lastPoint = pts[0];
                    break;
                case SkPathVerb::kLine:
                    // Unlike curve tessellation, wedges have to handle lines as part of the patch,
                    // effectively forming a single triangle with the fan point.
                    writer.writeLine(pts[0], pts[1]);
                    lastPoint = pts[1];
                    break;
                case SkPathVerb::kQuad:
                    writer.writeQuadratic(pts);
                    lastPoint = pts[2];
                    break;
                case SkPathVerb::kConic:
                    writer.writeConic(pts, *w);
                    lastPoint = pts[2];
                    break;
                case SkPathVerb::kCubic:
                    writer.writeCubic(pts);
                    lastPoint = pts[3];
                    break;
                default: break;
            }
        }

        // Explicitly close the contour with another line segment, which also differs from curve
        // tessellation since that approach's triangle step automatically closes the contour.
        if (lastPoint != startPoint) {
            writer.writeLine(lastPoint, startPoint);
        }
    }
}

void TessellateWedgesRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                          PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    gatherer->write(params.transform().matrix());
}

}  // namespace skgpu::graphite

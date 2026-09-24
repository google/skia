/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/AnalyticRRectBlurRenderStep.h"

#include "include/core/SkM44.h"
#include "include/core/SkRRect.h"
#include "include/private/SkDebug.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/geom/AnalyticRRectBlurMask.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

namespace skgpu::graphite {

struct Vertex {
    // x and y determine our index into xBounds and yBounds respectively, within the range [0, 5].
    // z and w determine if we should apply our bevel and the direction, expected to be -1, 0, or 1.
    // The bevel coefficient is multiplied by the bevel fraction and the radii of the current cell
    // (see analytic_rrect_blur_vertex_fn), this bevel is only set and applied for the two
    // outermost corner vertices in the corner cells.
    int32_t fGridAndBevel[4];

    // The cell index of the current cell. Bits 28-31 are set if the current cell is a corner safe
    // edge. The encoding from least significant to most significant is kept the same as
    // `canSaturateEdge`: left, right, top, and bottom.
    uint32_t fCellID;
};

static constexpr int kVertexCount = 104;
static constexpr int kIndexCount = 162;

// We construct a 25 cell grid (5x5) for our rrect, with 6 distinct boundaries for each axis:
// - Outset blur padding: the provided rrect's bounding box outset by 3 sigma.
// - Inset edge: the provided rrect's bounding box inset by 3.5 sigma.
// - Safe bounds: The furthest point from the edge which is inset by the deepest corner radii on
//                for that edge, then inset by an additional 3 sigma.
//
// This is an example of the layout of vertices for a rounded rect with a non-zero corner radius
// and a low sigma, where the corner radius plus three sigma from the edge of the rect doesn't
// exceed the halfway point of the rect (5x5 grid):
//
//      x0   x1       x2      x3       x4   x5
//      |    |        |        |        |    |
// y0 --+----+--------+--------+--------+----+-- Outset blur padding
//      |    |        |        |        |    |
// y1 --+----+--------+--------+--------+----+-- Inset edge
//      |    |        | Inside |        |    |
// y2 --+----+--------+--------+--------+----+-- Safe bounds
//      |    | Inside | Inside | Inside |    |
// y3 --+----+--------+--------+--------+----+-- Safe Bounds
//      |    |        | Inside |        |    |
// y4 --+----+--------+--------+--------+----+-- Inset edge
//      |    |        |        |        |    |
// y5 --+----+--------+--------+--------+----+-- Outset blur padding
//
// The labels on the y-axis also apply to the x-axis as well, this is the ideal case where we can
// classify 5 cells as fully saturated (not affected by the blur), so we can skip evaluating the
// fragment shader for those pixels since we guarantee they won't be affected by the blur.
// For the outermost edge cells between x2 and x3, or y2 and y3, we can further simplify the blur
// evaluation given the blur radius 3 sigma away doesn't go past the opposite edge of the rect. This
// allows us to simplify the CDF evaluation for that edge since we know the opposite edge CDF
// evalation is fully saturated (either 0 or 1), and for the other axis, we are fully
// within the rounded rect.
//
// In the case that the sigma is very large or the radii leaves no room for a straight edge on an
// axis, the inset and/or safe bounds are snapped to the middle of that axis and will result in
// degenerate triangles which can be ignored.
//
// Example of the resulting vertices with a large corner radius (4x4 grid):
//
//        x0    x1 x2/x3 x4   x5
//         |    |    |    |    |
//    y0 --+----+----+----+----+-- Outset blur padding
//         |    |    |    |    |
//    y1 --+----+----+----+----+-- Inset edge
//         |    |    |    |    |
// y2/y3 --+----+----+----+----+-- Safe Bounds
//         |    |    |    |    |
//    y4 --+----+----+----+----+-- Inset edge
//         |    |    |    |    |
//    y5 --+----+----+----+----+-- Outset blur padding
//
// Example of the resulting verices with a large sigma (2x2 grid):
//
//              x0  x1/x2/x3/x4  x5
//               |       |       |
//          y0 --+-------+-------+-- Outset blur padding
//               |       |       |
// y1/y2/y3/y4 --+-------+-------+-- Inset edge/safe Bounds
//               |       |       |
//          y5 --+-------+-------+-- Outset blur padding
//
// Since there is no area between the safe bounds for the 4x4 case, we cannot classify any inner
// cell as fully saturated as its possible that the corner's blur may affect one of the inner cells.
// For the 2x2 case, we are only left with the corner cells so every pixel may be affected by the
// blur.
//
// For each corner cell, we perform beveling dependent on the corner radius to reduce the number
// of pixels we must evaluate for the blur. For this, we use the following template of 5 vertices
// and 3 triangles, where v0 and v4 have a non-zero bevel value associated with them:
//
//      v0-----v1
//     / \  f0  |
//    /   \__   |
//   v4  f1  \  |
//   | \____  \ |
//   |  f2  \__\|
//   v3--------v2
//
static void write_vertex_buffer(VertexWriter writer) {
    if (!writer) return;

    // Corner 0: TL, cell 0.
    writer << Vertex{{0, 0,  1,  0}, 0}   // v0
           << Vertex{{1, 0,  0,  0}, 0}   // v1
           << Vertex{{1, 1,  0,  0}, 0}   // v2
           << Vertex{{0, 1,  0,  0}, 0}   // v3
           << Vertex{{0, 0,  0,  1}, 0};  // v4

    // Corner 1: TR, cell 4.
    writer << Vertex{{4, 0,  0,  0}, 4}   // v5
           << Vertex{{5, 0, -1,  0}, 4}   // v6
           << Vertex{{5, 0,  0,  1}, 4}   // v7
           << Vertex{{5, 1,  0,  0}, 4}   // v8
           << Vertex{{4, 1,  0,  0}, 4};  // v9

    // Corner 2: BR, cell 24.
    writer << Vertex{{4, 4,  0,  0}, 24}  // v10
           << Vertex{{5, 4,  0,  0}, 24}  // v11
           << Vertex{{5, 5,  0, -1}, 24}  // v12
           << Vertex{{5, 5, -1,  0}, 24}  // v13
           << Vertex{{4, 5,  0,  0}, 24}; // v14

    // Corner 3: BL, cell 20.
    writer << Vertex{{0, 4,  0,  0}, 20}  // v15
           << Vertex{{1, 4,  0,  0}, 20}  // v16
           << Vertex{{1, 5,  0,  0}, 20}  // v17
           << Vertex{{0, 5,  1,  0}, 20}  // v18
           << Vertex{{0, 5,  0, -1}, 20}; // v19

    // 21 quads for the remaining cells.
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            // Skip corners.
            if ((row == 0 && col == 0) || (row == 0 && col == 4) ||
                (row == 4 && col == 0) || (row == 4 && col == 4)) {
                continue;
            }

            uint32_t cId = row * 5 + col;
            int x0 = col;
            int x1 = col + 1;
            int y0 = row;
            int y1 = row + 1;

            // Encode the corner safe edge bit.
            if (row == 2) {
                if (col == 0) {
                    cId |= 1 << 28;  // Left
                } else if (col == 4) {
                    cId |= 1 << 29;  // Right
                }
            } else if (col == 2) {
                if (row == 0) {
                    cId |= 1 << 30;  // Top
                } else if (row == 4) {
                    cId |= 1 << 31;  // Bottom
                }
            }

            writer << Vertex{{x0, y0, 0, 0}, cId}
                   << Vertex{{x1, y0, 0, 0}, cId}
                   << Vertex{{x1, y1, 0, 0}, cId}
                   << Vertex{{x0, y1, 0, 0}, cId};
        }
    }
}

static void write_index_buffer(VertexWriter writer) {
    if (!writer) return;

    // Corner 0: TL Corner, fan from v2.
    writer << uint16_t(2) << uint16_t(0) << uint16_t(1)
           << uint16_t(2) << uint16_t(4) << uint16_t(0)
           << uint16_t(2) << uint16_t(3) << uint16_t(4);

    // Corner 1: TR Corner, fan from v9.
    writer << uint16_t(9) << uint16_t(5) << uint16_t(6)
           << uint16_t(9) << uint16_t(6) << uint16_t(7)
           << uint16_t(9) << uint16_t(7) << uint16_t(8);

    // Corner 2: BR Corner, fan from v10.
    writer << uint16_t(10) << uint16_t(11) << uint16_t(12)
           << uint16_t(10) << uint16_t(12) << uint16_t(13)
           << uint16_t(10) << uint16_t(13) << uint16_t(14);

    // Corner 3: BL Corner, fan from v16.
    writer << uint16_t(16) << uint16_t(17) << uint16_t(18)
           << uint16_t(16) << uint16_t(18) << uint16_t(19)
           << uint16_t(16) << uint16_t(19) << uint16_t(15);

    // Create remaining quads.
    uint16_t base = 20;
    for (int i = 0; i < 21; i++) {
        writer << uint16_t(base + 0) << uint16_t(base + 1) << uint16_t(base + 3)
               << uint16_t(base + 1) << uint16_t(base + 2) << uint16_t(base + 3);
        base += 4;
    }
}

AnalyticRRectBlurRenderStep::AnalyticRRectBlurRenderStep(Layout layout,
                                                         StaticBufferManager* bufferManager)
        : RenderStep(layout,
                     RenderStepID::kAnalyticRRectBlur,
                     Flags::kPerformsShading | Flags::kHasTextures | Flags::kEmitsCoverage |
                     Flags::kAppendInstances,
                     /*uniforms=*/
                     {{"rect", SkSLType::kFloat4},
                      {"drawPad", SkSLType::kFloat2},
                      {"sqrtHalfOverSigma", SkSLType::kHalf2},
                      {"rrectRadii", SkSLType::kFloat4, 2},
                      {"blurRadius", SkSLType::kFloat2}},
                     PrimitiveType::kTriangles,
                     kDirectDepthLessPass,
                     /*staticAttrs=*/
                     {{"gridAndBevel", VertexAttribType::kInt4, SkSLType::kInt4},
                      {"cellID", VertexAttribType::kUInt, SkSLType::kUInt}},
                     /*appendAttrs=*/
                     {{"bounds0", VertexAttribType::kFloat4, SkSLType::kFloat4},
                      {"bounds1", VertexAttribType::kFloat4, SkSLType::kFloat4},
                      {"bounds2", VertexAttribType::kFloat4, SkSLType::kFloat4},
                      {"cornerSafeBounds", VertexAttribType::kFloat4, SkSLType::kFloat4},
                      {"canSaturateEdge", VertexAttribType::kUInt, SkSLType::kUInt},
                      {"localToDevice0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"localToDevice1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"localToDevice2", VertexAttribType::kFloat3, SkSLType::kFloat3},
                      {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                      {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}},
                     /*storageUniforms=*/{},
                     /*varyings=*/
                     {{"scaledShapeCoords", SkSLType::kFloat2},
                      {"vFlags", SkSLType::kHalf4}}) {
    write_vertex_buffer(bufferManager->getVertexWriter(kVertexCount, sizeof(Vertex),
                                                       &fVertexBuffer));
    write_index_buffer(bufferManager->getIndexWriter(sizeof(uint16_t) * kIndexCount,
                                                     &fIndexBuffer));
}

std::string AnalyticRRectBlurRenderStep::vertexSkSL(const RootNodesInfo&) const {
    return "float4 devPosition = analytic_rrect_blur_vertex_fn("
                   "gridAndBevel, cellID, rect, "
                   "bounds0, bounds1, bounds2, "
                   "cornerSafeBounds, canSaturateEdge, drawPad, rrectRadii, depth, "
                   "localToDevice0, localToDevice1, localToDevice2, "
                   "scaledShapeCoords, vFlags, stepLocalCoords);\n";
}

std::string AnalyticRRectBlurRenderStep::texturesAndSamplersSkSL(
        const ResourceBindingRequirements& bindingReqs, int* nextBindingIndex) const {
    return EmitSamplerLayout(bindingReqs, nextBindingIndex) + " sampler2D cdfLut;";
}

const char* AnalyticRRectBlurRenderStep::fragmentCoverageSkSL() const {
    return "outputCoverage = analytic_rrect_blur_coverage_fn(scaledShapeCoords, "
                                                            "vFlags, "
                                                            "rect, "
                                                            "sqrtHalfOverSigma, "
                                                            "rrectRadii, "
                                                            "blurRadius, "
                                                            "cdfLut);";
}

void AnalyticRRectBlurRenderStep::writeVertices(DrawWriter* writer,
                                                StorageContext* /*storageContext*/,
                                                const DrawParams& params,
                                                uint32_t ssboIndex) const {
    const AnalyticRRectBlurMask& blur = params.geometry().analyticRRectBlurMask();
    SkRect rect = blur.rrect().getBounds();
    SkSpan<const SkVector> radii = blur.rrect().radii();

    float drawPadX = blur.drawPadX();
    float drawPadY = blur.drawPadY();
    float satPadX = std::ceil(3.5f * blur.localSigma().x);
    float satPadY = std::ceil(3.5f * blur.localSigma().y);

    // Calculate distance from edge where the corner curvature has ended and is saturated.
    float safeOffsetLeft = std::max(radii[0].fX, radii[3].fX) + drawPadX;
    float safeOffsetRight = std::max(radii[1].fX, radii[2].fX) + drawPadX;
    float safeOffsetTop = std::max(radii[0].fY, radii[1].fY) + drawPadY;
    float safeOffsetBottom = std::max(radii[2].fY, radii[3].fY) + drawPadY;

    float cornerSafeXMin = rect.fLeft + safeOffsetLeft;
    float cornerSafeXMax = rect.fRight - safeOffsetRight;
    float cornerSafeYMin = rect.fTop + safeOffsetTop;
    float cornerSafeYMax = rect.fBottom - safeOffsetBottom;

    // Innermost safe bounds that we define as our fully saturated bounds.
    float insXMin = rect.fLeft + std::max(satPadX, safeOffsetLeft);
    float insXMax = rect.fRight - std::max(satPadX, safeOffsetRight);
    float insYMin = rect.fTop + std::max(satPadY, safeOffsetTop);
    float insYMax = rect.fBottom - std::max(satPadY, safeOffsetBottom);

    // Snap innermost inset bounds to the center if they are overlapping.
    if (insXMin >= insXMax) {
        insXMin = insXMax = (rect.fLeft + rect.fRight) * 0.5f;
    }
    if (insYMin >= insYMax) {
        insYMin = insYMax = (rect.fTop + rect.fBottom) * 0.5f;
    }

    // Our outermost edge safe inset bounds. This allows us to assume full coverage when we are
    // far enough from an edge on one axis and within the corner radius safe limits on the other
    // axis.
    float edgeInsXMin = std::min(insXMin, rect.fLeft + std::min(satPadX, safeOffsetLeft));
    float edgeInsXMax = std::max(insXMax, rect.fRight - std::min(satPadX, safeOffsetRight));
    float edgeInsYMin = std::min(insYMin, rect.fTop + std::min(satPadY, safeOffsetTop));
    float edgeInsYMax = std::max(insYMax, rect.fBottom - std::min(satPadY, safeOffsetBottom));

    // Bounds mapping to each row and columns of our 5x5 vertex grid.
    const float xBounds[6] = {rect.fLeft - drawPadX, edgeInsXMin, insXMin,
                              insXMax, edgeInsXMax, rect.fRight + drawPadX};
    const float yBounds[6] = {rect.fTop - drawPadY,  edgeInsYMin, insYMin,
                              insYMax, edgeInsYMax, rect.fBottom + drawPadY};

    const SkM44& mat = params.transform().matrix();

    // Bitmask determining which edges we can assume are fully saturated by the opposite edge.
    // Dependent on the current edge cell's blur not going past the opposite edge of the rect.
    // In order from least significant to most significant: left, right, top, and bottom.
    uint32_t canSaturateEdge = 0;
    if (edgeInsXMin + drawPadX < rect.fRight) {
        canSaturateEdge |= 1;
    }
    if (edgeInsXMax - drawPadX > rect.fLeft) {
        canSaturateEdge |= 2;
    }
    if (edgeInsYMin + drawPadY < rect.fBottom) {
        canSaturateEdge |= 4;
    }
    if (edgeInsYMax - drawPadY > rect.fTop) {
        canSaturateEdge |= 8;
    }

    DrawWriter::Instances instances{*writer, fVertexBuffer, fIndexBuffer, kIndexCount};
    instances.append(1) << VertexWriter::Array(xBounds, 6)               // bounds0, bounds1
                        << VertexWriter::Array(yBounds, 6)               // bounds2
                        << cornerSafeXMin << cornerSafeYMin
                        << cornerSafeXMax << cornerSafeYMax              // cornerSafeBounds
                        << canSaturateEdge
                        << mat.rc(0, 0) << mat.rc(1, 0) << mat.rc(3, 0)  // localToDevice0
                        << mat.rc(0, 1) << mat.rc(1, 1) << mat.rc(3, 1)  // localToDevice1
                        << mat.rc(0, 3) << mat.rc(1, 3) << mat.rc(3, 3)  // localToDevice2
                        << params.order().depthAsFloat()
                        << ssboIndex;
}

void AnalyticRRectBlurRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                                          PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)

    const AnalyticRRectBlurMask& blur = params.geometry().analyticRRectBlurMask();

    const SkRRect& rrect = blur.rrect();
    SkSpan<const SkVector> radii = rrect.radii();

    SkV2 localSigma = blur.localSigma();
    SkV2 sqrtHalfOverSigma = {(1.f / SK_FloatSqrt2) / localSigma.x,
                              (1.f / SK_FloatSqrt2) / localSigma.y};
    SkSize blurRadius = {static_cast<SkScalar>(std::floor(std::ceil(6.f * localSigma.x) / 2.0)),
                         static_cast<SkScalar>(std::floor(std::ceil(6.f * localSigma.y) / 2.0))};

    gatherer->write(rrect.getBounds());
    gatherer->write(blur.drawPad());
    gatherer->writeHalf(sqrtHalfOverSigma);
    const SkV4 radiiArr[2] = {SkV4{radii[0].fX, radii[0].fY, radii[1].fX, radii[1].fY},
                              SkV4{radii[2].fX, radii[2].fY, radii[3].fX, radii[3].fY}};
    gatherer->writeArray(SkSpan(radiiArr, 2));
    gatherer->write(blurRadius);

    gatherer->add(blur.refCdfProxy(), {SkFilterMode::kLinear, SkTileMode::kClamp});
}

}  // namespace skgpu::graphite

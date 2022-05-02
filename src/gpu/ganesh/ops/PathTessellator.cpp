/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/PathTessellator.h"

#include "src/core/SkPathPriv.h"
#include "src/gpu/ganesh/GrMeshDrawTarget.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/tessellate/AffineMatrix.h"
#include "src/gpu/tessellate/FixedCountBufferUtils.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/MidpointContourParser.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/WangsFormula.h"

namespace skgpu::v1 {

namespace {

using CurveWriter = PatchWriter<GrVertexChunkBuilder,
                                Optional<PatchAttribs::kColor>,
                                Optional<PatchAttribs::kWideColorIfEnabled>,
                                Optional<PatchAttribs::kExplicitCurveType>,
                                AddTrianglesWhenChopping,
                                DiscardFlatCurves>;

int write_curve_patches(CurveWriter&& patchWriter,
                        const SkMatrix& shaderMatrix,
                        const PathTessellator::PathDrawList& pathDrawList) {
    wangs_formula::VectorXform shaderXform(shaderMatrix);
    for (auto [pathMatrix, path, color] : pathDrawList) {
        AffineMatrix m(pathMatrix);
        if (patchWriter.attribs() & PatchAttribs::kColor) {
            patchWriter.updateColorAttrib(color);
        }
        for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
            switch (verb) {
                case SkPathVerb::kQuad: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto p2 = m.map1Point(pts+2);

                    patchWriter.writeQuadratic(p0, p1, p2, shaderXform);
                    break;
                }

                case SkPathVerb::kConic: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto p2 = m.map1Point(pts+2);

                    patchWriter.writeConic(p0, p1, p2, *w, shaderXform);
                    break;
                }

                case SkPathVerb::kCubic: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto [p2, p3] = m.map2Points(pts+2);

                    patchWriter.writeCubic(p0, p1, p2, p3, shaderXform);
                    break;
                }

                default: break;
            }
        }
    }

    return patchWriter.requiredResolveLevel();
}

using WedgeWriter = PatchWriter<GrVertexChunkBuilder,
                                Required<PatchAttribs::kFanPoint>,
                                Optional<PatchAttribs::kColor>,
                                Optional<PatchAttribs::kWideColorIfEnabled>,
                                Optional<PatchAttribs::kExplicitCurveType>>;

int write_wedge_patches(WedgeWriter&& patchWriter,
                        const SkMatrix& shaderMatrix,
                        const PathTessellator::PathDrawList& pathDrawList) {
    wangs_formula::VectorXform shaderXform(shaderMatrix);
    for (auto [pathMatrix, path, color] : pathDrawList) {
        AffineMatrix m(pathMatrix);
        if (patchWriter.attribs() & PatchAttribs::kColor) {
            patchWriter.updateColorAttrib(color);
        }
        MidpointContourParser parser(path);
        while (parser.parseNextContour()) {
            patchWriter.updateFanPointAttrib(m.mapPoint(parser.currentMidpoint()));
            SkPoint lastPoint = {0, 0};
            SkPoint startPoint = {0, 0};
            for (auto [verb, pts, w] : parser.currentContour()) {
                switch (verb) {
                    case SkPathVerb::kMove: {
                        startPoint = lastPoint = pts[0];
                        break;
                    }

                    case SkPathVerb::kLine: {
                        // Explicitly convert the line to an equivalent cubic w/ four distinct
                        // control points because it fans better and avoids double-hitting pixels.
                        patchWriter.writeLine(m.map2Points(pts));
                        lastPoint = pts[1];
                        break;
                    }

                    case SkPathVerb::kQuad: {
                        auto [p0, p1] = m.map2Points(pts);
                        auto p2 = m.map1Point(pts+2);

                        patchWriter.writeQuadratic(p0, p1, p2, shaderXform);
                        lastPoint = pts[2];
                        break;
                    }

                    case SkPathVerb::kConic: {
                        auto [p0, p1] = m.map2Points(pts);
                        auto p2 = m.map1Point(pts+2);

                        patchWriter.writeConic(p0, p1, p2, *w, shaderXform);
                        lastPoint = pts[2];
                        break;
                    }

                    case SkPathVerb::kCubic: {
                        auto [p0, p1] = m.map2Points(pts);
                        auto [p2, p3] = m.map2Points(pts+2);

                        patchWriter.writeCubic(p0, p1, p2, p3, shaderXform);
                        lastPoint = pts[3];
                        break;
                    }

                    case SkPathVerb::kClose: {
                        break;  // Ignore. We can assume an implicit close at the end.
                    }
                }
            }
            if (lastPoint != startPoint) {
                SkPoint pts[2] = {lastPoint, startPoint};
                patchWriter.writeLine(m.map2Points(pts));
            }
        }
    }

    return patchWriter.requiredResolveLevel();
}

}  // namespace

SKGPU_DECLARE_STATIC_UNIQUE_KEY(gFixedCountCurveVertexBufferKey);
SKGPU_DECLARE_STATIC_UNIQUE_KEY(gFixedCountCurveIndexBufferKey);

void PathCurveTessellator::prepareWithTriangles(
        GrMeshDrawTarget* target,
        const SkMatrix& shaderMatrix,
        GrInnerFanTriangulator::BreadcrumbTriangleList* extraTriangles,
        const PathDrawList& pathDrawList,
        int totalCombinedPathVerbCnt) {
    int patchPreallocCount = FixedCountCurves::PreallocCount(totalCombinedPathVerbCnt) +
                             (extraTriangles ? extraTriangles->count() : 0);
    if (patchPreallocCount) {
        CurveWriter writer{fAttribs, skgpu::kMaxParametricSegments,
                           target, &fVertexChunkArray, patchPreallocCount};

        // Write out extra space-filling triangles to connect the curve patches with any external
        // source of geometry (e.g. inner triangulation that handles winding explicitly).
        if (extraTriangles) {
            SkDEBUGCODE(int breadcrumbCount = 0;)
            for (const auto* tri = extraTriangles->head(); tri; tri = tri->fNext) {
                SkDEBUGCODE(++breadcrumbCount;)
                auto p0 = float2::Load(tri->fPts);
                auto p1 = float2::Load(tri->fPts + 1);
                auto p2 = float2::Load(tri->fPts + 2);
                if (skvx::any((p0 == p1) & (p1 == p2))) {
                    // Cull completely horizontal or vertical triangles. GrTriangulator can't always
                    // get these breadcrumb edges right when they run parallel to the sweep
                    // direction because their winding is undefined by its current definition.
                    // FIXME(skia:12060): This seemed safe, but if there is a view matrix it will
                    // introduce T-junctions.
                    continue;
                }
                writer.writeTriangle(p0, p1, p2);
            }
            SkASSERT(breadcrumbCount == extraTriangles->count());
        }

        int resolveLevel = write_curve_patches(std::move(writer), shaderMatrix, pathDrawList);
        this->updateResolveLevel(resolveLevel);
    }

    GrResourceProvider* rp = target->resourceProvider();

    SKGPU_DEFINE_STATIC_UNIQUE_KEY(gFixedCountCurveVertexBufferKey);

    fFixedVertexBuffer = rp->findOrMakeStaticBuffer(GrGpuBufferType::kVertex,
                                                    FixedCountCurves::VertexBufferSize(),
                                                    gFixedCountCurveVertexBufferKey,
                                                    FixedCountCurves::WriteVertexBuffer);

    SKGPU_DEFINE_STATIC_UNIQUE_KEY(gFixedCountCurveIndexBufferKey);

    fFixedIndexBuffer = rp->findOrMakeStaticBuffer(GrGpuBufferType::kIndex,
                                                   FixedCountCurves::IndexBufferSize(),
                                                   gFixedCountCurveIndexBufferKey,
                                                   FixedCountCurves::WriteIndexBuffer);
}

void PathCurveTessellator::draw(GrOpFlushState* flushState) const {
    if (!fFixedVertexBuffer || !fFixedIndexBuffer) {
        return;
    }
    int fixedIndexCount = NumCurveTrianglesAtResolveLevel(fFixedResolveLevel) * 3;
    for (const GrVertexChunk& chunk : fVertexChunkArray) {
        flushState->bindBuffers(fFixedIndexBuffer, chunk.fBuffer, fFixedVertexBuffer);
        flushState->drawIndexedInstanced(fixedIndexCount, 0, chunk.fCount, chunk.fBase, 0);
    }
}

void PathCurveTessellator::drawHullInstances(GrOpFlushState* flushState,
                                             sk_sp<const GrGpuBuffer> vertexBufferIfNeeded) const {
    for (const GrVertexChunk& chunk : fVertexChunkArray) {
        flushState->bindBuffers(nullptr, chunk.fBuffer, vertexBufferIfNeeded);
        flushState->drawInstanced(chunk.fCount, chunk.fBase, 4, 0);
    }
}


SKGPU_DECLARE_STATIC_UNIQUE_KEY(gFixedCountWedgesVertexBufferKey);
SKGPU_DECLARE_STATIC_UNIQUE_KEY(gFixedCountWedgesIndexBufferKey);

void PathWedgeTessellator::prepare(GrMeshDrawTarget* target,
                                   const SkMatrix& shaderMatrix,
                                   const PathDrawList& pathDrawList,
                                   int totalCombinedPathVerbCnt) {
    if (int patchPreallocCount = FixedCountWedges::PreallocCount(totalCombinedPathVerbCnt)) {
        WedgeWriter writer{fAttribs, skgpu::kMaxParametricSegments,
                           target, &fVertexChunkArray, patchPreallocCount};
        int resolveLevel = write_wedge_patches(std::move(writer), shaderMatrix, pathDrawList);
        this->updateResolveLevel(resolveLevel);
    }

    GrResourceProvider* rp = target->resourceProvider();

    SKGPU_DEFINE_STATIC_UNIQUE_KEY(gFixedCountWedgesVertexBufferKey);

    fFixedVertexBuffer = rp->findOrMakeStaticBuffer(GrGpuBufferType::kVertex,
                                                    FixedCountWedges::VertexBufferSize(),
                                                    gFixedCountWedgesVertexBufferKey,
                                                    FixedCountWedges::WriteVertexBuffer);

    SKGPU_DEFINE_STATIC_UNIQUE_KEY(gFixedCountWedgesIndexBufferKey);

    fFixedIndexBuffer = rp->findOrMakeStaticBuffer(GrGpuBufferType::kIndex,
                                                   FixedCountWedges::IndexBufferSize(),
                                                   gFixedCountWedgesIndexBufferKey,
                                                   FixedCountWedges::WriteIndexBuffer);
}

void PathWedgeTessellator::draw(GrOpFlushState* flushState) const {
    if (!fFixedVertexBuffer || !fFixedIndexBuffer) {
        return;
    }
    // Emit 3 vertices per curve triangle, plus 3 more for the fan triangle.
    int fixedIndexCount = (NumCurveTrianglesAtResolveLevel(fFixedResolveLevel) + 1) * 3;
    for (const GrVertexChunk& chunk : fVertexChunkArray) {
        flushState->bindBuffers(fFixedIndexBuffer, chunk.fBuffer, fFixedVertexBuffer);
        flushState->drawIndexedInstanced(fixedIndexCount, 0, chunk.fCount, chunk.fBase, 0);
    }
}

}  // namespace skgpu::v1

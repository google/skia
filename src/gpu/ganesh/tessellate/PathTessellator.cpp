/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/tessellate/PathTessellator.h"

#include "include/core/SkPathTypes.h"
#include "include/private/base/SkAlignedStorage.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkOnce.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkVx.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrMeshDrawTarget.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/tessellate/VertexChunkPatchAllocator.h"
#include "src/gpu/tessellate/AffineMatrix.h"
#include "src/gpu/tessellate/FixedCountBufferUtils.h"
#include "src/gpu/tessellate/LinearTolerances.h"
#include "src/gpu/tessellate/MidpointContourParser.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/WangsFormula.h"

#include <utility>

namespace skgpu::ganesh {

namespace {

using namespace skgpu::tess;

using CurveWriter = PatchWriter<VertexChunkPatchAllocator,
                                Optional<PatchAttribs::kColor>,
                                Optional<PatchAttribs::kWideColorIfEnabled>,
                                Optional<PatchAttribs::kExplicitCurveType>,
                                AddTrianglesWhenChopping,
                                DiscardFlatCurves>;

void write_curve_patches(CurveWriter&& patchWriter,
                         const SkMatrix& shaderMatrix,
                         const PathTessellator::PathDrawList& pathDrawList) {
    patchWriter.setShaderTransform(wangs_formula::VectorXform{shaderMatrix});
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

                    patchWriter.writeQuadratic(p0, p1, p2);
                    break;
                }

                case SkPathVerb::kConic: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto p2 = m.map1Point(pts+2);

                    patchWriter.writeConic(p0, p1, p2, *w);
                    break;
                }

                case SkPathVerb::kCubic: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto [p2, p3] = m.map2Points(pts+2);

                    patchWriter.writeCubic(p0, p1, p2, p3);
                    break;
                }

                default: break;
            }
        }
    }
}

using WedgeWriter = PatchWriter<VertexChunkPatchAllocator,
                                Required<PatchAttribs::kFanPoint>,
                                Optional<PatchAttribs::kColor>,
                                Optional<PatchAttribs::kWideColorIfEnabled>,
                                Optional<PatchAttribs::kExplicitCurveType>>;

void write_wedge_patches(WedgeWriter&& patchWriter,
                         const SkMatrix& shaderMatrix,
                         const PathTessellator::PathDrawList& pathDrawList) {
    patchWriter.setShaderTransform(wangs_formula::VectorXform{shaderMatrix});
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

                        patchWriter.writeQuadratic(p0, p1, p2);
                        lastPoint = pts[2];
                        break;
                    }

                    case SkPathVerb::kConic: {
                        auto [p0, p1] = m.map2Points(pts);
                        auto p2 = m.map1Point(pts+2);

                        patchWriter.writeConic(p0, p1, p2, *w);
                        lastPoint = pts[2];
                        break;
                    }

                    case SkPathVerb::kCubic: {
                        auto [p0, p1] = m.map2Points(pts);
                        auto [p2, p3] = m.map2Points(pts+2);

                        patchWriter.writeCubic(p0, p1, p2, p3);
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
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
    int patchPreallocCount = FixedCountCurves::PreallocCount(totalCombinedPathVerbCnt) +
                             (extraTriangles ? extraTriangles->count() : 0);
#else
    SkASSERT(!extraTriangles);
    int patchPreallocCount = FixedCountCurves::PreallocCount(totalCombinedPathVerbCnt);
#endif


    if (patchPreallocCount) {
        LinearTolerances worstCase;
        CurveWriter writer{fAttribs, &worstCase, target, &fVertexChunkArray, patchPreallocCount};

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
        // Write out extra space-filling triangles to connect the curve patches with any external
        // source of geometry (e.g. inner triangulation that handles winding explicitly).
        if (extraTriangles) {
            SkDEBUGCODE(int breadcrumbCount = 0;)
            for (const auto* tri = extraTriangles->head(); tri; tri = tri->fNext) {
                SkDEBUGCODE(++breadcrumbCount;)
                auto p0 = skvx::float2::Load(tri->fPts);
                auto p1 = skvx::float2::Load(tri->fPts + 1);
                auto p2 = skvx::float2::Load(tri->fPts + 2);
                if (any((p0 == p1) & (p1 == p2))) {
                    // Cull completely horizontal or vertical triangles. GrTriangulator can't always
                    // get these breadcrumb edges right when they run parallel to the sweep
                    // direction because their winding is undefined by its current definition.
                    // FIXME(skbug.com/40043149): This seemed safe, but if there is a view matrix it will
                    // introduce T-junctions.
                    continue;
                }
                writer.writeTriangle(p0, p1, p2);
            }
            SkASSERT(breadcrumbCount == extraTriangles->count());
        }
#endif

        write_curve_patches(std::move(writer), shaderMatrix, pathDrawList);
        fMaxVertexCount = FixedCountCurves::VertexCount(worstCase);
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
    for (const GrVertexChunk& chunk : fVertexChunkArray) {
        flushState->bindBuffers(fFixedIndexBuffer, chunk.fBuffer, fFixedVertexBuffer);
        // The max vertex count is the logical number of vertices that the GPU needs to emit, so
        // since we're using drawIndexedInstanced, it's provided as the "index count" parameter.
        flushState->drawIndexedInstanced(fMaxVertexCount, 0, chunk.fCount, chunk.fBase, 0);
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
        LinearTolerances worstCase;
        WedgeWriter writer{fAttribs, &worstCase, target, &fVertexChunkArray, patchPreallocCount};
        write_wedge_patches(std::move(writer), shaderMatrix, pathDrawList);
        fMaxVertexCount = FixedCountWedges::VertexCount(worstCase);
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
    for (const GrVertexChunk& chunk : fVertexChunkArray) {
        flushState->bindBuffers(fFixedIndexBuffer, chunk.fBuffer, fFixedVertexBuffer);
        flushState->drawIndexedInstanced(fMaxVertexCount, 0, chunk.fCount, chunk.fBase, 0);
    }
}

}  // namespace skgpu::ganesh

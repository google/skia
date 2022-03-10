/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/PathCurveTessellator.h"

#include "src/gpu/tessellate/AffineMatrix.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/WangsFormula.h"

#if SK_GPU_V1
#include "src/gpu/GrMeshDrawTarget.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrResourceProvider.h"
#endif

namespace skgpu {

namespace {

using Writer = PatchWriter<GrVertexChunkBuilder,
                           Optional<PatchAttribs::kColor>,
                           Optional<PatchAttribs::kWideColorIfEnabled>,
                           Optional<PatchAttribs::kExplicitCurveType>,
                           AddTrianglesWhenChopping,
                           DiscardFlatCurves>;

int write_patches(Writer&& patchWriter,
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

}  // namespace

void PathCurveTessellator::WriteFixedVertexBuffer(VertexWriter vertexWriter, size_t bufferSize) {
    SkASSERT(bufferSize >= sizeof(SkPoint) * 2);
    SkASSERT(bufferSize % sizeof(SkPoint) == 0);
    int vertexCount = bufferSize / sizeof(SkPoint);
    SkASSERT(vertexCount > 3);
    SkDEBUGCODE(auto end = vertexWriter.mark(vertexCount * sizeof(SkPoint));)

    // Lay out the vertices in "middle-out" order:
    //
    // T= 0/1, 1/1,              ; resolveLevel=0
    //    1/2,                   ; resolveLevel=1  (0/2 and 2/2 are already in resolveLevel 0)
    //    1/4, 3/4,              ; resolveLevel=2  (2/4 is already in resolveLevel 1)
    //    1/8, 3/8, 5/8, 7/8,    ; resolveLevel=3  (2/8 and 6/8 are already in resolveLevel 2)
    //    ...                    ; resolveLevel=...
    //
    // Resolve level 0 is just the beginning and ending vertices.
    vertexWriter << (float)0/*resolveLevel*/ << (float)0/*idx*/;
    vertexWriter << (float)0/*resolveLevel*/ << (float)1/*idx*/;

    // Resolve levels 1..kMaxResolveLevel.
    int maxResolveLevel = SkPrevLog2(vertexCount - 1);
    SkASSERT((1 << maxResolveLevel) + 1 == vertexCount);
    for (int resolveLevel = 1; resolveLevel <= maxResolveLevel; ++resolveLevel) {
        int numSegmentsInResolveLevel = 1 << resolveLevel;
        // Write out the odd vertices in this resolveLevel. The even vertices were already written
        // out in previous resolveLevels and will be indexed from there.
        for (int i = 1; i < numSegmentsInResolveLevel; i += 2) {
            vertexWriter << (float)resolveLevel << (float)i;
        }
    }

    SkASSERT(vertexWriter.mark() == end);
}

void PathCurveTessellator::WriteFixedIndexBufferBaseIndex(VertexWriter vertexWriter,
                                                          size_t bufferSize,
                                                          uint16_t baseIndex) {
    SkASSERT(bufferSize % (sizeof(uint16_t) * 3) == 0);
    int triangleCount = bufferSize / (sizeof(uint16_t) * 3);
    SkASSERT(triangleCount >= 1);
    SkTArray<std::array<uint16_t, 3>> indexData(triangleCount);

    // Connect the vertices with a middle-out triangulation. Refer to InitFixedCountVertexBuffer()
    // for the exact vertex ordering.
    //
    // Resolve level 1 is just a single triangle at T=[0, 1/2, 1].
    const auto* neighborInLastResolveLevel = &indexData.push_back({baseIndex,
                                                                   (uint16_t)(baseIndex + 2),
                                                                   (uint16_t)(baseIndex + 1)});

    // Resolve levels 2..maxResolveLevel
    int maxResolveLevel = SkPrevLog2(triangleCount + 1);
    uint16_t nextIndex = baseIndex + 3;
    SkASSERT(NumCurveTrianglesAtResolveLevel(maxResolveLevel) == triangleCount);
    for (int resolveLevel = 2; resolveLevel <= maxResolveLevel; ++resolveLevel) {
        SkDEBUGCODE(auto* firstTriangleInCurrentResolveLevel = indexData.end());
        int numOuterTrianglelsInResolveLevel = 1 << (resolveLevel - 1);
        SkASSERT(numOuterTrianglelsInResolveLevel % 2 == 0);
        int numTrianglePairsInResolveLevel = numOuterTrianglelsInResolveLevel >> 1;
        for (int i = 0; i < numTrianglePairsInResolveLevel; ++i) {
            // First triangle shares the left edge of "neighborInLastResolveLevel".
            indexData.push_back({(*neighborInLastResolveLevel)[0],
                                 nextIndex++,
                                 (*neighborInLastResolveLevel)[1]});
            // Second triangle shares the right edge of "neighborInLastResolveLevel".
            indexData.push_back({(*neighborInLastResolveLevel)[1],
                                 nextIndex++,
                                 (*neighborInLastResolveLevel)[2]});
            ++neighborInLastResolveLevel;
        }
        SkASSERT(neighborInLastResolveLevel == firstTriangleInCurrentResolveLevel);
    }
    SkASSERT(indexData.count() == triangleCount);
    SkASSERT(nextIndex == baseIndex + triangleCount + 2);

    vertexWriter << VertexWriter::Array(indexData.data(), indexData.count());
}

#if SK_GPU_V1

SKGPU_DECLARE_STATIC_UNIQUE_KEY(gFixedVertexBufferKey);
SKGPU_DECLARE_STATIC_UNIQUE_KEY(gFixedIndexBufferKey);

void PathCurveTessellator::prepareWithTriangles(
        GrMeshDrawTarget* target,
        int maxTessellationSegments,
        const SkMatrix& shaderMatrix,
        GrInnerFanTriangulator::BreadcrumbTriangleList* extraTriangles,
        const PathDrawList& pathDrawList,
        int totalCombinedPathVerbCnt,
        bool willUseTessellationShaders) {
    int patchPreallocCount = PatchPreallocCount(totalCombinedPathVerbCnt) +
                             (extraTriangles ? extraTriangles->count() : 0);
    if (patchPreallocCount) {
        Writer writer{fAttribs, maxTessellationSegments,
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

        int resolveLevel = write_patches(std::move(writer), shaderMatrix, pathDrawList);
        this->updateResolveLevel(resolveLevel);
    }
    if (!willUseTessellationShaders) {
        this->prepareFixedCountBuffers(target);
    }
}

void PathCurveTessellator::prepareFixedCountBuffers(GrMeshDrawTarget* target) {
    GrResourceProvider* rp = target->resourceProvider();

    SKGPU_DEFINE_STATIC_UNIQUE_KEY(gFixedVertexBufferKey);

    fFixedVertexBuffer = rp->findOrMakeStaticBuffer(GrGpuBufferType::kVertex,
                                                    FixedVertexBufferSize(kMaxFixedResolveLevel),
                                                    gFixedVertexBufferKey,
                                                    WriteFixedVertexBuffer);

    SKGPU_DEFINE_STATIC_UNIQUE_KEY(gFixedIndexBufferKey);

    fFixedIndexBuffer = rp->findOrMakeStaticBuffer(GrGpuBufferType::kIndex,
                                                   FixedIndexBufferSize(kMaxFixedResolveLevel),
                                                   gFixedIndexBufferKey,
                                                   WriteFixedIndexBuffer);
}

void PathCurveTessellator::drawTessellated(GrOpFlushState* flushState) const {
    for (const GrVertexChunk& chunk : fVertexChunkArray) {
        flushState->bindBuffers(nullptr, nullptr, chunk.fBuffer);
        flushState->draw(chunk.fCount * 4, chunk.fBase * 4);
    }
}

void PathCurveTessellator::drawFixedCount(GrOpFlushState* flushState) const {
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

#endif

}  // namespace skgpu

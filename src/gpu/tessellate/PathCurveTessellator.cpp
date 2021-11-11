/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/PathCurveTessellator.h"

#include "src/core/SkUtils.h"
#include "src/gpu/GrMeshDrawTarget.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/tessellate/AffineMatrix.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/WangsFormula.h"
#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

#if SK_GPU_V1
#include "src/gpu/GrOpFlushState.h"
#endif

namespace skgpu {

using CubicPatch = PatchWriter::CubicPatch;
using ConicPatch = PatchWriter::ConicPatch;
using TrianglePatch = PatchWriter::TrianglePatch;

PathCurveTessellator* PathCurveTessellator::Make(SkArenaAlloc* arena,
                                                 const SkMatrix& viewMatrix,
                                                 const SkPMColor4f& color,
                                                 DrawInnerFan drawInnerFan,
                                                 int numPathVerbs,
                                                 const GrPipeline& pipeline,
                                                 const GrCaps& caps) {
    using PatchType = GrPathTessellationShader::PatchType;
    GrPathTessellationShader* shader;
    if (caps.shaderCaps()->tessellationSupport() &&
        caps.shaderCaps()->infinitySupport() &&  // The hw tessellation shaders use infinity.
        !pipeline.usesLocalCoords() &&  // Our tessellation back door doesn't handle varyings.
        numPathVerbs >= caps.minPathVerbsForHwTessellation()) {
        shader = GrPathTessellationShader::MakeHardwareTessellationShader(arena, viewMatrix, color,
                                                                          PatchType::kCurves);
    } else {
        shader = GrPathTessellationShader::MakeMiddleOutFixedCountShader(*caps.shaderCaps(), arena,
                                                                         viewMatrix, color,
                                                                         PatchType::kCurves);
    }
    return arena->make([=](void* objStart) {
        return new(objStart) PathCurveTessellator(shader, drawInnerFan);
    });
}

GR_DECLARE_STATIC_UNIQUE_KEY(gFixedCountVertexBufferKey);
GR_DECLARE_STATIC_UNIQUE_KEY(gFixedCountIndexBufferKey);

void PathCurveTessellator::prepare(GrMeshDrawTarget* target,
                                   const PathDrawList& pathDrawList,
                                   int totalCombinedPathVerbCnt,
                                   const BreadcrumbTriangleList* breadcrumbTriangleList) {
    SkASSERT(fVertexChunkArray.empty());

    const GrShaderCaps& shaderCaps = *target->caps().shaderCaps();

    // Determine how many triangles to allocate.
    int maxTriangles = 0;
    if (fDrawInnerFan) {
        int maxCombinedFanEdges = MaxCombinedFanEdgesInPathDrawList(totalCombinedPathVerbCnt);
        // A single n-sided polygon is fanned by n-2 triangles. Multiple polygons with a combined
        // edge count of n are fanned by strictly fewer triangles.
        int maxTrianglesInFans = std::max(maxCombinedFanEdges - 2, 0);
        maxTriangles += maxTrianglesInFans;
    }
    if (breadcrumbTriangleList) {
        maxTriangles += breadcrumbTriangleList->count();
    }
    // Over-allocate enough curves for 1 in 4 to chop.
    int curveAllocCount = (totalCombinedPathVerbCnt * 5 + 3) / 4;  // i.e., ceil(numVerbs * 5/4)
    int patchAllocCount = maxTriangles + curveAllocCount;
    if (!patchAllocCount) {
        return;
    }
    size_t patchStride = fShader->willUseTessellationShaders() ? fShader->vertexStride() * 4
                                                               : fShader->instanceStride();

    auto attribs = PatchAttribs::kNone;
    if (!shaderCaps.infinitySupport()) {
        attribs |= PatchAttribs::kExplicitCurveType;
    }
    PatchWriter patchWriter(target, &fVertexChunkArray, patchStride, patchAllocCount, attribs);

    // Write out inner fan triangles.
    if (fDrawInnerFan) {
        for (auto [pathMatrix, path] : pathDrawList) {
            AffineMatrix m(pathMatrix);
            for (PathMiddleOutFanIter it(path); !it.done();) {
                for (auto [p0, p1, p2] : it.nextStack()) {
                    TrianglePatch(patchWriter) << m.map2Points(p0, p1) << m.mapPoint(p2);
                }
            }
        }
    }

    // Write out breadcrumb triangles.
    if (breadcrumbTriangleList) {
        SkDEBUGCODE(int count = 0;)
#ifdef SK_DEBUG
        for (auto [pathMatrix, path] : pathDrawList) {
            // This assert isn't actually necessary, but we currently only use breadcrumb
            // triangles with an identity pathMatrix. If that ever changes, this assert will
            // serve as a gentle reminder to make sure the breadcrumb triangles are also
            // transformed on the CPU.
            SkASSERT(pathMatrix.isIdentity());
        }
#endif
        for (const auto* tri = breadcrumbTriangleList->head(); tri; tri = tri->fNext) {
            SkDEBUGCODE(++count;)
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
            TrianglePatch(patchWriter) << p0 << p1 << p2;
        }
        SkASSERT(count == breadcrumbTriangleList->count());
    }

    int maxFixedCountResolveLevel = GrPathTessellationShader::kMaxFixedCountResolveLevel;
    int maxSegments;
    if (fShader->willUseTessellationShaders()) {
        // The curve shader tessellates T=0..(1/2) on the first side of the canonical triangle and
        // T=(1/2)..1 on the second side. This means we get double the max tessellation segments
        // for the range T=0..1.
        maxSegments = shaderCaps.maxTessellationSegments() * 2;
    } else {
        maxSegments = 1 << maxFixedCountResolveLevel;
    }
    float maxSegments_pow2 = pow2(maxSegments);
    float maxSegments_pow4 = pow2(maxSegments_pow2);

    // If using fixed count, this is the number of segments we need to emit per instance. Always
    // emit at least 2 segments so we can support triangles.
    float numFixedSegments_pow4 = 2*2*2*2;

    for (auto [pathMatrix, path] : pathDrawList) {
        AffineMatrix m(pathMatrix);
        wangs_formula::VectorXform totalXform(SkMatrix::Concat(fShader->viewMatrix(), pathMatrix));
        for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
            switch (verb) {
                case SkPathVerb::kQuad: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto p2 = m.map1Point(pts+2);
                    float n4 = wangs_formula::quadratic_pow4(kTessellationPrecision,
                                                             pts,
                                                             totalXform);
                    if (n4 <= 1) {
                        break;  // This quad only needs 1 segment, which is empty.
                    }
                    if (n4 <= maxSegments_pow4) {
                        // This quad already fits into "maxSegments" tessellation segments.
                        CubicPatch(patchWriter) << QuadToCubic{p0, p1, p2};
                    } else {
                        // Chop until each quad tessellation requires "maxSegments" or fewer.
                        int numPatches =
                                SkScalarCeilToInt(wangs_formula::root4(n4/maxSegments_pow4));
                        patchWriter.chopAndWriteQuads(p0, p1, p2, numPatches);
                    }
                    numFixedSegments_pow4 = std::max(n4, numFixedSegments_pow4);
                    break;
                }

                case SkPathVerb::kConic: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto p2 = m.map1Point(pts+2);
                    float n2 = wangs_formula::conic_pow2(kTessellationPrecision,
                                                         pts,
                                                         *w,
                                                         totalXform);
                    if (n2 <= 1) {
                        break;  // This conic only needs 1 segment, which is empty.
                    }
                    if (n2 <= maxSegments_pow2) {
                        // This conic already fits into "maxSegments" tessellation segments.
                        ConicPatch(patchWriter) << p0 << p1 << p2 << *w;
                    } else {
                        // Chop until each conic tessellation requires "maxSegments" or fewer.
                        int numPatches = SkScalarCeilToInt(sqrtf(n2/maxSegments_pow2));
                        patchWriter.chopAndWriteConics(p0, p1, p2, *w, numPatches);
                    }
                    numFixedSegments_pow4 = std::max(n2*n2, numFixedSegments_pow4);
                    break;
                }

                case SkPathVerb::kCubic: {
                    auto [p0, p1] = m.map2Points(pts);
                    auto [p2, p3] = m.map2Points(pts+2);
                    float n4 = wangs_formula::cubic_pow4(kTessellationPrecision,
                                                         pts,
                                                         totalXform);
                    if (n4 <= 1) {
                        break;  // This cubic only needs 1 segment, which is empty.
                    }
                    if (n4 <= maxSegments_pow4) {
                        // This cubic already fits into "maxSegments" tessellation segments.
                        CubicPatch(patchWriter) << p0 << p1 << p2 << p3;
                    } else {
                        // Chop until each cubic tessellation requires "maxSegments" or fewer.
                        int numPatches =
                                SkScalarCeilToInt(wangs_formula::root4(n4/maxSegments_pow4));
                        patchWriter.chopAndWriteCubics(p0, p1, p2, p3, numPatches);
                    }
                    numFixedSegments_pow4 = std::max(n4, numFixedSegments_pow4);
                    break;
                }

                default: break;
            }
        }
    }

    if (!fShader->willUseTessellationShaders()) {
        // log16(n^4) == log2(n).
        // We already chopped curves to make sure none needed a higher resolveLevel than
        // kMaxFixedCountResolveLevel.
        int fixedResolveLevel = std::min(wangs_formula::nextlog16(numFixedSegments_pow4),
                                         maxFixedCountResolveLevel);
        fFixedIndexCount =
                GrPathTessellationShader::NumCurveTrianglesAtResolveLevel(fixedResolveLevel) * 3;

        GR_DEFINE_STATIC_UNIQUE_KEY(gFixedCountVertexBufferKey);

        fFixedCountVertexBuffer = target->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kVertex,
                GrPathTessellationShader::SizeOfVertexBufferForMiddleOutCurves(),
                gFixedCountVertexBufferKey,
                GrPathTessellationShader::InitializeVertexBufferForMiddleOutCurves);

        GR_DEFINE_STATIC_UNIQUE_KEY(gFixedCountIndexBufferKey);

        fFixedCountIndexBuffer = target->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kIndex,
                GrPathTessellationShader::SizeOfIndexBufferForMiddleOutCurves(),
                gFixedCountIndexBufferKey,
                GrPathTessellationShader::InitializeIndexBufferForMiddleOutCurves);
    }
}

#if SK_GPU_V1
void PathCurveTessellator::draw(GrOpFlushState* flushState) const {
    if (fShader->willUseTessellationShaders()) {
        for (const GrVertexChunk& chunk : fVertexChunkArray) {
            flushState->bindBuffers(nullptr, nullptr, chunk.fBuffer);
            flushState->draw(chunk.fCount * 4, chunk.fBase * 4);
        }
    } else {
        SkASSERT(fShader->hasInstanceAttributes());
        for (const GrVertexChunk& chunk : fVertexChunkArray) {
            flushState->bindBuffers(fFixedCountIndexBuffer, chunk.fBuffer, fFixedCountVertexBuffer);
            flushState->drawIndexedInstanced(fFixedIndexCount, 0, chunk.fCount, chunk.fBase, 0);
        }
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

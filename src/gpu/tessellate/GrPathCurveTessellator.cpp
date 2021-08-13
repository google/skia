/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathCurveTessellator.h"

#include "src/core/SkUtils.h"
#include "src/gpu/GrMeshDrawTarget.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/geometry/GrWangsFormula.h"
#include "src/gpu/tessellate/GrCullTest.h"
#include "src/gpu/tessellate/GrMiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/GrPathXform.h"
#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

namespace {

constexpr static float kPrecision = GrTessellationShader::kLinearizationPrecision;

// Writes out curve patches, chopping as necessary so none require more segments than are
// supported by the hardware.
class CurveWriter {
public:
    CurveWriter(int maxSegments)
            : fMaxSegments_pow2(maxSegments * maxSegments)
            , fMaxSegments_pow4(fMaxSegments_pow2 * fMaxSegments_pow2) {
    }

    void setMatrices(const SkRect& cullBounds,
                     const SkMatrix& shaderMatrix,
                     const SkMatrix& pathMatrix) {
        SkMatrix totalMatrix;
        totalMatrix.setConcat(shaderMatrix, pathMatrix);
        fCullTest.set(cullBounds, totalMatrix);
        fTotalVectorXform = totalMatrix;
        fPathXform = pathMatrix;
    }

    SK_ALWAYS_INLINE void writeQuadratic(const GrShaderCaps& shaderCaps,
                                         GrVertexChunkBuilder* chunker, const SkPoint p[3]) {
        float numSegments_pow4 = GrWangsFormula::quadratic_pow4(kPrecision, p, fTotalVectorXform);
        if (numSegments_pow4 > fMaxSegments_pow4) {
            this->chopAndWriteQuadratic(shaderCaps, chunker, p);
            return;
        }
        if (numSegments_pow4 > 1) {
            if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
                fPathXform.mapQuadToCubic(&vertexWriter, p);
                vertexWriter.write(GrVertexWriter::If(!shaderCaps.infinitySupport(),
                                                      GrTessellationShader::kCubicCurveType));
            }
            fNumFixedSegments_pow4 = std::max(numSegments_pow4, fNumFixedSegments_pow4);
        }
    }

    SK_ALWAYS_INLINE void writeConic(const GrShaderCaps& shaderCaps, GrVertexChunkBuilder* chunker,
                                     const SkPoint p[3], float w) {
        float numSegments_pow2 = GrWangsFormula::conic_pow2(kPrecision, p, w, fTotalVectorXform);
        if (numSegments_pow2 > fMaxSegments_pow2) {
            this->chopAndWriteConic(shaderCaps, chunker, {p, w});
            return;
        }
        if (numSegments_pow2 > 1) {
            if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
                fPathXform.mapConicToPatch(&vertexWriter, p, w);
                vertexWriter.write(GrVertexWriter::If(!shaderCaps.infinitySupport(),
                                                      GrTessellationShader::kConicCurveType));
            }
            fNumFixedSegments_pow4 = std::max(numSegments_pow2 * numSegments_pow2,
                                              fNumFixedSegments_pow4);
        }
    }

    SK_ALWAYS_INLINE void writeCubic(const GrShaderCaps& shaderCaps, GrVertexChunkBuilder* chunker,
                                     const SkPoint p[4]) {
        float numSegments_pow4 = GrWangsFormula::cubic_pow4(kPrecision, p, fTotalVectorXform);
        if (numSegments_pow4 > fMaxSegments_pow4) {
            this->chopAndWriteCubic(shaderCaps, chunker, p);
            return;
        }
        if (numSegments_pow4 > 1) {
            if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
                fPathXform.map4Points(&vertexWriter, p);
                vertexWriter.write(GrVertexWriter::If(!shaderCaps.infinitySupport(),
                                                      GrTessellationShader::kCubicCurveType));
            }
            fNumFixedSegments_pow4 = std::max(numSegments_pow4, fNumFixedSegments_pow4);
        }
    }

    int numFixedSegments_pow4() const { return fNumFixedSegments_pow4; }

private:
    void chopAndWriteQuadratic(const GrShaderCaps& shaderCaps, GrVertexChunkBuilder* chunker,
                               const SkPoint p[3]) {
        SkPoint chops[5];
        SkChopQuadAtHalf(p, chops);
        for (int i = 0; i < 2; ++i) {
            const SkPoint* q = chops + i*2;
            if (fCullTest.areVisible3(q)) {
                this->writeQuadratic(shaderCaps, chunker, q);
            }
        }
        // Connect the two halves.
        this->writeTriangle(shaderCaps, chunker, chops[0], chops[2], chops[4]);
    }

    void chopAndWriteConic(const GrShaderCaps& shaderCaps, GrVertexChunkBuilder* chunker,
                           const SkConic& conic) {
        SkConic chops[2];
        if (!conic.chopAt(.5, chops)) {
            return;
        }
        for (int i = 0; i < 2; ++i) {
            if (fCullTest.areVisible3(chops[i].fPts)) {
                this->writeConic(shaderCaps, chunker, chops[i].fPts, chops[i].fW);
            }
        }
        // Connect the two halves.
        this->writeTriangle(shaderCaps, chunker, conic.fPts[0], chops[0].fPts[2], chops[1].fPts[2]);
    }

    void chopAndWriteCubic(const GrShaderCaps& shaderCaps, GrVertexChunkBuilder* chunker,
                           const SkPoint p[4]) {
        SkPoint chops[7];
        SkChopCubicAtHalf(p, chops);
        for (int i = 0; i < 2; ++i) {
            const SkPoint* c = chops + i*3;
            if (fCullTest.areVisible4(c)) {
                this->writeCubic(shaderCaps, chunker, c);
            }
        }
        // Connect the two halves.
        this->writeTriangle(shaderCaps, chunker, chops[0], chops[3], chops[6]);
    }

    void writeTriangle(const GrShaderCaps& shaderCaps, GrVertexChunkBuilder* chunker, SkPoint p0,
                       SkPoint p1, SkPoint p2) {
        if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
            vertexWriter.write(fPathXform.mapPoint(p0),
                               fPathXform.mapPoint(p1),
                               fPathXform.mapPoint(p2));
            // Mark this instance as a triangle by setting it to a conic with w=Inf.
            vertexWriter.fill(GrVertexWriter::kIEEE_32_infinity, 2);
            vertexWriter.write(GrVertexWriter::If(!shaderCaps.infinitySupport(),
                                                  GrTessellationShader::kTriangularConicCurveType));
        }
    }

    GrCullTest fCullTest;
    GrVectorXform fTotalVectorXform;
    GrPathXform fPathXform;
    const float fMaxSegments_pow2;
    const float fMaxSegments_pow4;

    // If using fixed count, this is the number of segments we need to emit per instance. Always
    // emit at least 2 segments so we can support triangles.
    float fNumFixedSegments_pow4 = 2*2*2*2;
};

}  // namespace


GrPathCurveTessellator* GrPathCurveTessellator::Make(SkArenaAlloc* arena,
                                                     const SkMatrix& viewMatrix,
                                                     const SkPMColor4f& color,
                                                     DrawInnerFan drawInnerFan, int numPathVerbs,
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
        return new(objStart) GrPathCurveTessellator(shader, drawInnerFan);
    });
}

GR_DECLARE_STATIC_UNIQUE_KEY(gFixedCountVertexBufferKey);
GR_DECLARE_STATIC_UNIQUE_KEY(gFixedCountIndexBufferKey);

void GrPathCurveTessellator::prepare(GrMeshDrawTarget* target,
                                     const SkRect& cullBounds,
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
    GrVertexChunkBuilder chunker(target, &fVertexChunkArray, patchStride, patchAllocCount);

    // Write out the triangles.
    if (maxTriangles) {
        GrVertexWriter vertexWriter = chunker.appendVertices(maxTriangles);
        if (!vertexWriter) {
            return;
        }
        int numRemainingTriangles = maxTriangles;
        if (fDrawInnerFan) {
            // Pad the triangles with 2 infinities. This produces conic patches with w=Infinity. In
            // the case where infinity is not supported, we also write out a 3rd float that
            // explicitly tells the shader to interpret these patches as triangles.
            int pad32Count = shaderCaps.infinitySupport() ? 2 : 3;
            uint32_t pad32Value = shaderCaps.infinitySupport()
                    ? GrVertexWriter::kIEEE_32_infinity
                    : sk_bit_cast<uint32_t>(GrTessellationShader::kTriangularConicCurveType);
            for (auto [pathMatrix, path] : pathDrawList) {
                int numTrianglesWritten;
                vertexWriter = GrMiddleOutPolygonTriangulator::WritePathInnerFan(
                        std::move(vertexWriter),
                        pad32Count,
                        pad32Value,
                        pathMatrix,
                        path,
                        &numTrianglesWritten);
                numRemainingTriangles -= numTrianglesWritten;
            }
        }
        if (breadcrumbTriangleList) {
            int numWritten = 0;
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
                auto p0 = grvx::float2::Load(tri->fPts);
                auto p1 = grvx::float2::Load(tri->fPts + 1);
                auto p2 = grvx::float2::Load(tri->fPts + 2);
                if (skvx::any((p0 == p1) & (p1 == p2))) {
                    // Cull completely horizontal or vertical triangles. GrTriangulator can't always
                    // get these breadcrumb edges right when they run parallel to the sweep
                    // direction because their winding is undefined by its current definition.
                    // FIXME(skia:12060): This seemed safe, but if there is a view matrix it will
                    // introduce T-junctions.
                    continue;
                }
                vertexWriter.writeArray(tri->fPts, 3);
                // Mark this instance as a triangle by setting it to a conic with w=Inf.
                vertexWriter.fill(GrVertexWriter::kIEEE_32_infinity, 2);
                vertexWriter.write(
                        GrVertexWriter::If(!shaderCaps.infinitySupport(),
                                           GrTessellationShader::kTriangularConicCurveType));
                ++numWritten;
            }
            SkASSERT(count == breadcrumbTriangleList->count());
            numRemainingTriangles -= numWritten;
        }
        chunker.popVertices(numRemainingTriangles);
    }

    int maxSegments;
    if (fShader->willUseTessellationShaders()) {
        // The curve shader tessellates T=0..(1/2) on the first side of the canonical triangle and
        // T=(1/2)..1 on the second side. This means we get double the max tessellation segments
        // for the range T=0..1.
        maxSegments = shaderCaps.maxTessellationSegments() * 2;
    } else {
        maxSegments = GrPathTessellationShader::kMaxFixedCountSegments;
    }

    CurveWriter curveWriter(maxSegments);
    for (auto [pathMatrix, path] : pathDrawList) {
        curveWriter.setMatrices(cullBounds, fShader->viewMatrix(), pathMatrix);
        for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
            switch (verb) {
                case SkPathVerb::kQuad:
                    curveWriter.writeQuadratic(shaderCaps, &chunker, pts);
                    break;
                case SkPathVerb::kConic:
                    curveWriter.writeConic(shaderCaps, &chunker, pts, *w);
                    break;
                case SkPathVerb::kCubic:
                    curveWriter.writeCubic(shaderCaps, &chunker, pts);
                    break;
                default:
                    break;
            }
        }
    }

    if (!fShader->willUseTessellationShaders()) {
        // log2(n) == log16(n^4).
        int fixedResolveLevel = GrWangsFormula::nextlog16(curveWriter.numFixedSegments_pow4());
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
#include "src/gpu/GrOpFlushState.h"

void GrPathCurveTessellator::draw(GrOpFlushState* flushState) const {
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

void GrPathCurveTessellator::drawHullInstances(
        GrOpFlushState* flushState, sk_sp<const GrGpuBuffer> vertexBufferIfNeeded) const {
    for (const GrVertexChunk& chunk : fVertexChunkArray) {
        flushState->bindBuffers(nullptr, chunk.fBuffer, vertexBufferIfNeeded);
        flushState->drawInstanced(chunk.fCount, chunk.fBase, 4, 0);
    }
}
#endif

/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathCurveTessellator.h"

#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/geometry/GrWangsFormula.h"
#include "src/gpu/tessellate/GrCullTest.h"
#include "src/gpu/tessellate/GrMiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

namespace {

constexpr static float kPrecision = GrTessellationPathRenderer::kLinearizationPrecision;

// Writes out curve patches, chopping as necessary so none require more segments than are
// supported by the hardware.
class CurveWriter {
public:
    CurveWriter(const SkRect& cullBounds, const SkMatrix& viewMatrix, int maxSegments)
            : fCullTest(cullBounds, viewMatrix)
            , fVectorXform(viewMatrix)
            , fMaxSegments_pow2(maxSegments * maxSegments)
            , fMaxSegments_pow4(fMaxSegments_pow2 * fMaxSegments_pow2) {
    }

    SK_ALWAYS_INLINE void writeQuadratic(GrVertexChunkBuilder* chunker, const SkPoint p[3]) {
        float numSegments_pow4 = GrWangsFormula::quadratic_pow4(kPrecision, p, fVectorXform);
        if (numSegments_pow4 > fMaxSegments_pow4) {
            this->chopAndWriteQuadratic(chunker, p);
            return;
        }
        if (numSegments_pow4 > 1) {
            if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
                GrPathUtils::writeQuadAsCubic(p, &vertexWriter);
            }
            fNumFixedSegments_pow4 = std::max(numSegments_pow4, fNumFixedSegments_pow4);
        }
    }

    SK_ALWAYS_INLINE void writeConic(GrVertexChunkBuilder* chunker, const SkPoint p[3], float w) {
        float numSegments_pow2 = GrWangsFormula::conic_pow2(kPrecision, p, w, fVectorXform);
        if (numSegments_pow2 > fMaxSegments_pow2) {
            this->chopAndWriteConic(chunker, {p, w});
            return;
        }
        if (numSegments_pow2 > 1) {
            if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
                GrTessellationShader::WriteConicPatch(p, w, &vertexWriter);
            }
            fNumFixedSegments_pow4 = std::max(numSegments_pow2 * numSegments_pow2,
                                              fNumFixedSegments_pow4);
        }
    }

    SK_ALWAYS_INLINE void writeCubic(GrVertexChunkBuilder* chunker, const SkPoint p[4]) {
        float numSegments_pow4 = GrWangsFormula::cubic_pow4(kPrecision, p, fVectorXform);
        if (numSegments_pow4 > fMaxSegments_pow4) {
            this->chopAndWriteCubic(chunker, p);
            return;
        }
        if (numSegments_pow4 > 1) {
            if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
                vertexWriter.writeArray(p, 4);
            }
            fNumFixedSegments_pow4 = std::max(numSegments_pow4, fNumFixedSegments_pow4);
        }
    }

    int numFixedSegments_pow4() const { return fNumFixedSegments_pow4; }

private:
    void chopAndWriteQuadratic(GrVertexChunkBuilder* chunker, const SkPoint p[3]) {
        SkPoint chops[5];
        SkChopQuadAtHalf(p, chops);
        for (int i = 0; i < 2; ++i) {
            const SkPoint* q = chops + i*2;
            if (fCullTest.areVisible3(q)) {
                this->writeQuadratic(chunker, q);
            }
        }
        // Connect the two halves.
        this->writeTriangle(chunker, chops[0], chops[2], chops[4]);
    }

    void chopAndWriteConic(GrVertexChunkBuilder* chunker, const SkConic& conic) {
        SkConic chops[2];
        if (!conic.chopAt(.5, chops)) {
            return;
        }
        for (int i = 0; i < 2; ++i) {
            if (fCullTest.areVisible3(chops[i].fPts)) {
                this->writeConic(chunker, chops[i].fPts, chops[i].fW);
            }
        }
        // Connect the two halves.
        this->writeTriangle(chunker, conic.fPts[0], chops[0].fPts[2], chops[1].fPts[2]);
    }

    void chopAndWriteCubic(GrVertexChunkBuilder* chunker, const SkPoint p[4]) {
        SkPoint chops[7];
        SkChopCubicAtHalf(p, chops);
        for (int i = 0; i < 2; ++i) {
            const SkPoint* c = chops + i*3;
            if (fCullTest.areVisible4(c)) {
                this->writeCubic(chunker, c);
            }
        }
        // Connect the two halves.
        this->writeTriangle(chunker, chops[0], chops[3], chops[6]);
    }

    void writeTriangle(GrVertexChunkBuilder* chunker, SkPoint p0, SkPoint p1, SkPoint p2) {
        if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
            vertexWriter.write(p0, p1, p2);
            // Mark this instance as a triangle by setting it to a conic with w=Inf.
            vertexWriter.fill(GrVertexWriter::kIEEE_32_infinity, 2);
        }
    }

    GrCullTest fCullTest;
    GrVectorXform fVectorXform;
    const float fMaxSegments_pow2;
    const float fMaxSegments_pow4;

    // If using fixed count, this is the number of segments we need to emit per instance. Always
    // emit at least 2 segments so we can support triangles.
    float fNumFixedSegments_pow4 = 2*2*2*2;
};

}  // namespace


GrPathTessellator* GrPathCurveTessellator::Make(SkArenaAlloc* arena, const SkMatrix& viewMatrix,
                                                const SkPMColor4f& color, DrawInnerFan drawInnerFan,
                                                int numPathVerbs, const GrPipeline& pipeline,
                                                const GrCaps& caps) {
    using PatchType = GrPathTessellationShader::PatchType;
    GrPathTessellationShader* shader;
    if (caps.shaderCaps()->tessellationSupport() &&
        !pipeline.usesVaryingCoords() &&  // Our tessellation back door doesn't handle varyings.
        numPathVerbs >= caps.minPathVerbsForHwTessellation()) {
        shader = GrPathTessellationShader::MakeHardwareTessellationShader(arena, viewMatrix, color,
                                                                          PatchType::kCurves);
    } else {
        shader = GrPathTessellationShader::MakeMiddleOutFixedCountShader(arena, viewMatrix, color,
                                                                         PatchType::kCurves);
    }
    return arena->make([=](void* objStart) {
        return new(objStart) GrPathCurveTessellator(shader, drawInnerFan);
    });
}

void GrPathCurveTessellator::prepare(GrMeshDrawOp::Target* target, const SkRect& cullBounds,
                                     const SkPath& path,
                                     const BreadcrumbTriangleList* breadcrumbTriangleList) {
    SkASSERT(fVertexChunkArray.empty());

    // Determine how many triangles to allocate.
    int maxTriangles = 0;
    if (fDrawInnerFan) {
        // An n-sided polygon is fanned by n-2 triangles.
        int maxEdgesInFan = GrPathTessellator::MaxSegmentsInPath(path);
        int maxTrianglesInFan = std::max(maxEdgesInFan - 2, 0);
        maxTriangles += maxTrianglesInFan;
    }
    if (breadcrumbTriangleList) {
        maxTriangles += breadcrumbTriangleList->count();
    }
    // Over-allocate enough curves for 1 in 4 to chop.
    int curveAllocCount = (path.countVerbs() * 5 + 3) / 4;  // i.e., ceil(numVerbs * 5/4)
    int patchAllocCount = maxTriangles + curveAllocCount;
    if (!patchAllocCount) {
        return;
    }
    GrVertexChunkBuilder chunker(target, &fVertexChunkArray, sizeof(SkPoint) * 4, patchAllocCount);

    // Write out the triangles.
    if (maxTriangles) {
        GrVertexWriter vertexWriter = chunker.appendVertices(maxTriangles);
        if (!vertexWriter) {
            return;
        }
        int numRemainingTriangles = maxTriangles;
        if (fDrawInnerFan) {
            int numWritten = GrMiddleOutPolygonTriangulator::WritePathInnerFan(
                    &vertexWriter,
                    GrMiddleOutPolygonTriangulator::OutputType::kConicsWithInfiniteWeight, path);
            numRemainingTriangles -= numWritten;
        }
        if (breadcrumbTriangleList) {
            int numWritten = 0;
            SkDEBUGCODE(int count = 0;)
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
        maxSegments = target->caps().shaderCaps()->maxTessellationSegments() * 2;
    } else {
        maxSegments = kMaxFixedCountSegments;
    }

    CurveWriter curveWriter(cullBounds, fShader->viewMatrix(), maxSegments);
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kQuad:
                curveWriter.writeQuadratic(&chunker, pts);
                break;
            case SkPathVerb::kConic:
                curveWriter.writeConic(&chunker, pts, *w);
                break;
            case SkPathVerb::kCubic:
                curveWriter.writeCubic(&chunker, pts);
                break;
            default:
                break;
        }
    }

    if (!fShader->willUseTessellationShaders()) {
        // log2(n) == log16(n^4).
        int fixedResolveLevel = GrWangsFormula::nextlog16(curveWriter.numFixedSegments_pow4());
        fFixedVertexCount =
                GrPathTessellationShader::NumCurveTrianglesAtResolveLevel(fixedResolveLevel) * 3;
    }
}

void GrPathCurveTessellator::draw(GrOpFlushState* flushState) const {
    if (fShader->willUseTessellationShaders()) {
        for (const GrVertexChunk& chunk : fVertexChunkArray) {
            flushState->bindBuffers(nullptr, nullptr, chunk.fBuffer);
            flushState->draw(chunk.fCount * 4, chunk.fBase * 4);
        }
    } else {
        SkASSERT(fShader->hasInstanceAttributes());
        for (const GrVertexChunk& chunk : fVertexChunkArray) {
            flushState->bindBuffers(nullptr, chunk.fBuffer, nullptr);
            flushState->drawInstanced(chunk.fCount, chunk.fBase, fFixedVertexCount, 0);
        }
    }
}

void GrPathCurveTessellator::drawHullInstances(GrOpFlushState* flushState) const {
    for (const GrVertexChunk& chunk : fVertexChunkArray) {
        flushState->bindBuffers(nullptr, chunk.fBuffer, nullptr);
        flushState->drawInstanced(chunk.fCount, chunk.fBase, 4, 0);
    }
}

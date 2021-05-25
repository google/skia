/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathTessellator.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/geometry/GrWangsFormula.h"
#include "src/gpu/tessellate/GrCullTest.h"
#include "src/gpu/tessellate/GrMiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/GrMidpointContourParser.h"
#include "src/gpu/tessellate/GrStencilPathShader.h"

constexpr static float kPrecision = GrTessellationPathRenderer::kLinearizationPrecision;

static bool can_use_hardware_tessellation(const SkPath& path, const GrCaps& caps) {
    if (!caps.shaderCaps()->tessellationSupport()) {
        return false;
    }
    // Only use hardware tessellation if we're drawing a somewhat large number of verbs. Otherwise
    // we seem to be better off using instanced draws.
    return path.countVerbs() >= caps.minPathVerbsForHwTessellation();
}

GrPathTessellator* GrPathTessellator::Make(SkArenaAlloc* arena, const SkMatrix& viewMatrix,
                                           const SkPath& path, DrawInnerFan drawInnerFan,
                                           const GrCaps& caps) {
    if (can_use_hardware_tessellation(path, caps)) {
        if (drawInnerFan == DrawInnerFan::kNo) {
            return GrPathOuterCurveTessellator::Make(arena, viewMatrix, drawInnerFan);
        } else {
            return GrPathWedgeTessellator::Make(arena, viewMatrix);
        }
    } else {
        return GrPathIndirectTessellator::Make(arena, viewMatrix, path, drawInnerFan);
    }
}

GrPathTessellator* GrPathIndirectTessellator::Make(SkArenaAlloc* arena, const SkMatrix& viewMatrix,
                                                   const SkPath& path, DrawInnerFan drawInnerFan) {
    auto shader = arena->make<GrCurveMiddleOutShader>(viewMatrix);
    return arena->make<GrPathIndirectTessellator>(shader, path, drawInnerFan);
}

GrPathIndirectTessellator::GrPathIndirectTessellator(GrStencilPathShader* shader,
                                                     const SkPath& path, DrawInnerFan drawInnerFan)
        : GrPathTessellator(shader)
        , fDrawInnerFan(drawInnerFan != DrawInnerFan::kNo) {
    // Count the number of instances at each resolveLevel.
    GrVectorXform xform(fShader->viewMatrix());
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        int level;
        switch (verb) {
            case SkPathVerb::kConic:
                level = GrWangsFormula::conic_log2(1/kPrecision, pts, *w, xform);
                break;
            case SkPathVerb::kQuad:
                level = GrWangsFormula::quadratic_log2(kPrecision, pts, xform);
                break;
            case SkPathVerb::kCubic:
                level = GrWangsFormula::cubic_log2(kPrecision, pts, xform);
                break;
            default:
                continue;
        }
        SkASSERT(level >= 0);
        // Instances with 2^0=1 segments are empty (zero area). We ignore them completely.
        if (level > 0) {
            level = std::min(level, kMaxResolveLevel);
            ++fResolveLevelCounts[level];
            ++fOuterCurveInstanceCount;
        }
    }
}

// Returns an upper bound on the number of segments (lineTo, quadTo, conicTo, cubicTo) in a path,
// also accounting for any implicit lineTos from closing contours.
static int max_segments_in_path(const SkPath& path) {
    // There might be an implicit kClose at the end, but the path always begins with kMove. So the
    // max number of segments in the path is equal to the number of verbs.
    SkASSERT(path.countVerbs() == 0 || SkPathPriv::VerbData(path)[0] == SkPath::kMove_Verb);
    return path.countVerbs();
}

// Returns an upper bound on the number of triangles it would require to fan a path's inner polygon,
// in the case where no additional vertices are introduced.
static int max_triangles_in_inner_fan(const SkPath& path) {
    int maxEdgesInFan = max_segments_in_path(path);
    return std::max(maxEdgesInFan - 2, 0);  // An n-sided polygon is fanned by n-2 triangles.
}

static int write_breadcrumb_triangles(
        GrVertexWriter* writer,
        const GrInnerFanTriangulator::BreadcrumbTriangleList* breadcrumbTriangleList) {
    int numWritten = 0;
    SkDEBUGCODE(int count = 0;)
    for (const auto* tri = breadcrumbTriangleList->head(); tri; tri = tri->fNext) {
        SkDEBUGCODE(++count;)
        const SkPoint* p = tri->fPts;
        if ((p[0].fX == p[1].fX && p[1].fX == p[2].fX) ||
            (p[0].fY == p[1].fY && p[1].fY == p[2].fY)) {
            // Completely degenerate triangles have undefined winding. And T-junctions shouldn't
            // happen on axis-aligned edges.
            continue;
        }
        writer->writeArray(p, 3);
        // Mark this instance as a triangle by setting it to a conic with w=Inf.
        writer->fill(GrVertexWriter::kIEEE_32_infinity, 2);
        ++numWritten;
    }
    SkASSERT(count == breadcrumbTriangleList->count());
    return numWritten;
}

void GrPathIndirectTessellator::prepare(GrMeshDrawOp::Target* target, const SkRect& /*cullBounds*/,
                                        const SkPath& path,
                                        const BreadcrumbTriangleList* breadcrumbTriangleList) {
    SkASSERT(fTotalInstanceCount == 0);
    SkASSERT(fIndirectDrawCount == 0);
    SkASSERT(target->caps().drawInstancedSupport());

    int instanceLockCount = fOuterCurveInstanceCount;
    if (fDrawInnerFan) {
        instanceLockCount += max_triangles_in_inner_fan(path);
    }
    if (breadcrumbTriangleList) {
        instanceLockCount += breadcrumbTriangleList->count();
    }
    if (instanceLockCount == 0) {
        return;
    }

    // Allocate a buffer to store the instance data.
    GrEagerDynamicVertexAllocator vertexAlloc(target, &fInstanceBuffer, &fBaseInstance);
    GrVertexWriter instanceWriter = static_cast<SkPoint*>(vertexAlloc.lock(sizeof(SkPoint) * 4,
                                                                           instanceLockCount));
    if (!instanceWriter) {
        return;
    }

    // Write out any triangles at the beginning of the cubic data. Since this shader draws curves,
    // output the triangles as conics with w=infinity (which is equivalent to a triangle).
    int numTrianglesAtBeginningOfData = 0;
    if (fDrawInnerFan) {
        numTrianglesAtBeginningOfData = GrMiddleOutPolygonTriangulator::WritePathInnerFan(
                &instanceWriter,
                GrMiddleOutPolygonTriangulator::OutputType::kConicsWithInfiniteWeight, path);
    }
    if (breadcrumbTriangleList) {
        numTrianglesAtBeginningOfData += write_breadcrumb_triangles(&instanceWriter,
                                                                    breadcrumbTriangleList);
    }

    // Allocate space for the GrDrawIndexedIndirectCommand structs. Allocate enough for each
    // possible resolve level (kMaxResolveLevel; resolveLevel=0 never has any instances), plus one
    // more for the optional inner fan triangles.
    int indirectLockCnt = kMaxResolveLevel + 1;
    GrDrawIndirectWriter indirectWriter = target->makeDrawIndirectSpace(indirectLockCnt,
                                                                        &fIndirectDrawBuffer,
                                                                        &fIndirectDrawOffset);
    if (!indirectWriter) {
        SkASSERT(!fIndirectDrawBuffer);
        vertexAlloc.unlock(0);
        return;
    }

    // Fill out the GrDrawIndexedIndirectCommand structs and determine the starting instance data
    // location at each resolve level.
    GrVertexWriter instanceLocations[kMaxResolveLevel + 1];
    int currentBaseInstance = fBaseInstance;
    SkASSERT(fResolveLevelCounts[0] == 0);
    for (int resolveLevel=1, numExtraInstances=numTrianglesAtBeginningOfData;
         resolveLevel <= kMaxResolveLevel;
         ++resolveLevel, numExtraInstances=0) {
        int instanceCountAtCurrLevel = fResolveLevelCounts[resolveLevel];
        if (!(instanceCountAtCurrLevel + numExtraInstances)) {
            SkDEBUGCODE(instanceLocations[resolveLevel] = nullptr;)
            continue;
        }
        instanceLocations[resolveLevel] = instanceWriter.makeOffset(0);
        SkASSERT(fIndirectDrawCount < indirectLockCnt);
        GrCurveMiddleOutShader::WriteDrawIndirectCmd(&indirectWriter, resolveLevel,
                                                     instanceCountAtCurrLevel + numExtraInstances,
                                                     currentBaseInstance);
        ++fIndirectDrawCount;
        currentBaseInstance += instanceCountAtCurrLevel + numExtraInstances;
        instanceWriter = instanceWriter.makeOffset(instanceCountAtCurrLevel * 4 * sizeof(SkPoint));
    }

    target->putBackIndirectDraws(indirectLockCnt - fIndirectDrawCount);

#ifdef SK_DEBUG
    SkASSERT(currentBaseInstance ==
             fBaseInstance + numTrianglesAtBeginningOfData + fOuterCurveInstanceCount);

    GrVertexWriter endLocations[kMaxResolveLevel + 1];
    int lastResolveLevel = 0;
    for (int resolveLevel = 1; resolveLevel <= kMaxResolveLevel; ++resolveLevel) {
        if (!instanceLocations[resolveLevel]) {
            endLocations[resolveLevel] = nullptr;
            continue;
        }
        endLocations[lastResolveLevel] = instanceLocations[resolveLevel].makeOffset(0);
        lastResolveLevel = resolveLevel;
    }
    endLocations[lastResolveLevel] = instanceWriter.makeOffset(0);
#endif

    fTotalInstanceCount = numTrianglesAtBeginningOfData;

    // Write out the cubic instances.
    if (fOuterCurveInstanceCount) {
        GrVectorXform xform(fShader->viewMatrix());
        for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
            int level;
            switch (verb) {
                default:
                    continue;
                case SkPathVerb::kConic:
                    level = GrWangsFormula::conic_log2(1/kPrecision, pts, *w, xform);
                    break;
                case SkPathVerb::kQuad:
                    level = GrWangsFormula::quadratic_log2(kPrecision, pts, xform);
                    break;
                case SkPathVerb::kCubic:
                    level = GrWangsFormula::cubic_log2(kPrecision, pts, xform);
                    break;
            }
            if (level == 0) {
                continue;
            }
            level = std::min(level, kMaxResolveLevel);
            switch (verb) {
                case SkPathVerb::kQuad:
                    GrPathUtils::writeQuadAsCubic(pts, &instanceLocations[level]);
                    break;
                case SkPathVerb::kCubic:
                    instanceLocations[level].writeArray(pts, 4);
                    break;
                case SkPathVerb::kConic:
                    GrPathShader::WriteConicPatch(pts, *w, &instanceLocations[level]);
                    break;
                default:
                    SkUNREACHABLE;
            }
            ++fTotalInstanceCount;
        }
    }

#ifdef SK_DEBUG
    for (int i = 1; i <= kMaxResolveLevel; ++i) {
        SkASSERT(instanceLocations[i] == endLocations[i]);
    }
    SkASSERT(fTotalInstanceCount == numTrianglesAtBeginningOfData + fOuterCurveInstanceCount);
#endif

    vertexAlloc.unlock(fTotalInstanceCount);
}

void GrPathIndirectTessellator::draw(GrOpFlushState* flushState) const {
    if (fIndirectDrawCount) {
        flushState->bindBuffers(nullptr, fInstanceBuffer, nullptr);
        flushState->drawIndirect(fIndirectDrawBuffer.get(), fIndirectDrawOffset,
                                 fIndirectDrawCount);
    }
}

void GrPathIndirectTessellator::drawHullInstances(GrOpFlushState* flushState) const {
    if (fTotalInstanceCount) {
        flushState->bindBuffers(nullptr, fInstanceBuffer, nullptr);
        flushState->drawInstanced(fTotalInstanceCount, fBaseInstance, 4, 0);
    }
}

GrPathTessellator* GrPathOuterCurveTessellator::Make(SkArenaAlloc* arena,
                                                     const SkMatrix& viewMatrix,
                                                     DrawInnerFan drawInnerFan) {
    auto shader = arena->make<GrCurveTessellateShader>(viewMatrix);
    return arena->make<GrPathOuterCurveTessellator>(shader, drawInnerFan);
}

void GrPathOuterCurveTessellator::prepare(GrMeshDrawOp::Target* target, const SkRect& cullBounds,
                                          const SkPath& path,
                                          const BreadcrumbTriangleList* breadcrumbTriangleList) {
    SkASSERT(target->caps().shaderCaps()->tessellationSupport());
    SkASSERT(fVertexChunkArray.empty());

    // Determine how many triangles to allocate.
    int maxTriangles = 0;
    if (fDrawInnerFan) {
        maxTriangles += max_triangles_in_inner_fan(path);
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
            int numWritten = write_breadcrumb_triangles(&vertexWriter, breadcrumbTriangleList);
            numRemainingTriangles -= numWritten;
        }
        chunker.popVertices(numRemainingTriangles);
    }

    // Writes out curve patches, chopping as necessary so none require more segments than are
    // supported by the hardware.
    class CurveWriter {
    public:
        CurveWriter(const SkRect& cullBounds, const SkMatrix& viewMatrix,
                    const GrShaderCaps& shaderCaps)
                : fCullTest(cullBounds, viewMatrix)
                , fVectorXform(viewMatrix) {
            // GrCurveTessellateShader tessellates T=0..(1/2) on the first side of the triangle and
            // T=(1/2)..1 on the second side. This means we get double the max tessellation segments
            // for the range T=0..1.
            float maxSegments = shaderCaps.maxTessellationSegments() * 2;
            fMaxSegments_pow2 = maxSegments * maxSegments;
            fMaxSegments_pow4 = fMaxSegments_pow2 * fMaxSegments_pow2;
        }

        SK_ALWAYS_INLINE void writeQuadratic(GrVertexChunkBuilder* chunker, const SkPoint p[3]) {
            if (GrWangsFormula::quadratic_pow4(kPrecision, p, fVectorXform) > fMaxSegments_pow4) {
                this->chopAndWriteQuadratic(chunker, p);
                return;
            }
            if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
                GrPathUtils::writeQuadAsCubic(p, &vertexWriter);
            }
        }

        SK_ALWAYS_INLINE void writeConic(GrVertexChunkBuilder* chunker, const SkPoint p[3],
                                         float w) {
            if (GrWangsFormula::conic_pow2(1/kPrecision, p, w, fVectorXform) > fMaxSegments_pow2) {
                this->chopAndWriteConic(chunker, {p, w});
                return;
            }
            if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
                GrPathShader::WriteConicPatch(p, w, &vertexWriter);
            }
        }

        SK_ALWAYS_INLINE void writeCubic(GrVertexChunkBuilder* chunker, const SkPoint p[4]) {
            if (GrWangsFormula::cubic_pow4(kPrecision, p, fVectorXform) > fMaxSegments_pow4) {
                this->chopAndWriteCubic(chunker, p);
                return;
            }
            if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
                vertexWriter.writeArray(p, 4);
            }
        }

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
        float fMaxSegments_pow2;
        float fMaxSegments_pow4;
    };

    CurveWriter curveWriter(cullBounds, fShader->viewMatrix(), *target->caps().shaderCaps());
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
}

GrPathTessellator* GrPathWedgeTessellator::Make(SkArenaAlloc* arena, const SkMatrix& viewMatrix) {
    auto shader = arena->make<GrWedgeTessellateShader>(viewMatrix);
    return arena->make<GrPathWedgeTessellator>(shader);
}

void GrPathWedgeTessellator::prepare(GrMeshDrawOp::Target* target, const SkRect& cullBounds,
                                     const SkPath& path,
                                     const BreadcrumbTriangleList* breadcrumbTriangleList) {
    SkASSERT(target->caps().shaderCaps()->tessellationSupport());
    SkASSERT(!breadcrumbTriangleList);
    SkASSERT(fVertexChunkArray.empty());

    // Over-allocate enough wedges for 1 in 4 to chop.
    int maxWedges = max_segments_in_path(path);
    int wedgeAllocCount = (maxWedges * 5 + 3) / 4;  // i.e., ceil(maxWedges * 5/4)
    if (!wedgeAllocCount) {
        return;
    }
    GrVertexChunkBuilder chunker(target, &fVertexChunkArray, sizeof(SkPoint) * 5, wedgeAllocCount);

    // Writes out wedge patches, chopping as necessary so none require more segments than are
    // supported by the hardware.
    class WedgeWriter {
    public:
        WedgeWriter(const SkRect& cullBounds, const SkMatrix& viewMatrix,
                    const GrShaderCaps& shaderCaps)
                : fCullTest(cullBounds, viewMatrix)
                , fVectorXform(viewMatrix) {
            float maxSegments = shaderCaps.maxTessellationSegments();
            fMaxSegments_pow2 = maxSegments * maxSegments;
            fMaxSegments_pow4 = fMaxSegments_pow2 * fMaxSegments_pow2;
        }

        SK_ALWAYS_INLINE void writeFlatWedge(GrVertexChunkBuilder* chunker, SkPoint p0, SkPoint p1,
                                             SkPoint midpoint) {
            if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
                GrPathUtils::writeLineAsCubic(p0, p1, &vertexWriter);
                vertexWriter.write(midpoint);
            }
        }

        SK_ALWAYS_INLINE void writeQuadraticWedge(GrVertexChunkBuilder* chunker, const SkPoint p[3],
                                                  SkPoint midpoint) {
            if (GrWangsFormula::quadratic_pow4(kPrecision, p, fVectorXform) > fMaxSegments_pow4) {
                this->chopAndWriteQuadraticWedges(chunker, p, midpoint);
                return;
            }
            if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
                GrPathUtils::writeQuadAsCubic(p, &vertexWriter);
                vertexWriter.write(midpoint);
            }
        }

        SK_ALWAYS_INLINE void writeConicWedge(GrVertexChunkBuilder* chunker, const SkPoint p[3],
                                              float w, SkPoint midpoint) {
            if (GrWangsFormula::conic_pow2(1/kPrecision, p, w, fVectorXform) > fMaxSegments_pow2) {
                this->chopAndWriteConicWedges(chunker, {p, w}, midpoint);
                return;
            }
            if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
                GrPathShader::WriteConicPatch(p, w, &vertexWriter);
                vertexWriter.write(midpoint);
            }
        }

        SK_ALWAYS_INLINE void writeCubicWedge(GrVertexChunkBuilder* chunker, const SkPoint p[4],
                                              SkPoint midpoint) {
            if (GrWangsFormula::cubic_pow4(kPrecision, p, fVectorXform) > fMaxSegments_pow4) {
                this->chopAndWriteCubicWedges(chunker, p, midpoint);
                return;
            }
            if (GrVertexWriter vertexWriter = chunker->appendVertex()) {
                vertexWriter.writeArray(p, 4);
                vertexWriter.write(midpoint);
            }
        }

    private:
        void chopAndWriteQuadraticWedges(GrVertexChunkBuilder* chunker, const SkPoint p[3],
                                         SkPoint midpoint) {
            SkPoint chops[5];
            SkChopQuadAtHalf(p, chops);
            for (int i = 0; i < 2; ++i) {
                const SkPoint* q = chops + i*2;
                if (fCullTest.areVisible3(q)) {
                    this->writeQuadraticWedge(chunker, q, midpoint);
                } else {
                    this->writeFlatWedge(chunker, q[0], q[2], midpoint);
                }
            }
        }

        void chopAndWriteConicWedges(GrVertexChunkBuilder* chunker, const SkConic& conic,
                                     SkPoint midpoint) {
            SkConic chops[2];
            if (!conic.chopAt(.5, chops)) {
                return;
            }
            for (int i = 0; i < 2; ++i) {
                if (fCullTest.areVisible3(chops[i].fPts)) {
                    this->writeConicWedge(chunker, chops[i].fPts, chops[i].fW, midpoint);
                } else {
                    this->writeFlatWedge(chunker, chops[i].fPts[0], chops[i].fPts[2], midpoint);
                }
            }
        }

        void chopAndWriteCubicWedges(GrVertexChunkBuilder* chunker, const SkPoint p[4],
                                     SkPoint midpoint) {
            SkPoint chops[7];
            SkChopCubicAtHalf(p, chops);
            for (int i = 0; i < 2; ++i) {
                const SkPoint* c = chops + i*3;
                if (fCullTest.areVisible4(c)) {
                    this->writeCubicWedge(chunker, c, midpoint);
                } else {
                    this->writeFlatWedge(chunker, c[0], c[3], midpoint);
                }
            }
        }

        GrCullTest fCullTest;
        GrVectorXform fVectorXform;
        float fMaxSegments_pow2;
        float fMaxSegments_pow4;
    };

    WedgeWriter wedgeWriter(cullBounds, fShader->viewMatrix(), *target->caps().shaderCaps());
    GrMidpointContourParser parser(path);
    while (parser.parseNextContour()) {
        SkPoint midpoint = parser.currentMidpoint();
        SkPoint startPoint = {0, 0};
        SkPoint lastPoint = startPoint;
        for (auto [verb, pts, w] : parser.currentContour()) {
            switch (verb) {
                case SkPathVerb::kMove:
                    startPoint = lastPoint = pts[0];
                    break;
                case SkPathVerb::kClose:
                    break;  // Ignore. We can assume an implicit close at the end.
                case SkPathVerb::kLine:
                    wedgeWriter.writeFlatWedge(&chunker, pts[0], pts[1], midpoint);
                    lastPoint = pts[1];
                    break;
                case SkPathVerb::kQuad:
                    wedgeWriter.writeQuadraticWedge(&chunker, pts, midpoint);
                    lastPoint = pts[2];
                    break;
                case SkPathVerb::kConic:
                    wedgeWriter.writeConicWedge(&chunker, pts, *w, midpoint);
                    lastPoint = pts[2];
                    break;
                case SkPathVerb::kCubic:
                    wedgeWriter.writeCubicWedge(&chunker, pts, midpoint);
                    lastPoint = pts[3];
                    break;
            }
        }
        if (lastPoint != startPoint) {
            wedgeWriter.writeFlatWedge(&chunker, lastPoint, startPoint, midpoint);
        }
    }
}

void GrPathHardwareTessellator::draw(GrOpFlushState* flushState) const {
    for (const GrVertexChunk& chunk : fVertexChunkArray) {
        flushState->bindBuffers(nullptr, nullptr, chunk.fBuffer);
        flushState->draw(chunk.fCount * fNumVerticesPerPatch, chunk.fBase * fNumVerticesPerPatch);
        if (flushState->caps().requiresManualFBBarrierAfterTessellatedStencilDraw()) {
            flushState->gpu()->insertManualFramebufferBarrier();  // http://skbug.com/9739
        }
    }
}

void GrPathOuterCurveTessellator::drawHullInstances(GrOpFlushState* flushState) const {
    SkASSERT(fNumVerticesPerPatch == 4);
    for (const GrVertexChunk& chunk : fVertexChunkArray) {
        flushState->bindBuffers(nullptr, chunk.fBuffer, nullptr);
        flushState->drawInstanced(chunk.fCount, chunk.fBase, 4, 0);
    }
}

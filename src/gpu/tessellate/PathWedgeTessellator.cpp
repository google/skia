/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/PathWedgeTessellator.h"

#include "src/gpu/GrMeshDrawTarget.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/tessellate/AffineMatrix.h"
#include "src/gpu/tessellate/PatchWriter.h"
#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/WangsFormula.h"
#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

#if SK_GPU_V1
#include "src/gpu/GrOpFlushState.h"
#endif

namespace skgpu {

using CubicPatch = PatchWriter::CubicPatch;
using ConicPatch = PatchWriter::ConicPatch;

namespace {

// Parses out each contour in a path and tracks the midpoint. Example usage:
//
//   SkTPathContourParser parser;
//   while (parser.parseNextContour()) {
//       SkPoint midpoint = parser.currentMidpoint();
//       for (auto [verb, pts] : parser.currentContour()) {
//           ...
//       }
//   }
//
class MidpointContourParser {
public:
    MidpointContourParser(const SkPath& path)
            : fPath(path)
            , fVerbs(SkPathPriv::VerbData(fPath))
            , fNumRemainingVerbs(fPath.countVerbs())
            , fPoints(SkPathPriv::PointData(fPath))
            , fWeights(SkPathPriv::ConicWeightData(fPath)) {}
    // Advances the internal state to the next contour in the path. Returns false if there are no
    // more contours.
    bool parseNextContour() {
        bool hasGeometry = false;
        for (; fVerbsIdx < fNumRemainingVerbs; ++fVerbsIdx) {
            switch (fVerbs[fVerbsIdx]) {
                case SkPath::kMove_Verb:
                    if (!hasGeometry) {
                        fMidpoint = {0,0};
                        fMidpointWeight = 0;
                        this->advance();  // Resets fPtsIdx to 0 and advances fPoints.
                        fPtsIdx = 1;  // Increment fPtsIdx past the kMove.
                        continue;
                    }
                    if (fPoints[0] != fPoints[fPtsIdx - 1]) {
                        // There's an implicit close at the end. Add the start point to our mean.
                        fMidpoint += fPoints[0];
                        ++fMidpointWeight;
                    }
                    return true;
                default:
                    continue;
                case SkPath::kLine_Verb:
                    ++fPtsIdx;
                    break;
                case SkPath::kConic_Verb:
                    ++fWtsIdx;
                    [[fallthrough]];
                case SkPath::kQuad_Verb:
                    fPtsIdx += 2;
                    break;
                case SkPath::kCubic_Verb:
                    fPtsIdx += 3;
                    break;
            }
            fMidpoint += fPoints[fPtsIdx - 1];
            ++fMidpointWeight;
            hasGeometry = true;
        }
        if (hasGeometry && fPoints[0] != fPoints[fPtsIdx - 1]) {
            // There's an implicit close at the end. Add the start point to our mean.
            fMidpoint += fPoints[0];
            ++fMidpointWeight;
        }
        return hasGeometry;
    }

    // Allows for iterating the current contour using a range-for loop.
    SkPathPriv::Iterate currentContour() {
        return SkPathPriv::Iterate(fVerbs, fVerbs + fVerbsIdx, fPoints, fWeights);
    }

    SkPoint currentMidpoint() { return fMidpoint * (1.f / fMidpointWeight); }

private:
    void advance() {
        fVerbs += fVerbsIdx;
        fNumRemainingVerbs -= fVerbsIdx;
        fVerbsIdx = 0;
        fPoints += fPtsIdx;
        fPtsIdx = 0;
        fWeights += fWtsIdx;
        fWtsIdx = 0;
    }

    const SkPath& fPath;

    const uint8_t* fVerbs;
    int fNumRemainingVerbs = 0;
    int fVerbsIdx = 0;

    const SkPoint* fPoints;
    int fPtsIdx = 0;

    const float* fWeights;
    int fWtsIdx = 0;

    SkPoint fMidpoint;
    int fMidpointWeight;
};

}  // namespace

PathTessellator* PathWedgeTessellator::Make(SkArenaAlloc* arena,
                                            const SkMatrix& viewMatrix,
                                            const SkPMColor4f& color,
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
                                                                          PatchType::kWedges);
    } else {
        shader = GrPathTessellationShader::MakeMiddleOutFixedCountShader(*caps.shaderCaps(), arena,
                                                                         viewMatrix, color,
                                                                         PatchType::kWedges);
    }
    return arena->make([=](void* objStart) {
        return new(objStart) PathWedgeTessellator(shader);
    });
}

GR_DECLARE_STATIC_UNIQUE_KEY(gFixedCountVertexBufferKey);
GR_DECLARE_STATIC_UNIQUE_KEY(gFixedCountIndexBufferKey);

void PathWedgeTessellator::prepare(GrMeshDrawTarget* target,
                                   const PathDrawList& pathDrawList,
                                   int totalCombinedPathVerbCnt) {
    SkASSERT(fVertexChunkArray.empty());

    const GrShaderCaps& shaderCaps = *target->caps().shaderCaps();

    // Over-allocate enough wedges for 1 in 4 to chop.
    int maxWedges = MaxCombinedFanEdgesInPathDrawList(totalCombinedPathVerbCnt);
    int wedgeAllocCount = (maxWedges * 5 + 3) / 4;  // i.e., ceil(maxWedges * 5/4)
    if (!wedgeAllocCount) {
        return;
    }
    size_t patchStride = fShader->willUseTessellationShaders() ? fShader->vertexStride() * 5
                                                               : fShader->instanceStride();

    auto attribs = PatchAttribs::kFanPoint;
    if (!shaderCaps.infinitySupport()) {
        attribs |= PatchAttribs::kExplicitCurveType;
    }
    PatchWriter patchWriter(target, &fVertexChunkArray, patchStride, wedgeAllocCount, attribs);

    int maxFixedCountResolveLevel = GrPathTessellationShader::kMaxFixedCountResolveLevel;
    int maxSegments;
    if (fShader->willUseTessellationShaders()) {
        maxSegments = shaderCaps.maxTessellationSegments();
    } else {
        maxSegments = 1 << maxFixedCountResolveLevel;
    }
    float maxSegments_pow2 = pow2(maxSegments);
    float maxSegments_pow4 = pow2(maxSegments_pow2);

    // If using fixed count, this is the number of segments we need to emit per instance. Always
    // emit at least 1 segment.
    float numFixedSegments_pow4 = 1;

    for (auto [pathMatrix, path] : pathDrawList) {
        AffineMatrix m(pathMatrix);
        wangs_formula::VectorXform totalXform(SkMatrix::Concat(fShader->viewMatrix(), pathMatrix));
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
                        CubicPatch(patchWriter) << LineToCubic{m.map2Points(pts)};
                        lastPoint = pts[1];
                        break;
                    }

                    case SkPathVerb::kQuad: {
                        auto [p0, p1] = m.map2Points(pts);
                        auto p2 = m.map1Point(pts+2);
                        float n4 = wangs_formula::quadratic_pow4(kTessellationPrecision,
                                                                 pts,
                                                                 totalXform);
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
                        lastPoint = pts[2];
                        break;
                    }

                    case SkPathVerb::kConic: {
                        auto [p0, p1] = m.map2Points(pts);
                        auto p2 = m.map1Point(pts+2);
                        float n2 = wangs_formula::conic_pow2(kTessellationPrecision,
                                                             pts,
                                                             *w,
                                                             totalXform);
                        if (n2 <= maxSegments_pow2) {
                            // This conic already fits into "maxSegments" tessellation segments.
                            ConicPatch(patchWriter) << p0 << p1 << p2 << *w;
                        } else {
                            // Chop until each conic tessellation requires "maxSegments" or fewer.
                            int numPatches = SkScalarCeilToInt(sqrtf(n2/maxSegments_pow2));
                            patchWriter.chopAndWriteConics(p0, p1, p2, *w, numPatches);
                        }
                        numFixedSegments_pow4 = std::max(n2*n2, numFixedSegments_pow4);
                        lastPoint = pts[2];
                        break;
                    }

                    case SkPathVerb::kCubic: {
                        auto [p0, p1] = m.map2Points(pts);
                        auto [p2, p3] = m.map2Points(pts+2);
                        float n4 = wangs_formula::cubic_pow4(kTessellationPrecision,
                                                             pts,
                                                             totalXform);
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
                CubicPatch(patchWriter) << LineToCubic{m.map2Points(pts)};
            }
        }
    }

    if (!fShader->willUseTessellationShaders()) {
        // log16(n^4) == log2(n).
        // We already chopped curves to make sure none needed a higher resolveLevel than
        // kMaxFixedCountResolveLevel.
        int fixedResolveLevel = std::min(wangs_formula::nextlog16(numFixedSegments_pow4),
                                         maxFixedCountResolveLevel);
        int numCurveTriangles =
                GrPathTessellationShader::NumCurveTrianglesAtResolveLevel(fixedResolveLevel);
        // Emit 3 vertices per curve triangle, plus 3 more for the fan triangle.
        fFixedIndexCount = numCurveTriangles * 3 + 3;

        GR_DEFINE_STATIC_UNIQUE_KEY(gFixedCountVertexBufferKey);

        fFixedCountVertexBuffer = target->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kVertex,
                GrPathTessellationShader::SizeOfVertexBufferForMiddleOutWedges(),
                gFixedCountVertexBufferKey,
                GrPathTessellationShader::InitializeVertexBufferForMiddleOutWedges);

        GR_DEFINE_STATIC_UNIQUE_KEY(gFixedCountIndexBufferKey);

        fFixedCountIndexBuffer = target->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kIndex,
                GrPathTessellationShader::SizeOfIndexBufferForMiddleOutWedges(),
                gFixedCountIndexBufferKey,
                GrPathTessellationShader::InitializeIndexBufferForMiddleOutWedges);
    }
}

#if SK_GPU_V1
void PathWedgeTessellator::draw(GrOpFlushState* flushState) const {
    if (fShader->willUseTessellationShaders()) {
        for (const GrVertexChunk& chunk : fVertexChunkArray) {
            flushState->bindBuffers(nullptr, nullptr, chunk.fBuffer);
            flushState->draw(chunk.fCount * 5, chunk.fBase * 5);
        }
    } else {
        SkASSERT(fShader->hasInstanceAttributes());
        for (const GrVertexChunk& chunk : fVertexChunkArray) {
            flushState->bindBuffers(fFixedCountIndexBuffer, chunk.fBuffer, fFixedCountVertexBuffer);
            flushState->drawIndexedInstanced(fFixedIndexCount, 0, chunk.fCount, chunk.fBase, 0);
        }
    }
}
#endif

}  // namespace skgpu

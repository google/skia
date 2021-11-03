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
#include "src/gpu/tessellate/PathXform.h"
#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/WangsFormula.h"
#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

#if SK_GPU_V1
#include "src/gpu/GrOpFlushState.h"
#endif

namespace skgpu {

struct LineToCubic {
    float4 fP0P1;
};

static VertexWriter& operator<<(VertexWriter& vertexWriter, const LineToCubic& line) {
    float4 p0p1 = line.fP0P1;
    float4 v = p0p1.zwxy() - p0p1;
    return vertexWriter << p0p1.lo << (v * (1/3.f) + p0p1) << p0p1.hi;
}

struct QuadToCubic {
    float2 fP0, fP1, fP2;
};

static VertexWriter& operator<<(VertexWriter& vertexWriter, const QuadToCubic& quadratic) {
    auto [p0, p1, p2] = quadratic;
    return vertexWriter << p0 << mix(float4(p0,p2), p1.xyxy(), 2/3.f) << p2;
}

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
                        fMidpoint = fPoints[fPtsIdx];
                        fMidpointWeight = 1;
                        this->advance();
                        ++fPtsIdx;
                        continue;
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

// Writes out wedge patches, chopping as necessary so none require more segments than are supported
// by the hardware.
class WedgeWriter {
public:
    WedgeWriter(GrMeshDrawTarget* target,
                GrVertexChunkArray* vertexChunkArray,
                size_t patchStride,
                int initialPatchAllocCount,
                int maxSegments,
                const GrShaderCaps& shaderCaps)
            : fChunker(target, vertexChunkArray, patchStride, initialPatchAllocCount)
            , fMaxSegments_pow2(maxSegments * maxSegments)
            , fMaxSegments_pow4(fMaxSegments_pow2 * fMaxSegments_pow2)
            , fGPUInfinitySupport(shaderCaps.infinitySupport()) {
    }

    void setMatrices(const SkMatrix& shaderMatrix,
                     const SkMatrix& pathMatrix) {
        SkMatrix totalMatrix;
        totalMatrix.setConcat(shaderMatrix, pathMatrix);
        fTotalVectorXform = totalMatrix;
        fPathXform = pathMatrix;
    }

    void setMidpoint(SkPoint midpoint) { fMidpoint = fPathXform.mapPoint(midpoint); }

    void lineTo(const SkPoint p[2]) {
        CubicPatch(this) << LineToCubic{fPathXform.map2Points(p)};
    }

    void quadTo(const SkPoint p[3]) {
        auto [p0, p1] = fPathXform.map2Points(p);
        auto p2 = fPathXform.map1Point(p+2);
        float n4 = wangs_formula::quadratic_pow4(kTessellationPrecision, p, fTotalVectorXform);
        if (n4 <= fMaxSegments_pow4) {
            // This quad already fits into "maxSegments" tessellation segments.
            CubicPatch(this) << QuadToCubic{p0, p1, p2};
            fNumFixedSegments_pow4 = std::max(n4, fNumFixedSegments_pow4);
        } else {
            // Chop until each quad requires "maxSegments" tessellation segments or fewer.
            int numPatches = SkScalarCeilToInt(wangs_formula::root4(n4/fMaxSegments_pow4));
            for (; numPatches >= 3; numPatches -= 2) {
                // Chop into 3 quads.
                float4 T = float4(1,1,2,2) / numPatches;
                float4 ab = mix(p0.xyxy(), p1.xyxy(), T);
                float4 bc = mix(p1.xyxy(), p2.xyxy(), T);
                float4 abc = mix(ab, bc, T);
                // p1 & p2 of the cubic representation of the middle quad.
                float4 middle = mix(ab, bc, mix(T, T.zwxy(), 2/3.f));

                CubicPatch(this) << QuadToCubic{p0, ab.lo, abc.lo};  // Write 1st quad.
                CubicPatch(this) << abc.lo << middle << abc.hi;  // Write 2nd quad.
                std::tie(p0, p1) = {abc.hi, bc.hi};  // Save 3rd quad.
            }
            if (numPatches == 2) {
                // Chop into 2 quads.
                float2 ab = (p0 + p1) * .5f;
                float2 bc = (p1 + p2) * .5f;
                float2 abc = (ab + bc) * .5f;

                CubicPatch(this) << QuadToCubic{p0, ab, abc};  // Write 1st quad.
                CubicPatch(this) << QuadToCubic{abc, bc, p2};  // Write 2nd quad.
            } else {
                SkASSERT(numPatches == 1);
                CubicPatch(this) << QuadToCubic{p0, p1, p2};  // Write single quad.
            }
            fNumFixedSegments_pow4 = fMaxSegments_pow4;
        }
    }

    void conicTo(const SkPoint p[3], float w) {
        float n2 = wangs_formula::conic_pow2(kTessellationPrecision, p, w, fTotalVectorXform);
        if (n2 <= fMaxSegments_pow2) {
            // This conic already fits into "maxSegments" tessellation segments.
            ConicPatch(this) << fPathXform.map2Points(p) << fPathXform.map1Point(p+2) << w;
            fNumFixedSegments_pow4 = std::max(n2*n2, fNumFixedSegments_pow4);
        } else {
            // Load the conic in homogeneous (unprojected) space.
            float4 p0 = float4(fPathXform.map1Point(p), 1, 1);
            float4 p1 = float4(fPathXform.map1Point(p+1), 1, 1) * w;
            float4 p2 = float4(fPathXform.map1Point(p+2), 1, 1);
            // Chop until each conic requires "maxSegments" tessellation segments or fewer.
            int numPatches = SkScalarCeilToInt(sqrtf(n2/fMaxSegments_pow2));
            for (; numPatches >= 2; --numPatches) {
                // Chop in homogeneous space.
                float T = 1.f/numPatches;
                float4 ab = mix(p0, p1, T);
                float4 bc = mix(p1, p2, T);
                float4 abc = mix(ab, bc, T);

                // Project and write the 1st conic.
                ConicPatch(this) << (p0.xy() / p0.w())
                                 << (ab.xy() / ab.w())
                                 << (abc.xy() / abc.w())
                                 << (ab.w() / sqrtf(p0.w() * abc.w()));
                std::tie(p0, p1) = {abc, bc};  // Save the 2nd conic (in homogeneous space).
            }
            // Project and write the remaining conic.
            SkASSERT(numPatches == 1);
            ConicPatch(this) << (p0.xy() / p0.w())
                             << (p1.xy() / p1.w())
                             << p2.xy()  // p2.w == 1
                             << (p1.w() / sqrtf(p0.w()));
            fNumFixedSegments_pow4 = fMaxSegments_pow4;
        }
    }

    void cubicTo(const SkPoint p[4]) {
        auto [p0, p1] = fPathXform.map2Points(p);
        auto [p2, p3] = fPathXform.map2Points(p+2);
        float n4 = wangs_formula::cubic_pow4(kTessellationPrecision, p, fTotalVectorXform);
        if (n4 <= fMaxSegments_pow4) {
            // This cubic already fits into "maxSegments" tessellation segments.
            CubicPatch(this) << p0 << p1 << p2 << p3;
            fNumFixedSegments_pow4 = std::max(n4, fNumFixedSegments_pow4);
        } else {
            // Chop until each cubic requires "maxSegments" tessellation segments or fewer.
            int numPatches = SkScalarCeilToInt(wangs_formula::root4(n4/fMaxSegments_pow4));
            for (; numPatches >= 3; numPatches -= 2) {
                // Chop into 3 cubics.
                float4 T = float4(1,1,2,2) / numPatches;
                float4 ab = mix(p0.xyxy(), p1.xyxy(), T);
                float4 bc = mix(p1.xyxy(), p2.xyxy(), T);
                float4 cd = mix(p2.xyxy(), p3.xyxy(), T);
                float4 abc = mix(ab, bc, T);
                float4 bcd = mix(bc, cd, T);
                float4 abcd = mix(abc, bcd, T);
                float4 middle = mix(abc, bcd, T.zwxy());  // p1 & p2 of the middle cubic.

                CubicPatch(this) << p0 << ab.lo << abc.lo << abcd.lo;  // Write 1st cubic.
                CubicPatch(this) << abcd.lo << middle << abcd.hi;  // Write 2nd cubic.
                std::tie(p0, p1, p2) = {abcd.hi, bcd.hi, cd.hi};  // Save 3rd cubic.
            }
            if (numPatches == 2) {
                // Chop into 2 cubics.
                float2 ab = (p0 + p1) * .5f;
                float2 bc = (p1 + p2) * .5f;
                float2 cd = (p2 + p3) * .5f;
                float2 abc = (ab + bc) * .5f;
                float2 bcd = (bc + cd) * .5f;
                float2 abcd = (abc + bcd) * .5f;

                CubicPatch(this) << p0 << ab << abc << abcd;  // Write 1st cubic.
                CubicPatch(this) << abcd << bcd << cd << p3;  // Write 2nd cubic.
            } else {
                SkASSERT(numPatches == 1);
                CubicPatch(this) << p0 << p1 << p2 << p3;  // Write single cubic.
            }
            fNumFixedSegments_pow4 = fMaxSegments_pow4;
        }
    }

    float numFixedSegments_pow4() const { return fNumFixedSegments_pow4; }

private:
    template <typename T>
    static VertexWriter::Conditional<T> If(bool c, const T& v) { return VertexWriter::If(c,v); }

    // RAII. Appends a patch during construction and writes the remaining data for a cubic during
    // destruction. The caller outputs p0,p1,p2,p3 (8 floats):
    //
    //    CubicPatch(this) << p0 << p1 << p2 << p3;
    //
    struct CubicPatch {
        CubicPatch(WedgeWriter* _this) : fThis(_this), fVertexWriter(fThis->appendPatch()) {}
        ~CubicPatch() {
            fVertexWriter << fThis->fMidpoint
                          << If(!fThis->fGPUInfinitySupport, GrTessellationShader::kCubicCurveType);
        }
        operator VertexWriter&() { return fVertexWriter; }
        WedgeWriter* fThis;
        VertexWriter fVertexWriter;
    };

    // RAII. Appends a patch during construction and writes the remaining data for a conic during
    // destruction. The caller outputs p0,p1,p2,w (7 floats):
    //
    //     ConicPatch(this) << p0 << p1 << p2 << w;
    //
    struct ConicPatch {
        ConicPatch(WedgeWriter* _this) : fThis(_this), fVertexWriter(fThis->appendPatch()) {}
        ~ConicPatch() {
            fVertexWriter << VertexWriter::kIEEE_32_infinity  // p3.y=Inf indicates a conic.
                          << fThis->fMidpoint
                          << If(!fThis->fGPUInfinitySupport, GrTessellationShader::kConicCurveType);
        }
        operator VertexWriter&() { return fVertexWriter; }
        WedgeWriter* fThis;
        VertexWriter fVertexWriter;
    };

    VertexWriter appendPatch() {
        VertexWriter vertexWriter = fChunker.appendVertex();
        if (!vertexWriter) {
            // Failed to allocate GPU storage for the patch. Write to a throwaway location so the
            // callsites don't have to do null checks.
            if (!fFallbackPatchStorage) {
                fFallbackPatchStorage.reset(fChunker.stride());
            }
            vertexWriter = fFallbackPatchStorage.data();
        }
        return vertexWriter;
    }

    GrVertexChunkBuilder fChunker;
    const float fMaxSegments_pow2;
    const float fMaxSegments_pow4;
    const bool fGPUInfinitySupport;
    wangs_formula::VectorXform fTotalVectorXform;
    PathXform fPathXform;
    SkPoint fMidpoint;

    // For when fChunker fails to allocate a patch in GPU memory.
    SkAutoTMalloc<char> fFallbackPatchStorage;

    // If using fixed count, this is the max number of curve segments we need to draw per instance.
    float fNumFixedSegments_pow4 = 1;
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

    int maxSegments;
    if (fShader->willUseTessellationShaders()) {
        maxSegments = shaderCaps.maxTessellationSegments();
    } else {
        maxSegments = GrPathTessellationShader::kMaxFixedCountSegments;
    }
    WedgeWriter wedgeWriter(target, &fVertexChunkArray, patchStride, wedgeAllocCount, maxSegments,
                            shaderCaps);
    for (auto [pathMatrix, path] : pathDrawList) {
        wedgeWriter.setMatrices(fShader->viewMatrix(), pathMatrix);
        MidpointContourParser parser(path);
        while (parser.parseNextContour()) {
            wedgeWriter.setMidpoint(parser.currentMidpoint());
            SkPoint lastPoint = {0, 0};
            SkPoint startPoint = {0, 0};
            for (auto [verb, pts, w] : parser.currentContour()) {
                switch (verb) {
                    case SkPathVerb::kMove:
                        startPoint = lastPoint = pts[0];
                        break;
                    case SkPathVerb::kClose:
                        break;  // Ignore. We can assume an implicit close at the end.
                    case SkPathVerb::kLine:
                        wedgeWriter.lineTo(pts);
                        lastPoint = pts[1];
                        break;
                    case SkPathVerb::kQuad:
                        wedgeWriter.quadTo(pts);
                        lastPoint = pts[2];
                        break;
                    case SkPathVerb::kConic:
                        wedgeWriter.conicTo(pts, *w);
                        lastPoint = pts[2];
                        break;
                    case SkPathVerb::kCubic:
                        wedgeWriter.cubicTo(pts);
                        lastPoint = pts[3];
                        break;
                }
            }
            if (lastPoint != startPoint) {
                SkPoint pts[2] = {lastPoint, startPoint};
                wedgeWriter.lineTo(pts);
            }
        }
    }

    if (!fShader->willUseTessellationShaders()) {
        // log2(n) == log16(n^4).
        int fixedResolveLevel = wangs_formula::nextlog16(wedgeWriter.numFixedSegments_pow4());
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

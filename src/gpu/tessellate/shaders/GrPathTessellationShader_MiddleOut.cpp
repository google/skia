/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

#include "src/core/SkMathPriv.h"
#include "src/gpu/geometry/GrWangsFormula.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

namespace {

// Uses instanced draws to triangulate standalone closed curves with a "middle-out" topology.
// Middle-out draws a triangle with vertices at T=[0, 1/2, 1] and then recurses breadth first:
//
//   depth=0: T=[0, 1/2, 1]
//   depth=1: T=[0, 1/4, 2/4], T=[2/4, 3/4, 1]
//   depth=2: T=[0, 1/8, 2/8], T=[2/8, 3/8, 4/8], T=[4/8, 5/8, 6/8], T=[6/8, 7/8, 1]
//   ...
//
// The shader determines how many segments are required to render each individual curve smoothly,
// and emits empty triangles at any vertices whose sk_VertexIDs are higher than necessary. It is the
// caller's responsibility to draw enough vertices per instance for the most complex curve in the
// batch to render smoothly (i.e., NumTrianglesAtResolveLevel() * 3).
class MiddleOutShader : public GrPathTessellationShader {
public:
    MiddleOutShader(const GrShaderCaps& shaderCaps, const SkMatrix& viewMatrix,
                    const SkPMColor4f& color, PatchType patchType)
            : GrPathTessellationShader(kTessellate_MiddleOutShader_ClassID,
                                       GrPrimitiveType::kTriangles, 0, viewMatrix, color)
            , fPatchType(patchType) {
        fInstanceAttribs.emplace_back("p01", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        fInstanceAttribs.emplace_back("p23", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        if (fPatchType == PatchType::kWedges) {
            fInstanceAttribs.emplace_back("fanPoint", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
        }
        if (!shaderCaps.infinitySupport()) {
            // A conic curve is written out with p3=[w,Infinity], but GPUs that don't support
            // infinity can't detect this. On these platforms we also write out an extra float with
            // each patch that explicitly tells the shader what type of curve it is.
            fInstanceAttribs.emplace_back("curveType", kFloat_GrVertexAttribType, kFloat_GrSLType);
        }
        this->setInstanceAttributes(fInstanceAttribs.data(), fInstanceAttribs.count());
        SkASSERT(fInstanceAttribs.count() <= kMaxInstanceAttribCount);

        constexpr static Attribute kVertexAttrib("resolveLevel_and_idx", kFloat2_GrVertexAttribType,
                                                 kFloat2_GrSLType);
        this->setVertexAttributes(&kVertexAttrib, 1);
    }

private:
    const char* name() const final { return "tessellate_MiddleOutShader"; }
    void addToKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {
        b->add32((uint32_t)fPatchType);
    }
    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;
    const PatchType fPatchType;

    constexpr static int kMaxInstanceAttribCount = 4;
    SkSTArray<kMaxInstanceAttribCount, Attribute> fInstanceAttribs;
};

std::unique_ptr<GrGeometryProcessor::ProgramImpl> MiddleOutShader::makeProgramImpl(
        const GrShaderCaps&) const {
    class Impl : public GrPathTessellationShader::Impl {
        void emitVertexCode(const GrShaderCaps& shaderCaps, const GrPathTessellationShader& shader,
                            GrGLSLVertexBuilder* v, GrGPArgs* gpArgs) override {
            v->defineConstant("PRECISION", GrTessellationShader::kLinearizationPrecision);
            v->defineConstant("MAX_FIXED_RESOLVE_LEVEL", (float)kMaxFixedCountResolveLevel);
            v->defineConstant("MAX_FIXED_SEGMENTS", (float)kMaxFixedCountSegments);
            v->insertFunction(GrWangsFormula::as_sksl().c_str());
            if (shaderCaps.infinitySupport()) {
                v->insertFunction(R"(
                bool is_conic_curve() { return isinf(p23.w); }
                bool is_triangular_conic_curve() { return isinf(p23.z); })");
            } else {
                v->insertFunction(SkStringPrintf(R"(
                bool is_conic_curve() { return curveType != %g; })", kCubicCurveType).c_str());
                v->insertFunction(SkStringPrintf(R"(
                bool is_triangular_conic_curve() {
                    return curveType == %g;
                })", kTriangularConicCurveType).c_str());
            }
            if (shaderCaps.bitManipulationSupport()) {
                v->insertFunction(R"(
                float ldexp_portable(float x, float p) {
                    return ldexp(x, int(p));
                })");
            } else {
                v->insertFunction(R"(
                float ldexp_portable(float x, float p) {
                    return x * exp2(p);
                })");
            }
            v->codeAppend(R"(
            float resolveLevel = resolveLevel_and_idx.x;
            float idxInResolveLevel = resolveLevel_and_idx.y;
            float2 localcoord;)");
            if (shader.cast<MiddleOutShader>().fPatchType == PatchType::kWedges) {
                v->codeAppend(R"(
                // A negative resolve level means this is the fan point.
                if (resolveLevel < 0) {
                    localcoord = fanPoint;
                } else)");  // Fall through to next if ().
            }
            v->codeAppend(R"(
            if (is_triangular_conic_curve()) {
                // This patch is an exact triangle.
                localcoord = (resolveLevel != 0)      ? p01.zw
                           : (idxInResolveLevel != 0) ? p23.xy
                                                      : p01.xy;
            } else {
                float2 p0=p01.xy, p1=p01.zw, p2=p23.xy, p3=p23.zw;
                float w = -1;  // w < 0 tells us to treat the instance as an integral cubic.
                float maxResolveLevel;
                if (is_conic_curve()) {
                    // Conics are 3 points, with the weight in p3.
                    w = p3.x;
                    maxResolveLevel = wangs_formula_conic_log2(PRECISION, AFFINE_MATRIX * p0,
                                                                          AFFINE_MATRIX * p1,
                                                                          AFFINE_MATRIX * p2, w);
                    p1 *= w;  // Unproject p1.
                    p3 = p2;  // Duplicate the endpoint for shared code that also runs on cubics.
                } else {
                    // The patch is an integral cubic.
                    maxResolveLevel = wangs_formula_cubic_log2(PRECISION, p0, p1, p2, p3,
                                                               AFFINE_MATRIX);
                }
                if (resolveLevel > maxResolveLevel) {
                    // This vertex is at a higher resolve level than we need. Demote to a lower
                    // resolveLevel, which will produce a degenerate triangle.
                    idxInResolveLevel = floor(ldexp_portable(idxInResolveLevel,
                                                             maxResolveLevel - resolveLevel));
                    resolveLevel = maxResolveLevel;
                }
                // Promote our location to a discrete position in the maximum fixed resolve level.
                // This is extra paranoia to ensure we get the exact same fp32 coordinates for
                // colocated points from different resolve levels (e.g., the vertices T=3/4 and
                // T=6/8 should be exactly colocated).
                float fixedVertexID = floor(.5 + ldexp_portable(
                        idxInResolveLevel, MAX_FIXED_RESOLVE_LEVEL - resolveLevel));
                if (0 < fixedVertexID && fixedVertexID < MAX_FIXED_SEGMENTS) {
                    float T = fixedVertexID * (1 / MAX_FIXED_SEGMENTS);

                    // Evaluate at T. Use De Casteljau's for its accuracy and stability.
                    float2 ab = mix(p0, p1, T);
                    float2 bc = mix(p1, p2, T);
                    float2 cd = mix(p2, p3, T);
                    float2 abc = mix(ab, bc, T);
                    float2 bcd = mix(bc, cd, T);
                    float2 abcd = mix(abc, bcd, T);

                    // Evaluate the conic weight at T.
                    float u = mix(1.0, w, T);
                    float v = w + 1 - u;  // == mix(w, 1, T)
                    float uv = mix(u, v, T);

                    localcoord = (w < 0) ? /*cubic*/ abcd : /*conic*/ abc/uv;
                } else {
                    localcoord = (fixedVertexID == 0) ? p0.xy : p3.xy;
                }
            }
            float2 vertexpos = AFFINE_MATRIX * localcoord + TRANSLATE;)");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localcoord");
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");
        }
    };
    return std::make_unique<Impl>();
}

}  // namespace

GrPathTessellationShader* GrPathTessellationShader::MakeMiddleOutFixedCountShader(
        const GrShaderCaps& shaderCaps, SkArenaAlloc* arena, const SkMatrix& viewMatrix,
        const SkPMColor4f& color, PatchType patchType) {
    return arena->make<MiddleOutShader>(shaderCaps, viewMatrix, color, patchType);
}

void GrPathTessellationShader::InitializeVertexBufferForMiddleOutCurves(GrVertexWriter vertexWriter,
                                                                        size_t bufferSize) {
    SkASSERT(bufferSize >= kMiddleOutVertexStride * 2);
    SkASSERT(bufferSize % kMiddleOutVertexStride == 0);
    int vertexCount = bufferSize / kMiddleOutVertexStride;
    SkASSERT(vertexCount > 3);
    SkDEBUGCODE(GrVertexWriter end = vertexWriter.makeOffset(vertexCount * kMiddleOutVertexStride);)

    // Lay out the vertices in "middle-out" order:
    //
    // T= 0/1, 1/1,              ; resolveLevel=0
    //    1/2,                   ; resolveLevel=1  (0/2 and 2/2 are already in resolveLevel 0)
    //    1/4, 3/4,              ; resolveLevel=2  (2/4 is already in resolveLevel 1)
    //    1/8, 3/8, 5/8, 7/8,    ; resolveLevel=3  (2/8 and 6/8 are already in resolveLevel 2)
    //    ...                    ; resolveLevel=...
    //
    // Resolve level 0 is just the beginning and ending vertices.
    vertexWriter.write<float, float>(0/*resolveLevel*/, 0/*idx*/);
    vertexWriter.write<float, float>(0/*resolveLevel*/, 1/*idx*/);

    // Resolve levels 1..kMaxResolveLevel.
    int maxResolveLevel = SkPrevLog2(vertexCount - 1);
    SkASSERT((1 << maxResolveLevel) + 1 == vertexCount);
    for (int resolveLevel = 1; resolveLevel <= maxResolveLevel; ++resolveLevel) {
        int numSegmentsInResolveLevel = 1 << resolveLevel;
        // Write out the odd vertices in this resolveLevel. The even vertices were already written
        // out in previous resolveLevels and will be indexed from there.
        for (int i = 1; i < numSegmentsInResolveLevel; i += 2) {
            vertexWriter.write<float, float>(resolveLevel, i);
        }
    }

    SkASSERT(vertexWriter == end);
}

void GrPathTessellationShader::InitializeVertexBufferForMiddleOutWedges(GrVertexWriter vertexWriter,
                                                                        size_t bufferSize) {
    SkASSERT(bufferSize >= kMiddleOutVertexStride);
    // Start out with the fan point. A negative resolve level indicates the fan point.
    vertexWriter.write<float, float>(-1/*resolveLevel*/, -1/*idx*/);

    InitializeVertexBufferForMiddleOutCurves(std::move(vertexWriter),
                                             bufferSize - kMiddleOutVertexStride);
}

static void fill_index_buffer_for_curves(GrVertexWriter vertexWriter, size_t bufferSize,
                                         uint16_t baseIndex) {
    SkASSERT(bufferSize % (sizeof(uint16_t) * 3) == 0);
    int triangleCount = bufferSize / (sizeof(uint16_t) * 3);
    SkASSERT(triangleCount >= 1);
    SkTArray<std::array<uint16_t, 3>> indexData(triangleCount);

    // Connect the vertices with a middle-out triangulation. Refer to
    // InitializeVertexBufferForMiddleOutCurves() for the exact vertex ordering.
    //
    // Resolve level 1 is just a single triangle at T=[0, 1/2, 1].
    const auto* neighborInLastResolveLevel = &indexData.push_back({baseIndex,
                                                                   (uint16_t)(baseIndex + 2),
                                                                   (uint16_t)(baseIndex + 1)});

    // Resolve levels 2..maxResolveLevel
    int maxResolveLevel = SkPrevLog2(triangleCount + 1);
    uint16_t nextIndex = baseIndex + 3;
    SkASSERT(GrPathTessellationShader::NumCurveTrianglesAtResolveLevel(maxResolveLevel) ==
             triangleCount);
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

    vertexWriter.writeArray(indexData.data(), indexData.count());
}

void GrPathTessellationShader::InitializeIndexBufferForMiddleOutCurves(GrVertexWriter vertexWriter,
                                                                       size_t bufferSize) {
    fill_index_buffer_for_curves(std::move(vertexWriter), bufferSize, 0);
}

void GrPathTessellationShader::InitializeIndexBufferForMiddleOutWedges(GrVertexWriter vertexWriter,
                                                                       size_t bufferSize) {
    SkASSERT(bufferSize >= sizeof(uint16_t) * 3);
    // Start out with the fan triangle.
    vertexWriter.write<uint16_t, uint16_t, uint16_t>(0, 1, 2);

    fill_index_buffer_for_curves(std::move(vertexWriter), bufferSize - sizeof(uint16_t) * 3, 1);
}

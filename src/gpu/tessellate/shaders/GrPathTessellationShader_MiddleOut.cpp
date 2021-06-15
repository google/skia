/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

#include "src/gpu/geometry/GrWangsFormula.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/tessellate/GrPathTessellator.h"

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
    MiddleOutShader(const SkMatrix& viewMatrix, const SkPMColor4f& color, PatchType patchType)
            : GrPathTessellationShader(kTessellate_MiddleOutShader_ClassID,
                                       GrPrimitiveType::kTriangles, 0, viewMatrix, color)
            , fPatchType(patchType) {
        fAttribs.emplace_back("inputPoints_0_1", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        fAttribs.emplace_back("inputPoints_2_3", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        if (fPatchType == PatchType::kWedges) {
            fAttribs.emplace_back("fanPoint", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
        }
        this->setInstanceAttributes(fAttribs.data(), fAttribs.count());
        SkASSERT(fAttribs.count() <= kMaxAttribCount);
    }

private:
    const char* name() const final { return "tessellate_MiddleOutShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {
        b->add32((uint32_t)fPatchType);
    }
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const final;
    const PatchType fPatchType;

    constexpr static int kMaxAttribCount = 3;
    SkSTArray<kMaxAttribCount, Attribute> fAttribs;
};

GrGLSLGeometryProcessor* MiddleOutShader::createGLSLInstance(const GrShaderCaps&) const {
    class Impl : public GrPathTessellationShader::Impl {
        void emitVertexCode(const GrPathTessellationShader& shader, GrGLSLVertexBuilder* v,
                            GrGPArgs* gpArgs) override {
            v->defineConstant("PRECISION", GrPathTessellator::kLinearizationPrecision);
            v->insertFunction(GrWangsFormula::as_sksl().c_str());
            if (v->getProgramBuilder()->shaderCaps()->bitManipulationSupport()) {
                // Determines the T value at which to place the given vertex in a "middle-out"
                // topology.
                v->insertFunction(R"(
                float find_middle_out_T(int middleOutVertexID, float maxResolveLevel) {
                    int totalTriangleIdx = middleOutVertexID/3 + 1;
                    int resolveLevel = findMSB(totalTriangleIdx) + 1;
                    if (resolveLevel > int(maxResolveLevel)) {
                        // This vertex is at a higher resolve level than we need. Emit a degenerate
                        // triangle at T=1.
                        return 1;
                    }
                    int firstTriangleAtDepth = (1 << (resolveLevel - 1));
                    int triangleIdxWithinDepth = totalTriangleIdx - firstTriangleAtDepth;
                    int vertexIdxWithinDepth = triangleIdxWithinDepth * 2 + middleOutVertexID % 3;
                    return ldexp(float(vertexIdxWithinDepth), -resolveLevel);
                })");
            } else {
                // Determines the T value at which to place the given vertex in a "middle-out"
                // topology.
                v->insertFunction(R"(
                float find_middle_out_T(int middleOutVertexID, float maxResolveLevel) {
                    float totalTriangleIdx = float(middleOutVertexID/3) + 1;
                    float resolveLevel = floor(log2(totalTriangleIdx)) + 1;
                    if (resolveLevel > maxResolveLevel) {
                        // This vertex is at a higher resolve level than we need. Emit a degenerate
                        // triangle at T=1.
                        return 1;
                    }
                    float firstTriangleAtDepth = exp2(resolveLevel - 1);
                    float triangleIdxWithinDepth = totalTriangleIdx - firstTriangleAtDepth;
                    float vertexIdxWithinDepth =
                            triangleIdxWithinDepth * 2 + float(middleOutVertexID % 3);
                    return vertexIdxWithinDepth * exp2(-resolveLevel);
                })");
            }
            v->codeAppend(R"(
            int middleOutVertexID = sk_VertexID;
            float2 localcoord;)");
            if (shader.cast<MiddleOutShader>().fPatchType == PatchType::kWedges) {
                v->codeAppend(R"(
                // The first triangle is the fan triangle.
                middleOutVertexID -= 3;
                if (middleOutVertexID < 0) {
                    float2 endpoint = isinf(inputPoints_2_3.w) ? inputPoints_2_3.xy /*conic*/
                                                               : inputPoints_2_3.zw /*cubic*/;
                    localcoord = (sk_VertexID < 1)  ? inputPoints_0_1.xy
                               : (sk_VertexID == 1) ? endpoint
                                                    : fanPoint;
                } else)");  // Fall through to next if ().
            }
            v->codeAppend(R"(
            if (isinf(inputPoints_2_3.z)) {
                // A conic with w=Inf is an exact triangle.
                localcoord = (middleOutVertexID < 1)  ? inputPoints_0_1.xy
                           : (middleOutVertexID == 1) ? inputPoints_0_1.zw
                                                      : inputPoints_2_3.xy;
            } else {
                float w = -1;  // w < 0 tells us to treat the instance as an integral cubic.
                float4x2 P = float4x2(inputPoints_0_1, inputPoints_2_3);
                float maxResolveLevel;
                if (isinf(P[3].y)) {
                    // The patch is a conic.
                    w = P[3].x;
                    maxResolveLevel =
                            wangs_formula_conic_log2(PRECISION, AFFINE_MATRIX * float3x2(P), w);
                    P[3] = P[2];  // Duplicate the endpoint.
                    P[1] *= w;  // Unproject p1.
                } else {
                    // The patch is an integral cubic.
                    maxResolveLevel = wangs_formula_cubic_log2(PRECISION, P, AFFINE_MATRIX);
                }
                float T = find_middle_out_T(middleOutVertexID, maxResolveLevel);
                if (0 < T && T < 1) {
                    // Evaluate at T. Use De Casteljau's for its accuracy and stability.
                    float2 ab = mix(P[0], P[1], T);
                    float2 bc = mix(P[1], P[2], T);
                    float2 cd = mix(P[2], P[3], T);
                    float2 abc = mix(ab, bc, T);
                    float2 bcd = mix(bc, cd, T);
                    float2 abcd = mix(abc, bcd, T);

                    // Evaluate the conic weight at T.
                    float u = mix(1.0, w, T);
                    float v = w + 1 - u;  // == mix(w, 1, T)
                    float uv = mix(u, v, T);

                    localcoord = (w < 0) ? /*cubic*/ abcd : /*conic*/ abc/uv;
                } else {
                    localcoord = (T == 0) ? P[0].xy : P[3].xy;
                }
            }
            float2 vertexpos = AFFINE_MATRIX * localcoord + TRANSLATE;)");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localcoord");
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");
        }
    };
    return new Impl;
}

}  // namespace

GrPathTessellationShader* GrPathTessellationShader::MakeMiddleOutFixedCountShader(
        SkArenaAlloc* arena, const SkMatrix& viewMatrix, const SkPMColor4f& color,
        PatchType patchType) {
    return arena->make<MiddleOutShader>(viewMatrix, color, patchType);
}

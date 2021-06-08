/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
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
// The caller may compute each cubic's resolveLevel on the CPU (i.e., the log2 number of line
// segments it will be divided into; see GrWangsFormula::cubic_log2/quadratic_log2/conic_log2), and
// then sort the instance buffer by resolveLevel for efficient batching of indirect draws.
class MiddleOutShader : public GrPathTessellationShader {
public:
    MiddleOutShader(const SkMatrix& viewMatrix, const SkPMColor4f& color)
            : GrPathTessellationShader(kTessellate_MiddleOutShader_ClassID,
                                       GrPrimitiveType::kTriangles, 0, viewMatrix, color) {
        constexpr static Attribute kInputPtsAttribs[] = {
                {"inputPoints_0_1", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
                {"inputPoints_2_3", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};
        this->setInstanceAttributes(kInputPtsAttribs, 2);
    }

private:
    const char* name() const final { return "tessellate_MiddleOutShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const final {}
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const final;
};

GrGLSLGeometryProcessor* MiddleOutShader::createGLSLInstance(const GrShaderCaps&) const {
    class Impl : public GrPathTessellationShader::Impl {
        void emitVertexCode(GrGLSLVertexBuilder* v, GrGPArgs* gpArgs) override {
            if (v->getProgramBuilder()->shaderCaps()->bitManipulationSupport()) {
                // Determines the T value at which to place the given vertex in a "middle-out"
                // topology.
                v->insertFunction(R"(
                float find_middle_out_T() {
                    int totalTriangleIdx = sk_VertexID/3 + 1;
                    int resolveLevel = findMSB(totalTriangleIdx) + 1;
                    int firstTriangleAtDepth = (1 << (resolveLevel - 1));
                    int triangleIdxWithinDepth = totalTriangleIdx - firstTriangleAtDepth;
                    int vertexIdxWithinDepth = triangleIdxWithinDepth * 2 + sk_VertexID % 3;
                    return ldexp(float(vertexIdxWithinDepth), -resolveLevel);
                })");
            } else {
                // Determines the T value at which to place the given vertex in a "middle-out"
                // topology.
                v->insertFunction(R"(
                float find_middle_out_T() {
                    float totalTriangleIdx = float(sk_VertexID/3) + 1;
                    float resolveLevel = floor(log2(totalTriangleIdx)) + 1;
                    float firstTriangleAtDepth = exp2(resolveLevel - 1);
                    float triangleIdxWithinDepth = totalTriangleIdx - firstTriangleAtDepth;
                    float vertexIdxWithinDepth = triangleIdxWithinDepth*2 + float(sk_VertexID % 3);
                    return vertexIdxWithinDepth * exp2(-resolveLevel);
                })");
            }
            v->codeAppend(R"(
            float2 localcoord;
            if (isinf(inputPoints_2_3.z)) {
                // A conic with w=Inf is an exact triangle.
                localcoord = (sk_VertexID < 1)  ? inputPoints_0_1.xy
                           : (sk_VertexID == 1) ? inputPoints_0_1.zw
                                                : inputPoints_2_3.xy;
            } else {
                float w = -1;  // w < 0 tells us to treat the instance as an integral cubic.
                float4x2 P = float4x2(inputPoints_0_1, inputPoints_2_3);
                if (isinf(P[3].y)) {
                    // The patch is a conic.
                    w = P[3].x;
                    P[3] = P[2];  // Duplicate the endpoint.
                    P[1] *= w;  // Unproject p1.
                }
                float T = find_middle_out_T();
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

GrPathTessellationShader* GrPathTessellationShader::MakeMiddleOutInstancedShader(
        SkArenaAlloc* arena, const SkMatrix& viewMatrix, const SkPMColor4f& color) {
    return arena->make<MiddleOutShader>(viewMatrix, color);
}

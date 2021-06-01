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
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {}
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const final;
};

GrGLSLGeometryProcessor* MiddleOutShader::createGLSLInstance(const GrShaderCaps&) const {
    class Impl : public GrPathTessellationShader::Impl {
        void emitVertexCode(GrGLSLVertexBuilder* v, GrGPArgs* gpArgs) override {
            v->insertFunction(kUnpackRationalCubicFn);
            v->insertFunction(kEvalRationalCubicFn);
            if (v->getProgramBuilder()->shaderCaps()->bitManipulationSupport()) {
                // Determines the T value at which to place the given vertex in a "middle-out"
                // topology.
                v->insertFunction(R"(
                float find_middle_out_T() {
                    int totalTriangleIdx = sk_VertexID/3 + 1;
                    int depth = findMSB(totalTriangleIdx);
                    int firstTriangleAtDepth = (1 << depth);
                    int triangleIdxWithinDepth = totalTriangleIdx - firstTriangleAtDepth;
                    int vertexIdxWithinDepth = triangleIdxWithinDepth * 2 + sk_VertexID % 3;
                    return ldexp(float(vertexIdxWithinDepth), -1 - depth);
                })");
            } else {
                // Determines the T value at which to place the given vertex in a "middle-out"
                // topology.
                v->insertFunction(R"(
                float find_middle_out_T() {
                    float totalTriangleIdx = float(sk_VertexID/3) + 1;
                    float depth = floor(log2(totalTriangleIdx));
                    float firstTriangleAtDepth = exp2(depth);
                    float triangleIdxWithinDepth = totalTriangleIdx - firstTriangleAtDepth;
                    float vertexIdxWithinDepth = triangleIdxWithinDepth*2 + float(sk_VertexID % 3);
                    return vertexIdxWithinDepth * exp2(-1 - depth);
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
                float4x3 P = unpack_rational_cubic(inputPoints_0_1.xy, inputPoints_0_1.zw,
                                                   inputPoints_2_3.xy, inputPoints_2_3.zw);
                float T = find_middle_out_T();
                localcoord = eval_rational_cubic(P, T);
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

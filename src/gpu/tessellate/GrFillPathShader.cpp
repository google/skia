/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrFillPathShader.h"

#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

class GrFillPathShader::Impl : public GrGLSLGeometryProcessor {
public:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        auto& shader = args.fGP.cast<GrFillPathShader>();

        const char* viewMatrix;
        fViewMatrixUniform = args.fUniformHandler->addUniform(
                kVertex_GrShaderFlag, kFloat3x3_GrSLType, "view_matrix", &viewMatrix);

        args.fVaryingHandler->emitAttributes(shader);

        args.fVertBuilder->codeAppend("float2 localcoord, vertexpos;");
        shader.emitVertexCode(this, args.fVertBuilder, viewMatrix, args.fUniformHandler);

        this->emitTransforms(args.fVertBuilder, args.fVaryingHandler, args.fUniformHandler,
                             GrShaderVar("localcoord", kFloat2_GrSLType),
                             args.fFPCoordTransformHandler);

        gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");

        const char* color;
        fColorUniform = args.fUniformHandler->addUniform(
                kFragment_GrShaderFlag, kHalf4_GrSLType, "color", &color);

        args.fFragBuilder->codeAppendf("%s = %s;", args.fOutputColor, color);
        args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 const CoordTransformRange& transformRange) override {
        const GrFillPathShader& shader = primProc.cast<GrFillPathShader>();
        pdman.setSkMatrix(fViewMatrixUniform, shader.viewMatrix());

        const SkPMColor4f& color = shader.fColor;
        pdman.set4f(fColorUniform, color.fR, color.fG, color.fB, color.fA);

        if (fPathBoundsUniform.isValid()) {
            const SkRect& b = primProc.cast<GrFillBoundingBoxShader>().pathBounds();
            pdman.set4f(fPathBoundsUniform, b.left(), b.top(), b.right(), b.bottom());
        }

        this->setTransformDataHelper(SkMatrix::I(), pdman, transformRange);
    }

    GrGLSLUniformHandler::UniformHandle fViewMatrixUniform;
    GrGLSLUniformHandler::UniformHandle fColorUniform;
    GrGLSLUniformHandler::UniformHandle fPathBoundsUniform;
};

GrGLSLPrimitiveProcessor* GrFillPathShader::createGLSLInstance(const GrShaderCaps&) const {
    return new Impl;
}

void GrFillTriangleShader::emitVertexCode(Impl*, GrGLSLVertexBuilder* v, const char* viewMatrix,
                                          GrGLSLUniformHandler* uniformHandler) const {
    v->codeAppendf(R"(
            localcoord = input_point;
            vertexpos = (%s * float3(localcoord, 1)).xy;)", viewMatrix);
}

void GrFillCubicHullShader::emitVertexCode(Impl*, GrGLSLVertexBuilder* v, const char* viewMatrix,
                                           GrGLSLUniformHandler* uniformHandler) const {
    v->codeAppend(R"(
            float4x2 P = float4x2(input_points_0_1, input_points_2_3);

            // Translate the points to v0..3 where v0=0.
            float2 v1 = P[1] - P[0], v2 = P[2] - P[0], v3 = P[3] - P[0];

            // Reorder the points so v2 bisects v1 and v3.
            if (sign(determinant(float2x2(v2,v1))) == sign(determinant(float2x2(v2,v3)))) {
                float2 tmp = P[2];
                if (sign(determinant(float2x2(v1,v2))) != sign(determinant(float2x2(v1,v3)))) {
                    P[2] = P[1];  // swap(P2, P1)
                    P[1] = tmp;
                } else {
                    P[2] = P[3];  // swap(P2, P3)
                    P[3] = tmp;
                }
            }

            // Find the "turn direction" of each corner and net turn direction.
            float4 dir;
            float netdir = 0.0;
            for (int i = 0; i < 4; ++i) {
                float2 prev = P[i] - P[(i + 3) & 3], next = P[(i + 1) & 3] - P[i];
                dir[i] = sign(determinant(float2x2(prev, next)));
                netdir += dir[i];
            }

            // sk_VertexID comes in fan order. Convert to strip order.
            int vertexidx = sk_VertexID;
            vertexidx ^= vertexidx >> 1;

            // Remove the non-convex vertex, if any.
            if (dir[vertexidx] != sign(netdir)) {
                vertexidx = (vertexidx + 1) & 3;
            }

            localcoord = P[vertexidx];)");

    v->codeAppendf("vertexpos = (%s * float3(localcoord, 1)).xy;", viewMatrix);
}

void GrFillBoundingBoxShader::emitVertexCode(Impl* impl, GrGLSLVertexBuilder* v,
                                             const char* viewMatrix,
                                             GrGLSLUniformHandler* uniformHandler) const {
    const char* pathBounds;
    impl->fPathBoundsUniform = uniformHandler->addUniform(
            kVertex_GrShaderFlag, kFloat4_GrSLType, "path_bounds", &pathBounds);

    v->codeAppendf(R"(
            // Use sk_VertexID and uniforms (instead of vertex data) to find vertex positions.
            float2 T = float2(sk_VertexID & 1, sk_VertexID >> 1);
            localcoord = mix(%s.xy, %s.zw, T);
            vertexpos = (%s * float3(localcoord, 1)).xy;

            // Outset to avoid possible T-junctions with extreme edges of the path.
            float2x2 M2 = float2x2(%s);
            float2 devoutset = .25 * sign(M2 * (T - .5));
            localcoord += inverse(M2) * devoutset;
            vertexpos += devoutset;)", pathBounds, pathBounds, viewMatrix, viewMatrix);
}

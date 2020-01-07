/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrCoverShader.h"

#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

class GrCoverShader::Impl : public GrGLSLGeometryProcessor {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) final {
        const char* viewMatrix;
        fViewMatrixUniform = args.fUniformHandler->addUniform(
                kVertex_GrShaderFlag, kFloat3x3_GrSLType, "view_matrix", &viewMatrix);

        const char* pathBounds;
        fPathBoundsUniform = args.fUniformHandler->addUniform(
                kVertex_GrShaderFlag, kFloat4_GrSLType, "path_bounds", &pathBounds);

        const char* color;
        fColorUniform = args.fUniformHandler->addUniform(
                kFragment_GrShaderFlag, kHalf4_GrSLType, "color", &color);

        args.fVaryingHandler->emitAttributes(args.fGP.cast<GrCoverShader>());

        args.fVertBuilder->codeAppendf(R"(
                // Use sk_VertexID and uniforms (instead of vertex data) to find vertex positions.
                float2 T = float2(sk_VertexID & 1, sk_VertexID >> 1);
                float2 localcoord = mix(%s.xy, %s.zw, T);
                float2 vertexpos = (%s * float3(localcoord, 1)).xy;

                // Outset to avoid possible T-junctions with extreme edges of the path.
                float2x2 M2 = float2x2(%s);
                float2 devoutset = .25 * sign(M2 * (T - .5));
                vertexpos += devoutset;
                localcoord += inverse(M2) * devoutset;)",
                pathBounds, pathBounds, viewMatrix, viewMatrix);

        this->emitTransforms(args.fVertBuilder, args.fVaryingHandler, args.fUniformHandler,
                             GrShaderVar("localcoord", kFloat2_GrSLType),
                             args.fFPCoordTransformHandler);

        gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");

        args.fFragBuilder->codeAppendf(R"(
                %s = %s;
                %s = half4(1);)",
                args.fOutputColor, color, args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 const CoordTransformRange& transformRange) override {
        const GrCoverShader& shader = primProc.cast<GrCoverShader>();
        const SkRect& b = shader.fPathBounds;
        const SkPMColor4f& color = shader.fColor;
        pdman.setSkMatrix(fViewMatrixUniform, shader.fViewMatrix);
        pdman.set4f(fPathBoundsUniform, b.left(), b.top(), b.right(), b.bottom());
        pdman.set4f(fColorUniform, color.fR, color.fG, color.fB, color.fA);
        this->setTransformDataHelper(SkMatrix::I(), pdman, transformRange);
    }

    GrGLSLUniformHandler::UniformHandle fViewMatrixUniform;
    GrGLSLUniformHandler::UniformHandle fPathBoundsUniform;
    GrGLSLUniformHandler::UniformHandle fColorUniform;
};

GrGLSLPrimitiveProcessor* GrCoverShader::createGLSLInstance(const GrShaderCaps&) const {
    return new Impl;
}

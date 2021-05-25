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
        auto& shader = args.fGeomProc.cast<GrFillPathShader>();

        args.fVaryingHandler->emitAttributes(shader);

        const char* affineMatrix, *translate;
        fAffineMatrixUniform = args.fUniformHandler->addUniform(
                nullptr, kVertex_GrShaderFlag, kFloat4_GrSLType, "affineMatrix", &affineMatrix);
        fTranslateUniform = args.fUniformHandler->addUniform(
                nullptr, kVertex_GrShaderFlag, kFloat2_GrSLType, "translate", &translate);

        args.fVertBuilder->codeAppendf("float2x2 AFFINE_MATRIX = float2x2(%s);", affineMatrix);
        args.fVertBuilder->codeAppendf("float2 TRANSLATE = %s;", translate);
        args.fVertBuilder->codeAppend("float2 localcoord, vertexpos;");
        shader.emitVertexCode(this, args.fVertBuilder, args.fUniformHandler);

        gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");
        gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localcoord");

        const char* color;
        fColorUniform = args.fUniformHandler->addUniform(
                nullptr, kFragment_GrShaderFlag, kHalf4_GrSLType, "color", &color);

        args.fFragBuilder->codeAppendf("half4 %s = %s;", args.fOutputColor, color);
        args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrShaderCaps&,
                 const GrGeometryProcessor& geomProc) override {
        const GrFillPathShader& shader = geomProc.cast<GrFillPathShader>();
        const SkMatrix& m = shader.viewMatrix();
        pdman.set4f(fAffineMatrixUniform, m.getScaleX(), m.getSkewY(), m.getSkewX(), m.getScaleY());
        pdman.set2f(fTranslateUniform, m.getTranslateX(), m.getTranslateY());

        const SkPMColor4f& color = shader.fColor;
        pdman.set4f(fColorUniform, color.fR, color.fG, color.fB, color.fA);
    }

    GrGLSLUniformHandler::UniformHandle fAffineMatrixUniform;
    GrGLSLUniformHandler::UniformHandle fTranslateUniform;
    GrGLSLUniformHandler::UniformHandle fColorUniform;
};

GrGLSLGeometryProcessor* GrFillPathShader::createGLSLInstance(const GrShaderCaps&) const {
    return new Impl;
}

void GrFillTriangleShader::emitVertexCode(Impl*, GrGLSLVertexBuilder* v,
                                          GrGLSLUniformHandler* uniformHandler) const {
    v->codeAppend(R"(
    localcoord = input_point;
    vertexpos = AFFINE_MATRIX * localcoord + TRANSLATE;)");
}

void GrFillCubicHullShader::emitVertexCode(Impl*, GrGLSLVertexBuilder* v,
                                           GrGLSLUniformHandler* uniformHandler) const {
    v->codeAppend(R"(
    float4x2 P = float4x2(input_points_0_1, input_points_2_3);
    if (isinf(P[3].y)) {  // Is the curve a conic?
        float w = P[3].x;
        if (isinf(w)) {
            // A conic with w=Inf is an exact triangle.
            P = float4x2(P[0], P[1], P[2], P[2]);
        } else {
            // Convert the control points to a trapeziodal hull that circumcscribes the conic.
            float2 p1w = P[1] * w;
            float T = .51;  // Bias outward a bit to ensure we cover the outermost samples.
            float2 c1 = mix(P[0], p1w, T);
            float2 c2 = mix(P[2], p1w, T);
            float iw = 1 / mix(1, w, T);
            P = float4x2(P[0], c1 * iw, c2 * iw, P[2]);
        }
    }

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

    // sk_VertexID comes in fan order. Convert to strip order.
    int vertexidx = sk_VertexID;
    vertexidx ^= vertexidx >> 1;

    // Find the "turn direction" of each corner and net turn direction.
    float vertexdir = 0;
    float netdir = 0;
    for (int i = 0; i < 4; ++i) {
        float2 prev = P[i] - P[(i + 3) & 3], next = P[(i + 1) & 3] - P[i];
        float dir = sign(determinant(float2x2(prev, next)));
        if (i == vertexidx) {
            vertexdir = dir;
        }
        netdir += dir;
    }

    // Remove the non-convex vertex, if any.
    if (vertexdir != sign(netdir)) {
        vertexidx = (vertexidx + 1) & 3;
    }

    localcoord = P[vertexidx];
    vertexpos = AFFINE_MATRIX * localcoord + TRANSLATE;)");
}

void GrFillBoundingBoxShader::emitVertexCode(Impl* impl, GrGLSLVertexBuilder* v,
                                             GrGLSLUniformHandler* uniformHandler) const {
    v->codeAppendf(R"(
    // Bloat the bounding box by 1/4px to avoid potential T-junctions at the edges.
    float2x2 M_ = inverse(AFFINE_MATRIX);
    float2 bloat = float2(abs(M_[0]) + abs(M_[1])) * .25;

    // Find the vertex position.
    float2 T = float2(sk_VertexID & 1, sk_VertexID >> 1);
    localcoord = mix(pathBounds.xy - bloat, pathBounds.zw + bloat, T);
    vertexpos = AFFINE_MATRIX * localcoord + TRANSLATE;)");
}

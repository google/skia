/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStencilPathShader.h"

#include "src/gpu/geometry/GrWangsFormula.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

constexpr static char kSkSLTypeDefs[] = R"(
#define float4x3 mat4x3
#define float2 vec2
#define float3 vec3
#define float4 vec4
)";

// Converts a 4-point input patch into the rational cubic it intended to represent.
constexpr static char kUnpackRationalCubicFn[] = R"(
float4x3 unpack_rational_cubic(float2 p0, float2 p1, float2 p2, float2 p3) {
    float4x3 P = float4x3(p0,1, p1,1, p2,1, p3,1);
    if (isinf(P[3].y)) {
        // This patch is actually a conic. Convert to a rational cubic.
        float w = P[3].x;
        float3 c = P[1] * ((2.0/3.0) * w);
        P = float4x3(P[0], fma(P[0], float3(1.0/3.0), c), fma(P[2], float3(1.0/3.0), c), P[2]);
    }
    return P;
})";

// Evaluate our point of interest using numerically stable linear interpolations. We add our own
// "safe_mix" method to guarantee we get exactly "b" when T=1. The builtin mix() function seems
// spec'd to behave this way, but empirical results results have shown it does not always.
constexpr static char kEvalRationalCubicFn[] = R"(
float3 safe_mix(float3 a, float3 b, float T, float one_minus_T) {
    return a*one_minus_T + b*T;
}
float2 eval_rational_cubic(float4x3 P, float T) {
    float one_minus_T = 1.0 - T;
    float3 ab = safe_mix(P[0], P[1], T, one_minus_T);
    float3 bc = safe_mix(P[1], P[2], T, one_minus_T);
    float3 cd = safe_mix(P[2], P[3], T, one_minus_T);
    float3 abc = safe_mix(ab, bc, T, one_minus_T);
    float3 bcd = safe_mix(bc, cd, T, one_minus_T);
    float3 abcd = safe_mix(abc, bcd, T, one_minus_T);
    return abcd.xy / abcd.z;
})";

class GrStencilPathShader::Impl : public GrGLSLGeometryProcessor {
protected:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& shader = args.fGeomProc.cast<GrStencilPathShader>();
        args.fVaryingHandler->emitAttributes(shader);
        auto v = args.fVertBuilder;

        GrShaderVar vertexPos = (*shader.vertexAttributes().begin()).asShaderVar();
        if (!shader.viewMatrix().isIdentity()) {
            const char* viewMatrix;
            fViewMatrixUniform = args.fUniformHandler->addUniform(
                    nullptr, kVertex_GrShaderFlag, kFloat3x3_GrSLType, "view_matrix", &viewMatrix);
            v->codeAppendf("float2 vertexpos = (%s * float3(inputPoint, 1)).xy;", viewMatrix);
            if (shader.willUseTessellationShaders()) {
                // If y is infinity then x is a conic weight. Don't transform.
                v->codeAppendf("vertexpos = (isinf(inputPoint.y)) ? inputPoint : vertexpos;");
            }
            vertexPos.set(kFloat2_GrSLType, "vertexpos");
        }

        if (!shader.willUseTessellationShaders()) {  // This is the case for the triangle shader.
            gpArgs->fPositionVar = vertexPos;
        } else {
            v->declareGlobal(GrShaderVar("P", kFloat2_GrSLType, GrShaderVar::TypeModifier::Out));
            v->codeAppendf("P = %s;", vertexPos.c_str());
        }

        // The fragment shader is normally disabled, but output fully opaque white.
        args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputColor);
        args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrShaderCaps&,
                 const GrGeometryProcessor& geomProc) override {
        const auto& shader = geomProc.cast<GrStencilPathShader>();
        if (!shader.viewMatrix().isIdentity()) {
            pdman.setSkMatrix(fViewMatrixUniform, shader.viewMatrix());
        }
    }

    GrGLSLUniformHandler::UniformHandle fViewMatrixUniform;
};

GrGLSLGeometryProcessor* GrStencilPathShader::createGLSLInstance(const GrShaderCaps&) const {
    return new Impl;
}

GrGLSLGeometryProcessor* GrCurveTessellateShader::createGLSLInstance(const GrShaderCaps&) const {
    class Impl : public GrStencilPathShader::Impl {
        SkString getTessControlShaderGLSL(const GrGeometryProcessor&,
                                          const char* versionAndExtensionDecls,
                                          const GrGLSLUniformHandler&,
                                          const GrShaderCaps&) const override {
            SkString code(versionAndExtensionDecls);
            code.appendf(R"(
            #define PRECISION %f)", GrTessellationPathRenderer::kLinearizationPrecision);
            code.append(kSkSLTypeDefs);
            code.append(GrWangsFormula::as_sksl(true/*hasConics*/));
            code.append(kUnpackRationalCubicFn);
            code.append(R"(
            layout(vertices = 1) out;

            in vec2 P[];
            patch out mat4x2 rationalCubicXY;
            patch out float rationalCubicW;

            void main() {
                float w = -1;  // w<0 means a cubic.
                vec2 p1w = P[1];
                if (isinf(P[3].y)) {
                    // This patch is actually a conic. Project to homogeneous space.
                    w = P[3].x;
                    p1w *= w;
                }

                // Chop the curve at T=1/2.
                vec2 ab = (P[0] + p1w) * .5;
                vec2 bc = (p1w + P[2]) * .5;
                vec2 cd = (P[2] + P[3]) * .5;
                vec2 abc = (ab + bc) * .5;
                vec2 bcd = (bc + cd) * .5;
                vec2 abcd = (abc + bcd) * .5;

                float n0, n1;
                if (w < 0 || isinf(w)) {
                    if (w < 0) {
                        // The patch is a cubic. Calculate how many segments are required to
                        // linearize each half of the curve.
                        n0 = wangs_formula(PRECISION, P[0], ab, abc, abcd, -1);  // w<0 means cubic.
                        n1 = wangs_formula(PRECISION, abcd, bcd, cd, P[3], -1);
                        rationalCubicW = 1;
                    } else {
                        // The patch is a triangle (a conic with infinite weight).
                        n0 = n1 = 1;
                        rationalCubicW = -1;  // In the next stage, rationalCubicW<0 means triangle.
                    }
                    rationalCubicXY = mat4x2(P[0], P[1], P[2], P[3]);
                } else {
                    // The patch is a conic. Unproject p0..5. w1 == w2 == w3 when chopping at .5.
                    // (See SkConic::chopAt().)
                    float r = 2.0 / (1.0 + w);
                    ab *= r, bc *= r, abc *= r;
                    // Put in "standard form" where w0 == w2 == w4 == 1.
                    float w_ = inversesqrt(r);  // Both halves have the same w' when chopping at .5.
                    // Calculate how many segments are needed to linearize each half of the curve.
                    n0 = wangs_formula(PRECISION, P[0], ab, abc, float2(0), w_);
                    n1 = wangs_formula(PRECISION, abc, bc, P[2], float2(0), w_);
                    // Covert the conic to a rational cubic in projected form.
                    rationalCubicXY = mat4x2(P[0],
                                             mix(float4(P[0],P[2]), p1w.xyxy, 2.0/3.0),
                                             P[2]);
                    rationalCubicW = fma(w, 2.0/3.0, 1.0/3.0);
                }

                gl_TessLevelOuter[0] = n1;
                gl_TessLevelOuter[1] = 1.0;
                gl_TessLevelOuter[2] = n0;

                // Changing the inner level to 1 when n0 == n1 == 1 collapses the entire patch to a
                // single triangle. Otherwise, we need an inner level of 2 so our curve triangles
                // have an interior point to originate from.
                gl_TessLevelInner[0] = min(max(n0, n1), 2.0);
            })");

            return code;
        }

        SkString getTessEvaluationShaderGLSL(const GrGeometryProcessor&,
                                             const char* versionAndExtensionDecls,
                                             const GrGLSLUniformHandler&,
                                             const GrShaderCaps&) const override {
            SkString code(versionAndExtensionDecls);
            code.append(kSkSLTypeDefs);
            code.append(kEvalRationalCubicFn);
            code.append(R"(
            layout(triangles, equal_spacing, ccw) in;

            uniform vec4 sk_RTAdjust;

            patch in mat4x2 rationalCubicXY;
            patch in float rationalCubicW;

            void main() {
                vec2 vertexpos;
                if (rationalCubicW < 0) {  // rationalCubicW < 0 means a triangle now.
                    vertexpos = (gl_TessCoord.x != 0) ? rationalCubicXY[0]
                              : (gl_TessCoord.y != 0) ? rationalCubicXY[1]
                                                      : rationalCubicXY[2];
                } else {
                    // Locate our parametric point of interest. T ramps from [0..1/2] on the left
                    // edge of the triangle, and [1/2..1] on the right. If we are the patch's
                    // interior vertex, then we want T=1/2. Since the barycentric coords are
                    // (1/3, 1/3, 1/3) at the interior vertex, the below fma() works in all 3
                    // scenarios.
                    float T = fma(.5, gl_TessCoord.y, gl_TessCoord.z);

                    mat4x3 P = mat4x3(rationalCubicXY[0], 1,
                                      rationalCubicXY[1], rationalCubicW,
                                      rationalCubicXY[2], rationalCubicW,
                                      rationalCubicXY[3], 1);
                    vertexpos = eval_rational_cubic(P, T);
                    if (all(notEqual(gl_TessCoord.xz, vec2(0)))) {
                        // We are the interior point of the patch; center it inside
                        // [C(0), C(.5), C(1)].
                        vertexpos = (P[0].xy + vertexpos + P[3].xy) / 3.0;
                    }
                }

                gl_Position = vec4(vertexpos * sk_RTAdjust.xz + sk_RTAdjust.yw, 0.0, 1.0);
            })");

            return code;
        }
    };

    return new Impl;
}

GrGLSLGeometryProcessor* GrWedgeTessellateShader::createGLSLInstance(const GrShaderCaps&) const {
    class Impl : public GrStencilPathShader::Impl {
        SkString getTessControlShaderGLSL(const GrGeometryProcessor&,
                                          const char* versionAndExtensionDecls,
                                          const GrGLSLUniformHandler&,
                                          const GrShaderCaps&) const override {
            SkString code(versionAndExtensionDecls);
            code.appendf(R"(
            #define PRECISION %f)", GrTessellationPathRenderer::kLinearizationPrecision);
            code.append(kSkSLTypeDefs);
            code.append(GrWangsFormula::as_sksl(true/*hasConics*/));
            code.append(kUnpackRationalCubicFn);
            code.append(R"(
            layout(vertices = 1) out;

            in vec2 P[];
            patch out mat4x2 rationalCubicXY;
            patch out float rationalCubicW;
            patch out vec2 fanpoint;

            void main() {
                // Figure out how many segments to divide the curve into.
                float w = isinf(P[3].y) ? P[3].x : -1;  // w<0 means cubic.
                float n = wangs_formula(PRECISION, P[0], P[1], P[2], P[3], w);

                // Tessellate the first side of the patch into n triangles.
                gl_TessLevelOuter[0] = n;

                // Leave the other two sides of the patch as single segments.
                gl_TessLevelOuter[1] = 1.0;
                gl_TessLevelOuter[2] = 1.0;

                // Changing the inner level to 1 when n == 1 collapses the entire
                // patch to a single triangle. Otherwise, we need an inner level of 2 so our curve
                // triangles have an interior point to originate from.
                gl_TessLevelInner[0] = min(n, 2.0);

                if (w < 0) {
                    rationalCubicXY = mat4x2(P[0], P[1], P[2], P[3]);
                    rationalCubicW = 1;
                } else {
                    // Convert the conic to a rational cubic in projected form.
                    rationalCubicXY = mat4x2(P[0],
                                             mix(vec4(P[0], P[2]), (P[1] * w).xyxy, 2.0/3.0),
                                             P[2]);
                    rationalCubicW = fma(w, 2.0/3.0, 1.0/3.0);
                }
                fanpoint = P[4];
            })");

            return code;
        }

        SkString getTessEvaluationShaderGLSL(const GrGeometryProcessor&,
                                             const char* versionAndExtensionDecls,
                                             const GrGLSLUniformHandler&,
                                             const GrShaderCaps&) const override {
            SkString code(versionAndExtensionDecls);
            code.append(kSkSLTypeDefs);
            code.append(kEvalRationalCubicFn);
            code.append(R"(
            layout(triangles, equal_spacing, ccw) in;

            uniform vec4 sk_RTAdjust;

            patch in mat4x2 rationalCubicXY;
            patch in float rationalCubicW;
            patch in vec2 fanpoint[];

            void main() {
                // Locate our parametric point of interest. It is equal to the barycentric
                // y-coordinate if we are a vertex on the tessellated edge of the triangle patch,
                // 0.5 if we are the patch's interior vertex, or N/A if we are the fan point.
                // NOTE: We are on the tessellated edge when the barycentric x-coordinate == 0.
                float T = (gl_TessCoord.x == 0.0) ? gl_TessCoord.y : 0.5;

                mat4x3 P = mat4x3(rationalCubicXY[0], 1,
                                  rationalCubicXY[1], rationalCubicW,
                                  rationalCubicXY[2], rationalCubicW,
                                  rationalCubicXY[3], 1);
                vec2 vertexpos = eval_rational_cubic(P, T);

                if (gl_TessCoord.x == 1.0) {
                    // We are the anchor point that fans from the center of the curve's contour.
                    vertexpos = fanpoint[0];
                } else if (gl_TessCoord.x != 0.0) {
                    // We are the interior point of the patch; center it inside [C(0), C(.5), C(1)].
                    vertexpos = (P[0].xy + vertexpos + P[3].xy) / 3.0;
                }

                gl_Position = vec4(vertexpos * sk_RTAdjust.xz + sk_RTAdjust.yw, 0.0, 1.0);
            })");

            return code;
        }
    };

    return new Impl;
}

class GrCurveMiddleOutShader::Impl : public GrStencilPathShader::Impl {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& shader = args.fGeomProc.cast<GrCurveMiddleOutShader>();
        args.fVaryingHandler->emitAttributes(shader);
        args.fVertBuilder->insertFunction(kUnpackRationalCubicFn);
        args.fVertBuilder->insertFunction(kEvalRationalCubicFn);
        if (args.fShaderCaps->bitManipulationSupport()) {
            // Determines the T value at which to place the given vertex in a "middle-out" topology.
            args.fVertBuilder->insertFunction(R"(
            float find_middle_out_T() {
                int totalTriangleIdx = sk_VertexID/3 + 1;
                int depth = findMSB(totalTriangleIdx);
                int firstTriangleAtDepth = (1 << depth);
                int triangleIdxWithinDepth = totalTriangleIdx - firstTriangleAtDepth;
                int vertexIdxWithinDepth = triangleIdxWithinDepth * 2 + sk_VertexID % 3;
                return ldexp(float(vertexIdxWithinDepth), -1 - depth);
            })");
        } else {
            // Determines the T value at which to place the given vertex in a "middle-out" topology.
            args.fVertBuilder->insertFunction(R"(
            float find_middle_out_T() {
                float totalTriangleIdx = float(sk_VertexID/3) + 1;
                float depth = floor(log2(totalTriangleIdx));
                float firstTriangleAtDepth = exp2(depth);
                float triangleIdxWithinDepth = totalTriangleIdx - firstTriangleAtDepth;
                float vertexIdxWithinDepth = triangleIdxWithinDepth * 2 + float(sk_VertexID % 3);
                return vertexIdxWithinDepth * exp2(-1 - depth);
            })");
        }
        args.fVertBuilder->codeAppend(R"(
        float2 pos;
        if (isinf(inputPoints_2_3.z)) {
            // A conic with w=Inf is an exact triangle.
            pos = (sk_VertexID < 1)  ? inputPoints_0_1.xy
                : (sk_VertexID == 1) ? inputPoints_0_1.zw
                                     : inputPoints_2_3.xy;
        } else {
            float4x3 P = unpack_rational_cubic(inputPoints_0_1.xy, inputPoints_0_1.zw,
                                               inputPoints_2_3.xy, inputPoints_2_3.zw);
            float T = find_middle_out_T();
            pos = eval_rational_cubic(P, T);
        })");
        if (!shader.viewMatrix().isIdentity()) {
            const char* viewMatrix;
            fViewMatrixUniform = args.fUniformHandler->addUniform(
                    nullptr, kVertex_GrShaderFlag, kFloat3x3_GrSLType, "view_matrix", &viewMatrix);
            args.fVertBuilder->codeAppendf(R"(
            pos = (%s * float3(pos, 1)).xy;)", viewMatrix);
        }
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "pos");

        // The fragment shader is normally disabled, but output fully opaque white.
        args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputColor);
        args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
    }
};

GrGLSLGeometryProcessor* GrCurveMiddleOutShader::createGLSLInstance(const GrShaderCaps&) const {
    return new Impl;
}

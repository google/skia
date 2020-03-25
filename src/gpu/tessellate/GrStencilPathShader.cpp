/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStencilPathShader.h"

#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

// Wang's formula for cubics (1985) gives us the number of evenly spaced (in the
// parametric sense) line segments that are guaranteed to be within a distance of
// "MAX_LINEARIZATION_ERROR" from the actual curve.
constexpr char kWangsFormulaCubicFn[] = R"(
        #define MAX_LINEARIZATION_ERROR 0.25  // 1/4 pixel
        float wangs_formula_cubic(vec2 p0, vec2 p1, vec2 p2, vec2 p3) {
            float k = (3.0 * 2.0) / (8.0 * MAX_LINEARIZATION_ERROR);
            float f = sqrt(k * length(max(abs(p2 - p1*2.0 + p0),
                                          abs(p3 - p2*2.0 + p1))));
            return max(1.0, ceil(f));
        })";

// Evaluate our point of interest using numerically stable mix() operations.
constexpr char kEvalCubicFn[] = R"(
        vec2 eval_cubic(mat4x2 P, float T) {
            vec2 ab = mix(P[0], P[1], T);
            vec2 bc = mix(P[1], P[2], T);
            vec2 cd = mix(P[2], P[3], T);
            vec2 abc = mix(ab, bc, T);
            vec2 bcd = mix(bc, cd, T);
            return mix(abc, bcd, T);
        })";

class GrStencilPathShader::Impl : public GrGLSLGeometryProcessor {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& shader = args.fGP.cast<GrStencilPathShader>();
        args.fVaryingHandler->emitAttributes(shader);

        GrShaderVar vertexPos = (*shader.vertexAttributes().begin()).asShaderVar();
        if (!shader.viewMatrix().isIdentity()) {
            const char* viewMatrix;
            fViewMatrixUniform = args.fUniformHandler->addUniform(
                    kVertex_GrShaderFlag, kFloat3x3_GrSLType, "view_matrix", &viewMatrix);
            args.fVertBuilder->codeAppendf(
                    "float2 vertexpos = (%s * float3(point, 1)).xy;", viewMatrix);
            vertexPos.set(kFloat2_GrSLType, "vertexpos");
        }

        if (!shader.willUseTessellationShaders()) {
            gpArgs->fPositionVar = vertexPos;
        } else {
            args.fVertBuilder->declareGlobal(GrShaderVar(
                    "P", kFloat2_GrSLType, GrShaderVar::TypeModifier::Out));
            args.fVertBuilder->codeAppendf("P = %s;", vertexPos.c_str());
        }

        // No fragment shader.
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 const CoordTransformRange& transformRange) override {
        const auto& shader = primProc.cast<GrStencilPathShader>();
        if (!shader.viewMatrix().isIdentity()) {
            pdman.setSkMatrix(fViewMatrixUniform, shader.viewMatrix());
        }
    }

    GrGLSLUniformHandler::UniformHandle fViewMatrixUniform;
};

GrGLSLPrimitiveProcessor* GrStencilPathShader::createGLSLInstance(const GrShaderCaps&) const {
    return new Impl;
}

SkString GrStencilCubicShader::getTessControlShaderGLSL(const char* versionAndExtensionDecls,
                                                        const GrShaderCaps&) const {
    SkString code(versionAndExtensionDecls);
    code.append(kWangsFormulaCubicFn);
    code.append(R"(
            layout(vertices = 1) out;

            in vec2 P[];
            out vec4 X[];
            out vec4 Y[];

            void main() {
                // Chop the curve at T=1/2.
                vec2 ab = mix(P[0], P[1], .5);
                vec2 bc = mix(P[1], P[2], .5);
                vec2 cd = mix(P[2], P[3], .5);
                vec2 abc = mix(ab, bc, .5);
                vec2 bcd = mix(bc, cd, .5);
                vec2 abcd = mix(abc, bcd, .5);

                // Calculate how many triangles we need to linearize each half of the curve.
                float l0 = wangs_formula_cubic(P[0], ab, abc, abcd);
                float l1 = wangs_formula_cubic(abcd, bcd, cd, P[3]);

                gl_TessLevelOuter[0] = l1;
                gl_TessLevelOuter[1] = 1.0;
                gl_TessLevelOuter[2] = l0;

                // Changing the inner level to 1 when l0 == l1 == 1 collapses the entire patch to a
                // single triangle. Otherwise, we need an inner level of 2 so our curve triangles
                // have an interior point to originate from.
                gl_TessLevelInner[0] = min(max(l0, l1), 2.0);

                X[gl_InvocationID /*== 0*/] = vec4(P[0].x, P[1].x, P[2].x, P[3].x);
                Y[gl_InvocationID /*== 0*/] = vec4(P[0].y, P[1].y, P[2].y, P[3].y);
            })");

    return code;
}

SkString GrStencilCubicShader::getTessEvaluationShaderGLSL(const char* versionAndExtensionDecls,
                                                           const GrShaderCaps&) const {
    SkString code(versionAndExtensionDecls);
    code.append(kEvalCubicFn);
    code.append(R"(
            layout(triangles, equal_spacing, ccw) in;

            uniform vec4 sk_RTAdjust;

            in vec4 X[];
            in vec4 Y[];

            void main() {
                // Locate our parametric point of interest. T ramps from [0..1/2] on the left edge
                // of the triangle, and [1/2..1] on the right. If we are the patch's interior
                // vertex, then we want T=1/2. Since the barycentric coords are (1/3, 1/3, 1/3) at
                // the interior vertex, the below fma() works in all 3 scenarios.
                float T = fma(.5, gl_TessCoord.y, gl_TessCoord.z);

                mat4x2 P = transpose(mat2x4(X[0], Y[0]));
                vec2 vertexpos = eval_cubic(P, T);
                if (all(notEqual(gl_TessCoord.xz, vec2(0)))) {
                    // We are the interior point of the patch; center it inside [C(0), C(.5), C(1)].
                    vertexpos = (P[0] + vertexpos + P[3]) / 3.0;
                }

                gl_Position = vec4(vertexpos * sk_RTAdjust.xz + sk_RTAdjust.yw, 0.0, 1.0);
            })");

    return code;
}

SkString GrStencilWedgeShader::getTessControlShaderGLSL(const char* versionAndExtensionDecls,
                                                        const GrShaderCaps&) const {
    SkString code(versionAndExtensionDecls);
    code.append(kWangsFormulaCubicFn);
    code.append(R"(
            layout(vertices = 1) out;

            in vec2 P[];
            out vec4 X[];
            out vec4 Y[];
            out vec2 fanpoint[];

            void main() {
                // Calculate how many triangles we need to linearize the curve.
                float num_segments = wangs_formula_cubic(P[0], P[1], P[2], P[3]);

                // Tessellate the first side of the patch into num_segments triangles.
                gl_TessLevelOuter[0] = num_segments;

                // Leave the other two sides of the patch as single segments.
                gl_TessLevelOuter[1] = 1.0;
                gl_TessLevelOuter[2] = 1.0;

                // Changing the inner level to 1 when num_segments == 1 collapses the entire
                // patch to a single triangle. Otherwise, we need an inner level of 2 so our curve
                // triangles have an interior point to originate from.
                gl_TessLevelInner[0] = min(num_segments, 2.0);

                X[gl_InvocationID /*== 0*/] = vec4(P[0].x, P[1].x, P[2].x, P[3].x);
                Y[gl_InvocationID /*== 0*/] = vec4(P[0].y, P[1].y, P[2].y, P[3].y);
                fanpoint[gl_InvocationID /*== 0*/] = P[4];
            })");

    return code;
}

SkString GrStencilWedgeShader::getTessEvaluationShaderGLSL(const char* versionAndExtensionDecls,
                                                           const GrShaderCaps&) const {
    SkString code(versionAndExtensionDecls);
    code.append(kEvalCubicFn);
    code.append(R"(
            layout(triangles, equal_spacing, ccw) in;

            uniform vec4 sk_RTAdjust;

            in vec4 X[];
            in vec4 Y[];
            in vec2 fanpoint[];

            void main() {
                // Locate our parametric point of interest. It is equal to the barycentric
                // y-coordinate if we are a vertex on the tessellated edge of the triangle patch,
                // 0.5 if we are the patch's interior vertex, or N/A if we are the fan point.
                // NOTE: We are on the tessellated edge when the barycentric x-coordinate == 0.
                float T = (gl_TessCoord.x == 0.0) ? gl_TessCoord.y : 0.5;

                mat4x2 P = transpose(mat2x4(X[0], Y[0]));
                vec2 vertexpos = eval_cubic(P, T);
                if (gl_TessCoord.x == 1.0) {
                    // We are the anchor point that fans from the center of the curve's contour.
                    vertexpos = fanpoint[0];
                } else if (gl_TessCoord.x != 0.0) {
                    // We are the interior point of the patch; center it inside [C(0), C(.5), C(1)].
                    vertexpos = (P[0] + vertexpos + P[3]) / 3.0;
                }

                gl_Position = vec4(vertexpos * sk_RTAdjust.xz + sk_RTAdjust.yw, 0.0, 1.0);
            })");

    return code;
}

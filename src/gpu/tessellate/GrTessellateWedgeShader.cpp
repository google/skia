/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrTessellateWedgeShader.h"

#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

class GrTessellateWedgeShader::Impl : public GrGLSLGeometryProcessor {
    void onEmitCode(EmitArgs&, GrGPArgs*) override;
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 const CoordTransformRange& transformRange) override;

    GrGLSLUniformHandler::UniformHandle fViewMatrixUniform;
};

void GrTessellateWedgeShader::Impl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const char* viewMatrix;
    fViewMatrixUniform = args.fUniformHandler->addUniform(
            kVertex_GrShaderFlag, kFloat3x3_GrSLType, "view_matrix", &viewMatrix);

    args.fVaryingHandler->emitAttributes(args.fGP.cast<GrTessellateWedgeShader>());

    args.fVertBuilder->declareGlobal(GrShaderVar(
            "P_", kFloat2_GrSLType, GrShaderVar::kOut_TypeModifier));
    args.fVertBuilder->codeAppendf("P_ = (%s * float3(P, 1)).xy;", viewMatrix);

    // No fragment shader.
};

SkString GrTessellateWedgeShader::getTessControlShaderGLSL(const char* versionAndExtensionDecls,
                                                           const GrShaderCaps&) const {
    SkString code(versionAndExtensionDecls);
    code.append(R"(
            layout(vertices = 1) out;

            in vec2 P_[];

            out mat4x2 P[];
            out vec2 fanpoint[];

            // Wang's formula for cubics (1985) gives us the number of evenly spaced (in the
            // parametric sense) line segments that are guaranteed to be within a distance of
            // "MAX_LINEARIZATION_ERROR" from the actual curve.
            #define MAX_LINEARIZATION_ERROR 0.25  // 1/4 pixel
            float wangs_formula_cubic(vec2 p0, vec2 p1, vec2 p2, vec2 p3) {
                float k = (3.0 * 2.0) / (8.0 * MAX_LINEARIZATION_ERROR);
                float f = sqrt(k * length(max(abs(p2 - p1*2.0 + p0),
                                              abs(p3 - p2*2.0 + p1))));
                return max(1.0, ceil(f));
            }

            void main() {
                // Calculate how many triangles we need to linearize the curve.
                float num_segments = wangs_formula_cubic(P_[0], P_[1], P_[2], P_[3]);

                // Tessellate the first side of the patch into num_segments triangles.
                gl_TessLevelOuter[0] = num_segments;

                // Leave the other two sides of the patch as single segments.
                gl_TessLevelOuter[1] = 1.0;
                gl_TessLevelOuter[2] = 1.0;

                // Changing the inner level to 1 when num_segments == 1 collapses the entire
                // patch to a single triangle. Otherwise, we need an inner level of 2 so our curve
                // triangles have an interior point to originate from.
                gl_TessLevelInner[0] = min(num_segments, 2.0);

                P[gl_InvocationID /*== 0*/] = mat4x2(P_[0], P_[1], P_[2], P_[3]);
                fanpoint[gl_InvocationID /*== 0*/] = P_[4];
            })");

    return code;
}


SkString GrTessellateWedgeShader::getTessEvaluationShaderGLSL(const char* versionAndExtensionDecls,
                                                              const GrShaderCaps&) const {
    SkString code(versionAndExtensionDecls);
    code.append(R"(
            layout(triangles, equal_spacing, cw) in;

            uniform vec4 sk_RTAdjust;

            in mat4x2 P[];
            in vec2 fanpoint[];

            void main() {
                vec2 p0 = P[0][0], p1 = P[0][1], p2 = P[0][2], p3 = P[0][3];

                // Locate our parametric point of interest. It is equal to the barycentric
                // y-coordinate if we are a vertex on the tessellated edge of the triangle patch,
                // 0.5 if we are the patch's interior vertex, or N/A if we are the fan point.
                // NOTE: We are on the tessellated edge when the barycentric x-coordinate == 0.
                float T = (gl_TessCoord.x == 0.0) ? gl_TessCoord.y : 0.5;

                // Evaluate our point of interest using numerically stable mix() operations.
                vec2 ab = mix(p0, p1, T);
                vec2 bc = mix(p1, p2, T);
                vec2 cd = mix(p2, p3, T);
                vec2 abc = mix(ab, bc, T);
                vec2 bcd = mix(bc, cd, T);
                vec2 vertexpos = mix(abc, bcd, T);

                if (gl_TessCoord.x == 1.0) {
                    // We are the anchor point that fans from the center of the curve's contour.
                    vertexpos = fanpoint[0];
                } else if (gl_TessCoord.x != 0.0) {
                    // We are the interior point of the patch; center it inside [C(0), C(.5), C(1)].
                    vertexpos = (p0 + vertexpos + p3) / 3.0;
                }

                gl_Position = vec4(vertexpos * sk_RTAdjust.xz + sk_RTAdjust.yw, 0.0, 1.0);
            })");

    return code;
}

void GrTessellateWedgeShader::Impl::setData(const GrGLSLProgramDataManager& pdman,
                                            const GrPrimitiveProcessor& primProc,
                                            const CoordTransformRange& transformRange) {
    const GrTessellateWedgeShader& shader = primProc.cast<GrTessellateWedgeShader>();
    pdman.setSkMatrix(fViewMatrixUniform, shader.fViewMatrix);
}

GrGLSLPrimitiveProcessor* GrTessellateWedgeShader::createGLSLInstance(const GrShaderCaps&) const {
    return new Impl;
}

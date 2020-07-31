/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStrokeTessellateShader.h"

#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/tessellate/GrWangsFormula.h"

class GrStrokeTessellateShader::Impl : public GrGLSLGeometryProcessor {
public:
    const char* getMiterLimitUniformName(const GrGLSLUniformHandler& uniformHandler) const {
        return uniformHandler.getUniformCStr(fMiterLimitUniform);
    }

    const char* getSkewMatrixUniformName(const GrGLSLUniformHandler& uniformHandler) const {
        return uniformHandler.getUniformCStr(fSkewMatrixUniform);
    }

private:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& shader = args.fGP.cast<GrStrokeTessellateShader>();
        args.fVaryingHandler->emitAttributes(shader);

        fMiterLimitUniform = args.fUniformHandler->addUniform(nullptr, kTessControl_GrShaderFlag,
                                                              kFloat_GrSLType, "miterLimit",
                                                              nullptr);

        if (!shader.viewMatrix().isIdentity()) {
            fSkewMatrixUniform = args.fUniformHandler->addUniform(nullptr,
                                                                  kTessEvaluation_GrShaderFlag,
                                                                  kFloat3x3_GrSLType, "skewMatrix",
                                                                  nullptr);
        }

        const char* colorUniformName;
        fColorUniform = args.fUniformHandler->addUniform(nullptr, kFragment_GrShaderFlag,
                                                         kHalf4_GrSLType, "color",
                                                         &colorUniformName);

        // The vertex shader is pure pass-through. Stroke widths and normals are defined in local
        // path space, so we don't apply the view matrix until after tessellation.
        args.fVertBuilder->declareGlobal(GrShaderVar("P", kFloat2_GrSLType,
                                                     GrShaderVar::TypeModifier::Out));
        args.fVertBuilder->codeAppendf("P = inputPoint;");

        // The fragment shader just outputs a uniform color.
        args.fFragBuilder->codeAppendf("%s = %s;", args.fOutputColor, colorUniformName);
        args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrPrimitiveProcessor& primProc) override {
        const auto& shader = primProc.cast<GrStrokeTessellateShader>();

        if (shader.fMiterLimitOrZero != 0 && fCachedMiterLimitValue != shader.fMiterLimitOrZero) {
            pdman.set1f(fMiterLimitUniform, shader.fMiterLimitOrZero);
            fCachedMiterLimitValue = shader.fMiterLimitOrZero;
        }

        if (!shader.viewMatrix().isIdentity()) {
            // Since the view matrix is applied after tessellation, it cannot expand the geometry in
            // any direction.
            SkASSERT(shader.viewMatrix().getMaxScale() < 1 + SK_ScalarNearlyZero);
            pdman.setSkMatrix(fSkewMatrixUniform, shader.viewMatrix());
        }

        pdman.set4fv(fColorUniform, 1, shader.fColor.vec());
    }

    GrGLSLUniformHandler::UniformHandle fMiterLimitUniform;
    GrGLSLUniformHandler::UniformHandle fSkewMatrixUniform;
    GrGLSLUniformHandler::UniformHandle fColorUniform;

    float fCachedMiterLimitValue = -1;
};

SkString GrStrokeTessellateShader::getTessControlShaderGLSL(
        const GrGLSLPrimitiveProcessor* glslPrimProc, const char* versionAndExtensionDecls,
        const GrGLSLUniformHandler& uniformHandler, const GrShaderCaps& shaderCaps) const {
    auto impl = static_cast<const GrStrokeTessellateShader::Impl*>(glslPrimProc);

    SkString code(versionAndExtensionDecls);
    code.append("layout(vertices = 1) out;\n");

    // TODO: CCPR stroking was written with a linearization tolerance of 1/8 pixel. Readdress this
    // ASAP to see if we can use GrTessellationPathRenderer::kLinearizationIntolerance (1/4 pixel)
    // instead.
    constexpr static float kIntolerance = 8;  // 1/8 pixel.
    code.appendf("const float kTolerance = %f;\n", 1/kIntolerance);
    code.appendf("const float kCubicK = %f;\n", GrWangsFormula::cubic_k(kIntolerance));

    const char* miterLimitName = impl->getMiterLimitUniformName(uniformHandler);
    code.appendf("uniform float %s;\n", miterLimitName);
    code.appendf("#define uMiterLimit %s\n", miterLimitName);

    code.append(R"(
            in vec2 P[];

            out vec4 X[];
            out vec4 Y[];
            out vec2 fanAngles[];
            out vec2 strokeRadii[];
            out vec2 outsetClamp[];

            void main() {
                // The 5th point contains the patch type and stroke radius.
                float strokeRadius = P[4].y;

                X[gl_InvocationID /*== 0*/] = vec4(P[0].x, P[1].x, P[2].x, P[3].x);
                Y[gl_InvocationID /*== 0*/] = vec4(P[0].y, P[1].y, P[2].y, P[3].y);
                fanAngles[gl_InvocationID /*== 0*/] = vec2(0);
                strokeRadii[gl_InvocationID /*== 0*/] = vec2(strokeRadius);
                outsetClamp[gl_InvocationID /*== 0*/] = vec2(-1, 1);

                // Calculate how many linear segments to chop this curve into.
                // (See GrWangsFormula::cubic().)
                float numSegments = sqrt(kCubicK * length(max(abs(P[2] - P[1]*2.0 + P[0]),
                                                              abs(P[3] - P[2]*2.0 + P[1]))));

                // A patch can override the number of segments it gets chopped into by passing a
                // positive value as P[4].x.
                if (P[4].x > 0) {
                    numSegments = P[4].x;
                }

                // A negative value in P[4].x means this patch actually represents a join instead
                // of a stroked cubic. Joins are implemented as radial fans from the junction point.
                if (P[4].x < 0) {
                    // Start by finding the angle between the tangents coming in and out of the
                    // join.
                    vec2 c0 = P[1] - P[0];
                    vec2 c1 = P[3] - P[2];
                    float theta = atan(determinant(mat2(c0, c1)), dot(c0, c1));

                    // Determine the beginning and end angles of our join.
                    fanAngles[gl_InvocationID /*== 0*/] = atan(c0.y, c0.x) + vec2(0, theta);

                    float joinType = P[4].x;
                    if (joinType <= -3) {
                        // Round join. Decide how many fan segments we need in order to be smooth.
                        numSegments = abs(theta) / (2 * acos(1 - kTolerance/strokeRadius));
                    } else if (joinType == -2) {
                        // Miter join. Draw a fan with 2 segments and lengthen the interior radius
                        // so it matches the miter point.
                        // (Or draw a 1-segment fan if we exceed the miter limit.)
                        float miterRatio = 1.0 / cos(.5 * theta);
                        strokeRadii[gl_InvocationID /*== 0*/] = strokeRadius * vec2(1, miterRatio);
                        numSegments = (miterRatio <= uMiterLimit) ? 2.0 : 1.0;
                    } else {
                        // Bevel join. Make a fan with only one segment.
                        numSegments = 1;
                    }

                    if (strokeRadius * abs(theta) < kTolerance) {
                        // The join angle is too tight to guarantee there won't be gaps on the
                        // inside of the junction. Just in case our join was supposed to only go on
                        // the outside, switch to an internal bevel that ties all 4 incoming
                        // vertices together. The join angle is so tight that bevels, miters, and
                        // rounds will all look the same anyway.
                        numSegments = 1;
                        // Paranoia. The next shader uses "fanAngles.x != fanAngles.y" as the test
                        // to decide whether it is emitting a cubic or a fan. But if theta is close
                        // enough to zero, that might fail. Assign arbitrary, nonequal values. This
                        // is fine because we will only draw one segment with vertices at T=0 and
                        // T=1, and the shader won't use fanAngles on the two outer vertices.
                        fanAngles[gl_InvocationID /*== 0*/] = vec2(1, 0);
                    } else if (joinType != -4) {
                        // This is a standard join. Restrict it to the outside of the junction.
                        outsetClamp[gl_InvocationID /*== 0*/] = mix(
                                vec2(-1, 1), vec2(0), lessThan(vec2(-theta, theta), vec2(0)));
                    }
                }

                // Tessellate a "strip" of numSegments quads.
                numSegments = max(1, numSegments);
                gl_TessLevelInner[0] = numSegments;
                gl_TessLevelInner[1] = 2.0;
                gl_TessLevelOuter[0] = 2.0;
                gl_TessLevelOuter[1] = numSegments;
                gl_TessLevelOuter[2] = 2.0;
                gl_TessLevelOuter[3] = numSegments;
            }
    )");

    return code;
}

SkString GrStrokeTessellateShader::getTessEvaluationShaderGLSL(
        const GrGLSLPrimitiveProcessor* glslPrimProc, const char* versionAndExtensionDecls,
        const GrGLSLUniformHandler& uniformHandler, const GrShaderCaps&) const {
    auto impl = static_cast<const GrStrokeTessellateShader::Impl*>(glslPrimProc);

    SkString code(versionAndExtensionDecls);
    code.append("layout(quads, equal_spacing, ccw) in;\n");

    const char* skewMatrixName = nullptr;
    if (!this->viewMatrix().isIdentity()) {
        skewMatrixName = impl->getSkewMatrixUniformName(uniformHandler);
        code.appendf("uniform mat3x3 %s;\n", skewMatrixName);
    }

    code.append(R"(
            in vec4 X[];
            in vec4 Y[];
            in vec2 fanAngles[];
            in vec2 strokeRadii[];
            in vec2 outsetClamp[];

            uniform vec4 sk_RTAdjust;

            void main() {
                float strokeRadius = strokeRadii[0].x;

                mat4x2 P = transpose(mat2x4(X[0], Y[0]));
                float T = gl_TessCoord.x;

                // Evaluate the cubic at T. Use De Casteljau's for its accuracy and stability.
                vec2 ab = mix(P[0], P[1], T);
                vec2 bc = mix(P[1], P[2], T);
                vec2 cd = mix(P[2], P[3], T);
                vec2 abc = mix(ab, bc, T);
                vec2 bcd = mix(bc, cd, T);
                vec2 position = mix(abc, bcd, T);

                // Find the normalized tangent vector at T.
                vec2 tangent = bcd - abc;
                if (tangent == vec2(0)) {
                    // We get tangent=0 if (P0 == P1 and T == 0), of if (P2 == P3 and T == 1).
                    tangent = (T == 0) ? P[2] - P[0] : P[3] - P[1];
                }
                tangent = normalize(tangent);

                // If the fanAngles are not equal, it means this patch actually represents a join
                // instead of a stroked cubic. Joins are implemented as radial fans from the
                // junction point.
                //
                // The caller carefully sets up the control points on junctions so the above math
                // lines up exactly with the incoming stroke vertices at T=0 and T=1, but for
                // interior T values we fall back on the fan's arc equation instead.
                if (fanAngles[0].x != fanAngles[0].y && T != 0 && T != 1) {
                    position = P[0];
                    float theta = mix(fanAngles[0].x, fanAngles[0].y, T);
                    tangent = vec2(cos(theta), sin(theta));
                    // Miters use a larger radius for the internal vertex.
                    strokeRadius = strokeRadii[0].y;
                }

                // Determine how far to outset our vertex orthogonally from the curve.
                float outset = gl_TessCoord.y * 2 - 1;
                outset = clamp(outset, outsetClamp[0].x, outsetClamp[0].y);
                outset *= strokeRadius;

                vec2 vertexpos = position + vec2(-tangent.y, tangent.x) * outset;
    )");

    // Transform after tessellation. Stroke widths and normals are defined in (pre-transform) local
    // path space.
    if (!this->viewMatrix().isIdentity()) {
        code.appendf("vertexpos = (%s * vec3(vertexpos, 1)).xy;\n", skewMatrixName);
    }

    code.append(R"(
                gl_Position = vec4(vertexpos * sk_RTAdjust.xz + sk_RTAdjust.yw, 0.0, 1.0);
            }
    )");

    return code;
}

GrGLSLPrimitiveProcessor* GrStrokeTessellateShader::createGLSLInstance(
        const GrShaderCaps&) const {
    return new Impl;
}

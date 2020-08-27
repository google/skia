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

constexpr static float kLinearizationIntolerance =
        GrTessellationPathRenderer::kLinearizationIntolerance;

class GrStrokeTessellateShader::Impl : public GrGLSLGeometryProcessor {
public:
    const char* getTessControlArgsUniformName(const GrGLSLUniformHandler& uniformHandler) const {
        return uniformHandler.getUniformCStr(fTessControlArgsUniform);
    }
    const char* getTranslateUniformName(const GrGLSLUniformHandler& uniformHandler) const {
        return uniformHandler.getUniformCStr(fTranslateUniform);
    }
    const char* getAffineMatrixUniformName(const GrGLSLUniformHandler& uniformHandler) const {
        return uniformHandler.getUniformCStr(fAffineMatrixUniform);
    }

private:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& shader = args.fGP.cast<GrStrokeTessellateShader>();
        args.fVaryingHandler->emitAttributes(shader);

        auto* uniHandler = args.fUniformHandler;
        fTessControlArgsUniform = uniHandler->addUniform(nullptr, kTessControl_GrShaderFlag,
                                                         kFloat2_GrSLType, "tessControlArgs",
                                                         nullptr);
        if (!shader.viewMatrix().isIdentity()) {
            fTranslateUniform = uniHandler->addUniform(nullptr, kTessEvaluation_GrShaderFlag,
                                                       kFloat2_GrSLType, "translate", nullptr);
            fAffineMatrixUniform = uniHandler->addUniform(nullptr, kTessEvaluation_GrShaderFlag,
                                                          kFloat2x2_GrSLType, "affineMatrix",
                                                          nullptr);
        }
        const char* colorUniformName;
        fColorUniform = uniHandler->addUniform(nullptr, kFragment_GrShaderFlag, kHalf4_GrSLType,
                                               "color", &colorUniformName);

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
        // tessControlArgs.x is the tolerance in pixels.
        pdman.set2f(fTessControlArgsUniform, 1 / (kLinearizationIntolerance * shader.fMatrixScale),
                    shader.fMiterLimit);
        const SkMatrix& m = shader.viewMatrix();
        if (!m.isIdentity()) {
            pdman.set2f(fTranslateUniform, m.getTranslateX(), m.getTranslateY());
            float affineMatrix[4] = {m.getScaleX(), m.getSkewY(), m.getSkewX(), m.getScaleY()};
            pdman.setMatrix2f(fAffineMatrixUniform, affineMatrix);
        }
        pdman.set4fv(fColorUniform, 1, shader.fColor.vec());
    }

    GrGLSLUniformHandler::UniformHandle fTessControlArgsUniform;
    GrGLSLUniformHandler::UniformHandle fTranslateUniform;
    GrGLSLUniformHandler::UniformHandle fAffineMatrixUniform;
    GrGLSLUniformHandler::UniformHandle fColorUniform;
};

SkString GrStrokeTessellateShader::getTessControlShaderGLSL(
        const GrGLSLPrimitiveProcessor* glslPrimProc, const char* versionAndExtensionDecls,
        const GrGLSLUniformHandler& uniformHandler, const GrShaderCaps& shaderCaps) const {
    auto impl = static_cast<const GrStrokeTessellateShader::Impl*>(glslPrimProc);

    SkString code(versionAndExtensionDecls);
    code.append("layout(vertices = 1) out;\n");

    code.appendf("const float kPI = 3.141592653589793238;\n");
    code.appendf("const float kMaxTessellationSegments = %i;\n",
                 shaderCaps.maxTessellationSegments());

    const char* tessControlArgsName = impl->getTessControlArgsUniformName(uniformHandler);
    code.appendf("uniform vec2 %s;\n", tessControlArgsName);
    code.appendf("#define uTolerance %s.x\n", tessControlArgsName);
    code.appendf("#define uMiterLimit %s.y\n", tessControlArgsName);

    code.append(R"(
    in vec2 P[];

    // Cubic control points.
    out vec4 P01[];
    out vec4 P23[];

    // Cubic derivative matrix colums.
    out vec3 C_x[];
    out vec3 C_y[];

    out vec4 packedArgs0[];
    out vec4 packedArgs1[];

    // The built-in atan() is undefined when x == 0. This method relieves that restriction, but also
    // can return values larger than 2*kPI. This shouldn't matter for our purposes.
    float atan2(vec2 v) {
        float bias = 0;
        if (abs(v.y) > abs(v.x)) {
            v = vec2(v.y, -v.x);
            bias = kPI/2;
        }
        return atan(v.y, v.x) + bias;
    }

    void main() {
        // Unpack the 5th input point, which contains the patch type and stroke radius.
        float patchType = P[4].x;
        float strokeRadius = P[4].y;

        // Calculate the number of evenly spaced (in the parametric sense) segments to chop the
        // curve into. (See GrWangsFormula::cubic() for more documentation on this formula.) The
        // final tessellated strip will be a composition of these parametric segments as well as
        // radial segments.
        float numParametricSegments = sqrt(
                .75/uTolerance * length(max(abs(P[2] - P[1]*2.0 + P[0]),
                                            abs(P[3] - P[2]*2.0 + P[1]))));
        if (P[1] == P[0] && P[2] == P[3]) {
            // This type of curve is used to represent flat lines, but wang's formula does not
            // return 1 segment. Force numParametricSegments to 1.
            numParametricSegments = 1;
        }
        numParametricSegments = max(ceil(numParametricSegments), 1);

        // Find a tangent matrix C' in power basis form. (This gives the derivative scaled by 1/3.)
        //
        //                                                  |C'x  C'y|
        //     dx,dy (divided by 3) = tangent = |T^2 T 1| * | .    . |
        //                                                  | .    . |
        mat2x3 C_ = mat4x3(-1,  2, -1,
                            3, -4,  1,
                           -3,  2,  0,
                            1,  0,  0) * transpose(mat4x2(P[0], P[1], P[2], P[3]));

        // Find the beginning and ending tangents. This works for now because it is illegal for the
        // caller to send us a curve where P0=P1=P2 or P1=P2=P3.
        vec2 tan0 = (P[1] == P[0]) ? P[2] - P[0] : P[1] - P[0];
        vec2 tan1 = (P[3] == P[2]) ? P[3] - P[1] : P[3] - P[2];

        // Determine the curve's start angle.
        float angle0 = atan2(tan0);

        // Determine the curve's total rotation. It is illegal for the caller to send us a curve
        // that rotates more than 180 degrees or a curve that has inflections, so we only need to
        // take the inverse cosine of the dot product.
        vec2 tan0norm = normalize(tan0);
        vec2 tan1norm = normalize(tan1);
        float cosTheta = dot(tan1norm, tan0norm);
        float rotation = acos(clamp(cosTheta, -1, +1));

        // Adjust sign of rotation to match the direction the curve turns.
        if (determinant(mat3x2(.25,1,.5,1,1,0) * C_) < 0) {  // i.e., if cross(F'(.5), F''(.5) < 0
            rotation = -rotation;
        }

        // Calculate the number of evenly spaced radial segments to chop the curve into. Radial
        // segments divide the curve's rotation into even steps. The final tessellated strip will be
        // a composition of both parametric and radial segments.
        float numRadialSegments = abs(rotation) / (2 * acos(max(1 - uTolerance/strokeRadius, -1)));
        numRadialSegments = max(ceil(numRadialSegments), 1);

        // Set up joins.
        float innerStrokeRadius = 0;  // Used for miter joins.
        vec2 strokeOutsetClamp = vec2(-1, 1);
        if (patchType != 0) {  // A non-zero patchType means this patch is a join.
            numParametricSegments = 1;  // Joins only have radial segments.
            innerStrokeRadius = strokeRadius;  // A non-zero innerStrokeRadius designates a join.
            float joinType = abs(patchType);
            if (joinType == 2) {
                // Miter join. Draw a fan with 2 segments and lengthen the interior radius
                // so it matches the miter point.
                // (Or draw a 1-segment fan if we exceed the miter limit.)
                float miterRatio = 1.0 / cos(.5 * rotation);
                numRadialSegments = (miterRatio <= uMiterLimit) ? 2.0 : 1.0;
                innerStrokeRadius = strokeRadius * miterRatio;
            } else if (joinType == 1) {
                // Bevel join. Make a fan with only one segment.
                numRadialSegments = 1;
            }
            if (length(tan0norm - tan1norm) * strokeRadius < uTolerance) {
                // The join angle is too tight to guarantee there won't be gaps on the inside of the
                // junction. Just in case our join was supposed to only go on the outside, switch to
                // a double sided bevel that ties all 4 incoming vertices together. The join angle
                // is so tight that bevels, miters, and rounds will all look the same anyway.
                numRadialSegments = 1;
            } else if (patchType > 0) {
                // This is join is single-sided. Clamp it to the outer side of the junction.
                strokeOutsetClamp = (rotation > 0) ? vec2(-1,0) : vec2(0,1);
            }
        }

        // Finalize the numbers of segments.
        numRadialSegments = min(numRadialSegments, kMaxTessellationSegments);
        numParametricSegments = min(numParametricSegments,
                                    kMaxTessellationSegments + 1 - numRadialSegments);

        // The first and last edges are shared by both the parametric and radial sets of edges, so
        // the total number of edges is:
        //
        //   numTotalEdges = numParametricEdges + numRadialEdges - 2
        //
        // It's also important to differentiate between the number of edges and segments in a strip:
        //
        //   numTotalSegments = numTotalEdges - 1
        //
        // So the total number of segments in the combined strip is:
        //
        //   numTotalSegments = numParametricEdges + numRadialEdges - 2 - 1
        //                    = numParametricSegments + 1 + numRadialSegments + 1 - 2 - 1
        //                    = numParametricSegments + numRadialSegments - 1
        //
        float numTotalSegments = numParametricSegments + numRadialSegments - 1;

        // Tessellate a "quad strip" with numTotalSegments.
        gl_TessLevelInner[0] = numTotalSegments;
        gl_TessLevelInner[1] = 2.0;
        gl_TessLevelOuter[0] = 2.0;
        gl_TessLevelOuter[1] = numTotalSegments;
        gl_TessLevelOuter[2] = 2.0;
        gl_TessLevelOuter[3] = numTotalSegments;

        // Pack the arguments for the next stage.
        P01[gl_InvocationID /*== 0*/] = vec4(P[0], P[1]);
        P23[gl_InvocationID /*== 0*/] = vec4(P[2], P[3]);
        C_x[gl_InvocationID /*== 0*/] = C_[0];
        C_y[gl_InvocationID /*== 0*/] = C_[1];
        packedArgs0[gl_InvocationID /*== 0*/] = vec4(numParametricSegments, numRadialSegments,
                                                     angle0, rotation / numRadialSegments);
        packedArgs1[gl_InvocationID /*== 0*/] = vec4(strokeRadius, innerStrokeRadius,
                                                     strokeOutsetClamp);
    }
    )");

    return code;
}

SkString GrStrokeTessellateShader::getTessEvaluationShaderGLSL(
        const GrGLSLPrimitiveProcessor* glslPrimProc, const char* versionAndExtensionDecls,
        const GrGLSLUniformHandler& uniformHandler, const GrShaderCaps& shaderCaps) const {
    auto impl = static_cast<const GrStrokeTessellateShader::Impl*>(glslPrimProc);

    SkString code(versionAndExtensionDecls);
    code.append("layout(quads, equal_spacing, ccw) in;\n");

    // Use a #define to make extra sure we don't prevent the loop from unrolling.
    code.appendf("#define MAX_TESSELLATION_SEGMENTS_LOG2 %i\n",
                 SkNextLog2(shaderCaps.maxTessellationSegments()));

    code.appendf("const float kPI = 3.141592653589793238;\n");

    if (!this->viewMatrix().isIdentity()) {
        const char* translateName = impl->getTranslateUniformName(uniformHandler);
        code.appendf("uniform vec2 %s;\n", translateName);
        code.appendf("#define uTranslate %s\n", translateName);
        const char* affineMatrixName = impl->getAffineMatrixUniformName(uniformHandler);
        code.appendf("uniform mat2x2 %s;\n", affineMatrixName);
        code.appendf("#define uAffineMatrix %s\n", affineMatrixName);
    }

    code.append(R"(
    // Cubic control points.
    in vec4 P01[];
    in vec4 P23[];

    // Cubic derivative matrix colums.
    in vec3 C_x[];
    in vec3 C_y[];

    in vec4 packedArgs0[];
    in vec4 packedArgs1[];

    uniform vec4 sk_RTAdjust;

    void main() {
        // Unpack arguments from the previous stage.
        mat4x2 P = mat4x2(P01[0], P23[0]);
        mat2x3 C_ = mat2x3(C_x[0], C_y[0]);
        float numParametricSegments = packedArgs0[0].x;
        float numRadialSegments = packedArgs0[0].y;
        float angle0 = packedArgs0[0].z;
        float radsPerSegment = packedArgs0[0].w;
        float strokeRadius = packedArgs1[0].x;
        float innerStrokeRadius = packedArgs1[0].y;
        vec2 strokeOutsetClamp = packedArgs1[0].zw;

        // Find the beginning and ending tangents. This works for now because it is illegal for the
        // caller to send us a curve where P0=P1=P2 or P1=P2=P3.
        vec2 tan0 = (P[1] == P[0]) ? P[2] - P[0] : P[1] - P[0];
        vec2 tan1 = (P[3] == P[2]) ? P[3] - P[1] : P[3] - P[2];

        // Our patch is composed of exactly "numTotalSegments + 1" stroke-width edges that run
        // orthogonal to the curve and make a strip of "numTotalSegments" quads. Determine which
        // discrete edge belongs to this invocation. An edge can either come from a parametric
        // segment or a radial one.
        float numTotalSegments = numParametricSegments + numRadialSegments - 1;
        float totalEdgeID = round(gl_TessCoord.x * numTotalSegments);

        // Find the matrix C_sT that produces an (arbitrarily scaled) tangent vector from a
        // parametric edge ID:
        //
        //     C_sT * |parametricEdgeID^2 parametricEdgeId 1| = tangent * C
        //
        vec3 s = vec3(1, numParametricSegments, numParametricSegments * numParametricSegments);
        mat3x2 C_sT = transpose(mat2x3(C_[0]*s, C_[1]*s));

        // Run an O(log N) search to determine the highest parametric edge that is located on or
        // before the totalEdgeID. A total edge ID is determined by the sum of complete parametric
        // and radial segments behind it. i.e., find the highest parametric edge where:
        //
        //    parametricEdgeID + floor(numRadialSegmentsAtParametricT) <= totalEdgeID
        //
        float lastParametricEdgeID = 0;
        float maxParametricEdgeID = min(numParametricSegments - 1, totalEdgeID);
        vec2 tan0norm = normalize(tan0);
        float negAbsRadsPerSegment = -abs(radsPerSegment);
        float maxRotation0 = (1 + totalEdgeID) * abs(radsPerSegment);
        for (int exp = MAX_TESSELLATION_SEGMENTS_LOG2 - 1; exp >= 0; --exp) {
            // Test the parametric edge at lastParametricEdgeID + 2^exp.
            float testParametricID = lastParametricEdgeID + (1 << exp);
            if (testParametricID <= maxParametricEdgeID) {
                vec2 testTan = fma(vec2(testParametricID), C_sT[0], C_sT[1]);
                testTan = fma(vec2(testParametricID), testTan, C_sT[2]);
                float cosRotation = dot(normalize(testTan), tan0norm);
                float maxRotation = fma(testParametricID, negAbsRadsPerSegment, maxRotation0);
                maxRotation = min(maxRotation, kPI);
                // Is rotation <= maxRotation? (i.e., is the number of complete radial segments
                // behind testT, + testParametricID <= totalEdgeID?)
                // NOTE: We bias cos(maxRotation) downward for fp32 error. Otherwise a flat section
                // following a 180 degree turn might not render properly.
                if (cosRotation >= cos(maxRotation) - 1e-5) {
                    // testParametricID is on or before the totalEdgeID. Keep it!
                    lastParametricEdgeID = testParametricID;
                }
            }
        }

        // Find the homogeneous T value of the parametric edge at lastParametricEdgeID.
        // (The scalar T value would be "parametricT.s / parametricT.t").
        vec2 parametricT = vec2(lastParametricEdgeID, numParametricSegments);

        // Now that we've identified the highest parametric edge on or before the totalEdgeID,
        // the highest radial edge is easy:
        float lastRadialEdgeID = totalEdgeID - lastParametricEdgeID;
        float radialAngle = fma(lastRadialEdgeID, radsPerSegment, angle0);

        // Find the T value of the edge at lastRadialEdgeID. This is the point whose tangent angle
        // is equal to radialAngle, or whose tangent vector is orthogonal to "norm".
        vec2 tangent = vec2(cos(radialAngle), sin(radialAngle));
        vec2 norm = vec2(-tangent.y, tangent.x);

        // Find the T value where the cubic's tangent is orthogonal to norm:
        //
        //                                          |C'x  C'y|
        //   dot(tangent, norm) == 0:   |T^2 T 1| * | .    . | * norm == 0
        //                                          | .    . |
        //
        // The coeffs for the quadratic equation we need to solve are therefore: C' * norm.
        vec3 coeffs = C_ * norm;
        float a=coeffs.x, b=coeffs.y, c=coeffs.z;

        // Quadratic formula from Numerical Recipes in C. Roots are: q/a and c/q. Combined with our
        // method for choosing a root below, this works for all a=0, b=0, and c=0.
        float x = sqrt(max(b*b - 4*a*c, 0));
        if (b < 0) {
            x = -x;
        }
        float q = -.5 * (b + x);

        // Pick the root T value closest to .5. Since we chop serpentines at inflections and all
        // other curves at the midtangent, it will always be the case that the root we want is the
        // one closest to .5. (But see the note about cusps below.)
        // NOTE: we express the root in homogeneous coordinates. The scalar T value would be:
        // "radialT.s / radialT.t".
        float qa_5 = q*a*.5;
        vec2 radialT = (abs(q*q - qa_5) < abs(a*c - qa_5)) ? vec2(q,a) : vec2(c,q);
        if (lastRadialEdgeID == 0) {
            // On a chopped cusp, the 0th radial edge has orthogonal tangents at T=0 and T=1. Both
            // are equally close to 0.5, but luckily we know the 0th radial edge is always at T=0.
            radialT = vec2(0,1);
        }
        radialT *= sign(radialT.t);  // Keep the denominator positive.

        // Now that we've identified the homogeneous T values of the last parametric and radial
        // edges, our final T value for totalEdgeID is whichever is larger.
        vec2 T = (determinant(mat2(parametricT, radialT)) > 0) ? parametricT : radialT;

        // Evaluate the cubic at T.s/T.t. Use De Casteljau's for its accuracy and stability.
        vec2 weights = vec2(T.t - T.s, T.s);
        vec2 ab = mat2(P[0], P[1]) * weights;
        vec2 bc = mat2(P[1], P[2]) * weights;
        vec2 cd = mat2(P[2], P[3]) * weights;
        vec2 abc = mat2(ab, bc) * weights;
        vec2 bcd = mat2(bc, cd) * weights;
        vec2 abcd = mat2(abc, bcd) * weights;
        vec2 position = abcd / (T.t * T.t * T.t);

        // If we went with T=parametricT, then update the tangent. Otherwise leave it at the radial
        // tangent found previously. (In the event that parametricT == radialT, we keep the radial
        // tangent.)
        if (T != radialT) {
            tangent = bcd - abc;
        }

        if (gl_TessCoord.x == 0) {
            // The first edge always uses P[0] and the tangent as identified by the control points.
            // This guarantees that adjecent patches always use the same fp32 values for their
            // shared edge and get a water tight seam.
            tangent = tan0;
            position = P[0];
        } else if (gl_TessCoord.x == 1) {
            // The final edge always uses P[3] and the tangent as identified by the control points.
            // This guarantees that adjecent patches always use the same fp32 values for their
            // shared edge and get a water tight seam.
            tangent = tan1;
            position = P[3];
        } else if (innerStrokeRadius != 0) {
            // Miter joins use a larger radius for the internal vertex in order to reach the miter
            // point.
            strokeRadius = innerStrokeRadius;
        }

        // If innerStrokeRadius != 0 then this patch is a join.
        if (innerStrokeRadius != 0) {
            // ... Aaaand nevermind again if we are a join. Those all rotate around P[1].
            position = P[1];
        }

        // Determine how far to outset our vertex orthogonally from the curve.
        float outset = gl_TessCoord.y * 2 - 1;
        outset = clamp(outset, strokeOutsetClamp.x, strokeOutsetClamp.y);
        outset *= strokeRadius;

        vec2 vertexPos = position + normalize(vec2(-tangent.y, tangent.x)) * outset;
    )");

    // Transform after tessellation. Stroke widths and normals are defined in (pre-transform) local
    // path space.
    if (!this->viewMatrix().isIdentity()) {
        code.append("vertexPos = uAffineMatrix * vertexPos + uTranslate;");
    }

    code.append(R"(
        gl_Position = vec4(vertexPos * sk_RTAdjust.xz + sk_RTAdjust.yw, 0.0, 1.0);
    }
    )");

    return code;
}

GrGLSLPrimitiveProcessor* GrStrokeTessellateShader::createGLSLInstance(
        const GrShaderCaps&) const {
    return new Impl;
}

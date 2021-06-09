/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/shaders/GrStrokeTessellationShader.h"

#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/tessellate/GrStrokeTessellator.h"

// The built-in atan() is undefined when x==0. This method relieves that restriction, but also can
// return values larger than 2*PI. This shouldn't matter for our purposes.
const char* GrStrokeTessellationShader::Impl::kAtan2Fn = R"(
float atan2(float2 v) {
    float bias = 0.0;
    if (abs(v.y) > abs(v.x)) {
        v = float2(v.y, -v.x);
        bias = PI/2.0;
    }
    return atan(v.y, v.x) + bias;
})";

const char* GrStrokeTessellationShader::Impl::kCosineBetweenVectorsFn = R"(
float cosine_between_vectors(float2 a, float2 b) {
    // FIXME(crbug.com/800804,skbug.com/11268): This can overflow if we don't normalize exponents.
    float ab_cosTheta = dot(a,b);
    float ab_pow2 = dot(a,a) * dot(b,b);
    return (ab_pow2 == 0.0) ? 1.0 : clamp(ab_cosTheta * inversesqrt(ab_pow2), -1.0, 1.0);
})";

// Extends the middle radius to either the miter point, or the bevel edge if we surpassed the miter
// limit and need to revert to a bevel join.
const char* GrStrokeTessellationShader::Impl::kMiterExtentFn = R"(
float miter_extent(float cosTheta, float miterLimit) {
    float x = fma(cosTheta, .5, .5);
    return (x * miterLimit * miterLimit >= 1.0) ? inversesqrt(x) : sqrt(x);
})";

// Returns the number of radial segments required for each radian of rotation, in order for the
// curve to appear "smooth" as defined by the parametricPrecision.
const char* GrStrokeTessellationShader::Impl::kNumRadialSegmentsPerRadianFn = R"(
float num_radial_segments_per_radian(float parametricPrecision, float strokeRadius) {
    return .5 / acos(max(1.0 - 1.0/(parametricPrecision * strokeRadius), -1.0));
})";

// Unlike mix(), this does not return b when t==1. But it otherwise seems to get better
// precision than "a*(1 - t) + b*t" for things like chopping cubics on exact cusp points.
// We override this result anyway when t==1 so it shouldn't be a problem.
const char* GrStrokeTessellationShader::Impl::kUncheckedMixFn = R"(
float unchecked_mix(float a, float b, float T) {
    return fma(b - a, T, a);
}
float2 unchecked_mix(float2 a, float2 b, float T) {
    return fma(b - a, float2(T), a);
}
float4 unchecked_mix(float4 a, float4 b, float4 T) {
    return fma(b - a, T, a);
})";

void GrStrokeTessellationShader::Impl::emitTessellationCode(
        const GrStrokeTessellationShader& shader, SkString* code, GrGPArgs* gpArgs,
        const GrShaderCaps& shaderCaps) const {
    // The subclass is responsible to define the following symbols before calling this method:
    //
    //     // Functions.
    //     float2 unchecked_mix(float2, float2, float);
    //     float unchecked_mix(float, float, float);
    //
    //     // Values provided by either uniforms or attribs.
    //     float4x2 P;
    //     float w;
    //     float STROKE_RADIUS;
    //     float 2x2 AFFINE_MATRIX;
    //     float2 TRANSLATE;
    //
    //     // Values calculated by the specific subclass.
    //     float combinedEdgeID;
    //     bool isFinalEdge;
    //     float numParametricSegments;
    //     float radsPerSegment;
    //     float2 tan0;
    //     float2 tan1;
    //     float angle0;
    //     float strokeOutset;
    //
    code->appendf(R"(
    float2 tangent, strokeCoord;
    if (combinedEdgeID != 0 && !isFinalEdge) {
        // Compute the location and tangent direction of the stroke edge with the integral id
        // "combinedEdgeID", where combinedEdgeID is the sorted-order index of parametric and radial
        // edges. Start by finding the tangent function's power basis coefficients. These define a
        // tangent direction (scaled by some uniform value) as:
        //                                                 |T^2|
        //     Tangent_Direction(T) = dx,dy = |A  2B  C| * |T  |
        //                                    |.   .  .|   |1  |
        float2 A, B, C = P[1] - P[0];
        float2 D = P[3] - P[0];
        if (w >= 0.0) {
            // P0..P2 represent a conic and P3==P2. The derivative of a conic has a cumbersome
            // order-4 denominator. However, this isn't necessary if we are only interested in a
            // vector in the same *direction* as a given tangent line. Since the denominator scales
            // dx and dy uniformly, we can throw it out completely after evaluating the derivative
            // with the standard quotient rule. This leaves us with a simpler quadratic function
            // that we use to find a tangent.
            C *= w;
            B = .5*D - C;
            A = (w - 1.0) * D;
            P[1] *= w;
        } else {
            float2 E = P[2] - P[1];
            B = E - C;
            A = fma(float2(-3), E, D);
        }
        // FIXME(crbug.com/800804,skbug.com/11268): Consider normalizing the exponents in A,B,C at
        // this point in order to prevent fp32 overflow.

        // Now find the coefficients that give a tangent direction from a parametric edge ID:
        //
        //                                                                 |parametricEdgeID^2|
        //     Tangent_Direction(parametricEdgeID) = dx,dy = |A  B_  C_| * |parametricEdgeID  |
        //                                                   |.   .   .|   |1                 |
        //
        float2 B_ = B * (numParametricSegments * 2.0);
        float2 C_ = C * (numParametricSegments * numParametricSegments);

        // Run a binary search to determine the highest parametric edge that is located on or before
        // the combinedEdgeID. A combined ID is determined by the sum of complete parametric and
        // radial segments behind it. i.e., find the highest parametric edge where:
        //
        //    parametricEdgeID + floor(numRadialSegmentsAtParametricT) <= combinedEdgeID
        //
        float lastParametricEdgeID = 0.0;
        float maxParametricEdgeID = min(numParametricSegments - 1.0, combinedEdgeID);
        // FIXME(crbug.com/800804,skbug.com/11268): This normalize() can overflow.
        float2 tan0norm = normalize(tan0);
        float negAbsRadsPerSegment = -abs(radsPerSegment);
        float maxRotation0 = (1.0 + combinedEdgeID) * abs(radsPerSegment);
        for (int exp = %i - 1; exp >= 0; --exp) {
            // Test the parametric edge at lastParametricEdgeID + 2^exp.
            float testParametricID = lastParametricEdgeID + float(1 << exp);
            if (testParametricID <= maxParametricEdgeID) {
                float2 testTan = fma(float2(testParametricID), A, B_);
                testTan = fma(float2(testParametricID), testTan, C_);
                float cosRotation = dot(normalize(testTan), tan0norm);
                float maxRotation = fma(testParametricID, negAbsRadsPerSegment, maxRotation0);
                maxRotation = min(maxRotation, PI);
                // Is rotation <= maxRotation? (i.e., is the number of complete radial segments
                // behind testT, + testParametricID <= combinedEdgeID?)
                if (cosRotation >= cos(maxRotation)) {
                    // testParametricID is on or before the combinedEdgeID. Keep it!
                    lastParametricEdgeID = testParametricID;
                }
            }
        }

        // Find the T value of the parametric edge at lastParametricEdgeID.
        float parametricT = lastParametricEdgeID / numParametricSegments;

        // Now that we've identified the highest parametric edge on or before the
        // combinedEdgeID, the highest radial edge is easy:
        float lastRadialEdgeID = combinedEdgeID - lastParametricEdgeID;

        // Find the tangent vector on the edge at lastRadialEdgeID.
        float radialAngle = fma(lastRadialEdgeID, radsPerSegment, angle0);
        tangent = float2(cos(radialAngle), sin(radialAngle));
        float2 norm = float2(-tangent.y, tangent.x);

        // Find the T value where the tangent is orthogonal to norm. This is a quadratic:
        //
        //     dot(norm, Tangent_Direction(T)) == 0
        //
        //                         |T^2|
        //     norm * |A  2B  C| * |T  | == 0
        //            |.   .  .|   |1  |
        //
        float3 coeffs = norm * float3x2(A,B,C);
        float a=coeffs.x, b_over_2=coeffs.y, c=coeffs.z;
        float discr_over_4 = max(b_over_2*b_over_2 - a*c, 0.0);
        float q = sqrt(discr_over_4);
        if (b_over_2 > 0.0) {
            q = -q;
        }
        q -= b_over_2;

        // Roots are q/a and c/q. Since each curve section does not inflect or rotate more than 180
        // degrees, there can only be one tangent orthogonal to "norm" inside 0..1. Pick the root
        // nearest .5.
        float _5qa = -.5*q*a;
        float2 root = (abs(fma(q,q,_5qa)) < abs(fma(a,c,_5qa))) ? float2(q,a) : float2(c,q);
        float radialT = (root.t != 0.0) ? root.s / root.t : 0.0;
        radialT = clamp(radialT, 0.0, 1.0);

        if (lastRadialEdgeID == 0.0) {
            // The root finder above can become unstable when lastRadialEdgeID == 0 (e.g., if
            // there are roots at exatly 0 and 1 both). radialT should always == 0 in this case.
            radialT = 0.0;
        }

        // Now that we've identified the T values of the last parametric and radial edges, our final
        // T value for combinedEdgeID is whichever is larger.
        float T = max(parametricT, radialT);

        // Evaluate the cubic at T. Use De Casteljau's for its accuracy and stability.
        float2 ab = unchecked_mix(P[0], P[1], T);
        float2 bc = unchecked_mix(P[1], P[2], T);
        float2 cd = unchecked_mix(P[2], P[3], T);
        float2 abc = unchecked_mix(ab, bc, T);
        float2 bcd = unchecked_mix(bc, cd, T);
        float2 abcd = unchecked_mix(abc, bcd, T);

        // Evaluate the conic weight at T.
        float u = unchecked_mix(1.0, w, T);
        float v = w + 1 - u;  // == mix(w, 1, T)
        float uv = unchecked_mix(u, v, T);

        // If we went with T=parametricT, then update the tangent. Otherwise leave it at the radial
        // tangent found previously. (In the event that parametricT == radialT, we keep the radial
        // tangent.)
        if (T != radialT) {
            tangent = (w >= 0.0) ? bc*u - ab*v : bcd - abc;
        }

        strokeCoord = (w >= 0.0) ? abc/uv : abcd;
    } else {
        // Edges at the beginning and end of the strip use exact endpoints and tangents. This
        // ensures crack-free seaming between instances.
        tangent = (combinedEdgeID == 0) ? tan0 : tan1;
        strokeCoord = (combinedEdgeID == 0) ? P[0] : P[3];
    })", shader.maxParametricSegments_log2() /* Parametric/radial sort loop count. */);

    code->append(R"(
    // FIXME(crbug.com/800804,skbug.com/11268): This normalize() can overflow.
    float2 ortho = normalize(float2(tangent.y, -tangent.x));
    strokeCoord += ortho * (STROKE_RADIUS * strokeOutset);)");

    if (!shader.stroke().isHairlineStyle()) {
        // Normal case. Do the transform after tessellation.
        code->append(R"(
        float2 devCoord = AFFINE_MATRIX * strokeCoord + TRANSLATE;)");
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "devCoord");
        gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "strokeCoord");
    } else {
        // Hairline case. The scale and skew already happened before tessellation.
        code->append(R"(
        float2 devCoord = strokeCoord + TRANSLATE;
        float2 localCoord = inverse(AFFINE_MATRIX) * strokeCoord;)");
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "devCoord");
        gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localCoord");
    }
}

void GrStrokeTessellationShader::Impl::emitFragmentCode(const GrStrokeTessellationShader& shader,
                                                        const EmitArgs& args) {
    if (!shader.hasDynamicColor()) {
        // The fragment shader just outputs a uniform color.
        const char* colorUniformName;
        fColorUniform = args.fUniformHandler->addUniform(nullptr, kFragment_GrShaderFlag,
                                                         kHalf4_GrSLType, "color",
                                                         &colorUniformName);
        args.fFragBuilder->codeAppendf("half4 %s = %s;", args.fOutputColor, colorUniformName);
    } else {
        args.fFragBuilder->codeAppendf("half4 %s = %s;", args.fOutputColor,
                                       fDynamicColorName.c_str());
    }
    args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
}

void GrStrokeTessellationShader::Impl::setData(const GrGLSLProgramDataManager& pdman,
                                               const GrShaderCaps&,
                                               const GrGeometryProcessor& geomProc) {
    const auto& shader = geomProc.cast<GrStrokeTessellationShader>();
    const auto& stroke = shader.stroke();

    if (!shader.hasDynamicStroke()) {
        // Set up the tessellation control uniforms.
        GrStrokeTolerances tolerances;
        if (!stroke.isHairlineStyle()) {
            tolerances = GrStrokeTolerances::MakeNonHairline(shader.viewMatrix().getMaxScale(),
                                                             stroke.getWidth());
        } else {
            // In the hairline case we transform prior to tessellation. Set up tolerances for an
            // identity viewMatrix and a strokeWidth of 1.
            tolerances = GrStrokeTolerances::MakeNonHairline(1, 1);
        }
        float strokeRadius = (stroke.isHairlineStyle()) ? .5f : stroke.getWidth() * .5;
        pdman.set4f(fTessControlArgsUniform,
                    tolerances.fParametricPrecision,  // PARAMETRIC_PRECISION
                    tolerances.fNumRadialSegmentsPerRadian,  // NUM_RADIAL_SEGMENTS_PER_RADIAN
                    GrStrokeTessellationShader::GetJoinType(stroke),  // JOIN_TYPE
                    strokeRadius);  // STROKE_RADIUS
    } else {
        SkASSERT(!stroke.isHairlineStyle());
        float maxScale = shader.viewMatrix().getMaxScale();
        pdman.set1f(fTessControlArgsUniform,
                    GrStrokeTolerances::CalcParametricPrecision(maxScale));
    }

    if (shader.mode() == GrStrokeTessellationShader::Mode::kFixedCount) {
        SkASSERT(shader.fixedCountNumTotalEdges() != 0);
        pdman.set1f(fEdgeCountUniform, (float)shader.fixedCountNumTotalEdges());
    }

    // Set up the view matrix, if any.
    const SkMatrix& m = shader.viewMatrix();
    pdman.set2f(fTranslateUniform, m.getTranslateX(), m.getTranslateY());
    pdman.set4f(fAffineMatrixUniform, m.getScaleX(), m.getSkewY(), m.getSkewX(),
                m.getScaleY());

    if (!shader.hasDynamicColor()) {
        pdman.set4fv(fColorUniform, 1, shader.color().vec());
    }
}

void GrStrokeTessellationShader::getGLSLProcessorKey(const GrShaderCaps&,
                                                     GrProcessorKeyBuilder* b) const {
    bool keyNeedsJoin = (fMode != Mode::kHardwareTessellation) &&
                        !(fShaderFlags & ShaderFlags::kDynamicStroke);
    SkASSERT((int)fMode >> 2 == 0);
    SkASSERT(fStroke.getJoin() >> 2 == 0);
    // Attribs get worked into the key automatically during GrGeometryProcessor::getAttributeKey().
    // When color is in a uniform, it's always wide. kWideColor doesn't need to be considered here.
    uint32_t key = (uint32_t)(fShaderFlags & ~ShaderFlags::kWideColor);
    key = (key << 2) | (uint32_t)fMode;
    key = (key << 2) | ((keyNeedsJoin) ? fStroke.getJoin() : 0);
    key = (key << 1) | (uint32_t)fStroke.isHairlineStyle();
    key = (key << 8) | fMaxParametricSegments_log2;
    b->add32(key);
}

GrGLSLGeometryProcessor* GrStrokeTessellationShader::createGLSLInstance(const GrShaderCaps&) const {
    switch (fMode) {
        case Mode::kHardwareTessellation:
            return new HardwareImpl;
        case Mode::kLog2Indirect:
        case Mode::kFixedCount:
            return new InstancedImpl;
    }
    SkUNREACHABLE;
}

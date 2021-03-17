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

// The built-in atan() is undefined when x==0. This method relieves that restriction, but also can
// return values larger than 2*PI. This shouldn't matter for our purposes.
static const char* kAtan2Fn = R"(
float atan2(float2 v) {
    float bias = 0.0;
    if (abs(v.y) > abs(v.x)) {
        v = float2(v.y, -v.x);
        bias = PI/2.0;
    }
    return atan(v.y, v.x) + bias;
})";

static const char* kCosineBetweenVectorsFn = R"(
float cosine_between_vectors(float2 a, float2 b) {
    float ab_cosTheta = dot(a,b);
    float ab_pow2 = dot(a,a) * dot(b,b);
    return (ab_pow2 == 0.0) ? 1.0 : clamp(ab_cosTheta * inversesqrt(ab_pow2), -1.0, 1.0);
})";

// Extends the middle radius to either the miter point, or the bevel edge if we surpassed the miter
// limit and need to revert to a bevel join.
static const char* kMiterExtentFn = R"(
float miter_extent(float cosTheta, float miterLimit) {
    float x = fma(cosTheta, .5, .5);
    return (x * miterLimit * miterLimit >= 1.0) ? inversesqrt(x) : sqrt(x);
})";

static const char* kLengthPow2Fn = R"(
float length_pow2(float2 v) {
    return dot(v, v);
})";

static const char* kNumRadialSegmentsPerRadian = R"(
float num_radial_segments_per_radian(float parametricIntolerance, float strokeRadius) {
    return .5 / acos(max(1.0 - 1.0/(parametricIntolerance * strokeRadius), -1.0));
})";

// Unlike mix(), this does not return b when t==1. But it otherwise seems to get better
// precision than "a*(1 - t) + b*t" for things like chopping cubics on exact cusp points.
// We override this result anyway when t==1 so it shouldn't be a problem.
static const char* kUncheckedMixFn = R"(
float unchecked_mix(float a, float b, float T) {
    return fma(b - a, T, a);
}
float2 unchecked_mix(float2 a, float2 b, float T) {
    return fma(b - a, float2(T), a);
}
float4 unchecked_mix(float4 a, float4 b, float4 T) {
    return fma(b - a, T, a);
})";

// Calculates the number of evenly spaced (in the parametric sense) segments to chop a cubic into.
// (See GrWangsFormula::cubic() for more documentation on this formula.) The final tessellated strip
// will be a composition of these parametric segments as well as radial segments.
static void append_wangs_formula_fn(SkString* code, bool hasConics) {
    code->appendf(R"(
    float wangs_formula(in float4x2 P, in float w, in float parametricIntolerance) {
        const float CUBIC_TERM_POW2 = %f;
        float l0 = length_pow2(fma(float2(-2), P[1], P[2]) + P[0]);
        float l1 = length_pow2(fma(float2(-2), P[2], P[3]) + P[1]);
        float m = CUBIC_TERM_POW2 * max(l0, l1);)", GrWangsFormula::length_term_pow2<3>(1));
    if (hasConics) {
        code->appendf(R"(
        const float QUAD_TERM_POW2 = %f;
        m = (w > 0.0) ? QUAD_TERM_POW2 * l0 : m;)", GrWangsFormula::length_term_pow2<2>(1));
    }
    code->append(R"(
        return max(ceil(sqrt(parametricIntolerance * sqrt(m))), 1.0);
    })");
}

class GrStrokeTessellateShader::TessellationImpl : public GrGLSLGeometryProcessor {
public:
    const char* getTessArgsUniformName(const GrGLSLUniformHandler& uniformHandler) const {
        return uniformHandler.getUniformCStr(fTessArgsUniform);
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
        auto* uniHandler = args.fUniformHandler;
        auto* v = args.fVertBuilder;

        args.fVaryingHandler->emitAttributes(shader);

        v->defineConstant("float", "PI", "3.141592653589793238");

        // The vertex shader chops the curve into 3 sections in order to meet our tessellation
        // requirements. The stroke tessellator does not allow curve sections to inflect or to
        // rotate more than 180 degrees.
        //
        // We start by chopping at inflections (if the curve has any), or else at midtangent. If we
        // still don't have 3 sections after that then we just subdivide uniformly in parametric
        // space.
        using TypeModifier = GrShaderVar::TypeModifier;
        v->defineConstantf("float", "kParametricEpsilon", "1.0 / (%i * 128)",
                           args.fShaderCaps->maxTessellationSegments());  // 1/128 of a segment.

        // [numSegmentsInJoin, innerJoinRadiusMultiplier, prevJoinTangent.xy]
        v->declareGlobal(GrShaderVar("vsJoinArgs0", kFloat4_GrSLType, TypeModifier::Out));

        // [joinAngle0, radsPerJoinSegment, joinOutsetClamp.xy]
        v->declareGlobal(GrShaderVar("vsJoinArgs1", kFloat4_GrSLType, TypeModifier::Out));

        // Curve args.
        v->declareGlobal(GrShaderVar("vsPts01", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsPts23", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsPts45", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsPts67", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsPts89", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsTans01", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsTans23", kFloat4_GrSLType, TypeModifier::Out));
        if (shader.hasDynamicStroke()) {
            // [NUM_RADIAL_SEGMENTS_PER_RADIAN, STROKE_RADIUS]
            v->declareGlobal(GrShaderVar("vsStrokeArgs", kFloat2_GrSLType, TypeModifier::Out));
        }
        if (shader.hasDynamicColor()) {
            v->declareGlobal(GrShaderVar("vsColor", kHalf4_GrSLType, TypeModifier::Out));
        }

        v->insertFunction(kAtan2Fn);
        v->insertFunction(kCosineBetweenVectorsFn);
        v->insertFunction(kMiterExtentFn);
        v->insertFunction(kUncheckedMixFn);
        v->insertFunction(kLengthPow2Fn);
        if (shader.hasDynamicStroke()) {
            v->insertFunction(kNumRadialSegmentsPerRadian);
        }

        if (!shader.hasDynamicStroke()) {
            // [PARAMETRIC_INTOLERANCE, NUM_RADIAL_SEGMENTS_PER_RADIAN, JOIN_TYPE, STROKE_RADIUS]
            const char* tessArgsName;
            fTessArgsUniform = uniHandler->addUniform(nullptr,
                                                      kVertex_GrShaderFlag |
                                                      kTessControl_GrShaderFlag |
                                                      kTessEvaluation_GrShaderFlag,
                                                      kFloat4_GrSLType, "tessArgs", &tessArgsName);
            v->codeAppendf(R"(
            float NUM_RADIAL_SEGMENTS_PER_RADIAN = %s.y;
            float JOIN_TYPE = %s.z;)", tessArgsName, tessArgsName);
        } else {
            const char* parametricIntoleranceName;
            fTessArgsUniform = uniHandler->addUniform(nullptr,
                                                      kVertex_GrShaderFlag |
                                                      kTessControl_GrShaderFlag |
                                                      kTessEvaluation_GrShaderFlag,
                                                      kFloat_GrSLType, "parametricIntolerance",
                                                      &parametricIntoleranceName);
            v->codeAppendf(R"(
            float STROKE_RADIUS = dynamicStrokeAttr.x;
            float NUM_RADIAL_SEGMENTS_PER_RADIAN = num_radial_segments_per_radian(%s,STROKE_RADIUS);
            float JOIN_TYPE = dynamicStrokeAttr.y;)", parametricIntoleranceName);
        }

        if (!shader.viewMatrix().isIdentity()) {
            fTranslateUniform = uniHandler->addUniform(nullptr, kTessEvaluation_GrShaderFlag,
                                                       kFloat2_GrSLType, "translate", nullptr);
            const char* affineMatrixName;
            // Hairlines apply the affine matrix in their vertex shader, prior to tessellation.
            // Otherwise the entire view matrix gets applied at the end of the tess eval shader.
            auto affineMatrixVisibility = (shader.fStroke.isHairlineStyle()) ?
                    kVertex_GrShaderFlag : kTessEvaluation_GrShaderFlag;
            fAffineMatrixUniform = uniHandler->addUniform(nullptr, affineMatrixVisibility,
                                                          kFloat4_GrSLType, "affineMatrix",
                                                          &affineMatrixName);
            if (affineMatrixVisibility & kVertex_GrShaderFlag) {
                v->codeAppendf("float2x2 AFFINE_MATRIX = float2x2(%s);\n", affineMatrixName);
            }
        }

        v->codeAppend(R"(
        // Unpack the control points.
        float2 prevControlPoint = prevCtrlPtAttr;
        float4x2 P = float4x2(pts01Attr, pts23Attr);)");

        if (shader.fStroke.isHairlineStyle() && !shader.viewMatrix().isIdentity()) {
            // Hairline case. Transform the points before tessellation. We can still hold off on the
            // translate until the end; we just need to perform the scale and skew right now.
            if (shader.hasConics()) {
                v->codeAppend(R"(
                P[0] = AFFINE_MATRIX * P[0];
                P[1] = AFFINE_MATRIX * P[1];
                P[2] = AFFINE_MATRIX * P[2];
                P[3] = isinf(P[3].y) ? P[3] : AFFINE_MATRIX * P[3];)");
            } else {
                v->codeAppend(R"(
                P = AFFINE_MATRIX * P;)");
            }
            v->codeAppend(R"(
            prevControlPoint = AFFINE_MATRIX * prevControlPoint;)");
        }

        v->codeAppend(R"(
        // Find the tangents. It's imperative that we compute these tangents from the original
        // (pre-chopping) input points or else the seams might crack.
        float2 prevJoinTangent = P[0] - prevControlPoint;
        float2 tan0 = ((P[1] == P[0]) ? P[2] : P[1]) - P[0];
        float2 tan1 = (P[3] == P[2] || isinf(P[3].y)) ? P[2] - P[1] : P[3] - P[2];

        if (tan0 == float2(0)) {
            // [p0, p0, p0, p3] is a reserved pattern that means this patch is a "bowtie".
            P[3] = P[0];  // Colocate all the points on the center of the bowtie.
            // Use the final curve section to draw the bowtie. Since the points are colocated, this
            // curve will register as a line, which overrides innerTangents as [tan0, tan0]. That
            // disables the first two sections of the curve because their tangents and points are
            // all equal.
            tan0 = prevJoinTangent;
            prevJoinTangent = float2(0);  // Disable the join section.
        }

        if (tan1 == float2(0)) {
            // [p0, p3, p3, p3] is a reserved pattern that means this patch is a join only. Colocate
            // all the curve's points to ensure it gets disabled by the tessellation stages.
            P[1] = P[2] = P[3] = P[0];
            // Since the points are colocated, this curve will register as a line, which overrides
            // innerTangents as [tan0, tan0]. Setting tan1=tan0 as well results in all tangents and
            // all points being equal, which disables every section of the curve.
            tan1 = tan0;
        }

        // Calculate the number of segments to chop the join into.
        float cosTheta = cosine_between_vectors(prevJoinTangent, tan0);
        float joinRotation = (cosTheta == 1) ? 0 : acos(cosTheta);
        if (cross(prevJoinTangent, tan0) < 0) {
            joinRotation = -joinRotation;
        }
        float joinRadialSegments = abs(joinRotation) * NUM_RADIAL_SEGMENTS_PER_RADIAN;
        float numSegmentsInJoin = (joinRadialSegments != 0 /*Is the join non-empty?*/ &&
                                   JOIN_TYPE >= 0 /*Is the join not a round type?*/)
                ? sign(JOIN_TYPE) + 1  // Non-empty bevel joins have 1 segment and miters have 2.
                : ceil(joinRadialSegments);  // Otherwise round up the number of radial segments.

        // Extends the middle join edge to the miter point.
        float innerJoinRadiusMultiplier = 1;
        if (JOIN_TYPE > 0 /*Is the join a miter type?*/) {
            innerJoinRadiusMultiplier = miter_extent(cosTheta, JOIN_TYPE/*miterLimit*/);
        }

        // Clamps join geometry to the exterior side of the junction.
        float2 joinOutsetClamp = float2(-1, 1);
        if (joinRadialSegments > .1 /*Does the join rotate more than 1/10 of a segment?*/) {
            // Only clamp if the join angle is large enough to guarantee there won't be cracks on
            // the interior side of the junction.
            joinOutsetClamp = (joinRotation > 0) ? float2(-1, 0) : float2(0, 1);
        }

        // Pack join args for the tessellation control stage.
        vsJoinArgs0 = float4(numSegmentsInJoin, innerJoinRadiusMultiplier, prevJoinTangent);
        vsJoinArgs1 = float4(atan2(prevJoinTangent), joinRotation / numSegmentsInJoin,
                             joinOutsetClamp);

        // Now find where to chop the curve so the resulting sub-curves are convex and do not rotate
        // more than 180 degrees. We don't need to worry about cusps because the caller chops those
        // out on the CPU. Start by finding the cubic's power basis coefficients. These define the
        // bezier curve as:
        //
        //                                    |T^3|
        //     Cubic(T) = x,y = |A  3B  3C| * |T^2| + P0
        //                      |.   .   .|   |T  |
        //
        // And the tangent direction (scaled by a uniform 1/3) will be:
        //
        //                                                 |T^2|
        //     Tangent_Direction(T) = dx,dy = |A  2B  C| * |T  |
        //                                    |.   .  .|   |1  |
        //
        float2 C = P[1] - P[0];
        float2 D = P[2] - P[1];
        float2 E = P[3] - P[0];
        float2 B = D - C;
        float2 A = fma(float2(-3), D, E);

        // Now find the cubic's inflection function. There are inflections where F' x F'' == 0.
        // We formulate this as a quadratic equation:  F' x F'' == aT^2 + bT + c == 0.
        // See: https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf
        // NOTE: We only need the roots, so a uniform scale factor does not affect the solution.
        float a = cross(A, B);
        float b = cross(A, C);
        float c = cross(B, C);
        float b_over_2 = b*.5;
        float discr_over_4 = b_over_2*b_over_2 - a*c;

        float2x2 innerTangents = float2x2(0);
        if (discr_over_4 <= 0) {
            // The curve does not inflect. This means it might rotate more than 180 degrees instead.
            // Craft a quadratic whose roots are the points were rotation == 180 deg and 0. (These
            // are the points where the tangent is parallel to tan0.)
            //
            //      Tangent_Direction(T) x tan0 == 0
            //      (AT^2 x tan0) + (2BT x tan0) + (C x tan0) == 0
            //      (A x C)T^2 + (2B x C)T + (C x C) == 0  [[because tan0 == P1 - P0 == C]]
            //      bT^2 + 2cT + 0 == 0  [[because A x C == b, B x C == c]]
            //
            // NOTE: When P0 == P1 then C != tan0, C == 0 and these roots will be undefined. But
            // that's ok because when P0 == P1 the curve cannot rotate more than 180 degrees anyway.
            a = b;
            b_over_2 = c;
            c = 0;
            discr_over_4 = b_over_2*b_over_2;
            innerTangents[0] = -C;
        }

        // Solve our quadratic equation for the chop points. This is inspired by the quadratic
        // formula from Numerical Recipes in C.
        float q = sqrt(discr_over_4);
        if (b_over_2 > 0) {
            q = -q;
        }
        q -= b_over_2;
        float2 chopT = float2((a != 0) ? q/a : 0,
                              (q != 0) ? c/q : 0);

        // Reposition any chop points that fall outside ~0..1 and clear their innerTangent.
        int numOutside = 0;
        if (chopT[0] <= kParametricEpsilon || chopT[0] >= 1 - kParametricEpsilon) {
            innerTangents[0] = float2(0);
            ++numOutside;
        }
        if (chopT[1] <= kParametricEpsilon || chopT[1] >= 1 - kParametricEpsilon) {
            // Swap places with chopT[0]. This ensures chopT[0] is outside when numOutside > 0.
            chopT = chopT.ts;
            innerTangents = float2x2(0,0, innerTangents[0]);
            ++numOutside;
        }
        if (numOutside == 2) {
            chopT[1] = 2/3.0;
        }
        if (numOutside >= 1) {
            chopT[0] = (chopT[1] <= .5) ? chopT[1] * .5 : fma(chopT[1], .5, .5);
        }

        // Sort the chop points.
        if (chopT[0] > chopT[1]) {
            chopT = chopT.ts;
            innerTangents = float2x2(innerTangents[1], innerTangents[0]);
        }

        // If the curve is a straight line, point, or conic, don't chop it into sections after all.
        if ((P[0] == P[1] && P[2] == P[3]) || isinf(P[3].y)) {
            chopT = float2(0);
            innerTangents = float2x2(tan0, tan0);
        }

        // Chop the curve at chopT[0] and chopT[1].
        float4 ab = unchecked_mix(P[0].xyxy, P[1].xyxy, chopT.sstt);
        float4 bc = unchecked_mix(P[1].xyxy, P[2].xyxy, chopT.sstt);
        float4 cd = isinf(P[3].y) ? P[2].xyxy : unchecked_mix(P[2].xyxy, P[3].xyxy, chopT.sstt);
        float4 abc = unchecked_mix(ab, bc, chopT.sstt);
        float4 bcd = unchecked_mix(bc, cd, chopT.sstt);
        float4 abcd = unchecked_mix(abc, bcd, chopT.sstt);
        float4 middle = unchecked_mix(abc, bcd, chopT.ttss);

        // Find tangents at the chop points if an inner tangent wasn't specified.
        if (innerTangents[0] == float2(0)) {
            innerTangents[0] = bcd.xy - abc.xy;
        }
        if (innerTangents[1] == float2(0)) {
            innerTangents[1] = bcd.zw - abc.zw;
        }

        // Pack curve args for the tessellation control stage.
        vsPts01 = float4(P[0], ab.xy);
        vsPts23 = float4(abc.xy, abcd.xy);
        vsPts45 = middle;
        vsPts67 = float4(abcd.zw, bcd.zw);
        vsPts89 = float4(cd.zw, P[3]);
        vsTans01 = float4(tan0, innerTangents[0]);
        vsTans23 = float4(innerTangents[1], tan1);)");
        if (shader.hasDynamicStroke()) {
            v->codeAppend(R"(
            vsStrokeArgs = float2(NUM_RADIAL_SEGMENTS_PER_RADIAN, STROKE_RADIUS);)");
        }
        if (shader.hasDynamicColor()) {
            v->codeAppend(R"(
            vsColor = dynamicColorAttr;)");
        }

        if (!shader.hasDynamicColor()) {
            // The fragment shader just outputs a uniform color.
            const char* colorUniformName;
            fColorUniform = uniHandler->addUniform(nullptr, kFragment_GrShaderFlag, kHalf4_GrSLType,
                                                   "color", &colorUniformName);
            args.fFragBuilder->codeAppendf("half4 %s = %s;", args.fOutputColor, colorUniformName);
        } else {
            // Color gets passed in from the tess evaluation shader.
            SkString flatness(args.fShaderCaps->preferFlatInterpolation() ? "flat" : "");
            args.fFragBuilder->declareGlobal(GrShaderVar(SkString("tesColor"), kHalf4_GrSLType,
                                                         TypeModifier::In, 0, SkString(),
                                                         flatness));
            args.fFragBuilder->codeAppendf("half4 %s = tesColor;", args.fOutputColor);
        }
        args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrPrimitiveProcessor& primProc) override {
        const auto& shader = primProc.cast<GrStrokeTessellateShader>();
        const auto& stroke = shader.fStroke;

        if (!shader.hasDynamicStroke()) {
            Tolerances tolerances;
            if (!stroke.isHairlineStyle()) {
                tolerances = Tolerances::MakeNonHairline(shader.viewMatrix().getMaxScale(),
                                                         stroke.getWidth());
            } else {
                // In the hairline case we transform prior to tessellation. Set up tolerances for an
                // identity viewMatrix and a strokeWidth of 1.
                tolerances = Tolerances::MakeNonHairline(1, 1);
            }
            float strokeRadius = (stroke.isHairlineStyle()) ? .5f : stroke.getWidth() * .5;
            pdman.set4f(fTessArgsUniform,
                        tolerances.fParametricIntolerance,  // PARAMETRIC_INTOLERANCE
                        tolerances.fNumRadialSegmentsPerRadian,  // NUM_RADIAL_SEGMENTS_PER_RADIAN
                        GetJoinType(shader.fStroke),  // JOIN_TYPE
                        strokeRadius);  // STROKE_RADIUS
        } else {
            SkASSERT(!stroke.isHairlineStyle());
            pdman.set1f(fTessArgsUniform,
                        Tolerances::CalcParametricIntolerance(shader.viewMatrix().getMaxScale()));
        }

        // Set up the view matrix, if any.
        const SkMatrix& m = shader.viewMatrix();
        if (!m.isIdentity()) {
            pdman.set2f(fTranslateUniform, m.getTranslateX(), m.getTranslateY());
            pdman.set4f(fAffineMatrixUniform, m.getScaleX(), m.getSkewY(), m.getSkewX(),
                        m.getScaleY());
        }

        if (!shader.hasDynamicColor()) {
            pdman.set4fv(fColorUniform, 1, shader.fColor.vec());
        }
    }

    GrGLSLUniformHandler::UniformHandle fTessArgsUniform;
    GrGLSLUniformHandler::UniformHandle fTranslateUniform;
    GrGLSLUniformHandler::UniformHandle fAffineMatrixUniform;
    GrGLSLUniformHandler::UniformHandle fColorUniform;
};

SkString GrStrokeTessellateShader::getTessControlShaderGLSL(
        const GrGLSLPrimitiveProcessor* glslPrimProc, const char* versionAndExtensionDecls,
        const GrGLSLUniformHandler& uniformHandler, const GrShaderCaps& shaderCaps) const {
    SkASSERT(fMode == Mode::kTessellation);
    auto impl = static_cast<const GrStrokeTessellateShader::TessellationImpl*>(glslPrimProc);

    SkString code(versionAndExtensionDecls);
    // Run 3 invocations: 1 for each section that the vertex shader chopped the curve into.
    code.append("layout(vertices = 3) out;\n");
    code.appendf("precision highp float;\n");

    code.appendf("#define float2 vec2\n");
    code.appendf("#define float3 vec3\n");
    code.appendf("#define float4 vec4\n");
    code.appendf("#define float2x2 mat2\n");
    code.appendf("#define float4x2 mat4x2\n");
    code.appendf("#define PI 3.141592653589793238\n");
    code.appendf("#define MAX_TESSELLATION_SEGMENTS %i.0\n", shaderCaps.maxTessellationSegments());
    code.appendf("#define cross cross2d\n");  // GLSL already has a function named "cross".

    const char* tessArgsName = impl->getTessArgsUniformName(uniformHandler);
    if (!this->hasDynamicStroke()) {
        code.appendf("uniform vec4 %s;\n", tessArgsName);
        code.appendf("#define PARAMETRIC_INTOLERANCE %s.x\n", tessArgsName);
        code.appendf("#define NUM_RADIAL_SEGMENTS_PER_RADIAN %s.y\n", tessArgsName);
    } else {
        code.appendf("uniform float %s;\n", tessArgsName);
        code.appendf("#define PARAMETRIC_INTOLERANCE %s\n", tessArgsName);
        code.appendf("#define NUM_RADIAL_SEGMENTS_PER_RADIAN vsStrokeArgs[0].x\n");
    }

    code.append(kLengthPow2Fn);
    append_wangs_formula_fn(&code, this->hasConics());
    code.append(kAtan2Fn);
    code.append(kCosineBetweenVectorsFn);
    code.append(kMiterExtentFn);
    code.append(R"(
    float cross2d(vec2 a, vec2 b) {
        return determinant(mat2(a,b));
    })");

    code.append(R"(
    in vec4 vsJoinArgs0[];
    in vec4 vsJoinArgs1[];
    in vec4 vsPts01[];
    in vec4 vsPts23[];
    in vec4 vsPts45[];
    in vec4 vsPts67[];
    in vec4 vsPts89[];
    in vec4 vsTans01[];
    in vec4 vsTans23[];)");
    if (this->hasDynamicStroke()) {
        code.append(R"(
        in vec2 vsStrokeArgs[];)");
    }
    if (this->hasDynamicColor()) {
        code.append(R"(
        in mediump vec4 vsColor[];)");
    }

    code.append(R"(
    out vec4 tcsPts01[];
    out vec4 tcsPt2Tan0[];
    out vec4 tcsTessArgs[];  // [numCombinedSegments, numParametricSegments, angle0, radsPerSegment]
    patch out vec4 tcsJoinArgs0; // [numSegmentsInJoin, innerJoinRadiusMultiplier,
                                 //  prevJoinTangent.xy]
    patch out vec4 tcsJoinArgs1;  // [joinAngle0, radsPerJoinSegment, joinOutsetClamp.xy]
    patch out vec4 tcsEndPtEndTan;)");
    if (this->hasDynamicStroke()) {
        code.append(R"(
        patch out float tcsStrokeRadius;)");
    }
    if (this->hasDynamicColor()) {
        code.append(R"(
        patch out mediump vec4 tcsColor;)");
    }

    code.append(R"(
    void main() {
        // Forward join args to the evaluation stage.
        tcsJoinArgs0 = vsJoinArgs0[0];
        tcsJoinArgs1 = vsJoinArgs1[0];)");
    if (this->hasDynamicStroke()) {
        code.append(R"(
        tcsStrokeRadius = vsStrokeArgs[0].y;)");
    }
    if (this->hasDynamicColor()) {
        code.append(R"(
        tcsColor = vsColor[0];)");
    }

    code.append(R"(
        // Unpack the curve args from the vertex shader.
        mat4x2 P;
        mat2 tangents;
        if (gl_InvocationID == 0) {
            // This is the first section of the curve.
            P = mat4x2(vsPts01[0], vsPts23[0]);
            tangents = mat2(vsTans01[0]);
        } else if (gl_InvocationID == 1) {
            // This is the middle section of the curve.
            P = mat4x2(vsPts23[0].zw, vsPts45[0], vsPts67[0].xy);
            tangents = mat2(vsTans01[0].zw, vsTans23[0].xy);
        } else {
            // This is the final section of the curve.
            P = mat4x2(vsPts67[0], vsPts89[0]);
            tangents = mat2(vsTans23[0]);
        }

        // Calculate the number of parametric segments. The final tessellated strip will be a
        // composition of these parametric segments as well as radial segments.
        float w = isinf(P[3].y) ? P[3].x : -1.0; // w<0 means the curve is an integral cubic.
        float numParametricSegments = wangs_formula(P, w, PARAMETRIC_INTOLERANCE);
        if (P[0] == P[1] && P[2] == P[3]) {
            // This is how the patch builder articulates lineTos but Wang's formula returns
            // >>1 segment in this scenario. Assign 1 parametric segment.
            numParametricSegments = 1.0;
        }

        // Determine the curve's start angle.
        float angle0 = atan2(tangents[0]);

        // Determine the curve's total rotation. The vertex shader ensures our curve does not rotate
        // more than 180 degrees or inflect, so the inverse cosine has enough range.
        float cosTheta = cosine_between_vectors(tangents[0], tangents[1]);
        float rotation = acos(cosTheta);

        // Adjust sign of rotation to match the direction the curve turns.
        // NOTE: Since the curve is not allowed to inflect, we can just check F'(.5) x F''(.5).
        // NOTE: F'(.5) x F''(.5) has the same sign as (P2 - P0) x (P3 - P1)
        float turn = isinf(P[3].y) ? cross2d(P[1] - P[0], P[2] - P[1])
                                   : cross2d(P[2] - P[0], P[3] - P[1]);
        if (turn == 0.0) {  // This is the case for joins and cusps where points are co-located.
            turn = determinant(tangents);
        }
        if (turn < 0.0) {
            rotation = -rotation;
        }

        // Calculate the number of evenly spaced radial segments to chop this section of the curve
        // into. Radial segments divide the curve's rotation into even steps. The final tessellated
        // strip will be a composition of both parametric and radial segments.
        float numRadialSegments = abs(rotation) * NUM_RADIAL_SEGMENTS_PER_RADIAN;
        numRadialSegments = max(ceil(numRadialSegments), 1.0);

        // The first and last edges are shared by both the parametric and radial sets of edges, so
        // the total number of edges is:
        //
        //   numCombinedEdges = numParametricEdges + numRadialEdges - 2
        //
        // It's also important to differentiate between the number of edges and segments in a strip:
        //
        //   numCombinedSegments = numCombinedEdges - 1
        //
        // So the total number of segments in the combined strip is:
        //
        //   numCombinedSegments = numParametricEdges + numRadialEdges - 2 - 1
        //                       = numParametricSegments + 1 + numRadialSegments + 1 - 2 - 1
        //                       = numParametricSegments + numRadialSegments - 1
        //
        float numCombinedSegments = numParametricSegments + numRadialSegments - 1.0;

        if (P[0] == P[3] && tangents[0] == tangents[1]) {
            // The vertex shader intentionally disabled our section. Set numCombinedSegments to 0.
            numCombinedSegments = 0.0;
        }

        // Pack the args for the evaluation stage.
        tcsPts01[gl_InvocationID] = vec4(P[0], P[1]);
        tcsPt2Tan0[gl_InvocationID] = vec4(P[2], tangents[0]);
        tcsTessArgs[gl_InvocationID] = vec4(numCombinedSegments, numParametricSegments, angle0,
                                            rotation / numRadialSegments);
        if (gl_InvocationID == 2) {
            tcsEndPtEndTan = vec4(P[3], tangents[1]);
        }

        barrier();

        // Tessellate a quad strip with enough segments for the join plus all 3 curve sections
        // combined.
        float numTotalCombinedSegments = tcsJoinArgs0.x + tcsTessArgs[0].x + tcsTessArgs[1].x +
                                         tcsTessArgs[2].x;

        if (tcsJoinArgs0.x != 0.0 && tcsJoinArgs0.x != numTotalCombinedSegments) {
            // We are tessellating a quad strip with both a single-sided join and a double-sided
            // stroke. Add one more edge to the join. This new edge will fall parallel with the
            // first edge of the stroke, eliminating artifacts on the transition from single
            // sided to double.
            ++tcsJoinArgs0.x;
            ++numTotalCombinedSegments;
        }

        numTotalCombinedSegments = min(numTotalCombinedSegments, MAX_TESSELLATION_SEGMENTS);
        gl_TessLevelInner[0] = numTotalCombinedSegments;
        gl_TessLevelInner[1] = 2.0;
        gl_TessLevelOuter[0] = 2.0;
        gl_TessLevelOuter[1] = numTotalCombinedSegments;
        gl_TessLevelOuter[2] = 2.0;
        gl_TessLevelOuter[3] = numTotalCombinedSegments;
    })");

    return code;
}

// Computes the location and tangent direction of the stroke edge with the integral id
// "combinedEdgeID", where combinedEdgeID is the sorted-order index of parametric and radial edges.
static void append_eval_stroke_edge_fn(SkString* code, bool hasConics) {
    code->append(R"(
    void eval_stroke_edge(in float4x2 P, in float w, in float numParametricSegments,
                          in float combinedEdgeID, in float2 tan0, in float radsPerSegment,
                          in float angle0, out float2 tangent, out float2 position) {
        // Start by finding the tangent function's power basis coefficients. These define a tangent
        // direction (scaled by some uniform value) as:
        //                                                 |T^2|
        //     Tangent_Direction(T) = dx,dy = |A  2B  C| * |T  |
        //                                    |.   .  .|   |1  |
        float2 A, B, C = P[1] - P[0];
        float2 D = P[3] - P[0];)");
    if (hasConics) {
        code->append(R"(
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
        } else {)");
    } else {
        code->append(R"(
        {)");
    }
    code->append(R"(
            float2 E = P[2] - P[1];
            B = E - C;
            A = fma(float2(-3), E, D);
        }

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
        float2 tan0norm = normalize(tan0);
        float negAbsRadsPerSegment = -abs(radsPerSegment);
        float maxRotation0 = (1.0 + combinedEdgeID) * abs(radsPerSegment);
        for (int exp = MAX_PARAMETRIC_SEGMENTS_LOG2 - 1; exp >= 0; --exp) {
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

        // Now that we've identified the highest parametric edge on or before the combinedEdgeID,
        // the highest radial edge is easy:
        float lastRadialEdgeID = combinedEdgeID - lastParametricEdgeID;

        // Find the tangent vector on the edge at lastRadialEdgeID.
        float radialAngle = fma(lastRadialEdgeID, radsPerSegment, angle0);
        tangent = float2(cos(radialAngle), sin(radialAngle));
        float2 norm = float2(-tangent.y, tangent.x);

        // Find the T value where the cubic's tangent is orthogonal to norm. This is a quadratic:
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
            // The root finder above can become unstable when lastRadialEdgeID == 0 (e.g., if there
            // are roots at exatly 0 and 1 both). radialT should always == 0 in this case.
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
        float2 abcd = unchecked_mix(abc, bcd, T);)");

    if (hasConics) {
        code->append(R"(
        // Evaluate the conic weights at T.
        float u = unchecked_mix(1.0, w, T);
        float v = unchecked_mix(w, 1.0, T);
        float uv = unchecked_mix(u, v, T);)");
    }

    code->appendf(R"(
        position =%s abcd;)", (hasConics) ? " (w >= 0.0) ? abc/uv :" : "");

    code->appendf(R"(
        // If we went with T=parametricT, then update the tangent. Otherwise leave it at the radial
        // tangent found previously. (In the event that parametricT == radialT, we keep the radial
        // tangent.)
        if (T != radialT) {)");
    code->appendf(R"(
            tangent =%s bcd - abc;)", (hasConics) ? " (w >= 0.0) ? bc*u - ab*v :" : "");
    code->appendf(R"(
        }
    })");
}

SkString GrStrokeTessellateShader::getTessEvaluationShaderGLSL(
        const GrGLSLPrimitiveProcessor* glslPrimProc, const char* versionAndExtensionDecls,
        const GrGLSLUniformHandler& uniformHandler, const GrShaderCaps& shaderCaps) const {
    SkASSERT(fMode == Mode::kTessellation);
    auto impl = static_cast<const GrStrokeTessellateShader::TessellationImpl*>(glslPrimProc);

    SkString code(versionAndExtensionDecls);
    code.append("layout(quads, equal_spacing, ccw) in;\n");
    code.appendf("precision highp float;\n");

    code.appendf("#define float2 vec2\n");
    code.appendf("#define float3 vec3\n");
    code.appendf("#define float4 vec4\n");
    code.appendf("#define float2x2 mat2\n");
    code.appendf("#define float3x2 mat3x2\n");
    code.appendf("#define float4x2 mat4x2\n");
    code.appendf("#define PI 3.141592653589793238\n");
    code.appendf("#define MAX_PARAMETRIC_SEGMENTS_LOG2 %i\n",
                 SkNextLog2(shaderCaps.maxTessellationSegments()));

    if (!this->hasDynamicStroke()) {
        const char* tessArgsName = impl->getTessArgsUniformName(uniformHandler);
        code.appendf("uniform vec4 %s;\n", tessArgsName);
        code.appendf("#define STROKE_RADIUS %s.w\n", tessArgsName);
    } else {
        code.appendf("#define STROKE_RADIUS tcsStrokeRadius\n");
    }

    if (!this->viewMatrix().isIdentity()) {
        const char* translateName = impl->getTranslateUniformName(uniformHandler);
        code.appendf("uniform vec2 %s;\n", translateName);
        code.appendf("#define TRANSLATE %s\n", translateName);
        if (!fStroke.isHairlineStyle()) {
            // In the normal case we need the affine matrix too. (In the hairline case we already
            // applied the affine matrix in the vertex shader.)
            const char* affineMatrixName = impl->getAffineMatrixUniformName(uniformHandler);
            code.appendf("uniform vec4 %s;\n", affineMatrixName);
            code.appendf("#define AFFINE_MATRIX mat2(%s)\n", affineMatrixName);
        }
    }

    code.append(R"(
    in vec4 tcsPts01[];
    in vec4 tcsPt2Tan0[];
    in vec4 tcsTessArgs[];  // [numCombinedSegments, numParametricSegments, angle0, radsPerSegment]
    patch in vec4 tcsJoinArgs0;  // [numSegmentsInJoin, innerJoinRadiusMultiplier,
                                 //  prevJoinTangent.xy]
    patch in vec4 tcsJoinArgs1;  // [joinAngle0, radsPerJoinSegment, joinOutsetClamp.xy]
    patch in vec4 tcsEndPtEndTan;)");
    if (this->hasDynamicStroke()) {
        code.append(R"(
        patch in float tcsStrokeRadius;)");
    }
    if (this->hasDynamicColor()) {
        code.appendf(R"(
        patch in mediump vec4 tcsColor;
        %s out mediump vec4 tesColor;)", shaderCaps.preferFlatInterpolation() ? "flat" : "");
    }

    code.append(R"(
    uniform vec4 sk_RTAdjust;)");

    code.append(kUncheckedMixFn);
    append_eval_stroke_edge_fn(&code, this->hasConics());

    code.append(R"(
    void main() {
        // Our patch is composed of exactly "numTotalCombinedSegments + 1" stroke-width edges that
        // run orthogonal to the curve and make a strip of "numTotalCombinedSegments" quads.
        // Determine which discrete edge belongs to this invocation. An edge can either come from a
        // parametric segment or a radial one.
        float numSegmentsInJoin = tcsJoinArgs0.x;
        float numTotalCombinedSegments = numSegmentsInJoin + tcsTessArgs[0].x + tcsTessArgs[1].x +
                                         tcsTessArgs[2].x;
        float totalEdgeID = round(gl_TessCoord.x * numTotalCombinedSegments);

        // Furthermore, the vertex shader may have chopped the curve into 3 different sections.
        // Determine which section we belong to, and where we fall relative to its first edge.
        float localEdgeID = totalEdgeID;
        mat4x2 P;
        vec2 tan0;
        vec3 tessellationArgs;
        float strokeRadius = STROKE_RADIUS;
        vec2 strokeOutsetClamp = vec2(-1, 1);
        if (localEdgeID < numSegmentsInJoin || numSegmentsInJoin == numTotalCombinedSegments) {
            // Our edge belongs to the join preceding the curve.
            P = mat4x2(tcsPts01[0].xyxy, tcsPts01[0].xyxy);
            tan0 = tcsJoinArgs0.zw;
            tessellationArgs = vec3(1, tcsJoinArgs1.xy);
            strokeRadius *= (localEdgeID == 1.0) ? tcsJoinArgs0.y : 1.0;
            strokeOutsetClamp = tcsJoinArgs1.zw;
        } else if ((localEdgeID -= numSegmentsInJoin) < tcsTessArgs[0].x) {
            // Our edge belongs to the first curve section.
            P = mat4x2(tcsPts01[0], tcsPt2Tan0[0].xy, tcsPts01[1].xy);
            tan0 = tcsPt2Tan0[0].zw;
            tessellationArgs = tcsTessArgs[0].yzw;
        } else if ((localEdgeID -= tcsTessArgs[0].x) < tcsTessArgs[1].x) {
            // Our edge belongs to the second curve section.
            P = mat4x2(tcsPts01[1], tcsPt2Tan0[1].xy, tcsPts01[2].xy);
            tan0 = tcsPt2Tan0[1].zw;
            tessellationArgs = tcsTessArgs[1].yzw;
        } else {
            // Our edge belongs to the third curve section.
            localEdgeID -= tcsTessArgs[1].x;
            P = mat4x2(tcsPts01[2], tcsPt2Tan0[2].xy, tcsEndPtEndTan.xy);
            tan0 = tcsPt2Tan0[2].zw;
            tessellationArgs = tcsTessArgs[2].yzw;
        }
        float numParametricSegments = tessellationArgs.x;
        float angle0 = tessellationArgs.y;
        float radsPerSegment = tessellationArgs.z;

        float w = -1.0;  // w<0 means the curve is an integral cubic.)");

    if (this->hasConics()) {
        code.append(R"(
        if (isinf(P[3].y)) {
            w = P[3].x;  // The curve is actually a conic.
            P[3] = P[2];  // Setting p3 equal to p2 works for the remaining rotational logic.
        })");
    }

    code.append(R"(
        float2 tangent, position;
        eval_stroke_edge(P, w, numParametricSegments, localEdgeID, tan0, radsPerSegment, angle0,
                         tangent, position);

        if (localEdgeID == 0.0) {
            // The first local edge of each section uses the provided tan0. This ensures continuous
            // rotation across chops made by the vertex shader as well as crack-free seaming between
            // patches. (NOTE: position is always equal to P[0] here when localEdgeID==0.)
            tangent = tan0;
        }

        if (gl_TessCoord.x == 1.0) {
            // The final edge of the quad strip always uses the provided endPt and endTan. This
            // ensures crack-free seaming between patches.
            tangent = tcsEndPtEndTan.zw;
            position = P[3];
        }

        // Determine how far to outset our vertex orthogonally from the curve.
        float outset = gl_TessCoord.y * 2.0 - 1.0;
        outset = clamp(outset, strokeOutsetClamp.x, strokeOutsetClamp.y);
        outset *= strokeRadius;

        vec2 vertexPos = position + normalize(vec2(-tangent.y, tangent.x)) * outset;)");

    if (!this->viewMatrix().isIdentity()) {
        if (!fStroke.isHairlineStyle()) {
            // Normal case. Do the transform after tessellation.
            code.append("vertexPos = AFFINE_MATRIX * vertexPos + TRANSLATE;");
        } else {
            // Hairline case. The scale and skew already happened before tessellation.
            code.append("vertexPos = vertexPos + TRANSLATE;");
        }
    }

    code.append(R"(
        gl_Position = vec4(vertexPos * sk_RTAdjust.xz + sk_RTAdjust.yw, 0.0, 1.0);)");

    if (this->hasDynamicColor()) {
        code.append(R"(
        tesColor = tcsColor;)");
    }

    code.append(R"(
    })");

    return code;
}

class GrStrokeTessellateShader::IndirectImpl : public GrGLSLGeometryProcessor {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& shader = args.fGP.cast<GrStrokeTessellateShader>();
        SkPaint::Join joinType = shader.fStroke.getJoin();
        args.fVaryingHandler->emitAttributes(shader);

        // Constants.
        args.fVertBuilder->defineConstant("MAX_PARAMETRIC_SEGMENTS_LOG2",
                                          GrTessellationPathRenderer::kMaxResolveLevel);
        args.fVertBuilder->defineConstant("float", "PI", "3.141592653589793238");

        // Helper functions.
        if (shader.hasDynamicStroke()) {
            args.fVertBuilder->insertFunction(kNumRadialSegmentsPerRadian);
        }
        args.fVertBuilder->insertFunction(kAtan2Fn);
        args.fVertBuilder->insertFunction(kLengthPow2Fn);
        args.fVertBuilder->insertFunction(kMiterExtentFn);
        args.fVertBuilder->insertFunction(kUncheckedMixFn);
        args.fVertBuilder->insertFunction(kCosineBetweenVectorsFn);
        append_wangs_formula_fn(&args.fVertBuilder->functions(), shader.hasConics());
        append_eval_stroke_edge_fn(&args.fVertBuilder->functions(), shader.hasConics());

        // Tessellation control uniforms and/or dynamic attributes.
        if (!shader.hasDynamicStroke()) {
            // [PARAMETRIC_INTOLERANCE, NUM_RADIAL_SEGMENTS_PER_RADIAN, JOIN_TYPE, STROKE_RADIUS]
            const char* tessArgsName;
            fTessControlArgsUniform = args.fUniformHandler->addUniform(
                    nullptr, kVertex_GrShaderFlag, kFloat4_GrSLType, "tessControlArgs",
                    &tessArgsName);
            args.fVertBuilder->codeAppendf(R"(
            float PARAMETRIC_INTOLERANCE = %s.x;
            float NUM_RADIAL_SEGMENTS_PER_RADIAN = %s.y;
            float JOIN_TYPE = %s.z;
            float STROKE_RADIUS = %s.w;)", tessArgsName, tessArgsName, tessArgsName, tessArgsName);
        } else {
            const char* parametricIntoleranceName;
            fTessControlArgsUniform = args.fUniformHandler->addUniform(
                    nullptr, kVertex_GrShaderFlag, kFloat_GrSLType, "parametricIntolerance",
                    &parametricIntoleranceName);
            args.fVertBuilder->codeAppendf(R"(
            float PARAMETRIC_INTOLERANCE = %s;
            float STROKE_RADIUS = dynamicStrokeAttr.x;
            float NUM_RADIAL_SEGMENTS_PER_RADIAN = num_radial_segments_per_radian(
                    PARAMETRIC_INTOLERANCE, STROKE_RADIUS);
            float JOIN_TYPE = dynamicStrokeAttr.y;)", parametricIntoleranceName);
        }

        // View matrix uniforms.
        if (!shader.viewMatrix().isIdentity()) {
            const char* translateName, *affineMatrixName;
            fAffineMatrixUniform = args.fUniformHandler->addUniform(
                    nullptr, kVertex_GrShaderFlag, kFloat4_GrSLType, "affineMatrix",
                    &affineMatrixName);
            fTranslateUniform = args.fUniformHandler->addUniform(
                    nullptr, kVertex_GrShaderFlag, kFloat2_GrSLType, "translate", &translateName);
            args.fVertBuilder->codeAppendf("float2x2 AFFINE_MATRIX = float2x2(%s);\n",
                                           affineMatrixName);
            args.fVertBuilder->codeAppendf("float2 TRANSLATE = %s;\n", translateName);
        }

        // Tessellation code.
        args.fVertBuilder->codeAppend(R"(
        float4x2 P = float4x2(pts01Attr, pts23Attr);
        float2 lastControlPoint = argsAttr.xy;
        float w = -1;  // w<0 means the curve is an integral cubic.)");
        if (shader.hasConics()) {
            args.fVertBuilder->codeAppend(R"(
            if (isinf(P[3].y)) {
                w = P[3].x;  // The curve is actually a conic.
                P[3] = P[2];  // Setting p3 equal to p2 works for the remaining rotational logic.
            })");
        }
        if (shader.fStroke.isHairlineStyle() && !shader.viewMatrix().isIdentity()) {
            // Hairline case. Transform the points before tessellation. We can still hold off on the
            // translate until the end; we just need to perform the scale and skew right now.
            args.fVertBuilder->codeAppend(R"(
            P = AFFINE_MATRIX * P;
            lastControlPoint = AFFINE_MATRIX * lastControlPoint;)");
        }
        args.fVertBuilder->codeAppend(R"(
        float numTotalEdges = abs(argsAttr.z);

        // Find how many parametric segments this stroke requires.
        float numParametricSegments = min(wangs_formula(P, w, PARAMETRIC_INTOLERANCE),
                                          float(1 << MAX_PARAMETRIC_SEGMENTS_LOG2));
        if (P[0] == P[1] && P[2] == P[3]) {
            // This is how we describe lines, but Wang's formula does not return 1 in this case.
            numParametricSegments = 1;
        }

        // Find the starting and ending tangents.
        float2 tan0 = ((P[0] == P[1]) ? (P[1] == P[2]) ? P[3] : P[2] : P[1]) - P[0];
        float2 tan1 = P[3] - ((P[3] == P[2]) ? (P[2] == P[1]) ? P[0] : P[1] : P[2]);
        if (tan0 == float2(0)) {
            // The stroke is a point. This special case tells us to draw a stroke-width circle as a
            // 180 degree point stroke instead.
            tan0 = float2(1,0);
            tan1 = float2(-1,0);
        })");

        if (shader.fStroke.getJoin() == SkPaint::kRound_Join || shader.hasDynamicStroke()) {
            args.fVertBuilder->codeAppend(R"(
            // Determine how many edges to give to the round join. We emit the first and final edges
            // of the join twice: once full width and once restricted to half width. This guarantees
            // perfect seaming by matching the vertices from the join as well as from the strokes on
            // either side.
            float joinRads = acos(cosine_between_vectors(P[0] - lastControlPoint, tan0));
            float numRadialSegmentsInJoin = max(ceil(joinRads * NUM_RADIAL_SEGMENTS_PER_RADIAN), 1);
            // +2 because we emit the beginning and ending edges twice (see above comment).
            float numEdgesInJoin = numRadialSegmentsInJoin + 2;
            // The stroke section needs at least two edges. Don't assign more to the join than
            // "numTotalEdges - 2".
            numEdgesInJoin = min(numEdgesInJoin, numTotalEdges - 2);
            // Lines give all their extra edges to the join.
            if (numParametricSegments == 1) {
                numEdgesInJoin = numTotalEdges - 2;
            }
            // Negative argsAttr.z means the join is a chop, and chop joins get exactly one segment.
            if (argsAttr.z < 0) {
                // +2 because we emit the beginning and ending edges twice (see above comment).
                numEdgesInJoin = 1 + 2;
            })");
            if (shader.hasDynamicStroke()) {
                args.fVertBuilder->codeAppend(R"(
                if (JOIN_TYPE >= 0 /*Is the join not a round type?*/) {
                    // Bevel and miter joins get 1 and 2 segments respectively.
                    // +2 because we emit the beginning and ending edges twice (see above comments).
                    numEdgesInJoin = sign(JOIN_TYPE) + 1 + 2;
                })");
            }
        } else {
            args.fVertBuilder->codeAppendf(R"(
            float numEdgesInJoin = %i;)", NumExtraEdgesInIndirectJoin(joinType));
        }

        args.fVertBuilder->codeAppend(R"(
        // Find which direction the curve turns.
        // NOTE: Since the curve is not allowed to inflect, we can just check F'(.5) x F''(.5).
        // NOTE: F'(.5) x F''(.5) has the same sign as (P2 - P0) x (P3 - P1)
        float turn = cross(P[2] - P[0], P[3] - P[1]);

        float numCombinedSegments;
        float outset = ((sk_VertexID & 1) == 0) ? +1 : -1;
        float combinedEdgeID = float(sk_VertexID >> 1) - numEdgesInJoin;
        if (combinedEdgeID < 0) {
            // We belong to the preceding join. The first and final edges get duplicated, so we only
            // have "numEdgesInJoin - 2" segments.
            numCombinedSegments = numEdgesInJoin - 2;
            numParametricSegments = 1;  // Joins don't have parametric segments.
            P = float4x2(P[0], P[0], P[0], P[0]);  // Colocate all points on the junction point.
            tan1 = tan0;
            // Don't let tan0 become zero. The code as-is isn't built to handle that case. tan0=0
            // means the join is disabled, and to disable it with the existing code we can leave
            // tan0 equal to tan1.
            if (lastControlPoint != P[0]) {
                tan0 = P[0] - lastControlPoint;
            }
            turn = cross(tan0, tan1);
            // Shift combinedEdgeID to the range [-1, numCombinedSegments]. This duplicates the
            // first edge and lands one edge at the very end of the join. (The duplicated final edge
            // will actually come from the section of our strip that belongs to the stroke.)
            combinedEdgeID += numCombinedSegments + 1;
            // We normally restrict the join on one side of the junction, but if the tangents are
            // nearly equivalent this could theoretically result in bad seaming and/or cracks on the
            // side we don't put it on. If the tangents are nearly equivalent then we leave the join
            // double-sided.
            float sinEpsilon = 1e-2;  // ~= sin(180deg / 3000)
            bool tangentsNearlyParallel =
                    (abs(turn) * inversesqrt(dot(tan0, tan0) * dot(tan1, tan1))) < sinEpsilon;
            if (!tangentsNearlyParallel || dot(tan0, tan1) < 0) {
                // There are two edges colocated at the beginning. Leave the first one double sided
                // for seaming with the previous stroke. (The double sided edge at the end will
                // actually come from the section of our strip that belongs to the stroke.)
                if (combinedEdgeID >= 0) {
                    outset = (turn < 0) ? min(outset, 0) : max(outset, 0);
                }
            }
            combinedEdgeID = max(combinedEdgeID, 0);
        } else {
            // We belong to the stroke.
            numCombinedSegments = numTotalEdges - numEdgesInJoin - 1;
        }

        // Don't take more parametric segments than there are total segments.
        numParametricSegments = min(numParametricSegments, numCombinedSegments);

        // Any leftover edges go to radial segments.
        float numRadialSegments = numCombinedSegments + 1 - numParametricSegments;

        // Calculate the curve's starting angle and rotation.
        float angle0 = atan2(tan0);
        float cosTheta = cosine_between_vectors(tan0, tan1);
        float rotation = acos(cosTheta);
        if (turn < 0) {
            // Adjust sign of rotation to match the direction the curve turns.
            rotation = -rotation;
        }
        float radsPerSegment = rotation / numRadialSegments;)");

        if (joinType == SkPaint::kMiter_Join || shader.hasDynamicStroke()) {
            args.fVertBuilder->codeAppendf(R"(
            // Vertices #4 and #5 belong to the edge of the join that extends to the miter point.
            if ((sk_VertexID | 1) == (4 | 5) && %s) {
                outset *= miter_extent(cosTheta, JOIN_TYPE/*miterLimit*/);
            })", shader.hasDynamicStroke() ? "JOIN_TYPE > 0/*Is the join a miter type?*/" : "true");
        }

        args.fVertBuilder->codeAppendf(R"(
        float2 tangent, strokeCoord;
        eval_stroke_edge(P, w, numParametricSegments, combinedEdgeID, tan0, radsPerSegment, angle0,
                         tangent, strokeCoord);)");

        args.fVertBuilder->codeAppend(R"(
        if (combinedEdgeID == 0) {
            // Edges at the beginning of their section use P[0] and tan0. This ensures crack-free
            // seaming between instances.
            strokeCoord = P[0];
            tangent = tan0;
        }

        if (combinedEdgeID == numCombinedSegments) {
            // Edges at the end of their section use P[1] and tan1. This ensures crack-free seaming
            // between instances.
            strokeCoord = P[3];
            tangent = tan1;
        }

        float2 ortho = normalize(float2(tangent.y, -tangent.x));
        strokeCoord += ortho * (STROKE_RADIUS * outset);)");

        if (shader.viewMatrix().isIdentity()) {
            // No transform matrix.
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "strokeCoord");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "strokeCoord");
        } else if (!shader.fStroke.isHairlineStyle()) {
            // Normal case. Do the transform after tessellation.
            args.fVertBuilder->codeAppend(R"(
            float2 devCoord = AFFINE_MATRIX * strokeCoord + TRANSLATE;)");
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "devCoord");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "strokeCoord");
        } else {
            // Hairline case. The scale and skew already happened before tessellation.
            args.fVertBuilder->codeAppend(R"(
            float2 devCoord = strokeCoord + TRANSLATE;
            float2 localCoord = inverse(AFFINE_MATRIX) * strokeCoord;)");
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "devCoord");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localCoord");
        }

        if (!shader.hasDynamicColor()) {
            // The fragment shader just outputs a uniform color.
            const char* colorUniformName;
            fColorUniform = args.fUniformHandler->addUniform(
                    nullptr, kFragment_GrShaderFlag, kHalf4_GrSLType, "color", &colorUniformName);
            args.fFragBuilder->codeAppendf("half4 %s = %s;", args.fOutputColor, colorUniformName);
        } else {
            // Color gets passed in through an instance attrib.
            args.fFragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
            args.fVaryingHandler->addPassThroughAttribute(
                    shader.fAttribs.back(), args.fOutputColor,
                    GrGLSLVaryingHandler::Interpolation::kCanBeFlat);
        }
        args.fFragBuilder->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrPrimitiveProcessor& primProc) override {
        const auto& shader = primProc.cast<GrStrokeTessellateShader>();
        const auto& stroke = shader.fStroke;

        if (!shader.hasDynamicStroke()) {
            // Set up the tessellation control uniforms.
            Tolerances tolerances;
            if (!stroke.isHairlineStyle()) {
                tolerances = Tolerances::MakeNonHairline(shader.viewMatrix().getMaxScale(),
                                                         stroke.getWidth());
            } else {
                // In the hairline case we transform prior to tessellation. Set up tolerances for an
                // identity viewMatrix and a strokeWidth of 1.
                tolerances = Tolerances::MakeNonHairline(1, 1);
            }
            float strokeRadius = (stroke.isHairlineStyle()) ? .5f : stroke.getWidth() * .5;
            pdman.set4f(fTessControlArgsUniform,
                        tolerances.fParametricIntolerance,  // PARAMETRIC_INTOLERANCE
                        tolerances.fNumRadialSegmentsPerRadian,  // NUM_RADIAL_SEGMENTS_PER_RADIAN
                        GetJoinType(shader.fStroke),  // JOIN_TYPE
                        strokeRadius);  // STROKE_RADIUS
        } else {
            SkASSERT(!stroke.isHairlineStyle());
            pdman.set1f(fTessControlArgsUniform,
                        Tolerances::CalcParametricIntolerance(shader.viewMatrix().getMaxScale()));
        }

        // Set up the view matrix, if any.
        const SkMatrix& m = shader.viewMatrix();
        if (!m.isIdentity()) {
            pdman.set2f(fTranslateUniform, m.getTranslateX(), m.getTranslateY());
            pdman.set4f(fAffineMatrixUniform, m.getScaleX(), m.getSkewY(), m.getSkewX(),
                        m.getScaleY());
        }

        if (!shader.hasDynamicColor()) {
            pdman.set4fv(fColorUniform, 1, shader.fColor.vec());
        }
    }

    GrGLSLUniformHandler::UniformHandle fTessControlArgsUniform;
    GrGLSLUniformHandler::UniformHandle fTranslateUniform;
    GrGLSLUniformHandler::UniformHandle fAffineMatrixUniform;
    GrGLSLUniformHandler::UniformHandle fColorUniform;
};

void GrStrokeTessellateShader::getGLSLProcessorKey(const GrShaderCaps&,
                                                   GrProcessorKeyBuilder* b) const {
    bool keyNeedsJoin = (fMode == Mode::kIndirect) && !(fShaderFlags & ShaderFlags::kDynamicStroke);
    SkASSERT(fStroke.getJoin() >> 2 == 0);
    // Attribs get worked into the key automatically during GrPrimitiveProcessor::getAttributeKey().
    // When color is in a uniform, it's always wide. kWideColor doesn't need to be considered here.
    uint32_t key = (uint32_t)(fShaderFlags & ~ShaderFlags::kWideColor);
    key = (key << 1) | (uint32_t)fMode;
    key = (key << 2) | ((keyNeedsJoin) ? fStroke.getJoin() : 0);
    key = (key << 1) | (uint32_t)fStroke.isHairlineStyle();
    key = (key << 1) | (uint32_t)this->viewMatrix().isIdentity();
    b->add32(key);
}

GrGLSLPrimitiveProcessor* GrStrokeTessellateShader::createGLSLInstance(
        const GrShaderCaps&) const {
    return (fMode == Mode::kTessellation) ?
            (GrGLSLPrimitiveProcessor*)new TessellationImpl : new IndirectImpl;
}

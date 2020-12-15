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

class GrStrokeTessellateShader::TessellationImpl : public GrGLSLGeometryProcessor {
public:
    const char* getTessArgs1UniformName(const GrGLSLUniformHandler& uniformHandler) const {
        return uniformHandler.getUniformCStr(fTessArgs1Uniform);
    }
    const char* getTessArgs2UniformName(const GrGLSLUniformHandler& uniformHandler) const {
        return uniformHandler.getUniformCStr(fTessArgs2Uniform);
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

        SkASSERT(!shader.fHasConics);

        args.fVaryingHandler->emitAttributes(shader);

        // uNumSegmentsInJoin, uWangsTermPow2, uNumRadialSegmentsPerRadian, uMiterLimitInvPow2.
        fTessArgs1Uniform = uniHandler->addUniform(nullptr, kTessControl_GrShaderFlag,
                                                   kFloat4_GrSLType, "tessArgs1", nullptr);
        // uJoinTolerancePow2, uStrokeRadius.
        fTessArgs2Uniform = uniHandler->addUniform(nullptr, kTessControl_GrShaderFlag |
                                                            kTessEvaluation_GrShaderFlag,
                                                   kFloat2_GrSLType,
                                                   "tessArgs2", nullptr);
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
                v->codeAppendf("float2x2 uAffineMatrix = float2x2(%s);\n", affineMatrixName);
            }
        }
        const char* colorUniformName;
        fColorUniform = uniHandler->addUniform(nullptr, kFragment_GrShaderFlag, kHalf4_GrSLType,
                                               "color", &colorUniformName);

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
        v->declareGlobal(GrShaderVar("vsPts01", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsPts23", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsPts45", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsPts67", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsPts89", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsTans01", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsTans23", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsPrevJoinTangent", kFloat2_GrSLType, TypeModifier::Out));

        // Unlike mix(), this does not return b when t==1. But it otherwise seems to get better
        // precision than "a*(1 - t) + b*t" for things like chopping cubics on exact cusp points.
        // The responsibility falls on the caller to ensure t != 1 before calling.
        v->insertFunction(R"(
        float4 unchecked_mix(float4 a, float4 b, float4 t) {
            return fma(b - a, t, a);
        })");

        v->codeAppendf(R"(
        // Unpack the control points.
        float4x2 P = float4x2(inputPts01, inputPts23);
        float2 prevControlPoint = inputPrevCtrlPt;)");
        if (shader.fStroke.isHairlineStyle() && !shader.viewMatrix().isIdentity()) {
            // Hairline case. Transform the points before tessellation. We can still hold off on the
            // translate until the end; we just need to perform the scale and skew right now.
            v->codeAppend(R"(
            P = uAffineMatrix * P;
            prevControlPoint = uAffineMatrix * prevControlPoint;)");
        }
        v->codeAppendf(R"(
        float2 prevJoinTangent = P[0] - prevControlPoint;

        // Find the beginning and ending tangents. It's imperative that we compute these tangents
        // form the original input points or else the seams might crack.
        float2 tan0 = (P[1] == P[0]) ? P[2] - P[0] : P[1] - P[0];
        float2 tan1 = (P[3] == P[2]) ? P[3] - P[1] : P[3] - P[2];

        if (tan1 == float2(0)) {
            // [p0, p3, p3, p3] is a reserved pattern that means this patch is a join only.
            P[1] = P[2] = P[3] = P[0];  // Colocate all the curve's points.
            // This will disable the (co-located) curve sections by making their tangents equal.
            tan1 = tan0;
        }

        if (tan0 == float2(0)) {
            // [p0, p0, p0, p3] is a reserved pattern that means this patch is a cusp point.
            P[3] = P[0];  // Colocate all the points on the cusp.
            // This will disable the join section by making its tangents equal.
            tan0 = prevJoinTangent;
        }

        // Start by finding the cubic's power basis coefficients. These define the bezier curve as:
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
        if (float3(a,b,c) == float3(0)) {
            // The curve is a flat line. Search for turnaround cusp points instead. (These are the
            // points where the tangent is perpendicular to tan0.)
            //
            //     dot(tan0, Tangent_Direction(T)) == 0
            //
            //                         |T^2|
            //     tan0 * |A  2B  C| * |T  | == 0
            //            |.   .  .|   |1  |
            //
            float3 coeffs = tan0 * float3x2(A,B,C);
            a = coeffs.x;
            b_over_2 = coeffs.y;
            c = coeffs.z;
            discr_over_4 = max(b_over_2*b_over_2 - a*c, 0);
            innerTangents = float2x2(-tan0, -tan0);
        } else if (discr_over_4 <= 0) {
            // The curve does not inflect. This means it might rotate more than 180 degrees instead.
            // Craft a quadratic whose roots are the points were rotation == 180 deg and 0. (These
            // are the points where the tangent is parallel to tan0.)
            //
            //      Tangent_Direction(T) x tan0 == 0
            //      (AT^2 x tan0) + (2BT x tan0) + (C x tan0) == 0
            //      (A x C)T^2 + (2B x C)T + (C x C) == 0  [[because tan0 == P1 - P0 == C]]
            //      bT^2 + 2c + 0 == 0  [[because A x C == b, B x C == c]]
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

        // If the curve is a straight line or point, don't chop it into sections after all.
        if (P[0] == P[1] && P[2] == P[3]) {
            chopT = float2(0);
            innerTangents = float2x2(tan0, tan0);
        }

        // Chop the curve at chopT[0] and chopT[1].
        float4 ab = unchecked_mix(P[0].xyxy, P[1].xyxy, chopT.sstt);
        float4 bc = unchecked_mix(P[1].xyxy, P[2].xyxy, chopT.sstt);
        float4 cd = unchecked_mix(P[2].xyxy, P[3].xyxy, chopT.sstt);
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

        // Package arguments for the tessellation control stage.
        vsPts01 = float4(P[0], ab.xy);
        vsPts23 = float4(abc.xy, abcd.xy);
        vsPts45 = middle;
        vsPts67 = float4(abcd.zw, bcd.zw);
        vsPts89 = float4(cd.zw, P[3]);
        vsTans01 = float4(tan0, innerTangents[0]);
        vsTans23 = float4(innerTangents[1], tan1);
        vsPrevJoinTangent = (prevJoinTangent == float2(0)) ? tan0 : prevJoinTangent;
        )");

        // The fragment shader just outputs a uniform color.
        args.fFragBuilder->codeAppendf("%s = %s;", args.fOutputColor, colorUniformName);
        args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrPrimitiveProcessor& primProc) override {
        const auto& shader = primProc.cast<GrStrokeTessellateShader>();
        const auto& stroke = shader.fStroke;
        float numSegmentsInJoin;
        switch (stroke.getJoin()) {
            case SkPaint::kBevel_Join:
                numSegmentsInJoin = 1;
                break;
            case SkPaint::kMiter_Join:
                numSegmentsInJoin = (stroke.getMiter() > 0) ? 2 : 1;
                break;
            case SkPaint::kRound_Join:
                numSegmentsInJoin = 0;  // Use the rotation to calculate the number of segments.
                break;
        }
        Tolerances tolerances;
        if (!stroke.isHairlineStyle()) {
            tolerances.set(shader.viewMatrix().getMaxScale(), stroke.getWidth());
        } else {
            // In the hairline case we transform prior to tessellation. Set up tolerances for an
            // identity viewMatrix and a strokeWidth of 1.
            tolerances.set(1, 1);
        }
        float miterLimit = shader.fStroke.getMiter();
        pdman.set4f(fTessArgs1Uniform,
                    numSegmentsInJoin,  // uNumSegmentsInJoin
                    GrWangsFormula::length_term_pow2<3>(
                            tolerances.fParametricIntolerance),  // uWangsTermPow2
                    tolerances.fNumRadialSegmentsPerRadian,  // uNumRadialSegmentsPerRadian
                    1 / (miterLimit * miterLimit));  // uMiterLimitInvPow2
        float strokeRadius = (stroke.isHairlineStyle()) ? .5f : stroke.getWidth() * .5;
        float joinTolerance = 1 / (strokeRadius * tolerances.fParametricIntolerance);
        pdman.set2f(fTessArgs2Uniform,
                    joinTolerance * joinTolerance,  // uJoinTolerancePow2
                    strokeRadius);  // uStrokeRadius
        const SkMatrix& m = shader.viewMatrix();
        if (!m.isIdentity()) {
            pdman.set2f(fTranslateUniform, m.getTranslateX(), m.getTranslateY());
            pdman.set4f(fAffineMatrixUniform, m.getScaleX(), m.getSkewY(), m.getSkewX(),
                        m.getScaleY());
        }
        pdman.set4fv(fColorUniform, 1, shader.fColor.vec());
    }

    GrGLSLUniformHandler::UniformHandle fTessArgs1Uniform;
    GrGLSLUniformHandler::UniformHandle fTessArgs2Uniform;
    GrGLSLUniformHandler::UniformHandle fTranslateUniform;
    GrGLSLUniformHandler::UniformHandle fAffineMatrixUniform;
    GrGLSLUniformHandler::UniformHandle fColorUniform;
};

// The built-in atan() is undefined when x==0. This method relieves that restriction, but also can
// return values larger than 2*PI. This shouldn't matter for our purposes.
static const char* kAtan2Fn = R"(
float atan2(float2 v) {
    float bias = 0;
    if (abs(v.y) > abs(v.x)) {
        v = float2(v.y, -v.x);
        bias = PI/2;
    }
    return atan(v.y, v.x) + bias;
})";

static const char* kLengthPow2Fn = R"(
float length_pow2(float2 v) {
    return dot(v, v);
})";

// Calculates the number of evenly spaced (in the parametric sense) segments to chop a cubic into.
// (See GrWangsFormula::cubic() for more documentation on this formula.) The final tessellated strip
// will be a composition of these parametric segments as well as radial segments.
static const char* kWangsFormulaCubicFn = R"(
float wangs_formula_cubic(float4x2 P, float wangsTermPow2) {
    float m = max(length_pow2(fma(float2(-2), P[1], P[2]) + P[0]),
                  length_pow2(fma(float2(-2), P[2], P[3]) + P[1])) * wangsTermPow2;
    return ceil(inversesqrt(inversesqrt(max(m, 1e-2))));
})";

// Extends the middle radius to either the miter point, or the bevel edge if we surpassed the miter
// limit and need to revert to a bevel join.
static const char* kMiterExtentFn = R"(
float miter_extent(float cosTheta, float miterLimitInvPow2) {
    float x = fma(cosTheta, .5, .5);
    return (x >= miterLimitInvPow2) ? inversesqrt(x) : sqrt(x);
})";

SkString GrStrokeTessellateShader::getTessControlShaderGLSL(
        const GrGLSLPrimitiveProcessor* glslPrimProc, const char* versionAndExtensionDecls,
        const GrGLSLUniformHandler& uniformHandler, const GrShaderCaps& shaderCaps) const {
    SkASSERT(fMode == Mode::kTessellation);
    auto impl = static_cast<const GrStrokeTessellateShader::TessellationImpl*>(glslPrimProc);

    SkString code(versionAndExtensionDecls);
    // Run 4 invocations: 1 for the previous join plus 1 for each section that the vertex shader
    // chopped the curve into.
    code.append("layout(vertices = 4) out;\n");
    code.appendf("precision highp float;\n");

    code.appendf("#define float2 vec2\n");
    code.appendf("#define float3 vec3\n");
    code.appendf("#define float4 vec4\n");
    code.appendf("#define float2x2 mat2\n");
    code.appendf("#define float4x2 mat4x2\n");
    code.appendf("#define PI 3.141592653589793238\n");
    code.appendf("#define MAX_TESSELLATION_SEGMENTS %i\n",
                 shaderCaps.maxTessellationSegments());

    const char* tessArgs1Name = impl->getTessArgs1UniformName(uniformHandler);
    code.appendf("uniform vec4 %s;\n", tessArgs1Name);
    code.appendf("#define uNumSegmentsInJoin %s.x\n", tessArgs1Name);
    code.appendf("#define uWangsTermPow2 %s.y\n", tessArgs1Name);
    code.appendf("#define uNumRadialSegmentsPerRadian %s.z\n", tessArgs1Name);
    code.appendf("#define uMiterLimitInvPow2 %s.w\n", tessArgs1Name);

    const char* tessArgs2Name = impl->getTessArgs2UniformName(uniformHandler);
    code.appendf("uniform vec2 %s;\n", tessArgs2Name);
    code.appendf("#define uJoinTolerancePow2 %s.x\n", tessArgs2Name);

    code.append(R"(
    in vec4 vsPts01[];
    in vec4 vsPts23[];
    in vec4 vsPts45[];
    in vec4 vsPts67[];
    in vec4 vsPts89[];
    in vec4 vsTans01[];
    in vec4 vsTans23[];
    in vec2 vsPrevJoinTangent[];

    out vec4 tcsPts01[];
    out vec4 tcsPt2Tan0[];
    out vec4 tcsTessArgs[];
    patch out vec4 tcsEndPtEndTan;
    patch out vec3 tcsJoinArgs;

    float cross(vec2 a, vec2 b) {
        return determinant(mat2(a,b));
    })");

    code.append(kAtan2Fn);
    code.append(kLengthPow2Fn);
    code.append(kWangsFormulaCubicFn);
    code.append(kMiterExtentFn);

    code.append(R"(
    void main() {
        // Unpack the input arguments from the vertex shader.
        mat4x2 P;
        mat2 tangents;
        if (gl_InvocationID == 0) {
            // This is the join section of the patch.
            P = mat4x2(vsPts01[0].xyxy, vsPts01[0].xyxy);
            tangents = mat2(vsPrevJoinTangent[0], vsTans01[0].xy);
        } else if (gl_InvocationID == 1) {
            // This is the first curve section of the patch.
            P = mat4x2(vsPts01[0], vsPts23[0]);
            tangents = mat2(vsTans01[0]);
        } else if (gl_InvocationID == 2) {
            // This is the second curve section of the patch.
            P = mat4x2(vsPts23[0].zw, vsPts45[0], vsPts67[0].xy);
            tangents = mat2(vsTans01[0].zw, vsTans23[0].xy);
        } else {
            // This is the third curve section of the patch.
            P = mat4x2(vsPts67[0], vsPts89[0]);
            tangents = mat2(vsTans23[0]);
        }

        // Calculate the number of parametric segments. The final tessellated strip will be a
        // composition of these parametric segments as well as radial segments.
        float numParametricSegments = wangs_formula_cubic(P, uWangsTermPow2);
        if (P[0] == P[1] && P[2] == P[3]) {
            // This is how the patch builder articulates lineTos but Wang's formula returns
            // >>1 segment in this scenario. Assign 1 parametric segment.
            numParametricSegments = 1;
        }

        // Determine the curve's start angle.
        float angle0 = atan2(tangents[0]);

        // Determine the curve's total rotation. The vertex shader ensures our curve does not rotate
        // more than 180 degrees or inflect, so the inverse cosine has enough range.
        vec2 tan0norm = normalize(tangents[0]);
        vec2 tan1norm = normalize(tangents[1]);
        float cosTheta = clamp(dot(tan1norm, tan0norm), -1, +1);
        float rotation = acos(cosTheta);

        // Adjust sign of rotation to match the direction the curve turns.
        // NOTE: Since the curve is not allowed to inflect, we can just check F'(.5) x F''(.5).
        // NOTE: F'(.5) x F''(.5) has the same sign as (P2 - P0) x (P3 - P1)
        float turn = cross(P[2] - P[0], P[3] - P[1]);
        if (turn == 0) {  // This will be the case for joins and cusps where points are co-located.
            turn = determinant(tangents);
        }
        if (turn < 0) {
            rotation = -rotation;
        }

        // Calculate the number of evenly spaced radial segments to chop this section of the curve
        // into. Radial segments divide the curve's rotation into even steps. The final tessellated
        // strip will be a composition of both parametric and radial segments.
        float numRadialSegments = abs(rotation) * uNumRadialSegmentsPerRadian;
        numRadialSegments = max(ceil(numRadialSegments), 1);

        if (gl_InvocationID == 0) {
            // Set up joins.
            numParametricSegments = 1;  // Joins don't have parametric segments.
            numRadialSegments = (uNumSegmentsInJoin == 0) ? numRadialSegments : uNumSegmentsInJoin;
            float innerStrokeRadiusMultiplier = 1;
            if (uNumSegmentsInJoin == 2) {
                innerStrokeRadiusMultiplier = miter_extent(cosTheta, uMiterLimitInvPow2);
            }
            vec2 strokeOutsetClamp = vec2(-1, 1);
            if (length_pow2(tan1norm - tan0norm) > uJoinTolerancePow2) {
                // Clamp the join to the exterior side of its junction. We only do this if the join
                // angle is large enough to guarantee there won't be cracks on the interior side of
                // the junction.
                strokeOutsetClamp = (rotation > 0) ? vec2(-1,0) : vec2(0,1);
            }
            tcsJoinArgs = vec3(innerStrokeRadiusMultiplier, strokeOutsetClamp);
        }

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
        float numCombinedSegments = numParametricSegments + numRadialSegments - 1;

        if (P[0] == P[3] && tangents[0] == tangents[1]) {
            // The vertex shader intentionally disabled our section. Set numCombinedSegments to 0.
            numCombinedSegments = 0;
        }

        // Pack the arguments for the evaluation stage.
        tcsPts01[gl_InvocationID] = vec4(P[0], P[1]);
        tcsPt2Tan0[gl_InvocationID] = vec4(P[2], tangents[0]);
        tcsTessArgs[gl_InvocationID] = vec4(numCombinedSegments, numParametricSegments, angle0,
                                            rotation / numRadialSegments);
        if (gl_InvocationID == 3) {
            tcsEndPtEndTan = vec4(P[3], tangents[1]);
        }

        barrier();

        if (gl_InvocationID == 0) {
            // Tessellate a quad strip with enough segments for the join plus all 3 curve sections
            // combined.
            float numTotalCombinedSegments = tcsTessArgs[0].x + tcsTessArgs[1].x +
                                             tcsTessArgs[2].x + tcsTessArgs[3].x;

            if (tcsTessArgs[0].x != 0 && tcsTessArgs[0].x != numTotalCombinedSegments) {
                // We are tessellating a quad strip with both a single-sided join and a double-sided
                // stroke. Add one more edge to the join. This new edge will fall parallel with the
                // first edge of the stroke, eliminating artifacts on the transition from single
                // sided to double.
                ++tcsTessArgs[gl_InvocationID].x;
                ++numTotalCombinedSegments;
            }

            numTotalCombinedSegments = min(numTotalCombinedSegments, MAX_TESSELLATION_SEGMENTS);
            gl_TessLevelInner[0] = numTotalCombinedSegments;
            gl_TessLevelInner[1] = 2.0;
            gl_TessLevelOuter[0] = 2.0;
            gl_TessLevelOuter[1] = numTotalCombinedSegments;
            gl_TessLevelOuter[2] = 2.0;
            gl_TessLevelOuter[3] = numTotalCombinedSegments;
        }
    }
    )");

    return code;
}

// Unlike mix(), this does not return b when t==1. But it otherwise seems to get better
// precision than "a*(1 - t) + b*t" for things like chopping cubics on exact cusp points.
// We override this result anyway when t==1 so it shouldn't be a problem.
static const char* kUncheckedMixFn = R"(
float unchecked_mix(float a, float b, float T) {
    return fma(b - a, T, a);
}
float2 unchecked_mix(float2 a, float2 b, float T) {
    return fma(b - a, float2(T), a);
})";

// Computes the location and tangent direction of the stroke edge with the integral id
// "combinedEdgeID", where combinedEdgeID is the sorted-order index of parametric and radial edges.
static void append_eval_stroke_edge_fn(SkString* code, bool hasConics) {
    code->append(R"(
    void eval_stroke_edge(in float4x2 P, )");
    if (hasConics) {
        code->append(R"(
                          in float w, )");
    }
    code->append(R"(
                          in float numParametricSegments, in float combinedEdgeID, in float2 tan0,
                          in float radsPerSegment, in float angle0, out float2 tangent,
                          out float2 position) {
        // Start by finding the tangent function's power basis coefficients. These define a tangent
        // direction (scaled by some uniform value) as:
        //                                                 |T^2|
        //     Tangent_Direction(T) = dx,dy = |A  2B  C| * |T  |
        //                                    |.   .  .|   |1  |
        float2 A, B, C = P[1] - P[0];
        float2 D = P[3] - P[0];)");
    if (hasConics) {
        code->append(R"(
        if (w >= 0) {
            // P0..P2 represent a conic and P3==P2. The derivative of a conic has a cumbersome
            // order-4 denominator. However, this isn't necessary if we are only interested in a
            // vector in the same *direction* as a given tangent line. Since the denominator scales
            // dx and dy uniformly, we can throw it out completely after evaluating the derivative
            // with the standard quotient rule. This leaves us with a simpler quadratic function
            // that we use to find a tangent.
            C *= w;
            B = .5*D - C;
            A = (w - 1) * D;
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
        float2 B_ = B * (numParametricSegments * 2);
        float2 C_ = C * (numParametricSegments * numParametricSegments);

        // Run a binary search to determine the highest parametric edge that is located on or before
        // the combinedEdgeID. A combined ID is determined by the sum of complete parametric and
        // radial segments behind it. i.e., find the highest parametric edge where:
        //
        //    parametricEdgeID + floor(numRadialSegmentsAtParametricT) <= combinedEdgeID
        //
        float lastParametricEdgeID = 0;
        float maxParametricEdgeID = min(numParametricSegments - 1, combinedEdgeID);
        float2 tan0norm = normalize(tan0);
        float negAbsRadsPerSegment = -abs(radsPerSegment);
        float maxRotation0 = (1 + combinedEdgeID) * abs(radsPerSegment);
        for (int exp = MAX_PARAMETRIC_SEGMENTS_LOG2 - 1; exp >= 0; --exp) {
            // Test the parametric edge at lastParametricEdgeID + 2^exp.
            float testParametricID = lastParametricEdgeID + (1 << exp);
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
        float discr_over_4 = max(b_over_2*b_over_2 - a*c, 0);
        float q = sqrt(discr_over_4);
        if (b_over_2 > 0) {
            q = -q;
        }
        q -= b_over_2;

        // Roots are q/a and c/q. Since each curve section does not inflect or rotate more than 180
        // degrees, there can only be one tangent orthogonal to "norm" inside 0..1. Pick the root
        // nearest .5.
        float _5qa = -.5*q*a;
        float2 root = (abs(fma(q,q,_5qa)) < abs(fma(a,c,_5qa))) ? float2(q,a) : float2(c,q);
        float radialT = (root.t != 0) ? root.s / root.t : 0;
        radialT = clamp(radialT, 0, 1);

        if (lastRadialEdgeID == 0) {
            // The root finder above can become unstable when lastRadialEdgeID == 0 (e.g., if there
            // are roots at exatly 0 and 1 both). radialT should always == 0 in this case.
            radialT = 0;
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
        float u = unchecked_mix(1, w, T);
        float v = unchecked_mix(w, 1, T);
        float uv = unchecked_mix(u, v, T);)");
    }

    code->appendf(R"(
        position =%s abcd;)", (hasConics) ? " (w >= 0) ? abc/uv :" : "");

    code->appendf(R"(
        // If we went with T=parametricT, then update the tangent. Otherwise leave it at the radial
        // tangent found previously. (In the event that parametricT == radialT, we keep the radial
        // tangent.)
        if (T != radialT) {)");
    code->appendf(R"(
            tangent =%s bcd - abc;)", (hasConics) ? " (w >= 0) ? bc*u - ab*v :" : "");
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

    // Use a #define to make extra sure we don't prevent the loop from unrolling.
    code.appendf("#define MAX_PARAMETRIC_SEGMENTS_LOG2 %i\n",
                 SkNextLog2(shaderCaps.maxTessellationSegments()));
    code.appendf("#define PI 3.141592653589793238\n");

    const char* tessArgs2Name = impl->getTessArgs2UniformName(uniformHandler);
    code.appendf("uniform vec2 %s;\n", tessArgs2Name);
    code.appendf("#define uStrokeRadius %s.y\n", tessArgs2Name);

    if (!this->viewMatrix().isIdentity()) {
        const char* translateName = impl->getTranslateUniformName(uniformHandler);
        code.appendf("uniform vec2 %s;\n", translateName);
        code.appendf("#define uTranslate %s\n", translateName);
        if (!fStroke.isHairlineStyle()) {
            // In the normal case we need the affine matrix too. (In the hairline case we already
            // applied the affine matrix in the vertex shader.)
            const char* affineMatrixName = impl->getAffineMatrixUniformName(uniformHandler);
            code.appendf("uniform vec4 %s;\n", affineMatrixName);
            code.appendf("#define uAffineMatrix mat2(%s)\n", affineMatrixName);
        }
    }

    code.append(R"(
    in vec4 tcsPts01[];
    in vec4 tcsPt2Tan0[];
    in vec4 tcsTessArgs[];
    patch in vec4 tcsEndPtEndTan;
    patch in vec3 tcsJoinArgs;

    uniform vec4 sk_RTAdjust;)");

    code.append(kUncheckedMixFn);
    append_eval_stroke_edge_fn(&code, false/*hasConics*/);

    code.append(R"(
    void main() {
        // Our patch is composed of exactly "numTotalCombinedSegments + 1" stroke-width edges that
        // run orthogonal to the curve and make a strip of "numTotalCombinedSegments" quads.
        // Determine which discrete edge belongs to this invocation. An edge can either come from a
        // parametric segment or a radial one.
        float numTotalCombinedSegments = tcsTessArgs[0].x + tcsTessArgs[1].x + tcsTessArgs[2].x +
                                         tcsTessArgs[3].x;
        float totalEdgeID = round(gl_TessCoord.x * numTotalCombinedSegments);

        // Furthermore, the vertex shader may have chopped the curve into 3 different sections.
        // Determine which section we belong to, and where we fall relative to its first edge.
        float localEdgeID = totalEdgeID;
        mat4x2 P;
        vec2 tan0;
        vec3 tessellationArgs;
        float strokeRadius = uStrokeRadius;
        vec2 strokeOutsetClamp = vec2(-1, 1);
        if (localEdgeID < tcsTessArgs[0].x || tcsTessArgs[0].x == numTotalCombinedSegments) {
            // Our edge belongs to the join preceding the curve.
            P = mat4x2(tcsPts01[0], tcsPt2Tan0[0].xy, tcsPts01[1].xy);
            tan0 = tcsPt2Tan0[0].zw;
            tessellationArgs = tcsTessArgs[0].yzw;
            strokeRadius *= (localEdgeID == 1) ? tcsJoinArgs.x : 1;
            strokeOutsetClamp = tcsJoinArgs.yz;
        } else if ((localEdgeID -= tcsTessArgs[0].x) < tcsTessArgs[1].x) {
            // Our edge belongs to the first curve section.
            P = mat4x2(tcsPts01[1], tcsPt2Tan0[1].xy, tcsPts01[2].xy);
            tan0 = tcsPt2Tan0[1].zw;
            tessellationArgs = tcsTessArgs[1].yzw;
        } else if ((localEdgeID -= tcsTessArgs[1].x) < tcsTessArgs[2].x) {
            // Our edge belongs to the second curve section.
            P = mat4x2(tcsPts01[2], tcsPt2Tan0[2].xy, tcsPts01[3].xy);
            tan0 = tcsPt2Tan0[2].zw;
            tessellationArgs = tcsTessArgs[2].yzw;
        } else {
            // Our edge belongs to the third curve section.
            localEdgeID -= tcsTessArgs[2].x;
            P = mat4x2(tcsPts01[3], tcsPt2Tan0[3].xy, tcsEndPtEndTan.xy);
            tan0 = tcsPt2Tan0[3].zw;
            tessellationArgs = tcsTessArgs[3].yzw;
        }
        float numParametricSegments = tessellationArgs.x;
        float angle0 = tessellationArgs.y;
        float radsPerSegment = tessellationArgs.z;

        float2 tangent, position;
        eval_stroke_edge(P, numParametricSegments, localEdgeID, tan0, radsPerSegment, angle0,
                         tangent, position);

        if (localEdgeID == 0) {
            // The first local edge of each section uses the provided tan0. This ensures continuous
            // rotation across chops made by the vertex shader as well as crack-free seaming between
            // patches. (NOTE: position is always equal to P[0] here when localEdgeID==0.)
            tangent = tan0;
        }

        if (gl_TessCoord.x == 1) {
            // The final edge of the quad strip always uses the provided endPt and endTan. This
            // ensures crack-free seaming between patches.
            tangent = tcsEndPtEndTan.zw;
            position = tcsEndPtEndTan.xy;
        }

        // Determine how far to outset our vertex orthogonally from the curve.
        float outset = gl_TessCoord.y * 2 - 1;
        outset = clamp(outset, strokeOutsetClamp.x, strokeOutsetClamp.y);
        outset *= strokeRadius;

        vec2 vertexPos = position + normalize(vec2(-tangent.y, tangent.x)) * outset;
    )");

    if (!this->viewMatrix().isIdentity()) {
        if (!fStroke.isHairlineStyle()) {
            // Normal case. Do the transform after tessellation.
            code.append("vertexPos = uAffineMatrix * vertexPos + uTranslate;");
        } else {
            // Hairline case. The scale and skew already happened before tessellation.
            code.append("vertexPos = vertexPos + uTranslate;");
        }
    }

    code.append(R"(
        gl_Position = vec4(vertexPos * sk_RTAdjust.xz + sk_RTAdjust.yw, 0.0, 1.0);
    }
    )");

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
        args.fVertBuilder->defineConstant("QUAD_TERM_POW2",
                                          GrWangsFormula::length_term_pow2<2>(1));
        args.fVertBuilder->defineConstant("CUBIC_TERM_POW2",
                                          GrWangsFormula::length_term_pow2<3>(1));

        // Helper functions.
        args.fVertBuilder->insertFunction(kAtan2Fn);
        args.fVertBuilder->insertFunction(kLengthPow2Fn);
        args.fVertBuilder->insertFunction(kMiterExtentFn);
        args.fVertBuilder->insertFunction(kUncheckedMixFn);
        append_eval_stroke_edge_fn(&args.fVertBuilder->functions(), shader.fHasConics);
        args.fVertBuilder->insertFunction(R"(
        float cosine_between_vectors(float2 a, float2 b) {
            float ab_cosTheta = dot(a,b);
            float ab_pow2 = dot(a,a) * dot(b,b);
            return (ab_pow2 == 0) ? 1 : clamp(ab_cosTheta * inversesqrt(ab_pow2), -1, 1);
        })");

        // Tessellation control uniforms.
        const char* tessArgsName;
        fTessControlArgsUniform = args.fUniformHandler->addUniform(
                nullptr, kVertex_GrShaderFlag, kFloat4_GrSLType, "tessControlArgs", &tessArgsName);
        args.fVertBuilder->codeAppendf("float uParametricIntolerance = %s.x;\n", tessArgsName);
        args.fVertBuilder->codeAppendf("float uNumRadialSegmentsPerRadian = %s.y;\n", tessArgsName);
        args.fVertBuilder->codeAppendf("float uMiterLimitInvPow2 = %s.z;\n", tessArgsName);
        args.fVertBuilder->codeAppendf("float uStrokeRadius = %s.w;\n", tessArgsName);

        // View matrix uniforms.
        if (!shader.viewMatrix().isIdentity()) {
            const char* translateName, *affineMatrixName;
            fAffineMatrixUniform = args.fUniformHandler->addUniform(
                    nullptr, kVertex_GrShaderFlag, kFloat4_GrSLType, "affineMatrix",
                    &affineMatrixName);
            fTranslateUniform = args.fUniformHandler->addUniform(
                    nullptr, kVertex_GrShaderFlag, kFloat2_GrSLType, "translate", &translateName);
            args.fVertBuilder->codeAppendf("float2x2 uAffineMatrix = float2x2(%s);\n",
                                           affineMatrixName);
            args.fVertBuilder->codeAppendf("float2 uTranslate = %s;\n", translateName);
        }

        // Tessellation code.
        args.fVertBuilder->codeAppend(R"(
        float4x2 P = float4x2(pts01, pts23);
        float2 lastControlPoint = args.xy;)");
        if (shader.fHasConics) {
            args.fVertBuilder->codeAppend(R"(
            float w = -1;  // w<0 means the curve is an integral cubic.
            if (isinf(P[3].y)) {
                w = P[3].x;  // The curve is actually a conic.
                P[3] = P[2];  // Setting p3 equal to p2 works for the remaining rotational logic.
            })");
        }
        if (shader.fStroke.isHairlineStyle() && !shader.viewMatrix().isIdentity()) {
            // Hairline case. Transform the points before tessellation. We can still hold off on the
            // translate until the end; we just need to perform the scale and skew right now.
            args.fVertBuilder->codeAppend(R"(
            P = uAffineMatrix * P;
            lastControlPoint = uAffineMatrix * lastControlPoint;)");
        }
        args.fVertBuilder->codeAppend(R"(
        float numTotalEdges = abs(args.z);

        // Use wang's formula to find how many parametric segments this stroke requires.
        float l0 = length_pow2(fma(float2(-2), P[1], P[2]) + P[0]);
        float l1 = length_pow2(fma(float2(-2), P[2], P[3]) + P[1]);)");

        args.fVertBuilder->codeAppendf(R"(
        float m =%s CUBIC_TERM_POW2 * max(l0, l1);)",
                (shader.fHasConics) ? " (w >= 0) ? QUAD_TERM_POW2 * l0 :" : "");

        args.fVertBuilder->codeAppend(R"(
        float numParametricSegments = ceil(sqrt(uParametricIntolerance * sqrt(m)));
        numParametricSegments = clamp(numParametricSegments, 1, 1 << MAX_PARAMETRIC_SEGMENTS_LOG2);
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

        if (shader.fStroke.getJoin() == SkPaint::kRound_Join) {
            args.fVertBuilder->codeAppend(R"(
            // Determine how many edges to give to the round join. We emit the first and final edges
            // of the join twice: once full width and once restricted to half width. This guarantees
            // perfect seaming by matching the vertices from the join as well as from the strokes on
            // either side.
            float joinRads = acos(cosine_between_vectors(P[0] - lastControlPoint, tan0));
            float numRadialSegmentsInJoin = max(ceil(joinRads * uNumRadialSegmentsPerRadian), 1);
            // +2 because we emit the beginning and ending edges twice (see above comment).
            float numEdgesInJoin = numRadialSegmentsInJoin + 2;
            // The stroke section needs at least two edges. Don't assign more to the join than
            // "numTotalEdges - 2".
            numEdgesInJoin = min(numEdgesInJoin, numTotalEdges - 2);
            // Lines give all their extra edges to the join.
            if (numParametricSegments == 1) {
                numEdgesInJoin = numTotalEdges - 2;
            }
            // Negative args.z means the join is a chop, and chop joins get exactly one segment.
            if (args.z < 0) {
                // +2 because we emit the beginning and ending edges twice (see above comment).
                numEdgesInJoin = 1 + 2;
            })");
        } else {
            args.fVertBuilder->codeAppendf(R"(
            float numEdgesInJoin = %i;")", IndirectInstance::NumExtraEdgesInJoin(joinType));
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
                    outset = clamp(outset, (turn < 0) ? -1 : 0, (turn >= 0) ? 1 : 0);
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

        if (joinType == SkPaint::kMiter_Join) {
            args.fVertBuilder->codeAppend(R"(
            // Vertices #4 and #5 belong to the edge of the join that extends to the miter point.
            if ((sk_VertexID | 1) == (4 | 5)) {
                outset *= miter_extent(cosTheta, uMiterLimitInvPow2);
            })");
        }

        args.fVertBuilder->codeAppendf(R"(
        float2 tangent, strokeCoord;
        eval_stroke_edge(P,)");
        if (shader.fHasConics) {
            args.fVertBuilder->codeAppend(R"(
                         w,)");
        }
        args.fVertBuilder->codeAppend(R"(
                         numParametricSegments, combinedEdgeID, tan0, radsPerSegment, angle0,
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
        strokeCoord += ortho * (uStrokeRadius * outset);)");

        if (shader.viewMatrix().isIdentity()) {
            // No transform matrix.
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "strokeCoord");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "strokeCoord");
        } else if (!shader.fStroke.isHairlineStyle()) {
            // Normal case. Do the transform after tessellation.
            args.fVertBuilder->codeAppend(R"(
            float2 devCoord = uAffineMatrix * strokeCoord + uTranslate;)");
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "devCoord");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "strokeCoord");
        } else {
            // Hairline case. The scale and skew already happened before tessellation.
            args.fVertBuilder->codeAppend(R"(
            float2 devCoord = strokeCoord + uTranslate;
            float2 localCoord = inverse(uAffineMatrix) * strokeCoord;)");
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "devCoord");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localCoord");
        }

        // The fragment shader just outputs a uniform color.
        const char* colorUniformName;
        fColorUniform = args.fUniformHandler->addUniform(
                nullptr, kFragment_GrShaderFlag, kHalf4_GrSLType, "color", &colorUniformName);
        args.fFragBuilder->codeAppendf("%s = %s;", args.fOutputColor, colorUniformName);
        args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrPrimitiveProcessor& primProc) override {
        const auto& shader = primProc.cast<GrStrokeTessellateShader>();
        const auto& stroke = shader.fStroke;

        // Set up the tessellation control uniforms.
        Tolerances tolerances;
        if (!stroke.isHairlineStyle()) {
            tolerances.set(shader.viewMatrix().getMaxScale(), stroke.getWidth());
        } else {
            // In the hairline case we transform prior to tessellation. Set up tolerances for an
            // identity viewMatrix and a strokeWidth of 1.
            tolerances.set(1, 1);
        }
        float miterLimit = stroke.getMiter();
        pdman.set4f(fTessControlArgsUniform,
                    tolerances.fParametricIntolerance,  // uParametricIntolerance
                    tolerances.fNumRadialSegmentsPerRadian,  // uNumRadialSegmentsPerRadian
                    1 / (miterLimit * miterLimit),  // uMiterLimitInvPow2
                    (stroke.isHairlineStyle()) ? .5f : stroke.getWidth() * .5);  // uStrokeRadius

        // Set up the view matrix, if any.
        const SkMatrix& m = shader.viewMatrix();
        if (!m.isIdentity()) {
            pdman.set2f(fTranslateUniform, m.getTranslateX(), m.getTranslateY());
            pdman.set4f(fAffineMatrixUniform, m.getScaleX(), m.getSkewY(), m.getSkewX(),
                        m.getScaleY());
        }

        pdman.set4fv(fColorUniform, 1, shader.fColor.vec());
    }

    GrGLSLUniformHandler::UniformHandle fTessControlArgsUniform;
    GrGLSLUniformHandler::UniformHandle fTranslateUniform;
    GrGLSLUniformHandler::UniformHandle fAffineMatrixUniform;
    GrGLSLUniformHandler::UniformHandle fColorUniform;
};

void GrStrokeTessellateShader::getGLSLProcessorKey(const GrShaderCaps&,
                                                   GrProcessorKeyBuilder* b) const {
    uint32_t key = this->viewMatrix().isIdentity();
    if (fMode == Mode::kIndirect) {
        SkASSERT(fStroke.getJoin() >> 2 == 0);
        key = (key << 2) | fStroke.getJoin();
    }
    key = (key << 1) | (uint32_t)fStroke.isHairlineStyle();
    key = (key << 1) | (uint32_t)fHasConics;
    key = (key << 1) | (uint32_t)fMode;  // Must be last.
    b->add32(key);
}

GrGLSLPrimitiveProcessor* GrStrokeTessellateShader::createGLSLInstance(
        const GrShaderCaps&) const {
    return (fMode == Mode::kTessellation) ?
            (GrGLSLPrimitiveProcessor*)new TessellationImpl : new IndirectImpl;
}

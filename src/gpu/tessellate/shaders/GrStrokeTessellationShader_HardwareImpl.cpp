/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/shaders/GrStrokeTessellationShader.h"

#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/tessellate/WangsFormula.h"

void GrStrokeTessellationShader::HardwareImpl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const auto& shader = args.fGeomProc.cast<GrStrokeTessellationShader>();
    auto* uniHandler = args.fUniformHandler;
    auto* v = args.fVertBuilder;

    args.fVaryingHandler->emitAttributes(shader);

    v->defineConstant("float", "PI", "3.141592653589793238");

    // The vertex shader chops the curve into 3 sections in order to meet our tessellation
    // requirements. The stroke tessellator does not allow curve sections to inflect or to rotate
    // more than 180 degrees.
    //
    // We start by chopping at inflections (if the curve has any), or else at midtangent. If we
    // still don't have 3 sections after that then we just subdivide uniformly in parametric space.
    using TypeModifier = GrShaderVar::TypeModifier;
    v->defineConstantf("float", "kParametricEpsilon", "1.0 / (%i * 128)",
                       args.fShaderCaps->maxTessellationSegments());  // 1/128 of a segment.

    // [numSegmentsInJoin, innerJoinRadiusMultiplier, prevJoinTangent.xy]
    v->declareGlobal(GrShaderVar("vsJoinArgs0", kFloat4_GrSLType, TypeModifier::Out));

    // [radsPerJoinSegment, joinOutsetClamp.xy]
    v->declareGlobal(GrShaderVar("vsJoinArgs1", kFloat3_GrSLType, TypeModifier::Out));

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

    v->insertFunction(kCosineBetweenVectorsFn);
    v->insertFunction(kMiterExtentFn);
    v->insertFunction(kUncheckedMixFn);
    if (shader.hasDynamicStroke()) {
        v->insertFunction(kNumRadialSegmentsPerRadianFn);
    }

    if (!shader.hasDynamicStroke()) {
        // [PARAMETRIC_PRECISION, NUM_RADIAL_SEGMENTS_PER_RADIAN, JOIN_TYPE, STROKE_RADIUS]
        const char* tessArgsName;
        fTessControlArgsUniform = uniHandler->addUniform(nullptr,
                                                         kVertex_GrShaderFlag |
                                                         kTessControl_GrShaderFlag |
                                                         kTessEvaluation_GrShaderFlag,
                                                         kFloat4_GrSLType, "tessArgs",
                                                         &tessArgsName);
        v->codeAppendf(R"(
        float NUM_RADIAL_SEGMENTS_PER_RADIAN = %s.y;
        float JOIN_TYPE = %s.z;)", tessArgsName, tessArgsName);
    } else {
        const char* parametricPrecisionName;
        fTessControlArgsUniform = uniHandler->addUniform(nullptr,
                                                         kVertex_GrShaderFlag |
                                                         kTessControl_GrShaderFlag |
                                                         kTessEvaluation_GrShaderFlag,
                                                         kFloat_GrSLType, "parametricPrecision",
                                                         &parametricPrecisionName);
        v->codeAppendf(R"(
        float STROKE_RADIUS = dynamicStrokeAttr.x;
        float NUM_RADIAL_SEGMENTS_PER_RADIAN = num_radial_segments_per_radian(%s,STROKE_RADIUS);
        float JOIN_TYPE = dynamicStrokeAttr.y;)", parametricPrecisionName);
    }

    fTranslateUniform = uniHandler->addUniform(nullptr, kTessEvaluation_GrShaderFlag,
                                               kFloat2_GrSLType, "translate", nullptr);
    // View matrix uniforms.
    const char* affineMatrixName;
    // Hairlines apply the affine matrix in their vertex shader, prior to tessellation.
    // Otherwise the entire view matrix gets applied at the end of the tess eval shader.
    auto affineMatrixVisibility = kTessEvaluation_GrShaderFlag;
    if (shader.stroke().isHairlineStyle()) {
        affineMatrixVisibility |= kVertex_GrShaderFlag;
    }
    fAffineMatrixUniform = uniHandler->addUniform(nullptr, affineMatrixVisibility, kFloat4_GrSLType,
                                                  "affineMatrix", &affineMatrixName);
    if (affineMatrixVisibility & kVertex_GrShaderFlag) {
        v->codeAppendf("float2x2 AFFINE_MATRIX = float2x2(%s);\n", affineMatrixName);
    }

    v->codeAppend(R"(
    // Unpack the control points.
    float2 prevControlPoint = prevCtrlPtAttr;
    float4x2 P = float4x2(pts01Attr.xy, pts01Attr.zw, pts23Attr.xy, pts23Attr.zw);)");

    if (shader.stroke().isHairlineStyle()) {
        // Hairline case. Transform the points before tessellation. We can still hold off on the
        // translate until the end; we just need to perform the scale and skew right now.
        v->codeAppend(R"(
        P = AFFINE_MATRIX * P;
        if (isinf(pts23Attr.w)) {
            // If y3 is infinity then x3 is a conic weight. Don't transform.
            P[3] = pts23Attr.zw;
        }
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
        // disables the first two sections of the curve because their tangents and points are all
        // equal.
        tan0 = prevJoinTangent;
        prevJoinTangent = float2(0);  // Disable the join section.
    }

    if (tan1 == float2(0)) {
        // [p0, p3, p3, p3] is a reserved pattern that means this patch is a join only. Colocate all
        // the curve's points to ensure it gets disabled by the tessellation stages.
        P[1] = P[2] = P[3] = P[0];
        // Since the points are colocated, this curve will register as a line, which overrides
        // innerTangents as [tan0, tan0]. Setting tan1=tan0 as well results in all tangents and all
        // points being equal, which disables every section of the curve.
        tan1 = tan0;
    }

    // Calculate the number of segments to chop the join into.
    float cosTheta = cosine_between_vectors(prevJoinTangent, tan0);
    float joinRotation = (cosTheta == 1) ? 0 : acos(cosTheta);
    if (cross_length_2d(prevJoinTangent, tan0) < 0) {
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
        joinOutsetClamp = (joinRotation < 0) ? float2(-1, 0) : float2(0, 1);
    }

    // Pack join args for the tessellation control stage.
    vsJoinArgs0 = float4(numSegmentsInJoin, innerJoinRadiusMultiplier, prevJoinTangent);
    vsJoinArgs1 = float3(joinRotation / numSegmentsInJoin, joinOutsetClamp);

    // Now find where to chop the curve so the resulting sub-curves are convex and do not rotate
    // more than 180 degrees. We don't need to worry about cusps because the caller chops those out
    // on the CPU. Start by finding the cubic's power basis coefficients. These define the bezier
    // curve as:
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
    float a = cross_length_2d(A, B);
    float b = cross_length_2d(A, C);
    float c = cross_length_2d(B, C);
    float b_over_2 = b*.5;
    float discr_over_4 = b_over_2*b_over_2 - a*c;

    float2x2 innerTangents = float2x2(0);
    if (discr_over_4 <= 0) {
        // The curve does not inflect. This means it might rotate more than 180 degrees instead.
        // Craft a quadratic whose roots are the points were rotation == 180 deg and 0. (These are
        // the points where the tangent is parallel to tan0.)
        //
        //      Tangent_Direction(T) x tan0 == 0
        //      (AT^2 x tan0) + (2BT x tan0) + (C x tan0) == 0
        //      (A x C)T^2 + (2B x C)T + (C x C) == 0  [[because tan0 == P1 - P0 == C]]
        //      bT^2 + 2cT + 0 == 0  [[because A x C == b, B x C == c]]
        //
        // NOTE: When P0 == P1 then C != tan0, C == 0 and these roots will be undefined. But that's
        // ok because when P0 == P1 the curve cannot rotate more than 180 degrees anyway.
        a = b;
        b_over_2 = c;
        c = 0;
        discr_over_4 = b_over_2*b_over_2;
        innerTangents[0] = -C;
    }

    // Solve our quadratic equation for the chop points. This is inspired by the quadratic formula
    // from Numerical Recipes in C.
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

    if (shader.hasDynamicColor()) {
        // Color gets passed in from the tess evaluation shader.
        fDynamicColorName = "dynamicColor";
        SkString flatness(args.fShaderCaps->preferFlatInterpolation() ? "flat" : "");
        args.fFragBuilder->declareGlobal(GrShaderVar(fDynamicColorName, kHalf4_GrSLType,
                                                     TypeModifier::In, 0, SkString(), flatness));
    }
    this->emitFragmentCode(shader, args);
}

SkString GrStrokeTessellationShader::HardwareImpl::getTessControlShaderGLSL(
        const GrGeometryProcessor& geomProc,
        const char* versionAndExtensionDecls,
        const GrGLSLUniformHandler& uniformHandler,
        const GrShaderCaps& shaderCaps) const {
    const auto& shader = geomProc.cast<GrStrokeTessellationShader>();
    SkASSERT(shader.mode() == GrStrokeTessellationShader::Mode::kHardwareTessellation);

    SkString code(versionAndExtensionDecls);
    // Run 3 invocations: 1 for each section that the vertex shader chopped the curve into.
    code.append("layout(vertices = 3) out;\n");
    code.appendf("precision highp float;\n");

    code.appendf("#define float2 vec2\n");
    code.appendf("#define float3 vec3\n");
    code.appendf("#define float4 vec4\n");
    code.appendf("#define float2x2 mat2\n");
    code.appendf("#define float3x2 mat3x2\n");
    code.appendf("#define float4x2 mat4x2\n");
    code.appendf("#define PI 3.141592653589793238\n");
    code.appendf("#define MAX_TESSELLATION_SEGMENTS %i.0\n", shaderCaps.maxTessellationSegments());
    code.appendf("#define cross cross2d\n");  // GLSL already has a function named "cross".

    const char* tessArgsName = uniformHandler.getUniformCStr(fTessControlArgsUniform);
    if (!shader.hasDynamicStroke()) {
        code.appendf("uniform vec4 %s;\n", tessArgsName);
        code.appendf("#define PARAMETRIC_PRECISION %s.x\n", tessArgsName);
        code.appendf("#define NUM_RADIAL_SEGMENTS_PER_RADIAN %s.y\n", tessArgsName);
    } else {
        code.appendf("uniform float %s;\n", tessArgsName);
        code.appendf("#define PARAMETRIC_PRECISION %s\n", tessArgsName);
        code.appendf("#define NUM_RADIAL_SEGMENTS_PER_RADIAN vsStrokeArgs[0].x\n");
    }

    code.append(skgpu::wangs_formula::as_sksl());
    code.append(kCosineBetweenVectorsFn);
    code.append(kMiterExtentFn);
    code.append(R"(
    float cross2d(vec2 a, vec2 b) {
        return determinant(mat2(a,b));
    })");

    code.append(R"(
    in vec4 vsJoinArgs0[];
    in vec3 vsJoinArgs1[];
    in vec4 vsPts01[];
    in vec4 vsPts23[];
    in vec4 vsPts45[];
    in vec4 vsPts67[];
    in vec4 vsPts89[];
    in vec4 vsTans01[];
    in vec4 vsTans23[];)");
    if (shader.hasDynamicStroke()) {
        code.append(R"(
        in vec2 vsStrokeArgs[];)");
    }
    if (shader.hasDynamicColor()) {
        code.append(R"(
        in mediump vec4 vsColor[];)");
    }

    code.append(R"(
    out vec4 tcsPts01[];
    out vec4 tcsPt2Tan0[];
    out vec3 tcsTessArgs[];  // [numCombinedSegments, numParametricSegments, radsPerSegment]
    patch out vec4 tcsJoinArgs0; // [numSegmentsInJoin, innerJoinRadiusMultiplier,
                                 //  prevJoinTangent.xy]
    patch out vec3 tcsJoinArgs1;  // [radsPerJoinSegment, joinOutsetClamp.xy]
    patch out vec4 tcsEndPtEndTan;)");
    if (shader.hasDynamicStroke()) {
        code.append(R"(
        patch out float tcsStrokeRadius;)");
    }
    if (shader.hasDynamicColor()) {
        code.append(R"(
        patch out mediump vec4 tcsColor;)");
    }

    code.append(R"(
    void main() {
        // Forward join args to the evaluation stage.
        tcsJoinArgs0 = vsJoinArgs0[0];
        tcsJoinArgs1 = vsJoinArgs1[0];)");
    if (shader.hasDynamicStroke()) {
        code.append(R"(
        tcsStrokeRadius = vsStrokeArgs[0].y;)");
    }
    if (shader.hasDynamicColor()) {
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
        float numParametricSegments;
        if (w < 0.0) {
            numParametricSegments = wangs_formula_cubic(PARAMETRIC_PRECISION, P[0], P[1], P[2],
                                                        P[3], mat2(1));
        } else {
            numParametricSegments = wangs_formula_conic(PARAMETRIC_PRECISION, P[0], P[1], P[2], w);
        }
        if (P[0] == P[1] && P[2] == P[3]) {
            // This is how the patch builder articulates lineTos but Wang's formula returns
            // >>1 segment in this scenario. Assign 1 parametric segment.
            numParametricSegments = 1.0;
        }

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
        tcsTessArgs[gl_InvocationID] = vec3(numCombinedSegments, numParametricSegments,
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

SkString GrStrokeTessellationShader::HardwareImpl::getTessEvaluationShaderGLSL(
        const GrGeometryProcessor& geomProc,
        const char* versionAndExtensionDecls,
        const GrGLSLUniformHandler& uniformHandler,
        const GrShaderCaps& shaderCaps) const {
    const auto& shader = geomProc.cast<GrStrokeTessellationShader>();
    SkASSERT(shader.mode() == GrStrokeTessellationShader::Mode::kHardwareTessellation);

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

    if (!shader.hasDynamicStroke()) {
        const char* tessArgsName = uniformHandler.getUniformCStr(fTessControlArgsUniform);
        code.appendf("uniform vec4 %s;\n", tessArgsName);
        code.appendf("#define STROKE_RADIUS %s.w\n", tessArgsName);
    } else {
        code.appendf("#define STROKE_RADIUS tcsStrokeRadius\n");
    }

    const char* translateName = uniformHandler.getUniformCStr(fTranslateUniform);
    code.appendf("uniform vec2 %s;\n", translateName);
    code.appendf("#define TRANSLATE %s\n", translateName);
    const char* affineMatrixName = uniformHandler.getUniformCStr(fAffineMatrixUniform);
    code.appendf("uniform vec4 %s;\n", affineMatrixName);
    code.appendf("#define AFFINE_MATRIX mat2(%s)\n", affineMatrixName);

    code.append(R"(
    in vec4 tcsPts01[];
    in vec4 tcsPt2Tan0[];
    in vec3 tcsTessArgs[];  // [numCombinedSegments, numParametricSegments, radsPerSegment]
    patch in vec4 tcsJoinArgs0;  // [numSegmentsInJoin, innerJoinRadiusMultiplier,
                                 //  prevJoinTangent.xy]
    patch in vec3 tcsJoinArgs1;  // [radsPerJoinSegment, joinOutsetClamp.xy]
    patch in vec4 tcsEndPtEndTan;)");
    if (shader.hasDynamicStroke()) {
        code.append(R"(
        patch in float tcsStrokeRadius;)");
    }
    if (shader.hasDynamicColor()) {
        code.appendf(R"(
        patch in mediump vec4 tcsColor;
        %s out mediump vec4 %s;)",
        shaderCaps.preferFlatInterpolation() ? "flat" : "", fDynamicColorName.c_str());
    }

    code.append(R"(
    uniform vec4 sk_RTAdjust;)");

    code.append(kUncheckedMixFn);

    code.append(R"(
    void main() {
        // Our patch is composed of exactly "numTotalCombinedSegments + 1" stroke-width edges that
        // run orthogonal to the curve and make a strip of "numTotalCombinedSegments" quads.
        // Determine which discrete edge belongs to this invocation. An edge can either come from a
        // parametric segment or a radial one.
        float numSegmentsInJoin = tcsJoinArgs0.x;
        float numTotalCombinedSegments = numSegmentsInJoin + tcsTessArgs[0].x + tcsTessArgs[1].x +
                                         tcsTessArgs[2].x;
        float combinedEdgeID = round(gl_TessCoord.x * numTotalCombinedSegments);
        float strokeOutset = gl_TessCoord.y * 2.0 - 1.0;

        // Furthermore, the vertex shader may have chopped the curve into 3 different sections.
        // Determine which section we belong to, and where we fall relative to its first edge.
        float2 p0, p1, p2, p3;
        vec2 tan0;
        float numParametricSegments, radsPerSegment;
        if (combinedEdgeID < numSegmentsInJoin || numSegmentsInJoin == numTotalCombinedSegments) {
            // Our edge belongs to the join preceding the curve.
            p3 = p2 = p1 = p0 = tcsPts01[0].xy;
            tan0 = tcsJoinArgs0.zw;
            numParametricSegments = 1;
            radsPerSegment = tcsJoinArgs1.x;
            strokeOutset = clamp(strokeOutset, tcsJoinArgs1.y, tcsJoinArgs1.z);
            strokeOutset *= (combinedEdgeID == 1.0) ? tcsJoinArgs0.y : 1.0;
        } else if ((combinedEdgeID -= numSegmentsInJoin) < tcsTessArgs[0].x) {
            // Our edge belongs to the first curve section.
            p0=tcsPts01[0].xy, p1=tcsPts01[0].zw, p2=tcsPt2Tan0[0].xy, p3=tcsPts01[1].xy;
            tan0 = tcsPt2Tan0[0].zw;
            numParametricSegments = tcsTessArgs[0].y;
            radsPerSegment = tcsTessArgs[0].z;
        } else if ((combinedEdgeID -= tcsTessArgs[0].x) < tcsTessArgs[1].x) {
            // Our edge belongs to the second curve section.
            p0=tcsPts01[1].xy, p1=tcsPts01[1].zw, p2=tcsPt2Tan0[1].xy, p3=tcsPts01[2].xy;
            tan0 = tcsPt2Tan0[1].zw;
            numParametricSegments = tcsTessArgs[1].y;
            radsPerSegment = tcsTessArgs[1].z;
        } else {
            // Our edge belongs to the third curve section.
            combinedEdgeID -= tcsTessArgs[1].x;
            p0=tcsPts01[2].xy, p1=tcsPts01[2].zw, p2=tcsPt2Tan0[2].xy, p3=tcsEndPtEndTan.xy;
            tan0 = tcsPt2Tan0[2].zw;
            numParametricSegments = tcsTessArgs[2].y;
            radsPerSegment = tcsTessArgs[2].z;
        }
        float2 tan1 = tcsEndPtEndTan.zw;
        bool isFinalEdge = (gl_TessCoord.x == 1);
        float w = -1.0;  // w<0 means the curve is an integral cubic.
        if (isinf(p3.y)) {
            w = p3.x;  // The curve is actually a conic.
            p3 = p2;  // Setting p3 equal to p2 works for the remaining rotational logic.
        })");

    GrGPArgs gpArgs;
    this->emitTessellationCode(shader, &code, &gpArgs, shaderCaps);

    // Manually map the position to OpenGL clip space, since we are generating raw GLSL.
    code.appendf(R"(
        gl_Position = vec4(%s * sk_RTAdjust.xz + sk_RTAdjust.yw, 0.0, 1.0);)",
        gpArgs.fPositionVar.c_str());

    if (shader.hasDynamicColor()) {
        // Pass color on to the fragment shader.
        code.appendf(R"(
        %s = tcsColor;)", fDynamicColorName.c_str());
    }

    code.append(R"(
    })");

    return code;
}

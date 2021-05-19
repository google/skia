/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStrokeInstancedShaderImpl.h"

#include "src/gpu/geometry/GrWangsFormula.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/tessellate/GrStrokeTessellator.h"

void GrStrokeInstancedShaderImpl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const auto& shader = args.fGeomProc.cast<GrStrokeShader>();
    SkPaint::Join joinType = shader.stroke().getJoin();
    args.fVaryingHandler->emitAttributes(shader);

    // Constants.
    args.fVertBuilder->defineConstant("MAX_PARAMETRIC_SEGMENTS_LOG2",
                                      GrTessellationPathRenderer::kMaxResolveLevel);
    args.fVertBuilder->defineConstant("float", "PI", "3.141592653589793238");

    // Helper functions.
    if (shader.hasDynamicStroke()) {
        args.fVertBuilder->insertFunction(kNumRadialSegmentsPerRadianFn);
    }
    args.fVertBuilder->insertFunction(kAtan2Fn);
    args.fVertBuilder->insertFunction(kCosineBetweenVectorsFn);
    args.fVertBuilder->insertFunction(kMiterExtentFn);
    args.fVertBuilder->insertFunction(kUncheckedMixFn);
    args.fVertBuilder->insertFunction(GrWangsFormula::as_sksl(shader.hasConics()).c_str());

    // Tessellation control uniforms and/or dynamic attributes.
    if (!shader.hasDynamicStroke()) {
        // [PARAMETRIC_PRECISION, NUM_RADIAL_SEGMENTS_PER_RADIAN, JOIN_TYPE, STROKE_RADIUS]
        const char* tessArgsName;
        fTessControlArgsUniform = args.fUniformHandler->addUniform(
                nullptr, kVertex_GrShaderFlag, kFloat4_GrSLType, "tessControlArgs",
                &tessArgsName);
        args.fVertBuilder->codeAppendf(R"(
        float PARAMETRIC_PRECISION = %s.x;
        float NUM_RADIAL_SEGMENTS_PER_RADIAN = %s.y;
        float JOIN_TYPE = %s.z;
        float STROKE_RADIUS = %s.w;)", tessArgsName, tessArgsName, tessArgsName, tessArgsName);
    } else {
        const char* parametricPrecisionName;
        fTessControlArgsUniform = args.fUniformHandler->addUniform(
                nullptr, kVertex_GrShaderFlag, kFloat_GrSLType, "parametricPrecision",
                &parametricPrecisionName);
        args.fVertBuilder->codeAppendf(R"(
        float PARAMETRIC_PRECISION = %s;
        float STROKE_RADIUS = dynamicStrokeAttr.x;
        float NUM_RADIAL_SEGMENTS_PER_RADIAN = num_radial_segments_per_radian(
                PARAMETRIC_PRECISION, STROKE_RADIUS);
        float JOIN_TYPE = dynamicStrokeAttr.y;)", parametricPrecisionName);
    }

    if (shader.hasDynamicColor()) {
        // Create a varying for color to get passed in through.
        GrGLSLVarying dynamicColor{kHalf4_GrSLType};
        args.fVaryingHandler->addVarying("dynamicColor", &dynamicColor);
        args.fVertBuilder->codeAppendf("%s = dynamicColorAttr;", dynamicColor.vsOut());
        fDynamicColorName = dynamicColor.fsIn();
    }

    if (shader.mode() == GrStrokeShader::Mode::kLog2Indirect) {
        args.fVertBuilder->codeAppend(R"(
        float NUM_TOTAL_EDGES = abs(argsAttr.z);)");
    } else {
        SkASSERT(shader.mode() == GrStrokeShader::Mode::kFixedCount);
        const char* edgeCountName;
        fEdgeCountUniform = args.fUniformHandler->addUniform(
                nullptr, kVertex_GrShaderFlag, kFloat_GrSLType, "edgeCount", &edgeCountName);
        args.fVertBuilder->codeAppendf(R"(
        float NUM_TOTAL_EDGES = %s;)", edgeCountName);
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
    if (shader.stroke().isHairlineStyle() && !shader.viewMatrix().isIdentity()) {
        // Hairline case. Transform the points before tessellation. We can still hold off on the
        // translate until the end; we just need to perform the scale and skew right now.
        args.fVertBuilder->codeAppend(R"(
        P = AFFINE_MATRIX * P;
        lastControlPoint = AFFINE_MATRIX * lastControlPoint;)");
    }

    args.fVertBuilder->codeAppend(R"(
    // Find how many parametric segments this stroke requires.
    float numParametricSegments = min(wangs_formula(PARAMETRIC_PRECISION,
                                                    P[0], P[1], P[2], P[3], w),
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

    // Potential optimization: (shader.hasDynamicStroke() && shader.hasRoundJoins())?
    if (shader.stroke().getJoin() == SkPaint::kRound_Join || shader.hasDynamicStroke()) {
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
        // "NUM_TOTAL_EDGES - 2".
        numEdgesInJoin = min(numEdgesInJoin, NUM_TOTAL_EDGES - 2);)");
        if (shader.mode() == GrStrokeShader::Mode::kLog2Indirect) {
            args.fVertBuilder->codeAppend(R"(
            // Negative argsAttr.z means the join is an internal chop or circle, and both of
            // those have empty joins. All we need is a bevel join.
            if (argsAttr.z < 0) {
                // +2 because we emit the beginning and ending edges twice (see above comment).
                numEdgesInJoin = 1 + 2;
            })");
        }
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
        float numEdgesInJoin = %i;)", GrStrokeShader::NumFixedEdgesInJoin(joinType));
    }

    args.fVertBuilder->codeAppend(R"(
    // Find which direction the curve turns.
    // NOTE: Since the curve is not allowed to inflect, we can just check F'(.5) x F''(.5).
    // NOTE: F'(.5) x F''(.5) has the same sign as (P2 - P0) x (P3 - P1)
    float turn = cross(P[2] - P[0], P[3] - P[1]);
    float combinedEdgeID = float(sk_VertexID >> 1) - numEdgesInJoin;
    if (combinedEdgeID < 0) {
        tan1 = tan0;
        // Don't let tan0 become zero. The code as-is isn't built to handle that case. tan0=0
        // means the join is disabled, and to disable it with the existing code we can leave
        // tan0 equal to tan1.
        if (lastControlPoint != P[0]) {
            tan0 = P[0] - lastControlPoint;
        }
        turn = cross(tan0, tan1);
    }

    // Calculate the curve's starting angle and rotation.
    float cosTheta = cosine_between_vectors(tan0, tan1);
    float rotation = acos(cosTheta);
    if (turn < 0) {
        // Adjust sign of rotation to match the direction the curve turns.
        rotation = -rotation;
    }

    float numRadialSegments;
    float strokeOutset = ((sk_VertexID & 1) == 0) ? +1 : -1;
    if (combinedEdgeID < 0) {
        // We belong to the preceding join. The first and final edges get duplicated, so we only
        // have "numEdgesInJoin - 2" segments.
        numRadialSegments = numEdgesInJoin - 2;
        numParametricSegments = 1;  // Joins don't have parametric segments.
        P = float4x2(P[0], P[0], P[0], P[0]);  // Colocate all points on the junction point.
        // Shift combinedEdgeID to the range [-1, numRadialSegments]. This duplicates the first
        // edge and lands one edge at the very end of the join. (The duplicated final edge will
        // actually come from the section of our strip that belongs to the stroke.)
        combinedEdgeID += numRadialSegments + 1;
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
                strokeOutset = (turn < 0) ? min(strokeOutset, 0) : max(strokeOutset, 0);
            }
        }
        combinedEdgeID = max(combinedEdgeID, 0);
    } else {
        // We belong to the stroke.
        float maxCombinedSegments = NUM_TOTAL_EDGES - numEdgesInJoin - 1;
        numRadialSegments = max(ceil(abs(rotation) * NUM_RADIAL_SEGMENTS_PER_RADIAN), 1);
        numRadialSegments = min(numRadialSegments, maxCombinedSegments);
        numParametricSegments = min(numParametricSegments,
                                    maxCombinedSegments - numRadialSegments + 1);
    }

    // Additional parameters for emitTessellationCode().
    float angle0 = atan2(tan0);
    float radsPerSegment = rotation / numRadialSegments;
    float numCombinedSegments = numParametricSegments + numRadialSegments - 1;
    bool isFinalEdge = (combinedEdgeID >= numCombinedSegments);
    if (combinedEdgeID > numCombinedSegments) {
        strokeOutset = 0;  // The strip has more edges than we need. Drop this one.
    })");

    if (joinType == SkPaint::kMiter_Join || shader.hasDynamicStroke()) {
        args.fVertBuilder->codeAppendf(R"(
        // Vertices #4 and #5 belong to the edge of the join that extends to the miter point.
        if ((sk_VertexID | 1) == (4 | 5) && %s) {
            strokeOutset *= miter_extent(cosTheta, JOIN_TYPE/*miterLimit*/);
        })", shader.hasDynamicStroke() ? "JOIN_TYPE > 0/*Is the join a miter type?*/" : "true");
    }

    this->emitTessellationCode(shader, &args.fVertBuilder->code(), gpArgs, *args.fShaderCaps);

    this->emitFragmentCode(shader, args);
}

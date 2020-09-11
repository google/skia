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

        // The vertex shader chops the curve into 3 sections in order to meet our tessellation
        // requirements. The stroke tessellator does not allow curve sections to inflect or to
        // rotate more than 180 degrees.
        //
        // We start by chopping at inflections (if the curve has any), or else at midtangent. If we
        // still don't have 3 sections after that then we just subdivide uniformly in parametric
        // space.
        using TypeModifier = GrShaderVar::TypeModifier;
        auto* v = args.fVertBuilder;
        v->defineConstantf("float", "kParametricEpsilon", "1.0 / (%i * 128)",
                           args.fShaderCaps->maxTessellationSegments());  // 1/128 of a segment.
        v->defineConstantf("float", "kStandardCubicType", "%.0f", Patch::kStandardCubicType);
        // Declare outputs to the tessellation control shader.
        v->declareGlobal(GrShaderVar("vsPts01", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsPts23", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsPts45", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsPts67", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsPts89", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsTans01", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsTans23", kFloat4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsTurnDirsAndPatchType", kHalf4_GrSLType, TypeModifier::Out));
        v->declareGlobal(GrShaderVar("vsStrokeRadius", kFloat_GrSLType, TypeModifier::Out));
        v->codeAppendf(R"(
        // Unpack the control points.
        float4x2 P = float4x2(inputPts01, inputPts23);
        half patchType = half(inputArgs.x);

        // Find the beginning and ending tangents. It's imperative that we compute these tangents
        // form the original input points or else the seams might crack.
        // (This works for now because P0=P1=P2 and/or P1=P2=P3 are both illegal.)
        float2 tan0 = (P[1] == P[0]) ? P[2] - P[0] : P[1] - P[0];
        float2 tan1 = (P[3] == P[2]) ? P[3] - P[1] : P[3] - P[2];

        // Find the cubic's power basis coefficient matrix "C":
        //
        //                                      |Cx  Cy|
        //     Cubic(T) = x,y = |T^3 T^2 T 1| * |.   . |
        //                                      |.   . |
        //                                      |.   . |
        //
        float2x4 C = float4x4(-1,  3, -3,  1,
                               3, -6,  3,  0,
                              -3,  3,  0,  0,
                               1,  0,  0,  0) * transpose(P);

        // Find the curve's inflection function. There are inflections at I==0.
        // See: https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf
        //
        //     Inflections are found where:  dot(|T^2 T 1|, I) == 0
        //
        float3 I = float3(-3*determinant(float2x2(C)),
                          -3*determinant(float2x2(C[0].xz, C[1].xz)),
                            -determinant(float2x2(C[0].yz, C[1].yz)));

        // Formulate a quadratic that finds the infection points.
        float a=I.x, b=I.y, c=I.z;
        float discr = b*b - 4*a*c;

        float2 midtangent = float2(0);
        if (discr <= 0) {
            // The curve does not inflect. Still chop, but at midtangent instead. This guarantees
            // the curve does not rotate more that 180 degrees.
            //
            // Start by finding u=tan0, v=-tan1. The midtangent is orthogonal to their bisector.
            float2 u=normalize(tan0), v=-normalize(tan1);

            if (dot(u,v) < 0) {
                // u,v are >90 degrees apart. Find the bisector of their normals instead. (After 90
                // degrees, the normals start cancelling each other out.)
                u = float2(-u.y, +u.x);
                v = float2(+v.y, -v.x);
            }

            float2 bisector = u + v;

            // The midtangent is orthogonal to the bisector. Its T value can be found as:
            //
            //   dot(midtangent, bisector) == 0:  |3*T^2  2*T  1| * float2x3(C) * bisector == 0
            //
            //                                                    |3    |
            // The quadratic formula coefficients are therefore:  |  2  | * float2x3(C) * bisector.
            //                                                    |    1|
            float3 coeffs = float2x3(C) * bisector;
            if (coeffs == float3(0)) {
                // The control points are exactly collinear and rotate either 0 or 360 degrees:
                // tangent is perpendicular to the bisector at every parametric point for which a
                // tangent is defined. Search instead for any locations where the flat curve does a
                // 180 degree turn.
                coeffs = float2x3(C) * tan0;
            }
            a=3*coeffs.x, b=2*coeffs.y, c=coeffs.z;

            // Every curve has a midtangent. If discr is less than zero it's due to FP error. Don't
            // let it fall below zero.
            discr = max(b*b - 4*a*c, 0);

            midtangent = float2(-bisector.y, bisector.x);
        }

        // We now have a quadratic formulated to chop the curve at the T values that meet our
        // tessellation requirements: no inflections, no rotations >180 degrees. Solve for T.
        float x = sqrt(discr);
        if (b < 0) {
            x = -x;
        }
        float q = -.5 * (b + x);
        float2 roots = float2((a != 0) ? q/a : 0,
                              (q != 0) ? c/q : 0);
        roots = (roots[0] <= roots[1]) ? roots : roots.ts;  // Sort.

        // Decide on two chop points for the curve that are inside 0..1. Start with the roots we
        // just found, then subdivide uniformly in parametric space if needed.
        float2 chopT = roots;
        if (chopT[0] <= kParametricEpsilon) {
            if (chopT[1] <= kParametricEpsilon) {
                chopT[1] = 2/3.0;
            }
            chopT[0] = chopT[1];
        }
        if (chopT[1] >= 1 - kParametricEpsilon) {
            if (chopT[0] >= 1 - kParametricEpsilon) {
                chopT[0] = 1/3.0;
            }
            chopT[1] = chopT[0];
        }
        if (chopT[0] == chopT[1]) {
            // The chop points are colocated. Split the larger section in half.
            if (chopT[0] > .5) {
                chopT[0] *= .5;
            } else {
                chopT[1] = fma(chopT[1], .5, .5);
            }
        }
        if (patchType != kStandardCubicType) {
            // The patch is actually a flat line or join. Don't chop it into sections after all.
            chopT = float2(0);
        }

        // Chop the curve at T0.
        float2 ab = mix(P[0], P[1], chopT[0]);
        float2 bc = mix(P[1], P[2], chopT[0]);
        float2 cd = mix(P[2], P[3], chopT[0]);
        float2 abc = mix(ab, bc, chopT[0]);
        float2 bcd = mix(bc, cd, chopT[0]);
        float2 abcd = mix(abc, bcd, chopT[0]);

        // Chop the curve in reverse at T1.
        float2 wz = mix(P[2], P[3], chopT[1]);
        float2 zy = mix(P[1], P[2], chopT[1]);
        float2 yx = mix(P[0], P[1], chopT[1]);
        float2 wzy = mix(zy, wz, chopT[1]);
        float2 zyx = mix(yx, zy, chopT[1]);
        float2 wzyx = mix(zyx, wzy, chopT[1]);

        // Find tangents at the chop points.
        float2 innerTan0 = (chopT[0] != 0) ? bcd - abc : tan0;
        float2 innerTan1 = (chopT[1] != 0) ? wzy - zyx : tan0;

        // Figure out which direction the curve turns as T approaches infinity.
        float inflectValAtInf = (I[0] != 0) ? I[0] : (I[1] != 0) ? I[1] : I[2];
        half turnDirAtInf = (inflectValAtInf >= 0) ? +1 : -1;
        half3 turnDirs;  // Will hold the turn directions for each section we just chopped.

        if (patchType != kStandardCubicType) {
            // The patch is a flat line or join. We didn't actually subdivide it.
            innerTan0 = innerTan1 = tan0;
            // The input points for joins aren't actually a cubic, but the math still works out that
            // we can use the inflection function to decide which direction the join turns.
            turnDirs = half3(-turnDirAtInf);
        } else if (midtangent != float2(0)) {
            // The curve did not inflect so we chopped at midtangent. The parametric definition of
            // tangent can be undefined at points that divide rotation in half, so use midtangent
            // instead at these locations.
            if (chopT[0] == roots[0] || chopT[0] == roots[1]) {
                float2 secondDerivate = float2(3*chopT[0], 1) * float2x2(C);
                float midtangentTurn = determinant(float2x2(midtangent, secondDerivate));
                innerTan0 = (midtangentTurn * turnDirAtInf >= 0) ? +midtangent : -midtangent;
            }
            if (chopT[1] == roots[0] || chopT[1] == roots[1]) {
                float2 secondDerivate = float2(3*chopT[1], 1) * float2x2(C);
                float midtangentTurn = determinant(float2x2(midtangent, secondDerivate));
                innerTan1 = (midtangentTurn * turnDirAtInf >= 0) ? +midtangent : -midtangent;
            }
            // Non-inflecting curves always turn in the same direction.
            turnDirs = half3(turnDirAtInf);
        } else {
            // We just chopped at inflection points so it is most stable to actually evaluate the
            // inflection function at the center of each section.
            float3 turnT = mix(float3(0, chopT), float3(chopT, 1), .5);
            turnDirs = half3(sign(float3x3(turnT*turnT, turnT, 1,1,1) * I));
        }

        // Package arguments for the tessellation control stage.
        vsPts01 = float4(P[0], ab);
        vsPts23 = float4(abc, abcd);
        vsPts45 = float4(mix(abcd, bcd, (chopT[1] - chopT[0]) / (1 - chopT[0])),
                       mix(zyx, wzyx, chopT[0] / chopT[1]));
        vsPts67 = float4(wzyx, wzy);
        vsPts89 = float4(wz, P[3]);
        vsTans01 = float4(tan0, innerTan0);
        vsTans23 = float4(innerTan1, tan1);
        vsTurnDirsAndPatchType = half4(turnDirs, patchType);
        vsStrokeRadius = inputArgs.y;
        )");

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
    // Run 3 invocations: 1 for each section that the vertex shader chopped the curve into.
    code.append("layout(vertices = 3) out;\n");

    code.appendf("const float kPI = 3.141592653589793238;\n");
    code.appendf("const float kMaxTessellationSegments = %i;\n",
                 shaderCaps.maxTessellationSegments());
    code.appendf("const float kStandardCubicType = %f;\n", Patch::kStandardCubicType);
    code.appendf("const float kMiterJoinType = %.0f;\n", Patch::kMiterJoinType);
    code.appendf("const float kBevelJoinType = %.0f;\n", Patch::kBevelJoinType);

    const char* tessControlArgsName = impl->getTessControlArgsUniformName(uniformHandler);
    code.appendf("uniform vec2 %s;\n", tessControlArgsName);
    code.appendf("#define uTolerance %s.x\n", tessControlArgsName);
    code.appendf("#define uMiterLimit %s.y\n", tessControlArgsName);

    code.append(R"(
    in vec4 vsPts01[];
    in vec4 vsPts23[];
    in vec4 vsPts45[];
    in vec4 vsPts67[];
    in vec4 vsPts89[];
    in vec4 vsTans01[];
    in vec4 vsTans23[];
    in mediump vec4 vsTurnDirsAndPatchType[];
    in float vsStrokeRadius[];

    out vec4 tcsPts01[];
    out vec4 tcsPt2Tan0[];
    out vec4 tcsTessArgs[];
    patch out vec4 tcsEndPtEndTan;
    patch out vec4 tcsStrokeArgs;

    // The built-in atan() is undefined when x==0. This method relieves that restriction, but also
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
        // Unpack the input arguments from the vertex shader.
        mat4x2 P;
        mat2 tangents;
        mediump float turnDir;
        if (gl_InvocationID == 0) {
            P = mat4x2(vsPts01[0], vsPts23[0]);
            tangents = mat2(vsTans01[0]);
            turnDir = vsTurnDirsAndPatchType[0].x;
        } else if (gl_InvocationID == 1) {
            P = mat4x2(vsPts23[0].zw, vsPts45[0], vsPts67[0].xy);
            tangents = mat2(vsTans01[0].zw, vsTans23[0].xy);
            turnDir = vsTurnDirsAndPatchType[0].y;
        } else {
            P = mat4x2(vsPts67[0], vsPts89[0]);
            tangents = mat2(vsTans23[0]);
            turnDir = vsTurnDirsAndPatchType[0].z;
        }
        mediump float patchType = vsTurnDirsAndPatchType[0].w;
        float strokeRadius = vsStrokeRadius[0];

        // Calculate the number of evenly spaced (in the parametric sense) segments to chop this
        // section of the curve into. (See GrWangsFormula::cubic() for more documentation on this
        // formula.) The final tessellated strip will be a composition of these parametric segments
        // as well as radial segments.
        float numParametricSegments = sqrt(
                .75/uTolerance * length(max(abs(P[2] - P[1]*2.0 + P[0]),
                                            abs(P[3] - P[2]*2.0 + P[1]))));
        numParametricSegments = max(ceil(numParametricSegments), 1);
        if (patchType != kStandardCubicType) {
            // Joins and flat lines don't need parametric segments.
            numParametricSegments = 1;
        }

        // Determine the curve's start angle.
        float angle0 = atan2(tangents[0]);

        // Determine the curve's total rotation. The vertex shader ensures our curve does not rotate
        // more than 180 degrees or inflect, so the inverse cosine has enough range.
        vec2 tan0norm = normalize(tangents[0]);
        vec2 tan1norm = normalize(tangents[1]);
        float cosTheta = dot(tan1norm, tan0norm);
        float rotation = acos(clamp(cosTheta, -1, +1));
        // Adjust sign of rotation to match the direction the curve turns.
        rotation *= turnDir;

        // Calculate the number of evenly spaced radial segments to chop this section of the curve
        // into. Radial segments divide the curve's rotation into even steps. The final tessellated
        // strip will be a composition of both parametric and radial segments.
        float numRadialSegments = abs(rotation) / (2 * acos(max(1 - uTolerance/strokeRadius, -1)));
        numRadialSegments = max(ceil(numRadialSegments), 1);

        // Set up joins.
        float innerStrokeRadius = 0;  // Used for miter joins.
        vec2 strokeOutsetClamp = vec2(-1, 1);
        float joinType = abs(patchType);
        if (joinType >= kBevelJoinType) {
            innerStrokeRadius = strokeRadius;  // A non-zero innerStrokeRadius designates a join.
            if (joinType == kMiterJoinType) {
                // Miter join. Draw a fan with 2 segments and lengthen the interior radius
                // so it reaches the miter point.
                // (Or draw a 1-segment fan if we exceed the miter limit.)
                float miterRatio = 1.0 / cos(.5 * rotation);
                numRadialSegments = (miterRatio <= uMiterLimit) ? 2.0 : 1.0;
                innerStrokeRadius = strokeRadius * miterRatio;
            } else if (joinType == kBevelJoinType) {
                // Bevel join. Make a fan with only one segment.
                numRadialSegments = 1;
            }
            if (length(tan0norm - tan1norm) * strokeRadius < uTolerance) {
                // The join angle is too tight to guarantee there won't be cracks on the interior
                // side of the junction. In case our join was intended to go on the exterior side
                // only, switch to a double sided bevel that ties all 4 incoming vertices together
                // like a bowtie. The join angle is so tight that bevels, miters, and rounds will
                // all look the same anyway.
                numRadialSegments = 1;
            } else if (patchType > 0) {
                // This join is single-sided. Clamp it to the outer side of the junction.
                strokeOutsetClamp = (rotation > 0) ? vec2(-1,0) : vec2(0,1);
            }
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

        // Pack the arguments for the evaluation stage.
        tcsPts01[gl_InvocationID] = vec4(P[0], P[1]);
        tcsPt2Tan0[gl_InvocationID] = vec4(P[2], tangents[0]);
        tcsTessArgs[gl_InvocationID] = vec4(numCombinedSegments, numParametricSegments, angle0,
                                            rotation / numRadialSegments);
        if (gl_InvocationID == 2) {
            tcsEndPtEndTan = vec4(P[3], tangents[1]);
            tcsStrokeArgs = vec4(strokeRadius, innerStrokeRadius, strokeOutsetClamp);
        } else if (patchType != kStandardCubicType) {
            // We don't actually chop flat lines or joins. Disable their 0th & 1st curve sections by
            // marking their number of segments as 0.
            tcsTessArgs[gl_InvocationID].x = 0;
        }

        barrier();

        // Tessellate a quad strip with enough segments for all 3 curve sections combined.
        float numTotalCombinedSegments = tcsTessArgs[0].x + tcsTessArgs[1].x + tcsTessArgs[2].x;
        gl_TessLevelInner[0] = numTotalCombinedSegments;
        gl_TessLevelInner[1] = 2.0;
        gl_TessLevelOuter[0] = 2.0;
        gl_TessLevelOuter[1] = numTotalCombinedSegments;
        gl_TessLevelOuter[2] = 2.0;
        gl_TessLevelOuter[3] = numTotalCombinedSegments;
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
    in vec4 tcsPts01[];
    in vec4 tcsPt2Tan0[];
    in vec4 tcsTessArgs[];
    patch in vec4 tcsEndPtEndTan;
    patch in vec4 tcsStrokeArgs;

    uniform vec4 sk_RTAdjust;

    void main() {
        // Our patch is composed of exactly "numTotalCombinedSegments + 1" stroke-width edges that
        // run orthogonal to the curve and make a strip of "numTotalCombinedSegments" quads.
        // Determine which discrete edge belongs to this invocation. An edge can either come from a
        // parametric segment or a radial one.
        float numTotalCombinedSegments = tcsTessArgs[0].x + tcsTessArgs[1].x + tcsTessArgs[2].x;
        float totalEdgeID = round(gl_TessCoord.x * numTotalCombinedSegments);

        // Furthermore, the vertex shader may have chopped the curve into 3 different sections.
        // Determine which section we belong to, and where we fall relative to its first edge.
        float localEdgeID = totalEdgeID;
        mat4x2 P;
        vec2 tan0;
        vec3 tessellationArgs;
        if (localEdgeID >= tcsTessArgs[0].x + tcsTessArgs[1].x) {
            localEdgeID -= tcsTessArgs[0].x + tcsTessArgs[1].x;
            P = mat4x2(tcsPts01[2], tcsPt2Tan0[2].xy, tcsEndPtEndTan.xy);
            tan0 = tcsPt2Tan0[2].zw;
            tessellationArgs = tcsTessArgs[2].yzw;
        } else if (localEdgeID >= tcsTessArgs[0].x) {
            localEdgeID -= tcsTessArgs[0].x;
            P = mat4x2(tcsPts01[1], tcsPt2Tan0[1].xy, tcsPts01[2].xy);
            tan0 = tcsPt2Tan0[1].zw;
            tessellationArgs = tcsTessArgs[1].yzw;
        } else {
            P = mat4x2(tcsPts01[0], tcsPt2Tan0[0].xy, tcsPts01[1].xy);
            tan0 = tcsPt2Tan0[0].zw;
            tessellationArgs = tcsTessArgs[0].yzw;
        }
        float numParametricSegments = tessellationArgs.x;
        float angle0 = tessellationArgs.y;
        float radsPerSegment = tessellationArgs.z;

        // Find a tangent matrix C' in power basis form. (This gives the derivative scaled by 1/3.)
        //
        //                                           |T^2|
        //     dx,dy (divided by 3) = tangent = C_ * |  T|
        //                                           |  1|
        mat3x2 C_ = P * mat3x4(-1,  3, -3,  1,
                                2, -4,  2,  0,
                               -1,  1,  0,  0);

        // Find the matrix C_s that produces an (arbitrarily scaled) tangent vector from a
        // parametric edge ID:
        //
        //           |parametricEdgeId^2|
        //     C_s * |  parametricEdgeId| = tangent * s
        //           |                 1|
        //
        mat3x2 C_s = mat3x2(C_[0], C_[1] * numParametricSegments,
                            C_[2] * numParametricSegments * numParametricSegments);

        // Run an O(log N) search to determine the highest parametric edge that is located on or
        // before the localEdgeID. A local edge ID is determined by the sum of complete parametric
        // and radial segments behind it. i.e., find the highest parametric edge where:
        //
        //    parametricEdgeID + floor(numRadialSegmentsAtParametricT) <= localEdgeID
        //
        float lastParametricEdgeID = 0;
        float maxParametricEdgeID = min(numParametricSegments - 1, localEdgeID);
        vec2 tan0norm = normalize(tan0);
        float negAbsRadsPerSegment = -abs(radsPerSegment);
        float maxRotation0 = (1 + localEdgeID) * abs(radsPerSegment);
        for (int exp = MAX_TESSELLATION_SEGMENTS_LOG2 - 1; exp >= 0; --exp) {
            // Test the parametric edge at lastParametricEdgeID + 2^exp.
            float testParametricID = lastParametricEdgeID + (1 << exp);
            if (testParametricID <= maxParametricEdgeID) {
                vec2 testTan = fma(vec2(testParametricID), C_s[0], C_s[1]);
                testTan = fma(vec2(testParametricID), testTan, C_s[2]);
                float cosRotation = dot(normalize(testTan), tan0norm);
                float maxRotation = fma(testParametricID, negAbsRadsPerSegment, maxRotation0);
                maxRotation = min(maxRotation, kPI);
                // Is rotation <= maxRotation? (i.e., is the number of complete radial segments
                // behind testT, + testParametricID <= localEdgeID?)
                // NOTE: We bias cos(maxRotation) downward for fp32 error. Otherwise a flat section
                // following a 180 degree turn might not render properly.
                if (cosRotation >= cos(maxRotation) - 1e-5) {
                    // testParametricID is on or before the localEdgeID. Keep it!
                    lastParametricEdgeID = testParametricID;
                }
            }
        }

        // Find the T value of the parametric edge at lastParametricEdgeID.
        float parametricT = lastParametricEdgeID / numParametricSegments;

        // Now that we've identified the highest parametric edge on or before the localEdgeID, the
        // highest radial edge is easy:
        float lastRadialEdgeID = localEdgeID - lastParametricEdgeID;
        float radialAngle = fma(lastRadialEdgeID, radsPerSegment, angle0);

        // Find the T value of the edge at lastRadialEdgeID. This is the point whose tangent angle
        // is equal to radialAngle, or whose tangent vector is orthogonal to "norm".
        vec2 tangent = vec2(cos(radialAngle), sin(radialAngle));
        vec2 norm = vec2(-tangent.y, tangent.x);

        // Find the T value where the cubic's tangent is orthogonal to norm:
        //
        //                                          |T^2|
        //   dot(tangent, norm) == 0:   norm * C_ * |  T| == 0
        //                                          |  1|
        //
        // The coeffs for the quadratic equation we need to solve are therefore: norm * C_.
        vec3 coeffs = norm * C_;
        float a=coeffs.x, b=coeffs.y, c=coeffs.z;

        // Quadratic formula from Numerical Recipes in C.
        float x = sqrt(max(b*b - 4*a*c, 0));
        if (b < 0) {
            x = -x;
        }
        float q = -.5 * (b + x);

        // Roots are q/a and c/q. Since each curve section does not inflect or rotate more than 180
        // degrees, there can only be one tangent orthogonal to "norm" inside 0..1. Pick the root
        // nearest 0.5.
        float _5qa = .5*q*a;
        vec2 root = (abs(q*q - _5qa) < abs(a*c - _5qa)) ? vec2(q,a) : vec2(c,q);
        float radialT = (root.t != 0) ? root.s / root.t : 0;
        radialT = clamp(radialT, 0, 1);

        if (lastRadialEdgeID == 0) {
            // The root finder above can become unstable when lastRadialEdgeID == 0 (e.g., if there
            // are roots at exatly 0 and 1 both). radialT should always == 0 in this case.
            radialT = 0;
        }

        // Now that we've identified the T values of the last parametric and radial edges, our final
        // T value for localEdgeID is whichever is larger.
        float T = max(parametricT, radialT);

        // Evaluate the cubic at T. Use De Casteljau's for its accuracy and stability.
        vec2 ab = mix(P[0], P[1], T);
        vec2 bc = mix(P[1], P[2], T);
        vec2 cd = mix(P[2], P[3], T);
        vec2 abc = mix(ab, bc, T);
        vec2 bcd = mix(bc, cd, T);
        vec2 position = mix(abc, bcd, T);

        // If we went with T=parametricT, then update the tangent. Otherwise leave it at the radial
        // tangent found previously. (In the event that parametricT == radialT, we keep the radial
        // tangent.)
        if (T != radialT) {
            tangent = bcd - abc;
        }

        float strokeRadius = tcsStrokeArgs.x;
        float innerStrokeRadius = tcsStrokeArgs.y;
        vec2 strokeOutsetClamp = tcsStrokeArgs.zw;

        if (localEdgeID == 0) {
            // The first local edge of each section uses the provided P[0] and tan0. This ensures
            // continuous rotation across chops made by the vertex shader as well as crack-free
            // seaming between patches.
            position = P[0];
            tangent = tan0;
        } else if (gl_TessCoord.x == 1) {
            // The final edge of the quad strip always uses the provided endPt and endTan. This
            // ensures crack-free seaming between patches.
            position = tcsEndPtEndTan.xy;
            tangent = tcsEndPtEndTan.zw;
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

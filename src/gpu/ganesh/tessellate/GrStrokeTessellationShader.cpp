/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/tessellate/GrStrokeTessellationShader.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMacros.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/tessellate/FixedCountBufferUtils.h"

#include <cmath>
#include <cstdint>

namespace {

// float2 robust_normalize_diff(float2 a, float b) { ... }
//
// Returns the normalized difference between a and b, i.e. normalize(a - b), with care taken for
// if 'a' and/or 'b' have large coordinates.
static const char* kRobustNormalizeDiffFn =
"float2 robust_normalize_diff(float2 a, float2 b) {"
    "float2 diff = a - b;"
    "if (diff == float2(0.0)) {"
        "return float2(0.0);"
    "} else {"
        "float invMag = 1.0 / max(abs(diff.x), abs(diff.y));"
        "return normalize(invMag * diff);"
    "}"
"}";

// float cosine_between_unit_vectors(float2 a, float2 b) { ...
//
// Returns the cosine of the angle between a and b, assuming a and b are unit vectors already.
// Guaranteed to be between [-1, 1].
static const char* kCosineBetweenUnitVectorsFn =
"float cosine_between_unit_vectors(float2 a, float2 b) {"
    // Since a and b are assumed to be normalized, the cosine is equal to the dot product, although
    // we clamp that to ensure it falls within the expected range of [-1, 1].
    "return clamp(dot(a, b), -1.0, 1.0);"
"}"
;


// float miter_extent(float cosTheta, float miterLimit) { ...
//
// Extends the middle radius to either the miter point, or the bevel edge if we surpassed the
// miter limit and need to revert to a bevel join.
static const char* kMiterExtentFn =
"float miter_extent(float cosTheta, float miterLimit) {"
    "float x = fma(cosTheta, .5, .5);"
    "return (x * miterLimit * miterLimit >= 1.0) ? inversesqrt(x) : sqrt(x);"
"}"
;

// float num_radial_segments_per_radian(float approxDevStrokeRadius) { ...
//
// Returns the number of radial segments required for each radian of rotation, in order for the
// curve to appear "smooth" as defined by the approximate device-space stroke radius.
static const char* kNumRadialSegmentsPerRadianFn =
"float num_radial_segments_per_radian(float approxDevStrokeRadius) {"
    "return .5 / acos(max(1.0 - (1.0 / PRECISION) / approxDevStrokeRadius, -1.0));"
"}";

// float<N> unchecked_mix(float<N> a, float<N> b, float<N> T) { ...
//
// Unlike mix(), this does not return b when t==1. But it otherwise seems to get better
// precision than "a*(1 - t) + b*t" for things like chopping cubics on exact cusp points.
// We override this result anyway when t==1 so it shouldn't be a problem.
static const char* kUncheckedMixFn =
"float unchecked_mix(float a, float b, float T) {"
    "return fma(b - a, T, a);"
"}"
"float2 unchecked_mix(float2 a, float2 b, float T) {"
    "return fma(b - a, float2(T), a);"
"}"
"float4 unchecked_mix(float4 a, float4 b, float4 T) {"
    "return fma(b - a, T, a);"
"}"
;

using skgpu::tess::FixedCountStrokes;

} // anonymous namespace

GrStrokeTessellationShader::GrStrokeTessellationShader(const GrShaderCaps& shaderCaps,
                                                       PatchAttribs attribs,
                                                       const SkMatrix& viewMatrix,
                                                       const SkStrokeRec& stroke,
                                                       SkPMColor4f color)
        : GrTessellationShader(kTessellate_GrStrokeTessellationShader_ClassID,
                               GrPrimitiveType::kTriangleStrip, viewMatrix, color)
        , fPatchAttribs(attribs | PatchAttribs::kJoinControlPoint)
        , fStroke(stroke) {
    // We should use explicit curve type when, and only when, there isn't infinity support.
    // Otherwise the GPU can infer curve type based on infinity.
    SkASSERT(shaderCaps.fInfinitySupport != (attribs & PatchAttribs::kExplicitCurveType));
    // pts 0..3 define the stroke as a cubic bezier. If p3.y is infinity, then it's a conic
    // with w=p3.x.
    //
    // An empty stroke (p0==p1==p2==p3) is a special case that denotes a circle, or
    // 180-degree point stroke.
    fAttribs.emplace_back("pts01Attr", kFloat4_GrVertexAttribType, SkSLType::kFloat4);
    fAttribs.emplace_back("pts23Attr", kFloat4_GrVertexAttribType, SkSLType::kFloat4);

    // argsAttr contains the lastControlPoint for setting up the join.
    fAttribs.emplace_back("argsAttr", kFloat2_GrVertexAttribType, SkSLType::kFloat2);

    if (fPatchAttribs & PatchAttribs::kStrokeParams) {
        fAttribs.emplace_back("dynamicStrokeAttr", kFloat2_GrVertexAttribType,
                              SkSLType::kFloat2);
    }
    if (fPatchAttribs & PatchAttribs::kColor) {
        fAttribs.emplace_back("dynamicColorAttr",
                              (fPatchAttribs & PatchAttribs::kWideColorIfEnabled)
                                      ? kFloat4_GrVertexAttribType
                                      : kUByte4_norm_GrVertexAttribType,
                              SkSLType::kHalf4);
    }
    if (fPatchAttribs & PatchAttribs::kExplicitCurveType) {
        // A conic curve is written out with p3=[w,Infinity], but GPUs that don't support
        // infinity can't detect this. On these platforms we write out an extra float with each
        // patch that explicitly tells the shader what type of curve it is.
        fAttribs.emplace_back("curveTypeAttr", kFloat_GrVertexAttribType, SkSLType::kFloat);
    }

    this->setInstanceAttributesWithImplicitOffsets(fAttribs.data(), fAttribs.size());
    SkASSERT(this->instanceStride() == sizeof(SkPoint) * 4 + PatchAttribsStride(fPatchAttribs));
    if (!shaderCaps.fVertexIDSupport) {
        constexpr static Attribute kVertexAttrib("edgeID", kFloat_GrVertexAttribType,
                                                    SkSLType::kFloat);
        this->setVertexAttributesWithImplicitOffsets(&kVertexAttrib, 1);
    }
    SkASSERT(fAttribs.size() <= kMaxAttribCount);
}

// This base class emits shader code for our parametric/radial stroke tessellation algorithm
// described above. The subclass emits its own specific setup code before calling into
// emitTessellationCode and emitFragment code.
class GrStrokeTessellationShader::Impl : public ProgramImpl {
    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    // Emits code that calculates the vertex position and any other inputs to the fragment shader.
    // The onEmitCode() is responsible to define the following symbols before calling this method:
    //
    //     // Functions.
    //     float2 unchecked_mix(float2, float2, float);
    //     float unchecked_mix(float, float, float);
    //
    //     // Values provided by either uniforms or attribs.
    //     float2 p0, p1, p2, p3;
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
    //     float2 tan0; // Must be pre-normalized
    //     float2 tan1; // Must be pre-normalized
    //     float strokeOutset;
    //
    void emitTessellationCode(const GrStrokeTessellationShader& shader, SkString* code,
                              GrGPArgs* gpArgs, const GrShaderCaps& shaderCaps) const;

    // Emits all necessary fragment code. If using dynamic color, the impl is responsible to set up
    // a half4 varying for color and provide its name in 'fDynamicColorName'.
    void emitFragmentCode(const GrStrokeTessellationShader&, const EmitArgs&);

    void setData(const GrGLSLProgramDataManager& pdman, const GrShaderCaps&,
                 const GrGeometryProcessor&) final;

    GrGLSLUniformHandler::UniformHandle fTessControlArgsUniform;
    GrGLSLUniformHandler::UniformHandle fTranslateUniform;
    GrGLSLUniformHandler::UniformHandle fAffineMatrixUniform;
    GrGLSLUniformHandler::UniformHandle fColorUniform;
    SkString fDynamicColorName;
};

void GrStrokeTessellationShader::Impl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const auto& shader = args.fGeomProc.cast<GrStrokeTessellationShader>();
    SkPaint::Join joinType = shader.stroke().getJoin();
    args.fVaryingHandler->emitAttributes(shader);

    args.fVertBuilder->defineConstant("float", "PI", "3.141592653589793238");
    args.fVertBuilder->defineConstant("PRECISION", skgpu::tess::kPrecision);
    // There is an artificial maximum number of edges (compared to the max limit calculated based on
    // the number of radial segments per radian, Wang's formula, and join type). When there is
    // vertex ID support, the limit is what can be represented in a uint16; otherwise the limit is
    // the size of the fallback vertex buffer.
    float maxEdges = args.fShaderCaps->fVertexIDSupport ? FixedCountStrokes::kMaxEdges
                                                        : FixedCountStrokes::kMaxEdgesNoVertexIDs;
    args.fVertBuilder->defineConstant("NUM_TOTAL_EDGES", maxEdges);

    // Helper functions.
    if (shader.hasDynamicStroke()) {
        args.fVertBuilder->insertFunction(kNumRadialSegmentsPerRadianFn);
    }
    args.fVertBuilder->insertFunction(kRobustNormalizeDiffFn);
    args.fVertBuilder->insertFunction(kCosineBetweenUnitVectorsFn);
    args.fVertBuilder->insertFunction(kMiterExtentFn);
    args.fVertBuilder->insertFunction(kUncheckedMixFn);
    args.fVertBuilder->insertFunction(GrTessellationShader::WangsFormulaSkSL());

    // Tessellation control uniforms and/or dynamic attributes.
    if (!shader.hasDynamicStroke()) {
        // [NUM_RADIAL_SEGMENTS_PER_RADIAN, JOIN_TYPE, STROKE_RADIUS]
        const char* tessArgsName;
        fTessControlArgsUniform = args.fUniformHandler->addUniform(
                nullptr, kVertex_GrShaderFlag, SkSLType::kFloat3, "tessControlArgs",
                &tessArgsName);
        args.fVertBuilder->codeAppendf(
        "float NUM_RADIAL_SEGMENTS_PER_RADIAN = %s.x;"
        "float JOIN_TYPE = %s.y;"
        "float STROKE_RADIUS = %s.z;", tessArgsName, tessArgsName, tessArgsName);
    } else {
        // The shader does not currently support dynamic hairlines, so this case only needs to
        // configure NUM_RADIAL_SEGMENTS_PER_RADIAN based on the fixed maxScale and per-instance
        // stroke radius attribute that's defined in local space.
        SkASSERT(!shader.stroke().isHairlineStyle());
        const char* maxScaleName;
        fTessControlArgsUniform = args.fUniformHandler->addUniform(
                nullptr, kVertex_GrShaderFlag, SkSLType::kFloat, "maxScale",
                &maxScaleName);
        args.fVertBuilder->codeAppendf(
        "float STROKE_RADIUS = dynamicStrokeAttr.x;"
        "float JOIN_TYPE = dynamicStrokeAttr.y;"
        "float NUM_RADIAL_SEGMENTS_PER_RADIAN = num_radial_segments_per_radian("
                "%s * STROKE_RADIUS);", maxScaleName);

    }

    if (shader.hasDynamicColor()) {
        // Create a varying for color to get passed in through.
        GrGLSLVarying dynamicColor{SkSLType::kHalf4};
        args.fVaryingHandler->addVarying("dynamicColor", &dynamicColor);
        args.fVertBuilder->codeAppendf("%s = dynamicColorAttr;", dynamicColor.vsOut());
        fDynamicColorName = dynamicColor.fsIn();
    }

    // View matrix uniforms.
    const char* translateName, *affineMatrixName;
    fAffineMatrixUniform = args.fUniformHandler->addUniform(nullptr, kVertex_GrShaderFlag,
                                                            SkSLType::kFloat4, "affineMatrix",
                                                            &affineMatrixName);
    fTranslateUniform = args.fUniformHandler->addUniform(nullptr, kVertex_GrShaderFlag,
                                                         SkSLType::kFloat2, "translate",
                                                         &translateName);
    args.fVertBuilder->codeAppendf("float2x2 AFFINE_MATRIX = float2x2(%s.xy, %s.zw);\n",
                                   affineMatrixName, affineMatrixName);
    args.fVertBuilder->codeAppendf("float2 TRANSLATE = %s;\n", translateName);

    if (shader.hasExplicitCurveType()) {
        args.fVertBuilder->insertFunction(SkStringPrintf(
        "bool is_conic_curve() { return curveTypeAttr != %g; }",
            skgpu::tess::kCubicCurveType).c_str());
    } else {
        args.fVertBuilder->insertFunction(
        "bool is_conic_curve() { return isinf(pts23Attr.w); }");
    }

    // Tessellation code.
    args.fVertBuilder->codeAppend(
    "float2 p0=pts01Attr.xy, p1=pts01Attr.zw, p2=pts23Attr.xy, p3=pts23Attr.zw;"
    "float2 lastControlPoint = argsAttr.xy;"
    "float w = -1;"  // w<0 means the curve is an integral cubic.
    "if (is_conic_curve()) {"
        // Conics are 3 points, with the weight in p3.
        "w = p3.x;"
        "p3 = p2;"  // Setting p3 equal to p2 works for the remaining rotational logic.
    "}"
    );

    // Emit code to call Wang's formula to determine parametric segments. We do this before
    // transform points for hairlines so that it is consistent with how the CPU tested the control
    // points for chopping.
    args.fVertBuilder->codeAppend(
    // Find how many parametric segments this stroke requires.
    "float numParametricSegments;"
    "if (w < 0) {"
        "if (p0 == p1 && p2 == p3) {"
            "numParametricSegments = 1;" // a line
        "} else {"
            "numParametricSegments = wangs_formula_cubic(PRECISION, p0, p1, p2, p3, AFFINE_MATRIX);"
        "}"
    "} else {"
        "numParametricSegments = wangs_formula_conic(PRECISION,"
                                                    "AFFINE_MATRIX * p0,"
                                                    "AFFINE_MATRIX * p1,"
                                                    "AFFINE_MATRIX * p2, w);"
    "}"
    );

    if (shader.stroke().isHairlineStyle()) {
        // Hairline case. Transform the points before tessellation. We can still hold off on the
        // translate until the end; we just need to perform the scale and skew right now.
        args.fVertBuilder->codeAppend(
        "p0 = AFFINE_MATRIX * p0;"
        "p1 = AFFINE_MATRIX * p1;"
        "p2 = AFFINE_MATRIX * p2;"
        "p3 = AFFINE_MATRIX * p3;"
        "lastControlPoint = AFFINE_MATRIX * lastControlPoint;"
        );
    }

    args.fVertBuilder->codeAppend(
    // Find the starting and ending tangents.
    "float2 tan0 = robust_normalize_diff((p0 == p1) ? ((p1 == p2) ? p3 : p2) : p1, p0);"
    "float2 tan1 = robust_normalize_diff(p3, (p3 == p2) ? ((p2 == p1) ? p0 : p1) : p2);"
    "if (tan0 == float2(0)) {"
        // The stroke is a point. This special case tells us to draw a stroke-width circle as a
        // 180 degree point stroke instead.
        "tan0 = float2(1,0);"
        "tan1 = float2(-1,0);"
    "}"
    );

    if (args.fShaderCaps->fVertexIDSupport) {
        // If we don't have sk_VertexID support then "edgeID" already came in as a vertex attrib.
        args.fVertBuilder->codeAppend(
        "float edgeID = float(sk_VertexID >> 1);"
        "if ((sk_VertexID & 1) != 0) {"
            "edgeID = -edgeID;"
        "}"
        );
    }

    // Potential optimization: (shader.hasDynamicStroke() && shader.hasRoundJoins())?
    if (shader.stroke().getJoin() == SkPaint::kRound_Join || shader.hasDynamicStroke()) {
        args.fVertBuilder->codeAppend(
        // Determine how many edges to give to the round join. We emit the first and final edges
        // of the join twice: once full width and once restricted to half width. This guarantees
        // perfect seaming by matching the vertices from the join as well as from the strokes on
        // either side.
        "float2 prevTan = robust_normalize_diff(p0, lastControlPoint);"
        "float joinRads = acos(cosine_between_unit_vectors(prevTan, tan0));"
        "float numRadialSegmentsInJoin = max(ceil(joinRads * NUM_RADIAL_SEGMENTS_PER_RADIAN), 1);"
        // +2 because we emit the beginning and ending edges twice (see above comment).
        "float numEdgesInJoin = numRadialSegmentsInJoin + 2;"
        // The stroke section needs at least two edges. Don't assign more to the join than
        // "NUM_TOTAL_EDGES - 2". (This is only relevant when the ideal max edge count calculated
        // on the CPU had to be limited to NUM_TOTAL_EDGES in the draw call).
        "numEdgesInJoin = min(numEdgesInJoin, NUM_TOTAL_EDGES - 2);");
        if (shader.hasDynamicStroke()) {
            args.fVertBuilder->codeAppend(
            "if (JOIN_TYPE >= 0) {" // Is the join not a round type?
                // Bevel and miter joins get 1 and 2 segments respectively.
                // +2 because we emit the beginning and ending edges twice (see above comments).
                "numEdgesInJoin = sign(JOIN_TYPE) + 1 + 2;"
            "}");
        }
    } else {
        args.fVertBuilder->codeAppendf("float numEdgesInJoin = %i;",
        skgpu::tess::NumFixedEdgesInJoin(joinType));
    }

    args.fVertBuilder->codeAppend(
    // Find which direction the curve turns.
    // NOTE: Since the curve is not allowed to inflect, we can just check F'(.5) x F''(.5).
    // NOTE: F'(.5) x F''(.5) has the same sign as (P2 - P0) x (P3 - P1)
    "float turn = cross_length_2d(p2 - p0, p3 - p1);"
    "float combinedEdgeID = abs(edgeID) - numEdgesInJoin;"
    "if (combinedEdgeID < 0) {"
        "tan1 = tan0;"
        // Don't let tan0 become zero. The code as-is isn't built to handle that case. tan0=0
        // means the join is disabled, and to disable it with the existing code we can leave
        // tan0 equal to tan1.
        "if (lastControlPoint != p0) {"
            "tan0 = robust_normalize_diff(p0, lastControlPoint);"
        "}"
        "turn = cross_length_2d(tan0, tan1);"
    "}"

    // Calculate the curve's starting angle and rotation.
    "float cosTheta = cosine_between_unit_vectors(tan0, tan1);"
    "float rotation = acos(cosTheta);"
    "if (turn < 0) {"
        // Adjust sign of rotation to match the direction the curve turns.
        "rotation = -rotation;"
    "}"

    "float numRadialSegments;"
    "float strokeOutset = sign(edgeID);"
    "if (combinedEdgeID < 0) {"
        // We belong to the preceding join. The first and final edges get duplicated, so we only
        // have "numEdgesInJoin - 2" segments.
        "numRadialSegments = numEdgesInJoin - 2;"
        "numParametricSegments = 1;"  // Joins don't have parametric segments.
        "p3 = p2 = p1 = p0;"  // Colocate all points on the junction point.
        // Shift combinedEdgeID to the range [-1, numRadialSegments]. This duplicates the first
        // edge and lands one edge at the very end of the join. (The duplicated final edge will
        // actually come from the section of our strip that belongs to the stroke.)
        "combinedEdgeID += numRadialSegments + 1;"
        // We normally restrict the join on one side of the junction, but if the tangents are
        // nearly equivalent this could theoretically result in bad seaming and/or cracks on the
        // side we don't put it on. If the tangents are nearly equivalent then we leave the join
        // double-sided.
       " float sinEpsilon = 1e-2;"  // ~= sin(180deg / 3000)
        "bool tangentsNearlyParallel ="
                "(abs(turn) * inversesqrt(dot(tan0, tan0) * dot(tan1, tan1))) < sinEpsilon;"
        "if (!tangentsNearlyParallel || dot(tan0, tan1) < 0) {"
            // There are two edges colocated at the beginning. Leave the first one double sided
            // for seaming with the previous stroke. (The double sided edge at the end will
            // actually come from the section of our strip that belongs to the stroke.)
            "if (combinedEdgeID >= 0) {"
                "strokeOutset = (turn < 0) ? min(strokeOutset, 0) : max(strokeOutset, 0);"
            "}"
        "}"
        "combinedEdgeID = max(combinedEdgeID, 0);"
    "} else {"
        // We belong to the stroke. Unless NUM_RADIAL_SEGMENTS_PER_RADIAN is incredibly high,
        // clamping to maxCombinedSegments will be a no-op because the draw call was invoked with
        // sufficient vertices to cover the worst case scenario of 180 degree rotation.
        "float maxCombinedSegments = NUM_TOTAL_EDGES - numEdgesInJoin - 1;"
        "numRadialSegments = max(ceil(abs(rotation) * NUM_RADIAL_SEGMENTS_PER_RADIAN), 1);"
        "numRadialSegments = min(numRadialSegments, maxCombinedSegments);"
        "numParametricSegments = min(numParametricSegments,"
                                    "maxCombinedSegments - numRadialSegments + 1);"
    "}"

    // Additional parameters for emitTessellationCode().
    "float radsPerSegment = rotation / numRadialSegments;"
    "float numCombinedSegments = numParametricSegments + numRadialSegments - 1;"
    "bool isFinalEdge = (combinedEdgeID >= numCombinedSegments);"
    "if (combinedEdgeID > numCombinedSegments) {"
        "strokeOutset = 0;"  // The strip has more edges than we need. Drop this one.
    "}");

    if (joinType == SkPaint::kMiter_Join || shader.hasDynamicStroke()) {
        args.fVertBuilder->codeAppendf(
        // Edge #2 extends to the miter point.
        "if (abs(edgeID) == 2 && %s) {"
            "strokeOutset *= miter_extent(cosTheta, JOIN_TYPE);" // miterLimit
        "}", shader.hasDynamicStroke() ? "JOIN_TYPE > 0" /*Is the join a miter type?*/ : "true");
    }

    this->emitTessellationCode(shader, &args.fVertBuilder->code(), gpArgs, *args.fShaderCaps);

    this->emitFragmentCode(shader, args);
}

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
    //     float2 p0, p1, p2, p3;
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
    //     float2 tan0; // Must be pre-normalized
    //     float2 tan1; // Must be pre-normalized
    //     float strokeOutset;
    //
    code->appendf(
    "float2 tangent, strokeCoord;"
    "if (combinedEdgeID != 0 && !isFinalEdge) {"
        // Compute the location and tangent direction of the stroke edge with the integral id
        // "combinedEdgeID", where combinedEdgeID is the sorted-order index of parametric and radial
        // edges. Start by finding the tangent function's power basis coefficients. These define a
        // tangent direction (scaled by some uniform value) as:
        //                                                 |T^2|
        //     Tangent_Direction(T) = dx,dy = |A  2B  C| * |T  |
        //                                    |.   .  .|   |1  |
        "float2 A, B, C = p1 - p0;"
        "float2 D = p3 - p0;"
        "if (w >= 0.0) {"
            // P0..P2 represent a conic and P3==P2. The derivative of a conic has a cumbersome
            // order-4 denominator. However, this isn't necessary if we are only interested in a
            // vector in the same *direction* as a given tangent line. Since the denominator scales
            // dx and dy uniformly, we can throw it out completely after evaluating the derivative
            // with the standard quotient rule. This leaves us with a simpler quadratic function
            // that we use to find a tangent.
            "C *= w;"
            "B = .5*D - C;"
            "A = (w - 1.0) * D;"
            "p1 *= w;"
        "} else {"
            "float2 E = p2 - p1;"
            "B = E - C;"
            "A = fma(float2(-3), E, D);"
        "}"
        // FIXME(crbug.com/800804,skbug.com/40042642): Consider normalizing the exponents in A,B,C at
        // this point in order to prevent fp32 overflow.

        // Now find the coefficients that give a tangent direction from a parametric edge ID:
        //
        //                                                                 |parametricEdgeID^2|
        //     Tangent_Direction(parametricEdgeID) = dx,dy = |A  B_  C_| * |parametricEdgeID  |
        //                                                   |.   .   .|   |1                 |
        //
        "float2 B_ = B * (numParametricSegments * 2.0);"
        "float2 C_ = C * (numParametricSegments * numParametricSegments);"

        // Run a binary search to determine the highest parametric edge that is located on or before
        // the combinedEdgeID. A combined ID is determined by the sum of complete parametric and
        // radial segments behind it. i.e., find the highest parametric edge where:
        //
        //    parametricEdgeID + floor(numRadialSegmentsAtParametricT) <= combinedEdgeID
        //
        "float lastParametricEdgeID = 0.0;"
        "float maxParametricEdgeID = min(numParametricSegments - 1.0, combinedEdgeID);"
        "float negAbsRadsPerSegment = -abs(radsPerSegment);"
        "float maxRotation0 = (1.0 + combinedEdgeID) * abs(radsPerSegment);"
        "for (int exp = %i - 1; exp >= 0; --exp) {"
            // Test the parametric edge at lastParametricEdgeID + 2^exp.
            "float testParametricID = lastParametricEdgeID + exp2(float(exp));"
            "if (testParametricID <= maxParametricEdgeID) {"
                "float2 testTan = fma(float2(testParametricID), A, B_);"
                "testTan = fma(float2(testParametricID), testTan, C_);"
                "float cosRotation = dot(normalize(testTan), tan0);"
                "float maxRotation = fma(testParametricID, negAbsRadsPerSegment, maxRotation0);"
                "maxRotation = min(maxRotation, PI);"
                // Is rotation <= maxRotation? (i.e., is the number of complete radial segments
                // behind testT, + testParametricID <= combinedEdgeID?)
                "if (cosRotation >= cos(maxRotation)) {"
                    // testParametricID is on or before the combinedEdgeID. Keep it!
                    "lastParametricEdgeID = testParametricID;"
                "}"
            "}"
        "}"

        // Find the T value of the parametric edge at lastParametricEdgeID.
        "float parametricT = lastParametricEdgeID / numParametricSegments;"

        // Now that we've identified the highest parametric edge on or before the
        // combinedEdgeID, the highest radial edge is easy:
        "float lastRadialEdgeID = combinedEdgeID - lastParametricEdgeID;"

        // Find the angle of tan0, i.e. the angle between tan0 and the positive x axis.
        "float angle0 = acos(clamp(tan0.x, -1.0, 1.0));"
        "angle0 = tan0.y >= 0.0 ? angle0 : -angle0;"

        // Find the tangent vector on the edge at lastRadialEdgeID. By construction it is already
        // normalized.
        "float radialAngle = fma(lastRadialEdgeID, radsPerSegment, angle0);"
        "tangent = float2(cos(radialAngle), sin(radialAngle));"
        "float2 norm = float2(-tangent.y, tangent.x);"

        // Find the T value where the tangent is orthogonal to norm. This is a quadratic:
        //
        //     dot(norm, Tangent_Direction(T)) == 0
        //
        //                         |T^2|
        //     norm * |A  2B  C| * |T  | == 0
        //            |.   .  .|   |1  |
        //
        "float a=dot(norm,A), b_over_2=dot(norm,B), c=dot(norm,C);"
        "float discr_over_4 = max(b_over_2*b_over_2 - a*c, 0.0);"
        "float q = sqrt(discr_over_4);"
        "if (b_over_2 > 0.0) {"
            "q = -q;"
        "}"
        "q -= b_over_2;"

        // Roots are q/a and c/q. Since each curve section does not inflect or rotate more than 180
        // degrees, there can only be one tangent orthogonal to "norm" inside 0..1. Pick the root
        // nearest .5.
        "float _5qa = -.5*q*a;"
        "float2 root = (abs(fma(q,q,_5qa)) < abs(fma(a,c,_5qa))) ? float2(q,a) : float2(c,q);"
        "float radialT = (root.t != 0.0) ? root.s / root.t : 0.0;"
        "radialT = clamp(radialT, 0.0, 1.0);"

        "if (lastRadialEdgeID == 0.0) {"
            // The root finder above can become unstable when lastRadialEdgeID == 0 (e.g., if
            // there are roots at exatly 0 and 1 both). radialT should always == 0 in this case.
            "radialT = 0.0;"
        "}"

        // Now that we've identified the T values of the last parametric and radial edges, our final
        // T value for combinedEdgeID is whichever is larger.
        "float T = max(parametricT, radialT);"

        // Evaluate the cubic at T. Use De Casteljau's for its accuracy and stability.
        "float2 ab = unchecked_mix(p0, p1, T);"
        "float2 bc = unchecked_mix(p1, p2, T);"
        "float2 cd = unchecked_mix(p2, p3, T);"
        "float2 abc = unchecked_mix(ab, bc, T);"
        "float2 bcd = unchecked_mix(bc, cd, T);"
        "float2 abcd = unchecked_mix(abc, bcd, T);"

        // Evaluate the conic weight at T.
        "float u = unchecked_mix(1.0, w, T);"
        "float v = w + 1 - u;"  // == mix(w, 1, T)
        "float uv = unchecked_mix(u, v, T);"

        // If we went with T=parametricT, then update the tangent. Otherwise leave it at the radial
        // tangent found previously. (In the event that parametricT == radialT, we keep the radial
        // tangent.)
        "if (T != radialT) {"
            // We must re-normalize here because the tangent is determined by the curve coefficients
            "tangent = w >= 0.0 ? robust_normalize_diff(bc*u, ab*v)"
                               ": robust_normalize_diff(bcd, abc);"
        "}"

        "strokeCoord = (w >= 0.0) ? abc/uv : abcd;"
    "} else {"
        // Edges at the beginning and end of the strip use exact endpoints and tangents. This
        // ensures crack-free seaming between instances.
        "tangent = (combinedEdgeID == 0) ? tan0 : tan1;"
        "strokeCoord = (combinedEdgeID == 0) ? p0 : p3;"
    "}", skgpu::tess::kMaxResolveLevel /* Parametric/radial sort loop count. */);

    code->append(
    // At this point 'tangent' is normalized, so the orthogonal vector is also normalized.
    "float2 ortho = float2(tangent.y, -tangent.x);"
    "strokeCoord += ortho * (STROKE_RADIUS * strokeOutset);");

    if (!shader.stroke().isHairlineStyle()) {
        // Normal case. Do the transform after tessellation.
        code->append("float2 devCoord = AFFINE_MATRIX * strokeCoord + TRANSLATE;");
        gpArgs->fPositionVar.set(SkSLType::kFloat2, "devCoord");
        gpArgs->fLocalCoordVar.set(SkSLType::kFloat2, "strokeCoord");
    } else {
        // Hairline case. The scale and skew already happened before tessellation.
        code->append(
        "float2 devCoord = strokeCoord + TRANSLATE;"
        "float2 localCoord = inverse(AFFINE_MATRIX) * strokeCoord;");
        gpArgs->fPositionVar.set(SkSLType::kFloat2, "devCoord");
        gpArgs->fLocalCoordVar.set(SkSLType::kFloat2, "localCoord");
    }
}

void GrStrokeTessellationShader::Impl::emitFragmentCode(const GrStrokeTessellationShader& shader,
                                                        const EmitArgs& args) {
    if (!shader.hasDynamicColor()) {
        // The fragment shader just outputs a uniform color.
        const char* colorUniformName;
        fColorUniform = args.fUniformHandler->addUniform(nullptr, kFragment_GrShaderFlag,
                                                         SkSLType::kHalf4, "color",
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

    // getMaxScale() returns -1 if it can't compute a scale factor (e.g. perspective), taking the
    // absolute value automatically converts that to an identity scale factor for our purposes.
    const float maxScale = std::abs(shader.viewMatrix().getMaxScale());
    if (!shader.hasDynamicStroke()) {
        // Set up the tessellation control uniforms. In the hairline case we transform prior to
        // tessellation, so it will be defined in device space units instead of local units.
        const float strokeRadius = 0.5f * (stroke.isHairlineStyle() ? 1.f : stroke.getWidth());
        float numRadialSegmentsPerRadian = skgpu::tess::CalcNumRadialSegmentsPerRadian(
                (stroke.isHairlineStyle() ? 1.f : maxScale) * strokeRadius);

        pdman.set3f(fTessControlArgsUniform,
                    numRadialSegmentsPerRadian,  // NUM_RADIAL_SEGMENTS_PER_RADIAN
                    skgpu::tess::GetJoinType(stroke),  // JOIN_TYPE
                    strokeRadius);  // STROKE_RADIUS
    } else {
        SkASSERT(!stroke.isHairlineStyle());
        pdman.set1f(fTessControlArgsUniform, maxScale);
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

void GrStrokeTessellationShader::addToKey(const GrShaderCaps&, skgpu::KeyBuilder* b) const {
    bool keyNeedsJoin = !(fPatchAttribs & PatchAttribs::kStrokeParams);
    SkASSERT(fStroke.getJoin() >> 2 == 0);
    // Attribs get worked into the key automatically during GrGeometryProcessor::getAttributeKey().
    // When color is in a uniform, it's always wide. kWideColor doesn't need to be considered here.
    uint32_t key = (uint32_t)(fPatchAttribs & ~PatchAttribs::kColor);
    key = (key << 2) | ((keyNeedsJoin) ? fStroke.getJoin() : 0);
    key = (key << 1) | (uint32_t)fStroke.isHairlineStyle();
    b->add32(key);
}

std::unique_ptr<GrGeometryProcessor::ProgramImpl> GrStrokeTessellationShader::makeProgramImpl(
        const GrShaderCaps&) const {
    return std::make_unique<Impl>();
}

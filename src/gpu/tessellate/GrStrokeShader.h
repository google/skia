/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeShader_DEFINED
#define GrStrokeShader_DEFINED

#include "src/gpu/tessellate/GrPathShader.h"

#include "include/core/SkStrokeRec.h"
#include "src/gpu/GrVx.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

// Tessellates a batch of stroke patches directly to the canvas. Tessellated stroking works by
// creating stroke-width, orthogonal edges at set locations along the curve and then connecting them
// with a quad strip. These orthogonal edges come from two different sets: "parametric edges" and
// "radial edges". Parametric edges are spaced evenly in the parametric sense, and radial edges
// divide the curve's _rotation_ into even steps. The tessellation shader evaluates both sets of
// edges and sorts them into a single quad strip. With this combined set of edges we can stroke any
// curve, regardless of curvature.
class GrStrokeShader : public GrPathShader {
public:
    // Are we using hardware tessellation or indirect draws?
    enum class Mode {
        kHardwareTessellation,
        kLog2Indirect,
        kFixedCount
    };

    enum class ShaderFlags {
        kNone          = 0,
        kWideColor     = 1 << 0,
        kDynamicStroke = 1 << 1,  // Each patch or instance has its own stroke width and join type.
        kDynamicColor  = 1 << 2,  // Each patch or instance has its own color.
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(ShaderFlags);

    // Returns the fixed number of edges that are always emitted with the given join type. If the
    // join is round, the caller needs to account for the additional radial edges on their own.
    // Specifically, each join always emits:
    //
    //   * Two colocated edges at the beginning (a full-width edge to seam with the preceding stroke
    //     and a half-width edge to begin the join).
    //
    //   * An extra edge in the middle for miter joins, or else a variable number of radial edges
    //     for round joins (the caller is responsible for counting radial edges from round joins).
    //
    //   * A half-width edge at the end of the join that will be colocated with the first
    //     (full-width) edge of the stroke.
    //
    constexpr static int NumFixedEdgesInJoin(SkPaint::Join joinType) {
        switch (joinType) {
            case SkPaint::kMiter_Join:
                return 4;
            case SkPaint::kRound_Join:
                // The caller is responsible for counting the variable number of middle, radial
                // segments on round joins.
                [[fallthrough]];
            case SkPaint::kBevel_Join:
                return 3;
        }
        SkUNREACHABLE;
    }

    // We encode all of a join's information in a single float value:
    //
    //     Negative => Round Join
    //     Zero     => Bevel Join
    //     Positive => Miter join, and the value is also the miter limit
    //
    static float GetJoinType(const SkStrokeRec& stroke) {
        switch (stroke.getJoin()) {
            case SkPaint::kRound_Join: return -1;
            case SkPaint::kBevel_Join: return 0;
            case SkPaint::kMiter_Join: SkASSERT(stroke.getMiter() >= 0); return stroke.getMiter();
        }
        SkUNREACHABLE;
    }

    // This struct gets written out to each patch or instance if kDynamicStroke is enabled.
    struct DynamicStroke {
        static bool StrokesHaveEqualDynamicState(const SkStrokeRec& a, const SkStrokeRec& b) {
            return a.getWidth() == b.getWidth() && a.getJoin() == b.getJoin() &&
                   (a.getJoin() != SkPaint::kMiter_Join || a.getMiter() == b.getMiter());
        }
        void set(const SkStrokeRec& stroke) {
            fRadius = stroke.getWidth() * .5f;
            fJoinType = GetJoinType(stroke);
        }
        float fRadius;
        float fJoinType;  // See GetJoinType().
    };

    // 'viewMatrix' is applied to the geometry post tessellation. It cannot have perspective.
    GrStrokeShader(Mode mode, ShaderFlags shaderFlags, const SkMatrix& viewMatrix,
                   const SkStrokeRec& stroke, SkPMColor4f color)
            : GrPathShader(kTessellate_GrStrokeShader_ClassID, viewMatrix,
                           (mode == Mode::kHardwareTessellation) ?
                                   GrPrimitiveType::kPatches : GrPrimitiveType::kTriangleStrip,
                           (mode == Mode::kHardwareTessellation) ? 1 : 0)
            , fMode(mode)
            , fShaderFlags(shaderFlags)
            , fStroke(stroke)
            , fColor(color) {
        if (fMode == Mode::kHardwareTessellation) {
            // A join calculates its starting angle using prevCtrlPtAttr.
            fAttribs.emplace_back("prevCtrlPtAttr", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
            // pts 0..3 define the stroke as a cubic bezier. If p3.y is infinity, then it's a conic
            // with w=p3.x.
            //
            // If p0 == prevCtrlPtAttr, then no join is emitted.
            //
            // pts=[p0, p3, p3, p3] is a reserved pattern that means this patch is a join only,
            // whose start and end tangents are (p0 - inputPrevCtrlPt) and (p3 - p0).
            //
            // pts=[p0, p0, p0, p3] is a reserved pattern that means this patch is a "bowtie", or
            // double-sided round join, anchored on p0 and rotating from (p0 - prevCtrlPtAttr) to
            // (p3 - p0).
            fAttribs.emplace_back("pts01Attr", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
            fAttribs.emplace_back("pts23Attr", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        } else {
            // pts 0..3 define the stroke as a cubic bezier. If p3.y is infinity, then it's a conic
            // with w=p3.x.
            //
            // An empty stroke (p0==p1==p2==p3) is a special case that denotes a circle, or
            // 180-degree point stroke.
            fAttribs.emplace_back("pts01Attr", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
            fAttribs.emplace_back("pts23Attr", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
            if (fMode == Mode::kLog2Indirect) {
                // argsAttr.xy contains the lastControlPoint for setting up the join.
                //
                // "argsAttr.z=numTotalEdges" tells the shader the literal number of edges in the
                // triangle strip being rendered (i.e., it should be vertexCount/2). If
                // numTotalEdges is negative and the join type is "kRound", it also instructs the
                // shader to only allocate one segment the preceding round join.
                fAttribs.emplace_back("argsAttr", kFloat3_GrVertexAttribType, kFloat3_GrSLType);
            } else {
                SkASSERT(fMode == Mode::kFixedCount);
                // argsAttr contains the lastControlPoint for setting up the join.
                fAttribs.emplace_back("argsAttr", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
            }
        }
        if (fShaderFlags & ShaderFlags::kDynamicStroke) {
            fAttribs.emplace_back("dynamicStrokeAttr", kFloat2_GrVertexAttribType,
                                  kFloat2_GrSLType);
        }
        if (fShaderFlags & ShaderFlags::kDynamicColor) {
            fAttribs.emplace_back("dynamicColorAttr",
                                  (fShaderFlags & ShaderFlags::kWideColor)
                                          ? kFloat4_GrVertexAttribType
                                          : kUByte4_norm_GrVertexAttribType,
                                  kHalf4_GrSLType);
        }
        if (fMode == Mode::kHardwareTessellation) {
            this->setVertexAttributes(fAttribs.data(), fAttribs.count());
        } else {
            this->setInstanceAttributes(fAttribs.data(), fAttribs.count());
        }
        SkASSERT(fAttribs.count() <= kMaxAttribCount);
    }

    Mode mode() const { return fMode; }
    ShaderFlags flags() const { return fShaderFlags; }
    bool hasDynamicStroke() const { return fShaderFlags & ShaderFlags::kDynamicStroke; }
    bool hasDynamicColor() const { return fShaderFlags & ShaderFlags::kDynamicColor; }
    const SkStrokeRec& stroke() const { return fStroke;}
    const SkPMColor4f& color() const { return fColor;}
    float fixedCountNumTotalEdges() const { return fFixedCountNumTotalEdges;}

    // Used by GrFixedCountTessellator to configure the uniform value that tells the shader how many
    // total edges are in the triangle strip.
    void setFixedCountNumTotalEdges(int value) {
        SkASSERT(fMode == Mode::kFixedCount);
        fFixedCountNumTotalEdges = value;
    }

private:
    const char* name() const override { return "GrStrokeShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override;
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const final;

    const Mode fMode;
    const ShaderFlags fShaderFlags;
    const SkStrokeRec fStroke;
    const SkPMColor4f fColor;

    constexpr static int kMaxAttribCount = 5;
    SkSTArray<kMaxAttribCount, Attribute> fAttribs;

    // This is a uniform value used when fMode is kFixedCount that tells the shader how many total
    // edges are in the triangle strip.
    float fFixedCountNumTotalEdges = 0;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrStrokeShader::ShaderFlags);

// This common base class emits shader code for our parametric/radial stroke tessellation algorithm
// described above. The subclass emits its own specific setup code before calling into
// emitTessellationCode and emitFragment code.
class GrStrokeShaderImpl : public GrGLSLGeometryProcessor {
protected:
    // float atan2(float2 v) { ...
    //
    // The built-in atan() is undefined when x==0. This method relieves that restriction, but also
    // can return values larger than 2*PI. This shouldn't matter for our purposes.
    static const char* kAtan2Fn;

    // float cosine_between_vectors(float2 a, float2 b) { ...
    //
    // Returns dot(a, b) / (length(a) * length(b)).
    static const char* kCosineBetweenVectorsFn;

    // float miter_extent(float cosTheta, float miterLimit) { ...
    //
    // Extends the middle radius to either the miter point, or the bevel edge if we surpassed the
    // miter limit and need to revert to a bevel join.
    static const char* kMiterExtentFn;

    // float num_radial_segments_per_radian(float parametricPrecision, float strokeRadius) { ...
    //
    // Returns the number of radial segments required for each radian of rotation, in order for the
    // curve to appear "smooth" as defined by the parametricPrecision.
    static const char* kNumRadialSegmentsPerRadianFn;

    // float<N> unchecked_mix(float<N> a, float<N> b, float<N> T) { ...
    //
    // Unlike mix(), this does not return b when t==1. But it otherwise seems to get better
    // precision than "a*(1 - t) + b*t" for things like chopping cubics on exact cusp points.
    // We override this result anyway when t==1 so it shouldn't be a problem.
    static const char* kUncheckedMixFn;

    // Emits code that calculates the vertex position and any other inputs to the fragment shader.
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
    void emitTessellationCode(const GrStrokeShader& shader, SkString* code, GrGPArgs* gpArgs,
                              const GrShaderCaps& shaderCaps) const;

    // Emits all necessary fragment code. If using dynamic color, the impl is responsible to set up
    // a half4 varying for color and provide its name in 'fDynamicColorName'.
    void emitFragmentCode(const GrStrokeShader&, const EmitArgs&);

    void setData(const GrGLSLProgramDataManager& pdman, const GrShaderCaps&,
                 const GrGeometryProcessor&) final;

    GrGLSLUniformHandler::UniformHandle fTessControlArgsUniform;
    GrGLSLUniformHandler::UniformHandle fTranslateUniform;
    GrGLSLUniformHandler::UniformHandle fAffineMatrixUniform;
    GrGLSLUniformHandler::UniformHandle fEdgeCountUniform;
    GrGLSLUniformHandler::UniformHandle fColorUniform;
    SkString fDynamicColorName;
};

#endif

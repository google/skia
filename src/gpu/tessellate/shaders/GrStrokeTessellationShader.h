/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeTessellationShader_DEFINED
#define GrStrokeTessellationShader_DEFINED

#include "src/gpu/tessellate/shaders/GrTessellationShader.h"

#include "include/core/SkStrokeRec.h"
#include "src/gpu/GrVx.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

// Tessellates a batch of stroke patches directly to the canvas. Tessellated stroking works by
// creating stroke-width, orthogonal edges at set locations along the curve and then connecting them
// with a quad strip. These orthogonal edges come from two different sets: "parametric edges" and
// "radial edges". Parametric edges are spaced evenly in the parametric sense, and radial edges
// divide the curve's _rotation_ into even steps. The tessellation shader evaluates both sets of
// edges and sorts them into a single quad strip. With this combined set of edges we can stroke any
// curve, regardless of curvature.
class GrStrokeTessellationShader : public GrTessellationShader {
public:
    // Are we using hardware tessellation or indirect draws?
    enum class Mode : int8_t {
        kHardwareTessellation,
        kLog2Indirect,
        kFixedCount
    };

    enum class ShaderFlags : uint8_t {
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
    GrStrokeTessellationShader(const GrShaderCaps&, Mode, ShaderFlags, const SkMatrix& viewMatrix,
                               const SkStrokeRec&, SkPMColor4f, int8_t maxParametricSegments_log2);

    Mode mode() const { return fMode; }
    ShaderFlags flags() const { return fShaderFlags; }
    bool hasDynamicStroke() const { return fShaderFlags & ShaderFlags::kDynamicStroke; }
    bool hasDynamicColor() const { return fShaderFlags & ShaderFlags::kDynamicColor; }
    const SkStrokeRec& stroke() const { return fStroke;}
    int8_t maxParametricSegments_log2() const { return fMaxParametricSegments_log2; }
    float fixedCountNumTotalEdges() const { return fFixedCountNumTotalEdges;}

    // Used by GrFixedCountTessellator to configure the uniform value that tells the shader how many
    // total edges are in the triangle strip.
    void setFixedCountNumTotalEdges(int value) {
        SkASSERT(fMode == Mode::kFixedCount);
        fFixedCountNumTotalEdges = value;
    }

    // Initializes the fallback vertex buffer that should be bound when drawing in Mode::kFixedCount
    // and sk_VertexID is not supported. Each vertex is a single float and each edge is composed of
    // two vertices, so the desired edge count in the buffer is presumed to be
    // "bufferSize / (sizeof(float) * 2)". The caller cannot draw more vertices than edgeCount * 2.
    static void InitializeVertexIDFallbackBuffer(GrVertexWriter vertexWriter, size_t bufferSize);

private:
    const char* name() const override {
        switch (fMode) {
            case Mode::kHardwareTessellation:
                return "GrStrokeTessellationShader_HardwareImpl";
            case Mode::kLog2Indirect:
            case Mode::kFixedCount:
                return "GrStrokeTessellationShader_InstancedImpl";
        }
        SkUNREACHABLE;
    }
    void addToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;

    const Mode fMode;
    const ShaderFlags fShaderFlags;
    const SkStrokeRec fStroke;
    const int8_t fMaxParametricSegments_log2;

    constexpr static int kMaxAttribCount = 6;
    SkSTArray<kMaxAttribCount, Attribute> fAttribs;

    // This is a uniform value used when fMode is kFixedCount that tells the shader how many total
    // edges are in the triangle strip.
    float fFixedCountNumTotalEdges = 0;

    class Impl;
    class HardwareImpl;
    class InstancedImpl;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrStrokeTessellationShader::ShaderFlags)

// This common base class emits shader code for our parametric/radial stroke tessellation algorithm
// described above. The subclass emits its own specific setup code before calling into
// emitTessellationCode and emitFragment code.
class GrStrokeTessellationShader::Impl : public ProgramImpl {
protected:
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
    //     float2 tan0;
    //     float2 tan1;
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
    GrGLSLUniformHandler::UniformHandle fEdgeCountUniform;
    GrGLSLUniformHandler::UniformHandle fColorUniform;
    SkString fDynamicColorName;
};

class GrStrokeTessellationShader::InstancedImpl : public GrStrokeTessellationShader::Impl {
    void onEmitCode(EmitArgs&, GrGPArgs*) override;
};

class GrStrokeTessellationShader::HardwareImpl : public GrStrokeTessellationShader::Impl {
    void onEmitCode(EmitArgs&, GrGPArgs*) override;
    SkString getTessControlShaderGLSL(const GrGeometryProcessor&,
                                      const char* versionAndExtensionDecls,
                                      const GrGLSLUniformHandler&,
                                      const GrShaderCaps&) const override;
    SkString getTessEvaluationShaderGLSL(const GrGeometryProcessor&,
                                         const char* versionAndExtensionDecls,
                                         const GrGLSLUniformHandler&,
                                         const GrShaderCaps&) const override;
};

#endif

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
#include "src/gpu/tessellate/Tessellation.h"

// Tessellates a batch of stroke patches directly to the canvas. Tessellated stroking works by
// creating stroke-width, orthogonal edges at set locations along the curve and then connecting them
// with a quad strip. These orthogonal edges come from two different sets: "parametric edges" and
// "radial edges". Parametric edges are spaced evenly in the parametric sense, and radial edges
// divide the curve's _rotation_ into even steps. The tessellation shader evaluates both sets of
// edges and sorts them into a single quad strip. With this combined set of edges we can stroke any
// curve, regardless of curvature.
class GrStrokeTessellationShader : public GrTessellationShader {
    using PatchAttribs = skgpu::PatchAttribs;

public:
    // Are we using hardware tessellation or indirect draws?
    enum class Mode : int8_t {
        kHardwareTessellation,
        kLog2Indirect,
        kFixedCount
    };

    // 'viewMatrix' is applied to the geometry post tessellation. It cannot have perspective.
    GrStrokeTessellationShader(const GrShaderCaps&, Mode, PatchAttribs, const SkMatrix& viewMatrix,
                               const SkStrokeRec&, SkPMColor4f, int8_t maxParametricSegments_log2);

    Mode mode() const { return fMode; }
    PatchAttribs attribs() const { return fPatchAttribs; }
    bool hasDynamicStroke() const { return fPatchAttribs & PatchAttribs::kStrokeParams; }
    bool hasDynamicColor() const { return fPatchAttribs & PatchAttribs::kColor; }
    bool hasExplicitCurveType() const { return fPatchAttribs & PatchAttribs::kExplicitCurveType; }
    const SkStrokeRec& stroke() const { return fStroke;}
    int8_t maxParametricSegments_log2() const { return fMaxParametricSegments_log2; }
    float fixedCountNumTotalEdges() const { return fFixedCountNumTotalEdges;}

    // Used by GrFixedCountTessellator to configure the uniform value that tells the shader how many
    // total edges are in the triangle strip.
    void setFixedCountNumTotalEdges(int value) {
        SkASSERT(fMode == Mode::kFixedCount);
        fFixedCountNumTotalEdges = value;
    }

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
    void addToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;
    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const final;

    const Mode fMode;
    const PatchAttribs fPatchAttribs;
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

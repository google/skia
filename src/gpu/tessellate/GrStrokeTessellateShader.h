/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeTessellateShader_DEFINED
#define GrStrokeTessellateShader_DEFINED

#include "src/gpu/tessellate/GrPathShader.h"

#include "include/core/SkStrokeRec.h"
#include "src/gpu/GrVx.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"
#include <array>

class GrGLSLUniformHandler;

// Tessellates a batch of stroke patches directly to the canvas. Tessellated stroking works by
// creating stroke-width, orthogonal edges at set locations along the curve and then connecting them
// with a quad strip. These orthogonal edges come from two different sets: "parametric edges" and
// "radial edges". Parametric edges are spaced evenly in the parametric sense, and radial edges
// divide the curve's _rotation_ into even steps. The tessellation shader evaluates both sets of
// edges and sorts them into a single quad strip. With this combined set of edges we can stroke any
// curve, regardless of curvature.
class GrStrokeTessellateShader : public GrPathShader {
public:
    // Are we using hardware tessellation or indirect draws?
    enum class Mode : bool {
        kTessellation,
        kIndirect
    };

    enum class ShaderFlags {
        kNone          = 0,
        kHasConics     = 1 << 0,
        kWideColor     = 1 << 1,
        kDynamicStroke = 1 << 2,  // Each patch or instance has its own stroke width and join type.
        kDynamicColor  = 1 << 3,  // Each patch or instance has its own color.
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(ShaderFlags);

    // When using indirect draws, we expect a fixed number of additional edges to be appended onto
    // each instance in order to implement its preceding join. Specifically, each join emits:
    //
    //   * Two colocated edges at the beginning (a double-sided edge to seam with the preceding
    //     stroke and a single-sided edge to seam with the join).
    //
    //   * An extra edge in the middle for miter joins, or else a variable number for round joins
    //     (counted in the resolveLevel).
    //
    //   * A single sided edge at the end of the join that is colocated with the first (double
    //     sided) edge of the stroke
    //
    constexpr static int NumExtraEdgesInIndirectJoin(SkPaint::Join joinType) {
        switch (joinType) {
            case SkPaint::kMiter_Join:
                return 4;
            case SkPaint::kRound_Join:
                // The inner edges for round joins are counted in the stroke's resolveLevel.
                [[fallthrough]];
            case SkPaint::kBevel_Join:
                return 3;
        }
        SkUNREACHABLE;
    }

    // These tolerances decide the number of parametric and radial segments the tessellator will
    // linearize curves into. These decisions are made in (pre-viewMatrix) local path space.
    struct Tolerances {
        // Decides the number of parametric segments the tessellator adds for each curve. (Uniform
        // steps in parametric space.) The tessellator will add enough parametric segments so that,
        // once transformed into device space, they never deviate by more than
        // 1/GrTessellationPathRenderer::kLinearizationIntolerance pixels from the true curve.
        constexpr static float CalcParametricIntolerance(float matrixMaxScale) {
            return matrixMaxScale * GrTessellationPathRenderer::kLinearizationIntolerance;
        }
        // Decides the number of radial segments the tessellator adds for each curve. (Uniform steps
        // in tangent angle.) The tessellator will add this number of radial segments for each
        // radian of rotation in local path space.
        static float CalcNumRadialSegmentsPerRadian(float parametricIntolerance,
                                                    float strokeWidth) {
            return .5f / acosf(std::max(1 - 2 / (parametricIntolerance * strokeWidth), -1.f));
        }
        template<int N> static grvx::vec<N> ApproxNumRadialSegmentsPerRadian(
                float parametricIntolerance, grvx::vec<N> strokeWidths) {
            grvx::vec<N> cosTheta = skvx::max(1 - 2 / (parametricIntolerance * strokeWidths), -1);
            // Subtract GRVX_APPROX_ACOS_MAX_ERROR so we never account for too few segments.
            return .5f / (grvx::approx_acos(cosTheta) - GRVX_APPROX_ACOS_MAX_ERROR);
        }
        // Returns the equivalent stroke width in (pre-viewMatrix) local path space that the
        // tessellator will use when rendering this stroke. This only differs from the actual stroke
        // width for hairlines.
        static float GetLocalStrokeWidth(const float matrixMinMaxScales[2], float strokeWidth) {
            SkASSERT(strokeWidth >= 0);
            float localStrokeWidth = strokeWidth;
            if (localStrokeWidth == 0) {  // Is the stroke a hairline?
                float matrixMinScale = matrixMinMaxScales[0];
                float matrixMaxScale = matrixMinMaxScales[1];
                // If the stroke is hairline then the tessellator will operate in post-transform
                // space instead. But for the sake of CPU methods that need to conservatively
                // approximate the number of segments to emit, we use
                // localStrokeWidth ~= 1/matrixMinScale.
                float approxScale = matrixMinScale;
                // If the matrix has strong skew, don't let the scale shoot off to infinity. (This
                // does not affect the tessellator; only the CPU methods that approximate the number
                // of segments to emit.)
                approxScale = std::max(matrixMinScale, matrixMaxScale * .25f);
                localStrokeWidth = 1/approxScale;
                if (localStrokeWidth == 0) {
                    // We just can't accidentally return zero from this method because zero means
                    // "hairline". Otherwise return whatever we calculated above.
                    localStrokeWidth = SK_ScalarNearlyZero;
                }
            }
            return localStrokeWidth;
        }
        static Tolerances Make(const float matrixMinMaxScales[2], float strokeWidth) {
            return MakeNonHairline(matrixMinMaxScales[1],
                                   GetLocalStrokeWidth(matrixMinMaxScales, strokeWidth));
        }
        static Tolerances MakeNonHairline(float matrixMaxScale, float strokeWidth) {
            SkASSERT(strokeWidth > 0);
            float parametricIntolerance = CalcParametricIntolerance(matrixMaxScale);
            return {parametricIntolerance,
                    CalcNumRadialSegmentsPerRadian(parametricIntolerance, strokeWidth)};
        }
        float fParametricIntolerance;
        float fNumRadialSegmentsPerRadian;
    };

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

    // Size in bytes of a tessellation patch with the given shader flags.
    static size_t PatchStride(ShaderFlags shaderFlags) {
        return sizeof(SkPoint) * 5 + DynamicStateStride(shaderFlags);
    }

    // Size in bytes of an indirect draw instance with the given shader flags.
    static size_t IndirectInstanceStride(ShaderFlags shaderFlags) {
        return sizeof(float) * 11 + DynamicStateStride(shaderFlags);
    }

    // Combined size in bytes of the dynamic state attribs enabled in the given shader flags.
    static size_t DynamicStateStride(ShaderFlags shaderFlags) {
        size_t stride = 0;
        if (shaderFlags & ShaderFlags::kDynamicStroke) {
            stride += sizeof(DynamicStroke);
        }
        if (shaderFlags & ShaderFlags::kDynamicColor) {
            stride += (shaderFlags & ShaderFlags::kWideColor) ? sizeof(float) * 4 : 4;
        }
        return stride;
    }

    // 'viewMatrix' is applied to the geometry post tessellation. It cannot have perspective.
    GrStrokeTessellateShader(Mode mode, ShaderFlags shaderFlags, const SkMatrix& viewMatrix,
                             const SkStrokeRec& stroke, SkPMColor4f color)
            : GrPathShader(kTessellate_GrStrokeTessellateShader_ClassID, viewMatrix,
                           (mode == Mode::kTessellation) ?
                                   GrPrimitiveType::kPatches : GrPrimitiveType::kTriangleStrip,
                           (mode == Mode::kTessellation) ? 1 : 0)
            , fMode(mode)
            , fShaderFlags(shaderFlags)
            , fStroke(stroke)
            , fColor(color) {
        if (fMode == Mode::kTessellation) {
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
            // "lastControlPoint" and "numTotalEdges" are both packed into argsAttr.
            //
            // A join calculates its starting angle using "argsAttr.xy=lastControlPoint".
            //
            // "abs(argsAttr.z=numTotalEdges)" tells the shader the literal number of edges in the
            // triangle strip being rendered (i.e., it should be vertexCount/2). If numTotalEdges is
            // negative and the join type is "kRound", it also instructs the shader to only allocate
            // one segment the preceding round join.
            fAttribs.emplace_back("argsAttr", kFloat3_GrVertexAttribType, kFloat3_GrSLType);
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
        if (fMode == Mode::kTessellation) {
            this->setVertexAttributes(fAttribs.data(), fAttribs.count());
            SkASSERT(this->vertexStride() == PatchStride(fShaderFlags));
        } else {
            this->setInstanceAttributes(fAttribs.data(), fAttribs.count());
            SkASSERT(this->instanceStride() == IndirectInstanceStride(fShaderFlags));
        }
        SkASSERT(fAttribs.count() <= kMaxAttribCount);
    }

    bool hasConics() const { return fShaderFlags & ShaderFlags::kHasConics; }
    bool hasDynamicStroke() const { return fShaderFlags & ShaderFlags::kDynamicStroke; }
    bool hasDynamicColor() const { return fShaderFlags & ShaderFlags::kDynamicColor; }

private:
    const char* name() const override { return "GrStrokeTessellateShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override;
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

    SkString getTessControlShaderGLSL(const GrGLSLPrimitiveProcessor*,
                                      const char* versionAndExtensionDecls,
                                      const GrGLSLUniformHandler&,
                                      const GrShaderCaps&) const override;
    SkString getTessEvaluationShaderGLSL(const GrGLSLPrimitiveProcessor*,
                                         const char* versionAndExtensionDecls,
                                         const GrGLSLUniformHandler&,
                                         const GrShaderCaps&) const override;

    const Mode fMode;
    const ShaderFlags fShaderFlags;
    const SkStrokeRec fStroke;
    const SkPMColor4f fColor;

    constexpr static int kMaxAttribCount = 5;
    SkSTArray<kMaxAttribCount, Attribute> fAttribs;

    class TessellationImpl;
    class IndirectImpl;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrStrokeTessellateShader::ShaderFlags);

#endif

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

    // When using Mode::kTessellation, this is how patches sent to the GPU are structured.
    struct Patch {
        // A join calculates its starting angle using fPrevControlPoint.
        SkPoint fPrevControlPoint;
        // fPts define the cubic stroke as well as the ending angle of the previous join.
        //
        // If fPts[0] == fPrevControlPoint, then no join is emitted.
        //
        // fPts=[p0, p3, p3, p3] is a reserved pattern that means this patch is a join only, whose
        // start and end tangents are (fPts[0] - fPrevControlPoint) and (fPts[3] - fPts[0]).
        //
        // fPts=[p0, p0, p0, p3] is a reserved pattern that means this patch is a cusp point
        // anchored on p0 and rotating from (fPts[0] - fPrevControlPoint) to (fPts[3] - fPts[0]).
        std::array<SkPoint, 4> fPts;
    };

    // When using Mode::kIndirect, these are the instances that get sent to the GPU.
    struct IndirectInstance {
        constexpr static int NumExtraEdgesInJoin(SkPaint::Join joinType) {
            // We expect a fixed number of additional edges to be appended onto each instance in
            // order to implement its preceding join. Specifically, each join emits:
            //
            //   * Two colocated edges at the beginning (a double-sided edge to seam with the
            //     preceding stroke and a single-sided edge to seam with the join).
            //
            //   * An extra edge in the middle for miter joins, or else a variable number for round
            //     joins (counted in the resolveLevel).
            //
            //   * A single sided edge at the end of the join that is colocated with the first
            //     (double sided) edge of the stroke
            //
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
        // Writes the given stroke into this instance. "abs(numTotalEdges)" tells the shader the
        // literal number of edges in the triangle strip being rendered (i.e., it should be
        // vertexCount/2). If numTotalEdges is negative and the join type is "kRound", it also
        // instructs the shader to only allocate one segment the preceding round join.
        void set(const SkPoint pts[4], SkPoint lastControlPoint, int numTotalEdges) {
            memcpy(fPts.data(), pts, sizeof(fPts));
            fLastControlPoint = lastControlPoint;
            fNumTotalEdges = numTotalEdges;
        }
        // A "circle" is a stroke-width circle drawn as a 180-degree point stroke. They should be
        // drawn at cusp points on the curve and for round caps.
        void setCircle(SkPoint pt, int numEdgesForCircles) {
            SkASSERT(numEdgesForCircles >= 0);
            // An empty stroke is a special case that denotes a circle, or 180-degree point stroke.
            fPts.fill(pt);
            fLastControlPoint = pt;
            // Mark fNumTotalEdges negative so the shader assigns the least possible number of edges
            // to its (empty) preceding join.
            fNumTotalEdges = -numEdgesForCircles;
        }
        std::array<SkPoint, 4> fPts;
        SkPoint fLastControlPoint;
        float fNumTotalEdges;
    };

    // These tolerances decide the number of parametric and radial segments the tessellator will
    // linearize curves into. These decisions are made in (pre-viewMatrix) local path space.
    struct Tolerances {
        Tolerances() = default;
        Tolerances(float matrixMaxScale, float strokeWidth) {
            this->set(matrixMaxScale, strokeWidth);
        }
        void set(float matrixMaxScale, float strokeWidth) {
            fParametricIntolerance =
                    matrixMaxScale * GrTessellationPathRenderer::kLinearizationIntolerance;
            fNumRadialSegmentsPerRadian =
                    .5f / acosf(std::max(1 - 2/(fParametricIntolerance * strokeWidth), -1.f));
        }
        // Decides the number of parametric segments the tessellator adds for each curve. (Uniform
        // steps in parametric space.) The tessellator will add enough parametric segments so that,
        // once transformed into device space, they never deviate by more than
        // 1/GrTessellationPathRenderer::kLinearizationIntolerance pixels from the true curve.
        float fParametricIntolerance;
        // Decides the number of radial segments the tessellator adds for each curve. (Uniform steps
        // in tangent angle.) The tessellator will add this number of radial segments for each
        // radian of rotation in local path space.
        float fNumRadialSegmentsPerRadian;
    };

    // 'viewMatrix' is applied to the geometry post tessellation. It cannot have perspective.
    GrStrokeTessellateShader(Mode mode, bool hasConics, const SkStrokeRec& stroke,
                             const SkMatrix& viewMatrix, SkPMColor4f color)
            : GrPathShader(kTessellate_GrStrokeTessellateShader_ClassID, viewMatrix,
                           (mode == Mode::kTessellation) ?
                                   GrPrimitiveType::kPatches : GrPrimitiveType::kTriangleStrip,
                           (mode == Mode::kTessellation) ? 1 : 0)
            , fMode(mode)
            , fHasConics(hasConics)
            , fStroke(stroke)
            , fColor(color) {
        if (fMode == Mode::kTessellation) {
            constexpr static Attribute kTessellationAttribs[] = {
                    {"inputPrevCtrlPt", kFloat2_GrVertexAttribType, kFloat2_GrSLType},
                    {"inputPts01", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
                    {"inputPts23", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};
            this->setVertexAttributes(kTessellationAttribs, SK_ARRAY_COUNT(kTessellationAttribs));
            SkASSERT(this->vertexStride() == sizeof(Patch));
        } else {
            constexpr static Attribute kIndirectAttribs[] = {
                    {"pts01", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
                    {"pts23", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
                    // "fLastControlPoint" and "fNumTotalEdges" are both packed into these args.
                    // See IndirectInstance.
                    {"args", kFloat3_GrVertexAttribType, kFloat3_GrSLType}};
            this->setInstanceAttributes(kIndirectAttribs, SK_ARRAY_COUNT(kIndirectAttribs));
            SkASSERT(this->instanceStride() == sizeof(IndirectInstance));
        }
    }

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
    const bool fHasConics;
    const SkStrokeRec fStroke;
    const SkPMColor4f fColor;

    class TessellationImpl;
    class IndirectImpl;
};

#endif

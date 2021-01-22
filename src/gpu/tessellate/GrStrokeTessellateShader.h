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
    // The vertex array bound for this shader should contain a vector of Patch structs. A Patch is
    // a join followed by a cubic stroke.
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

    // 'parametricIntolerance' controls the number of parametric segments we add for each curve.
    // We add enough parametric segments so that the center of each one falls within
    // 1/parametricIntolerance local path units from the true curve.
    //
    // 'numRadialSegmentsPerRadian' controls the number of radial segments we add for each curve.
    // We add this number of radial segments for each radian of rotation, in order to guarantee
    // smoothness.
    //
    // 'viewMatrix' is applied to the geometry post tessellation. It cannot have perspective.
    GrStrokeTessellateShader(const SkStrokeRec& stroke, float parametricIntolerance,
                             float numRadialSegmentsPerRadian, const SkMatrix& viewMatrix,
                             SkPMColor4f color)
            : GrPathShader(kTessellate_GrStrokeTessellateShader_ClassID, viewMatrix,
                           GrPrimitiveType::kPatches, 1)
            , fStroke(stroke)
            , fParametricIntolerance(parametricIntolerance)
            , fNumRadialSegmentsPerRadian(numRadialSegmentsPerRadian)
            , fColor(color) {
        SkASSERT(!fStroke.isHairlineStyle());  // No hairline support yet.
        constexpr static Attribute kInputPointAttribs[] = {
                {"inputPrevCtrlPt", kFloat2_GrVertexAttribType, kFloat2_GrSLType},
                {"inputPts01", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
                {"inputPts23", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};
        this->setVertexAttributes(kInputPointAttribs, SK_ARRAY_COUNT(kInputPointAttribs));
        SkASSERT(this->vertexStride() == sizeof(Patch));
    }

private:
    const char* name() const override { return "GrStrokeTessellateShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(this->viewMatrix().isIdentity());
    }
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

    SkString getTessControlShaderGLSL(const GrGLSLPrimitiveProcessor*,
                                      const char* versionAndExtensionDecls,
                                      const GrGLSLUniformHandler&,
                                      const GrShaderCaps&) const override;
    SkString getTessEvaluationShaderGLSL(const GrGLSLPrimitiveProcessor*,
                                         const char* versionAndExtensionDecls,
                                         const GrGLSLUniformHandler&,
                                         const GrShaderCaps&) const override;

    const SkStrokeRec fStroke;
    const float fParametricIntolerance;
    const float fNumRadialSegmentsPerRadian;
    const SkPMColor4f fColor;

    class Impl;
};

#endif

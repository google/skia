/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeTessellateShader_DEFINED
#define GrStrokeTessellateShader_DEFINED

#include "src/gpu/tessellate/GrPathShader.h"

#include "src/gpu/tessellate/GrTessellationPathRenderer.h"
#include <array>

class GrGLSLUniformHandler;

// Tessellates a batch of stroke patches directly to the canvas.
//
// Current limitations: (These restrictions will hopefully all be lifted in the future.)
//
//     * A given curve must not have inflection points. Chop curves at inflection points on CPU
//       before sending them down.
//
//     * A given curve must not rotate more than 180 degrees. Chop curves that rotate more than 180
//       degrees at midtangent before sending them down.
//
//     * It is illegal for P1 and P2 to both be coincident with P0 or P3. If this is the case, send
//       the curve [P0, P0, P3, P3] instead.
//
//     * Perspective is not supported.
//
// Tessellated stroking works by creating stroke-width, orthogonal edges at set locations along the
// curve and then connecting them with a quad strip. These orthogonal edges come from two different
// sets: "parametric edges" and "radial edges". Parametric edges are spaced evenly in the parametric
// sense, and radial edges divide the curve's _rotation_ into even steps. The tessellation shader
// evaluates both sets of edges and sorts them into a single quad strip. With this combined set of
// edges we can stroke any curve, regardless of curvature.
class GrStrokeTessellateShader : public GrPathShader {
public:
    // The vertex array bound for this shader should contain a vector of Patch structs. A Patch is
    // either a "cubic" (single stroked bezier curve with butt caps) or a "join". A set of
    // coincident cubic patches with join patches in between will render a complete stroke.
    struct Patch {
        // A value of 0 in fPatchType means this patch is a normal stroked cubic.
        constexpr static float kStandardCubicType = 0;

        // A value of 1 in fPatchType means this patch is a flat line.
        constexpr static float kFlatLineType = 1;

        // An absolute value >=2 in fPatchType means that this patch is a join. A positive value
        // means the join geometry should only go on the outer side of the junction point (spec
        // behavior for standard joins), and a negative value means the join geometry should be
        // double-sided.
        //
        // If a patch is a join, fPts[0] must equal the control point coming into the junction,
        // fPts[1] and fPts[2] must both equal the junction point, and fPts[3] must equal the
        // control point going out. It's imperative for a join's control points match the control
        // points of their adjoining cubics exactly or else the seams might crack.
        constexpr static float kBevelJoinType = 2;
        constexpr static float kMiterJoinType = 3;
        constexpr static float kRoundJoinType = 4;

        std::array<SkPoint, 4> fPts;
        float fPatchType;
        float fStrokeRadius;
    };

    // 'matrixScale' is used to set up an appropriate number of tessellation triangles. It should be
    // equal to viewMatrix.getMaxScale(). (This works because perspective isn't supported.)
    //
    // 'miterLimit' contains the stroke's miter limit, or may be zero if no patches being drawn will
    // be miter joins.
    //
    // 'viewMatrix' is applied to the geometry post tessellation. It cannot have perspective.
    GrStrokeTessellateShader(float matrixScale, float miterLimit, const SkMatrix& viewMatrix,
                             SkPMColor4f color)
            : GrPathShader(kTessellate_GrStrokeTessellateShader_ClassID, viewMatrix,
                           GrPrimitiveType::kPatches, 1)
            , fMatrixScale(matrixScale)
            , fMiterLimit(miterLimit)
            , fColor(color) {
        constexpr static Attribute kInputPointAttribs[] = {
                {"inputPts01", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
                {"inputPts23", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
                {"inputArgs", kFloat2_GrVertexAttribType, kFloat2_GrSLType}};
        this->setVertexAttributes(kInputPointAttribs, SK_ARRAY_COUNT(kInputPointAttribs));
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

    const float fMatrixScale;
    const float fMiterLimit;
    const SkPMColor4f fColor;

    class Impl;
};

#endif

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
// curve and then connecting them with a triangle strip. These orthogonal edges come from two
// different sets: "parametric edges" and "radial edges". Parametric edges are spaced evenly in the
// parametric sense, and radial edges divide the curve's _rotation_ into even steps. The
// tessellation shader evaluates both sets of edges and sorts them into a single triangle strip.
// With this combined set of edges we can stroke any curve, regardless of curvature.
//
// A patch is either a "cubic" (single stroked bezier curve with butt caps) or a "join". A patch is
// defined by 5 points as follows:
//
//   P0..P3           : Represent the cubic control points.
//   (P4.x == 0)      : The patch is a normal cubic.
//   (abs(P4.x) == 1) : The patch is a bevel join.
//   (abs(P4.x) == 2) : The patch is a miter join.
//                      (NOTE: If miterLimitOrZero == 0, then miter join patches are illegal.)
//   (abs(P4.x) >= 3) : The patch is a round join.
//   (P4.x < 0)       : The patch join is double sided. (Positive value joins only draw on the
//                      outer side of their junction.)
//   P4.y             : Represents the stroke radius.
//
// If a patch is a join, P0 must equal the control point coming into the junction, P1 and P2 must
// equal the junction point, and P3 must equal the control point going out. It's imperative that a
// junction's control points match the control points of their neighbor cubics exactly, or the
// seaming might not be water tight. (Also note that if P1==P0 or P2==P3, the junction needs to be
// given its neighbor's opposite cubic control point.)
//
// To use this shader, construct a GrProgramInfo with a primitiveType of "kPatches" and a
// tessellationPatchVertexCount of 5.
class GrStrokeTessellateShader : public GrPathShader {
public:
    constexpr static float kStandardCubicType = 0;
    constexpr static float kBevelJoinType = 1;
    constexpr static float kMiterJoinType = 2;
    constexpr static float kRoundJoinType = 3;

    constexpr static int kNumVerticesPerPatch = 5;

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
                           GrPrimitiveType::kPatches, kNumVerticesPerPatch)
            , fMatrixScale(matrixScale)
            , fMiterLimit(miterLimit)
            , fColor(color) {
        constexpr static Attribute kInputPointAttrib{"inputPoint", kFloat2_GrVertexAttribType,
                                                     kFloat2_GrSLType};
        this->setVertexAttributes(&kInputPointAttrib, 1);
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

/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTessellateStrokeShader_DEFINED
#define GrTessellateStrokeShader_DEFINED

#include "src/gpu/tessellate/GrPathShader.h"

#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

class GrGLSLUniformHandler;

// Tessellates a batch of stroke patches directly to the canvas. A patch is either a "cubic"
// (single stroked bezier curve with butt caps) or a "join". A patch is defined by 5 points as
// follows:
//
//   P0..P3      : Represent the cubic control points.
//   (P4.x == 0) : The patch is a cubic and the shader decides how many linear segments to produce.
//   (P4.x < 0)  : The patch is still a cubic, but will be linearized into exactly |P4.x| segments.
//   (P4.x == 1) : The patch is an outer bevel join.
//   (P4.x == 2) : The patch is an outer miter join.
//                 (NOTE: If miterLimitOrZero == 0, then miter join patches are illegal.)
//   (P4.x == 3) : The patch is an outer round join.
//   (P4.x == 4) : The patch is an inner and outer round join.
//   P4.y        : Represents the stroke radius.
//
// If a patch is a join, P0 must equal P3, P1 must equal the control point coming into the junction,
// and P2 must equal the control point going out. It's imperative that a junction's control points
// match the control points of their neighbor cubics exactly, or the rasterization might not be
// water tight. (Also note that if P1==P0 or P2==P3, the junction needs to be given its neighbor's
// opposite cubic control point.)
//
// To use this shader, construct a GrProgramInfo with a primitiveType of "kPatches" and a
// tessellationPatchVertexCount of 5.
class GrTessellateStrokeShader : public GrPathShader {
public:
    GrTessellateStrokeShader(const SkMatrix& viewMatrix, SkPMColor4f color, float miterLimitOrZero)
            : GrPathShader(kTessellate_GrTessellateStrokeShader_ClassID, viewMatrix,
                           GrPrimitiveType::kPatches, 5)
            , fColor(color)
            , fMiterLimitOrZero(miterLimitOrZero) {
        constexpr static Attribute kInputPointAttrib{"inputPoint", kFloat2_GrVertexAttribType,
                                                     kFloat2_GrSLType};
        this->setVertexAttributes(&kInputPointAttrib, 1);
    }

private:
    const char* name() const override { return "GrTessellateStrokeShader"; }
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

    const SkPMColor4f fColor;
    const float fMiterLimitOrZero;  // Zero if there will not be any miter join patches.

    class Impl;
};

#endif

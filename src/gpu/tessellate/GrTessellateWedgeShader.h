/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTessellateWedgeShader_DEFINED
#define GrTessellateWedgeShader_DEFINED

#include "src/gpu/GrGeometryProcessor.h"

// Uses GPU tessellation shaders to linearize, triangulate, and render cubic "wedge" patches. A
// wedge is a 5-point patch consisting of 4 cubic control points, plus an anchor point fanning from
// the center of the curve's resident contour. We stencil paths by converting lines and quadratics
// to cubics, then rendering a cubic wedge for each verb.
// TODO: Eventually we want to use rational cubic wedges in order to support perspective and conics.
class GrTessellateWedgeShader : public GrGeometryProcessor {
public:
    GrTessellateWedgeShader(const SkMatrix& viewMatrix)
            : GrGeometryProcessor(kGrTessellateWedgeShader_ClassID)
            , fViewMatrix(viewMatrix) {
        static constexpr Attribute kPtAttrib = {"P", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        this->setVertexAttributes(&kPtAttrib, 1);
        this->setWillUseTessellationShaders();
    }

    const char* name() const override { return "GrTessellateWedgeShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {}
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

    SkString getTessControlShaderGLSL(const char* versionAndExtensionDecls,
                                      const GrShaderCaps&) const override;
    SkString getTessEvaluationShaderGLSL(const char* versionAndExtensionDecls,
                                         const GrShaderCaps&) const override;

private:
    const SkMatrix fViewMatrix;

    class Impl;
};

#endif

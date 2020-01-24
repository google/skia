/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStencilPathShader_DEFINED
#define GrStencilPathShader_DEFINED

#include "src/gpu/tessellate/GrPathShader.h"

// This is the base class for shaders that stencil path elements, namely, triangles, standalone
// cubics, and wedges.
class GrStencilPathShader : public GrPathShader {
public:
    GrStencilPathShader(ClassID classID, const SkMatrix& viewMatrix, GrPrimitiveType primitiveType,
                        int tessellationPatchVertexCount = 0)
            : GrPathShader(classID, viewMatrix, primitiveType, tessellationPatchVertexCount) {
        constexpr static Attribute kPointAttrib = {
                "point", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        this->setVertexAttributes(&kPointAttrib, 1);
    }

private:
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {
        b->add32(this->viewMatrix().isIdentity());
    }
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

    class Impl;
};

// Draws simple triangles to the stencil buffer.
class GrStencilTriangleShader : public GrStencilPathShader {
public:
    GrStencilTriangleShader(const SkMatrix& viewMatrix) : GrStencilPathShader(
            kTessellate_GrStencilTriangleShader_ClassID, viewMatrix, GrPrimitiveType::kTriangles) {}
    const char* name() const override { return "tessellate_GrStencilTriangleShader"; }
};

// Uses GPU tessellation shaders to linearize, triangulate, and render standalone cubics. Here, a
// "cubic" is a standalone closed contour consisting of a single cubic bezier.
// TODO: Eventually we want to use rational cubic wedges in order to support perspective and conics.
class GrStencilCubicShader : public GrStencilPathShader {
public:
    GrStencilCubicShader(const SkMatrix& viewMatrix) : GrStencilPathShader(
            kTessellate_GrStencilCubicShader_ClassID, viewMatrix, GrPrimitiveType::kPatches, 4) {}
    const char* name() const override { return "tessellate_GrStencilCubicShader"; }

private:
    SkString getTessControlShaderGLSL(const char* versionAndExtensionDecls,
                                      const GrShaderCaps&) const override;
    SkString getTessEvaluationShaderGLSL(const char* versionAndExtensionDecls,
                                         const GrShaderCaps&) const override;
};

// Uses GPU tessellation shaders to linearize, triangulate, and render cubic "wedge" patches. A
// wedge is a 5-point patch consisting of 4 cubic control points, plus an anchor point fanning from
// the center of the curve's resident contour.
// TODO: Eventually we want to use rational cubic wedges in order to support perspective and conics.
class GrStencilWedgeShader : public GrStencilPathShader {
public:
    GrStencilWedgeShader(const SkMatrix& viewMatrix) : GrStencilPathShader(
            kTessellate_GrStencilWedgeShader_ClassID, viewMatrix, GrPrimitiveType::kPatches, 5) {}
    const char* name() const override { return "tessellate_GrStencilWedgeShader"; }

private:
    SkString getTessControlShaderGLSL(const char* versionAndExtensionDecls,
                                      const GrShaderCaps&) const override;
    SkString getTessEvaluationShaderGLSL(const char* versionAndExtensionDecls,
                                         const GrShaderCaps&) const override;
};

#endif

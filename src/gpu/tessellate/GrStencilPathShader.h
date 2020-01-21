/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStencilPathShader_DEFINED
#define GrStencilPathShader_DEFINED

#include "src/gpu/GrGeometryProcessor.h"

class GrStencilPathShader : public GrGeometryProcessor {
public:
    GrStencilPathShader(ClassID classID, const SkMatrix& viewMatrix, GrPrimitiveType primitiveType,
                        int tessellationPatchVertexCount = 0)
            : GrGeometryProcessor(classID)
            , fViewMatrix(viewMatrix)
            , fPrimitiveType(primitiveType)
            , fTessellationPatchVertexCount(tessellationPatchVertexCount) {
        constexpr static Attribute kPointAttrib = {
                "point", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        this->setVertexAttributes(&kPointAttrib, 1);
        if (fTessellationPatchVertexCount) {
            this->setWillUseTessellationShaders();
        }
    }
    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    int tessellationPatchVertexCount() const { return fTessellationPatchVertexCount; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {
        b->add32(fViewMatrix.isIdentity());
    }
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

private:
    const SkMatrix fViewMatrix;
    const GrPrimitiveType fPrimitiveType;
    const int fTessellationPatchVertexCount;

    class Impl;
};

class GrStencilTriangleShader : public GrStencilPathShader {
public:
    GrStencilTriangleShader(const SkMatrix& viewMatrix) : GrStencilPathShader(
            kTessellate_GrStencilTriangleShader_ClassID, viewMatrix, GrPrimitiveType::kTriangles) {}
    const char* name() const override { return "tessellate_GrStencilTriangleShader"; }
};

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

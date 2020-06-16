/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokePathShader_DEFINED
#define GrStrokePathShader_DEFINED

#include "src/gpu/tessellate/GrPathShader.h"

#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

class GrGLSLUniformHandler;
class GrGLSLVertexBuilder;

// Blah.
class GrTessellateCubicStrokeShader : public GrPathShader {
public:
    GrTessellateCubicStrokeShader(const SkMatrix& viewMatrix, float miterLimit, SkPMColor4f color)
            : GrPathShader(kTessellate_GrTessellateCubicStrokeShader_ClassID, viewMatrix,
                           GrPrimitiveType::kPatches, 5)
            , fMiterLimit(miterLimit)
            , fColor(color) {
        constexpr static Attribute kInputPointAttrib{"inputPoint", kFloat2_GrVertexAttribType,
                                                     kFloat2_GrSLType};
        this->setVertexAttributes(&kInputPointAttrib, 1);
    }

private:
    const char* name() const override { return "GrTessellateCubicStrokeShader"; }
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

    const float fMiterLimit;
    const SkPMColor4f fColor;

    class Impl;
};

// Blah.
class GrStrokePathShader : public GrPathShader {
public:
    // Returns the vertex buffer that should be bound when drawing with this shader.
    static sk_sp<const GrGpuBuffer> FindOrMakeVertexBuffer(GrResourceProvider*);

    // Blah.
    static GrDrawIndirectCommand MakeDrawStrokesIndirectCmd(int resolveLevel,
                                                            uint32_t instanceCount,
                                                            uint32_t baseInstance) {
        SkASSERT(resolveLevel >= 0 && resolveLevel <= GrTessellationPathRenderer::kMaxResolveLevel);
        // Each stroke has 2^resolveLevel segments. Each segment has 6 vertices (two triangles).
        uint32_t vertexCount = (1 << resolveLevel) * 6;
        // Silly math.
        uint32_t baseVertex = vertexCount - 6;
        return {vertexCount, instanceCount, baseVertex, baseInstance};
    }

    GrStrokePathShader(const SkMatrix& viewMatrix, SkPMColor4f color)
            : GrPathShader(kTessellate_GrStrokePathShader_ClassID, viewMatrix,
                           GrPrimitiveType::kTriangles, 0)
            , fColor(color) {
        static constexpr Attribute kVertexAttribs[] = {
                {"vertexInfo", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
                {"vertexInfo2", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};
        static constexpr Attribute kInstanceAttribs[] = {
                {"inputPoints_0_1", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
                {"inputPoints_2_3", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};
        this->setVertexAttributes(kVertexAttribs, SK_ARRAY_COUNT(kVertexAttribs));
        this->setInstanceAttributes(kInstanceAttribs, SK_ARRAY_COUNT(kInstanceAttribs));
    }

private:
    const char* name() const override { return "GrStrokePathShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(this->viewMatrix().isIdentity());
    }
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

    const SkPMColor4f fColor;

    class Impl;
};

#endif

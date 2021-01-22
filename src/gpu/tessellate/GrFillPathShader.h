/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFillPathShader_DEFINED
#define GrFillPathShader_DEFINED

#include "src/gpu/tessellate/GrPathShader.h"

class GrGLSLUniformHandler;
class GrGLSLVertexBuilder;

// This is the base class for shaders that fill a path's pixels in the final render target.
class GrFillPathShader : public GrPathShader {
public:
    GrFillPathShader(ClassID classID, const SkMatrix& viewMatrix, SkPMColor4f color,
                     GrPrimitiveType primitiveType)
            : GrPathShader(classID, viewMatrix, primitiveType, 0)
            , fColor(color) {
    }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

protected:
    class Impl;

    virtual void emitVertexCode(Impl*, GrGLSLVertexBuilder*, const char* viewMatrix,
                                GrGLSLUniformHandler*) const = 0;

private:
    const SkPMColor4f fColor;
};

// Fills a simple array of triangles.
class GrFillTriangleShader : public GrFillPathShader {
public:
    GrFillTriangleShader(const SkMatrix& viewMatrix, SkPMColor4f color)
            : GrFillPathShader(kTessellate_GrFillTriangleShader_ClassID, viewMatrix, color,
                               GrPrimitiveType::kTriangles) {
        static constexpr Attribute kPtAttrib = {
                "input_point", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        this->setVertexAttributes(&kPtAttrib, 1);
    }

private:
    const char* name() const override { return "GrFillTriangleShader"; }
    void emitVertexCode(Impl*, GrGLSLVertexBuilder*, const char* viewMatrix,
                        GrGLSLUniformHandler*) const override;
};

// Fills an array of convex hulls surrounding 4-point cubic instances.
class GrFillCubicHullShader : public GrFillPathShader {
public:
    GrFillCubicHullShader(const SkMatrix& viewMatrix, SkPMColor4f color)
            : GrFillPathShader(kTessellate_GrFillCubicHullShader_ClassID, viewMatrix, color,
                               GrPrimitiveType::kTriangleStrip) {
        static constexpr Attribute kPtsAttribs[] = {
                {"input_points_0_1", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
                {"input_points_2_3", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};
        this->setInstanceAttributes(kPtsAttribs, SK_ARRAY_COUNT(kPtsAttribs));
    }

private:
    const char* name() const override { return "GrFillCubicHullShader"; }
    void emitVertexCode(Impl*, GrGLSLVertexBuilder*, const char* viewMatrix,
                        GrGLSLUniformHandler*) const override;
};

// Fills a path's bounding box, with subpixel outset to avoid possible T-junctions with extreme
// edges of the path.
// NOTE: The emitted geometry may not be axis-aligned, depending on the view matrix.
class GrFillBoundingBoxShader : public GrFillPathShader {
public:
    GrFillBoundingBoxShader(const SkMatrix& viewMatrix, SkPMColor4f color, const SkRect& pathBounds)
            : GrFillPathShader(kTessellate_GrFillBoundingBoxShader_ClassID, viewMatrix, color,
                               GrPrimitiveType::kTriangleStrip)
            , fPathBounds(pathBounds) {
    }

    const SkRect& pathBounds() const { return fPathBounds; }

private:
    const char* name() const override { return "GrFillBoundingBoxShader"; }
    void emitVertexCode(Impl*, GrGLSLVertexBuilder*, const char* viewMatrix,
                        GrGLSLUniformHandler*) const override;

    const SkRect fPathBounds;
};

#endif

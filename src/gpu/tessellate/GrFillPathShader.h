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

    static const GrPipeline* MakeFillPassPipeline(const GrPathShader::ProgramArgs& args,
                                                  GrAAType aaType, GrAppliedClip&& appliedClip,
                                                  GrProcessorSet&& processors) {
        auto pipelineFlags = GrPipeline::InputFlags::kNone;
        if (aaType != GrAAType::kNone) {
            if (args.fWriteView.asRenderTargetProxy()->numSamples() == 1) {
                // We are mixed sampled. We need to either enable conservative raster (preferred) or
                // disable MSAA in order to avoid double blend artifacts. (Even if we disable MSAA for
                // the cover geometry, the stencil test is still multisampled and will still produce
                // smooth results.)
                SkASSERT(aaType == GrAAType::kCoverage);
                if (args.fCaps->conservativeRasterSupport()) {
                    pipelineFlags |= GrPipeline::InputFlags::kHWAntialias;
                    pipelineFlags |= GrPipeline::InputFlags::kConservativeRaster;
                }
            } else {
                // We are standard MSAA. Leave MSAA enabled for the cover geometry.
                pipelineFlags |= GrPipeline::InputFlags::kHWAntialias;
            }
        }
        return GrSimpleMeshDrawOpHelper::CreatePipeline(
                args.fCaps, args.fArena, args.fWriteView.swizzle(), std::move(appliedClip),
                *args.fDstProxyView, std::move(processors), pipelineFlags);
    }

    // Allows non-zero stencil values to pass and write a color, and resets the stencil value back
    // to zero; discards immediately on stencil values of zero.
    static const GrUserStencilSettings* TestAndResetStencilSettings() {
        constexpr static GrUserStencilSettings kTestAndResetStencil(
            GrUserStencilSettings::StaticInit<
                0x0000,
                // No need to check the clip because the previous stencil pass will have only
                // written to samples already inside the clip.
                GrUserStencilTest::kNotEqual,
                0xffff,
                GrUserStencilOp::kZero,
                GrUserStencilOp::kKeep,
                0xffff>());
        return &kTestAndResetStencil;
    }

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

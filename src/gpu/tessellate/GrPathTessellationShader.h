/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathTessellationShader_DEFINED
#define GrPathTessellationShader_DEFINED

#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"
#include "src/gpu/tessellate/GrTessellationShader.h"

// This is the base class for shaders in the GPU tessellator that fill paths.
class GrPathTessellationShader : public GrTessellationShader {
public:
    GrPathTessellationShader(ClassID classID, GrPrimitiveType primitiveType,
                             int tessellationPatchVertexCount, const SkMatrix& viewMatrix,
                             const SkPMColor4f& color)
            : GrTessellationShader(classID, primitiveType, tessellationPatchVertexCount, viewMatrix,
                                   color) {
    }

    // Returns the stencil settings to use for a standard Redbook "stencil" pass.
    static const GrUserStencilSettings* StencilPathSettings(SkPathFillType fillType) {
        // Increments clockwise triangles and decrements counterclockwise. Used for "winding" fill.
        constexpr static GrUserStencilSettings kIncrDecrStencil(
            GrUserStencilSettings::StaticInitSeparate<
                0x0000,                                0x0000,
                GrUserStencilTest::kAlwaysIfInClip,    GrUserStencilTest::kAlwaysIfInClip,
                0xffff,                                0xffff,
                GrUserStencilOp::kIncWrap,             GrUserStencilOp::kDecWrap,
                GrUserStencilOp::kKeep,                GrUserStencilOp::kKeep,
                0xffff,                                0xffff>());

        // Inverts the bottom stencil bit. Used for "even/odd" fill.
        constexpr static GrUserStencilSettings kInvertStencil(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kAlwaysIfInClip,
                0xffff,
                GrUserStencilOp::kInvert,
                GrUserStencilOp::kKeep,
                0x0001>());

        SkASSERT(fillType == SkPathFillType::kWinding || fillType == SkPathFillType::kEvenOdd);
        return (fillType == SkPathFillType::kWinding) ? &kIncrDecrStencil : &kInvertStencil;
    }

    // Returns the stencil settings to use for a standard Redbook "fill" pass. Allows non-zero
    // stencil values to pass and write a color, and resets the stencil value back to zero; discards
    // immediately on stencil values of zero.
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

    // Creates a pipeline that does not write to the color buffer.
    static const GrPipeline* MakeStencilOnlyPipeline(const ProgramArgs& args, GrAAType aaType,
                                                     GrTessellationPathRenderer::OpFlags opFlags,
                                                     const GrAppliedHardClip& hardClip) {
        using OpFlags = GrTessellationPathRenderer::OpFlags;
        GrPipeline::InitArgs pipelineArgs;
        if (aaType == GrAAType::kMSAA) {
            pipelineArgs.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
        }
        if (args.fCaps->wireframeSupport() && (opFlags & OpFlags::kWireframe)) {
            pipelineArgs.fInputFlags |= GrPipeline::InputFlags::kWireframe;
        }
        pipelineArgs.fCaps = args.fCaps;
        return args.fArena->make<GrPipeline>(pipelineArgs,
                                             GrDisableColorXPFactory::MakeXferProcessor(),
                                             hardClip);
    }

protected:
    // Default path tessellation shader implementation that manages a uniform matrix and color.
    class Impl : public GrGLSLGeometryProcessor {
    public:
        void onEmitCode(EmitArgs&, GrGPArgs*) final;
        void setData(const GrGLSLProgramDataManager&, const GrShaderCaps&,
                     const GrGeometryProcessor&) override;

    protected:
        virtual void emitVertexCode(GrGLSLVertexBuilder*, GrGPArgs*) = 0;

        GrGLSLUniformHandler::UniformHandle fAffineMatrixUniform;
        GrGLSLUniformHandler::UniformHandle fTranslateUniform;
        GrGLSLUniformHandler::UniformHandle fColorUniform;
    };
};

// Draws a simple array of triangles.
class GrTriangleShader : public GrPathTessellationShader {
public:
    GrTriangleShader(const SkMatrix& viewMatrix, SkPMColor4f color)
            : GrPathTessellationShader(kTessellate_GrTriangleShader_ClassID,
                                       GrPrimitiveType::kTriangles, 0, viewMatrix, color) {
        constexpr static Attribute kInputPointAttrib{"inputPoint", kFloat2_GrVertexAttribType,
                                                     kFloat2_GrSLType};
        this->setVertexAttributes(&kInputPointAttrib, 1);
    }

private:
    const char* name() const final { return "tessellate_GrTriangleShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const final {}
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const final;
};

// Uses GPU tessellation shaders to linearize, triangulate, and render standalone closed cubics.
// TODO: Eventually we want to use rational cubic wedges in order to support perspective and conics.
class GrCurveTessellateShader : public GrPathTessellationShader {
public:
    GrCurveTessellateShader(const SkMatrix& viewMatrix, const SkPMColor4f& color)
            : GrPathTessellationShader(kTessellate_GrCurveTessellateShader_ClassID,
                                       GrPrimitiveType::kPatches, 4, viewMatrix, color) {
        constexpr static Attribute kInputPointAttrib{"inputPoint", kFloat2_GrVertexAttribType,
                                                     kFloat2_GrSLType};
        this->setVertexAttributes(&kInputPointAttrib, 1);
    }

private:
    const char* name() const final { return "tessellate_GrCurveTessellateShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {}
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const final;
};

// Uses GPU tessellation shaders to linearize, triangulate, and render cubic "wedge" patches. A
// wedge is a 5-point patch consisting of 4 cubic control points, plus an anchor point fanning from
// the center of the curve's resident contour.
// TODO: Eventually we want to use rational cubic wedges in order to support perspective and conics.
class GrWedgeTessellateShader : public GrPathTessellationShader {
public:
    GrWedgeTessellateShader(const SkMatrix& viewMatrix, const SkPMColor4f& color)
            : GrPathTessellationShader(kTessellate_GrWedgeTessellateShader_ClassID,
                                       GrPrimitiveType::kPatches, 5, viewMatrix, color) {
        constexpr static Attribute kInputPointAttrib{"inputPoint", kFloat2_GrVertexAttribType,
                                                     kFloat2_GrSLType};
        this->setVertexAttributes(&kInputPointAttrib, 1);
    }

private:
    const char* name() const final { return "tessellate_GrWedgeTessellateShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {}
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const final;
};

// Uses instanced draws to triangulate standalone closed curves with a "middle-out" topology.
// Middle-out draws a triangle with vertices at T=[0, 1/2, 1] and then recurses breadth first:
//
//   depth=0: T=[0, 1/2, 1]
//   depth=1: T=[0, 1/4, 2/4], T=[2/4, 3/4, 1]
//   depth=2: T=[0, 1/8, 2/8], T=[2/8, 3/8, 4/8], T=[4/8, 5/8, 6/8], T=[6/8, 7/8, 1]
//   ...
//
// The caller may compute each cubic's resolveLevel on the CPU (i.e., the log2 number of line
// segments it will be divided into; see GrWangsFormula::cubic_log2/quadratic_log2/conic_log2), and
// then sort the instance buffer by resolveLevel for efficient batching of indirect draws.
class GrCurveMiddleOutShader : public GrPathTessellationShader {
public:
    // How many vertices do we need to draw in order to triangulate a cubic with 2^resolveLevel
    // line segments?
    constexpr static int NumVerticesAtResolveLevel(int resolveLevel) {
        // resolveLevel=0 -> 0 line segments -> 0 triangles -> 0 vertices
        // resolveLevel=1 -> 2 line segments -> 1 triangle -> 3 vertices
        // resolveLevel=2 -> 4 line segments -> 3 triangles -> 9 vertices
        // resolveLevel=3 -> 8 line segments -> 7 triangles -> 21 vertices
        // ...
        return ((1 << resolveLevel) - 1) * 3;
    }

    // Configures an indirect draw to render cubic instances with 2^resolveLevel evenly-spaced (in
    // the parametric sense) line segments.
    static void WriteDrawIndirectCmd(GrDrawIndirectWriter* indirectWriter, int resolveLevel,
                                     uint32_t instanceCount, uint32_t baseInstance) {
        SkASSERT(resolveLevel > 0 && resolveLevel <= GrTessellationPathRenderer::kMaxResolveLevel);
        // The vertex shader determines the T value at which to draw each vertex. Since the
        // triangles are arranged in "middle-out" order, we can conveniently control the
        // resolveLevel by changing only the vertexCount.
        uint32_t vertexCount = NumVerticesAtResolveLevel(resolveLevel);
        indirectWriter->write(instanceCount, baseInstance, vertexCount, 0);
    }

    GrCurveMiddleOutShader(const SkMatrix& viewMatrix, const SkPMColor4f& color)
            : GrPathTessellationShader(kTessellate_GrCurveMiddleOutShader_ClassID,
                                       GrPrimitiveType::kTriangles, 0, viewMatrix, color) {
        constexpr static Attribute kInputPtsAttribs[] = {
                {"inputPoints_0_1", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
                {"inputPoints_2_3", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};
        this->setInstanceAttributes(kInputPtsAttribs, 2);
    }

private:
    const char* name() const final { return "tessellate_GrCurveMiddleOutShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {}
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const final;

    class Impl;
};

#endif

/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStencilPathShader_DEFINED
#define GrStencilPathShader_DEFINED

#include "src/gpu/GrDrawIndirectCommand.h"
#include "src/gpu/tessellate/GrPathShader.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

// This is the base class for shaders that stencil path elements, namely, triangles, standalone
// cubics, and wedges.
class GrStencilPathShader : public GrPathShader {
public:
    GrStencilPathShader(ClassID classID, const SkMatrix& viewMatrix, GrPrimitiveType primitiveType,
                        int tessellationPatchVertexCount = 0)
            : GrPathShader(classID, viewMatrix, primitiveType, tessellationPatchVertexCount) {
    }

    // Creates a pipeline that can be used for normal Redbook stencil draws.
    static const GrPipeline* MakeStencilPassPipeline(const GrPathShader::ProgramArgs& args,
                                                     GrAAType aaType,
                                                     GrTessellationPathRenderer::OpFlags opFlags,
                                                     const GrAppliedHardClip& hardClip) {
        using OpFlags = GrTessellationPathRenderer::OpFlags;
        GrPipeline::InitArgs pipelineArgs;
        if (aaType != GrAAType::kNone) {
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

    // Returns the stencil settings to use for a standard Redbook stencil draw.
    static const GrUserStencilSettings* StencilPassSettings(SkPathFillType fillType) {
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

    template<typename ShaderType>
    static GrProgramInfo* MakeStencilProgram(const ProgramArgs& args, const SkMatrix& viewMatrix,
                                             const GrPipeline* pipeline,
                                             const GrUserStencilSettings* stencil) {
        const auto* shader = args.fArena->make<ShaderType>(viewMatrix);
        return GrPathShader::MakeProgram(args, shader, pipeline, stencil);
    }

    template<typename ShaderType>
    static GrProgramInfo* MakeStencilProgram(const ProgramArgs& args, const SkMatrix& viewMatrix,
                                             const GrPipeline* pipeline,
                                             const SkPathFillType fillType) {
        return MakeStencilProgram<ShaderType>(args, viewMatrix, pipeline,
                                              StencilPassSettings(fillType));
    }

protected:
    constexpr static Attribute kSinglePointAttrib{"inputPoint", kFloat2_GrVertexAttribType,
                                                  kFloat2_GrSLType};
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(this->viewMatrix().isIdentity());
    }
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const override;

    class Impl;
};

// Draws simple triangles to the stencil buffer.
class GrStencilTriangleShader : public GrStencilPathShader {
public:
    GrStencilTriangleShader(const SkMatrix& viewMatrix) : GrStencilPathShader(
            kTessellate_GrStencilTriangleShader_ClassID, viewMatrix, GrPrimitiveType::kTriangles) {
        this->setVertexAttributes(&kSinglePointAttrib, 1);
    }
    const char* name() const override { return "tessellate_GrStencilTriangleShader"; }
};

// Uses GPU tessellation shaders to linearize, triangulate, and render standalone closed cubics.
// TODO: Eventually we want to use rational cubic wedges in order to support perspective and conics.
class GrCurveTessellateShader : public GrStencilPathShader {
public:
    GrCurveTessellateShader(const SkMatrix& viewMatrix) : GrStencilPathShader(
            kTessellate_GrCurveTessellateShader_ClassID, viewMatrix, GrPrimitiveType::kPatches, 4) {
        this->setVertexAttributes(&kSinglePointAttrib, 1);
    }
    const char* name() const override { return "tessellate_GrCurveTessellateShader"; }

private:
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const override;
};

// Uses GPU tessellation shaders to linearize, triangulate, and render cubic "wedge" patches. A
// wedge is a 5-point patch consisting of 4 cubic control points, plus an anchor point fanning from
// the center of the curve's resident contour.
// TODO: Eventually we want to use rational cubic wedges in order to support perspective and conics.
class GrWedgeTessellateShader : public GrStencilPathShader {
public:
    GrWedgeTessellateShader(const SkMatrix& viewMatrix) : GrStencilPathShader(
            kTessellate_GrWedgeTessellateShader_ClassID, viewMatrix, GrPrimitiveType::kPatches, 5) {
        this->setVertexAttributes(&kSinglePointAttrib, 1);
    }
    const char* name() const override { return "tessellate_GrWedgeTessellateShader"; }

private:
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const override;
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
class GrCurveMiddleOutShader : public GrStencilPathShader {
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

    GrCurveMiddleOutShader(const SkMatrix& viewMatrix)
            : GrStencilPathShader(kTessellate_GrCurveMiddleOutShader_ClassID, viewMatrix,
                                  GrPrimitiveType::kTriangles) {
        constexpr static Attribute kInputPtsAttribs[] = {
                {"inputPoints_0_1", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
                {"inputPoints_2_3", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};
        this->setInstanceAttributes(kInputPtsAttribs, 2);
    }

    const char* name() const override { return "tessellate_GrCurveMiddleOutShader"; }

private:
    GrGLSLGeometryProcessor* createGLSLInstance(const GrShaderCaps&) const override;

    class Impl;
};

#endif

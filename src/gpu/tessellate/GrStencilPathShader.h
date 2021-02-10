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
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

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
class GrCubicTessellateShader : public GrStencilPathShader {
public:
    GrCubicTessellateShader(const SkMatrix& viewMatrix) : GrStencilPathShader(
            kTessellate_GrCubicTessellateShader_ClassID, viewMatrix, GrPrimitiveType::kPatches, 4) {
        this->setVertexAttributes(&kSinglePointAttrib, 1);
    }
    const char* name() const override { return "tessellate_GrCubicTessellateShader"; }

private:
    SkString getTessControlShaderGLSL(const GrGLSLPrimitiveProcessor*,
                                      const char* versionAndExtensionDecls,
                                      const GrGLSLUniformHandler&,
                                      const GrShaderCaps&) const override;
    SkString getTessEvaluationShaderGLSL(const GrGLSLPrimitiveProcessor*,
                                         const char* versionAndExtensionDecls,
                                         const GrGLSLUniformHandler&,
                                         const GrShaderCaps&) const override;
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
    SkString getTessControlShaderGLSL(const GrGLSLPrimitiveProcessor*,
                                      const char* versionAndExtensionDecls,
                                      const GrGLSLUniformHandler&,
                                      const GrShaderCaps&) const override;
    SkString getTessEvaluationShaderGLSL(const GrGLSLPrimitiveProcessor*,
                                         const char* versionAndExtensionDecls,
                                         const GrGLSLUniformHandler&,
                                         const GrShaderCaps&) const override;
};

// Uses indirect (instanced) draws to triangulate standalone closed cubics with a "middle-out"
// topology. The caller must compute each cubic's resolveLevel on the CPU (i.e., the log2 number of
// line segments it will be divided into; see GrWangsFormula::cubic_log2/quadratic_log2), and then
// sort the instance buffer by resolveLevel for efficient batching of indirect draws.
class GrMiddleOutCubicShader : public GrStencilPathShader {
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
    static void WriteDrawCubicsIndirectCmd(GrDrawIndexedIndirectWriter* indirectWriter,
                                           int resolveLevel, uint32_t instanceCount,
                                           uint32_t baseInstance) {
        SkASSERT(resolveLevel > 0 && resolveLevel <= GrTessellationPathRenderer::kMaxResolveLevel);
        // Starting at baseIndex=3, the index buffer triangulates a cubic with 2^kMaxResolveLevel
        // line segments. Each index value corresponds to a parametric T value on the curve. Since
        // the triangles are arranged in "middle-out" order, we can conveniently control the
        // resolveLevel by changing only the indexCount.
        uint32_t indexCount = NumVerticesAtResolveLevel(resolveLevel);
        indirectWriter->writeIndexed(indexCount, 3, instanceCount, baseInstance, 0);
    }

    // For performance reasons we can often express triangles as an indirect cubic draw and sneak
    // them in alongside the other indirect draws. This method configures an indirect draw to emit
    // the triangle [P0, P1, P2] from a 4-point instance.
    static void WriteDrawTrianglesIndirectCmd(GrDrawIndexedIndirectWriter* indirectWriter,
                                              uint32_t instanceCount, uint32_t baseInstance) {
        // Indices 0,1,2 have special index values that emit points P0, P1, and P2 respectively.
        indirectWriter->writeIndexed(3, 0, instanceCount, baseInstance, 0);
    }

    // Returns the index buffer that should be bound when drawing with this shader.
    // (Our vertex shader uses raw index values directly, so there is no vertex buffer.)
    static sk_sp<const GrGpuBuffer> FindOrMakeMiddleOutIndexBuffer(GrResourceProvider*);

    GrMiddleOutCubicShader(const SkMatrix& viewMatrix)
            : GrStencilPathShader(kTessellate_GrMiddleOutCubicShader_ClassID, viewMatrix,
                                  GrPrimitiveType::kTriangles) {
        constexpr static Attribute kInputPtsAttribs[] = {
                {"inputPoints_0_1", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
                {"inputPoints_2_3", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};
        this->setInstanceAttributes(kInputPtsAttribs, 2);
    }

    const char* name() const override { return "tessellate_GrMiddleOutCubicShader"; }

private:
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

    class Impl;
};

#endif

/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrPathTessellationShader_DEFINED
#define GrPathTessellationShader_DEFINED

#include "include/core/SkString.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkColorData.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrUserStencilSettings.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/ganesh/tessellate/GrTessellationShader.h"

#include <cstddef>

class GrAppliedHardClip;
class GrGLSLProgramDataManager;
class GrGLSLVaryingHandler;
class GrGLSLVertexBuilder;
class SkArenaAlloc;
class SkMatrix;
struct GrShaderCaps;

namespace skgpu::tess {
enum class PatchAttribs;
}

// This is the base class for shaders in the GPU tessellator that fill paths.
class GrPathTessellationShader : public GrTessellationShader {
protected:
    using PatchAttribs = skgpu::tess::PatchAttribs;

public:
    // Draws a simple array of triangles.
    static GrPathTessellationShader* MakeSimpleTriangleShader(SkArenaAlloc*,
                                                              const SkMatrix& viewMatrix,
                                                              const SkPMColor4f&);

    // Uses instanced draws to triangulate curves with a "middle-out" topology. Middle-out draws a
    // triangle with vertices at T=[0, 1/2, 1] and then recurses breadth first:
    //
    //   depth=0: T=[0, 1/2, 1]
    //   depth=1: T=[0, 1/4, 2/4], T=[2/4, 3/4, 1]
    //   depth=2: T=[0, 1/8, 2/8], T=[2/8, 3/8, 4/8], T=[4/8, 5/8, 6/8], T=[6/8, 7/8, 1]
    //   ...
    //
    // The shader determines how many segments are required to render each individual curve
    // smoothly, and emits empty triangles at any vertices whose sk_VertexIDs are higher than
    // necessary. It is the caller's responsibility to draw enough vertices per instance for the
    // most complex curve in the batch to render smoothly (i.e., NumTrianglesAtResolveLevel() * 3).
    //
    // If PatchAttribs::kFanPoint is set, an additional triangle is added, connecting the base of
    // the curve to the fan point.
    static GrPathTessellationShader* Make(const GrShaderCaps&,
                                          SkArenaAlloc*,
                                          const SkMatrix& viewMatrix,
                                          const SkPMColor4f&,
                                          PatchAttribs);

    // Returns the stencil settings to use for a standard Redbook "stencil" pass.
    static const GrUserStencilSettings* StencilPathSettings(GrFillRule fillRule) {
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

        return (fillRule == GrFillRule::kNonzero) ? &kIncrDecrStencil : &kInvertStencil;
    }

    // Returns the stencil settings to use for a standard Redbook "fill" pass. Allows non-zero
    // stencil values to pass and write a color, and resets the stencil value back to zero; discards
    // immediately on stencil values of zero.
    static const GrUserStencilSettings* TestAndResetStencilSettings(bool isInverseFill = false) {
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

        constexpr static GrUserStencilSettings kTestAndResetStencilInverted(
            GrUserStencilSettings::StaticInit<
                0x0000,
                // No need to check the clip because the previous stencil pass will have only
                // written to samples already inside the clip.
                GrUserStencilTest::kEqual,
                0xffff,
                GrUserStencilOp::kKeep,
                GrUserStencilOp::kZero,
                0xffff>());

        return isInverseFill ? &kTestAndResetStencilInverted : &kTestAndResetStencil;
    }

    // Creates a pipeline that does not write to the color buffer.
    static const GrPipeline* MakeStencilOnlyPipeline(
            const ProgramArgs&,
            GrAAType,
            const GrAppliedHardClip&,
            GrPipeline::InputFlags = GrPipeline::InputFlags::kNone);

protected:
    constexpr static size_t kMiddleOutVertexStride = 2 * sizeof(float);

    GrPathTessellationShader(ClassID classID,
                             GrPrimitiveType primitiveType,
                             const SkMatrix& viewMatrix,
                             const SkPMColor4f& color,
                             PatchAttribs attribs)
            : GrTessellationShader(classID, primitiveType, viewMatrix, color)
            , fAttribs(attribs) {
    }

    // Default path tessellation shader implementation that manages a uniform matrix and color.
    class Impl : public ProgramImpl {
    public:
        void onEmitCode(EmitArgs&, GrGPArgs*) final;
        void setData(const GrGLSLProgramDataManager&, const GrShaderCaps&,
                     const GrGeometryProcessor&) override;

    protected:
        // float4x3 unpack_rational_cubic(float2 p0, float2 p1, float2 p2, float2 p3) { ...
        //
        // Evaluate our point of interest using numerically stable linear interpolations. We add our
        // own "safe_mix" method to guarantee we get exactly "b" when T=1. The builtin mix()
        // function seems spec'd to behave this way, but empirical results results have shown it
        // does not always.
        static const char* kEvalRationalCubicFn;

        virtual void emitVertexCode(const GrShaderCaps&,
                                    const GrPathTessellationShader&,
                                    GrGLSLVertexBuilder*,
                                    GrGLSLVaryingHandler*,
                                    GrGPArgs*) = 0;

        GrGLSLUniformHandler::UniformHandle fAffineMatrixUniform;
        GrGLSLUniformHandler::UniformHandle fTranslateUniform;
        GrGLSLUniformHandler::UniformHandle fColorUniform;
        SkString fVaryingColorName;
    };

    const PatchAttribs fAttribs;
};

#endif

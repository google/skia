/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrTessellationShader_DEFINED
#define GrTessellationShader_DEFINED

#include "include/core/SkMatrix.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkColorData.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrProgramInfo.h"

#include <cstdint>

class GrAppliedClip;
class GrCaps;
class GrDstProxyView;
class GrPipeline;
class GrProcessorSet;
class GrSurfaceProxyView;
enum class GrAAType : unsigned int;
enum class GrLoadOp;
enum class GrPrimitiveType : uint8_t;
enum class GrXferBarrierFlags;
struct GrUserStencilSettings;

// This is a common base class for shaders in the GPU tessellator.
class GrTessellationShader : public GrGeometryProcessor {
public:
    GrTessellationShader(ClassID classID, GrPrimitiveType primitiveType,
                         const SkMatrix& viewMatrix,
                         const SkPMColor4f& color)
            : GrGeometryProcessor(classID)
            , fPrimitiveType(primitiveType)
            , fViewMatrix(viewMatrix)
            , fColor(color) {
    }

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    const SkPMColor4f& color() const { return fColor;}

    struct ProgramArgs {
        SkArenaAlloc* fArena;
        const GrSurfaceProxyView& fWriteView;
        bool fUsesMSAASurface;
        const GrDstProxyView* fDstProxyView;
        GrXferBarrierFlags fXferBarrierFlags;
        GrLoadOp fColorLoadOp;
        const GrCaps* fCaps;
    };

    static const GrPipeline* MakePipeline(const ProgramArgs&, GrAAType,
                                          GrAppliedClip&&, GrProcessorSet&&);

    static GrProgramInfo* MakeProgram(const ProgramArgs& args,
                                      const GrTessellationShader* shader,
                                      const GrPipeline* pipeline,
                                      const GrUserStencilSettings* stencil) {
        return args.fArena->make<GrProgramInfo>(*args.fCaps, args.fWriteView, args.fUsesMSAASurface,
                                                pipeline, stencil, shader, shader->fPrimitiveType,
                                                args.fXferBarrierFlags, args.fColorLoadOp);
    }

    // SkSL functions that calculate Wang's formula for cubics or conics.
    static const char* WangsFormulaSkSL();

private:
    const GrPrimitiveType fPrimitiveType;
    const SkMatrix fViewMatrix;
    const SkPMColor4f fColor;
};

#endif

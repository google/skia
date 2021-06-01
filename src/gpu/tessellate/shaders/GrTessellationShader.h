/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTessellationShader_DEFINED
#define GrTessellationShader_DEFINED

#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

class SkArenaAlloc;

 // This is a common base class for shaders in the GPU tessellator.
class GrTessellationShader : public GrGeometryProcessor {
public:
    GrTessellationShader(ClassID classID, GrPrimitiveType primitiveType,
                         int tessellationPatchVertexCount, const SkMatrix& viewMatrix,
                         const SkPMColor4f& color)
            : GrGeometryProcessor(classID)
            , fPrimitiveType(primitiveType)
            , fTessellationPatchVertexCount(tessellationPatchVertexCount)
            , fViewMatrix(viewMatrix)
            , fColor(color) {
        if (fTessellationPatchVertexCount) {
            this->setWillUseTessellationShaders();
        }
    }

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    int tessellationPatchVertexCount() const { return fTessellationPatchVertexCount; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    const SkPMColor4f& color() const { return fColor;}

    // Fills in a 4-point patch in such a way that the shader will recognize it as a conic.
    static void WriteConicPatch(const SkPoint pts[3], float w, GrVertexWriter* writer) {
        // Write out the 3 conic points to patch[0..2], the weight to patch[3].x, and then set
        // patch[3].y as NaN to flag this patch as a conic.
        writer->writeArray(pts, 3);
        writer->write(w, GrVertexWriter::kIEEE_32_infinity);
    }
    static void WriteConicPatch(const SkPoint pts[3], float w, SkPoint patch[4]) {
        GrVertexWriter writer(patch);
        WriteConicPatch(pts, w, &writer);
    }

    struct ProgramArgs {
        SkArenaAlloc* fArena;
        const GrSurfaceProxyView& fWriteView;
        const GrXferProcessor::DstProxyView* fDstProxyView;
        GrXferBarrierFlags fXferBarrierFlags;
        GrLoadOp fColorLoadOp;
        const GrCaps* fCaps;
    };

    static const GrPipeline* MakePipeline(const ProgramArgs& args, GrAAType aaType,
                                          GrAppliedClip&& appliedClip,
                                          GrProcessorSet&& processors) {
        auto pipelineFlags = GrPipeline::InputFlags::kNone;
        if (aaType == GrAAType::kMSAA) {
            pipelineFlags |= GrPipeline::InputFlags::kHWAntialias;
        }
        return GrSimpleMeshDrawOpHelper::CreatePipeline(
                args.fCaps, args.fArena, args.fWriteView.swizzle(), std::move(appliedClip),
                *args.fDstProxyView, std::move(processors), pipelineFlags);
    }

    static GrProgramInfo* MakeProgram(const ProgramArgs& args, const GrTessellationShader* shader,
                                      const GrPipeline* pipeline,
                                      const GrUserStencilSettings* stencil) {
        return args.fArena->make<GrProgramInfo>(args.fWriteView, pipeline, stencil, shader,
                                                shader->fPrimitiveType,
                                                shader->fTessellationPatchVertexCount,
                                                args.fXferBarrierFlags, args.fColorLoadOp);
    }

private:
    const GrPrimitiveType fPrimitiveType;
    const int fTessellationPatchVertexCount;
    const SkMatrix fViewMatrix;
    const SkPMColor4f fColor;
};

#endif

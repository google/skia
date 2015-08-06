/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCommandBuilder_DEFINED
#define GrCommandBuilder_DEFINED

#include "GrTargetCommands.h"

class GrBufferedDrawTarget;

class GrCommandBuilder : ::SkNoncopyable {
public:
    typedef GrTargetCommands::Cmd Cmd;
    typedef GrTargetCommands::State State;

    static GrCommandBuilder* Create(GrGpu* gpu, bool reorder);

    virtual ~GrCommandBuilder() {}

    void reset() { fCommands.reset(); }
    void flush(GrBufferedDrawTarget* bufferedDrawTarget) { fCommands.flush(bufferedDrawTarget); }

    virtual Cmd* recordClearStencilClip(const SkIRect& rect,
                                        bool insideClip,
                                        GrRenderTarget* renderTarget);
    virtual Cmd* recordDiscard(GrRenderTarget*);
    virtual Cmd* recordDrawBatch(State*, GrBatch*) = 0;
    virtual Cmd* recordStencilPath(const GrPipelineBuilder&,
                                   const GrPathProcessor*,
                                   const GrPath*,
                                   const GrScissorState&,
                                   const GrStencilSettings&) = 0;
    virtual Cmd* recordDrawPath(State*,
                                const GrPathProcessor*,
                                const GrPath*,
                                const GrStencilSettings&) = 0;
    virtual Cmd* recordDrawPaths(State*,
                                 GrBufferedDrawTarget*,
                                 const GrPathProcessor*,
                                 const GrPathRange*,
                                 const void*,
                                 GrDrawTarget::PathIndexType,
                                 const float transformValues[],
                                 GrDrawTarget::PathTransformType ,
                                 int,
                                 const GrStencilSettings&,
                                 const GrDrawTarget::PipelineInfo&) = 0;
    virtual Cmd* recordClear(const SkIRect& rect,
                             GrColor,
                             GrRenderTarget*);
    virtual Cmd* recordCopySurface(GrSurface* dst,
                                   GrSurface* src,
                                   const SkIRect& srcRect,
                                   const SkIPoint& dstPoint);
    virtual Cmd* recordXferBarrierIfNecessary(const GrPipeline&, const GrCaps&);

protected:
    typedef GrTargetCommands::DrawBatch DrawBatch;
    typedef GrTargetCommands::StencilPath StencilPath;
    typedef GrTargetCommands::DrawPath DrawPath;
    typedef GrTargetCommands::DrawPaths DrawPaths;
    typedef GrTargetCommands::Clear Clear;
    typedef GrTargetCommands::ClearStencilClip ClearStencilClip;
    typedef GrTargetCommands::CopySurface CopySurface;
    typedef GrTargetCommands::XferBarrier XferBarrier;

    GrCommandBuilder(GrGpu* gpu) : fCommands(gpu) {}

    GrTargetCommands::CmdBuffer* cmdBuffer() { return fCommands.cmdBuffer(); }
    GrBatchTarget* batchTarget() { return fCommands.batchTarget(); }

private:
    GrTargetCommands fCommands;

};

#endif

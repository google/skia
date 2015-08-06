/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrImmediateDrawTarget_DEFINED
#define GrImmediateDrawTarget_DEFINED

#include "GrDrawTarget.h"

#include "GrBatchTarget.h"

/**
 * A debug GrDrawTarget which immediately flushes every command it receives
 */
class GrImmediateDrawTarget : public GrClipTarget {
public:

    /**
     * Creates a GrImmediateDrawTarget
     *
     * @param context    the context object that owns this draw buffer.
     */
    GrImmediateDrawTarget(GrContext* context);

    ~GrImmediateDrawTarget() override;

    void clearStencilClip(const SkIRect& rect,
                          bool insideClip,
                          GrRenderTarget* renderTarget) override;

    void discard(GrRenderTarget*) override;

private:
    void onReset() override;
    void onFlush() override;

    // overrides from GrDrawTarget
    void onDrawBatch(GrBatch*, const PipelineInfo&) override;
    void onStencilPath(const GrPipelineBuilder&,
                       const GrPathProcessor*,
                       const GrPath*,
                       const GrScissorState&,
                       const GrStencilSettings&) override {
        SkFAIL("Only batch implemented\n");
    }
    void onDrawPath(const GrPathProcessor*,
                    const GrPath*,
                    const GrStencilSettings&,
                    const PipelineInfo&) override {
        SkFAIL("Only batch implemented\n");
    }
    void onDrawPaths(const GrPathProcessor*,
                     const GrPathRange*,
                     const void* indices,
                     PathIndexType,
                     const float transformValues[],
                     PathTransformType,
                     int count,
                     const GrStencilSettings&,
                     const PipelineInfo&) override {
        SkFAIL("Only batch implemented\n");
    }
    void onClear(const SkIRect& rect,
                 GrColor color,
                 GrRenderTarget* renderTarget) override;
    void onCopySurface(GrSurface* dst,
                       GrSurface* src,
                       const SkIRect& srcRect,
                       const SkIPoint& dstPoint) override;

    bool isIssued(uint32_t drawID) override { return drawID != fDrawID; }

    bool SK_WARN_UNUSED_RESULT setupPipelineAndShouldDraw(void* pipelineAddr,
                                                          const GrDrawTarget::PipelineInfo&);

    void recordXferBarrierIfNecessary(const GrPipeline*);

    GrBatchTarget fBatchTarget;
    uint32_t fDrawID;

    typedef GrClipTarget INHERITED;
};

#endif

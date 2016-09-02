/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClearStencilClipBatch_DEFINED
#define GrClearStencilClipBatch_DEFINED

#include "GrBatch.h"
#include "GrBatchFlushState.h"
#include "GrGpu.h"
#include "GrGpuCommandBuffer.h"
#include "GrRenderTarget.h"

class GrClearStencilClipBatch final : public GrBatch {
public:
    DEFINE_BATCH_CLASS_ID

    GrClearStencilClipBatch(const SkIRect& rect, bool insideClip, GrRenderTarget* rt)
        : INHERITED(ClassID())
        , fRect(rect)
        , fInsideClip(insideClip)
        , fRenderTarget(rt) {
        this->setBounds(SkRect::Make(rect), HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "ClearStencilClip"; }

    uint32_t renderTargetUniqueID() const override { return fRenderTarget.get()->getUniqueID(); }
    GrRenderTarget* renderTarget() const override { return fRenderTarget.get(); }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("Rect [L: %d, T: %d, R: %d, B: %d], IC: %d, RT: %d",
                      fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom, fInsideClip,
                      fRenderTarget.get()->getUniqueID());
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override { return false; }

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state) override {
        state->commandBuffer()->clearStencilClip(fRect, fInsideClip, fRenderTarget.get());
    }

    SkIRect                                                 fRect;
    bool                                                    fInsideClip;
    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType>    fRenderTarget;

    typedef GrBatch INHERITED;
};

#endif

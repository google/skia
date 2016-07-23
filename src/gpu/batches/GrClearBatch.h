/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClearBatch_DEFINED
#define GrClearBatch_DEFINED

#include "GrBatch.h"
#include "GrBatchFlushState.h"
#include "GrGpu.h"
#include "GrGpuCommandBuffer.h"
#include "GrRenderTarget.h"

class GrClearBatch final : public GrBatch {
public:
    DEFINE_BATCH_CLASS_ID

    GrClearBatch(const SkIRect& rect,  GrColor color, GrRenderTarget* rt)
        : INHERITED(ClassID())
        , fRect(rect)
        , fColor(color)
        , fRenderTarget(rt) {
        fBounds = SkRect::Make(rect);
    }

    const char* name() const override { return "Clear"; }

    uint32_t renderTargetUniqueID() const override { return fRenderTarget.get()->getUniqueID(); }
    GrRenderTarget* renderTarget() const override { return fRenderTarget.get(); }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("Color: 0x%08x, Rect [L: %d, T: %d, R: %d, B: %d], RT: %d",
                      fColor, fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom,
                      fRenderTarget.get()->getUniqueID());
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        // This could be much more complicated. Currently we look at cases where the new clear
        // contains the old clear, or when the new clear is a subset of the old clear and is the
        // same color.
        GrClearBatch* cb = t->cast<GrClearBatch>();
        SkASSERT(cb->fRenderTarget == fRenderTarget);
        if (cb->fRect.contains(fRect)) {
            fRect = cb->fRect;
            fBounds = cb->fBounds;
            fColor = cb->fColor;
            return true;
        } else if (cb->fColor == fColor && fRect.contains(cb->fRect)) {
            return true;
        }
        return false;
    }

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state) override {
        state->commandBuffer()->clear(fRect, fColor, fRenderTarget.get());
    }

    SkIRect                                                 fRect;
    GrColor                                                 fColor;
    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType>    fRenderTarget;

    typedef GrBatch INHERITED;
};

class GrClearStencilClipBatch final : public GrBatch {
public:
    DEFINE_BATCH_CLASS_ID

    GrClearStencilClipBatch(const SkIRect& rect, bool insideClip, GrRenderTarget* rt)
        : INHERITED(ClassID())
        , fRect(rect)
        , fInsideClip(insideClip)
        , fRenderTarget(rt) {
        fBounds = SkRect::Make(rect);
    }

    const char* name() const override { return "ClearStencilClip"; }

    uint32_t renderTargetUniqueID() const override { return fRenderTarget.get()->getUniqueID(); }
    GrRenderTarget* renderTarget() const override { return fRenderTarget.get(); }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("Rect [L: %d, T: %d, R: %d, B: %d], IC: %d, RT: 0x%p",
                      fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom, fInsideClip,
                      fRenderTarget.get());
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

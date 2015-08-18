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
#include "GrRenderTarget.h"

class GrClearBatch final : public GrBatch {
public:
    GrClearBatch(const SkIRect& rect,  GrColor color, GrRenderTarget* rt)
        : fRect(rect)
        , fColor(color)
        , fRenderTarget(rt) {
        this->initClassID<GrClearBatch>();
        fBounds = SkRect::Make(rect);
    }

    const char* name() const override { return "Clear"; }

    uint32_t renderTargetUniqueID() const override { return fRenderTarget.get()->getUniqueID(); }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("Color: 0x%08x, Rect [L: %d, T: %d, R: %d, B: %d], RT: 0x%p",
                      fColor, fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom,
                      fRenderTarget.get());
        return string;
    }

private:
    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        // We could combine clears. TBD how much complexity to put here.
        return false;
    }

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state) override {
        state->gpu()->clear(fRect, fColor, fRenderTarget.get());
    }

    SkIRect                                                 fRect;
    GrColor                                                 fColor;
    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType>    fRenderTarget;
};

#endif

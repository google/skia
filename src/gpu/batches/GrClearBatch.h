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
#include "GrFixedClip.h"
#include "GrGpu.h"
#include "GrGpuCommandBuffer.h"
#include "GrRenderTarget.h"

class GrClearBatch final : public GrBatch {
public:
    DEFINE_BATCH_CLASS_ID

    static sk_sp<GrClearBatch> Make(const GrFixedClip& clip, GrColor color, GrRenderTarget* rt) {
        sk_sp<GrClearBatch> batch(new GrClearBatch(clip, color, rt));
        if (!batch->renderTarget()) {
            return nullptr; // The clip did not contain any pixels within the render target.
        }
        return batch;
    }

    const char* name() const override { return "Clear"; }

    uint32_t renderTargetUniqueID() const override { return fRenderTarget.get()->uniqueID(); }
    GrRenderTarget* renderTarget() const override { return fRenderTarget.get(); }

    SkString dumpInfo() const override {
        SkString string("Scissor [");
        if (fClip.scissorEnabled()) {
            const SkIRect& r = fClip.scissorRect();
            string.appendf("L: %d, T: %d, R: %d, B: %d", r.fLeft, r.fTop, r.fRight, r.fBottom);
        }
        string.appendf("], Color: 0x%08x, RT: %d", fColor, fRenderTarget.get()->uniqueID());
        string.append(INHERITED::dumpInfo());
        return string;
    }

    void setColor(GrColor color) { fColor = color; }

private:
    GrClearBatch(const GrFixedClip& clip, GrColor color, GrRenderTarget* rt)
        : INHERITED(ClassID())
        , fClip(clip)
        , fColor(color) {
        SkIRect rtRect = SkIRect::MakeWH(rt->width(), rt->height());
        if (fClip.scissorEnabled()) {
            // Don't let scissors extend outside the RT. This may improve batching.
            if (!fClip.intersect(rtRect)) {
                return;
            }
            if (fClip.scissorRect() == rtRect) {
                fClip.disableScissor();
            }
        }
        this->setBounds(SkRect::Make(fClip.scissorEnabled() ? fClip.scissorRect() : rtRect),
                        HasAABloat::kNo, IsZeroArea::kNo);
        fRenderTarget.reset(rt);
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        // This could be much more complicated. Currently we look at cases where the new clear
        // contains the old clear, or when the new clear is a subset of the old clear and is the
        // same color.
        GrClearBatch* cb = t->cast<GrClearBatch>();
        SkASSERT(cb->fRenderTarget == fRenderTarget);
        if (!fClip.windowRectsState().cheapEqualTo(cb->fClip.windowRectsState())) {
            return false;
        }
        if (cb->contains(this)) {
            fClip = cb->fClip;
            this->replaceBounds(*t);
            fColor = cb->fColor;
            return true;
        } else if (cb->fColor == fColor && this->contains(cb)) {
            return true;
        }
        return false;
    }

    bool contains(const GrClearBatch* that) const {
        // The constructor ensures that scissor gets disabled on any clip that fills the entire RT.
        return !fClip.scissorEnabled() ||
               (that->fClip.scissorEnabled() &&
                fClip.scissorRect().contains(that->fClip.scissorRect()));
    }

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state) override {
        state->commandBuffer()->clear(fClip, fColor, fRenderTarget.get());
    }

    GrFixedClip                                             fClip;
    GrColor                                                 fColor;
    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType>    fRenderTarget;

    typedef GrBatch INHERITED;
};

#endif

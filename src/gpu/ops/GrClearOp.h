/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClearOp_DEFINED
#define GrClearOp_DEFINED

#include "GrFixedClip.h"
#include "GrGpu.h"
#include "GrGpuCommandBuffer.h"
#include "GrOp.h"
#include "GrOpFlushState.h"
#include "GrRenderTarget.h"

class GrClearOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrClearOp> Make(const GrFixedClip& clip, GrColor color,
                                           GrRenderTarget* rt) {
        std::unique_ptr<GrClearOp> op(new GrClearOp(clip, color, rt));
        if (!op->fRenderTarget) {
            return nullptr; // The clip did not contain any pixels within the render target.
        }
        return op;
    }

    static std::unique_ptr<GrClearOp> Make(const SkIRect& rect, GrColor color, GrRenderTarget* rt,
                                           bool fullScreen) {
        return std::unique_ptr<GrClearOp>(new GrClearOp(rect, color, rt, fullScreen));
    }

    const char* name() const override { return "Clear"; }

    SkString dumpInfo() const override {
        SkString string("Scissor [");
        if (fClip.scissorEnabled()) {
            const SkIRect& r = fClip.scissorRect();
            string.appendf("L: %d, T: %d, R: %d, B: %d", r.fLeft, r.fTop, r.fRight, r.fBottom);
        }
        string.appendf("], Color: 0x%08x, RT: %d", fColor,
                                                   fRenderTarget.get()->uniqueID().asUInt());
        string.append(INHERITED::dumpInfo());
        return string;
    }

    void setColor(GrColor color) { fColor = color; }

private:
    GrClearOp(const GrFixedClip& clip, GrColor color, GrRenderTarget* rt)
        : INHERITED(ClassID())
        , fClip(clip)
        , fColor(color) {
        SkIRect rtRect = SkIRect::MakeWH(rt->width(), rt->height());
        if (fClip.scissorEnabled()) {
            // Don't let scissors extend outside the RT. This may improve op combining.
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

    GrClearOp(const SkIRect& rect, GrColor color, GrRenderTarget* rt, bool fullScreen)
        : INHERITED(ClassID())
        , fClip(GrFixedClip(rect))
        , fColor(color)
        , fRenderTarget(rt) {
        if (fullScreen) {
            fClip.disableScissor();
        }
        this->setBounds(SkRect::Make(rect), HasAABloat::kNo, IsZeroArea::kNo);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        // This could be much more complicated. Currently we look at cases where the new clear
        // contains the old clear, or when the new clear is a subset of the old clear and is the
        // same color.
        GrClearOp* cb = t->cast<GrClearOp>();
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

    bool contains(const GrClearOp* that) const {
        // The constructor ensures that scissor gets disabled on any clip that fills the entire RT.
        return !fClip.scissorEnabled() ||
               (that->fClip.scissorEnabled() &&
                fClip.scissorRect().contains(that->fClip.scissorRect()));
    }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state, const SkRect& /*bounds*/) override {
        state->commandBuffer()->clear(fRenderTarget.get(), fClip, fColor);
    }

    GrFixedClip                                             fClip;
    GrColor                                                 fColor;
    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType>    fRenderTarget;

    typedef GrOp INHERITED;
};

#endif

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
                                           GrRenderTargetContext* rtc) {
        if (clip.scissorEnabled() && clip.scissorRect().isEmpty()) {
            return nullptr;
        }

        // MDB TODO: remove this
        if (!rtc->accessRenderTarget()) {
            return nullptr;
        }

        std::unique_ptr<GrClearOp> op(new GrClearOp(clip, color, rtc));
        if (!op->fRenderTargetProxy1) {
            return nullptr; // The clip did not contain any pixels within the render target.
        }
        return op;
    }

    static std::unique_ptr<GrClearOp> Make(const SkIRect& rect, GrColor color,
                                           GrRenderTargetContext* rtc,
                                           bool fullScreen) {
        SkASSERT(fullScreen || !rect.isEmpty());

        // MDB TODO: remove this
        if (!rtc->accessRenderTarget()) {
            return nullptr;
        }

        return std::unique_ptr<GrClearOp>(new GrClearOp(rect, color, rtc, fullScreen));
    }

    const char* name() const override { return "Clear"; }

    SkString dumpInfo() const override {
        SkString string("Scissor [");
        if (fClip.scissorEnabled()) {
            const SkIRect& r = fClip.scissorRect();
            string.appendf("L: %d, T: %d, R: %d, B: %d", r.fLeft, r.fTop, r.fRight, r.fBottom);
        }
        string.appendf("], Color: 0x%08x, RT: %d", fColor,
                                                   fRenderTargetProxy1->uniqueID().asUInt());
        string.append(INHERITED::dumpInfo());
        return string;
    }

    void setColor(GrColor color) { fColor = color; }

private:
    GrClearOp(const GrFixedClip& clip, GrColor color, GrRenderTargetContext* rtc)
        : INHERITED(ClassID())
        , fClip(clip)
        , fColor(color)
        , fRenderTargetProxy1(rtc->asRenderTargetProxyRef()) {
        SkIRect rtRect = SkIRect::MakeWH(fRenderTargetProxy1->width(), fRenderTargetProxy1->height());
        if (GrResourceProvider::IsFunctionallyExact(fRenderTargetProxy1.get()) && fClip.scissorEnabled()) {
            // Don't let scissors extend outside the RT. This may improve op combining.
            if (!fClip.intersect(rtRect)) {
                fRenderTargetProxy1 = nullptr;
                return;
            }
            if (fClip.scissorRect() == rtRect) {
                fClip.disableScissor();
            }
        }
        this->setBounds(SkRect::Make(fClip.scissorEnabled() ? fClip.scissorRect() : rtRect),
                        HasAABloat::kNo, IsZeroArea::kNo);
        fRenderTarget1.reset(rtc->accessRenderTarget());
    }

    GrClearOp(const SkIRect& rect, GrColor color, GrRenderTargetContext* rtc, bool fullScreen)
        : INHERITED(ClassID())
        , fClip(GrFixedClip(rect))
        , fColor(color)
        , fRenderTargetProxy1(rtc->asRenderTargetProxyRef()) {

        if (fullScreen) {
            fClip.disableScissor();
        }
        this->setBounds(SkRect::Make(rect), HasAABloat::kNo, IsZeroArea::kNo);
        fRenderTarget1.reset(rtc->accessRenderTarget());
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        // This could be much more complicated. Currently we look at cases where the new clear
        // contains the old clear, or when the new clear is a subset of the old clear and is the
        // same color.
        GrClearOp* cb = t->cast<GrClearOp>();
        SkASSERT(cb->fRenderTarget1 == fRenderTarget1);
        SkASSERT(cb->fRenderTargetProxy1 == fRenderTargetProxy1);
        if (fClip.windowRectsState() != cb->fClip.windowRectsState()) {
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

    void onExecute(GrOpFlushState* state) override {
        state->commandBuffer()->clear(fRenderTarget1.get(), fClip, fColor);
    }

    GrFixedClip                                             fClip;
    GrColor                                                 fColor;
    sk_sp<GrRenderTargetProxy>                              fRenderTargetProxy1;
    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType>    fRenderTarget1;

    typedef GrOp INHERITED;
};

#endif

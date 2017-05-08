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

    // MDB TODO: replace the renderTargetContext with just the renderTargetProxy.
    // For now, we need the renderTargetContext for its accessRenderTarget powers.
    static std::unique_ptr<GrClearOp> Make(const GrFixedClip& clip, GrColor color,
                                           GrRenderTargetContext* rtc) {
        const SkIRect rtRect = SkIRect::MakeWH(rtc->width(), rtc->height());
        if (clip.scissorEnabled() && !SkIRect::Intersects(clip.scissorRect(), rtRect)) {
            return nullptr;
        }

        // MDB TODO: remove this. In this hybrid state we need to be sure the RT is instantiable
        // so it can carry the IO refs. In the future we will just get the proxy and
        // it carry the IO refs.
        if (!rtc->accessRenderTarget()) {
            return nullptr;
        }

        return std::unique_ptr<GrClearOp>(new GrClearOp(clip, color, rtc));
    }

    // MDB TODO: replace the renderTargetContext with just the renderTargetProxy.
    static std::unique_ptr<GrClearOp> Make(const SkIRect& rect, GrColor color,
                                           GrRenderTargetContext* rtc,
                                           bool fullScreen) {
        SkASSERT(fullScreen || !rect.isEmpty());

        // MDB TODO: remove this. See above comment.
        if (!rtc->accessRenderTarget()) {
            return nullptr;
        }

        return std::unique_ptr<GrClearOp>(new GrClearOp(rect, color, rtc, fullScreen));
    }

    const char* name() const override { return "Clear"; }

    SkString dumpInfo() const override {
        SkString string;
        string.appendf("rtID: %d proxyID: %d Scissor [",
                       fRenderTarget.get()->uniqueID().asUInt(),
                       fProxyUniqueID.asUInt());
        if (fClip.scissorEnabled()) {
            const SkIRect& r = fClip.scissorRect();
            string.appendf("L: %d, T: %d, R: %d, B: %d", r.fLeft, r.fTop, r.fRight, r.fBottom);
        } else {
            string.append("disabled");
        }
        string.appendf("], Color: 0x%08x ", fColor);
        string.append(INHERITED::dumpInfo());
        return string;
    }

    void setColor(GrColor color) { fColor = color; }

private:
    GrClearOp(const GrFixedClip& clip, GrColor color, GrRenderTargetContext* rtc)
        : INHERITED(ClassID())
        , fClip(clip)
        , fColor(color)
        , fProxyUniqueID(rtc->asSurfaceProxy()->uniqueID()) {

        GrSurfaceProxy* proxy = rtc->asSurfaceProxy();
        const SkIRect rtRect = SkIRect::MakeWH(proxy->width(), proxy->height());
        if (fClip.scissorEnabled()) {
            // Don't let scissors extend outside the RT. This may improve op combining.
            if (!fClip.intersect(rtRect)) {
                SkASSERT(0);  // should be caught upstream
                fClip = GrFixedClip(SkIRect::MakeEmpty());
            }

            if (GrResourceProvider::IsFunctionallyExact(proxy) && fClip.scissorRect() == rtRect) {
                fClip.disableScissor();
            }
        }
        this->setBounds(SkRect::Make(fClip.scissorEnabled() ? fClip.scissorRect() : rtRect),
                        HasAABloat::kNo, IsZeroArea::kNo);
        fRenderTarget.reset(rtc->accessRenderTarget());
    }

    GrClearOp(const SkIRect& rect, GrColor color, GrRenderTargetContext* rtc, bool fullScreen)
        : INHERITED(ClassID())
        , fClip(GrFixedClip(rect))
        , fColor(color)
        , fProxyUniqueID(rtc->asSurfaceProxy()->uniqueID()) {

        if (fullScreen) {
            fClip.disableScissor();
        }
        this->setBounds(SkRect::Make(rect), HasAABloat::kNo, IsZeroArea::kNo);
        fRenderTarget.reset(rtc->accessRenderTarget());
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        // This could be much more complicated. Currently we look at cases where the new clear
        // contains the old clear, or when the new clear is a subset of the old clear and is the
        // same color.
        GrClearOp* cb = t->cast<GrClearOp>();
        SkASSERT(cb->fRenderTarget == fRenderTarget);
        SkASSERT(cb->fProxyUniqueID == fProxyUniqueID);
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
        // MDB TODO: instantiate the renderTarget from the proxy in here
        state->commandBuffer()->clear(fRenderTarget.get(), fClip, fColor);
    }

    GrFixedClip                                             fClip;
    GrColor                                                 fColor;

    // MDB TODO: remove this. When the renderTargetProxy carries the refs this will be redundant.
    GrSurfaceProxy::UniqueID                                fProxyUniqueID;
    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType>    fRenderTarget;

    typedef GrOp INHERITED;
};

#endif

/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClearStencilClipOp_DEFINED
#define GrClearStencilClipOp_DEFINED

#include "GrFixedClip.h"
#include "GrGpu.h"
#include "GrGpuCommandBuffer.h"
#include "GrOp.h"
#include "GrOpFlushState.h"
#include "GrRenderTarget.h"

class GrClearStencilClipOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static sk_sp<GrOp> Make(const GrFixedClip& clip, bool insideStencilMask, GrRenderTarget* rt) {
        return sk_sp<GrOp>(new GrClearStencilClipOp(clip, insideStencilMask, rt));
    }

    const char* name() const override { return "ClearStencilClip"; }

    // TODO: this needs to be updated to return GrSurfaceProxy::UniqueID
    GrGpuResource::UniqueID renderTargetUniqueID() const override {
        return fRenderTarget.get()->uniqueID();
    }

    SkString dumpInfo() const override {
        SkString string("Scissor [");
        if (fClip.scissorEnabled()) {
            const SkIRect& r = fClip.scissorRect();
            string.appendf("L: %d, T: %d, R: %d, B: %d", r.fLeft, r.fTop, r.fRight, r.fBottom);
        }
        string.appendf("], IC: %d, RT: %d", fInsideStencilMask,
                       fRenderTarget.get()->uniqueID().asUInt());
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    GrClearStencilClipOp(const GrFixedClip& clip, bool insideStencilMask, GrRenderTarget* rt)
            : INHERITED(ClassID())
            , fClip(clip)
            , fInsideStencilMask(insideStencilMask)
            , fRenderTarget(rt) {
        const SkRect& bounds = fClip.scissorEnabled() ? SkRect::Make(fClip.scissorRect())
                                                      : SkRect::MakeIWH(rt->width(), rt->height());
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override { return false; }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state, const SkRect& /*bounds*/) override {
        state->commandBuffer()->clearStencilClip(fRenderTarget.get(), fClip, fInsideStencilMask);
    }

    const GrFixedClip fClip;
    const bool fInsideStencilMask;
    GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;

    typedef GrOp INHERITED;
};

#endif

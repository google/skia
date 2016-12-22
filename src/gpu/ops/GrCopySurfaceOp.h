/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCopySurfaceOp_DEFINED
#define GrCopySurfaceOp_DEFINED

#include "GrGpu.h"
#include "GrOp.h"
#include "GrOpFlushState.h"
#include "GrRenderTarget.h"

class GrCopySurfaceOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    /** This should not really be exposed as Create() will apply this clipping, but there is
     *  currently a workaround in GrContext::copySurface() for non-render target dsts that relies
     *  on it. */
    static bool ClipSrcRectAndDstPoint(const GrSurface* dst,
                                       const GrSurface* src,
                                       const SkIRect& srcRect,
                                       const SkIPoint& dstPoint,
                                       SkIRect* clippedSrcRect,
                                       SkIPoint* clippedDstPoint);

    static sk_sp<GrOp> Make(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                            const SkIPoint& dstPoint);

    const char* name() const override { return "CopySurface"; }

    SkString dumpInfo() const override {
        SkString string;
        string.printf(
                "SRC: 0x%p, DST: 0x%p, SRECT: [L: %d, T: %d, R: %d, B: %d], "
                "DPT:[X: %d, Y: %d]",
                fDst.get(), fSrc.get(), fSrcRect.fLeft, fSrcRect.fTop, fSrcRect.fRight,
                fSrcRect.fBottom, fDstPoint.fX, fDstPoint.fY);
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    GrCopySurfaceOp(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                    const SkIPoint& dstPoint)
            : INHERITED(ClassID()), fDst(dst), fSrc(src), fSrcRect(srcRect), fDstPoint(dstPoint) {
        SkRect bounds =
                SkRect::MakeXYWH(SkIntToScalar(dstPoint.fX), SkIntToScalar(dstPoint.fY),
                                 SkIntToScalar(srcRect.width()), SkIntToScalar(srcRect.height()));
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
    }

    bool onCombineIfPossible(GrOp* that, const GrCaps& caps) override { return false; }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state, const SkRect& /*bounds*/) override {
        if (!state->commandBuffer()) {
            state->gpu()->copySurface(fDst.get(), fSrc.get(), fSrcRect, fDstPoint);
        } else {
            // Currently we are not sending copies through the GrGpuCommandBuffer. See comment in
            // renderTargetUniqueID().
            SkASSERT(false);
        }
    }

    GrPendingIOResource<GrSurface, kWrite_GrIOType> fDst;
    GrPendingIOResource<GrSurface, kRead_GrIOType> fSrc;
    SkIRect fSrcRect;
    SkIPoint fDstPoint;

    typedef GrOp INHERITED;
};

#endif

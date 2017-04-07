/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCopySurfaceOp_DEFINED
#define GrCopySurfaceOp_DEFINED

#include "GrOp.h"
#include "GrOpFlushState.h"

class GrCopySurfaceOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    // MDB TODO: remove the resourceProvider parameter
    static std::unique_ptr<GrOp> Make(GrResourceProvider*,
                                      GrSurfaceProxy* dst, GrSurfaceProxy* src,
                                      const SkIRect& srcRect,
                                      const SkIPoint& dstPoint);

    const char* name() const override { return "CopySurface"; }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("src: (proxyID: %d, rtID: %d), dst: (proxyID: %d, rtID: %d), "
                      "srcRect: [L: %d, T: %d, R: %d, B: %d], dstPt: [X: %d, Y: %d]",
                      fSrcProxyID.asUInt(), fSrc.get()->uniqueID().asUInt(),
                      fDstProxyID.asUInt(), fDst.get()->uniqueID().asUInt(),
                      fSrcRect.fLeft, fSrcRect.fTop, fSrcRect.fRight, fSrcRect.fBottom,
                      fDstPoint.fX, fDstPoint.fY);
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    GrCopySurfaceOp(GrSurface* dst, GrSurface* src,
                    GrSurfaceProxy::UniqueID dstID, GrSurfaceProxy::UniqueID srcID,
                    const SkIRect& srcRect, const SkIPoint& dstPoint)
            : INHERITED(ClassID())
            , fDstProxyID(dstID)
            , fSrcProxyID(srcID)
            , fDst(dst)
            , fSrc(src)
            , fSrcRect(srcRect)
            , fDstPoint(dstPoint) {
        SkRect bounds =
                SkRect::MakeXYWH(SkIntToScalar(dstPoint.fX), SkIntToScalar(dstPoint.fY),
                                 SkIntToScalar(srcRect.width()), SkIntToScalar(srcRect.height()));
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
    }

    bool onCombineIfPossible(GrOp* that, const GrCaps& caps) override { return false; }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override {
        if (!state->commandBuffer()) {
            state->gpu()->copySurface(fDst.get(), fSrc.get(), fSrcRect, fDstPoint);
        } else {
            // Currently we are not sending copies through the GrGpuCommandBuffer. See comment in
            // renderTargetUniqueID().
            SkASSERT(false);
        }
    }

    // MDB TODO: remove the proxy IDs once the GrSurfaceProxy carries the ref since they will
    // be redundant
    GrSurfaceProxy::UniqueID                        fDstProxyID;
    GrSurfaceProxy::UniqueID                        fSrcProxyID;
    GrPendingIOResource<GrSurface, kWrite_GrIOType> fDst;
    GrPendingIOResource<GrSurface, kRead_GrIOType>  fSrc;
    SkIRect                                         fSrcRect;
    SkIPoint                                        fDstPoint;

    typedef GrOp INHERITED;
};

#endif

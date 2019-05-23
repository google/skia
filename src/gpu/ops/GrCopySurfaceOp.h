/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCopySurfaceOp_DEFINED
#define GrCopySurfaceOp_DEFINED

#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/ops/GrOp.h"

class GrRecordingContext;

class GrCopySurfaceOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrOp> Make(GrRecordingContext*,
                                      GrSurfaceProxy* dst,
                                      GrSurfaceProxy* src,
                                      const SkIRect& srcRect,
                                      const SkIPoint& dstPoint);

    const char* name() const override { return "CopySurface"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        func(fSrc.get(), GrMipMapped::kNo);
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString string;
        string = INHERITED::dumpInfo();
        string.appendf(
                "srcProxyID: %d,\n"
                "srcRect: [ L: %d, T: %d, R: %d, B: %d ], dstPt: [ X: %d, Y: %d ]\n",
                fSrc.get()->uniqueID().asUInt(), fSrcRect.fLeft, fSrcRect.fTop, fSrcRect.fRight,
                fSrcRect.fBottom, fDstPoint.fX, fDstPoint.fY);
        return string;
    }
#endif

private:
    friend class GrOpMemoryPool; // for ctor

    GrCopySurfaceOp(GrSurfaceProxy* src, const SkIRect& srcRect, const SkIPoint& dstPoint)
            : INHERITED(ClassID())
            , fSrc(src)
            , fSrcRect(srcRect)
            , fDstPoint(dstPoint) {
        SkRect bounds =
                SkRect::MakeXYWH(SkIntToScalar(dstPoint.fX), SkIntToScalar(dstPoint.fY),
                                 SkIntToScalar(srcRect.width()), SkIntToScalar(srcRect.height()));
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    GrPendingIOResource<GrSurfaceProxy, kRead_GrIOType>  fSrc;
    SkIRect                                              fSrcRect;
    SkIPoint                                             fDstPoint;

    typedef GrOp INHERITED;
};

#endif

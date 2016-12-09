/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCopySurfaceBatch_DEFINED
#define GrCopySurfaceBatch_DEFINED

#include "GrGpu.h"
#include "GrOp.h"
#include "GrOpFlushState.h"
#include "GrRenderTarget.h"

class GrCopySurfaceBatch final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    /** This should not really be exposed as Create() will apply this clipping, but there is
     *  currently a workaround in GrContext::copySurface() for non-render target dsts that relies
     *  on it. */
    static bool ClipSrcRectAndDstPoint(const GrSurfaceProxy* dst,
                                       const GrSurface* src,
                                       const SkIRect& srcRect,
                                       const SkIPoint& dstPoint,
                                       SkIRect* clippedSrcRect,
                                       SkIPoint* clippedDstPoint);

    static GrOp* Create(GrTextureProvider* textureProvider,
                        GrSurfaceProxy* dst, GrSurface* src, const SkIRect& srcRect,
                        const SkIPoint& dstPoint);

    const char* name() const override { return "CopySurface"; }

#if 0
    // TODO: this needs to be updated to return GrSurfaceProxy::UniqueID
    GrGpuResource::UniqueID renderTargetUniqueID() const override {
        // Copy surface doesn't work through a GrGpuCommandBuffer. By returning an invalid RT ID we
        // force the caller to end the previous command buffer and execute this copy before
        // beginning a new one.
        return GrGpuResource::UniqueID::InvalidID();
    }
#endif

    GrSurfaceProxy::UniqueID renderTargetProxyUniqueID() const override {
        return fDst.get()->uniqueID();
    }
    // TODO: this seems odd - figure it out and add a comment!
    GrRenderTargetProxy* renderTargetProxy() const override { return nullptr; }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("SRC: 0x%p, DST: 0x%p, SRECT: [L: %d, T: %d, R: %d, B: %d], "
                      "DPT:[X: %d, Y: %d]",
                      fDst.get(), fSrc.get(), fSrcRect.fLeft, fSrcRect.fTop, fSrcRect.fRight,
                      fSrcRect.fBottom, fDstPoint.fX, fDstPoint.fY);
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    GrCopySurfaceBatch(GrTextureProvider* textureProvider,
                       GrSurfaceProxy* dst, GrSurface* src, const SkIRect& srcRect,
                       const SkIPoint& dstPoint)
        : INHERITED(ClassID())
        , fTextureProvider(textureProvider)
        , fDst(dst)
        , fSrc(src)
        , fSrcRect(srcRect)
        , fDstPoint(dstPoint) {
        SkRect bounds =
                SkRect::MakeXYWH(SkIntToScalar(dstPoint.fX), SkIntToScalar(dstPoint.fY),
                                 SkIntToScalar(srcRect.width()), SkIntToScalar(srcRect.height()));
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
    }

    bool onCombineIfPossible(GrOp* that, const GrCaps& caps, GrTextureProvider* texProvider) override { return false; }

    void onPrepare(GrOpFlushState*) override {}

    void onDraw(GrOpFlushState* state, const SkRect& /*bounds*/) override {
        if (!state->commandBuffer()) {
            GrSurface* surf = fDst.get()->instantiate(fTextureProvider);
            if (surf) {
                state->gpu()->copySurface(surf, fSrc.get(), fSrcRect, fDstPoint);
            }
        } else {
            // Currently we are not sending copies through the GrGpuCommandBuffer. See comment in
            // renderTargetUniqueID().
            SkASSERT(false);
        }
    }

    GrTextureProvider*                                   fTextureProvider;
    GrPendingIOResource<GrSurfaceProxy, kWrite_GrIOType> fDst;
    GrPendingIOResource<GrSurface, kRead_GrIOType>       fSrc;
    SkIRect                                              fSrcRect;
    SkIPoint                                             fDstPoint;

    typedef GrOp INHERITED;
};

#endif

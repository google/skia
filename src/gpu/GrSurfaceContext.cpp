/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSurfaceContext.h"
#include "GrContextPriv.h"
#include "GrDrawingManager.h"
#include "GrOpList.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
#include "SkGr.h"
#include "../private/GrAuditTrail.h"

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(this->singleOwner());)
#define RETURN_FALSE_IF_ABANDONED  if (this->fContext->priv().abandoned()) { return false; }

// In MDB mode the reffing of the 'getLastOpList' call's result allows in-progress
// GrOpLists to be picked up and added to by renderTargetContexts lower in the call
// stack. When this occurs with a closed GrOpList, a new one will be allocated
// when the renderTargetContext attempts to use it (via getOpList).
GrSurfaceContext::GrSurfaceContext(GrRecordingContext* context,
                                   GrDrawingManager* drawingMgr,
                                   GrPixelConfig config,
                                   sk_sp<SkColorSpace> colorSpace,
                                   GrAuditTrail* auditTrail,
                                   GrSingleOwner* singleOwner)
        : fContext(context)
        , fAuditTrail(auditTrail)
        , fColorSpaceInfo(std::move(colorSpace), config)
        , fDrawingManager(drawingMgr)
#ifdef SK_DEBUG
        , fSingleOwner(singleOwner)
#endif
{
}

bool GrSurfaceContext::readPixels(const SkImageInfo& dstInfo, void* dstBuffer,
                                  size_t dstRowBytes, int x, int y, uint32_t flags) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrSurfaceContext::readPixels");

    // TODO: this seems to duplicate code in SkImage_Gpu::onReadPixels
    if (kUnpremul_SkAlphaType == dstInfo.alphaType() &&
        !GrPixelConfigIsOpaque(this->asSurfaceProxy()->config())) {
        flags |= GrContextPriv::kUnpremul_PixelOpsFlag;
    }
    auto colorType = SkColorTypeToGrColorType(dstInfo.colorType());
    if (GrColorType::kUnknown == colorType) {
        return false;
    }

    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        return false;
    }

    return direct->priv().readSurfacePixels(this, x, y, dstInfo.width(), dstInfo.height(),
                                            colorType, dstInfo.colorSpace(), dstBuffer,
                                            dstRowBytes, flags);
}

bool GrSurfaceContext::writePixels(const SkImageInfo& srcInfo, const void* srcBuffer,
                                   size_t srcRowBytes, int x, int y, uint32_t flags) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrSurfaceContext::writePixels");

    if (kUnpremul_SkAlphaType == srcInfo.alphaType()) {
        flags |= GrContextPriv::kUnpremul_PixelOpsFlag;
    }
    auto colorType = SkColorTypeToGrColorType(srcInfo.colorType());
    if (GrColorType::kUnknown == colorType) {
        return false;
    }

    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        return false;
    }

    return direct->priv().writeSurfacePixels(this, x, y, srcInfo.width(), srcInfo.height(),
                                             colorType, srcInfo.colorSpace(), srcBuffer,
                                             srcRowBytes, flags);
}

bool GrSurfaceContext::copy(GrSurfaceProxy* src, const SkIRect& srcRect, const SkIPoint& dstPoint) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrSurfaceContext::copy");

    if (!fContext->priv().caps()->canCopySurface(this->asSurfaceProxy(), src, srcRect,
                                                        dstPoint)) {
        return false;
    }

    return this->getOpList()->copySurface(fContext, this->asSurfaceProxy(),
                                          src, srcRect, dstPoint);
}

/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSurfaceContext.h"

#include "GrContextPriv.h"
#include "SkColorSpace_Base.h"
#include "SkGr.h"

#include "../private/GrAuditTrail.h"


// In MDB mode the reffing of the 'getLastOpList' call's result allows in-progress
// GrOpLists to be picked up and added to by renderTargetContexts lower in the call
// stack. When this occurs with a closed GrOpList, a new one will be allocated
// when the renderTargetContext attempts to use it (via getOpList).
GrSurfaceContext::GrSurfaceContext(GrContext* context,
                                   GrDrawingManager* drawingMgr,
                                   sk_sp<SkColorSpace> colorSpace,
                                   GrAuditTrail* auditTrail,
                                   GrSingleOwner* singleOwner)
    : fContext(context)
    , fColorSpace(std::move(colorSpace))
    , fAuditTrail(auditTrail)
    , fDrawingManager(drawingMgr)
#ifdef SK_DEBUG
    , fSingleOwner(singleOwner)
#endif
{
}

bool GrSurfaceContext::readPixels(const SkImageInfo& dstInfo, void* dstBuffer,
                                  size_t dstRowBytes, int x, int y, uint32_t flags) {
    // TODO: teach GrRenderTarget to take ImageInfo directly to specify the src pixels
    GrPixelConfig config = SkImageInfo2GrPixelConfig(dstInfo, *fContext->caps());
    if (kUnknown_GrPixelConfig == config) {
        return false;
    }

    // TODO: this seems to duplicate code in SkImage_Gpu::onReadPixels
    if (kUnpremul_SkAlphaType == dstInfo.alphaType()) {
        flags |= GrContextPriv::kUnpremul_PixelOpsFlag;
    }

    return fContext->contextPriv().readSurfacePixels(this->asSurfaceProxy(),
                                                     this->getColorSpace(), x, y,
                                                     dstInfo.width(), dstInfo.height(), config,
                                                     dstInfo.colorSpace(),
                                                     dstBuffer, dstRowBytes, flags);
}

bool GrSurfaceContext::writePixels(const SkImageInfo& srcInfo, const void* srcBuffer,
                                   size_t srcRowBytes, int x, int y, uint32_t flags) {
    // TODO: teach GrRenderTarget to take ImageInfo directly to specify the src pixels
    GrPixelConfig config = SkImageInfo2GrPixelConfig(srcInfo, *fContext->caps());
    if (kUnknown_GrPixelConfig == config) {
        return false;
    }
    if (kUnpremul_SkAlphaType == srcInfo.alphaType()) {
        flags |= GrContextPriv::kUnpremul_PixelOpsFlag;
    }

    return fContext->contextPriv().writeSurfacePixels(this->asSurfaceProxy(),
                                                      this->getColorSpace(), x, y,
                                                      srcInfo.width(), srcInfo.height(),
                                                      config, srcInfo.colorSpace(),
                                                      srcBuffer, srcRowBytes, flags);
}

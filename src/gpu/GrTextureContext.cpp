/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureContext.h"
#include "GrDrawingManager.h"
#include "GrResourceProvider.h"
#include "GrTextureOpList.h"

#include "../private/GrAuditTrail.h"

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fSingleOwner);)
#define RETURN_FALSE_IF_ABANDONED  if (this->drawingManager()->wasAbandoned()) { return false; }

GrTextureContext::GrTextureContext(GrContext* context,
                                   GrDrawingManager* drawingMgr,
                                   sk_sp<GrTextureProxy> textureProxy,
                                   sk_sp<SkColorSpace> colorSpace,
                                   GrAuditTrail* auditTrail,
                                   GrSingleOwner* singleOwner)
    : GrSurfaceContext(context, drawingMgr, std::move(colorSpace), auditTrail, singleOwner)
    , fTextureProxy(std::move(textureProxy))
    , fOpList(SkSafeRef(fTextureProxy->getLastTextureOpList())) {
    SkDEBUGCODE(this->validate();)
}

#ifdef SK_DEBUG
void GrTextureContext::validate() const {
    SkASSERT(fTextureProxy);
    fTextureProxy->validate(fContext);

    if (fOpList && !fOpList->isClosed()) {
        SkASSERT(fTextureProxy->getLastOpList() == fOpList);
    }
}
#endif

GrTextureContext::~GrTextureContext() {
    ASSERT_SINGLE_OWNER
    SkSafeUnref(fOpList);
}

GrRenderTargetProxy* GrTextureContext::asRenderTargetProxy() {
    // If the proxy can return an RTProxy it should've been wrapped in a RTContext
    SkASSERT(!fTextureProxy->asRenderTargetProxy());
    return nullptr;
}

sk_sp<GrRenderTargetProxy> GrTextureContext::asRenderTargetProxyRef() {
    // If the proxy can return an RTProxy it should've been wrapped in a RTContext
    SkASSERT(!fTextureProxy->asRenderTargetProxy());
    return nullptr;
}

GrTextureOpList* GrTextureContext::getOpList() {
    ASSERT_SINGLE_OWNER
    SkDEBUGCODE(this->validate();)

    if (!fOpList || fOpList->isClosed()) {
        fOpList = this->drawingManager()->newOpList(fTextureProxy.get());
    }

    return fOpList;
}

// TODO: move this (and GrRenderTargetContext::copy) to GrSurfaceContext?
bool GrTextureContext::onCopy(GrSurfaceProxy* srcProxy,
                              const SkIRect& srcRect,
                              const SkIPoint& dstPoint) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrTextureContext::copy");

    // TODO: defer instantiation until flush time
    sk_sp<GrSurface> src(sk_ref_sp(srcProxy->instantiate(fContext->resourceProvider())));
    if (!src) {
        return false;
    }

#ifndef ENABLE_MDB
    // We can't yet fully defer copies to textures, so GrTextureContext::copySurface will
    // execute the copy immediately. Ensure the data is ready.
    src->flushWrites();
#endif

    // TODO: this needs to be fixed up since it ends the deferrable of the GrTexture
    sk_sp<GrTexture> tex(sk_ref_sp(fTextureProxy->instantiate(fContext->resourceProvider())));
    if (!tex) {
        return false;
    }

    GrTextureOpList* opList = this->getOpList();
    bool result = opList->copySurface(tex.get(), src.get(), srcRect, dstPoint);

#ifndef ENABLE_MDB
    GrOpFlushState flushState(fContext->getGpu(), nullptr);
    opList->prepareOps(&flushState);
    opList->executeOps(&flushState);
    opList->reset();
#endif

    return result;
}

// TODO: move this (and GrRenderTargetContext::onReadPixels) to GrSurfaceContext?
bool GrTextureContext::onReadPixels(const SkImageInfo& dstInfo, void* dstBuffer,
                                    size_t dstRowBytes, int x, int y) {
    // TODO: teach GrTexture to take ImageInfo directly to specify the src pixels
    GrPixelConfig config = SkImageInfo2GrPixelConfig(dstInfo, *fContext->caps());
    if (kUnknown_GrPixelConfig == config) {
        return false;
    }

    uint32_t flags = 0;
    if (kUnpremul_SkAlphaType == dstInfo.alphaType()) {
        flags = GrContext::kUnpremul_PixelOpsFlag;
    }

    // Deferral of the VRAM resources must end in this instance anyway
    sk_sp<GrTexture> tex(sk_ref_sp(fTextureProxy->instantiate(fContext->resourceProvider())));
    if (!tex) {
        return false;
    }

    return tex->readPixels(this->getColorSpace(), x, y, dstInfo.width(), dstInfo.height(),
                           config, dstInfo.colorSpace(), dstBuffer, dstRowBytes, flags);
}

// TODO: move this (and GrRenderTargetContext::onReadPixels) to GrSurfaceContext?
bool GrTextureContext::onWritePixels(const SkImageInfo& srcInfo, const void* srcBuffer,
                                     size_t srcRowBytes, int x, int y,
                                     uint32_t flags) {
    // TODO: teach GrTexture to take ImageInfo directly to specify the src pixels
    GrPixelConfig config = SkImageInfo2GrPixelConfig(srcInfo, *fContext->caps());
    if (kUnknown_GrPixelConfig == config) {
        return false;
    }
    if (kUnpremul_SkAlphaType == srcInfo.alphaType()) {
        flags |= GrContext::kUnpremul_PixelOpsFlag;
    }

    // Deferral of the VRAM resources must end in this instance anyway
    sk_sp<GrTexture> tex(sk_ref_sp(fTextureProxy->instantiate(fContext->resourceProvider())));
    if (!tex) {
        return false;
    }

    return tex->writePixels(this->getColorSpace(), x, y, srcInfo.width(), srcInfo.height(),
                            config, srcInfo.colorSpace(), srcBuffer, srcRowBytes, flags);
}

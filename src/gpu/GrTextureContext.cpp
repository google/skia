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
#define RETURN_FALSE_IF_ABANDONED  if (fDrawingManager->wasAbandoned()) { return false; }

GrTextureContext::GrTextureContext(GrContext* context,
                                   GrDrawingManager* drawingMgr,
                                   sk_sp<GrTextureProxy> textureProxy,
                                   GrAuditTrail* auditTrail,
                                   GrSingleOwner* singleOwner)
    : GrSurfaceContext(context, auditTrail, singleOwner)
    , fDrawingManager(drawingMgr)
    , fTextureProxy(std::move(textureProxy))
    , fOpList(SkSafeRef(fTextureProxy->getLastTextureOpList()))
{
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

GrRenderTargetProxy* GrTextureContext::asDeferredRenderTarget() {
    // If the proxy can return an RTProxy it should've been wrapped in a RTContext
    SkASSERT(!fTextureProxy->asRenderTargetProxy());
    return nullptr;
}

GrTextureOpList* GrTextureContext::getOpList() {
    ASSERT_SINGLE_OWNER
    SkDEBUGCODE(this->validate();)

    if (!fOpList || fOpList->isClosed()) {
        fOpList = fDrawingManager->newOpList(fTextureProxy.get());
    }

    return fOpList;
}

bool GrTextureContext::copySurface(GrSurface* src, const SkIRect& srcRect,
                                   const SkIPoint& dstPoint) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fAuditTrail, "GrTextureContext::copySurface");

    // TODO: this needs to be fixed up since it ends the deferrable of the GrTexture
    sk_sp<GrTexture> tex(sk_ref_sp(fTextureProxy->instantiate(fContext->textureProvider())));
    if (!tex) {
        return false;
    }

    GrTextureOpList* opList = this->getOpList();
    bool result = opList->copySurface(tex.get(), src, srcRect, dstPoint);

#ifndef ENABLE_MDB
    GrOpFlushState flushState(fContext->getGpu(), nullptr);
    opList->prepareOps(&flushState);
    opList->executeOps(&flushState);
    opList->reset();
#endif

    return result;
}

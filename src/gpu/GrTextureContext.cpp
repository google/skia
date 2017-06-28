/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureContext.h"

#include "GrContextPriv.h"
#include "GrDrawingManager.h"
#include "GrTextureOpList.h"

#include "../private/GrAuditTrail.h"

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(this->singleOwner());)
#define RETURN_FALSE_IF_ABANDONED  if (this->drawingManager()->wasAbandoned()) { return false; }

GrTextureContext::GrTextureContext(GrContext* context,
                                   GrDrawingManager* drawingMgr,
                                   sk_sp<GrTextureProxy> textureProxy,
                                   sk_sp<SkColorSpace> colorSpace,
                                   GrAuditTrail* auditTrail,
                                   GrSingleOwner* singleOwner)
    : GrSurfaceContext(context, drawingMgr, std::move(colorSpace), auditTrail, singleOwner)
    , fTextureProxy(std::move(textureProxy))
    , fOpList(sk_ref_sp(fTextureProxy->getLastTextureOpList())) {
    SkDEBUGCODE(this->validate();)
}

#ifdef SK_DEBUG
void GrTextureContext::validate() const {
    SkASSERT(fTextureProxy);
    fTextureProxy->validate(fContext);

    if (fOpList && !fOpList->isClosed()) {
        SkASSERT(fTextureProxy->getLastOpList() == fOpList.get());
    }
}
#endif

GrTextureContext::~GrTextureContext() {
    ASSERT_SINGLE_OWNER
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

GrOpList* GrTextureContext::getOpList() {
    ASSERT_SINGLE_OWNER
    SkDEBUGCODE(this->validate();)

    if (!fOpList || fOpList->isClosed()) {
        fOpList = this->drawingManager()->newTextureOpList(fTextureProxy.get());
    }

    return fOpList.get();
}

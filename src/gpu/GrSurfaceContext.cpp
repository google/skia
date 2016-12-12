/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSurfaceContext.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "../private/GrAuditTrail.h"


// In MDB mode the reffing of the 'getLastOpList' call's result allows in-progress
// GrOpLists to be picked up and added to by renderTargetContexts lower in the call
// stack. When this occurs with a closed GrOpList, a new one will be allocated
// when the renderTargetContext attempts to use it (via getOpList).
GrSurfaceContext::GrSurfaceContext(GrContext* context,
                                   GrAuditTrail* auditTrail,
                                   GrSingleOwner* singleOwner)
    : fContext(context)
    , fAuditTrail(auditTrail)
#ifdef SK_DEBUG
    , fSingleOwner(singleOwner)
#endif
{
}

sk_sp<GrSurfaceProxy> GrSurfaceContext::Copy(GrContext* context,
                                             GrSurfaceProxy* src,
                                             const SkIRect& srcRect,
                                             const SkIPoint& dstPoint,
                                             SkBudgeted budgeted) {
    // TODO: add error handling on subset (i.e., the allocated dst can be smaller)
    GrSurfaceDesc dstDesc = src->desc();
    dstDesc.fWidth = srcRect.width();
    dstDesc.fHeight = srcRect.height();

    sk_sp<GrSurfaceContext> dstContext(context->contextPriv().makeDeferredSurfaceContext(dstDesc,
                                                                                         budgeted));
    if (!dstContext) {
        return nullptr;
    }

    if (!dstContext->copy(src, srcRect, dstPoint)) {
        return nullptr;
    }

    return sk_ref_sp(dstContext->asDeferredSurface());
}

sk_sp<GrSurfaceProxy> GrSurfaceContext::TestCopy(GrContext* context, const GrSurfaceDesc& dstDesc,
                                                 GrTexture* srcTexture, SkBudgeted budgeted) {

    sk_sp<GrSurfaceContext> dstContext(context->contextPriv().makeDeferredSurfaceContext(
                                                                                dstDesc,
                                                                                budgeted));
    if (!dstContext) {
        return nullptr;
    }

    sk_sp<GrSurfaceProxy> srcProxy(GrSurfaceProxy::MakeWrapped(sk_ref_sp(srcTexture)));
    if (!srcProxy) {
        return nullptr;
    }

    if (!dstContext->copy(srcProxy.get())) {
        return nullptr;
    }

    return sk_ref_sp(dstContext->asDeferredSurface());
}

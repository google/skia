/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPreFlushResourceProvider.h"

#include "GrDrawingManager.h"
#include "GrSurfaceProxy.h"

sk_sp<GrRenderTargetContext> GrPreFlushResourceProvider::makeRenderTargetContext(
                                                        const GrSurfaceDesc& desc,
                                                        sk_sp<SkColorSpace> colorSpace,
                                                        const SkSurfaceProps* props) {
    GrSurfaceDesc tmpDesc = desc;
    tmpDesc.fFlags |= kRenderTarget_GrSurfaceFlag;

    // Because this is being allocated at the start of a flush we must ensure the proxy
    // will, when instantiated, have no pending IO.
    // TODO: fold the kNoPendingIO_Flag into GrSurfaceFlags?
    sk_sp<GrSurfaceProxy> proxy = GrSurfaceProxy::MakeDeferred(
                                                    fDrawingMgr->getContext()->resourceProvider(),
                                                    tmpDesc,
                                                    SkBackingFit::kExact,
                                                    SkBudgeted::kYes,
                                                    GrResourceProvider::kNoPendingIO_Flag);
    if (!proxy->asRenderTargetProxy()) {
        return nullptr;
    }

    sk_sp<GrRenderTargetOpList> opList(new GrRenderTargetOpList(
                                                    proxy->asRenderTargetProxy(),
                                                    fDrawingMgr->fContext->getGpu(),
                                                    fDrawingMgr->fContext->resourceProvider(),
                                                    fDrawingMgr->fContext->getAuditTrail(),
                                                    fDrawingMgr->fOptionsForOpLists));
    proxy->setLastOpList(opList.get());

    sk_sp<GrRenderTargetContext> renderTargetContext(
        fDrawingMgr->makeRenderTargetContext(std::move(proxy),
                                             std::move(colorSpace),
                                             props));

    if (!renderTargetContext) {
        return nullptr;
    }

    renderTargetContext->discard();

    return renderTargetContext;
}

// TODO: we only need this entry point as long as we have to pre-allocate the atlas.
// Remove it ASAP.
sk_sp<GrRenderTargetContext> GrPreFlushResourceProvider::makeRenderTargetContext(
                                                        sk_sp<GrSurfaceProxy> proxy,
                                                        sk_sp<SkColorSpace> colorSpace,
                                                        const SkSurfaceProps* props) {

    sk_sp<GrRenderTargetOpList> opList(new GrRenderTargetOpList(
                                                    proxy->asRenderTargetProxy(),
                                                    fDrawingMgr->fContext->getGpu(),
                                                    fDrawingMgr->fContext->resourceProvider(),
                                                    fDrawingMgr->fContext->getAuditTrail(),
                                                    fDrawingMgr->fOptionsForOpLists));
    proxy->setLastOpList(opList.get());

    sk_sp<GrRenderTargetContext> renderTargetContext(
        fDrawingMgr->makeRenderTargetContext(std::move(proxy),
                                             std::move(colorSpace),
                                             props));

    if (!renderTargetContext) {
        return nullptr;
    }

    renderTargetContext->discard();

    return renderTargetContext;
}


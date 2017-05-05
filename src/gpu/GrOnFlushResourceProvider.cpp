/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOnFlushResourceProvider.h"

#include "GrDrawingManager.h"
#include "GrSurfaceProxy.h"

sk_sp<GrRenderTargetContext> GrOnFlushResourceProvider::makeRenderTargetContext(
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

    // MDB TODO: This explicit resource creation is required in the pre-MDB world so that the
    // pre-Flush ops are placed in their own opList.
    sk_sp<GrRenderTargetOpList> opList(new GrRenderTargetOpList(
                                                    sk_ref_sp(proxy->asRenderTargetProxy()),
                                                    fDrawingMgr->fContext->getGpu(),
                                                    fDrawingMgr->fContext->getAuditTrail()));
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
sk_sp<GrRenderTargetContext> GrOnFlushResourceProvider::makeRenderTargetContext(
                                                        sk_sp<GrSurfaceProxy> proxy,
                                                        sk_sp<SkColorSpace> colorSpace,
                                                        const SkSurfaceProps* props) {
    // MDB TODO: This explicit resource creation is required in the pre-MDB world so that the
    // pre-Flush ops are placed in their own opList.
    sk_sp<GrRenderTargetOpList> opList(new GrRenderTargetOpList(
                                                    sk_ref_sp(proxy->asRenderTargetProxy()),
                                                    fDrawingMgr->fContext->getGpu(),
                                                    fDrawingMgr->fContext->getAuditTrail()));
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


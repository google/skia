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

    sk_sp<GrRenderTargetContext> renderTargetContext(
        fDrawingMgr->makeRenderTargetContext(std::move(proxy),
                                             std::move(colorSpace),
                                             props, false));

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
    sk_sp<GrRenderTargetContext> renderTargetContext(
        fDrawingMgr->makeRenderTargetContext(std::move(proxy),
                                             std::move(colorSpace),
                                             props, false));

    if (!renderTargetContext) {
        return nullptr;
    }

    renderTargetContext->discard();

    return renderTargetContext;
}

sk_sp<GrBuffer> GrOnFlushResourceProvider::makeBuffer(GrBufferType intendedType, size_t size,
                                                      const void* data) {
    GrResourceProvider* rp = fDrawingMgr->getContext()->resourceProvider();
    return sk_sp<GrBuffer>(rp->createBuffer(size, intendedType, kDynamic_GrAccessPattern,
                                            GrResourceProvider::kNoPendingIO_Flag,
                                            data));
}

sk_sp<GrBuffer> GrOnFlushResourceProvider::findOrMakeStaticBuffer(const GrUniqueKey& key,
                                                                  GrBufferType intendedType,
                                                                  size_t size, const void* data) {
    GrResourceProvider* rp = fDrawingMgr->getContext()->resourceProvider();
    sk_sp<GrBuffer> buffer(rp->findAndRefTByUniqueKey<GrBuffer>(key));
    if (!buffer) {
        buffer.reset(rp->createBuffer(size, intendedType, kStatic_GrAccessPattern, 0, data));
        if (!buffer) {
            return nullptr;
        }
        SkASSERT(buffer->sizeInBytes() == size); // rp shouldn't bin and/or cache static buffers.
        buffer->resourcePriv().setUniqueKey(key);
    }
    return buffer;
}

const GrCaps* GrOnFlushResourceProvider::caps() const {
    return fDrawingMgr->getContext()->caps();
}

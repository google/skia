/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOnFlushResourceProvider.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrDrawingManager.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrSurfaceProxy.h"

sk_sp<GrRenderTargetContext> GrOnFlushResourceProvider::makeRenderTargetContext(
        const GrSurfaceDesc& desc,
        GrSurfaceOrigin origin,
        sk_sp<SkColorSpace> colorSpace,
        const SkSurfaceProps* props) {
    GrSurfaceDesc tmpDesc = desc;
    tmpDesc.fFlags |= kRenderTarget_GrSurfaceFlag;

    auto proxyProvider = fDrawingMgr->getContext()->contextPriv().proxyProvider();
    auto resourceProvider = fDrawingMgr->getContext()->contextPriv().resourceProvider();

    // Because this is being allocated at the start of a flush we must ensure the proxy
    // will, when instantiated, have no pending IO.
    // TODO: fold the kNoPendingIO_Flag into GrSurfaceFlags?
    sk_sp<GrSurfaceProxy> proxy =
            proxyProvider->createProxy(tmpDesc, origin, SkBackingFit::kExact, SkBudgeted::kYes,
                                       GrInternalSurfaceFlags::kNoPendingIO);
    if (!proxy || !proxy->asRenderTargetProxy()) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(
        fDrawingMgr->makeRenderTargetContext(std::move(proxy),
                                             std::move(colorSpace),
                                             props, false));

    if (!renderTargetContext) {
        return nullptr;
    }

    // Since this is at flush time and these won't be allocated for us by the GrResourceAllocator
    // we have to manually ensure it is allocated here. The proxy had best have been created
    // with the kNoPendingIO flag!
    if (!renderTargetContext->asSurfaceProxy()->instantiate(resourceProvider)) {
        return nullptr;
    }

    renderTargetContext->discard();

    return renderTargetContext;
}

sk_sp<GrRenderTargetContext> GrOnFlushResourceProvider::makeRenderTargetContext(
                                                        sk_sp<GrSurfaceProxy> proxy,
                                                        sk_sp<SkColorSpace> colorSpace,
                                                        const SkSurfaceProps* props) {
    // Since this is at flush time and these won't be allocated for us by the GrResourceAllocator
    // we have to manually ensure it is allocated here. The proxy had best have been created
    // with the kNoPendingIO flag!
    if (!this->instatiateProxy(proxy.get())) {
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

bool GrOnFlushResourceProvider::assignUniqueKeyToProxy(const GrUniqueKey& key,
                                                       GrTextureProxy* proxy) {
    auto proxyProvider = fDrawingMgr->getContext()->contextPriv().proxyProvider();
    return proxyProvider->assignUniqueKeyToProxy(key, proxy);
}

void GrOnFlushResourceProvider::removeUniqueKeyFromProxy(const GrUniqueKey& key,
                                                         GrTextureProxy* proxy) {
    auto proxyProvider = fDrawingMgr->getContext()->contextPriv().proxyProvider();
    proxyProvider->removeUniqueKeyFromProxy(key, proxy);
}

sk_sp<GrTextureProxy> GrOnFlushResourceProvider::findOrCreateProxyByUniqueKey(
        const GrUniqueKey& key, GrSurfaceOrigin origin) {
    auto proxyProvider = fDrawingMgr->getContext()->contextPriv().proxyProvider();
    return proxyProvider->findOrCreateProxyByUniqueKey(key, origin);
}

bool GrOnFlushResourceProvider::instatiateProxy(GrSurfaceProxy* proxy) {
    auto resourceProvider = fDrawingMgr->getContext()->contextPriv().resourceProvider();

    if (GrSurfaceProxy::LazyState::kNot != proxy->lazyInstantiationState()) {
        // DDL TODO: Decide if we ever plan to have these proxies use the GrUninstantiateTracker
        // to support unistantiating them at the end of a flush.
        return proxy->priv().doLazyInstantiation(resourceProvider);
    }

    return proxy->instantiate(resourceProvider);
}

sk_sp<GrBuffer> GrOnFlushResourceProvider::makeBuffer(GrBufferType intendedType, size_t size,
                                                      const void* data) {
    auto resourceProvider = fDrawingMgr->getContext()->contextPriv().resourceProvider();
    return sk_sp<GrBuffer>(resourceProvider->createBuffer(size, intendedType,
                                                          kDynamic_GrAccessPattern,
                                                          GrResourceProvider::kNoPendingIO_Flag,
                                                          data));
}

sk_sp<const GrBuffer> GrOnFlushResourceProvider::findOrMakeStaticBuffer(GrBufferType intendedType,
                                                                        size_t size,
                                                                        const void* data,
                                                                        const GrUniqueKey& key) {
    auto resourceProvider = fDrawingMgr->getContext()->contextPriv().resourceProvider();
    sk_sp<const GrBuffer> buffer = resourceProvider->findOrMakeStaticBuffer(intendedType, size,
                                                                            data, key);
    // Static buffers should never have pending IO.
    SkASSERT(!buffer->resourcePriv().hasPendingIO_debugOnly());
    return buffer;
}

uint32_t GrOnFlushResourceProvider::contextUniqueID() const {
    return fDrawingMgr->getContext()->uniqueID();
}

const GrCaps* GrOnFlushResourceProvider::caps() const {
    return fDrawingMgr->getContext()->contextPriv().caps();
}

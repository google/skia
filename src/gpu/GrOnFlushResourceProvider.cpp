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

void GrOnFlushResourceProvider::removeUniqueKeyFromProxy(GrTextureProxy* proxy) {
    auto proxyProvider = fDrawingMgr->getContext()->contextPriv().proxyProvider();
    proxyProvider->removeUniqueKeyFromProxy(proxy);
}

void GrOnFlushResourceProvider::processInvalidUniqueKey(const GrUniqueKey& key) {
    auto proxyProvider = fDrawingMgr->getContext()->contextPriv().proxyProvider();
    proxyProvider->processInvalidUniqueKey(key, nullptr,
                                           GrProxyProvider::InvalidateGPUResource::kYes);
}

sk_sp<GrTextureProxy> GrOnFlushResourceProvider::findOrCreateProxyByUniqueKey(
        const GrUniqueKey& key, GrSurfaceOrigin origin) {
    auto proxyProvider = fDrawingMgr->getContext()->contextPriv().proxyProvider();
    return proxyProvider->findOrCreateProxyByUniqueKey(key, origin);
}

bool GrOnFlushResourceProvider::instatiateProxy(GrSurfaceProxy* proxy) {
    auto resourceProvider = fDrawingMgr->getContext()->contextPriv().resourceProvider();

    if (GrSurfaceProxy::LazyState::kNot != proxy->lazyInstantiationState()) {
        // DDL TODO: Decide if we ever plan to have these proxies use the GrDeinstantiateTracker
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
                                                          GrResourceProvider::Flags::kNone,
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
    SkASSERT(!buffer || !buffer->resourcePriv().hasPendingIO_debugOnly());
    return buffer;
}

uint32_t GrOnFlushResourceProvider::contextID() const {
    return fDrawingMgr->getContext()->contextPriv().contextID();
}

const GrCaps* GrOnFlushResourceProvider::caps() const {
    return fDrawingMgr->getContext()->contextPriv().caps();
}

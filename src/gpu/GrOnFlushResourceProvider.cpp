/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOnFlushResourceProvider.h"

#include "GrContextPriv.h"
#include "GrDrawingManager.h"
#include "GrProxyProvider.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
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
    auto proxyProvider = fDrawingMgr->getContext()->priv().proxyProvider();
    return proxyProvider->assignUniqueKeyToProxy(key, proxy);
}

void GrOnFlushResourceProvider::removeUniqueKeyFromProxy(GrTextureProxy* proxy) {
    auto proxyProvider = fDrawingMgr->getContext()->priv().proxyProvider();
    proxyProvider->removeUniqueKeyFromProxy(proxy);
}

void GrOnFlushResourceProvider::processInvalidUniqueKey(const GrUniqueKey& key) {
    auto proxyProvider = fDrawingMgr->getContext()->priv().proxyProvider();
    proxyProvider->processInvalidUniqueKey(key, nullptr,
                                           GrProxyProvider::InvalidateGPUResource::kYes);
}

sk_sp<GrTextureProxy> GrOnFlushResourceProvider::findOrCreateProxyByUniqueKey(
        const GrUniqueKey& key, GrSurfaceOrigin origin) {
    auto proxyProvider = fDrawingMgr->getContext()->priv().proxyProvider();
    return proxyProvider->findOrCreateProxyByUniqueKey(key, origin);
}

bool GrOnFlushResourceProvider::instatiateProxy(GrSurfaceProxy* proxy) {
    // TODO: this class should probably just get a GrDirectContext
    auto direct = fDrawingMgr->getContext()->priv().asDirectContext();
    if (!direct) {
        return false;
    }

    auto resourceProvider = direct->priv().resourceProvider();

    if (GrSurfaceProxy::LazyState::kNot != proxy->lazyInstantiationState()) {
        // DDL TODO: Decide if we ever plan to have these proxies use the GrDeinstantiateTracker
        // to support unistantiating them at the end of a flush.
        return proxy->priv().doLazyInstantiation(resourceProvider);
    }

    return proxy->instantiate(resourceProvider);
}

sk_sp<GrGpuBuffer> GrOnFlushResourceProvider::makeBuffer(GrGpuBufferType intendedType, size_t size,
                                                         const void* data) {
    // TODO: this class should probably just get a GrDirectContext
    auto direct = fDrawingMgr->getContext()->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    auto resourceProvider = direct->priv().resourceProvider();

    return sk_sp<GrGpuBuffer>(
            resourceProvider->createBuffer(size, intendedType, kDynamic_GrAccessPattern, data));
}

sk_sp<const GrGpuBuffer> GrOnFlushResourceProvider::findOrMakeStaticBuffer(
        GrGpuBufferType intendedType, size_t size, const void* data, const GrUniqueKey& key) {
    // TODO: class should probably just get a GrDirectContext
    auto direct = fDrawingMgr->getContext()->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    auto resourceProvider = direct->priv().resourceProvider();

    sk_sp<const GrGpuBuffer> buffer =
            resourceProvider->findOrMakeStaticBuffer(intendedType, size, data, key);
    // Static buffers should never have pending IO.
    SkASSERT(!buffer || !buffer->resourcePriv().hasPendingIO_debugOnly());
    return buffer;
}

uint32_t GrOnFlushResourceProvider::contextID() const {
    return fDrawingMgr->getContext()->priv().contextID();
}

const GrCaps* GrOnFlushResourceProvider::caps() const {
    return fDrawingMgr->getContext()->priv().caps();
}

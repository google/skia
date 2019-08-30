/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrOnFlushResourceProvider.h"

#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrTextureResolveRenderTask.h"

std::unique_ptr<GrRenderTargetContext> GrOnFlushResourceProvider::makeRenderTargetContext(
        sk_sp<GrSurfaceProxy> proxy, GrColorType colorType, sk_sp<SkColorSpace> colorSpace,
        const SkSurfaceProps* props) {
    // Since this is at flush time and these won't be allocated for us by the GrResourceAllocator
    // we have to manually ensure it is allocated here. The proxy had best have been created
    // with the kNoPendingIO flag!
    if (!this->instatiateProxy(proxy.get())) {
        return nullptr;
    }

    auto renderTargetContext = fDrawingMgr->makeRenderTargetContext(
            std::move(proxy), colorType, std::move(colorSpace), props, false);

    if (!renderTargetContext) {
        return nullptr;
    }

    renderTargetContext->discard();

    // FIXME: http://skbug.com/9357: This breaks if the renderTargetContext splits its opsTask.
    fDrawingMgr->fOnFlushRenderTasks.push_back(sk_ref_sp(renderTargetContext->getOpsTask()));

    return renderTargetContext;
}

void GrOnFlushResourceProvider::addTextureResolveTask(sk_sp<GrTextureProxy> textureProxy,
                                                      GrSurfaceProxy::ResolveFlags resolveFlags) {
    // Since we are bypassing normal DAG operation, we need to ensure the textureProxy's last render
    // task gets closed before making a texture resolve task. makeClosed is what will mark msaa and
    // mipmaps dirty.
    if (GrRenderTask* renderTask = textureProxy->getLastRenderTask()) {
        renderTask->makeClosed(*this->caps());
    }
    auto task = static_cast<GrTextureResolveRenderTask*>(fDrawingMgr->fOnFlushRenderTasks.push_back(
            sk_make_sp<GrTextureResolveRenderTask>(std::move(textureProxy), resolveFlags)).get());
    task->init(*this->caps());
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
        const GrUniqueKey& key,
        GrColorType colorType,
        GrSurfaceOrigin origin,
        UseAllocator useAllocator) {
    auto proxyProvider = fDrawingMgr->getContext()->priv().proxyProvider();
    return proxyProvider->findOrCreateProxyByUniqueKey(key, colorType, origin, useAllocator);
}

bool GrOnFlushResourceProvider::instatiateProxy(GrSurfaceProxy* proxy) {
    SkASSERT(proxy->canSkipResourceAllocator());

    // TODO: this class should probably just get a GrDirectContext
    auto direct = fDrawingMgr->getContext()->priv().asDirectContext();
    if (!direct) {
        return false;
    }

    auto resourceProvider = direct->priv().resourceProvider();

    if (proxy->isLazy()) {
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

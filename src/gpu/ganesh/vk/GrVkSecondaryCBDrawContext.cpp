/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/chromium/GrVkSecondaryCBDrawContext.h"

#include "include/core/SkImageInfo.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/chromium/GrDeferredDisplayList.h"
#include "include/private/chromium/GrSurfaceCharacterization.h"
#include "src/core/SkSurfacePriv.h"
#include "src/gpu/ganesh/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"

sk_sp<GrVkSecondaryCBDrawContext> GrVkSecondaryCBDrawContext::Make(GrRecordingContext* rContext,
                                                                   const SkImageInfo& imageInfo,
                                                                   const GrVkDrawableInfo& vkInfo,
                                                                   const SkSurfaceProps* props) {
    if (!rContext) {
        return nullptr;
    }

    if (rContext->backend() != GrBackendApi::kVulkan) {
        return nullptr;
    }

    sk_sp<GrSurfaceProxy> proxy(
            rContext->priv().proxyProvider()->wrapVulkanSecondaryCBAsRenderTarget(imageInfo,
                                                                                  vkInfo));
    if (!proxy) {
        return nullptr;
    }

    SkASSERT(proxy->isInstantiated());

    auto device = rContext->priv().createDevice(SkColorTypeToGrColorType(imageInfo.colorType()),
                                                std::move(proxy),
                                                imageInfo.refColorSpace(),
                                                kTopLeft_GrSurfaceOrigin,
                                                SkSurfacePropsCopyOrDefault(props),
                                                skgpu::ganesh::Device::InitContents::kUninit);
    if (!device) {
        return nullptr;
    }

    return sk_sp<GrVkSecondaryCBDrawContext>(new GrVkSecondaryCBDrawContext(std::move(device),
                                                                            props));
}

GrVkSecondaryCBDrawContext::GrVkSecondaryCBDrawContext(sk_sp<skgpu::ganesh::Device> device,
                                                       const SkSurfaceProps* props)
        : fDevice(std::move(device)), fProps(SkSurfacePropsCopyOrDefault(props)) {}

GrVkSecondaryCBDrawContext::~GrVkSecondaryCBDrawContext() {
    SkASSERT(!fDevice);
    SkASSERT(!fCachedCanvas.get());
}

SkCanvas* GrVkSecondaryCBDrawContext::getCanvas() {
    if (!fCachedCanvas) {
        fCachedCanvas = std::make_unique<SkCanvas>(fDevice);
    }
    return fCachedCanvas.get();
}

void GrVkSecondaryCBDrawContext::flush() {
    auto dContext = GrAsDirectContext(fDevice->recordingContext());

    if (dContext) {
        dContext->priv().flushSurface(fDevice->targetProxy());
        dContext->submit();
    }
}

bool GrVkSecondaryCBDrawContext::wait(int numSemaphores,
                                      const GrBackendSemaphore waitSemaphores[],
                                      bool deleteSemaphoresAfterWait) {
    return fDevice->wait(numSemaphores, waitSemaphores, deleteSemaphoresAfterWait);
}

void GrVkSecondaryCBDrawContext::releaseResources() {
    fCachedCanvas.reset();
    fDevice.reset();
}

bool GrVkSecondaryCBDrawContext::characterize(GrSurfaceCharacterization* characterization) const {
    auto direct = fDevice->recordingContext()->asDirectContext();
    if (!direct) {
        return false;
    }

    SkImageInfo ii = fDevice->imageInfo();
    if (ii.colorType() == kUnknown_SkColorType) {
        return false;
    }

    GrSurfaceProxyView readSurfaceView = fDevice->readSurfaceView();
    size_t maxResourceBytes = direct->getResourceCacheLimit();

    // We current don't support textured GrVkSecondaryCBDrawContexts.
    SkASSERT(!readSurfaceView.asTextureProxy());

    GrBackendFormat format = readSurfaceView.asRenderTargetProxy()->backendFormat();
    int numSamples = readSurfaceView.asRenderTargetProxy()->numSamples();
    GrProtected isProtected = readSurfaceView.asRenderTargetProxy()->isProtected();

    characterization->set(direct->threadSafeProxy(),
                          maxResourceBytes,
                          ii,
                          format,
                          readSurfaceView.origin(),
                          numSamples,
                          GrSurfaceCharacterization::Textureable(false),
                          GrSurfaceCharacterization::MipMapped(false),
                          GrSurfaceCharacterization::UsesGLFBO0(false),
                          GrSurfaceCharacterization::VkRTSupportsInputAttachment(false),
                          GrSurfaceCharacterization::VulkanSecondaryCBCompatible(true),
                          isProtected,
                          this->props());

    return true;
}

bool GrVkSecondaryCBDrawContext::isCompatible(
        const GrSurfaceCharacterization& characterization) const {

    auto dContext = fDevice->recordingContext()->asDirectContext();
    if (!dContext) {
        return false;
    }

    if (!characterization.isValid()) {
        return false;
    }

    if (!characterization.vulkanSecondaryCBCompatible()) {
        return false;
    }

    if (characterization.isTextureable()) {
        // We don't support textureable DDL when rendering to a GrVkSecondaryCBDrawContext.
        return false;
    }

    if (characterization.usesGLFBO0()) {
        return false;
    }

    SkImageInfo ii = fDevice->imageInfo();
    if (ii.colorType() == kUnknown_SkColorType) {
        return false;
    }

    GrSurfaceProxyView readSurfaceView = fDevice->readSurfaceView();
    // As long as the current state in the context allows for greater or equal resources,
    // we allow the DDL to be replayed.
    // DDL TODO: should we just remove the resource check and ignore the cache limits on playback?
    size_t maxResourceBytes = dContext->getResourceCacheLimit();

    GrBackendFormat format = readSurfaceView.asRenderTargetProxy()->backendFormat();
    int numSamples = readSurfaceView.asRenderTargetProxy()->numSamples();
    GrProtected isProtected = readSurfaceView.asRenderTargetProxy()->isProtected();

    return characterization.contextInfo() &&
           characterization.contextInfo()->priv().matches(dContext) &&
           characterization.cacheMaxResourceBytes() <= maxResourceBytes &&
           characterization.origin() == readSurfaceView.origin() &&
           characterization.backendFormat() == format &&
           characterization.width() == ii.width() &&
           characterization.height() == ii.height() &&
           characterization.colorType() == ii.colorType() &&
           characterization.sampleCount() == numSamples &&
           SkColorSpace::Equals(characterization.colorSpace(), ii.colorInfo().colorSpace()) &&
           characterization.isProtected() == isProtected &&
           characterization.surfaceProps() == fDevice->surfaceProps();
}

#ifndef SK_DDL_IS_UNIQUE_POINTER
bool GrVkSecondaryCBDrawContext::draw(sk_sp<const GrDeferredDisplayList> ddl) {
#else
bool GrVkSecondaryCBDrawContext::draw(const GrDeferredDisplayList* ddl) {
#endif
    if (!ddl || !this->isCompatible(ddl->characterization())) {
        return false;
    }

    auto direct = fDevice->recordingContext()->asDirectContext();
    if (!direct) {
        return false;
    }

    GrSurfaceProxyView readSurfaceView = fDevice->readSurfaceView();

    direct->priv().createDDLTask(std::move(ddl), readSurfaceView.asRenderTargetProxyRef());
    return true;
}

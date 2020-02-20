/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkSecondaryCBDrawContext.h"

#include "include/core/SkDeferredDisplayList.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "src/core/SkSurfacePriv.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/SkGpuDevice.h"

sk_sp<GrVkSecondaryCBDrawContext> GrVkSecondaryCBDrawContext::Make(GrContext* ctx,
                                                                   const SkImageInfo& imageInfo,
                                                                   const GrVkDrawableInfo& vkInfo,
                                                                   const SkSurfaceProps* props) {
    if (!ctx) {
        return nullptr;
    }

    if (ctx->backend() != GrBackendApi::kVulkan) {
        return nullptr;
    }

    auto rtc = GrRenderTargetContext::MakeFromVulkanSecondaryCB(ctx, imageInfo, vkInfo, props);
    SkASSERT(rtc->asSurfaceProxy()->isInstantiated());

    sk_sp<SkGpuDevice> device(
            SkGpuDevice::Make(ctx, std::move(rtc), SkGpuDevice::kUninit_InitContents));
    if (!device) {
        return nullptr;
    }

    return sk_sp<GrVkSecondaryCBDrawContext>(new GrVkSecondaryCBDrawContext(std::move(device),
                                                                            props));
}

GrVkSecondaryCBDrawContext::GrVkSecondaryCBDrawContext(sk_sp<SkGpuDevice> device,
                                                       const SkSurfaceProps* props)
    : fDevice(device)
    , fProps(SkSurfacePropsCopyOrDefault(props)) {}

GrVkSecondaryCBDrawContext::~GrVkSecondaryCBDrawContext() {
    SkASSERT(!fDevice);
    SkASSERT(!fCachedCanvas.get());
}

SkCanvas* GrVkSecondaryCBDrawContext::getCanvas() {
    if (!fCachedCanvas) {
        fCachedCanvas = std::unique_ptr<SkCanvas>(new SkCanvas(fDevice));
    }
    return fCachedCanvas.get();
}

void GrVkSecondaryCBDrawContext::flush() {
    fDevice->flush();
}

bool GrVkSecondaryCBDrawContext::wait(int numSemaphores,
                                      const GrBackendSemaphore waitSemaphores[]) {
    return fDevice->wait(numSemaphores, waitSemaphores);
}

void GrVkSecondaryCBDrawContext::releaseResources() {
    fCachedCanvas.reset();
    fDevice.reset();
}

bool GrVkSecondaryCBDrawContext::characterize(SkSurfaceCharacterization* characterization) const {
    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
    GrContext* ctx = fDevice->context();

    size_t maxResourceBytes = ctx->getResourceCacheLimit();

    // We current don't support textured GrVkSecondaryCBDrawContexts.
    SkASSERT(!rtc->asTextureProxy());

    SkColorType ct = GrColorTypeToSkColorType(rtc->colorInfo().colorType());
    if (ct == kUnknown_SkColorType) {
        return false;
    }

    SkImageInfo ii = SkImageInfo::Make(rtc->width(), rtc->height(), ct, kPremul_SkAlphaType,
                                       rtc->colorInfo().refColorSpace());

    GrBackendFormat format = rtc->asRenderTargetProxy()->backendFormat();

    characterization->set(ctx->threadSafeProxy(), maxResourceBytes, ii, format,
                          rtc->origin(), rtc->numSamples(),
                          SkSurfaceCharacterization::Textureable(false),
                          SkSurfaceCharacterization::MipMapped(false),
                          SkSurfaceCharacterization::UsesGLFBO0(false),
                          SkSurfaceCharacterization::VulkanSecondaryCBCompatible(true),
                          rtc->asRenderTargetProxy()->isProtected(),
                          this->props());

    return true;
}

bool GrVkSecondaryCBDrawContext::isCompatible(
        const SkSurfaceCharacterization& characterization) const {
    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
    GrContext* ctx = fDevice->context();

    if (!characterization.isValid()) {
        return false;
    }

    if (!characterization.vulkanSecondaryCBCompatible()) {
        return false;
    }

    // As long as the current state in the context allows for greater or equal resources,
    // we allow the DDL to be replayed.
    // DDL TODO: should we just remove the resource check and ignore the cache limits on playback?
    size_t maxResourceBytes = ctx->getResourceCacheLimit();

    if (characterization.isTextureable()) {
        // We don't support textureable DDL when rendering to a GrVkSecondaryCBDrawContext.
        return false;
    }

    if (characterization.usesGLFBO0()) {
        return false;
    }

    SkColorType rtColorType = GrColorTypeToSkColorType(rtc->colorInfo().colorType());
    if (rtColorType == kUnknown_SkColorType) {
        return false;
    }

    GrBackendFormat rtcFormat = rtc->asRenderTargetProxy()->backendFormat();
    GrProtected isProtected = rtc->asRenderTargetProxy()->isProtected();

    return characterization.contextInfo() && characterization.contextInfo()->priv().matches(ctx) &&
           characterization.cacheMaxResourceBytes() <= maxResourceBytes &&
           characterization.origin() == rtc->origin() &&
           characterization.backendFormat() == rtcFormat &&
           characterization.width() == rtc->width() &&
           characterization.height() == rtc->height() &&
           characterization.colorType() == rtColorType &&
           characterization.sampleCount() == rtc->numSamples() &&
           SkColorSpace::Equals(characterization.colorSpace(), rtc->colorInfo().colorSpace()) &&
           characterization.isProtected() == isProtected &&
           characterization.surfaceProps() == rtc->surfaceProps();
}

bool GrVkSecondaryCBDrawContext::draw(SkDeferredDisplayList* ddl) {
    if (!ddl || !this->isCompatible(ddl->characterization())) {
        return false;
    }

    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
    GrContext* ctx = fDevice->context();

    ctx->priv().copyRenderTasksFromDDL(ddl, rtc->asRenderTargetProxy());
    return true;
}



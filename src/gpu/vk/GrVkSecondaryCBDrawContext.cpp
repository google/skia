/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vk/GrVkSecondaryCBDrawContext.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrContextThreadSafeProxyPriv.h"
#include "GrRenderTargetContext.h"
#include "SkDeferredDisplayList.h"
#include "SkGpuDevice.h"
#include "SkImageInfo.h"
#include "SkSurfaceCharacterization.h"
#include "SkSurfacePriv.h"
#include "vk/GrVkTypes.h"

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

    sk_sp<GrRenderTargetContext> rtc(
            ctx->priv().makeVulkanSecondaryCBRenderTargetContext(imageInfo, vkInfo, props));

    int width = rtc->width();
    int height = rtc->height();

    sk_sp<SkGpuDevice> device(SkGpuDevice::Make(ctx, std::move(rtc), width, height,
                                                SkGpuDevice::kUninit_InitContents));
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

    int maxResourceCount;
    size_t maxResourceBytes;
    ctx->getResourceCacheLimits(&maxResourceCount, &maxResourceBytes);

    // We current don't support textured GrVkSecondaryCBDrawContexts.
    SkASSERT(!rtc->asTextureProxy());

    // TODO: the addition of colorType to the surfaceContext should remove this calculation
    SkColorType ct;
    if (!GrPixelConfigToColorType(rtc->colorSpaceInfo().config(), &ct)) {
        return false;
    }

    SkImageInfo ii = SkImageInfo::Make(rtc->width(), rtc->height(), ct, kPremul_SkAlphaType,
                                       rtc->colorSpaceInfo().refColorSpace());

    characterization->set(ctx->threadSafeProxy(), maxResourceBytes, ii, rtc->origin(),
                          rtc->colorSpaceInfo().config(), rtc->fsaaType(), rtc->numStencilSamples(),
                          SkSurfaceCharacterization::Textureable(false),
                          SkSurfaceCharacterization::MipMapped(false),
                          SkSurfaceCharacterization::UsesGLFBO0(false),
                          SkSurfaceCharacterization::VulkanSecondaryCBCompatible(true),
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
    int maxResourceCount;
    size_t maxResourceBytes;
    ctx->getResourceCacheLimits(&maxResourceCount, &maxResourceBytes);

    if (characterization.isTextureable()) {
        // We don't support textureable DDL when rendering to a GrVkSecondaryCBDrawContext.
        return false;
    }

    if (characterization.usesGLFBO0()) {
        return false;
    }

    // TODO: the addition of colorType to the surfaceContext should remove this calculation
    SkColorType rtcColorType;
    if (!GrPixelConfigToColorType(rtc->colorSpaceInfo().config(), &rtcColorType)) {
        return false;
    }

    return characterization.contextInfo() && characterization.contextInfo()->priv().matches(ctx) &&
           characterization.cacheMaxResourceBytes() <= maxResourceBytes &&
           characterization.origin() == rtc->origin() &&
           characterization.config() == rtc->colorSpaceInfo().config() &&
           characterization.width() == rtc->width() &&
           characterization.height() == rtc->height() &&
           characterization.colorType() == rtcColorType &&
           characterization.fsaaType() == rtc->fsaaType() &&
           characterization.stencilCount() == rtc->numStencilSamples() &&
           SkColorSpace::Equals(characterization.colorSpace(),
                                rtc->colorSpaceInfo().colorSpace()) &&
           characterization.surfaceProps() == rtc->surfaceProps();
}

bool GrVkSecondaryCBDrawContext::draw(SkDeferredDisplayList* ddl) {
    if (!ddl || !this->isCompatible(ddl->characterization())) {
        return false;
    }

    GrRenderTargetContext* rtc = fDevice->accessRenderTargetContext();
    GrContext* ctx = fDevice->context();

    ctx->priv().copyOpListsFromDDL(ddl, rtc->asRenderTargetProxy());
    return true;
}



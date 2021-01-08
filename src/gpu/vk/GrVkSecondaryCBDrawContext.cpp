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
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "src/core/SkSurfacePriv.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/SkGpuDevice.h"

sk_sp<GrVkSecondaryCBDrawContext> GrVkSecondaryCBDrawContext::Make(GrRecordingContext* ctx,
                                                                   const SkImageInfo& imageInfo,
                                                                   const GrVkDrawableInfo& vkInfo,
                                                                   const SkSurfaceProps* props) {
    if (!ctx) {
        return nullptr;
    }

    if (ctx->backend() != GrBackendApi::kVulkan) {
        return nullptr;
    }

    auto rtc = GrSurfaceDrawContext::MakeFromVulkanSecondaryCB(ctx, imageInfo, vkInfo, props);
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
        fCachedCanvas = std::make_unique<SkCanvas>(fDevice);
    }
    return fCachedCanvas.get();
}

void GrVkSecondaryCBDrawContext::flush() {
    auto dContext = GrAsDirectContext(fDevice->recordingContext());

    if (dContext) {
        dContext->priv().flushSurface(fDevice->surfaceDrawContext()->asSurfaceProxy());
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

bool GrVkSecondaryCBDrawContext::characterize(SkSurfaceCharacterization* characterization) const {
    GrSurfaceDrawContext* sdc = fDevice->surfaceDrawContext();

    auto direct = fDevice->recordingContext()->asDirectContext();
    if (!direct) {
        return false;
    }

    size_t maxResourceBytes = direct->getResourceCacheLimit();

    // We current don't support textured GrVkSecondaryCBDrawContexts.
    SkASSERT(!sdc->asTextureProxy());

    SkColorType ct = GrColorTypeToSkColorType(sdc->colorInfo().colorType());
    if (ct == kUnknown_SkColorType) {
        return false;
    }

    SkImageInfo ii = SkImageInfo::Make(sdc->width(), sdc->height(), ct, kPremul_SkAlphaType,
                                       sdc->colorInfo().refColorSpace());

    GrBackendFormat format = sdc->asRenderTargetProxy()->backendFormat();

    characterization->set(direct->threadSafeProxy(),
                          maxResourceBytes,
                          ii,
                          format,
                          sdc->origin(),
                          sdc->numSamples(),
                          SkSurfaceCharacterization::Textureable(false),
                          SkSurfaceCharacterization::MipMapped(false),
                          SkSurfaceCharacterization::UsesGLFBO0(false),
                          SkSurfaceCharacterization::VkRTSupportsInputAttachment(false),
                          SkSurfaceCharacterization::VulkanSecondaryCBCompatible(true),
                          sdc->asRenderTargetProxy()->isProtected(),
                          this->props());

    return true;
}

bool GrVkSecondaryCBDrawContext::isCompatible(
        const SkSurfaceCharacterization& characterization) const {
    GrSurfaceDrawContext* sdc = fDevice->surfaceDrawContext();

    auto direct = fDevice->recordingContext()->asDirectContext();
    if (!direct) {
        return false;
    }

    if (!characterization.isValid()) {
        return false;
    }

    if (!characterization.vulkanSecondaryCBCompatible()) {
        return false;
    }

    // As long as the current state in the context allows for greater or equal resources,
    // we allow the DDL to be replayed.
    // DDL TODO: should we just remove the resource check and ignore the cache limits on playback?
    size_t maxResourceBytes = direct->getResourceCacheLimit();

    if (characterization.isTextureable()) {
        // We don't support textureable DDL when rendering to a GrVkSecondaryCBDrawContext.
        return false;
    }

    if (characterization.usesGLFBO0()) {
        return false;
    }

    SkColorType rtColorType = GrColorTypeToSkColorType(sdc->colorInfo().colorType());
    if (rtColorType == kUnknown_SkColorType) {
        return false;
    }

    GrBackendFormat rtcFormat = sdc->asRenderTargetProxy()->backendFormat();
    GrProtected isProtected = sdc->asRenderTargetProxy()->isProtected();

    return characterization.contextInfo() &&
           characterization.contextInfo()->priv().matches(direct) &&
           characterization.cacheMaxResourceBytes() <= maxResourceBytes &&
           characterization.origin() == sdc->origin() &&
           characterization.backendFormat() == rtcFormat &&
           characterization.width() == sdc->width() &&
           characterization.height() == sdc->height() &&
           characterization.colorType() == rtColorType &&
           characterization.sampleCount() == sdc->numSamples() &&
           SkColorSpace::Equals(characterization.colorSpace(), sdc->colorInfo().colorSpace()) &&
           characterization.isProtected() == isProtected &&
           characterization.surfaceProps() == sdc->surfaceProps();
}

#ifndef SK_DDL_IS_UNIQUE_POINTER
bool GrVkSecondaryCBDrawContext::draw(sk_sp<const SkDeferredDisplayList> ddl) {
#else
bool GrVkSecondaryCBDrawContext::draw(const SkDeferredDisplayList* ddl) {
#endif
    if (!ddl || !this->isCompatible(ddl->characterization())) {
        return false;
    }

    GrSurfaceDrawContext* sdc = fDevice->surfaceDrawContext();

    auto direct = fDevice->recordingContext()->asDirectContext();
    if (!direct) {
        return false;
    }

    direct->priv().createDDLTask(std::move(ddl), sdc->asRenderTargetProxyRef(), {0, 0});
    return true;
}



/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vk/GrVkSecondaryCBDrawContext.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "SkGpuDevice.h"
#include "SkImageInfo.h"
#include "SkSurfaceProps.h"
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
            ctx->contextPriv().makeVulkanSecondaryCBRenderTargetContext(imageInfo, vkInfo, props));

    int width = rtc->width();
    int height = rtc->height();

    sk_sp<SkGpuDevice> device(SkGpuDevice::Make(ctx, std::move(rtc), width, height,
                                                SkGpuDevice::kUninit_InitContents));
    if (!device) {
        return nullptr;
    }

    return sk_sp<GrVkSecondaryCBDrawContext>(new GrVkSecondaryCBDrawContext(std::move(device)));
}

GrVkSecondaryCBDrawContext::GrVkSecondaryCBDrawContext(sk_sp<SkGpuDevice> device)
    : fDevice(device) {}

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


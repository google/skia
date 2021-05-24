/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrRecordingContextPriv.h"

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/SkGpuDevice.h"
#ifdef SK_NGA
#include "src/gpu/SkGpuDevice_nga.h"
#endif

sk_sp<SkBaseGpuDevice> GrRecordingContextPriv::createDevice(GrColorType colorType,
                                                            sk_sp<GrSurfaceProxy> proxy,
                                                            sk_sp<SkColorSpace> colorSpace,
                                                            GrSurfaceOrigin origin,
                                                            const SkSurfaceProps& props,
                                                            SkBaseGpuDevice::InitContents init) {
    if (this->options().fUseNGA == GrContextOptions::Enable::kYes) {
#ifdef SK_NGA
        return SkGpuDevice_nga::Make1(fContext, colorType, std::move(proxy), std::move(colorSpace),
                                     origin, props, init);
#endif
    } else {
        return SkGpuDevice::Make1(fContext, colorType, std::move(proxy), std::move(colorSpace),
                                 origin, props, init);
    }
    return nullptr;
}

sk_sp<SkBaseGpuDevice> GrRecordingContextPriv::createDevice(SkBudgeted budgeted,
                                                            const SkImageInfo& ii,
                                                            SkBackingFit fit,
                                                            int sampleCount,
                                                            GrMipmapped mipmapped,
                                                            GrProtected isProtected,
                                                            GrSurfaceOrigin origin,
                                                            const SkSurfaceProps& props,
                                                            SkBaseGpuDevice::InitContents init) {
    if (this->options().fUseNGA == GrContextOptions::Enable::kYes) {
#ifdef SK_NGA
        return SkGpuDevice_nga::Make2(fContext, budgeted, ii, SkBackingFit::kExact, sampleCount,
                                     mipmapped, GrProtected::kNo, origin, props, init);
#endif
    } else {
        return SkGpuDevice::Make2(fContext, budgeted, ii, SkBackingFit::kExact, sampleCount,
                                 mipmapped, GrProtected::kNo, origin, props, init);
    }
    return nullptr;
}

void GrRecordingContextPriv::moveRenderTasksToDDL(SkDeferredDisplayList* ddl) {
    fContext->drawingManager()->moveRenderTasksToDDL(ddl);
}

GrSDFTControl GrRecordingContextPriv::getSDFTControl(bool useSDFTForSmallText) const {
    return GrSDFTControl{
            this->caps()->shaderCaps()->supportsDistanceFieldText(),
            useSDFTForSmallText,
            this->options().fMinDistanceFieldFontSize,
            this->options().fGlyphsAsPathsFontSize};
}

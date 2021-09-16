/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/v2/SurfaceDrawContext_v2.h"

#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"

namespace skgpu::v2 {

std::unique_ptr<SurfaceDrawContext> SurfaceDrawContext::Make(GrRecordingContext* rContext,
                                                             GrColorType colorType,
                                                             sk_sp<GrSurfaceProxy> proxy,
                                                             sk_sp<SkColorSpace> colorSpace,
                                                             GrSurfaceOrigin origin,
                                                             const SkSurfaceProps& surfaceProps) {
    if (!rContext || !proxy || colorType == GrColorType::kUnknown) {
        return nullptr;
    }

    const GrBackendFormat& format = proxy->backendFormat();
    GrSwizzle readSwizzle = rContext->priv().caps()->getReadSwizzle(format, colorType);
    GrSwizzle writeSwizzle = rContext->priv().caps()->getWriteSwizzle(format, colorType);

    GrSurfaceProxyView readView (          proxy,  origin, readSwizzle);
    GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);

    return std::make_unique<SurfaceDrawContext>(rContext,
                                                std::move(readView),
                                                std::move(writeView),
                                                colorType,
                                                std::move(colorSpace),
                                                surfaceProps);
}

std::unique_ptr<SurfaceDrawContext> SurfaceDrawContext::Make(GrRecordingContext* rContext,
                                                             GrColorType colorType,
                                                             sk_sp<SkColorSpace> colorSpace,
                                                             SkBackingFit fit,
                                                             SkISize dimensions,
                                                             const SkSurfaceProps& surfaceProps,
                                                             int sampleCnt,
                                                             GrMipmapped mipMapped,
                                                             GrProtected isProtected,
                                                             GrSurfaceOrigin origin,
                                                             SkBudgeted budgeted) {
    if (!rContext) {
        return nullptr;
    }

    auto format = rContext->priv().caps()->getDefaultBackendFormat(colorType, GrRenderable::kYes);
    if (!format.isValid()) {
        return nullptr;
    }
    sk_sp<GrTextureProxy> proxy = rContext->priv().proxyProvider()->createProxy(format,
                                                                                dimensions,
                                                                                GrRenderable::kYes,
                                                                                sampleCnt,
                                                                                mipMapped,
                                                                                fit,
                                                                                budgeted,
                                                                                isProtected);
    if (!proxy) {
        return nullptr;
    }

    return SurfaceDrawContext::Make(rContext,
                                    colorType,
                                    std::move(proxy),
                                    std::move(colorSpace),
                                    origin,
                                    surfaceProps);
}

SurfaceDrawContext::SurfaceDrawContext(GrRecordingContext* rContext,
                                       GrSurfaceProxyView readView,
                                       GrSurfaceProxyView writeView,
                                       GrColorType colorType,
                                       sk_sp<SkColorSpace> colorSpace,
                                       const SkSurfaceProps& surfaceProps)
        : SurfaceFillContext(rContext,
                             std::move(readView),
                             std::move(writeView),
                             {colorType, kPremul_SkAlphaType, std::move(colorSpace)})
        , fSurfaceProps(surfaceProps) {
}

SurfaceDrawContext::~SurfaceDrawContext() {}

} // namespace skgpu::v2

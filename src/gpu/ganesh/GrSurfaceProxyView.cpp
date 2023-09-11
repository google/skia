/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrSurfaceProxyView.h"

#include "include/core/SkRect.h"
#include "include/gpu/GpuTypes.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrTextureProxy.h"

bool GrSurfaceProxyView::operator==(const GrSurfaceProxyView& view) const {
    return fProxy->uniqueID() == view.fProxy->uniqueID() && fOrigin == view.fOrigin &&
           fSwizzle == view.fSwizzle;
}

skgpu::Mipmapped GrSurfaceProxyView::mipmapped() const {
    if (const GrTextureProxy* proxy = this->asTextureProxy()) {
        return proxy->mipmapped();
    }
    return skgpu::Mipmapped::kNo;
}

GrTextureProxy* GrSurfaceProxyView::asTextureProxy() const {
    if (!fProxy) {
        return nullptr;
    }
    return fProxy->asTextureProxy();
}

sk_sp<GrTextureProxy> GrSurfaceProxyView::asTextureProxyRef() const {
    return sk_ref_sp<GrTextureProxy>(this->asTextureProxy());
}

GrRenderTargetProxy* GrSurfaceProxyView::asRenderTargetProxy() const {
    if (!fProxy) {
        return nullptr;
    }
    return fProxy->asRenderTargetProxy();
}

sk_sp<GrRenderTargetProxy> GrSurfaceProxyView::asRenderTargetProxyRef() const {
    return sk_ref_sp<GrRenderTargetProxy>(this->asRenderTargetProxy());
}

void GrSurfaceProxyView::concatSwizzle(skgpu::Swizzle swizzle) {
    fSwizzle = skgpu::Swizzle::Concat(fSwizzle, swizzle);
}

GrSurfaceProxyView GrSurfaceProxyView::makeSwizzle(skgpu::Swizzle swizzle) const& {
    return {fProxy, fOrigin, skgpu::Swizzle::Concat(fSwizzle, swizzle)};
}

GrSurfaceProxyView GrSurfaceProxyView::makeSwizzle(skgpu::Swizzle swizzle) && {
    return {std::move(fProxy), fOrigin, skgpu::Swizzle::Concat(fSwizzle, swizzle)};
}

void GrSurfaceProxyView::reset() { *this = {}; }

GrSurfaceProxyView GrSurfaceProxyView::Copy(GrRecordingContext* context,
                                            GrSurfaceProxyView src,
                                            skgpu::Mipmapped mipmapped,
                                            SkIRect srcRect,
                                            SkBackingFit fit,
                                            skgpu::Budgeted budgeted,
                                            std::string_view label) {
    auto copy = GrSurfaceProxy::Copy(
            context, src.refProxy(), src.origin(), mipmapped, srcRect, fit, budgeted, label);
    return {std::move(copy), src.origin(), src.swizzle()};
}

GrSurfaceProxyView GrSurfaceProxyView::Copy(GrRecordingContext* rContext,
                                            GrSurfaceProxyView src,
                                            skgpu::Mipmapped mipmapped,
                                            SkBackingFit fit,
                                            skgpu::Budgeted budgeted,
                                            std::string_view label) {
    auto copy = GrSurfaceProxy::Copy(
            rContext, src.refProxy(), src.origin(), mipmapped, fit, budgeted, label);
    return {std::move(copy), src.origin(), src.swizzle()};
}

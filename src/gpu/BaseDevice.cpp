/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/BaseDevice.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceProxyView.h"

#define ASSERT_SINGLE_OWNER GR_ASSERT_SINGLE_OWNER(fContext->priv().singleOwner())

namespace skgpu {

BaseDevice::BaseDevice(sk_sp<GrRecordingContext> rContext,
                       const SkImageInfo& ii,
                       const SkSurfaceProps& props)
    : INHERITED(ii, props)
    , fContext(std::move(rContext)) {
}

GrRenderTargetProxy* BaseDevice::targetProxy() {
    return this->readSurfaceView().asRenderTargetProxy();
}

bool BaseDevice::replaceBackingProxy(SkSurface::ContentChangeMode mode) {
    ASSERT_SINGLE_OWNER

    const SkImageInfo& ii = this->imageInfo();
    GrRenderTargetProxy* oldRTP = this->targetProxy();
    GrSurfaceProxyView oldView = this->readSurfaceView();

    auto grColorType = SkColorTypeToGrColorType(ii.colorType());
    auto format = fContext->priv().caps()->getDefaultBackendFormat(grColorType, GrRenderable::kYes);
    if (!format.isValid()) {
        return false;
    }

    GrProxyProvider* proxyProvider = fContext->priv().proxyProvider();
    // This entry point is used by SkSurface_Gpu::onCopyOnWrite so it must create a
    // kExact-backed render target proxy
    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxy(format,
                                                             ii.dimensions(),
                                                             GrRenderable::kYes,
                                                             oldRTP->numSamples(),
                                                             oldView.mipmapped(),
                                                             SkBackingFit::kExact,
                                                             oldRTP->isBudgeted(),
                                                             GrProtected::kNo);
    if (!proxy) {
        return false;
    }

    return this->replaceBackingProxy(mode, sk_ref_sp(proxy->asRenderTargetProxy()),
                                     grColorType, ii.refColorSpace(), oldView.origin(),
                                     this->surfaceProps());
}

} // namespace skgpu

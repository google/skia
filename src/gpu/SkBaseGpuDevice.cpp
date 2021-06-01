/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/SkBaseGpuDevice.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"

#define ASSERT_SINGLE_OWNER GR_ASSERT_SINGLE_OWNER(fContext->priv().singleOwner())

bool SkBaseGpuDevice::replaceBackingProxy(SkSurface::ContentChangeMode mode) {
    ASSERT_SINGLE_OWNER
    auto rContext = this->recordingContext();
    if (!rContext) {
        return false;
    }

    const SkImageInfo& ii = this->imageInfo();
    GrRenderTargetProxy* oldRTP = this->targetProxy();
    GrSurfaceProxyView oldView = this->readSurfaceView();

    auto grColorType = SkColorTypeToGrColorType(ii.colorType());
    auto format = rContext->priv().caps()->getDefaultBackendFormat(grColorType, GrRenderable::kYes);
    if (!format.isValid()) {
        return false;
    }

    GrProxyProvider* proxyProvier = rContext->priv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvier->createProxy(format,
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

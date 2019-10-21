/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrResourceProviderPriv.h"
#include "src/image/SkSurface_Gpu.h"

#if SK_SUPPORT_GPU

#include "include/gpu/GrSurface.h"
#include "src/gpu/mtl/GrMtlRenderTarget.h"

#ifdef SK_METAL
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

sk_sp<SkSurface> SkSurface::MakeFromCAMetalLayer(GrContext* context,
                                                 GrMTLHandle layer,
                                                 GrSurfaceOrigin origin,
                                                 int sampleCnt,
                                                 SkColorType colorType,
                                                 sk_sp<SkColorSpace> colorSpace,
                                                 const SkSurfaceProps* surfaceProps,
                                                 GrMTLHandle* drawable) {
    const GrCaps* caps = context->priv().caps();

    CAMetalLayer* metalLayer = (__bridge CAMetalLayer*)layer;
    GrBackendFormat backendFormat = GrBackendFormat::MakeMtl(metalLayer.pixelFormat);

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);

    GrPixelConfig config = caps->getConfigFromBackendFormat(backendFormat, grColorType);
    if (config == kUnknown_GrPixelConfig) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    CGSize size = [metalLayer drawableSize];
    desc.fWidth = size.width * [metalLayer contentsScale];
    desc.fHeight = size.height * [metalLayer contentsScale];
    desc.fConfig = config;

    sk_sp<SkSurface> surface = SkSurface::MakeFromLazyRenderTargetProxy(
            context,
            [layer, drawable, desc, sampleCnt](GrResourceProvider* resourceProvider) {
                CAMetalLayer* metalLayer = (__bridge CAMetalLayer*)layer;
                id<CAMetalDrawable> currentDrawable = [metalLayer nextDrawable];

                GrMtlGpu* mtlGpu = (GrMtlGpu*) resourceProvider->priv().gpu();
                auto surface = GrMtlRenderTarget::MakeWrappedRenderTarget(mtlGpu, desc, sampleCnt,
                                                                          currentDrawable.texture);

                *drawable = (__bridge_retained GrMTLHandle) currentDrawable;
                return surface;
            },
            desc.fWidth, desc.fHeight,
            backendFormat,
            origin,
            sampleCnt,
            colorType,
            colorSpace,
            surfaceProps);

    return surface;
}
#endif

#endif

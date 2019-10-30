/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/mtl/GrMtlTypes.h"

#if SK_SUPPORT_GPU

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
    // TODO: Apple recommends grabbing the drawable (which we're implicitly doing here)
    // for as little time as possible. I'm not sure it matters for our test apps, but
    // you can get better throughput by doing any offscreen renders, texture uploads, or
    // other non-dependant tasks first before grabbing the drawable.
    CAMetalLayer* metalLayer = (__bridge CAMetalLayer*)layer;
    id<CAMetalDrawable> currentDrawable = [metalLayer nextDrawable];

    GrMtlTextureInfo fbInfo;
    fbInfo.fTexture.retain((__bridge const void*)(currentDrawable.texture));

    CGSize size = [metalLayer drawableSize];
    sk_sp<SkSurface> surface;
    if (sampleCnt <= 1) {
        GrBackendRenderTarget backendRT(size.width,
                                        size.height,
                                        sampleCnt,
                                        fbInfo);

        surface = SkSurface::MakeFromBackendRenderTarget(context, backendRT, origin, colorType,
                                                         colorSpace, surfaceProps);
    } else {
        GrBackendTexture backendTexture(size.width,
                                        size.height,
                                        GrMipMapped::kNo,
                                        fbInfo);

        surface = SkSurface::MakeFromBackendTexture(context, backendTexture, origin, sampleCnt,
                                                    colorType, colorSpace, surfaceProps);
    }
    *drawable = (__bridge_retained GrMTLHandle) currentDrawable;

    return surface;
}
#endif

#endif

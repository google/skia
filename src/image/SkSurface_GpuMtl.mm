/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrResourceProviderPriv.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#include "src/image/SkSurface_Gpu.h"

#if SK_SUPPORT_GPU

#include "src/gpu/GrSurface.h"
#include "src/gpu/mtl/GrMtlTextureRenderTarget.h"

#ifdef SK_METAL
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <MetalKit/MetalKit.h>

sk_sp<SkSurface> SkSurface::MakeFromCAMetalLayer(GrRecordingContext* rContext,
                                                 GrMTLHandle layer,
                                                 GrSurfaceOrigin origin,
                                                 int sampleCnt,
                                                 SkColorType colorType,
                                                 sk_sp<SkColorSpace> colorSpace,
                                                 const SkSurfaceProps* surfaceProps,
                                                 GrMTLHandle* drawable) {
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    CAMetalLayer* metalLayer = (__bridge CAMetalLayer*)layer;
    GrBackendFormat backendFormat = GrBackendFormat::MakeMtl(metalLayer.pixelFormat);

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);

    SkISize dims = {(int)metalLayer.drawableSize.width, (int)metalLayer.drawableSize.height};

    GrProxyProvider::TextureInfo texInfo;
    texInfo.fMipmapped = GrMipmapped::kNo;
    texInfo.fTextureType = GrTextureType::k2D;

    sk_sp<GrRenderTargetProxy> proxy = proxyProvider->createLazyRenderTargetProxy(
            [layer, drawable](GrResourceProvider* resourceProvider,
                              const GrSurfaceProxy::LazySurfaceDesc& desc) {
                CAMetalLayer* metalLayer = (__bridge CAMetalLayer*)layer;
                id<CAMetalDrawable> currentDrawable = [metalLayer nextDrawable];

                GrMtlGpu* mtlGpu = (GrMtlGpu*) resourceProvider->priv().gpu();
                sk_sp<GrRenderTarget> surface;
                if (metalLayer.framebufferOnly) {
                    surface = GrMtlRenderTarget::MakeWrappedRenderTarget(
                            mtlGpu, desc.fDimensions, desc.fSampleCnt, currentDrawable.texture);
                } else {
                    surface = GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(
                            mtlGpu, desc.fDimensions, desc.fSampleCnt, currentDrawable.texture,
                            GrWrapCacheable::kNo);
                }
                if (surface && desc.fSampleCnt > 1) {
                    surface->setRequiresManualMSAAResolve();
                }

                *drawable = (__bridge_retained GrMTLHandle) currentDrawable;
                return GrSurfaceProxy::LazyCallbackResult(std::move(surface));
            },
            backendFormat,
            dims,
            sampleCnt,
            sampleCnt > 1 ? GrInternalSurfaceFlags::kRequiresManualMSAAResolve
                          : GrInternalSurfaceFlags::kNone,
            metalLayer.framebufferOnly ? nullptr : &texInfo,
            GrMipmapStatus::kNotAllocated,
            SkBackingFit::kExact,
            SkBudgeted::kYes,
            GrProtected::kNo,
            false,
            GrSurfaceProxy::UseAllocator::kYes);

    auto device = rContext->priv().createDevice(grColorType,
                                                std::move(proxy),
                                                std::move(colorSpace),
                                                origin,
                                                SkSurfacePropsCopyOrDefault(surfaceProps),
                                                skgpu::BaseDevice::InitContents::kUninit);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

sk_sp<SkSurface> SkSurface::MakeFromMTKView(GrRecordingContext* rContext,
                                            GrMTLHandle view,
                                            GrSurfaceOrigin origin,
                                            int sampleCnt,
                                            SkColorType colorType,
                                            sk_sp<SkColorSpace> colorSpace,
                                            const SkSurfaceProps* surfaceProps) {
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    MTKView* mtkView = (__bridge MTKView*)view;
    GrBackendFormat backendFormat = GrBackendFormat::MakeMtl(mtkView.colorPixelFormat);

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);

    SkISize dims = {(int)mtkView.drawableSize.width, (int)mtkView.drawableSize.height};

    GrProxyProvider::TextureInfo texInfo;
    texInfo.fMipmapped = GrMipmapped::kNo;
    texInfo.fTextureType = GrTextureType::k2D;

    sk_sp<GrRenderTargetProxy> proxy = proxyProvider->createLazyRenderTargetProxy(
            [view](GrResourceProvider* resourceProvider,
                   const GrSurfaceProxy::LazySurfaceDesc& desc) {
                MTKView* mtkView = (__bridge MTKView*)view;
                id<CAMetalDrawable> currentDrawable = [mtkView currentDrawable];

                GrMtlGpu* mtlGpu = (GrMtlGpu*) resourceProvider->priv().gpu();
                sk_sp<GrRenderTarget> surface;
                if (mtkView.framebufferOnly) {
                    surface = GrMtlRenderTarget::MakeWrappedRenderTarget(
                            mtlGpu, desc.fDimensions, desc.fSampleCnt, currentDrawable.texture);
                } else {
                    surface = GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(
                            mtlGpu, desc.fDimensions, desc.fSampleCnt, currentDrawable.texture,
                            GrWrapCacheable::kNo);
                }
                if (surface && desc.fSampleCnt > 1) {
                    surface->setRequiresManualMSAAResolve();
                }

                return GrSurfaceProxy::LazyCallbackResult(std::move(surface));
            },
            backendFormat,
            dims,
            sampleCnt,
            sampleCnt > 1 ? GrInternalSurfaceFlags::kRequiresManualMSAAResolve
                          : GrInternalSurfaceFlags::kNone,
            mtkView.framebufferOnly ? nullptr : &texInfo,
            GrMipmapStatus::kNotAllocated,
            SkBackingFit::kExact,
            SkBudgeted::kYes,
            GrProtected::kNo,
            false,
            GrSurfaceProxy::UseAllocator::kYes);


    auto device = rContext->priv().createDevice(grColorType,
                                                std::move(proxy),
                                                std::move(colorSpace),
                                                origin,
                                                SkSurfacePropsCopyOrDefault(surfaceProps),
                                                skgpu::BaseDevice::InitContents::kUninit);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<SkSurface_Gpu>(std::move(device));
}

#endif

#endif

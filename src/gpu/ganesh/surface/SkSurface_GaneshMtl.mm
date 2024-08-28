/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/mtl/GrMtlBackendSurface.h"
#include "include/gpu/ganesh/mtl/GrMtlTypes.h"
#include "include/gpu/ganesh/mtl/SkSurfaceMetal.h"
#include "src/core/SkSurfacePriv.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrResourceProviderPriv.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/mtl/GrMtlTextureRenderTarget.h"
#include "src/gpu/ganesh/surface/SkSurface_Ganesh.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>

namespace SkSurfaces {

sk_sp<SkSurface> WrapCAMetalLayer(GrRecordingContext* rContext,
                                  GrMTLHandle layer,
                                  GrSurfaceOrigin origin,
                                  int sampleCnt,
                                  SkColorType colorType,
                                  sk_sp<SkColorSpace> colorSpace,
                                  const SkSurfaceProps* surfaceProps,
                                  GrMTLHandle* drawable) {
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    CAMetalLayer* metalLayer = (__bridge CAMetalLayer*)layer;
    GrBackendFormat backendFormat = GrBackendFormats::MakeMtl(metalLayer.pixelFormat);

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);

    SkISize dims = {(int)metalLayer.drawableSize.width, (int)metalLayer.drawableSize.height};

    GrProxyProvider::TextureInfo texInfo;
    texInfo.fMipmapped = skgpu::Mipmapped::kNo;
    texInfo.fTextureType = GrTextureType::k2D;

    sk_sp<GrRenderTargetProxy> proxy = proxyProvider->createLazyRenderTargetProxy(
            [layer, drawable](GrResourceProvider* resourceProvider,
                              const GrSurfaceProxy::LazySurfaceDesc& desc) {
                CAMetalLayer* metalLayer = (__bridge CAMetalLayer*)layer;
                id<CAMetalDrawable> currentDrawable = [metalLayer nextDrawable];

                GrMtlGpu* mtlGpu = (GrMtlGpu*)resourceProvider->priv().gpu();
                sk_sp<GrRenderTarget> surface;
                if (metalLayer.framebufferOnly) {
                    surface = GrMtlRenderTarget::MakeWrappedRenderTarget(
                            mtlGpu, desc.fDimensions, desc.fSampleCnt, currentDrawable.texture);
                } else {
                    surface = GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(
                            mtlGpu,
                            desc.fDimensions,
                            desc.fSampleCnt,
                            currentDrawable.texture,
                            GrWrapCacheable::kNo);
                }
                if (surface && desc.fSampleCnt > 1) {
                    surface->setRequiresManualMSAAResolve();
                }

                *drawable = (__bridge_retained GrMTLHandle)currentDrawable;
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
            skgpu::Budgeted::kYes,
            GrProtected::kNo,
            false,
            GrSurfaceProxy::UseAllocator::kYes);

    auto device = rContext->priv().createDevice(grColorType,
                                                std::move(proxy),
                                                std::move(colorSpace),
                                                origin,
                                                SkSurfacePropsCopyOrDefault(surfaceProps),
                                                skgpu::ganesh::Device::InitContents::kUninit);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<SkSurface_Ganesh>(std::move(device));
}

sk_sp<SkSurface> WrapMTKView(GrRecordingContext* rContext,
                             GrMTLHandle view,
                             GrSurfaceOrigin origin,
                             int sampleCnt,
                             SkColorType colorType,
                             sk_sp<SkColorSpace> colorSpace,
                             const SkSurfaceProps* surfaceProps) {
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    MTKView* mtkView = (__bridge MTKView*)view;
    GrBackendFormat backendFormat = GrBackendFormats::MakeMtl(mtkView.colorPixelFormat);

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);

    SkISize dims = {(int)mtkView.drawableSize.width, (int)mtkView.drawableSize.height};

    GrProxyProvider::TextureInfo texInfo;
    texInfo.fMipmapped = skgpu::Mipmapped::kNo;
    texInfo.fTextureType = GrTextureType::k2D;

    sk_sp<GrRenderTargetProxy> proxy = proxyProvider->createLazyRenderTargetProxy(
            [view](GrResourceProvider* resourceProvider,
                   const GrSurfaceProxy::LazySurfaceDesc& desc) {
                MTKView* mtkView = (__bridge MTKView*)view;
                id<CAMetalDrawable> currentDrawable = [mtkView currentDrawable];

                GrMtlGpu* mtlGpu = (GrMtlGpu*)resourceProvider->priv().gpu();
                sk_sp<GrRenderTarget> surface;
                if (mtkView.framebufferOnly) {
                    surface = GrMtlRenderTarget::MakeWrappedRenderTarget(
                            mtlGpu, desc.fDimensions, desc.fSampleCnt, currentDrawable.texture);
                } else {
                    surface = GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(
                            mtlGpu,
                            desc.fDimensions,
                            desc.fSampleCnt,
                            currentDrawable.texture,
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
            skgpu::Budgeted::kYes,
            GrProtected::kNo,
            false,
            GrSurfaceProxy::UseAllocator::kYes);

    auto device = rContext->priv().createDevice(grColorType,
                                                std::move(proxy),
                                                std::move(colorSpace),
                                                origin,
                                                SkSurfacePropsCopyOrDefault(surfaceProps),
                                                skgpu::ganesh::Device::InitContents::kUninit);
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<SkSurface_Ganesh>(std::move(device));
}

}  // namespace SkSurfaces

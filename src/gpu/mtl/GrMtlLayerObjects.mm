/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlLayerObjects.h"

#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

// Called for wrapped non-texture render targets.
GrMtlLayerRenderTarget::GrMtlLayerRenderTarget(GrMtlGpu* gpu,
                                               const GrSurfaceDesc& desc,
                                               int sampleCnt,
                                               id<MTLTexture> colorTexture,
                                               CAMetalLayer* layer,
                                               GrMTLHandle* drawable)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo)
        , GrMtlRenderTarget(gpu, desc, sampleCnt, colorTexture, nil)
        , fLayer(layer)
        , fDrawable(drawable)
        , fSampleCount(sampleCnt) {
    SkASSERT(sampleCnt > 1);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

GrMtlLayerRenderTarget::GrMtlLayerRenderTarget(GrMtlGpu* gpu,
                                               const GrSurfaceDesc& desc,
                                               CAMetalLayer* layer,
                                               GrMTLHandle* drawable)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo)
        , GrMtlRenderTarget(gpu, desc, nil)
        , fLayer(layer)
        , fDrawable(drawable)
        , fSampleCount(1) {
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}


sk_sp<GrMtlLayerRenderTarget> GrMtlLayerRenderTarget::MakeWrappedRenderTarget(
        GrMtlGpu* gpu, const GrSurfaceDesc& desc, int sampleCnt, CAMetalLayer* layer,
        GrMTLHandle* drawable) {
//    SkASSERT(nil != texture);
//    SkASSERT(1 == texture.mipmapLevelCount);
//    SkASSERT(MTLTextureUsageRenderTarget & texture.usage);

    GrMtlLayerRenderTarget* mtlRT;
    if (sampleCnt > 1) {
        MTLPixelFormat format;
        if (!GrPixelConfigToMTLFormat(desc.fConfig, &format)) {
            return nullptr;
        }
        MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
        texDesc.textureType = MTLTextureType2DMultisample;
        texDesc.pixelFormat = format;
        texDesc.width = desc.fWidth;
        texDesc.height = desc.fHeight;
        texDesc.depth = 1;
        texDesc.mipmapLevelCount = 1;
        texDesc.sampleCount = sampleCnt;
        texDesc.arrayLength = 1;
        texDesc.storageMode = MTLStorageModePrivate;
        texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;

        id<MTLTexture> colorTexture = [gpu->device() newTextureWithDescriptor:texDesc];
        if (!colorTexture) {
            return nullptr;
        }
        SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & colorTexture.usage);
        mtlRT = new GrMtlLayerRenderTarget(gpu, desc, sampleCnt, colorTexture, layer, drawable);
    } else {
        mtlRT = new GrMtlLayerRenderTarget(gpu, desc, layer, drawable);
    }

    return sk_sp<GrMtlLayerRenderTarget>(mtlRT);
}

GrBackendRenderTarget GrMtlLayerRenderTarget::getBackendRenderTarget() const {
    GrMtlLayerInfo info;
    info.fLayer.reset(GrRetainPtrFromId(fLayer));
    return GrBackendRenderTarget(this->width(), this->height(), fSampleCount, info);
}

GrBackendFormat GrMtlLayerRenderTarget::backendFormat() const {
    return GrBackendFormat::MakeMtl(fLayer.pixelFormat);
}

id<MTLTexture> GrMtlLayerRenderTarget::mtlColorTexture() const {
    if (!fColorTexture) {
        id<CAMetalDrawable> currentDrawable = [fLayer nextDrawable];
        fColorTexture = currentDrawable.texture;
        *fDrawable = (__bridge_retained GrMTLHandle) currentDrawable;
    }
    return fColorTexture;
}

id<MTLTexture> GrMtlLayerRenderTarget::mtlResolveTexture() const {
    if (!fResolveTexture) {
        id<CAMetalDrawable> currentDrawable = [fLayer nextDrawable];
        fResolveTexture = currentDrawable.texture;
        *fDrawable = (__bridge_retained GrMTLHandle) currentDrawable;
    }
    return fResolveTexture;
}

/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlTextureRenderTarget.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   SkBudgeted budgeted,
                                                   const GrSurfaceDesc& desc,
                                                   int sampleCnt,
                                                   id<MTLTexture> colorTexture,
                                                   id<MTLTexture> resolveTexture,
                                                   GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc, GrProtected::kNo)
        , GrMtlTexture(gpu, desc, resolveTexture, mipMapsStatus)
        , GrMtlRenderTarget(gpu, desc, sampleCnt, colorTexture, resolveTexture) {
    this->registerWithCache(budgeted);
}

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   SkBudgeted budgeted,
                                                   const GrSurfaceDesc& desc,
                                                   id<MTLTexture> colorTexture,
                                                   GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc, GrProtected::kNo)
        , GrMtlTexture(gpu, desc, colorTexture, mipMapsStatus)
        , GrMtlRenderTarget(gpu, desc, colorTexture) {
    this->registerWithCache(budgeted);
}

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   const GrSurfaceDesc& desc,
                                                   int sampleCnt,
                                                   id<MTLTexture> colorTexture,
                                                   id<MTLTexture> resolveTexture,
                                                   GrMipMapsStatus mipMapsStatus,
                                                   GrWrapCacheable cacheable)
        : GrSurface(gpu, desc, GrProtected::kNo)
        , GrMtlTexture(gpu, desc, resolveTexture, mipMapsStatus)
        , GrMtlRenderTarget(gpu, desc, sampleCnt, colorTexture, resolveTexture) {
    this->registerWithCacheWrapped(cacheable);
}

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   const GrSurfaceDesc& desc,
                                                   id<MTLTexture> colorTexture,
                                                   GrMipMapsStatus mipMapsStatus,
                                                   GrWrapCacheable cacheable)
        : GrSurface(gpu, desc, GrProtected::kNo)
        , GrMtlTexture(gpu, desc, colorTexture, mipMapsStatus)
        , GrMtlRenderTarget(gpu, desc, colorTexture) {
    this->registerWithCacheWrapped(cacheable);
}

id<MTLTexture> create_msaa_texture(GrMtlGpu* gpu, const GrSurfaceDesc& desc, int sampleCnt) {
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

    return [gpu->device() newTextureWithDescriptor:texDesc];
}

sk_sp<GrMtlTextureRenderTarget> GrMtlTextureRenderTarget::MakeNewTextureRenderTarget(
        GrMtlGpu* gpu,
        SkBudgeted budgeted,
        const GrSurfaceDesc& desc,
        int sampleCnt,
        MTLTextureDescriptor* texDesc,
        GrMipMapsStatus mipMapsStatus) {
    id<MTLTexture> texture = [gpu->device() newTextureWithDescriptor:texDesc];
    if (!texture) {
        return nullptr;
    }
    SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & texture.usage);

    if (sampleCnt > 1) {
        id<MTLTexture> colorTexture = create_msaa_texture(gpu, desc, sampleCnt);
        if (!colorTexture) {
            return nullptr;
        }
        SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & colorTexture.usage);
        return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(
                gpu, budgeted, desc, sampleCnt, colorTexture, texture, mipMapsStatus));
    } else {
        return sk_sp<GrMtlTextureRenderTarget>(
                new GrMtlTextureRenderTarget(gpu, budgeted, desc, texture, mipMapsStatus));
    }
}

sk_sp<GrMtlTextureRenderTarget> GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(
        GrMtlGpu* gpu,
        const GrSurfaceDesc& desc,
        int sampleCnt,
        id<MTLTexture> texture,
        GrWrapCacheable cacheable) {
    SkASSERT(nil != texture);
    SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & texture.usage);
    GrMipMapsStatus mipMapsStatus = texture.mipmapLevelCount > 1
                                            ? GrMipMapsStatus::kDirty
                                            : GrMipMapsStatus::kNotAllocated;
    if (sampleCnt > 1) {
        id<MTLTexture> colorTexture = create_msaa_texture(gpu, desc, sampleCnt);
        if (!colorTexture) {
            return nullptr;
        }
        SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & colorTexture.usage);
        return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(
                gpu, desc, sampleCnt, colorTexture, texture, mipMapsStatus, cacheable));
    } else {
        return sk_sp<GrMtlTextureRenderTarget>(
                new GrMtlTextureRenderTarget(gpu, desc, texture, mipMapsStatus, cacheable));
    }
}

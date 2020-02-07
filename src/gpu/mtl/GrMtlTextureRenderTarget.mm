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
                                                   SkISize dimensions,
                                                   int sampleCnt,
                                                   id<MTLTexture> colorTexture,
                                                   id<MTLTexture> resolveTexture,
                                                   GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrMtlTexture(gpu, dimensions, resolveTexture, mipMapsStatus)
        , GrMtlRenderTarget(gpu, dimensions, sampleCnt, colorTexture, resolveTexture) {
    this->registerWithCache(budgeted);
}

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   SkBudgeted budgeted,
                                                   SkISize dimensions,
                                                   id<MTLTexture> colorTexture,
                                                   GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrMtlTexture(gpu, dimensions, colorTexture, mipMapsStatus)
        , GrMtlRenderTarget(gpu, dimensions, colorTexture) {
    this->registerWithCache(budgeted);
}

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   SkISize dimensions,
                                                   int sampleCnt,
                                                   id<MTLTexture> colorTexture,
                                                   id<MTLTexture> resolveTexture,
                                                   GrMipMapsStatus mipMapsStatus,
                                                   GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrMtlTexture(gpu, dimensions, resolveTexture, mipMapsStatus)
        , GrMtlRenderTarget(gpu, dimensions, sampleCnt, colorTexture, resolveTexture) {
    this->registerWithCacheWrapped(cacheable);
}

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   SkISize dimensions,
                                                   id<MTLTexture> colorTexture,
                                                   GrMipMapsStatus mipMapsStatus,
                                                   GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrMtlTexture(gpu, dimensions, colorTexture, mipMapsStatus)
        , GrMtlRenderTarget(gpu, dimensions, colorTexture) {
    this->registerWithCacheWrapped(cacheable);
}

id<MTLTexture> create_msaa_texture(GrMtlGpu* gpu, SkISize dimensions, MTLPixelFormat format,
                                   int sampleCnt) {
    if (!gpu->mtlCaps().isFormatRenderable(format, sampleCnt)) {
        return nullptr;
    }
    MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
    texDesc.textureType = MTLTextureType2DMultisample;
    texDesc.pixelFormat = format;
    texDesc.width = dimensions.fWidth;
    texDesc.height = dimensions.fHeight;
    texDesc.depth = 1;
    texDesc.mipmapLevelCount = 1;
    texDesc.sampleCount = sampleCnt;
    texDesc.arrayLength = 1;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        texDesc.storageMode = MTLStorageModePrivate;
        texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
    }

    return [gpu->device() newTextureWithDescriptor:texDesc];
}

sk_sp<GrMtlTextureRenderTarget> GrMtlTextureRenderTarget::MakeNewTextureRenderTarget(
        GrMtlGpu* gpu,
        SkBudgeted budgeted,
        SkISize dimensions,
        int sampleCnt,
        MTLTextureDescriptor* texDesc,
        GrMipMapsStatus mipMapsStatus) {
    id<MTLTexture> texture = [gpu->device() newTextureWithDescriptor:texDesc];
    if (!texture) {
        return nullptr;
    }
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & texture.usage);
    }

    if (sampleCnt > 1) {
        id<MTLTexture> colorTexture =
                create_msaa_texture(gpu, dimensions, texture.pixelFormat, sampleCnt);
        if (!colorTexture) {
            return nullptr;
        }
        if (@available(macOS 10.11, iOS 9.0, *)) {
            SkASSERT((MTLTextureUsageShaderRead|MTLTextureUsageRenderTarget) & colorTexture.usage);
        }
        return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(
                gpu, budgeted, dimensions, sampleCnt, colorTexture, texture, mipMapsStatus));
    } else {
        return sk_sp<GrMtlTextureRenderTarget>(
                new GrMtlTextureRenderTarget(gpu, budgeted, dimensions, texture, mipMapsStatus));
    }
}

sk_sp<GrMtlTextureRenderTarget> GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(
        GrMtlGpu* gpu,
        SkISize dimensions,
        int sampleCnt,
        id<MTLTexture> texture,
        GrWrapCacheable cacheable) {
    SkASSERT(nil != texture);
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & texture.usage);
    }
    GrMipMapsStatus mipMapsStatus = texture.mipmapLevelCount > 1
                                            ? GrMipMapsStatus::kDirty
                                            : GrMipMapsStatus::kNotAllocated;
    if (sampleCnt > 1) {
        id<MTLTexture> colorTexture =
                create_msaa_texture(gpu, dimensions, texture.pixelFormat, sampleCnt);
        if (!colorTexture) {
            return nullptr;
        }
        if (@available(macOS 10.11, iOS 9.0, *)) {
            SkASSERT((MTLTextureUsageShaderRead|MTLTextureUsageRenderTarget) & colorTexture.usage);
        }
        return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(
                gpu, dimensions, sampleCnt, colorTexture, texture, mipMapsStatus, cacheable));
    } else {
        return sk_sp<GrMtlTextureRenderTarget>(
                new GrMtlTextureRenderTarget(gpu, dimensions, texture, mipMapsStatus, cacheable));
    }
}

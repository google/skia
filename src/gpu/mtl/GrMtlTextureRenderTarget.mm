/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlTextureRenderTarget.h"
#include "src/gpu/mtl/GrMtlUtil.h"

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   SkBudgeted budgeted,
                                                   SkISize dimensions,
                                                   int sampleCnt,
                                                   sk_cf_obj<id<MTLTexture>> colorTexture,
                                                   sk_cf_obj<id<MTLTexture>> resolveTexture,
                                                   GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrMtlTexture(gpu, dimensions, resolveTexture, mipmapStatus)
        , GrMtlRenderTarget(gpu, dimensions, sampleCnt, std::move(colorTexture),
                            std::move(resolveTexture)) {
    this->registerWithCache(budgeted);
}

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   SkBudgeted budgeted,
                                                   SkISize dimensions,
                                                   sk_cf_obj<id<MTLTexture>> colorTexture,
                                                   GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrMtlTexture(gpu, dimensions, colorTexture, mipmapStatus)
        , GrMtlRenderTarget(gpu, dimensions, std::move(colorTexture)) {
    this->registerWithCache(budgeted);
}

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   SkISize dimensions,
                                                   int sampleCnt,
                                                   sk_cf_obj<id<MTLTexture>> colorTexture,
                                                   sk_cf_obj<id<MTLTexture>> resolveTexture,
                                                   GrMipmapStatus mipmapStatus,
                                                   GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrMtlTexture(gpu, dimensions, resolveTexture, mipmapStatus)
        , GrMtlRenderTarget(gpu, dimensions, sampleCnt, std::move(colorTexture),
                            std::move(resolveTexture)) {
    this->registerWithCacheWrapped(cacheable);
}

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   SkISize dimensions,
                                                   sk_cf_obj<id<MTLTexture>> colorTexture,
                                                   GrMipmapStatus mipmapStatus,
                                                   GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrMtlTexture(gpu, dimensions, colorTexture, mipmapStatus)
        , GrMtlRenderTarget(gpu, dimensions, std::move(colorTexture)) {
    this->registerWithCacheWrapped(cacheable);
}

sk_cf_obj<id<MTLTexture>> create_msaa_texture(GrMtlGpu* gpu, SkISize dimensions,
                                              MTLPixelFormat format, int sampleCnt) {
    if (!gpu->mtlCaps().isFormatRenderable(format, sampleCnt)) {
        return nullptr;
    }
    sk_cf_obj<MTLTextureDescriptor*> texDesc([[MTLTextureDescriptor alloc] init]);
    (*texDesc).textureType = MTLTextureType2DMultisample;
    (*texDesc).pixelFormat = format;
    (*texDesc).width = dimensions.fWidth;
    (*texDesc).height = dimensions.fHeight;
    (*texDesc).depth = 1;
    (*texDesc).mipmapLevelCount = 1;
    (*texDesc).sampleCount = sampleCnt;
    (*texDesc).arrayLength = 1;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        (*texDesc).storageMode = MTLStorageModePrivate;
        (*texDesc).usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
    }

    return sk_cf_obj<id<MTLTexture>>([gpu->device() newTextureWithDescriptor:texDesc.get()]);
}

sk_sp<GrMtlTextureRenderTarget> GrMtlTextureRenderTarget::MakeNewTextureRenderTarget(
        GrMtlGpu* gpu,
        SkBudgeted budgeted,
        SkISize dimensions,
        int sampleCnt,
        MTLTextureDescriptor* texDesc,
        GrMipmapStatus mipmapStatus) {
    sk_cf_obj<id<MTLTexture>> texture([gpu->device() newTextureWithDescriptor:texDesc]);
    if (!texture) {
        return nullptr;
    }
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & (*texture).usage);
    }

    if (sampleCnt > 1) {
        sk_cf_obj<id<MTLTexture>> colorTexture =
                create_msaa_texture(gpu, dimensions, (*texture).pixelFormat, sampleCnt);
        if (!colorTexture) {
            return nullptr;
        }
        if (@available(macOS 10.11, iOS 9.0, *)) {
            SkASSERT((MTLTextureUsageShaderRead|MTLTextureUsageRenderTarget) &
                             (*colorTexture).usage);
        }
        return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(
                gpu, budgeted, dimensions, sampleCnt, std::move(colorTexture), std::move(texture),
                mipmapStatus));
    } else {
        return sk_sp<GrMtlTextureRenderTarget>(
                new GrMtlTextureRenderTarget(gpu, budgeted, dimensions, std::move(texture),
                                             mipmapStatus));
    }
}

sk_sp<GrMtlTextureRenderTarget> GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(
        GrMtlGpu* gpu,
        SkISize dimensions,
        int sampleCnt,
        sk_cf_obj<id<MTLTexture>> texture,
        GrWrapCacheable cacheable) {
    SkASSERT(texture);
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & (*texture).usage);
    }
    GrMipmapStatus mipmapStatus = (*texture).mipmapLevelCount > 1
                                            ? GrMipmapStatus::kDirty
                                            : GrMipmapStatus::kNotAllocated;
    if (sampleCnt > 1) {
        sk_cf_obj<id<MTLTexture>> colorTexture =
                create_msaa_texture(gpu, dimensions, (*texture).pixelFormat, sampleCnt);
        if (!colorTexture) {
            return nullptr;
        }
        if (@available(macOS 10.11, iOS 9.0, *)) {
            SkASSERT((MTLTextureUsageShaderRead|MTLTextureUsageRenderTarget) &
                             (*colorTexture).usage);
        }
        return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(
                gpu, dimensions, sampleCnt, std::move(colorTexture), std::move(texture),
                mipmapStatus, cacheable));
    } else {
        return sk_sp<GrMtlTextureRenderTarget>(
                new GrMtlTextureRenderTarget(gpu, dimensions, std::move(texture), mipmapStatus,
                                             cacheable));
    }
}

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
                                                   const GrSurfaceDesc& desc,
                                                   id<MTLTexture> renderTexture,
                                                   GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc)
        , GrMtlTexture(gpu, desc, renderTexture, mipMapsStatus)
        , GrMtlRenderTarget(gpu, desc, renderTexture) {
    this->registerWithCache(budgeted);
}

GrMtlTextureRenderTarget::GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                                                   const GrSurfaceDesc& desc,
                                                   id<MTLTexture> renderTexture,
                                                   GrMipMapsStatus mipMapsStatus,
                                                   GrWrapCacheable cacheable)
        : GrSurface(gpu, desc)
        , GrMtlTexture(gpu, desc, renderTexture, mipMapsStatus)
        , GrMtlRenderTarget(gpu, desc, renderTexture) {
    this->registerWithCacheWrapped(cacheable);
}

sk_sp<GrMtlTextureRenderTarget>
GrMtlTextureRenderTarget::CreateNewTextureRenderTarget(GrMtlGpu* gpu,
                                                       SkBudgeted budgeted,
                                                       const GrSurfaceDesc& desc,
                                                       MTLTextureDescriptor* texDesc,
                                                       GrMipMapsStatus mipMapsStatus) {
    id<MTLTexture> renderTexture = [gpu->device() newTextureWithDescriptor:texDesc];
    SkASSERT(nil != renderTexture);
    if (desc.fSampleCnt > 1) {
        return nullptr;
    }
    SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & renderTexture.usage);
    return sk_sp<GrMtlTextureRenderTarget>(
            new GrMtlTextureRenderTarget(gpu, budgeted, desc, renderTexture, mipMapsStatus));
}

sk_sp<GrMtlTextureRenderTarget> GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(
        GrMtlGpu* gpu,
        const GrSurfaceDesc& desc,
        id<MTLTexture> renderTexture,
        GrWrapCacheable cacheable) {
    SkASSERT(nil != renderTexture);
    GrMipMapsStatus mipMapsStatus = renderTexture.mipmapLevelCount > 1
                                            ? GrMipMapsStatus::kDirty
                                            : GrMipMapsStatus::kNotAllocated;
    if (desc.fSampleCnt > 1) {
        return nullptr;
    }
    SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & renderTexture.usage);
    return sk_sp<GrMtlTextureRenderTarget>(
            new GrMtlTextureRenderTarget(gpu, desc, renderTexture, mipMapsStatus, cacheable));
}

/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlTextureRenderTarget.h"
#include "GrMtlGpu.h"
#include "GrMtlUtil.h"

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
                                                   GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc)
        , GrMtlTexture(gpu, desc, renderTexture, mipMapsStatus)
        , GrMtlRenderTarget(gpu, desc, renderTexture) {
    this->registerWithCacheWrapped();
}

sk_sp<GrMtlTextureRenderTarget>
GrMtlTextureRenderTarget::Make(GrMtlGpu* gpu,
                               const GrSurfaceDesc& desc,
                               id<MTLTexture> renderTexture,
                               GrMipMapsStatus mipMapsStatus,
                               SkBudgeted budgeted,
                               bool isWrapped) {
    SkASSERT(nil != renderTexture);
    if (desc.fSampleCnt > 1) {
        SkASSERT(false); // Currently don't support MSAA
        return nullptr;
    }
    SkASSERT((MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) & renderTexture.usage);
    if (!isWrapped) {
        return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(gpu,
                                                                            budgeted,
                                                                            desc,
                                                                            renderTexture,
                                                                            mipMapsStatus));
    } else {
        return sk_sp<GrMtlTextureRenderTarget>(new GrMtlTextureRenderTarget(gpu,
                                                                            desc,
                                                                            renderTexture,
                                                                            mipMapsStatus));
    }
}


sk_sp<GrMtlTextureRenderTarget>
GrMtlTextureRenderTarget::CreateNewTextureRenderTarget(GrMtlGpu* gpu,
                                                       SkBudgeted budgeted,
                                                       const GrSurfaceDesc& desc,
                                                       MTLTextureDescriptor* texDesc,
                                                       GrMipMapsStatus mipMapsStatus) {
    id<MTLTexture> texture = [gpu->device() newTextureWithDescriptor:texDesc];
    return Make(gpu, desc, texture, mipMapsStatus, budgeted, false);
}

sk_sp<GrMtlTextureRenderTarget>
GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(GrMtlGpu* gpu,
                                                         const GrSurfaceDesc& desc,
                                                         id<MTLTexture> texture) {

    SkASSERT(nil != texture);
    GrMipMapsStatus mipMapsStatus = texture.mipmapLevelCount > 1 ? GrMipMapsStatus::kDirty
                                                                 : GrMipMapsStatus::kNotAllocated;
    return Make(gpu, desc, texture, mipMapsStatus, SkBudgeted::kNo, true);
}


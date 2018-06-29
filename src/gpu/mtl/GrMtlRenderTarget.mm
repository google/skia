/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlRenderTarget.h"

#include "GrMtlGpu.h"
#include "GrMtlUtil.h"

GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     SkBudgeted budgeted,
                                     const GrSurfaceDesc& desc,
                                     id<MTLTexture> renderTexture)
        : GrSurface(gpu, desc)
        , GrRenderTarget(gpu, desc)
        , fRenderTexture(renderTexture)
        , fResolveTexture(nil) {
    SkASSERT(1 == desc.fSampleCnt);
    this->registerWithCache(budgeted);
}

GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     const GrSurfaceDesc& desc,
                                     id<MTLTexture> renderTexture)
        : GrSurface(gpu, desc)
        , GrRenderTarget(gpu, desc)
        , fRenderTexture(renderTexture)
        , fResolveTexture(nil) {
    SkASSERT(1 == desc.fSampleCnt);
}

sk_sp<GrMtlRenderTarget>
GrMtlRenderTarget::MakeWrappedRenderTarget(GrMtlGpu* gpu, const GrSurfaceDesc& desc,
                                           id<MTLTexture> renderTexture) {
    SkASSERT(nil != renderTexture);
    SkASSERT(1 == renderTexture.mipmapLevelCount);
    SkASSERT(MTLTextureUsageRenderTarget & renderTexture.usage);
    return sk_sp<GrMtlRenderTarget>(new GrMtlRenderTarget(gpu, SkBudgeted::kNo,
                                                          desc, renderTexture));
}

GrMtlRenderTarget::~GrMtlRenderTarget() {
    SkASSERT(nil == fRenderTexture);
    SkASSERT(nil == fResolveTexture);
}


GrBackendRenderTarget GrMtlRenderTarget::getBackendRenderTarget() const {
    GrMtlTextureInfo info;
    info.fTexture = GrGetPtrFromId(fRenderTexture);
    return GrBackendRenderTarget(this->width(), this->height(), fRenderTexture.sampleCount, info);
}

GrMtlGpu* GrMtlRenderTarget::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

void GrMtlRenderTarget::onAbandon() {
    fRenderTexture = nil;
    fResolveTexture = nil;
}

void GrMtlRenderTarget::onRelease() {
    fRenderTexture = nil;
    fResolveTexture = nil;
}

bool GrMtlRenderTarget::completeStencilAttachment() {
    // TODO: fill this out
    return true;
}


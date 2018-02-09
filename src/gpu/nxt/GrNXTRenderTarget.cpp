/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTRenderTarget.h"

#include "GrBackendSurface.h"
#include "GrNXTGpu.h"
#include "GrNXTUtil.h"

GrNXTRenderTarget::GrNXTRenderTarget(GrNXTGpu* gpu,
                                     const GrSurfaceDesc& desc,
                                     const GrNXTImageInfo& info,
                                     GrBackendObjectOwnership ownership)
    : GrSurface(gpu, desc)
    , GrRenderTarget(gpu, desc)
    , fInfo(info) {
    this->registerWithCacheWrapped();
}

GrNXTRenderTarget*
GrNXTRenderTarget::Create(GrNXTGpu* gpu,
                         const GrSurfaceDesc& desc,
                         const GrNXTImageInfo& info,
                         GrBackendObjectOwnership ownership) {
    SkASSERT(1 == info.fLevelCount);
    return new GrNXTRenderTarget(gpu, desc, info, ownership);
}

sk_sp<GrNXTRenderTarget>
GrNXTRenderTarget::MakeWrapped(GrNXTGpu* gpu,
                              const GrSurfaceDesc& desc,
                              const GrNXTImageInfo* info) {
    SkASSERT(info);
    SkASSERT(info->fTexture != nullptr);

    return sk_sp<GrNXTRenderTarget>(
        GrNXTRenderTarget::Create(gpu, desc, *info,
                                  GrBackendObjectOwnership::kBorrowed));
}

bool GrNXTRenderTarget::completeStencilAttachment() {
    return true;
}

GrNXTRenderTarget::~GrNXTRenderTarget() {
}

void GrNXTRenderTarget::onRelease() {
    INHERITED::onRelease();
}

void GrNXTRenderTarget::onAbandon() {
    INHERITED::onAbandon();
}


GrBackendObject GrNXTRenderTarget::getRenderTargetHandle() const {
    return (GrBackendObject) &fInfo;
}

GrBackendRenderTarget GrNXTRenderTarget::getBackendRenderTarget() const {
    return GrBackendRenderTarget(this->width(), this->height(), this->numColorSamples(),
                                 this->numStencilSamples(), fInfo);
}

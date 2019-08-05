/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnRenderTarget.h"

#include "include/gpu/GrBackendSurface.h"
#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnUtil.h"

GrDawnRenderTarget::GrDawnRenderTarget(GrDawnGpu* gpu,
                                       const GrSurfaceDesc& desc,
                                       int sampleCnt,
                                       const GrDawnImageInfo& info,
                                       GrBackendObjectOwnership ownership)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo)
        , GrRenderTarget(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, sampleCnt,
                         GrProtected::kNo)
        , fInfo(info) {
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

GrDawnRenderTarget*
GrDawnRenderTarget::Create(GrDawnGpu* gpu,
                           const GrSurfaceDesc& desc,
                           int sampleCnt,
                           const GrDawnImageInfo& info,
                           GrBackendObjectOwnership ownership) {
    SkASSERT(1 == info.fLevelCount);
    return new GrDawnRenderTarget(gpu, desc, sampleCnt, info, ownership);
}

sk_sp<GrDawnRenderTarget>
GrDawnRenderTarget::MakeWrapped(GrDawnGpu* gpu,
                                const GrSurfaceDesc& desc,
                                int sampleCnt,
                                const GrDawnImageInfo& info) {
    return sk_sp<GrDawnRenderTarget>(
        GrDawnRenderTarget::Create(gpu, desc, sampleCnt, info,
                                  GrBackendObjectOwnership::kBorrowed));
}

bool GrDawnRenderTarget::completeStencilAttachment() {
    return true;
}

GrDawnRenderTarget::~GrDawnRenderTarget() {
}

void GrDawnRenderTarget::onRelease() {
    INHERITED::onRelease();
}

void GrDawnRenderTarget::onAbandon() {
    INHERITED::onAbandon();
}

GrBackendRenderTarget GrDawnRenderTarget::getBackendRenderTarget() const {
    return GrBackendRenderTarget(this->width(), this->height(), this->numSamples(),
                                 this->numSamples(), fInfo);
}

GrBackendFormat GrDawnRenderTarget::backendFormat() const {
    return GrBackendFormat::MakeDawn(fInfo.fFormat);
}

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
                                       const GrDawnImageInfo& info)
    : GrSurface(gpu, desc, GrProtected::kNo)
    , GrRenderTarget(gpu, desc, sampleCnt, GrProtected::kNo)
    , fInfo(info) {
}

sk_sp<GrDawnRenderTarget>
GrDawnRenderTarget::MakeWrapped(GrDawnGpu* gpu,
                                const GrSurfaceDesc& desc,
                                int sampleCnt,
                                const GrDawnImageInfo& info) {
    sk_sp<GrDawnRenderTarget> rt(new GrDawnRenderTarget(gpu, desc, sampleCnt, info));
    rt->registerWithCacheWrapped(GrWrapCacheable::kNo);
    return rt;
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

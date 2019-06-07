/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnRenderTarget.h"

#include "include/gpu/GrBackendSurface.h"
#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnUtil.h"

GrDawnRenderTarget::GrDawnRenderTarget(GrDawnGpu* gpu,
                                       const GrSurfaceDesc& desc,
                                       const GrDawnImageInfo& info)
    : GrSurface(gpu, desc)
    , GrRenderTarget(gpu, desc)
    , fInfo(info) {
}

sk_sp<GrDawnRenderTarget>
GrDawnRenderTarget::MakeWrapped(GrDawnGpu* gpu,
                                const GrSurfaceDesc& desc,
                                const GrDawnImageInfo& info) {
    sk_sp<GrDawnRenderTarget> rt(new GrDawnRenderTarget(gpu, desc, info));
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
    return GrBackendRenderTarget(this->width(), this->height(), this->numColorSamples(),
                                 this->numStencilSamples(), fInfo);
}

GrBackendFormat GrDawnRenderTarget::backendFormat() const {
    return GrBackendFormat::MakeDawn(fInfo.fFormat);
}

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
                                     const GrNXTImageInfo& info)
    : GrSurface(gpu, desc)
    , GrRenderTarget(gpu, desc)
    , fInfo(info) {
}

sk_sp<GrNXTRenderTarget>
GrNXTRenderTarget::MakeWrapped(GrNXTGpu* gpu,
                              const GrSurfaceDesc& desc,
                              const GrNXTImageInfo& info) {
    SkASSERT(info.fTexture != nullptr);

    sk_sp<GrNXTRenderTarget> rt(new GrNXTRenderTarget(gpu, desc, info));
    rt->registerWithCacheWrapped();
    return rt;
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

GrBackendRenderTarget GrNXTRenderTarget::getBackendRenderTarget() const {
    return GrBackendRenderTarget(this->width(), this->height(), this->numColorSamples(),
                                 this->numStencilSamples(), fInfo);
}

/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLGpuCommandBuffer.h"

#include "GrContextPriv.h"
#include "GrFixedClip.h"
#include "GrRenderTargetPriv.h"

void GrGLGpuRTCommandBuffer::begin() {
    if (GrLoadOp::kClear == fColorLoadAndStoreInfo.fLoadOp) {
        fGpu->clear(GrFixedClip::Disabled(), fColorLoadAndStoreInfo.fClearColor,
                    fRenderTarget, fOrigin);
    }
    if (GrLoadOp::kClear == fStencilLoadAndStoreInfo.fLoadOp) {
        GrStencilAttachment* sb = fRenderTarget->renderTargetPriv().getStencilAttachment();
        if (sb && (sb->isDirty() || fRenderTarget->alwaysClearStencil())) {
            fGpu->clearStencil(fRenderTarget, 0x0);
        }
    }
}

void GrGLGpuRTCommandBuffer::set(GrRenderTarget* rt, GrSurfaceOrigin origin,
                                 const GrGpuRTCommandBuffer::LoadAndStoreInfo& colorInfo,
                                 const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo& stencilInfo) {
    SkASSERT(fGpu);
    SkASSERT(!fRenderTarget);
    SkASSERT(fGpu == rt->getContext()->priv().getGpu());

    this->INHERITED::set(rt, origin);
    fColorLoadAndStoreInfo = colorInfo;
    fStencilLoadAndStoreInfo = stencilInfo;
}

/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLGpuCommandBuffer.h"

#include "GrFixedClip.h"
#include "GrRenderTargetPriv.h"

void GrGLGpuRTCommandBuffer::begin() {
    if (fRenderTarget->renderTargetPriv().hasCoverageCountBuffer()) {
        // SkDebugf("@@@@@@@@@> clearing!!!!\n");
        GR_GL_CALL(fGpu->glInterface(), StartTiling(0, 0, fRenderTarget->width(),
                                                    fRenderTarget->height(), 0));
        fGpu->clearCoverageCountBuffer(fRenderTarget);
    }
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

void GrGLGpuRTCommandBuffer::coverageCountReadBarrier() {
    // FRAMEBUFFER_FETCH_NONCOHERENT (needed???)
    GR_GL_CALL(((GrGLGpu*)this->gpu())->glInterface(), FramebufferFetchBarrier());
}

void GrGLGpuRTCommandBuffer::end() {
    if (fRenderTarget->renderTargetPriv().hasCoverageCountBuffer()) {
        // SkDebugf("@@@@@@@@@> discarding!!!!\n");
#define GL_COLOR_BUFFER_BIT0                     0x00000001
        GR_GL_CALL(fGpu->glInterface(), EndTiling(0));
        fGpu->discardCoverageCountBuffer(fRenderTarget);
    }
}

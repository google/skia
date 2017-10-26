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
    const char* kStr[] = { "kLoad", "kClear", "kDiscard" };

    printf("GrGLGpuRTCommandBuffer::begin %s %s\n",
        kStr[(int)fColorLoadAndStoreInfo.fLoadOp], kStr[(int)fStencilLoadAndStoreInfo.fLoadOp]);

    if (GrLoadOp::kClear == fColorLoadAndStoreInfo.fLoadOp) {
        fGpu->clear(GrFixedClip::Disabled(), fColorLoadAndStoreInfo.fClearColor,
                    fRenderTarget, fOrigin);
    }
    if (GrLoadOp::kClear == fStencilLoadAndStoreInfo.fLoadOp) {
        GrStencilAttachment* sb = fRenderTarget->renderTargetPriv().getStencilAttachment();

        printf("clearing SB %p %s %s\n", sb, !sb ? "nullSB" : sb->isDirty() ? "dirty" : "clean",
            fRenderTarget->alwaysClearStencil() ? "alwaysClear" : "!alwaysClear");

        if (sb && (sb->isDirty() || fRenderTarget->alwaysClearStencil())) {
            fGpu->clearStencil(fRenderTarget, 0x0);
        }
    }
}

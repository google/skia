
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilBuffer.h"

#include "GrContext.h"
#include "GrGpu.h"

void GrStencilBuffer::wasDetachedFromRenderTarget(const GrRenderTarget* rt) {
    GrAssert(fRTAttachmentCnt > 0);
    if (0 == --fRTAttachmentCnt && NULL != fCacheEntry) {
        this->getGpu()->getContext()->unlockStencilBuffer(fCacheEntry);
        // At this point we could be deleted!
    }
}

void GrStencilBuffer::transferToCacheAndLock() {
    GrAssert(NULL == fCacheEntry);
    fCacheEntry = 
        this->getGpu()->getContext()->addAndLockStencilBuffer(this);
}

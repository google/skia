
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
    if (0 == --fRTAttachmentCnt) {
        this->unlockInCache();
        // At this point we could be deleted!
    }
}

void GrStencilBuffer::transferToCacheAndLock() {
    GrAssert(NULL == fCacheEntry);
    fCacheEntry = 
        this->getGpu()->getContext()->addAndLockStencilBuffer(this);
}

void GrStencilBuffer::onRelease() {
    // When the GrGpu rips through its list of resources and releases
    // them it may release an SB before it releases its attached RTs.
    // In that case when GrStencilBuffer sees its last detach it no
    // long has a gpu ptr (gets nulled in GrResource::release()) and can't
    // access the cache to unlock itself. So if we're being released and still
    // have attachments go ahead and unlock now.
    if (fRTAttachmentCnt) {
        this->unlockInCache();
        // we shouldn't be deleted here because some RT still has a ref on us.
    }
    fCacheEntry = NULL;
}

void GrStencilBuffer::onAbandon() {
    // we can use the same behavior as release.
    this->onRelease();
}

void GrStencilBuffer::unlockInCache() {
    if (NULL != fCacheEntry) {
        GrGpu* gpu = this->getGpu();
        if (NULL != gpu) {
            GrAssert(NULL != gpu->getContext());
            gpu->getContext()->unlockStencilBuffer(fCacheEntry);
        }
    }
}

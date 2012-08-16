
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilBuffer.h"

#include "GrContext.h"
#include "GrGpu.h"
#include "GrResourceCache.h"

GR_DEFINE_RESOURCE_CACHE_TYPE(GrStencilBuffer)

void GrStencilBuffer::wasDetachedFromRenderTarget(const GrRenderTarget* rt) {
    GrAssert(fRTAttachmentCnt > 0);
    if (0 == --fRTAttachmentCnt) {
        this->unlockInCache();
        // At this point we could be deleted!
    }
}

void GrStencilBuffer::transferToCacheAndLock() {
    GrAssert(NULL == this->getCacheEntry());
    GrAssert(!fHoldingLock);

    this->getGpu()->getContext()->addAndLockStencilBuffer(this);
    fHoldingLock = true;
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
    fHoldingLock = false;
}

void GrStencilBuffer::onAbandon() {
    // we can use the same behavior as release.
    this->onRelease();
}

void GrStencilBuffer::unlockInCache() {
    if (fHoldingLock) {
        GrGpu* gpu = this->getGpu();
        if (NULL != gpu) {
            GrAssert(NULL != gpu->getContext());
            gpu->getContext()->unlockStencilBuffer(this);
        }
    }
}

namespace {
// we should never have more than one stencil buffer with same combo of
// (width,height,samplecount)
void gen_stencil_key_values(int width, 
                            int height,
                            int sampleCnt,
                            GrCacheID* cacheID) {
    cacheID->fPublicID = GrCacheID::kDefaultPublicCacheID;
    cacheID->fResourceSpecific32 = width | (height << 16);
    cacheID->fDomain = GrCacheData::kScratch_ResourceDomain;

    GrAssert(sampleCnt >= 0 && sampleCnt < 256);
    cacheID->fResourceSpecific16 = sampleCnt << 8;

    // last 8 bits of 'fResourceSpecific16' is free for flags
}
}

GrResourceKey GrStencilBuffer::ComputeKey(int width, 
                                          int height, 
                                          int sampleCnt) {
    GrCacheID id(GrStencilBuffer::GetResourceType());
    gen_stencil_key_values(width, height, sampleCnt, &id);

    uint32_t v[4];
    id.toRaw(v);
    return GrResourceKey(v);
}


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

SK_DEFINE_INST_COUNT(GrStencilBuffer)

void GrStencilBuffer::transferToCache() {
    SkASSERT(NULL == this->getCacheEntry());

    this->getGpu()->getContext()->addStencilBuffer(this);
}

namespace {
// we should never have more than one stencil buffer with same combo of (width,height,samplecount)
void gen_cache_id(int width, int height, int sampleCnt, GrCacheID* cacheID) {
    static const GrCacheID::Domain gStencilBufferDomain = GrCacheID::GenerateDomain();
    GrCacheID::Key key;
    uint32_t* keyData = key.fData32;
    keyData[0] = width;
    keyData[1] = height;
    keyData[2] = sampleCnt;
    memset(keyData + 3, 0, sizeof(key) - 3 * sizeof(uint32_t));
    GR_STATIC_ASSERT(sizeof(key) >= 3 * sizeof(uint32_t));
    cacheID->reset(gStencilBufferDomain, key);
}
}

GrResourceKey GrStencilBuffer::ComputeKey(int width,
                                          int height,
                                          int sampleCnt) {
    // All SBs are created internally to attach to RTs so they all use the same domain.
    static const GrResourceKey::ResourceType gStencilBufferResourceType =
        GrResourceKey::GenerateResourceType();
    GrCacheID id;
    gen_cache_id(width, height, sampleCnt, &id);

    // we don't use any flags for SBs currently.
    return GrResourceKey(id, gStencilBufferResourceType, 0);
}

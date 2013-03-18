/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk64.h"
#include "SkLazyPixelRef.h"
#include "SkColorTable.h"
#include "SkData.h"
#include "SkImageCache.h"
#include "SkImagePriv.h"

#if LAZY_CACHE_STATS
#include "SkThread.h"

int32_t SkLazyPixelRef::gCacheHits;
int32_t SkLazyPixelRef::gCacheMisses;
#endif

SkLazyPixelRef::SkLazyPixelRef(SkData* data, SkBitmapFactory::DecodeProc proc, SkImageCache* cache)
    // Pass NULL for the Mutex so that the default (ring buffer) will be used.
    : INHERITED(NULL)
    , fDecodeProc(proc)
    , fImageCache(cache)
    , fCacheId(SkImageCache::UNINITIALIZED_ID)
    , fRowBytes(0) {
    SkASSERT(fDecodeProc != NULL);
    if (NULL == data) {
        fData = SkData::NewEmpty();
        fErrorInDecoding = true;
    } else {
        fData = data;
        fData->ref();
        fErrorInDecoding = data->size() == 0;
    }
    SkASSERT(cache != NULL);
    cache->ref();
    // Since this pixel ref bases its data on encoded data, it should never change.
    this->setImmutable();
}

SkLazyPixelRef::~SkLazyPixelRef() {
    SkASSERT(fData != NULL);
    fData->unref();
    SkASSERT(fImageCache);
    if (fCacheId != SkImageCache::UNINITIALIZED_ID) {
        fImageCache->throwAwayCache(fCacheId);
    }
    fImageCache->unref();
}

static size_t ComputeMinRowBytesAndSize(const SkImage::Info& info, size_t* rowBytes) {
    *rowBytes = SkImageMinRowBytes(info);

    Sk64 safeSize;
    safeSize.setZero();
    if (info.fHeight > 0) {
        safeSize.setMul(info.fHeight, SkToS32(*rowBytes));
    }
    SkASSERT(!safeSize.isNeg());
    return safeSize.is32() ? safeSize.get32() : 0;
}

void* SkLazyPixelRef::onLockPixels(SkColorTable**) {
    if (fErrorInDecoding) {
        return NULL;
    }
    SkBitmapFactory::Target target;
    // Check to see if the pixels still exist in the cache.
    if (SkImageCache::UNINITIALIZED_ID == fCacheId) {
        target.fAddr = NULL;
    } else {
        SkImageCache::DataStatus status;
        target.fAddr = fImageCache->pinCache(fCacheId, &status);
        if (target.fAddr == NULL) {
            fCacheId = SkImageCache::UNINITIALIZED_ID;
        } else {
            if (SkImageCache::kRetained_DataStatus == status) {
#if LAZY_CACHE_STATS
                sk_atomic_inc(&gCacheHits);
#endif
                return target.fAddr;
            }
            SkASSERT(SkImageCache::kUninitialized_DataStatus == status);
        }
        // Cache miss. Either pinCache returned NULL or it returned a memory address without the old
        // data
#if LAZY_CACHE_STATS
        sk_atomic_inc(&gCacheMisses);
#endif
    }
    SkImage::Info info;
    SkASSERT(fData != NULL && fData->size() > 0);
    if (NULL == target.fAddr) {
        // Determine the size of the image in order to determine how much memory to allocate.
        // FIXME: As an optimization, only do this part once.
        fErrorInDecoding = !fDecodeProc(fData->data(), fData->size(), &info, NULL);
        if (fErrorInDecoding) {
            // We can only reach here if fCacheId was already set to UNINITIALIZED_ID, or if
            // pinCache returned NULL, in which case it was reset to UNINITIALIZED_ID.
            SkASSERT(SkImageCache::UNINITIALIZED_ID == fCacheId);
            return NULL;
        }

        size_t bytes = ComputeMinRowBytesAndSize(info, &target.fRowBytes);
        target.fAddr = fImageCache->allocAndPinCache(bytes, &fCacheId);
        if (NULL == target.fAddr) {
            // Space could not be allocated.
            // Just like the last assert, fCacheId must be UNINITIALIZED_ID.
            SkASSERT(SkImageCache::UNINITIALIZED_ID == fCacheId);
            return NULL;
        }
    } else {
        // pinCache returned purged memory to which target.fAddr already points. Set
        // target.fRowBytes properly.
        target.fRowBytes = fRowBytes;
        // Assume that the size is correct, since it was determined by this same function
        // previously.
    }
    SkASSERT(target.fAddr != NULL);
    SkASSERT(SkImageCache::UNINITIALIZED_ID != fCacheId);
    fErrorInDecoding = !fDecodeProc(fData->data(), fData->size(), &info, &target);
    if (fErrorInDecoding) {
        fImageCache->throwAwayCache(fCacheId);
        fCacheId = SkImageCache::UNINITIALIZED_ID;
        return NULL;
    }
    // Upon success, store fRowBytes so it can be used in case pinCache later returns purged memory.
    fRowBytes = target.fRowBytes;
    return target.fAddr;
}

void SkLazyPixelRef::onUnlockPixels() {
    if (fErrorInDecoding) {
        return;
    }
    if (fCacheId != SkImageCache::UNINITIALIZED_ID) {
        fImageCache->releaseCache(fCacheId);
    }
}

SkData* SkLazyPixelRef::onRefEncodedData() {
    fData->ref();
    return fData;
}

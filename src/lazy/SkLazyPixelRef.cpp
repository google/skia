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

SkLazyPixelRef::SkLazyPixelRef(SkData* data, SkBitmapFactory::DecodeProc proc, SkImageCache* cache)
    // Pass NULL for the Mutex so that the default (ring buffer) will be used.
    : INHERITED(NULL)
    , fDecodeProc(proc)
    , fImageCache(cache)
    , fCacheId(SkImageCache::UNINITIALIZED_ID) {
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
    target.fAddr = SkImageCache::UNINITIALIZED_ID == fCacheId ?
                   NULL : fImageCache->pinCache(fCacheId);
    if (NULL == target.fAddr) {
        SkImage::Info info;
        SkASSERT(fData != NULL && fData->size() > 0);
        // FIXME: As an optimization, only do this part once.
        fErrorInDecoding = !fDecodeProc(fData->data(), fData->size(), &info, NULL);
        if (fErrorInDecoding) {
            fCacheId = SkImageCache::UNINITIALIZED_ID;
            return NULL;
        }
        // Allocate the memory.
        size_t bytes = ComputeMinRowBytesAndSize(info, &target.fRowBytes);

        target.fAddr = fImageCache->allocAndPinCache(bytes, &fCacheId);
        if (NULL == target.fAddr) {
            // Space could not be allocated.
            fCacheId = SkImageCache::UNINITIALIZED_ID;
            return NULL;
        }
        SkASSERT(SkImageCache::UNINITIALIZED_ID != fCacheId);
        fErrorInDecoding = !fDecodeProc(fData->data(), fData->size(), &info, &target);
        if (fErrorInDecoding) {
            fImageCache->throwAwayCache(fCacheId);
            fCacheId = SkImageCache::UNINITIALIZED_ID;
            return NULL;
        }
    }
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

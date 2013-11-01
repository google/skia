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
#include "SkScaledImageCache.h"

#if LAZY_CACHE_STATS
#include "SkThread.h"

int32_t SkLazyPixelRef::gCacheHits;
int32_t SkLazyPixelRef::gCacheMisses;
#endif

SkLazyPixelRef::SkLazyPixelRef(SkData* data, SkBitmapFactory::DecodeProc proc, SkImageCache* cache)
    // Pass NULL for the Mutex so that the default (ring buffer) will be used.
    : INHERITED(NULL)
    , fErrorInDecoding(false)
    , fDecodeProc(proc)
    , fImageCache(cache)
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
    if (fImageCache != NULL) {
        fImageCache->ref();
        fCacheId = SkImageCache::UNINITIALIZED_ID;
    } else {
        fScaledCacheId = NULL;
    }

    // mark as uninitialized -- all fields are -1
    memset(&fLazilyCachedInfo, 0xFF, sizeof(fLazilyCachedInfo));

    // Since this pixel ref bases its data on encoded data, it should never change.
    this->setImmutable();
}

SkLazyPixelRef::~SkLazyPixelRef() {
    SkASSERT(fData != NULL);
    fData->unref();
    if (NULL == fImageCache) {
        if (fScaledCacheId != NULL) {
            SkScaledImageCache::Unlock(fScaledCacheId);
            // TODO(halcanary): SkScaledImageCache needs a
            // throwAwayCache(id) method.
        }
        return;
    }
    SkASSERT(fImageCache);
    if (fCacheId != SkImageCache::UNINITIALIZED_ID) {
        fImageCache->throwAwayCache(fCacheId);
    }
    fImageCache->unref();
}

static size_t ComputeMinRowBytesAndSize(const SkImageInfo& info, size_t* rowBytes) {
    *rowBytes = SkImageMinRowBytes(info);

    Sk64 safeSize;
    safeSize.setZero();
    if (info.fHeight > 0) {
        safeSize.setMul(info.fHeight, SkToS32(*rowBytes));
    }
    SkASSERT(!safeSize.isNeg());
    return safeSize.is32() ? safeSize.get32() : 0;
}

const SkImageInfo* SkLazyPixelRef::getCachedInfo() {
    if (fLazilyCachedInfo.fWidth < 0) {
        SkImageInfo info;
        fErrorInDecoding = !fDecodeProc(fData->data(), fData->size(), &info, NULL);
        if (fErrorInDecoding) {
            return NULL;
        }
        fLazilyCachedInfo = info;
    }
    return &fLazilyCachedInfo;
}

/**
   Returns bitmap->getPixels() on success; NULL on failure */
static void* decode_into_bitmap(SkImageInfo* info,
                                SkBitmapFactory::DecodeProc decodeProc,
                                size_t* rowBytes,
                                SkData* data,
                                SkBitmap* bm) {
    SkASSERT(info && decodeProc && rowBytes && data && bm);
    if (!(bm->setConfig(SkImageInfoToBitmapConfig(*info), info->fWidth,
                        info->fHeight, *rowBytes, info->fAlphaType)
          && bm->allocPixels(NULL, NULL))) {
        // Use the default allocator.  It may be necessary for the
        // SkLazyPixelRef to have a allocator field which is passed
        // into allocPixels().
        return NULL;
    }
    SkBitmapFactory::Target target;
    target.fAddr = bm->getPixels();
    target.fRowBytes = bm->rowBytes();
    *rowBytes = target.fRowBytes;
    if (!decodeProc(data->data(), data->size(), info, &target)) {
        return NULL;
    }
    return target.fAddr;
}

void* SkLazyPixelRef::lockScaledImageCachePixels() {
    SkASSERT(!fErrorInDecoding);
    SkASSERT(NULL == fImageCache);
    SkBitmap bitmap;
    const SkImageInfo* info = this->getCachedInfo();
    if (info == NULL) {
        return NULL;
    }
    // If this is the first time though, this is guaranteed to fail.
    // Maybe we should have a flag that says "don't even bother looking"
    fScaledCacheId = SkScaledImageCache::FindAndLock(this->getGenerationID(),
                                                     info->fWidth,
                                                     info->fHeight,
                                                     &bitmap);
    if (fScaledCacheId != NULL) {
        SkAutoLockPixels autoLockPixels(bitmap);
        void* pixels = bitmap.getPixels();
        SkASSERT(NULL != pixels);
        // At this point, the autoLockPixels will unlockPixels()
        // to remove bitmap's lock on the pixels.  We will then
        // destroy bitmap.  The *only* guarantee that this pointer
        // remains valid is the guarantee made by
        // SkScaledImageCache that it will not destroy the *other*
        // bitmap (SkScaledImageCache::Rec.fBitmap) that holds a
        // reference to the concrete PixelRef while this record is
        // locked.
        return pixels;
    } else {
        // Cache has been purged, must re-decode.
        void* pixels = decode_into_bitmap(const_cast<SkImageInfo*>(info),
                                          fDecodeProc, &fRowBytes, fData,
                                          &bitmap);
        if (NULL == pixels) {
            fErrorInDecoding = true;
            return NULL;
        }
        fScaledCacheId = SkScaledImageCache::AddAndLock(this->getGenerationID(),
                                                        info->fWidth,
                                                        info->fHeight,
                                                        bitmap);
        SkASSERT(fScaledCacheId != NULL);
        return pixels;
    }
}

void* SkLazyPixelRef::onLockPixels(SkColorTable**) {
    if (fErrorInDecoding) {
        return NULL;
    }
    if (NULL == fImageCache) {
        return this->lockScaledImageCachePixels();
    } else {
        return this->lockImageCachePixels();
    }
}

void* SkLazyPixelRef::lockImageCachePixels() {
    SkASSERT(fImageCache != NULL);
    SkASSERT(!fErrorInDecoding);
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

    SkASSERT(fData != NULL && fData->size() > 0);
    if (NULL == target.fAddr) {
        const SkImageInfo* info = this->getCachedInfo();
        if (NULL == info) {
            SkASSERT(SkImageCache::UNINITIALIZED_ID == fCacheId);
            return NULL;
        }
        size_t bytes = ComputeMinRowBytesAndSize(*info, &target.fRowBytes);
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
    fErrorInDecoding = !fDecodeProc(fData->data(), fData->size(), NULL, &target);
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
    if (NULL == fImageCache) {
        // onUnlockPixels() should never be called a second time from
        // PixelRef::Unlock() without calling onLockPixels() first.
        SkASSERT(NULL != fScaledCacheId);
        if (NULL != fScaledCacheId) {
            SkScaledImageCache::Unlock(fScaledCacheId);
            fScaledCacheId = NULL;
        }
    } else {  // use fImageCache
        SkASSERT(SkImageCache::UNINITIALIZED_ID != fCacheId);
        if (SkImageCache::UNINITIALIZED_ID != fCacheId) {
            fImageCache->releaseCache(fCacheId);
        }
    }
}

SkData* SkLazyPixelRef::onRefEncodedData() {
    fData->ref();
    return fData;
}

static bool init_from_info(SkBitmap* bm, const SkImageInfo& info,
                           size_t rowBytes) {
    SkBitmap::Config config = SkImageInfoToBitmapConfig(info);
    if (SkBitmap::kNo_Config == config) {
        return false;
    }

    return bm->setConfig(config, info.fWidth, info.fHeight, rowBytes, info.fAlphaType)
           &&
           bm->allocPixels();
}

bool SkLazyPixelRef::onImplementsDecodeInto() {
    return true;
}

bool SkLazyPixelRef::onDecodeInto(int pow2, SkBitmap* bitmap) {
    SkASSERT(fData != NULL && fData->size() > 0);
    if (fErrorInDecoding) {
        return false;
    }

    SkImageInfo info;
    // Determine the size of the image in order to determine how much memory to allocate.
    // FIXME: As an optimization, only do this part once.
    fErrorInDecoding = !fDecodeProc(fData->data(), fData->size(), &info, NULL);
    if (fErrorInDecoding) {
        return false;
    }

    SkBitmapFactory::Target target;
    (void)ComputeMinRowBytesAndSize(info, &target.fRowBytes);

    SkBitmap tmp;
    if (!init_from_info(&tmp, info, target.fRowBytes)) {
        return false;
    }

    target.fAddr = tmp.getPixels();
    fErrorInDecoding = !fDecodeProc(fData->data(), fData->size(), &info, &target);
    if (fErrorInDecoding) {
        return false;
    }

    *bitmap = tmp;
    return true;
}

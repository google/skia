/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCachingPixelRef.h"
#include "SkScaledImageCache.h"


bool SkCachingPixelRef::Install(SkImageGenerator* generator,
                                SkBitmap* dst) {
    SkImageInfo info;
    SkASSERT(generator != NULL);
    SkASSERT(dst != NULL);
    if ((NULL == generator)
        || !(generator->getInfo(&info))
        || !dst->setConfig(info, 0)) {
        SkDELETE(generator);
        return false;
    }
    SkAutoTUnref<SkCachingPixelRef> ref(SkNEW_ARGS(SkCachingPixelRef,
                                                   (generator,
                                                    info,
                                                    dst->rowBytes())));
    dst->setPixelRef(ref);
    return true;
}

SkCachingPixelRef::SkCachingPixelRef(SkImageGenerator* generator,
                                     const SkImageInfo& info,
                                     size_t rowBytes)
    : fImageGenerator(generator)
    , fErrorInDecoding(false)
    , fScaledCacheId(NULL)
    , fInfo(info)
    , fRowBytes(rowBytes) {
    SkASSERT(fImageGenerator != NULL);
}
SkCachingPixelRef::~SkCachingPixelRef() {
    SkDELETE(fImageGenerator);
    SkASSERT(NULL == fScaledCacheId);
    // Assert always unlock before unref.
}

void* SkCachingPixelRef::onLockPixels(SkColorTable** colorTable) {
    (void)colorTable;
    if (fErrorInDecoding) {
        return NULL;  // don't try again.
    }
    SkBitmap bitmap;
    SkASSERT(NULL == fScaledCacheId);
    fScaledCacheId = SkScaledImageCache::FindAndLock(this->getGenerationID(),
                                                     fInfo.fWidth,
                                                     fInfo.fHeight,
                                                     &bitmap);
    if (NULL == fScaledCacheId) {
        // Cache has been purged, must re-decode.
        if ((!bitmap.setConfig(fInfo, fRowBytes)) || !bitmap.allocPixels()) {
            fErrorInDecoding = true;
            return NULL;
        }
        SkAutoLockPixels autoLockPixels(bitmap);
        if (!fImageGenerator->getPixels(fInfo, bitmap.getPixels(), fRowBytes)) {
            fErrorInDecoding = true;
            return NULL;
        }
        fScaledCacheId = SkScaledImageCache::AddAndLock(this->getGenerationID(),
                                                        fInfo.fWidth,
                                                        fInfo.fHeight,
                                                        bitmap);
        SkASSERT(fScaledCacheId != NULL);
    }

    // Now bitmap should contain a concrete PixelRef of the decoded
    // image.
    SkAutoLockPixels autoLockPixels(bitmap);
    void* pixels = bitmap.getPixels();
    SkASSERT(pixels != NULL);
    // At this point, the autoLockPixels will unlockPixels()
    // to remove bitmap's lock on the pixels.  We will then
    // destroy bitmap.  The *only* guarantee that this pointer
    // remains valid is the guarantee made by
    // SkScaledImageCache that it will not destroy the *other*
    // bitmap (SkScaledImageCache::Rec.fBitmap) that holds a
    // reference to the concrete PixelRef while this record is
    // locked.
    return pixels;
}

void SkCachingPixelRef::onUnlockPixels() {
    if (fScaledCacheId != NULL) {
        SkScaledImageCache::Unlock(
            static_cast<SkScaledImageCache::ID*>(fScaledCacheId));
        fScaledCacheId = NULL;
    }
}


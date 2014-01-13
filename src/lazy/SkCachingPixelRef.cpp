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
    SkASSERT(dst != NULL);
    if ((NULL == generator)
        || !(generator->getInfo(&info))
        || !dst->setConfig(info, 0)) {
        SkDELETE(generator);
        return false;
    }
    SkAutoTUnref<SkCachingPixelRef> ref(SkNEW_ARGS(SkCachingPixelRef,
                                           (info, generator, dst->rowBytes())));
    dst->setPixelRef(ref);
    return true;
}

SkCachingPixelRef::SkCachingPixelRef(const SkImageInfo& info,
                                     SkImageGenerator* generator,
                                     size_t rowBytes)
    : INHERITED(info)
    , fImageGenerator(generator)
    , fErrorInDecoding(false)
    , fScaledCacheId(NULL)
    , fRowBytes(rowBytes) {
    SkASSERT(fImageGenerator != NULL);
}
SkCachingPixelRef::~SkCachingPixelRef() {
    SkDELETE(fImageGenerator);
    SkASSERT(NULL == fScaledCacheId);
    // Assert always unlock before unref.
}

bool SkCachingPixelRef::onNewLockPixels(LockRec* rec) {
    if (fErrorInDecoding) {
        return false;  // don't try again.
    }

    const SkImageInfo& info = this->info();
    SkBitmap bitmap;
    SkASSERT(NULL == fScaledCacheId);
    fScaledCacheId = SkScaledImageCache::FindAndLock(this->getGenerationID(),
                                                     info.fWidth,
                                                     info.fHeight,
                                                     &bitmap);
    if (NULL == fScaledCacheId) {
        // Cache has been purged, must re-decode.
        if ((!bitmap.setConfig(info, fRowBytes)) || !bitmap.allocPixels()) {
            fErrorInDecoding = true;
            return false;
        }
        SkAutoLockPixels autoLockPixels(bitmap);
        if (!fImageGenerator->getPixels(info, bitmap.getPixels(), fRowBytes)) {
            fErrorInDecoding = true;
            return false;
        }
        fScaledCacheId = SkScaledImageCache::AddAndLock(this->getGenerationID(),
                                                        info.fWidth,
                                                        info.fHeight,
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
    rec->fPixels = pixels;
    rec->fColorTable = NULL;
    rec->fRowBytes = bitmap.rowBytes();
    return true;
}

void SkCachingPixelRef::onUnlockPixels() {
    SkASSERT(fScaledCacheId != NULL);
    SkScaledImageCache::Unlock( static_cast<SkScaledImageCache::ID*>(fScaledCacheId));
    fScaledCacheId = NULL;
}

/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCachingPixelRef.h"
#include "SkScaledImageCache.h"

SkCachingPixelRef::SkCachingPixelRef()
    : fErrorInDecoding(false)
    , fScaledCacheId(NULL) {
    memset(&fInfo, 0xFF, sizeof(fInfo));
}
SkCachingPixelRef::~SkCachingPixelRef() {
    SkASSERT(NULL == fScaledCacheId);
    // Assert always unlock before unref.
}

bool SkCachingPixelRef::getInfo(SkImageInfo* info) {
    SkASSERT(info != NULL);
    if (fErrorInDecoding) {
        return false;  // Don't try again.
    }
    if (fInfo.fWidth < 0) {
        SkImageInfo tmp;
        if (!this->onDecodeInfo(&tmp)) {
            fErrorInDecoding = true;
            return false;
        }
        SkASSERT(tmp.fWidth >= 0);
        fInfo = tmp;
    }
    *info = fInfo;
    return true;
}

bool SkCachingPixelRef::configure(SkBitmap* bitmap) {
    SkASSERT(bitmap != NULL);
    SkImageInfo info;
    if (!this->getInfo(&info)) {
        return false;
    }
    return bitmap->setConfig(info, 0);
}

void* SkCachingPixelRef::onLockPixels(SkColorTable** colorTable) {
    (void)colorTable;
    SkImageInfo info;
    if (!this->getInfo(&info)) {
        return NULL;
    }
    SkBitmap bitmap;

    fScaledCacheId = SkScaledImageCache::FindAndLock(this->getGenerationID(),
                                                     info.fWidth,
                                                     info.fHeight,
                                                     &bitmap);
    if (NULL == fScaledCacheId) {
        // Cache has been purged, must re-decode.
        if (!this->onDecodeInto(0, &bitmap)) {
            return NULL;
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
    return pixels;
}

void SkCachingPixelRef::onUnlockPixels() {
    if (fScaledCacheId != NULL) {
        SkScaledImageCache::Unlock(
            static_cast<SkScaledImageCache::ID*>(fScaledCacheId));
        fScaledCacheId = NULL;
    }
}

bool SkCachingPixelRef::onDecodeInto(int pow2, SkBitmap* bitmap) {
    SkASSERT(bitmap != NULL);
    SkBitmap tmp;
    SkImageInfo info;
    // TODO(halcanary) - Enable SkCachingPixelRef to use a custom
    // allocator. `tmp.allocPixels(fAllocator, NULL)`
    if (!(this->configure(&tmp) && tmp.allocPixels())) {
        return false;
    }
    SkAssertResult(this->getInfo(&info));  // since configure() succeeded.
    if (!this->onDecodePixels(info, tmp.getPixels(), tmp.rowBytes())) {
        fErrorInDecoding = true;
        return false;
    }
    *bitmap = tmp;
    return true;
}

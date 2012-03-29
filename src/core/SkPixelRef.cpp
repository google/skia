
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPixelRef.h"
#include "SkFlattenable.h"
#include "SkThread.h"

SK_DECLARE_STATIC_MUTEX(gPixelRefMutex);

extern int32_t SkNextPixelRefGenerationID();
int32_t SkNextPixelRefGenerationID() {
    static int32_t  gPixelRefGenerationID;
    // do a loop in case our global wraps around, as we never want to
    // return a 0
    int32_t genID;
    do {
        genID = sk_atomic_inc(&gPixelRefGenerationID) + 1;
    } while (0 == genID);
    return genID;
}


SkPixelRef::SkPixelRef(SkBaseMutex* mutex) {
    if (NULL == mutex) {
        mutex = &gPixelRefMutex;
    }
    fMutex = mutex;
    fPixels = NULL;
    fColorTable = NULL; // we do not track ownership of this
    fLockCount = 0;
    fGenerationID = 0;  // signal to rebuild
    fIsImmutable = false;
}

SkPixelRef::SkPixelRef(SkFlattenableReadBuffer& buffer, SkBaseMutex* mutex)
        : INHERITED(buffer) {
    if (NULL == mutex) {
        mutex = &gPixelRefMutex;
    }
    fMutex = mutex;
    fPixels = NULL;
    fColorTable = NULL; // we do not track ownership of this
    fLockCount = 0;
    fGenerationID = 0;  // signal to rebuild
    fIsImmutable = buffer.readBool();
}

void SkPixelRef::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeBool(fIsImmutable);
}

void SkPixelRef::lockPixels() {
    SkAutoMutexAcquire  ac(*fMutex);

    if (1 == ++fLockCount) {
        fPixels = this->onLockPixels(&fColorTable);
    }
}

void SkPixelRef::unlockPixels() {
    SkAutoMutexAcquire  ac(*fMutex);

    SkASSERT(fLockCount > 0);
    if (0 == --fLockCount) {
        this->onUnlockPixels();
        fPixels = NULL;
        fColorTable = NULL;
    }
}

bool SkPixelRef::lockPixelsAreWritable() const {
    return this->onLockPixelsAreWritable();
}

bool SkPixelRef::onLockPixelsAreWritable() const {
    return true;
}

uint32_t SkPixelRef::getGenerationID() const {
    if (0 == fGenerationID) {
        fGenerationID = SkNextPixelRefGenerationID();
    }
    return fGenerationID;
}

void SkPixelRef::notifyPixelsChanged() {
#ifdef SK_DEBUG
    if (fIsImmutable) {
        SkDebugf("========== notifyPixelsChanged called on immutable pixelref");
    }
#endif
    // this signals us to recompute this next time around
    fGenerationID = 0;
}

void SkPixelRef::setImmutable() {
    fIsImmutable = true;
}

bool SkPixelRef::readPixels(SkBitmap* dst, const SkIRect* subset) {
    return this->onReadPixels(dst, subset);
}

bool SkPixelRef::onReadPixels(SkBitmap* dst, const SkIRect* subset) {
    return false;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_BUILD_FOR_ANDROID
void SkPixelRef::globalRef(void* data) {
    this->ref();
}

void SkPixelRef::globalUnref() {
    this->unref();
}
#endif

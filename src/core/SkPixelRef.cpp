/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPixelRef.h"
#include "SkThread.h"

#ifdef SK_USE_POSIX_THREADS

    static SkBaseMutex gPixelRefMutexRing[] = {
        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,
        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,
        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,
        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,

        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,
        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,
        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,
        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,

        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,
        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,
        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,
        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,

        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,
        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,
        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,
        SK_BASE_MUTEX_INIT, SK_BASE_MUTEX_INIT,
    };

    // must be a power-of-2. undef to just use 1 mutex
    #define PIXELREF_MUTEX_RING_COUNT SK_ARRAY_COUNT(gPixelRefMutexRing)

#else // not pthreads

    // must be a power-of-2. undef to just use 1 mutex
    #define PIXELREF_MUTEX_RING_COUNT       32
    static SkBaseMutex gPixelRefMutexRing[PIXELREF_MUTEX_RING_COUNT];

#endif

static SkBaseMutex* get_default_mutex() {
    static int32_t gPixelRefMutexRingIndex;

    SkASSERT(SkIsPow2(PIXELREF_MUTEX_RING_COUNT));

    // atomic_inc might be overkill here. It may be fine if once in a while
    // we hit a race-condition and two subsequent calls get the same index...
    int index = sk_atomic_inc(&gPixelRefMutexRingIndex);
    return &gPixelRefMutexRing[index & (PIXELREF_MUTEX_RING_COUNT - 1)];
}

///////////////////////////////////////////////////////////////////////////////

int32_t SkNextPixelRefGenerationID();

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

///////////////////////////////////////////////////////////////////////////////

void SkPixelRef::setMutex(SkBaseMutex* mutex) {
    if (NULL == mutex) {
        mutex = get_default_mutex();
    }
    fMutex = mutex;
}

// just need a > 0 value, so pick a funny one to aid in debugging
#define SKPIXELREF_PRELOCKED_LOCKCOUNT     123456789

SkPixelRef::SkPixelRef(const SkImageInfo& info) : fInfo(info) {
    SkAssertResult(SkColorTypeValidateAlphaType(fInfo.colorType(), fInfo.alphaType(),
                                                const_cast<SkAlphaType*>(&fInfo.fAlphaType)));

    this->setMutex(NULL);
    fRec.zero();
    fLockCount = 0;
    this->needsNewGenID();
    fIsImmutable = false;
    fPreLocked = false;
}


SkPixelRef::SkPixelRef(const SkImageInfo& info, SkBaseMutex* mutex) : fInfo(info) {
    SkAssertResult(SkColorTypeValidateAlphaType(fInfo.colorType(), fInfo.alphaType(),
                                                const_cast<SkAlphaType*>(&fInfo.fAlphaType)));

    this->setMutex(mutex);
    fRec.zero();
    fLockCount = 0;
    this->needsNewGenID();
    fIsImmutable = false;
    fPreLocked = false;
}

SkPixelRef::~SkPixelRef() {
    this->callGenIDChangeListeners();
}

void SkPixelRef::needsNewGenID() {
    fGenerationID = 0;
    fUniqueGenerationID = false;
}

void SkPixelRef::cloneGenID(const SkPixelRef& that) {
    // This is subtle.  We must call that.getGenerationID() to make sure its genID isn't 0.
    this->fGenerationID = that.getGenerationID();
    this->fUniqueGenerationID = false;
    that.fUniqueGenerationID = false;
}

void SkPixelRef::setPreLocked(void* pixels, size_t rowBytes, SkColorTable* ctable) {
#ifndef SK_IGNORE_PIXELREF_SETPRELOCKED
    // only call me in your constructor, otherwise fLockCount tracking can get
    // out of sync.
    fRec.fPixels = pixels;
    fRec.fColorTable = ctable;
    fRec.fRowBytes = rowBytes;
    fLockCount = SKPIXELREF_PRELOCKED_LOCKCOUNT;
    fPreLocked = true;
#endif
}

bool SkPixelRef::lockPixels(LockRec* rec) {
    SkASSERT(!fPreLocked || SKPIXELREF_PRELOCKED_LOCKCOUNT == fLockCount);

    if (!fPreLocked) {
        SkAutoMutexAcquire  ac(*fMutex);

        if (1 == ++fLockCount) {
            SkASSERT(fRec.isZero());

            LockRec rec;
            if (!this->onNewLockPixels(&rec)) {
                return false;
            }
            SkASSERT(!rec.isZero());    // else why did onNewLock return true?
            fRec = rec;
        }
    }
    *rec = fRec;
    return true;
}

bool SkPixelRef::lockPixels() {
    LockRec rec;
    return this->lockPixels(&rec);
}

void SkPixelRef::unlockPixels() {
    SkASSERT(!fPreLocked || SKPIXELREF_PRELOCKED_LOCKCOUNT == fLockCount);

    if (!fPreLocked) {
        SkAutoMutexAcquire  ac(*fMutex);

        SkASSERT(fLockCount > 0);
        if (0 == --fLockCount) {
            // don't call onUnlockPixels unless onLockPixels succeeded
            if (fRec.fPixels) {
                this->onUnlockPixels();
                fRec.zero();
            } else {
                SkASSERT(fRec.isZero());
            }
        }
    }
}

bool SkPixelRef::lockPixelsAreWritable() const {
    return this->onLockPixelsAreWritable();
}

bool SkPixelRef::onLockPixelsAreWritable() const {
    return true;
}

bool SkPixelRef::onImplementsDecodeInto() {
    return false;
}

bool SkPixelRef::onDecodeInto(int pow2, SkBitmap* bitmap) {
    return false;
}

uint32_t SkPixelRef::getGenerationID() const {
    if (0 == fGenerationID) {
        fGenerationID = SkNextPixelRefGenerationID();
        fUniqueGenerationID = true;  // The only time we can be sure of this!
    }
    return fGenerationID;
}

void SkPixelRef::addGenIDChangeListener(GenIDChangeListener* listener) {
    if (NULL == listener || !fUniqueGenerationID) {
        // No point in tracking this if we're not going to call it.
        SkDELETE(listener);
        return;
    }
    *fGenIDChangeListeners.append() = listener;
}

void SkPixelRef::callGenIDChangeListeners() {
    // We don't invalidate ourselves if we think another SkPixelRef is sharing our genID.
    if (fUniqueGenerationID) {
        for (int i = 0; i < fGenIDChangeListeners.count(); i++) {
            fGenIDChangeListeners[i]->onChange();
        }
    }
    // Listeners get at most one shot, so whether these triggered or not, blow them away.
    fGenIDChangeListeners.deleteAll();
}

void SkPixelRef::notifyPixelsChanged() {
#ifdef SK_DEBUG
    if (fIsImmutable) {
        SkDebugf("========== notifyPixelsChanged called on immutable pixelref");
    }
#endif
    this->callGenIDChangeListeners();
    this->needsNewGenID();
}

void SkPixelRef::changeAlphaType(SkAlphaType at) {
    *const_cast<SkAlphaType*>(&fInfo.fAlphaType) = at;
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

SkData* SkPixelRef::onRefEncodedData() {
    return NULL;
}

bool SkPixelRef::onGetYUV8Planes(SkISize sizes[3], void* planes[3], size_t rowBytes[3]) {
    return false;
}

size_t SkPixelRef::getAllocatedSizeInBytes() const {
    return 0;
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

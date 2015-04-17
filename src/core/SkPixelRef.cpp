/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapCache.h"
#include "SkPixelRef.h"
#include "SkThread.h"
#include "SkTraceEvent.h"

#ifdef SK_BUILD_FOR_WIN32
    // We don't have SK_BASE_MUTEX_INIT on Windows.

    // must be a power-of-2. undef to just use 1 mutex
    #define PIXELREF_MUTEX_RING_COUNT       32
    static SkBaseMutex gPixelRefMutexRing[PIXELREF_MUTEX_RING_COUNT];

#else
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

static uint32_t next_gen_id() {
    static uint32_t gNextGenID = 0;
    uint32_t genID;
    // Loop in case our global wraps around, as we never want to return a 0.
    do {
        genID = sk_atomic_fetch_add(&gNextGenID, 2u) + 2;  // Never set the low bit.
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

static SkImageInfo validate_info(const SkImageInfo& info) {
    SkAlphaType newAlphaType = info.alphaType();
    SkAssertResult(SkColorTypeValidateAlphaType(info.colorType(), info.alphaType(), &newAlphaType));
    return info.makeAlphaType(newAlphaType);
}

SkPixelRef::SkPixelRef(const SkImageInfo& info)
    : fInfo(validate_info(info))
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    , fStableID(next_gen_id())
#endif

{
    this->setMutex(NULL);
    fRec.zero();
    fLockCount = 0;
    this->needsNewGenID();
    fIsImmutable = false;
    fPreLocked = false;
    fAddedToCache.store(false);
}


SkPixelRef::SkPixelRef(const SkImageInfo& info, SkBaseMutex* mutex)
    : fInfo(validate_info(info))
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    , fStableID(next_gen_id())
#endif
{
    this->setMutex(mutex);
    fRec.zero();
    fLockCount = 0;
    this->needsNewGenID();
    fIsImmutable = false;
    fPreLocked = false;
    fAddedToCache.store(false);
}

SkPixelRef::~SkPixelRef() {
    this->callGenIDChangeListeners();
}

void SkPixelRef::needsNewGenID() {
    fTaggedGenID.store(0);
    SkASSERT(!this->genIDIsUnique()); // This method isn't threadsafe, so the assert should be fine.
}

void SkPixelRef::cloneGenID(const SkPixelRef& that) {
    // This is subtle.  We must call that.getGenerationID() to make sure its genID isn't 0.
    uint32_t genID = that.getGenerationID();

    // Neither ID is unique any more.
    // (These & ~1u are actually redundant.  that.getGenerationID() just did it for us.)
    this->fTaggedGenID.store(genID & ~1u);
    that. fTaggedGenID.store(genID & ~1u);

    // This method isn't threadsafe, so these asserts should be fine.
    SkASSERT(!this->genIDIsUnique());
    SkASSERT(!that. genIDIsUnique());
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
        TRACE_EVENT_BEGIN0("skia", "SkPixelRef::lockPixelsMutex");
        SkAutoMutexAcquire  ac(*fMutex);
        TRACE_EVENT_END0("skia", "SkPixelRef::lockPixelsMutex");

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

uint32_t SkPixelRef::getGenerationID() const {
    uint32_t id = fTaggedGenID.load();
    if (0 == id) {
        uint32_t next = next_gen_id() | 1u;
        if (fTaggedGenID.compare_exchange(&id, next)) {
            id = next;  // There was no race or we won the race.  fTaggedGenID is next now.
        } else {
            // We lost a race to set fTaggedGenID. compare_exchange() filled id with the winner.
        }
        // We can't quite SkASSERT(this->genIDIsUnique()). It could be non-unique
        // if we got here via the else path (pretty unlikely, but possible).
    }
    return id & ~1u;  // Mask off bottom unique bit.
}

void SkPixelRef::addGenIDChangeListener(GenIDChangeListener* listener) {
    if (NULL == listener || !this->genIDIsUnique()) {
        // No point in tracking this if we're not going to call it.
        SkDELETE(listener);
        return;
    }
    *fGenIDChangeListeners.append() = listener;
}

// we need to be called *before* the genID gets changed or zerod
void SkPixelRef::callGenIDChangeListeners() {
    // We don't invalidate ourselves if we think another SkPixelRef is sharing our genID.
    if (this->genIDIsUnique()) {
        for (int i = 0; i < fGenIDChangeListeners.count(); i++) {
            fGenIDChangeListeners[i]->onChange();
        }

        // TODO: SkAtomic could add "old_value = atomic.xchg(new_value)" to make this clearer.
        if (fAddedToCache.load()) {
            SkNotifyBitmapGenIDIsStale(this->getGenerationID());
            fAddedToCache.store(false);
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
    *const_cast<SkImageInfo*>(&fInfo) = fInfo.makeAlphaType(at);
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

bool SkPixelRef::onGetYUV8Planes(SkISize sizes[3], void* planes[3], size_t rowBytes[3],
                                 SkYUVColorSpace* colorSpace) {
    return false;
}

size_t SkPixelRef::getAllocatedSizeInBytes() const {
    return 0;
}


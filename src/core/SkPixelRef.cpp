/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPixelRef.h"
#include "include/private/SkMutex.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkNextID.h"
#include "src/core/SkTraceEvent.h"
#include <atomic>

uint32_t SkNextID::ImageID() {
    // We never set the low bit.... see SkPixelRef::genIDIsUnique().
    static std::atomic<uint32_t> nextID{2};

    uint32_t id;
    do {
        id = nextID.fetch_add(2);
    } while (id == 0);
    return id;
}

///////////////////////////////////////////////////////////////////////////////

SkPixelRef::SkPixelRef(int width, int height, void* pixels, size_t rowBytes)
    : fWidth(width)
    , fHeight(height)
    , fPixels(pixels)
    , fRowBytes(rowBytes)
    , fAddedToCache(false)
{
    this->needsNewGenID();
    fMutability = kMutable;
}

SkPixelRef::~SkPixelRef() {
    this->callGenIDChangeListeners();
}

// This is undefined if there are clients in-flight trying to use us
void SkPixelRef::android_only_reset(int width, int height, size_t rowBytes) {
    fWidth = width;
    fHeight = height;
    fRowBytes = rowBytes;
    // note: we do not change fPixels

    // conservative, since its possible the "new" settings are the same as the old.
    this->notifyPixelsChanged();
}

void SkPixelRef::needsNewGenID() {
    fTaggedGenID.store(0);
    SkASSERT(!this->genIDIsUnique()); // This method isn't threadsafe, so the assert should be fine.
}

uint32_t SkPixelRef::getGenerationID() const {
    uint32_t id = fTaggedGenID.load();
    if (0 == id) {
        uint32_t next = SkNextID::ImageID() | 1u;
        if (fTaggedGenID.compare_exchange_strong(id, next)) {
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
    if (nullptr == listener || !this->genIDIsUnique()) {
        // No point in tracking this if we're not going to call it.
        delete listener;
        return;
    }
    SkAutoMutexExclusive lock(fGenIDChangeListenersMutex);
    *fGenIDChangeListeners.append() = listener;
}

// we need to be called *before* the genID gets changed or zerod
void SkPixelRef::callGenIDChangeListeners() {
    SkAutoMutexExclusive lock(fGenIDChangeListenersMutex);
    // We don't invalidate ourselves if we think another SkPixelRef is sharing our genID.
    if (this->genIDIsUnique()) {
        for (int i = 0; i < fGenIDChangeListeners.count(); i++) {
            fGenIDChangeListeners[i]->onChange();
        }

        if (fAddedToCache.exchange(false)) {
            SkNotifyBitmapGenIDIsStale(this->getGenerationID());
        }
    }
    // Listeners get at most one shot, so whether these triggered or not, blow them away.
    fGenIDChangeListeners.deleteAll();
}

void SkPixelRef::notifyPixelsChanged() {
#ifdef SK_DEBUG
    if (this->isImmutable()) {
        SkDebugf("========== notifyPixelsChanged called on immutable pixelref");
    }
#endif
    this->callGenIDChangeListeners();
    this->needsNewGenID();
}

void SkPixelRef::setImmutable() {
    fMutability = kImmutable;
}

void SkPixelRef::setImmutableWithID(uint32_t genID) {
    /*
     *  We are forcing the genID to match an external value. The caller must ensure that this
     *  value does not conflict with other content.
     *
     *  One use is to force this pixelref's id to match an SkImage's id
     */
    fMutability = kImmutable;
    fTaggedGenID.store(genID);
}

void SkPixelRef::setTemporarilyImmutable() {
    SkASSERT(fMutability != kImmutable);
    fMutability = kTemporarilyImmutable;
}

void SkPixelRef::restoreMutability() {
    SkASSERT(fMutability != kImmutable);
    fMutability = kMutable;
}

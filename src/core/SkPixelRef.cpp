/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapCache.h"
#include "SkMutex.h"
#include "SkPixelRef.h"
#include "SkTraceEvent.h"

//#define SK_TRACE_PIXELREF_LIFETIME

#include "SkNextID.h"

uint32_t SkNextID::ImageID() {
    static uint32_t gID = 0;
    uint32_t id;
    // Loop in case our global wraps around, as we never want to return a 0.
    do {
        id = sk_atomic_fetch_add(&gID, 2u) + 2;  // Never set the low bit.
    } while (0 == id);
    return id;
}

///////////////////////////////////////////////////////////////////////////////

static SkImageInfo validate_info(const SkImageInfo& info) {
    SkAlphaType newAlphaType = info.alphaType();
    SkAssertResult(SkColorTypeValidateAlphaType(info.colorType(), info.alphaType(), &newAlphaType));
    return info.makeAlphaType(newAlphaType);
}

static void validate_pixels_ctable(const SkImageInfo& info, const SkColorTable* ctable) {
    if (info.isEmpty()) {
        return; // can't require ctable if the dimensions are empty
    }
    if (kIndex_8_SkColorType == info.colorType()) {
        SkASSERT(ctable);
    } else {
        SkASSERT(nullptr == ctable);
    }
}

#ifdef SK_TRACE_PIXELREF_LIFETIME
    static int32_t gInstCounter;
#endif

SkPixelRef::SkPixelRef(const SkImageInfo& info, void* pixels, size_t rowBytes,
                       sk_sp<SkColorTable> ctable)
    : fInfo(validate_info(info))
    , fCTable(std::move(ctable))
    , fPixels(pixels)
    , fRowBytes(rowBytes)
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    , fStableID(SkNextID::ImageID())
#endif
{
    validate_pixels_ctable(fInfo, fCTable.get());
    SkASSERT(rowBytes >= info.minRowBytes());
#ifdef SK_TRACE_PIXELREF_LIFETIME
    SkDebugf(" pixelref %d\n", sk_atomic_inc(&gInstCounter));
#endif

    this->needsNewGenID();
    fMutability = kMutable;
    fAddedToCache.store(false);
}

SkPixelRef::~SkPixelRef() {
#ifdef SK_TRACE_PIXELREF_LIFETIME
    SkDebugf("~pixelref %d\n", sk_atomic_dec(&gInstCounter) - 1);
#endif
    this->callGenIDChangeListeners();
}

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
// This is undefined if there are clients in-flight trying to use us
void SkPixelRef::android_only_reset(const SkImageInfo& info, size_t rowBytes,
                                    sk_sp<SkColorTable> ctable) {
    validate_pixels_ctable(info, ctable.get());

    *const_cast<SkImageInfo*>(&fInfo) = info;
    fRowBytes = rowBytes;
    fCTable = std::move(ctable);
    // note: we do not change fPixels

    // conservative, since its possible the "new" settings are the same as the old.
    this->notifyPixelsChanged();
}
#endif

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

uint32_t SkPixelRef::getGenerationID() const {
    uint32_t id = fTaggedGenID.load();
    if (0 == id) {
        uint32_t next = SkNextID::ImageID() | 1u;
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
    if (nullptr == listener || !this->genIDIsUnique()) {
        // No point in tracking this if we're not going to call it.
        delete listener;
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
    if (this->isImmutable()) {
        SkDebugf("========== notifyPixelsChanged called on immutable pixelref");
    }
#endif
    this->callGenIDChangeListeners();
    this->needsNewGenID();
    this->onNotifyPixelsChanged();
}

void SkPixelRef::changeAlphaType(SkAlphaType at) {
    *const_cast<SkImageInfo*>(&fInfo) = fInfo.makeAlphaType(at);
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

void SkPixelRef::onNotifyPixelsChanged() { }

size_t SkPixelRef::getAllocatedSizeInBytes() const {
    return 0;
}

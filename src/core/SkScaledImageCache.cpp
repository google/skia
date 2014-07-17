/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkChecksum.h"
#include "SkScaledImageCache.h"
#include "SkMipMap.h"
#include "SkPixelRef.h"
#include "SkRect.h"

// This can be defined by the caller's build system
//#define SK_USE_DISCARDABLE_SCALEDIMAGECACHE

#ifndef SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT
#   define SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT   1024
#endif

#ifndef SK_DEFAULT_IMAGE_CACHE_LIMIT
    #define SK_DEFAULT_IMAGE_CACHE_LIMIT     (2 * 1024 * 1024)
#endif

static inline SkScaledImageCache::ID* rec_to_id(SkScaledImageCache::Rec* rec) {
    return reinterpret_cast<SkScaledImageCache::ID*>(rec);
}

static inline SkScaledImageCache::Rec* id_to_rec(SkScaledImageCache::ID* id) {
    return reinterpret_cast<SkScaledImageCache::Rec*>(id);
}

struct SkScaledImageCache::Key {
    Key(uint32_t genID,
        SkScalar scaleX,
        SkScalar scaleY,
        SkIRect  bounds)
        : fGenID(genID)
        , fScaleX(scaleX)
        , fScaleY(scaleY)
        , fBounds(bounds) {
        fHash = SkChecksum::Murmur3(&fGenID, 28);
    }

    bool operator<(const Key& other) const {
        const uint32_t* a = &fGenID;
        const uint32_t* b = &other.fGenID;
        for (int i = 0; i < 7; ++i) {
            if (a[i] < b[i]) {
                return true;
            }
            if (a[i] > b[i]) {
                return false;
            }
        }
        return false;
    }

    bool operator==(const Key& other) const {
        const uint32_t* a = &fHash;
        const uint32_t* b = &other.fHash;
        for (int i = 0; i < 8; ++i) {
            if (a[i] != b[i]) {
                return false;
            }
        }
        return true;
    }

    uint32_t    fHash;
    uint32_t    fGenID;
    float       fScaleX;
    float       fScaleY;
    SkIRect     fBounds;
};

struct SkScaledImageCache::Rec {
    Rec(const Key& key, const SkBitmap& bm) : fKey(key), fBitmap(bm) {
        fLockCount = 1;
        fMip = NULL;
    }

    Rec(const Key& key, const SkMipMap* mip) : fKey(key) {
        fLockCount = 1;
        fMip = mip;
        mip->ref();
    }

    ~Rec() {
        SkSafeUnref(fMip);
    }

    static const Key& GetKey(const Rec& rec) { return rec.fKey; }
    static uint32_t Hash(const Key& key) { return key.fHash; }

    size_t bytesUsed() const {
        return fMip ? fMip->getSize() : fBitmap.getSize();
    }

    Rec*    fNext;
    Rec*    fPrev;

    // this guy wants to be 64bit aligned
    Key     fKey;

    int32_t fLockCount;

    // we use either fBitmap or fMip, but not both
    SkBitmap fBitmap;
    const SkMipMap* fMip;
};

#include "SkTDynamicHash.h"

class SkScaledImageCache::Hash :
    public SkTDynamicHash<SkScaledImageCache::Rec, SkScaledImageCache::Key> {};


///////////////////////////////////////////////////////////////////////////////

// experimental hash to speed things up
#define USE_HASH

#if !defined(USE_HASH)
static inline SkScaledImageCache::Rec* find_rec_in_list(
        SkScaledImageCache::Rec* head, const Key & key) {
    SkScaledImageCache::Rec* rec = head;
    while ((rec != NULL) && (rec->fKey != key)) {
        rec = rec->fNext;
    }
    return rec;
}
#endif

void SkScaledImageCache::init() {
    fHead = NULL;
    fTail = NULL;
#ifdef USE_HASH
    fHash = new Hash;
#else
    fHash = NULL;
#endif
    fTotalBytesUsed = 0;
    fCount = 0;
    fSingleAllocationByteLimit = 0;
    fAllocator = NULL;

    // One of these should be explicit set by the caller after we return.
    fTotalByteLimit = 0;
    fDiscardableFactory = NULL;
}

#include "SkDiscardableMemory.h"

class SkOneShotDiscardablePixelRef : public SkPixelRef {
public:
    SK_DECLARE_INST_COUNT(SkOneShotDiscardablePixelRef)
    // Ownership of the discardablememory is transfered to the pixelref
    SkOneShotDiscardablePixelRef(const SkImageInfo&, SkDiscardableMemory*, size_t rowBytes);
    ~SkOneShotDiscardablePixelRef();

protected:
    virtual bool onNewLockPixels(LockRec*) SK_OVERRIDE;
    virtual void onUnlockPixels() SK_OVERRIDE;
    virtual size_t getAllocatedSizeInBytes() const SK_OVERRIDE;

private:
    SkDiscardableMemory* fDM;
    size_t               fRB;
    bool                 fFirstTime;

    typedef SkPixelRef INHERITED;
};

SkOneShotDiscardablePixelRef::SkOneShotDiscardablePixelRef(const SkImageInfo& info,
                                             SkDiscardableMemory* dm,
                                             size_t rowBytes)
    : INHERITED(info)
    , fDM(dm)
    , fRB(rowBytes)
{
    SkASSERT(dm->data());
    fFirstTime = true;
}

SkOneShotDiscardablePixelRef::~SkOneShotDiscardablePixelRef() {
    SkDELETE(fDM);
}

bool SkOneShotDiscardablePixelRef::onNewLockPixels(LockRec* rec) {
    if (fFirstTime) {
        // we're already locked
        SkASSERT(fDM->data());
        fFirstTime = false;
        goto SUCCESS;
    }

    // A previous call to onUnlock may have deleted our DM, so check for that
    if (NULL == fDM) {
        return false;
    }

    if (!fDM->lock()) {
        // since it failed, we delete it now, to free-up the resource
        delete fDM;
        fDM = NULL;
        return false;
    }

SUCCESS:
    rec->fPixels = fDM->data();
    rec->fColorTable = NULL;
    rec->fRowBytes = fRB;
    return true;
}

void SkOneShotDiscardablePixelRef::onUnlockPixels() {
    SkASSERT(!fFirstTime);
    fDM->unlock();
}

size_t SkOneShotDiscardablePixelRef::getAllocatedSizeInBytes() const {
    return this->info().getSafeSize(fRB);
}

class SkScaledImageCacheDiscardableAllocator : public SkBitmap::Allocator {
public:
    SkScaledImageCacheDiscardableAllocator(
                            SkScaledImageCache::DiscardableFactory factory) {
        SkASSERT(factory);
        fFactory = factory;
    }

    virtual bool allocPixelRef(SkBitmap*, SkColorTable*) SK_OVERRIDE;

private:
    SkScaledImageCache::DiscardableFactory fFactory;
};

bool SkScaledImageCacheDiscardableAllocator::allocPixelRef(SkBitmap* bitmap,
                                                       SkColorTable* ctable) {
    size_t size = bitmap->getSize();
    uint64_t size64 = bitmap->computeSize64();
    if (0 == size || size64 > (uint64_t)size) {
        return false;
    }

    SkDiscardableMemory* dm = fFactory(size);
    if (NULL == dm) {
        return false;
    }

    // can we relax this?
    if (kN32_SkColorType != bitmap->colorType()) {
        return false;
    }

    SkImageInfo info = bitmap->info();
    bitmap->setPixelRef(SkNEW_ARGS(SkOneShotDiscardablePixelRef,
                                   (info, dm, bitmap->rowBytes())))->unref();
    bitmap->lockPixels();
    return bitmap->readyToDraw();
}

SkScaledImageCache::SkScaledImageCache(DiscardableFactory factory) {
    this->init();
    fDiscardableFactory = factory;

    fAllocator = SkNEW_ARGS(SkScaledImageCacheDiscardableAllocator, (factory));
}

SkScaledImageCache::SkScaledImageCache(size_t byteLimit) {
    this->init();
    fTotalByteLimit = byteLimit;
}

SkScaledImageCache::~SkScaledImageCache() {
    SkSafeUnref(fAllocator);

    Rec* rec = fHead;
    while (rec) {
        Rec* next = rec->fNext;
        SkDELETE(rec);
        rec = next;
    }
    delete fHash;
}

////////////////////////////////////////////////////////////////////////////////


SkScaledImageCache::Rec* SkScaledImageCache::findAndLock(uint32_t genID,
                                                        SkScalar scaleX,
                                                        SkScalar scaleY,
                                                        const SkIRect& bounds) {
    const Key key(genID, scaleX, scaleY, bounds);
    return this->findAndLock(key);
}

/**
   This private method is the fully general record finder. All other
   record finders should call this function or the one above. */
SkScaledImageCache::Rec* SkScaledImageCache::findAndLock(const SkScaledImageCache::Key& key) {
    if (key.fBounds.isEmpty()) {
        return NULL;
    }
#ifdef USE_HASH
    Rec* rec = fHash->find(key);
#else
    Rec* rec = find_rec_in_list(fHead, key);
#endif
    if (rec) {
        this->moveToHead(rec);  // for our LRU
        rec->fLockCount += 1;
    }
    return rec;
}

/**
   This function finds the bounds of the bitmap *within its pixelRef*.
   If the bitmap lacks a pixelRef, it will return an empty rect, since
   that doesn't make sense.  This may be a useful enough function that
   it should be somewhere else (in SkBitmap?). */
static SkIRect get_bounds_from_bitmap(const SkBitmap& bm) {
    if (!(bm.pixelRef())) {
        return SkIRect::MakeEmpty();
    }
    SkIPoint origin = bm.pixelRefOrigin();
    return SkIRect::MakeXYWH(origin.fX, origin.fY, bm.width(), bm.height());
}


SkScaledImageCache::ID* SkScaledImageCache::findAndLock(uint32_t genID,
                                                        int32_t width,
                                                        int32_t height,
                                                        SkBitmap* bitmap) {
    Rec* rec = this->findAndLock(genID, SK_Scalar1, SK_Scalar1,
                                 SkIRect::MakeWH(width, height));
    if (rec) {
        SkASSERT(NULL == rec->fMip);
        SkASSERT(rec->fBitmap.pixelRef());
        *bitmap = rec->fBitmap;
    }
    return rec_to_id(rec);
}

SkScaledImageCache::ID* SkScaledImageCache::findAndLock(const SkBitmap& orig,
                                                        SkScalar scaleX,
                                                        SkScalar scaleY,
                                                        SkBitmap* scaled) {
    if (0 == scaleX || 0 == scaleY) {
        // degenerate, and the key we use for mipmaps
        return NULL;
    }
    Rec* rec = this->findAndLock(orig.getGenerationID(), scaleX,
                                 scaleY, get_bounds_from_bitmap(orig));
    if (rec) {
        SkASSERT(NULL == rec->fMip);
        SkASSERT(rec->fBitmap.pixelRef());
        *scaled = rec->fBitmap;
    }
    return rec_to_id(rec);
}

SkScaledImageCache::ID* SkScaledImageCache::findAndLockMip(const SkBitmap& orig,
                                                           SkMipMap const ** mip) {
    Rec* rec = this->findAndLock(orig.getGenerationID(), 0, 0,
                                 get_bounds_from_bitmap(orig));
    if (rec) {
        SkASSERT(rec->fMip);
        SkASSERT(NULL == rec->fBitmap.pixelRef());
        *mip = rec->fMip;
    }
    return rec_to_id(rec);
}


////////////////////////////////////////////////////////////////////////////////
/**
   This private method is the fully general record adder. All other
   record adders should call this funtion. */
SkScaledImageCache::ID* SkScaledImageCache::addAndLock(SkScaledImageCache::Rec* rec) {
    SkASSERT(rec);
    // See if we already have this key (racy inserts, etc.)
    Rec* existing = this->findAndLock(rec->fKey);
    if (NULL != existing) {
        // Since we already have a matching entry, just delete the new one and return.
        // Call sites cannot assume the passed in object will live past this call.
        existing->fBitmap = rec->fBitmap;
        SkDELETE(rec);
        return rec_to_id(existing);
    }

    this->addToHead(rec);
    SkASSERT(1 == rec->fLockCount);
#ifdef USE_HASH
    SkASSERT(fHash);
    fHash->add(rec);
#endif
    // We may (now) be overbudget, so see if we need to purge something.
    this->purgeAsNeeded();
    return rec_to_id(rec);
}

SkScaledImageCache::ID* SkScaledImageCache::addAndLock(uint32_t genID,
                                                       int32_t width,
                                                       int32_t height,
                                                       const SkBitmap& bitmap) {
    Key key(genID, SK_Scalar1, SK_Scalar1, SkIRect::MakeWH(width, height));
    Rec* rec = SkNEW_ARGS(Rec, (key, bitmap));
    return this->addAndLock(rec);
}

SkScaledImageCache::ID* SkScaledImageCache::addAndLock(const SkBitmap& orig,
                                                       SkScalar scaleX,
                                                       SkScalar scaleY,
                                                       const SkBitmap& scaled) {
    if (0 == scaleX || 0 == scaleY) {
        // degenerate, and the key we use for mipmaps
        return NULL;
    }
    SkIRect bounds = get_bounds_from_bitmap(orig);
    if (bounds.isEmpty()) {
        return NULL;
    }
    Key key(orig.getGenerationID(), scaleX, scaleY, bounds);
    Rec* rec = SkNEW_ARGS(Rec, (key, scaled));
    return this->addAndLock(rec);
}

SkScaledImageCache::ID* SkScaledImageCache::addAndLockMip(const SkBitmap& orig,
                                                          const SkMipMap* mip) {
    SkIRect bounds = get_bounds_from_bitmap(orig);
    if (bounds.isEmpty()) {
        return NULL;
    }
    Key key(orig.getGenerationID(), 0, 0, bounds);
    Rec* rec = SkNEW_ARGS(Rec, (key, mip));
    return this->addAndLock(rec);
}

void SkScaledImageCache::unlock(SkScaledImageCache::ID* id) {
    SkASSERT(id);

#ifdef SK_DEBUG
    {
        bool found = false;
        Rec* rec = fHead;
        while (rec != NULL) {
            if (rec == id_to_rec(id)) {
                found = true;
                break;
            }
            rec = rec->fNext;
        }
        SkASSERT(found);
    }
#endif
    Rec* rec = id_to_rec(id);
    SkASSERT(rec->fLockCount > 0);
    rec->fLockCount -= 1;

    // we may have been over-budget, but now have released something, so check
    // if we should purge.
    if (0 == rec->fLockCount) {
        this->purgeAsNeeded();
    }
}

void SkScaledImageCache::purgeAsNeeded() {
    size_t byteLimit;
    int    countLimit;

    if (fDiscardableFactory) {
        countLimit = SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT;
        byteLimit = SK_MaxU32;  // no limit based on bytes
    } else {
        countLimit = SK_MaxS32; // no limit based on count
        byteLimit = fTotalByteLimit;
    }

    size_t bytesUsed = fTotalBytesUsed;
    int    countUsed = fCount;

    Rec* rec = fTail;
    while (rec) {
        if (bytesUsed < byteLimit && countUsed < countLimit) {
            break;
        }

        Rec* prev = rec->fPrev;
        if (0 == rec->fLockCount) {
            size_t used = rec->bytesUsed();
            SkASSERT(used <= bytesUsed);
            this->detach(rec);
#ifdef USE_HASH
            fHash->remove(rec->fKey);
#endif

            SkDELETE(rec);

            bytesUsed -= used;
            countUsed -= 1;
        }
        rec = prev;
    }

    fTotalBytesUsed = bytesUsed;
    fCount = countUsed;
}

size_t SkScaledImageCache::setTotalByteLimit(size_t newLimit) {
    size_t prevLimit = fTotalByteLimit;
    fTotalByteLimit = newLimit;
    if (newLimit < prevLimit) {
        this->purgeAsNeeded();
    }
    return prevLimit;
}

///////////////////////////////////////////////////////////////////////////////

void SkScaledImageCache::detach(Rec* rec) {
    Rec* prev = rec->fPrev;
    Rec* next = rec->fNext;

    if (!prev) {
        SkASSERT(fHead == rec);
        fHead = next;
    } else {
        prev->fNext = next;
    }

    if (!next) {
        fTail = prev;
    } else {
        next->fPrev = prev;
    }

    rec->fNext = rec->fPrev = NULL;
}

void SkScaledImageCache::moveToHead(Rec* rec) {
    if (fHead == rec) {
        return;
    }

    SkASSERT(fHead);
    SkASSERT(fTail);

    this->validate();

    this->detach(rec);

    fHead->fPrev = rec;
    rec->fNext = fHead;
    fHead = rec;

    this->validate();
}

void SkScaledImageCache::addToHead(Rec* rec) {
    this->validate();

    rec->fPrev = NULL;
    rec->fNext = fHead;
    if (fHead) {
        fHead->fPrev = rec;
    }
    fHead = rec;
    if (!fTail) {
        fTail = rec;
    }
    fTotalBytesUsed += rec->bytesUsed();
    fCount += 1;

    this->validate();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void SkScaledImageCache::validate() const {
    if (NULL == fHead) {
        SkASSERT(NULL == fTail);
        SkASSERT(0 == fTotalBytesUsed);
        return;
    }

    if (fHead == fTail) {
        SkASSERT(NULL == fHead->fPrev);
        SkASSERT(NULL == fHead->fNext);
        SkASSERT(fHead->bytesUsed() == fTotalBytesUsed);
        return;
    }

    SkASSERT(NULL == fHead->fPrev);
    SkASSERT(NULL != fHead->fNext);
    SkASSERT(NULL == fTail->fNext);
    SkASSERT(NULL != fTail->fPrev);

    size_t used = 0;
    int count = 0;
    const Rec* rec = fHead;
    while (rec) {
        count += 1;
        used += rec->bytesUsed();
        SkASSERT(used <= fTotalBytesUsed);
        rec = rec->fNext;
    }
    SkASSERT(fCount == count);

    rec = fTail;
    while (rec) {
        SkASSERT(count > 0);
        count -= 1;
        SkASSERT(used >= rec->bytesUsed());
        used -= rec->bytesUsed();
        rec = rec->fPrev;
    }

    SkASSERT(0 == count);
    SkASSERT(0 == used);
}
#endif

void SkScaledImageCache::dump() const {
    this->validate();

    const Rec* rec = fHead;
    int locked = 0;
    while (rec) {
        locked += rec->fLockCount > 0;
        rec = rec->fNext;
    }

    SkDebugf("SkScaledImageCache: count=%d bytes=%d locked=%d %s\n",
             fCount, fTotalBytesUsed, locked,
             fDiscardableFactory ? "discardable" : "malloc");
}

size_t SkScaledImageCache::setSingleAllocationByteLimit(size_t newLimit) {
    size_t oldLimit = fSingleAllocationByteLimit;
    fSingleAllocationByteLimit = newLimit;
    return oldLimit;
}

size_t SkScaledImageCache::getSingleAllocationByteLimit() const {
    return fSingleAllocationByteLimit;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkThread.h"

SK_DECLARE_STATIC_MUTEX(gMutex);
static SkScaledImageCache* gScaledImageCache = NULL;
static void cleanup_gScaledImageCache() {
    // We'll clean this up in our own tests, but disable for clients.
    // Chrome seems to have funky multi-process things going on in unit tests that
    // makes this unsafe to delete when the main process atexit()s.
    // SkLazyPtr does the same sort of thing.
#if SK_DEVELOPER
    SkDELETE(gScaledImageCache);
#endif
}

/** Must hold gMutex when calling. */
static SkScaledImageCache* get_cache() {
    // gMutex is always held when this is called, so we don't need to be fancy in here.
    gMutex.assertHeld();
    if (NULL == gScaledImageCache) {
#ifdef SK_USE_DISCARDABLE_SCALEDIMAGECACHE
        gScaledImageCache = SkNEW_ARGS(SkScaledImageCache, (SkDiscardableMemory::Create));
#else
        gScaledImageCache = SkNEW_ARGS(SkScaledImageCache, (SK_DEFAULT_IMAGE_CACHE_LIMIT));
#endif
        atexit(cleanup_gScaledImageCache);
    }
    return gScaledImageCache;
}


SkScaledImageCache::ID* SkScaledImageCache::FindAndLock(
                                uint32_t pixelGenerationID,
                                int32_t width,
                                int32_t height,
                                SkBitmap* scaled) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->findAndLock(pixelGenerationID, width, height, scaled);
}

SkScaledImageCache::ID* SkScaledImageCache::AddAndLock(
                               uint32_t pixelGenerationID,
                               int32_t width,
                               int32_t height,
                               const SkBitmap& scaled) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->addAndLock(pixelGenerationID, width, height, scaled);
}


SkScaledImageCache::ID* SkScaledImageCache::FindAndLock(const SkBitmap& orig,
                                                        SkScalar scaleX,
                                                        SkScalar scaleY,
                                                        SkBitmap* scaled) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->findAndLock(orig, scaleX, scaleY, scaled);
}

SkScaledImageCache::ID* SkScaledImageCache::FindAndLockMip(const SkBitmap& orig,
                                                       SkMipMap const ** mip) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->findAndLockMip(orig, mip);
}

SkScaledImageCache::ID* SkScaledImageCache::AddAndLock(const SkBitmap& orig,
                                                       SkScalar scaleX,
                                                       SkScalar scaleY,
                                                       const SkBitmap& scaled) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->addAndLock(orig, scaleX, scaleY, scaled);
}

SkScaledImageCache::ID* SkScaledImageCache::AddAndLockMip(const SkBitmap& orig,
                                                          const SkMipMap* mip) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->addAndLockMip(orig, mip);
}

void SkScaledImageCache::Unlock(SkScaledImageCache::ID* id) {
    SkAutoMutexAcquire am(gMutex);
    get_cache()->unlock(id);

//    get_cache()->dump();
}

size_t SkScaledImageCache::GetTotalBytesUsed() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->getTotalBytesUsed();
}

size_t SkScaledImageCache::GetTotalByteLimit() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->getTotalByteLimit();
}

size_t SkScaledImageCache::SetTotalByteLimit(size_t newLimit) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->setTotalByteLimit(newLimit);
}

SkBitmap::Allocator* SkScaledImageCache::GetAllocator() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->allocator();
}

void SkScaledImageCache::Dump() {
    SkAutoMutexAcquire am(gMutex);
    get_cache()->dump();
}

size_t SkScaledImageCache::SetSingleAllocationByteLimit(size_t size) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->setSingleAllocationByteLimit(size);
}

size_t SkScaledImageCache::GetSingleAllocationByteLimit() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->getSingleAllocationByteLimit();
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGraphics.h"

size_t SkGraphics::GetImageCacheTotalBytesUsed() {
    return SkScaledImageCache::GetTotalBytesUsed();
}

size_t SkGraphics::GetImageCacheTotalByteLimit() {
    return SkScaledImageCache::GetTotalByteLimit();
}

size_t SkGraphics::SetImageCacheTotalByteLimit(size_t newLimit) {
    return SkScaledImageCache::SetTotalByteLimit(newLimit);
}

size_t SkGraphics::GetImageCacheSingleAllocationByteLimit() {
    return SkScaledImageCache::GetSingleAllocationByteLimit();
}

size_t SkGraphics::SetImageCacheSingleAllocationByteLimit(size_t newLimit) {
    return SkScaledImageCache::SetSingleAllocationByteLimit(newLimit);
}


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

// This can be defined by the caller's build system
//#define SK_USE_DISCARDABLE_SCALEDIMAGECACHE

#ifndef SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT
#   define SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT   1024
#endif

#ifndef SK_DEFAULT_IMAGE_CACHE_LIMIT
    #define SK_DEFAULT_IMAGE_CACHE_LIMIT     (2 * 1024 * 1024)
#endif

void SkScaledImageCache::Key::init(size_t length) {
    SkASSERT(SkAlign4(length) == length);
    // 2 is fCount32 and fHash
    fCount32 = SkToS32(2 + (length >> 2));
    // skip both of our fields whe computing the murmur
    fHash = SkChecksum::Murmur3(this->as32() + 2, (fCount32 - 2) << 2);
}

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

const SkScaledImageCache::Rec* SkScaledImageCache::findAndLock(const Key& key) {
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

const SkScaledImageCache::Rec* SkScaledImageCache::addAndLock(Rec* rec) {
    SkASSERT(rec);
    // See if we already have this key (racy inserts, etc.)
    const Rec* existing = this->findAndLock(rec->getKey());
    if (NULL != existing) {
        SkDELETE(rec);
        return existing;
    }

    this->addToHead(rec);
    SkASSERT(1 == rec->fLockCount);
#ifdef USE_HASH
    SkASSERT(fHash);
    fHash->add(rec);
#endif
    // We may (now) be overbudget, so see if we need to purge something.
    this->purgeAsNeeded();
    return rec;
}

void SkScaledImageCache::add(Rec* rec) {
    SkASSERT(rec);
    // See if we already have this key (racy inserts, etc.)
    const Rec* existing = this->findAndLock(rec->getKey());
    if (NULL != existing) {
        SkDELETE(rec);
        this->unlock(existing);
        return;
    }
    
    this->addToHead(rec);
    SkASSERT(1 == rec->fLockCount);
#ifdef USE_HASH
    SkASSERT(fHash);
    fHash->add(rec);
#endif
    this->unlock(rec);
}

void SkScaledImageCache::unlock(SkScaledImageCache::ID id) {
    SkASSERT(id);

#ifdef SK_DEBUG
    {
        bool found = false;
        Rec* rec = fHead;
        while (rec != NULL) {
            if (rec == id) {
                found = true;
                break;
            }
            rec = rec->fNext;
        }
        SkASSERT(found);
    }
#endif
    const Rec* rec = id;
    SkASSERT(rec->fLockCount > 0);
    // We're under our lock, and we're the only possible mutator, so unconsting is fine.
    const_cast<Rec*>(rec)->fLockCount -= 1;

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
            fHash->remove(rec->getKey());
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

void SkScaledImageCache::Unlock(SkScaledImageCache::ID id) {
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

const SkScaledImageCache::Rec* SkScaledImageCache::FindAndLock(const Key& key) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->findAndLock(key);
}

const SkScaledImageCache::Rec* SkScaledImageCache::AddAndLock(Rec* rec) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->addAndLock(rec);
}

void SkScaledImageCache::Add(Rec* rec) {
    SkAutoMutexAcquire am(gMutex);
    get_cache()->add(rec);
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


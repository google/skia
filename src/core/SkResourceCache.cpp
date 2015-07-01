/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkChecksum.h"
#include "SkMessageBus.h"
#include "SkMipMap.h"
#include "SkMutex.h"
#include "SkPixelRef.h"
#include "SkResourceCache.h"

#include <stddef.h>

DECLARE_SKMESSAGEBUS_MESSAGE(SkResourceCache::PurgeSharedIDMessage)

// This can be defined by the caller's build system
//#define SK_USE_DISCARDABLE_SCALEDIMAGECACHE

#ifndef SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT
#   define SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT   1024
#endif

#ifndef SK_DEFAULT_IMAGE_CACHE_LIMIT
    #define SK_DEFAULT_IMAGE_CACHE_LIMIT     (2 * 1024 * 1024)
#endif

void SkResourceCache::Key::init(void* nameSpace, uint64_t sharedID, size_t length) {
    SkASSERT(SkAlign4(length) == length);

    // fCount32 and fHash are not hashed
    static const int kUnhashedLocal32s = 2; // fCache32 + fHash
    static const int kSharedIDLocal32s = 2; // fSharedID_lo + fSharedID_hi
    static const int kHashedLocal32s = kSharedIDLocal32s + (sizeof(fNamespace) >> 2);
    static const int kLocal32s = kUnhashedLocal32s + kHashedLocal32s;

    SK_COMPILE_ASSERT(sizeof(Key) == (kLocal32s << 2), unaccounted_key_locals);
    SK_COMPILE_ASSERT(sizeof(Key) == offsetof(Key, fNamespace) + sizeof(fNamespace),
                      namespace_field_must_be_last);

    fCount32 = SkToS32(kLocal32s + (length >> 2));
    fSharedID_lo = (uint32_t)sharedID;
    fSharedID_hi = (uint32_t)(sharedID >> 32);
    fNamespace = nameSpace;
    // skip unhashed fields when computing the murmur
    fHash = SkChecksum::Murmur3(this->as32() + kUnhashedLocal32s,
                                (fCount32 - kUnhashedLocal32s) << 2);
}

#include "SkTDynamicHash.h"

class SkResourceCache::Hash :
    public SkTDynamicHash<SkResourceCache::Rec, SkResourceCache::Key> {};


///////////////////////////////////////////////////////////////////////////////

void SkResourceCache::init() {
    fHead = NULL;
    fTail = NULL;
    fHash = new Hash;
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

    // Ownership of the discardablememory is transfered to the pixelref
    SkOneShotDiscardablePixelRef(const SkImageInfo&, SkDiscardableMemory*, size_t rowBytes);
    ~SkOneShotDiscardablePixelRef();

protected:
    bool onNewLockPixels(LockRec*) override;
    void onUnlockPixels() override;
    size_t getAllocatedSizeInBytes() const override;

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

class SkResourceCacheDiscardableAllocator : public SkBitmap::Allocator {
public:
    SkResourceCacheDiscardableAllocator(SkResourceCache::DiscardableFactory factory) {
        SkASSERT(factory);
        fFactory = factory;
    }

    bool allocPixelRef(SkBitmap*, SkColorTable*) override;

private:
    SkResourceCache::DiscardableFactory fFactory;
};

bool SkResourceCacheDiscardableAllocator::allocPixelRef(SkBitmap* bitmap, SkColorTable* ctable) {
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

SkResourceCache::SkResourceCache(DiscardableFactory factory) {
    this->init();
    fDiscardableFactory = factory;

    fAllocator = SkNEW_ARGS(SkResourceCacheDiscardableAllocator, (factory));
}

SkResourceCache::SkResourceCache(size_t byteLimit) {
    this->init();
    fTotalByteLimit = byteLimit;
}

SkResourceCache::~SkResourceCache() {
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

bool SkResourceCache::find(const Key& key, FindVisitor visitor, void* context) {
    this->checkMessages();

    Rec* rec = fHash->find(key);
    if (rec) {
        if (visitor(*rec, context)) {
            this->moveToHead(rec);  // for our LRU
            return true;
        } else {
            this->remove(rec);  // stale
            return false;
        }
    }
    return false;
}

static void make_size_str(size_t size, SkString* str) {
    const char suffix[] = { 'b', 'k', 'm', 'g', 't', 0 };
    int i = 0;
    while (suffix[i] && (size > 1024)) {
        i += 1;
        size >>= 10;
    }
    str->printf("%zu%c", size, suffix[i]);
}

static bool gDumpCacheTransactions;

void SkResourceCache::add(Rec* rec) {
    this->checkMessages();

    SkASSERT(rec);
    // See if we already have this key (racy inserts, etc.)
    Rec* existing = fHash->find(rec->getKey());
    if (existing) {
        SkDELETE(rec);
        return;
    }

    this->addToHead(rec);
    fHash->add(rec);

    if (gDumpCacheTransactions) {
        SkString bytesStr, totalStr;
        make_size_str(rec->bytesUsed(), &bytesStr);
        make_size_str(fTotalBytesUsed, &totalStr);
        SkDebugf("RC:    add %5s %12p key %08x -- total %5s, count %d\n",
                 bytesStr.c_str(), rec, rec->getHash(), totalStr.c_str(), fCount);
    }

    // since the new rec may push us over-budget, we perform a purge check now
    this->purgeAsNeeded();
}

void SkResourceCache::remove(Rec* rec) {
    size_t used = rec->bytesUsed();
    SkASSERT(used <= fTotalBytesUsed);

    this->detach(rec);
    fHash->remove(rec->getKey());

    fTotalBytesUsed -= used;
    fCount -= 1;

    if (gDumpCacheTransactions) {
        SkString bytesStr, totalStr;
        make_size_str(used, &bytesStr);
        make_size_str(fTotalBytesUsed, &totalStr);
        SkDebugf("RC: remove %5s %12p key %08x -- total %5s, count %d\n",
                 bytesStr.c_str(), rec, rec->getHash(), totalStr.c_str(), fCount);
    }

    SkDELETE(rec);
}

void SkResourceCache::purgeAsNeeded(bool forcePurge) {
    size_t byteLimit;
    int    countLimit;

    if (fDiscardableFactory) {
        countLimit = SK_DISCARDABLEMEMORY_SCALEDIMAGECACHE_COUNT_LIMIT;
        byteLimit = SK_MaxU32;  // no limit based on bytes
    } else {
        countLimit = SK_MaxS32; // no limit based on count
        byteLimit = fTotalByteLimit;
    }

    Rec* rec = fTail;
    while (rec) {
        if (!forcePurge && fTotalBytesUsed < byteLimit && fCount < countLimit) {
            break;
        }

        Rec* prev = rec->fPrev;
        this->remove(rec);
        rec = prev;
    }
}

//#define SK_TRACK_PURGE_SHAREDID_HITRATE

#ifdef SK_TRACK_PURGE_SHAREDID_HITRATE
static int gPurgeCallCounter;
static int gPurgeHitCounter;
#endif

void SkResourceCache::purgeSharedID(uint64_t sharedID) {
    if (0 == sharedID) {
        return;
    }

#ifdef SK_TRACK_PURGE_SHAREDID_HITRATE
    gPurgeCallCounter += 1;
    bool found = false;
#endif
    // go backwards, just like purgeAsNeeded, just to make the code similar.
    // could iterate either direction and still be correct.
    Rec* rec = fTail;
    while (rec) {
        Rec* prev = rec->fPrev;
        if (rec->getKey().getSharedID() == sharedID) {
//            SkDebugf("purgeSharedID id=%llx rec=%p\n", sharedID, rec);
            this->remove(rec);
#ifdef SK_TRACK_PURGE_SHAREDID_HITRATE
            found = true;
#endif
        }
        rec = prev;
    }

#ifdef SK_TRACK_PURGE_SHAREDID_HITRATE
    if (found) {
        gPurgeHitCounter += 1;
    }

    SkDebugf("PurgeShared calls=%d hits=%d rate=%g\n", gPurgeCallCounter, gPurgeHitCounter,
             gPurgeHitCounter * 100.0 / gPurgeCallCounter);
#endif
}

size_t SkResourceCache::setTotalByteLimit(size_t newLimit) {
    size_t prevLimit = fTotalByteLimit;
    fTotalByteLimit = newLimit;
    if (newLimit < prevLimit) {
        this->purgeAsNeeded();
    }
    return prevLimit;
}

SkCachedData* SkResourceCache::newCachedData(size_t bytes) {
    this->checkMessages();

    if (fDiscardableFactory) {
        SkDiscardableMemory* dm = fDiscardableFactory(bytes);
        return dm ? SkNEW_ARGS(SkCachedData, (bytes, dm)) : NULL;
    } else {
        return SkNEW_ARGS(SkCachedData, (sk_malloc_throw(bytes), bytes));
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkResourceCache::detach(Rec* rec) {
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

void SkResourceCache::moveToHead(Rec* rec) {
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

void SkResourceCache::addToHead(Rec* rec) {
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
void SkResourceCache::validate() const {
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
    SkASSERT(fHead->fNext);
    SkASSERT(NULL == fTail->fNext);
    SkASSERT(fTail->fPrev);

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

void SkResourceCache::dump() const {
    this->validate();

    SkDebugf("SkResourceCache: count=%d bytes=%d %s\n",
             fCount, fTotalBytesUsed, fDiscardableFactory ? "discardable" : "malloc");
}

size_t SkResourceCache::setSingleAllocationByteLimit(size_t newLimit) {
    size_t oldLimit = fSingleAllocationByteLimit;
    fSingleAllocationByteLimit = newLimit;
    return oldLimit;
}

size_t SkResourceCache::getSingleAllocationByteLimit() const {
    return fSingleAllocationByteLimit;
}

size_t SkResourceCache::getEffectiveSingleAllocationByteLimit() const {
    // fSingleAllocationByteLimit == 0 means the caller is asking for our default
    size_t limit = fSingleAllocationByteLimit;

    // if we're not discardable (i.e. we are fixed-budget) then cap the single-limit
    // to our budget.
    if (NULL == fDiscardableFactory) {
        if (0 == limit) {
            limit = fTotalByteLimit;
        } else {
            limit = SkTMin(limit, fTotalByteLimit);
        }
    }
    return limit;
}

void SkResourceCache::checkMessages() {
    SkTArray<PurgeSharedIDMessage> msgs;
    fPurgeSharedIDInbox.poll(&msgs);
    for (int i = 0; i < msgs.count(); ++i) {
        this->purgeSharedID(msgs[i].fSharedID);
    }
}

///////////////////////////////////////////////////////////////////////////////

SK_DECLARE_STATIC_MUTEX(gMutex);
static SkResourceCache* gResourceCache = NULL;
static void cleanup_gResourceCache() {
    // We'll clean this up in our own tests, but disable for clients.
    // Chrome seems to have funky multi-process things going on in unit tests that
    // makes this unsafe to delete when the main process atexit()s.
    // SkLazyPtr does the same sort of thing.
#if SK_DEVELOPER
    SkDELETE(gResourceCache);
#endif
}

/** Must hold gMutex when calling. */
static SkResourceCache* get_cache() {
    // gMutex is always held when this is called, so we don't need to be fancy in here.
    gMutex.assertHeld();
    if (NULL == gResourceCache) {
#ifdef SK_USE_DISCARDABLE_SCALEDIMAGECACHE
        gResourceCache = SkNEW_ARGS(SkResourceCache, (SkDiscardableMemory::Create));
#else
        gResourceCache = SkNEW_ARGS(SkResourceCache, (SK_DEFAULT_IMAGE_CACHE_LIMIT));
#endif
        atexit(cleanup_gResourceCache);
    }
    return gResourceCache;
}

size_t SkResourceCache::GetTotalBytesUsed() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->getTotalBytesUsed();
}

size_t SkResourceCache::GetTotalByteLimit() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->getTotalByteLimit();
}

size_t SkResourceCache::SetTotalByteLimit(size_t newLimit) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->setTotalByteLimit(newLimit);
}

SkResourceCache::DiscardableFactory SkResourceCache::GetDiscardableFactory() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->discardableFactory();
}

SkBitmap::Allocator* SkResourceCache::GetAllocator() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->allocator();
}

SkCachedData* SkResourceCache::NewCachedData(size_t bytes) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->newCachedData(bytes);
}

void SkResourceCache::Dump() {
    SkAutoMutexAcquire am(gMutex);
    get_cache()->dump();
}

size_t SkResourceCache::SetSingleAllocationByteLimit(size_t size) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->setSingleAllocationByteLimit(size);
}

size_t SkResourceCache::GetSingleAllocationByteLimit() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->getSingleAllocationByteLimit();
}

size_t SkResourceCache::GetEffectiveSingleAllocationByteLimit() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->getEffectiveSingleAllocationByteLimit();
}

void SkResourceCache::PurgeAll() {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->purgeAll();
}

bool SkResourceCache::Find(const Key& key, FindVisitor visitor, void* context) {
    SkAutoMutexAcquire am(gMutex);
    return get_cache()->find(key, visitor, context);
}

void SkResourceCache::Add(Rec* rec) {
    SkAutoMutexAcquire am(gMutex);
    get_cache()->add(rec);
}

void SkResourceCache::PostPurgeSharedID(uint64_t sharedID) {
    if (sharedID) {
        SkMessageBus<PurgeSharedIDMessage>::Post(PurgeSharedIDMessage(sharedID));
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGraphics.h"
#include "SkImageFilter.h"

size_t SkGraphics::GetResourceCacheTotalBytesUsed() {
    return SkResourceCache::GetTotalBytesUsed();
}

size_t SkGraphics::GetResourceCacheTotalByteLimit() {
    return SkResourceCache::GetTotalByteLimit();
}

size_t SkGraphics::SetResourceCacheTotalByteLimit(size_t newLimit) {
    return SkResourceCache::SetTotalByteLimit(newLimit);
}

size_t SkGraphics::GetResourceCacheSingleAllocationByteLimit() {
    return SkResourceCache::GetSingleAllocationByteLimit();
}

size_t SkGraphics::SetResourceCacheSingleAllocationByteLimit(size_t newLimit) {
    return SkResourceCache::SetSingleAllocationByteLimit(newLimit);
}

void SkGraphics::PurgeResourceCache() {
    SkImageFilter::PurgeCache();
    return SkResourceCache::PurgeAll();
}


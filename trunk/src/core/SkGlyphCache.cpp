
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkGlyphCache.h"
#include "SkGraphics.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkTemplates.h"
#include "SkTLS.h"

//#define SPEW_PURGE_STATUS
//#define USE_CACHE_HASH
//#define RECORD_HASH_EFFICIENCY

bool gSkSuppressFontCachePurgeSpew;

///////////////////////////////////////////////////////////////////////////////

#ifdef RECORD_HASH_EFFICIENCY
    static uint32_t gHashSuccess;
    static uint32_t gHashCollision;

    static void RecordHashSuccess() {
        gHashSuccess += 1;
    }

    static void RecordHashCollisionIf(bool pred) {
        if (pred) {
            gHashCollision += 1;

            uint32_t total = gHashSuccess + gHashCollision;
            SkDebugf("Font Cache Hash success rate: %d%%\n",
                     100 * gHashSuccess / total);
        }
    }
#else
    #define RecordHashSuccess() (void)0
    #define RecordHashCollisionIf(pred) (void)0
#endif
#define RecordHashCollision() RecordHashCollisionIf(true)

///////////////////////////////////////////////////////////////////////////////

#define kMinGlphAlloc       (sizeof(SkGlyph) * 64)
#define kMinImageAlloc      (24 * 64)   // should be pointsize-dependent

#define METRICS_RESERVE_COUNT  128  // so we don't grow this array a lot

SkGlyphCache::SkGlyphCache(const SkDescriptor* desc)
        : fGlyphAlloc(kMinGlphAlloc), fImageAlloc(kMinImageAlloc) {
    fPrev = fNext = NULL;

    fDesc = desc->copy();
    fScalerContext = SkScalerContext::Create(desc);
    fScalerContext->getFontMetrics(NULL, &fFontMetricsY);

    // init to 0 so that all of the pointers will be null
    memset(fGlyphHash, 0, sizeof(fGlyphHash));
    // init with 0xFF so that the charCode field will be -1, which is invalid
    memset(fCharToGlyphHash, 0xFF, sizeof(fCharToGlyphHash));

    fMemoryUsed = sizeof(*this) + kMinGlphAlloc + kMinImageAlloc;

    fGlyphArray.setReserve(METRICS_RESERVE_COUNT);

    fMetricsCount = 0;
    fAdvanceCount = 0;
    fAuxProcList = NULL;
}

SkGlyphCache::~SkGlyphCache() {
    SkGlyph**   gptr = fGlyphArray.begin();
    SkGlyph**   stop = fGlyphArray.end();
    while (gptr < stop) {
        SkPath* path = (*gptr)->fPath;
        if (path) {
            SkDELETE(path);
        }
        gptr += 1;
    }
    SkDescriptor::Free(fDesc);
    SkDELETE(fScalerContext);
    this->invokeAndRemoveAuxProcs();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
#define VALIDATE()  AutoValidate av(this)
#else
#define VALIDATE()
#endif

uint16_t SkGlyphCache::unicharToGlyph(SkUnichar charCode) {
    VALIDATE();
    uint32_t id = SkGlyph::MakeID(charCode);
    const CharGlyphRec& rec = fCharToGlyphHash[ID2HashIndex(id)];

    if (rec.fID == id) {
        return rec.fGlyph->getGlyphID();
    } else {
        return fScalerContext->charToGlyphID(charCode);
    }
}

SkUnichar SkGlyphCache::glyphToUnichar(uint16_t glyphID) {
    return fScalerContext->glyphIDToChar(glyphID);
}

unsigned SkGlyphCache::getGlyphCount() {
    return fScalerContext->getGlyphCount();
}

///////////////////////////////////////////////////////////////////////////////

const SkGlyph& SkGlyphCache::getUnicharAdvance(SkUnichar charCode) {
    VALIDATE();
    uint32_t id = SkGlyph::MakeID(charCode);
    CharGlyphRec* rec = &fCharToGlyphHash[ID2HashIndex(id)];

    if (rec->fID != id) {
        // this ID is based on the UniChar
        rec->fID = id;
        // this ID is based on the glyph index
        id = SkGlyph::MakeID(fScalerContext->charToGlyphID(charCode));
        rec->fGlyph = this->lookupMetrics(id, kJustAdvance_MetricsType);
    }
    return *rec->fGlyph;
}

const SkGlyph& SkGlyphCache::getGlyphIDAdvance(uint16_t glyphID) {
    VALIDATE();
    uint32_t id = SkGlyph::MakeID(glyphID);
    unsigned index = ID2HashIndex(id);
    SkGlyph* glyph = fGlyphHash[index];

    if (NULL == glyph || glyph->fID != id) {
        glyph = this->lookupMetrics(glyphID, kJustAdvance_MetricsType);
        fGlyphHash[index] = glyph;
    }
    return *glyph;
}

///////////////////////////////////////////////////////////////////////////////

const SkGlyph& SkGlyphCache::getUnicharMetrics(SkUnichar charCode) {
    VALIDATE();
    uint32_t id = SkGlyph::MakeID(charCode);
    CharGlyphRec* rec = &fCharToGlyphHash[ID2HashIndex(id)];

    if (rec->fID != id) {
        RecordHashCollisionIf(rec->fGlyph != NULL);
        // this ID is based on the UniChar
        rec->fID = id;
        // this ID is based on the glyph index
        id = SkGlyph::MakeID(fScalerContext->charToGlyphID(charCode));
        rec->fGlyph = this->lookupMetrics(id, kFull_MetricsType);
    } else {
        RecordHashSuccess();
        if (rec->fGlyph->isJustAdvance()) {
            fScalerContext->getMetrics(rec->fGlyph);
        }
    }
    SkASSERT(rec->fGlyph->isFullMetrics());
    return *rec->fGlyph;
}

const SkGlyph& SkGlyphCache::getUnicharMetrics(SkUnichar charCode,
                                               SkFixed x, SkFixed y) {
    VALIDATE();
    uint32_t id = SkGlyph::MakeID(charCode, x, y);
    CharGlyphRec* rec = &fCharToGlyphHash[ID2HashIndex(id)];

    if (rec->fID != id) {
        RecordHashCollisionIf(rec->fGlyph != NULL);
        // this ID is based on the UniChar
        rec->fID = id;
        // this ID is based on the glyph index
        id = SkGlyph::MakeID(fScalerContext->charToGlyphID(charCode), x, y);
        rec->fGlyph = this->lookupMetrics(id, kFull_MetricsType);
    } else {
        RecordHashSuccess();
        if (rec->fGlyph->isJustAdvance()) {
            fScalerContext->getMetrics(rec->fGlyph);
        }
    }
    SkASSERT(rec->fGlyph->isFullMetrics());
    return *rec->fGlyph;
}

const SkGlyph& SkGlyphCache::getGlyphIDMetrics(uint16_t glyphID) {
    VALIDATE();
    uint32_t id = SkGlyph::MakeID(glyphID);
    unsigned index = ID2HashIndex(id);
    SkGlyph* glyph = fGlyphHash[index];

    if (NULL == glyph || glyph->fID != id) {
        RecordHashCollisionIf(glyph != NULL);
        glyph = this->lookupMetrics(glyphID, kFull_MetricsType);
        fGlyphHash[index] = glyph;
    } else {
        RecordHashSuccess();
        if (glyph->isJustAdvance()) {
            fScalerContext->getMetrics(glyph);
        }
    }
    SkASSERT(glyph->isFullMetrics());
    return *glyph;
}

const SkGlyph& SkGlyphCache::getGlyphIDMetrics(uint16_t glyphID,
                                               SkFixed x, SkFixed y) {
    VALIDATE();
    uint32_t id = SkGlyph::MakeID(glyphID, x, y);
    unsigned index = ID2HashIndex(id);
    SkGlyph* glyph = fGlyphHash[index];

    if (NULL == glyph || glyph->fID != id) {
        RecordHashCollisionIf(glyph != NULL);
        glyph = this->lookupMetrics(id, kFull_MetricsType);
        fGlyphHash[index] = glyph;
    } else {
        RecordHashSuccess();
        if (glyph->isJustAdvance()) {
            fScalerContext->getMetrics(glyph);
        }
    }
    SkASSERT(glyph->isFullMetrics());
    return *glyph;
}

SkGlyph* SkGlyphCache::lookupMetrics(uint32_t id, MetricsType mtype) {
    SkGlyph* glyph;

    int     hi = 0;
    int     count = fGlyphArray.count();

    if (count) {
        SkGlyph**   gptr = fGlyphArray.begin();
        int     lo = 0;

        hi = count - 1;
        while (lo < hi) {
            int mid = (hi + lo) >> 1;
            if (gptr[mid]->fID < id) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        glyph = gptr[hi];
        if (glyph->fID == id) {
            if (kFull_MetricsType == mtype && glyph->isJustAdvance()) {
                fScalerContext->getMetrics(glyph);
            }
            return glyph;
        }

        // check if we need to bump hi before falling though to the allocator
        if (glyph->fID < id) {
            hi += 1;
        }
    }

    // not found, but hi tells us where to inser the new glyph
    fMemoryUsed += sizeof(SkGlyph);

    glyph = (SkGlyph*)fGlyphAlloc.alloc(sizeof(SkGlyph),
                                        SkChunkAlloc::kThrow_AllocFailType);
    glyph->init(id);
    *fGlyphArray.insert(hi) = glyph;

    if (kJustAdvance_MetricsType == mtype) {
        fScalerContext->getAdvance(glyph);
        fAdvanceCount += 1;
    } else {
        SkASSERT(kFull_MetricsType == mtype);
        fScalerContext->getMetrics(glyph);
        fMetricsCount += 1;
    }

    return glyph;
}

const void* SkGlyphCache::findImage(const SkGlyph& glyph) {
    if (glyph.fWidth > 0 && glyph.fWidth < kMaxGlyphWidth) {
        if (glyph.fImage == NULL) {
            size_t  size = glyph.computeImageSize();
            const_cast<SkGlyph&>(glyph).fImage = fImageAlloc.alloc(size,
                                        SkChunkAlloc::kReturnNil_AllocFailType);
            // check that alloc() actually succeeded
            if (glyph.fImage) {
                fScalerContext->getImage(glyph);
                // TODO: the scaler may have changed the maskformat during
                // getImage (e.g. from AA or LCD to BW) which means we may have
                // overallocated the buffer. Check if the new computedImageSize
                // is smaller, and if so, strink the alloc size in fImageAlloc.
                fMemoryUsed += size;
            }
        }
    }
    return glyph.fImage;
}

const SkPath* SkGlyphCache::findPath(const SkGlyph& glyph) {
    if (glyph.fWidth) {
        if (glyph.fPath == NULL) {
            const_cast<SkGlyph&>(glyph).fPath = SkNEW(SkPath);
            fScalerContext->getPath(glyph, glyph.fPath);
            fMemoryUsed += sizeof(SkPath) +
                    glyph.fPath->countPoints() * sizeof(SkPoint);
        }
    }
    return glyph.fPath;
}

///////////////////////////////////////////////////////////////////////////////

bool SkGlyphCache::getAuxProcData(void (*proc)(void*), void** dataPtr) const {
    const AuxProcRec* rec = fAuxProcList;
    while (rec) {
        if (rec->fProc == proc) {
            if (dataPtr) {
                *dataPtr = rec->fData;
            }
            return true;
        }
        rec = rec->fNext;
    }
    return false;
}

void SkGlyphCache::setAuxProc(void (*proc)(void*), void* data) {
    if (proc == NULL) {
        return;
    }

    AuxProcRec* rec = fAuxProcList;
    while (rec) {
        if (rec->fProc == proc) {
            rec->fData = data;
            return;
        }
        rec = rec->fNext;
    }
    // not found, create a new rec
    rec = SkNEW(AuxProcRec);
    rec->fProc = proc;
    rec->fData = data;
    rec->fNext = fAuxProcList;
    fAuxProcList = rec;
}

void SkGlyphCache::invokeAndRemoveAuxProcs() {
    AuxProcRec* rec = fAuxProcList;
    while (rec) {
        rec->fProc(rec->fData);
        AuxProcRec* next = rec->fNext;
        SkDELETE(rec);
        rec = next;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef SK_DEFAULT_FONT_CACHE_LIMIT
    #define SK_DEFAULT_FONT_CACHE_LIMIT     (2 * 1024 * 1024)
#endif

#ifdef USE_CACHE_HASH
    #define HASH_BITCOUNT   6
    #define HASH_COUNT      (1 << HASH_BITCOUNT)
    #define HASH_MASK       (HASH_COUNT - 1)

    static unsigned desc_to_hashindex(const SkDescriptor* desc)
    {
        SkASSERT(HASH_MASK < 256);  // since our munging reduces to 8 bits

        uint32_t n = *(const uint32_t*)desc;    //desc->getChecksum();
        SkASSERT(n == desc->getChecksum());

        // don't trust that the low bits of checksum vary enough, so...
        n ^= (n >> 24) ^ (n >> 16) ^ (n >> 8) ^ (n >> 30);

        return n & HASH_MASK;
    }
#endif

#include "SkThread.h"

class SkGlyphCache_Globals {
public:
    enum UseMutex {
        kNo_UseMutex,  // thread-local cache
        kYes_UseMutex  // shared cache
    };

    SkGlyphCache_Globals(UseMutex um) {
        fHead = NULL;
        fTotalMemoryUsed = 0;
        fFontCacheLimit = SK_DEFAULT_FONT_CACHE_LIMIT;
        fMutex = (kYes_UseMutex == um) ? SkNEW(SkMutex) : NULL;

#ifdef USE_CACHE_HASH
        sk_bzero(fHash, sizeof(fHash));
#endif
    }

    ~SkGlyphCache_Globals() {
        SkGlyphCache* cache = fHead;
        while (cache) {
            SkGlyphCache* next = cache->fNext;
            SkDELETE(cache);
            cache = next;
        }

        SkDELETE(fMutex);
    }

    SkMutex*        fMutex;
    SkGlyphCache*   fHead;
    size_t          fTotalMemoryUsed;
#ifdef USE_CACHE_HASH
    SkGlyphCache*   fHash[HASH_COUNT];
#endif

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

    size_t  getFontCacheLimit() const { return fFontCacheLimit; }
    size_t  setFontCacheLimit(size_t limit);
    void    purgeAll(); // does not change budget

    // can return NULL
    static SkGlyphCache_Globals* FindTLS() {
        return (SkGlyphCache_Globals*)SkTLS::Find(CreateTLS);
    }

    static SkGlyphCache_Globals& GetTLS() {
        return *(SkGlyphCache_Globals*)SkTLS::Get(CreateTLS, DeleteTLS);
    }

    static void DeleteTLS() { SkTLS::Delete(CreateTLS); }

private:
    size_t  fFontCacheLimit;

    static void* CreateTLS() {
        return SkNEW_ARGS(SkGlyphCache_Globals, (kNo_UseMutex));
    }

    static void DeleteTLS(void* ptr) {
        SkDELETE((SkGlyphCache_Globals*)ptr);
    }
};

size_t SkGlyphCache_Globals::setFontCacheLimit(size_t newLimit) {
    static const size_t minLimit = 256 * 1024;
    if (newLimit < minLimit) {
        newLimit = minLimit;
    }

    size_t prevLimit = fFontCacheLimit;
    fFontCacheLimit = newLimit;

    size_t currUsed = fTotalMemoryUsed;
    if (currUsed > newLimit) {
        SkAutoMutexAcquire    ac(fMutex);
        SkGlyphCache::InternalFreeCache(this, currUsed - newLimit);
    }
    return prevLimit;
}

void SkGlyphCache_Globals::purgeAll() {
    SkAutoMutexAcquire    ac(fMutex);
    SkGlyphCache::InternalFreeCache(this, fTotalMemoryUsed);
}

// Returns the shared globals
static SkGlyphCache_Globals& getSharedGlobals() {
    // we leak this, so we don't incur any shutdown cost of the destructor
    static SkGlyphCache_Globals* gGlobals = SkNEW_ARGS(SkGlyphCache_Globals,
                                       (SkGlyphCache_Globals::kYes_UseMutex));
    return *gGlobals;
}

// Returns the TLS globals (if set), or the shared globals
static SkGlyphCache_Globals& getGlobals() {
    SkGlyphCache_Globals* tls = SkGlyphCache_Globals::FindTLS();
    return tls ? *tls : getSharedGlobals();
}

void SkGlyphCache::VisitAllCaches(bool (*proc)(SkGlyphCache*, void*),
                                  void* context) {
    SkGlyphCache_Globals& globals = getGlobals();
    SkAutoMutexAcquire    ac(globals.fMutex);
    SkGlyphCache*         cache;

    globals.validate();

    for (cache = globals.fHead; cache != NULL; cache = cache->fNext) {
        if (proc(cache, context)) {
            break;
        }
    }

    globals.validate();
}

/*  This guy calls the visitor from within the mutext lock, so the visitor
    cannot:
    - take too much time
    - try to acquire the mutext again
    - call a fontscaler (which might call into the cache)
*/
SkGlyphCache* SkGlyphCache::VisitCache(const SkDescriptor* desc,
                              bool (*proc)(const SkGlyphCache*, void*),
                              void* context) {
    SkASSERT(desc);

    SkGlyphCache_Globals& globals = getGlobals();
    SkAutoMutexAcquire    ac(globals.fMutex);
    SkGlyphCache*         cache;
    bool                  insideMutex = true;

    globals.validate();

#ifdef USE_CACHE_HASH
    SkGlyphCache** hash = globals.fHash;
    unsigned index = desc_to_hashindex(desc);
    cache = hash[index];
    if (cache && *cache->fDesc == *desc) {
        cache->detach(&globals.fHead);
        goto FOUND_IT;
    }
#endif

    for (cache = globals.fHead; cache != NULL; cache = cache->fNext) {
        if (cache->fDesc->equals(*desc)) {
            cache->detach(&globals.fHead);
            goto FOUND_IT;
        }
    }

    /* Release the mutex now, before we create a new entry (which might have
        side-effects like trying to access the cache/mutex (yikes!)
    */
    ac.release();           // release the mutex now
    insideMutex = false;    // can't use globals anymore

    cache = SkNEW_ARGS(SkGlyphCache, (desc));

FOUND_IT:

    AutoValidate av(cache);

    if (proc(cache, context)) {   // stay detached
        if (insideMutex) {
            SkASSERT(globals.fTotalMemoryUsed >= cache->fMemoryUsed);
            globals.fTotalMemoryUsed -= cache->fMemoryUsed;
#ifdef USE_CACHE_HASH
            hash[index] = NULL;
#endif
        }
    } else {                        // reattach
        if (insideMutex) {
            cache->attachToHead(&globals.fHead);
#ifdef USE_CACHE_HASH
            hash[index] = cache;
#endif
        } else {
            AttachCache(cache);
        }
        cache = NULL;
    }
    return cache;
}

void SkGlyphCache::AttachCache(SkGlyphCache* cache) {
    SkASSERT(cache);
    SkASSERT(cache->fNext == NULL);

    SkGlyphCache_Globals& globals = getGlobals();
    SkAutoMutexAcquire    ac(globals.fMutex);

    globals.validate();
    cache->validate();

    // if we have a fixed budget for our cache, do a purge here
    {
        size_t allocated = globals.fTotalMemoryUsed + cache->fMemoryUsed;
        size_t budgeted = SkGraphics::GetFontCacheLimit();
        if (allocated > budgeted) {
            (void)InternalFreeCache(&globals, allocated - budgeted);
        }
    }

    cache->attachToHead(&globals.fHead);
    globals.fTotalMemoryUsed += cache->fMemoryUsed;

#ifdef USE_CACHE_HASH
    unsigned index = desc_to_hashindex(cache->fDesc);
    SkASSERT(globals.fHash[index] != cache);
    globals.fHash[index] = cache;
#endif

    globals.validate();
}

///////////////////////////////////////////////////////////////////////////////

SkGlyphCache* SkGlyphCache::FindTail(SkGlyphCache* cache) {
    if (cache) {
        while (cache->fNext) {
            cache = cache->fNext;
        }
    }
    return cache;
}

#ifdef SK_DEBUG
void SkGlyphCache_Globals::validate() const {
    size_t computed = 0;

    const SkGlyphCache* head = fHead;
    while (head != NULL) {
        computed += head->fMemoryUsed;
        head = head->fNext;
    }

    if (fTotalMemoryUsed != computed) {
        printf("total %d, computed %d\n", (int)fTotalMemoryUsed, (int)computed);
    }
    SkASSERT(fTotalMemoryUsed == computed);
}
#endif

size_t SkGlyphCache::InternalFreeCache(SkGlyphCache_Globals* globals,
                                       size_t bytesNeeded) {
    globals->validate();

    size_t  bytesFreed = 0;
    int     count = 0;

    // don't do any "small" purges
    size_t minToPurge = globals->fTotalMemoryUsed >> 2;
    if (bytesNeeded < minToPurge)
        bytesNeeded = minToPurge;

    SkGlyphCache* cache = FindTail(globals->fHead);
    while (cache != NULL && bytesFreed < bytesNeeded) {
        SkGlyphCache* prev = cache->fPrev;
        bytesFreed += cache->fMemoryUsed;

#ifdef USE_CACHE_HASH
        unsigned index = desc_to_hashindex(cache->fDesc);
        if (cache == globals->fHash[index]) {
            globals->fHash[index] = NULL;
        }
#endif

        cache->detach(&globals->fHead);
        SkDELETE(cache);
        cache = prev;
        count += 1;
    }

    SkASSERT(bytesFreed <= globals->fTotalMemoryUsed);
    globals->fTotalMemoryUsed -= bytesFreed;
    globals->validate();

#ifdef SPEW_PURGE_STATUS
    if (count && !gSkSuppressFontCachePurgeSpew) {
        SkDebugf("purging %dK from font cache [%d entries]\n",
                 (int)(bytesFreed >> 10), count);
    }
#endif

    return bytesFreed;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void SkGlyphCache::validate() const {
    int count = fGlyphArray.count();
    for (int i = 0; i < count; i++) {
        const SkGlyph* glyph = fGlyphArray[i];
        SkASSERT(glyph);
        SkASSERT(fGlyphAlloc.contains(glyph));
        if (glyph->fImage) {
            SkASSERT(fImageAlloc.contains(glyph->fImage));
        }
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "SkTypefaceCache.h"

size_t SkGraphics::GetFontCacheLimit() {
    return getSharedGlobals().getFontCacheLimit();
}

size_t SkGraphics::SetFontCacheLimit(size_t bytes) {
    return getSharedGlobals().setFontCacheLimit(bytes);
}

size_t SkGraphics::GetFontCacheUsed() {
    return getSharedGlobals().fTotalMemoryUsed;
}

void SkGraphics::PurgeFontCache() {
    getSharedGlobals().purgeAll();
    SkTypefaceCache::PurgeAll();
}

size_t SkGraphics::GetTLSFontCacheLimit() {
    const SkGlyphCache_Globals* tls = SkGlyphCache_Globals::FindTLS();
    return tls ? tls->getFontCacheLimit() : 0;
}

void SkGraphics::SetTLSFontCacheLimit(size_t bytes) {
    if (0 == bytes) {
        SkGlyphCache_Globals::DeleteTLS();
    } else {
        SkGlyphCache_Globals::GetTLS().setFontCacheLimit(bytes);
    }
}



/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkGlyphCache.h"
#include "SkGlyphCache_Globals.h"
#include "SkGraphics.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkTemplates.h"
#include "SkTLS.h"
#include "SkTypeface.h"

//#define SPEW_PURGE_STATUS
//#define RECORD_HASH_EFFICIENCY

bool gSkSuppressFontCachePurgeSpew;

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

// so we don't grow our arrays a lot
#define kMinGlyphCount      16
#define kMinGlyphImageSize  (16*2)
#define kMinAllocAmount     ((sizeof(SkGlyph) + kMinGlyphImageSize) * kMinGlyphCount)

SkGlyphCache::SkGlyphCache(SkTypeface* typeface, const SkDescriptor* desc, SkScalerContext* ctx)
        : fScalerContext(ctx), fGlyphAlloc(kMinAllocAmount) {
    SkASSERT(typeface);
    SkASSERT(desc);
    SkASSERT(ctx);

    fPrev = fNext = NULL;

    fDesc = desc->copy();
    fScalerContext->getFontMetrics(&fFontMetrics);

    // init to 0 so that all of the pointers will be null
    memset(fGlyphHash, 0, sizeof(fGlyphHash));
    // init with 0xFF so that the charCode field will be -1, which is invalid
    memset(fCharToGlyphHash, 0xFF, sizeof(fCharToGlyphHash));

    fMemoryUsed = sizeof(*this);

    fGlyphArray.setReserve(kMinGlyphCount);

    fAuxProcList = NULL;
}

SkGlyphCache::~SkGlyphCache() {
#if 0
    {
        size_t ptrMem = fGlyphArray.count() * sizeof(SkGlyph*);
        size_t glyphAlloc = fGlyphAlloc.totalCapacity();
        size_t glyphHashUsed = 0;
        size_t uniHashUsed = 0;
        for (int i = 0; i < kHashCount; ++i) {
            glyphHashUsed += fGlyphHash[i] ? sizeof(fGlyphHash[0]) : 0;
            uniHashUsed += fCharToGlyphHash[i].fID != 0xFFFFFFFF ? sizeof(fCharToGlyphHash[0]) : 0;
        }
        size_t glyphUsed = fGlyphArray.count() * sizeof(SkGlyph);
        size_t imageUsed = 0;
        for (int i = 0; i < fGlyphArray.count(); ++i) {
            const SkGlyph& g = *fGlyphArray[i];
            if (g.fImage) {
                imageUsed += g.fHeight * g.rowBytes();
            }
        }

        printf("glyphPtrArray,%zu, Alloc,%zu, imageUsed,%zu, glyphUsed,%zu, glyphHashAlloc,%zu, glyphHashUsed,%zu, unicharHashAlloc,%zu, unicharHashUsed,%zu\n",
                 ptrMem, glyphAlloc, imageUsed, glyphUsed, sizeof(fGlyphHash), glyphHashUsed, sizeof(fCharToGlyphHash), uniHashUsed);

    }
#endif
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
    } else {
        SkASSERT(kFull_MetricsType == mtype);
        fScalerContext->getMetrics(glyph);
    }

    return glyph;
}

const void* SkGlyphCache::findImage(const SkGlyph& glyph) {
    if (glyph.fWidth > 0 && glyph.fWidth < kMaxGlyphWidth) {
        if (glyph.fImage == NULL) {
            size_t  size = glyph.computeImageSize();
            const_cast<SkGlyph&>(glyph).fImage = fGlyphAlloc.alloc(size,
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

#include "SkThread.h"

size_t SkGlyphCache_Globals::setCacheSizeLimit(size_t newLimit) {
    static const size_t minLimit = 256 * 1024;
    if (newLimit < minLimit) {
        newLimit = minLimit;
    }

    SkAutoMutexAcquire    ac(fMutex);

    size_t prevLimit = fCacheSizeLimit;
    fCacheSizeLimit = newLimit;
    this->internalPurge();
    return prevLimit;
}

int SkGlyphCache_Globals::setCacheCountLimit(int newCount) {
    if (newCount < 0) {
        newCount = 0;
    }

    SkAutoMutexAcquire    ac(fMutex);

    int prevCount = fCacheCountLimit;
    fCacheCountLimit = newCount;
    this->internalPurge();
    return prevCount;
}

void SkGlyphCache_Globals::purgeAll() {
    SkAutoMutexAcquire    ac(fMutex);
    this->internalPurge(fTotalMemoryUsed);
}

void SkGlyphCache::VisitAllCaches(bool (*proc)(SkGlyphCache*, void*),
                                  void* context) {
    SkGlyphCache_Globals& globals = getGlobals();
    SkAutoMutexAcquire    ac(globals.fMutex);
    SkGlyphCache*         cache;

    globals.validate();

    for (cache = globals.internalGetHead(); cache != NULL; cache = cache->fNext) {
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
SkGlyphCache* SkGlyphCache::VisitCache(SkTypeface* typeface,
                              const SkDescriptor* desc,
                              bool (*proc)(const SkGlyphCache*, void*),
                              void* context) {
    if (!typeface) {
        typeface = SkTypeface::GetDefaultTypeface();
    }
    SkASSERT(desc);

    SkGlyphCache_Globals& globals = getGlobals();
    SkAutoMutexAcquire    ac(globals.fMutex);
    SkGlyphCache*         cache;
    bool                  insideMutex = true;

    globals.validate();

    for (cache = globals.internalGetHead(); cache != NULL; cache = cache->fNext) {
        if (cache->fDesc->equals(*desc)) {
            globals.internalDetachCache(cache);
            goto FOUND_IT;
        }
    }

    /* Release the mutex now, before we create a new entry (which might have
        side-effects like trying to access the cache/mutex (yikes!)
    */
    ac.release();           // release the mutex now
    insideMutex = false;    // can't use globals anymore

    // Check if we can create a scaler-context before creating the glyphcache.
    // If not, we may have exhausted OS/font resources, so try purging the
    // cache once and try again.
    {
        // pass true the first time, to notice if the scalercontext failed,
        // so we can try the purge.
        SkScalerContext* ctx = typeface->createScalerContext(desc, true);
        if (!ctx) {
            getSharedGlobals().purgeAll();
            ctx = typeface->createScalerContext(desc, false);
            SkASSERT(ctx);
        }
        cache = SkNEW_ARGS(SkGlyphCache, (typeface, desc, ctx));
    }

FOUND_IT:

    AutoValidate av(cache);

    if (!proc(cache, context)) {   // need to reattach
        if (insideMutex) {
            globals.internalAttachCacheToHead(cache);
        } else {
            globals.attachCacheToHead(cache);
        }
        cache = NULL;
    }
    return cache;
}

void SkGlyphCache::AttachCache(SkGlyphCache* cache) {
    SkASSERT(cache);
    SkASSERT(cache->fNext == NULL);

    getGlobals().attachCacheToHead(cache);
}

///////////////////////////////////////////////////////////////////////////////

void SkGlyphCache_Globals::attachCacheToHead(SkGlyphCache* cache) {
    SkAutoMutexAcquire    ac(fMutex);

    this->validate();
    cache->validate();

    this->internalAttachCacheToHead(cache);
    this->internalPurge();
}

SkGlyphCache* SkGlyphCache_Globals::internalGetTail() const {
    SkGlyphCache* cache = fHead;
    if (cache) {
        while (cache->fNext) {
            cache = cache->fNext;
        }
    }
    return cache;
}

size_t SkGlyphCache_Globals::internalPurge(size_t minBytesNeeded) {
    this->validate();

    size_t bytesNeeded = 0;
    if (fTotalMemoryUsed > fCacheSizeLimit) {
        bytesNeeded = fTotalMemoryUsed - fCacheSizeLimit;
    }
    bytesNeeded = SkTMax(bytesNeeded, minBytesNeeded);
    if (bytesNeeded) {
        // no small purges!
        bytesNeeded = SkTMax(bytesNeeded, fTotalMemoryUsed >> 2);
    }

    int countNeeded = 0;
    if (fCacheCount > fCacheCountLimit) {
        countNeeded = fCacheCount - fCacheCountLimit;
        // no small purges!
        countNeeded = SkMax32(countNeeded, fCacheCount >> 2);
    }

    // early exit
    if (!countNeeded && !bytesNeeded) {
        return 0;
    }

    size_t  bytesFreed = 0;
    int     countFreed = 0;

    // we start at the tail and proceed backwards, as the linklist is in LRU
    // order, with unimportant entries at the tail.
    SkGlyphCache* cache = this->internalGetTail();
    while (cache != NULL &&
           (bytesFreed < bytesNeeded || countFreed < countNeeded)) {
        SkGlyphCache* prev = cache->fPrev;
        bytesFreed += cache->fMemoryUsed;
        countFreed += 1;

        this->internalDetachCache(cache);
        SkDELETE(cache);
        cache = prev;
    }

    this->validate();

#ifdef SPEW_PURGE_STATUS
    if (countFreed && !gSkSuppressFontCachePurgeSpew) {
        SkDebugf("purging %dK from font cache [%d entries]\n",
                 (int)(bytesFreed >> 10), countFreed);
    }
#endif

    return bytesFreed;
}

void SkGlyphCache_Globals::internalAttachCacheToHead(SkGlyphCache* cache) {
    SkASSERT(NULL == cache->fPrev && NULL == cache->fNext);
    if (fHead) {
        fHead->fPrev = cache;
        cache->fNext = fHead;
    }
    fHead = cache;

    fCacheCount += 1;
    fTotalMemoryUsed += cache->fMemoryUsed;
}

void SkGlyphCache_Globals::internalDetachCache(SkGlyphCache* cache) {
    SkASSERT(fCacheCount > 0);
    fCacheCount -= 1;
    fTotalMemoryUsed -= cache->fMemoryUsed;

    if (cache->fPrev) {
        cache->fPrev->fNext = cache->fNext;
    } else {
        fHead = cache->fNext;
    }
    if (cache->fNext) {
        cache->fNext->fPrev = cache->fPrev;
    }
    cache->fPrev = cache->fNext = NULL;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void SkGlyphCache::validate() const {
#ifdef SK_DEBUG_GLYPH_CACHE
    int count = fGlyphArray.count();
    for (int i = 0; i < count; i++) {
        const SkGlyph* glyph = fGlyphArray[i];
        SkASSERT(glyph);
        SkASSERT(fGlyphAlloc.contains(glyph));
        if (glyph->fImage) {
            SkASSERT(fGlyphAlloc.contains(glyph->fImage));
        }
    }
#endif
}

void SkGlyphCache_Globals::validate() const {
    size_t computedBytes = 0;
    int computedCount = 0;

    const SkGlyphCache* head = fHead;
    while (head != NULL) {
        computedBytes += head->fMemoryUsed;
        computedCount += 1;
        head = head->fNext;
    }

    SkASSERT(fTotalMemoryUsed == computedBytes);
    SkASSERT(fCacheCount == computedCount);
}

#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "SkTypefaceCache.h"

size_t SkGraphics::GetFontCacheLimit() {
    return getSharedGlobals().getCacheSizeLimit();
}

size_t SkGraphics::SetFontCacheLimit(size_t bytes) {
    return getSharedGlobals().setCacheSizeLimit(bytes);
}

size_t SkGraphics::GetFontCacheUsed() {
    return getSharedGlobals().getTotalMemoryUsed();
}

int SkGraphics::GetFontCacheCountLimit() {
    return getSharedGlobals().getCacheCountLimit();
}

int SkGraphics::SetFontCacheCountLimit(int count) {
    return getSharedGlobals().setCacheCountLimit(count);
}

int SkGraphics::GetFontCacheCountUsed() {
    return getSharedGlobals().getCacheCountUsed();
}

void SkGraphics::PurgeFontCache() {
    getSharedGlobals().purgeAll();
    SkTypefaceCache::PurgeAll();
}

size_t SkGraphics::GetTLSFontCacheLimit() {
    const SkGlyphCache_Globals* tls = SkGlyphCache_Globals::FindTLS();
    return tls ? tls->getCacheSizeLimit() : 0;
}

void SkGraphics::SetTLSFontCacheLimit(size_t bytes) {
    if (0 == bytes) {
        SkGlyphCache_Globals::DeleteTLS();
    } else {
        SkGlyphCache_Globals::GetTLS().setCacheSizeLimit(bytes);
    }
}

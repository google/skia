/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphCache.h"
#include "SkGlyphCache_Globals.h"
#include "SkGraphics.h"
#include "SkLazyPtr.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkTemplates.h"
#include "SkTLS.h"
#include "SkTypeface.h"

//#define SPEW_PURGE_STATUS

namespace {

SkGlyphCache_Globals* create_globals() {
    return SkNEW_ARGS(SkGlyphCache_Globals, (SkGlyphCache_Globals::kYes_UseMutex));
}

}  // namespace

SK_DECLARE_STATIC_LAZY_PTR(SkGlyphCache_Globals, globals, create_globals);

// Returns the shared globals
static SkGlyphCache_Globals& getSharedGlobals() {
    return *globals.get();
}

// Returns the TLS globals (if set), or the shared globals
static SkGlyphCache_Globals& getGlobals() {
    SkGlyphCache_Globals* tls = SkGlyphCache_Globals::FindTLS();
    return tls ? *tls : getSharedGlobals();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_GLYPHCACHE_TRACK_HASH_STATS
    #define RecordHashSuccess()             fHashHitCount += 1
    #define RecordHashCollisionIf(pred)     do { if (pred) fHashMissCount += 1; } while (0)
#else
    #define RecordHashSuccess()             (void)0
    #define RecordHashCollisionIf(pred)     (void)0
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

    // Create the sentinel SkGlyph.
    SkGlyph* sentinel = fGlyphArray.insert(0);
    sentinel->initGlyphFromCombinedID(SkGlyph::kImpossibleID);

    // Initialize all index to zero which points to the sentinel SkGlyph.
    memset(fGlyphHash, 0x00, sizeof(fGlyphHash));

    fMemoryUsed = sizeof(*this);

    fGlyphArray.setReserve(kMinGlyphCount);

    fAuxProcList = NULL;

#ifdef SK_GLYPHCACHE_TRACK_HASH_STATS
    fHashHitCount = fHashMissCount = 0;
#endif
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

        SkDebugf("glyphPtrArray,%zu, Alloc,%zu, imageUsed,%zu, glyphUsed,%zu, glyphHashAlloc,%zu, glyphHashUsed,%zu, unicharHashAlloc,%zu, unicharHashUsed,%zu\n",
                 ptrMem, glyphAlloc, imageUsed, glyphUsed, sizeof(fGlyphHash), glyphHashUsed, sizeof(CharGlyphRec) * kHashCount, uniHashUsed);

    }
#endif
    SkGlyph*   gptr = fGlyphArray.begin();
    SkGlyph*   stop = fGlyphArray.end();
    while (gptr < stop) {
        SkPath* path = gptr->fPath;
        if (path) {
            SkDELETE(path);
        }
        gptr += 1;
    }
    SkDescriptor::Free(fDesc);
    SkDELETE(fScalerContext);
    this->invokeAndRemoveAuxProcs();
}

SkGlyphCache::CharGlyphRec* SkGlyphCache::getCharGlyphRec(uint32_t id) {
    if (NULL == fCharToGlyphHash.get()) {
        // Allocate the array.
        fCharToGlyphHash.reset(kHashCount);
        // Initialize entries of fCharToGlyphHash to index the sentinel glyph and
        // an fID value that will not match any id.
        for (int i = 0; i <kHashCount; ++i) {
            fCharToGlyphHash[i].fID = SkGlyph::kImpossibleID;
            fCharToGlyphHash[i].fGlyphIndex = 0;
        }
    }

    return &fCharToGlyphHash[ID2HashIndex(id)];
}

void SkGlyphCache::adjustCaches(int insertion_index) {
    for (int i = 0; i < kHashCount; ++i) {
        if (fGlyphHash[i] >= SkToU16(insertion_index)) {
            fGlyphHash[i] += 1;
        }
    }
    if (fCharToGlyphHash.get() != NULL) {
        for (int i = 0; i < kHashCount; ++i) {
            if (fCharToGlyphHash[i].fGlyphIndex >= SkToU16(insertion_index)) {
                fCharToGlyphHash[i].fGlyphIndex += 1;
            }
        }
    }
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
    const CharGlyphRec& rec = *this->getCharGlyphRec(id);

    if (rec.fID == id) {
        return fGlyphArray[rec.fGlyphIndex].getGlyphID();
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
    return *this->lookupByChar(charCode, kJustAdvance_MetricsType);
}

const SkGlyph& SkGlyphCache::getGlyphIDAdvance(uint16_t glyphID) {
    VALIDATE();
    uint32_t id = SkGlyph::MakeID(glyphID);
    return *this->lookupByCombinedID(id, kJustAdvance_MetricsType);
}

///////////////////////////////////////////////////////////////////////////////

const SkGlyph& SkGlyphCache::getUnicharMetrics(SkUnichar charCode) {
    VALIDATE();
    return *this->lookupByChar(charCode, kFull_MetricsType);
}

const SkGlyph& SkGlyphCache::getUnicharMetrics(SkUnichar charCode,
                                               SkFixed x, SkFixed y) {
    VALIDATE();
    return *this->lookupByChar(charCode, kFull_MetricsType, x, y);
}

const SkGlyph& SkGlyphCache::getGlyphIDMetrics(uint16_t glyphID) {
    VALIDATE();
    uint32_t id = SkGlyph::MakeID(glyphID);
    return *this->lookupByCombinedID(id, kFull_MetricsType);
}

const SkGlyph& SkGlyphCache::getGlyphIDMetrics(uint16_t glyphID, SkFixed x, SkFixed y) {
    VALIDATE();
    uint32_t id = SkGlyph::MakeID(glyphID, x, y);
    return *this->lookupByCombinedID(id, kFull_MetricsType);
}

SkGlyph* SkGlyphCache::lookupByChar(SkUnichar charCode, MetricsType type, SkFixed x, SkFixed y) {
    uint32_t id = SkGlyph::MakeID(charCode, x, y);
    CharGlyphRec* rec = this->getCharGlyphRec(id);
    SkGlyph* glyph;
    if (rec->fID != id) {
        RecordHashCollisionIf(glyph_index != SkGlyph::kImpossibleID);
        // this ID is based on the UniChar
        rec->fID = id;
        // this ID is based on the glyph index
        id = SkGlyph::MakeID(fScalerContext->charToGlyphID(charCode), x, y);
        rec->fGlyphIndex = this->lookupMetrics(id, type);
        glyph = &fGlyphArray[rec->fGlyphIndex];
    } else {
        RecordHashSuccess();
        glyph = &fGlyphArray[rec->fGlyphIndex];
        if (type == kFull_MetricsType && glyph->isJustAdvance()) {
            fScalerContext->getMetrics(glyph);
        }
    }
    return glyph;
}

SkGlyph* SkGlyphCache::lookupByCombinedID(uint32_t id, MetricsType type) {
    uint32_t hash_index = ID2HashIndex(id);
    uint16_t glyph_index = fGlyphHash[hash_index];
    SkGlyph* glyph = &fGlyphArray[glyph_index];

    if (glyph->fID != id) {
        RecordHashCollisionIf(glyph_index != SkGlyph::kImpossibleID);
        glyph_index = this->lookupMetrics(id, type);
        fGlyphHash[hash_index] = glyph_index;
        glyph = &fGlyphArray[glyph_index];
    } else {
        RecordHashSuccess();
        if (type == kFull_MetricsType && glyph->isJustAdvance()) {
           fScalerContext->getMetrics(glyph);
        }
    }
    return glyph;
}

uint16_t SkGlyphCache::lookupMetrics(uint32_t id, MetricsType mtype) {
    SkASSERT(id != SkGlyph::kImpossibleID);
    // Count is always greater than 0 because of the sentinel.
    // The fGlyphArray cache is in descending order, so that the sentinel with a value of ~0 is
    // always at index 0.
    SkGlyph* gptr = fGlyphArray.begin();
    int lo = 0;
    int hi = fGlyphArray.count() - 1;
    while (lo < hi) {
        int mid = (hi + lo) >> 1;
        if (gptr[mid].fID > id) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    uint16_t glyph_index = hi;
    SkGlyph* glyph = &gptr[glyph_index];
    if (glyph->fID == id) {
        if (kFull_MetricsType == mtype && glyph->isJustAdvance()) {
            fScalerContext->getMetrics(glyph);
        }
        SkASSERT(glyph->fID != SkGlyph::kImpossibleID);
        return glyph_index;
    }

    // check if we need to bump hi before falling though to the allocator
    if (glyph->fID > id) {
        glyph_index += 1;
    }

    // Not found, but hi contains the index of the insertion point of the new glyph.
    fMemoryUsed += sizeof(SkGlyph);

    this->adjustCaches(glyph_index);

    glyph = fGlyphArray.insert(glyph_index);
    glyph->initGlyphFromCombinedID(id);

    if (kJustAdvance_MetricsType == mtype) {
        fScalerContext->getAdvance(glyph);
    } else {
        SkASSERT(kFull_MetricsType == mtype);
        fScalerContext->getMetrics(glyph);
    }

    SkASSERT(glyph->fID != SkGlyph::kImpossibleID);
    return glyph_index;
}

const void* SkGlyphCache::findImage(const SkGlyph& glyph) {
    if (glyph.fWidth > 0 && glyph.fWidth < kMaxGlyphWidth) {
        if (NULL == glyph.fImage) {
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

void SkGlyphCache::dump() const {
    const SkTypeface* face = fScalerContext->getTypeface();
    const SkScalerContextRec& rec = fScalerContext->getRec();
    SkMatrix matrix;
    rec.getSingleMatrix(&matrix);
    matrix.preScale(SkScalarInvert(rec.fTextSize), SkScalarInvert(rec.fTextSize));
    SkString name;
    face->getFamilyName(&name);

    SkString msg;
    msg.printf("cache typeface:%x %25s:%d size:%2g [%g %g %g %g] lum:%02X devG:%d pntG:%d cntr:%d glyphs:%3d",
               face->uniqueID(), name.c_str(), face->style(), rec.fTextSize,
               matrix[SkMatrix::kMScaleX], matrix[SkMatrix::kMSkewX],
               matrix[SkMatrix::kMSkewY], matrix[SkMatrix::kMScaleY],
               rec.fLumBits & 0xFF, rec.fDeviceGamma, rec.fPaintGamma, rec.fContrast,
               fGlyphArray.count());
#ifdef SK_GLYPHCACHE_TRACK_HASH_STATS
    const int sum = SkTMax(fHashHitCount + fHashMissCount, 1);   // avoid divide-by-zero
    msg.appendf(" hash:%2d\n", 100 * fHashHitCount / sum);
#endif
    SkDebugf("%s\n", msg.c_str());
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

void SkGlyphCache::Dump() {
    SkGlyphCache_Globals& globals = getGlobals();
    SkAutoMutexAcquire    ac(globals.fMutex);
    SkGlyphCache*         cache;

    globals.validate();

    SkDebugf("SkGlyphCache strikes:%d memory:%d\n",
             globals.getCacheCountUsed(), (int)globals.getTotalMemoryUsed());

#ifdef SK_GLYPHCACHE_TRACK_HASH_STATS
    int hitCount = 0;
    int missCount = 0;
#endif

    for (cache = globals.internalGetHead(); cache != NULL; cache = cache->fNext) {
#ifdef SK_GLYPHCACHE_TRACK_HASH_STATS
        hitCount += cache->fHashHitCount;
        missCount += cache->fHashMissCount;
#endif
        cache->dump();
    }
#ifdef SK_GLYPHCACHE_TRACK_HASH_STATS
    SkDebugf("Hash hit percent:%2d\n", 100 * hitCount / (hitCount + missCount));
#endif
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
    if (countFreed) {
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
        const SkGlyph* glyph = &fGlyphArray[i];
        SkASSERT(glyph);
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

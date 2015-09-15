/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphCache.h"
#include "SkGlyphCache_Globals.h"
#include "SkGraphics.h"
#include "SkOnce.h"
#include "SkOncePtr.h"
#include "SkPath.h"
#include "SkTemplates.h"
#include "SkTraceMemoryDump.h"
#include "SkTypeface.h"

//#define SPEW_PURGE_STATUS

namespace {

const char gGlyphCacheDumpName[] = "skia/sk_glyph_cache";

// Used to pass context to the sk_trace_dump_visitor.
struct SkGlyphCacheDumpContext {
    int* counter;
    SkTraceMemoryDump* dump;
};

}  // namespace

// Returns the shared globals
SK_DECLARE_STATIC_ONCE_PTR(SkGlyphCache_Globals, globals);
static SkGlyphCache_Globals& get_globals() {
    return *globals.get([]{ return new SkGlyphCache_Globals; });
}

///////////////////////////////////////////////////////////////////////////////

// so we don't grow our arrays a lot
#define kMinGlyphCount      16
#define kMinGlyphImageSize  (16*2)
#define kMinAllocAmount     ((sizeof(SkGlyph) + kMinGlyphImageSize) * kMinGlyphCount)

SkGlyphCache::SkGlyphCache(SkTypeface* typeface, const SkDescriptor* desc, SkScalerContext* ctx)
    : fNext(nullptr)
    , fPrev(nullptr)
    , fDesc(desc->copy())
    , fRefCount(0)
    , fGlyphAlloc(kMinAllocAmount)
    , fMemoryUsed(sizeof(*this))
    , fScalerContext(ctx)
    , fAuxProcList(nullptr) {
    SkASSERT(typeface);
    SkASSERT(desc);
    SkASSERT(ctx);

    fScalerContext->getFontMetrics(&fFontMetrics);
}

SkGlyphCache::~SkGlyphCache() {
    fGlyphMap.foreach ([](SkGlyph* g) { delete g->fPath; });
    SkDescriptor::Free(fDesc);
    delete fScalerContext;
    AuxProcRec* rec = fAuxProcList;
    while (rec) {
        rec->fProc(rec->fData);
        AuxProcRec* next = rec->fNext;
        delete rec;
        rec = next;
    }
}

void SkGlyphCache::increaseMemoryUsed(size_t used) {
    fMemoryUsed += used;
    get_globals().increaseTotalMemoryUsed(used);
}

SkGlyphCache::CharGlyphRec
SkGlyphCache::PackedUnicharIDtoCharGlyphRec(PackedUnicharID packedUnicharID) {
    SkFixed x = SkGlyph::SubToFixed(SkGlyph::ID2SubX(packedUnicharID));
    SkFixed y = SkGlyph::SubToFixed(SkGlyph::ID2SubY(packedUnicharID));
    SkUnichar unichar = SkGlyph::ID2Code(packedUnicharID);

    SkAutoMutexAcquire lock(fScalerMutex);
    PackedGlyphID packedGlyphID = SkGlyph::MakeID(fScalerContext->charToGlyphID(unichar), x, y);

    return {packedUnicharID, packedGlyphID};
}

SkGlyphCache::CharGlyphRec* SkGlyphCache::getCharGlyphRec(PackedUnicharID packedUnicharID) {
    if (nullptr == fPackedUnicharIDToPackedGlyphID.get()) {
        fMapMutex.releaseShared();

        // Add the map only if there is a call for char -> glyph mapping.
        {
            SkAutoTAcquire<SkSharedMutex> lock(fMapMutex);

            // Now that the cache is locked exclusively, make sure no one added this array
            // while unlocked.
            if (nullptr == fPackedUnicharIDToPackedGlyphID.get()) {
                // Allocate the array.
                fPackedUnicharIDToPackedGlyphID.reset(new PackedUnicharIDToPackedGlyphIDMap);
            }

            fPackedUnicharIDToPackedGlyphID->set(PackedUnicharIDtoCharGlyphRec(packedUnicharID));
        }
        fMapMutex.acquireShared();

        return fPackedUnicharIDToPackedGlyphID->find(packedUnicharID);
    }

    CharGlyphRec* answer = fPackedUnicharIDToPackedGlyphID->find(packedUnicharID);
    if (nullptr == answer) {
        fMapMutex.releaseShared();
        // Add a new char -> glyph mapping.
        {
            SkAutoTAcquire<SkSharedMutex> lock(fMapMutex);
            answer = fPackedUnicharIDToPackedGlyphID->find(packedUnicharID);
            if (nullptr == answer) {
                fPackedUnicharIDToPackedGlyphID->set(
                    PackedUnicharIDtoCharGlyphRec(packedUnicharID));
            }
        }
        fMapMutex.acquireShared();
        return fPackedUnicharIDToPackedGlyphID->find(packedUnicharID);
    }

    return answer;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
#define VALIDATE()  AutoValidate av(this)
#else
#define VALIDATE()
#endif

uint16_t SkGlyphCache::unicharToGlyph(SkUnichar charCode) {
    VALIDATE();
    PackedUnicharID packedUnicharID = SkGlyph::MakeID(charCode);
    const CharGlyphRec& rec = *this->getCharGlyphRec(packedUnicharID);
    return SkGlyph::ID2Code(rec.fPackedGlyphID);
}

SkUnichar SkGlyphCache::glyphToUnichar(uint16_t glyphID) {
    SkAutoMutexAcquire lock(fScalerMutex);
    return fScalerContext->glyphIDToChar(glyphID);
}

unsigned SkGlyphCache::getGlyphCount() const {
    return fScalerContext->getGlyphCount();
}

int SkGlyphCache::countCachedGlyphs() const {
    return fGlyphMap.count();
}

///////////////////////////////////////////////////////////////////////////////

SkGlyph* SkGlyphCache::lookupByChar(SkUnichar charCode, SkFixed x, SkFixed y) {
    PackedUnicharID targetUnicharID = SkGlyph::MakeID(charCode, x, y);
    CharGlyphRec* rec = this->getCharGlyphRec(targetUnicharID);
    PackedGlyphID packedGlyphID = rec->fPackedGlyphID;

    return this->lookupByPackedGlyphID(packedGlyphID);
}

const SkGlyph& SkGlyphCache::getUnicharAdvance(SkUnichar charCode) {
    VALIDATE();
    return *this->lookupByChar(charCode);
}

const SkGlyph& SkGlyphCache::getUnicharMetrics(SkUnichar charCode) {
    VALIDATE();
    return *this->lookupByChar(charCode);
}

const SkGlyph& SkGlyphCache::getUnicharMetrics(SkUnichar charCode, SkFixed x, SkFixed y) {
    VALIDATE();
    return *this->lookupByChar(charCode, x, y);
}

///////////////////////////////////////////////////////////////////////////////
SkGlyph* SkGlyphCache::allocateNewGlyph(PackedGlyphID packedGlyphID) {
    SkGlyph* glyphPtr;
    {
        fMapMutex.releaseShared();
        {
            SkAutoTAcquire<SkSharedMutex> mapLock(fMapMutex);
            glyphPtr = fGlyphMap.find(packedGlyphID);
            if (nullptr == glyphPtr) {
                SkGlyph glyph;
                glyph.initGlyphFromCombinedID(packedGlyphID);
                {
                    SkAutoMutexAcquire lock(fScalerMutex);
                    fScalerContext->getMetrics(&glyph);
                    this->increaseMemoryUsed(sizeof(SkGlyph));
                    glyphPtr = fGlyphMap.set(glyph);
                }  // drop scaler lock

            }
        }  // drop map lock
        fMapMutex.acquireShared();
        glyphPtr = fGlyphMap.find(packedGlyphID);
    }

    SkASSERT(glyphPtr->fID != SkGlyph::kImpossibleID);
    return glyphPtr;
}

SkGlyph* SkGlyphCache::lookupByPackedGlyphID(PackedGlyphID packedGlyphID) {
    SkGlyph* glyph = fGlyphMap.find(packedGlyphID);

    if (nullptr == glyph) {
        glyph = this->allocateNewGlyph(packedGlyphID);
    }
    return glyph;
}

const SkGlyph& SkGlyphCache::getGlyphIDAdvance(uint16_t glyphID) {
    VALIDATE();
    PackedGlyphID packedGlyphID = SkGlyph::MakeID(glyphID);
    return *this->lookupByPackedGlyphID(packedGlyphID);
}

const SkGlyph& SkGlyphCache::getGlyphIDMetrics(uint16_t glyphID) {
    VALIDATE();
    PackedGlyphID packedGlyphID = SkGlyph::MakeID(glyphID);
    return *this->lookupByPackedGlyphID(packedGlyphID);
}

const SkGlyph& SkGlyphCache::getGlyphIDMetrics(uint16_t glyphID, SkFixed x, SkFixed y) {
    VALIDATE();
    PackedGlyphID packedGlyphID = SkGlyph::MakeID(glyphID, x, y);
    return *this->lookupByPackedGlyphID(packedGlyphID);
}

///////////////////////////////////////////////////////////////////////////////

void SkGlyphCache::OnceFillInImage(GlyphAndCache gc) {
    SkGlyphCache* cache = gc.cache;
    const SkGlyph* glyph = gc.glyph;
    cache->fScalerMutex.assertHeld();
    if (glyph->fWidth > 0 && glyph->fWidth < kMaxGlyphWidth) {
        size_t size = glyph->computeImageSize();
        sk_atomic_store(&const_cast<SkGlyph*>(glyph)->fImage,
                cache->fGlyphAlloc.alloc(size, SkChunkAlloc::kReturnNil_AllocFailType),
                        sk_memory_order_relaxed);
        if (glyph->fImage != nullptr) {
            cache->fScalerContext->getImage(*glyph);
            cache->increaseMemoryUsed(size);
        }
    }
}

const void* SkGlyphCache::findImage(const SkGlyph& glyph) {
    SkOnce<SkMutex, GlyphAndCache>(
        &glyph.fImageIsSet, &fScalerMutex, &SkGlyphCache::OnceFillInImage, {this, &glyph});
    return sk_atomic_load(&glyph.fImage, sk_memory_order_seq_cst);
}

void SkGlyphCache::OnceFillInPath(GlyphAndCache gc) {
    SkGlyphCache* cache = gc.cache;
    const SkGlyph* glyph = gc.glyph;
    cache->fScalerMutex.assertHeld();
    if (glyph->fWidth > 0) {
        sk_atomic_store(&const_cast<SkGlyph*>(glyph)->fPath, new SkPath, sk_memory_order_relaxed);
        cache->fScalerContext->getPath(*glyph, glyph->fPath);
        size_t size = sizeof(SkPath) + glyph->fPath->countPoints() * sizeof(SkPoint);
        cache->increaseMemoryUsed(size);
    }
}

const SkPath* SkGlyphCache::findPath(const SkGlyph& glyph) {
    SkOnce<SkMutex, GlyphAndCache>(
        &glyph.fPathIsSet, &fScalerMutex, &SkGlyphCache::OnceFillInPath, {this, &glyph});
    return sk_atomic_load(&glyph.fPath, sk_memory_order_seq_cst);
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
    msg.printf(
        "cache typeface:%x %25s:%d size:%2g [%g %g %g %g] "
        "lum:%02X devG:%d pntG:%d cntr:%d glyphs:%3d",
        face->uniqueID(), name.c_str(), face->style(), rec.fTextSize,
        matrix[SkMatrix::kMScaleX], matrix[SkMatrix::kMSkewX],
        matrix[SkMatrix::kMSkewY], matrix[SkMatrix::kMScaleY],
        rec.fLumBits & 0xFF, rec.fDeviceGamma, rec.fPaintGamma, rec.fContrast,
        fGlyphMap.count());
    SkDebugf("%s\n", msg.c_str());
}

///////////////////////////////////////////////////////////////////////////////

bool SkGlyphCache::getAuxProcData(void (*proc)(void*), void** dataPtr) const {
    // Borrow the fScalerMutex to protect the AuxProc list.
    SkAutoMutexAcquire lock(fScalerMutex);
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
    if (proc == nullptr) {
        return;
    }

    // Borrow the fScalerMutex to protect the AuxProc linked list.
    SkAutoMutexAcquire lock(fScalerMutex);
    AuxProcRec* rec = fAuxProcList;
    while (rec) {
        if (rec->fProc == proc) {
            rec->fData = data;
            return;
        }
        rec = rec->fNext;
    }
    // not found, create a new rec
    rec = new AuxProcRec;
    rec->fProc = proc;
    rec->fData = data;
    rec->fNext = fAuxProcList;
    fAuxProcList = rec;
}

///////////////////////////////////////////////////////////////////////////////

typedef SkAutoTAcquire<SkSpinlock> AutoAcquire;

size_t SkGlyphCache_Globals::setCacheSizeLimit(size_t newLimit) {
    static const size_t minLimit = 256 * 1024;
    if (newLimit < minLimit) {
        newLimit = minLimit;
    }

    AutoAcquire ac(fLock);

    size_t prevLimit = fCacheSizeLimit;
    fCacheSizeLimit = newLimit;
    this->internalPurge();
    return prevLimit;
}

int SkGlyphCache_Globals::setCacheCountLimit(int newCount) {
    if (newCount < 0) {
        newCount = 0;
    }

    AutoAcquire ac(fLock);

    int prevCount = fCacheCountLimit;
    fCacheCountLimit = newCount;
    this->internalPurge();
    return prevCount;
}

void SkGlyphCache_Globals::purgeAll() {
    AutoAcquire ac(fLock);
    this->internalPurge(fTotalMemoryUsed.load());
}

/*  This guy calls the visitor from within the mutext lock, so the visitor
    cannot:
    - take too much time
    - try to acquire the mutext again
    - call a fontscaler (which might call into the cache)
*/
SkGlyphCache* SkGlyphCache::VisitCache(
    SkTypeface* typeface, const SkDescriptor* desc, VisitProc proc, void* context) {
    if (!typeface) {
        typeface = SkTypeface::GetDefaultTypeface();
    }
    SkASSERT(desc);

    SkGlyphCache_Globals& globals = get_globals();
    SkGlyphCache*         cache;

    {
        AutoAcquire ac(globals.fLock);

        globals.validate();

        for (cache = globals.internalGetHead(); cache != nullptr; cache = cache->fNext) {
            if (cache->fDesc->equals(*desc)) {
                globals.internalMoveToHead(cache);
                cache->fMapMutex.acquireShared();
                if (!proc(cache, context)) {
                    cache->fMapMutex.releaseShared();
                    return nullptr;
                }
                // The caller will take reference on this SkGlyphCache, and the corresponding
                // Attach call will decrement the reference.
                cache->fRefCount += 1;
                return cache;
            }
        }
    }

    // Check if we can create a scaler-context before creating the glyphcache.
    // If not, we may have exhausted OS/font resources, so try purging the
    // cache once and try again.
    {
        // pass true the first time, to notice if the scalercontext failed,
        // so we can try the purge.
        SkScalerContext* ctx = typeface->createScalerContext(desc, true);
        if (nullptr == ctx) {
            get_globals().purgeAll();
            ctx = typeface->createScalerContext(desc, false);
            SkASSERT(ctx);
        }

        cache = new SkGlyphCache(typeface, desc, ctx);
    }

    AutoAcquire ac(globals.fLock);
    globals.internalAttachCacheToHead(cache);

    cache->fMapMutex.acquireShared();
    if (!proc(cache, context)) {   // need to reattach
        cache->fMapMutex.releaseShared();
        return nullptr;
    }
    // The caller will take reference on this SkGlyphCache, and the corresponding
    // Attach call will decrement the reference.
    cache->fRefCount += 1;
    return cache;
}

void SkGlyphCache::AttachCache(SkGlyphCache* cache) {
    SkASSERT(cache);
    cache->fMapMutex.releaseShared();
    SkGlyphCache_Globals& globals = get_globals();
    AutoAcquire ac(globals.fLock);
    globals.validate();
    cache->validate();

    // Unref and delete if no longer in the LRU list.
    cache->fRefCount -= 1;
    if (cache->fRefCount == 0) {
        delete cache;
    }
    globals.internalPurge();
}

static void dump_visitor(const SkGlyphCache& cache, void* context) {
    int* counter = (int*)context;
    int index = *counter;
    *counter += 1;

    const SkScalerContextRec& rec = cache.getScalerContext()->getRec();

    SkDebugf("[%3d] ID %3d, glyphs %3d, size %g, scale %g, skew %g, [%g %g %g %g]\n",
             index, rec.fFontID, cache.countCachedGlyphs(),
             rec.fTextSize, rec.fPreScaleX, rec.fPreSkewX,
             rec.fPost2x2[0][0], rec.fPost2x2[0][1], rec.fPost2x2[1][0], rec.fPost2x2[1][1]);
}

void SkGlyphCache::Dump() {
    SkDebugf("GlyphCache [     used    budget ]\n");
    SkDebugf("    bytes  [ %8zu  %8zu ]\n",
             SkGraphics::GetFontCacheUsed(), SkGraphics::GetFontCacheLimit());
    SkDebugf("    count  [ %8zu  %8zu ]\n",
             SkGraphics::GetFontCacheCountUsed(), SkGraphics::GetFontCacheCountLimit());

    int counter = 0;
    SkGlyphCache::VisitAll(dump_visitor, &counter);
}

static void sk_trace_dump_visitor(const SkGlyphCache& cache, void* context) {
    SkGlyphCacheDumpContext* dumpContext = static_cast<SkGlyphCacheDumpContext*>(context);
    SkTraceMemoryDump* dump = dumpContext->dump;
    int* counter = dumpContext->counter;
    int index = *counter;
    *counter += 1;

    const SkTypeface* face = cache.getScalerContext()->getTypeface();
    SkString font_name;
    face->getFamilyName(&font_name);
    const SkScalerContextRec& rec = cache.getScalerContext()->getRec();

    SkString dump_name = SkStringPrintf("%s/%s_%3d/index_%d",
                                        gGlyphCacheDumpName, font_name.c_str(), rec.fFontID, index);

    dump->dumpNumericValue(dump_name.c_str(), "size", "bytes", cache.getMemoryUsed());
    dump->dumpNumericValue(dump_name.c_str(), "glyph_count", "objects", cache.countCachedGlyphs());
    dump->setMemoryBacking(dump_name.c_str(), "malloc", nullptr);
}

void SkGlyphCache::DumpMemoryStatistics(SkTraceMemoryDump* dump) {
    dump->dumpNumericValue(gGlyphCacheDumpName, "size", "bytes", SkGraphics::GetFontCacheUsed());
    dump->dumpNumericValue(gGlyphCacheDumpName, "budget_size", "bytes",
                           SkGraphics::GetFontCacheLimit());
    dump->dumpNumericValue(gGlyphCacheDumpName, "glyph_count", "objects",
                           SkGraphics::GetFontCacheCountUsed());
    dump->dumpNumericValue(gGlyphCacheDumpName, "budget_glyph_count", "objects",
                           SkGraphics::GetFontCacheCountLimit());

    int counter = 0;
    SkGlyphCacheDumpContext context = { &counter, dump };
    SkGlyphCache::VisitAll(sk_trace_dump_visitor, &context);
}

void SkGlyphCache::VisitAll(Visitor visitor, void* context) {
    SkGlyphCache_Globals& globals = get_globals();
    AutoAcquire           ac(globals.fLock);
    SkGlyphCache*         cache;

    globals.validate();

    for (cache = globals.internalGetHead(); cache != nullptr; cache = cache->fNext) {
        visitor(*cache, context);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkGlyphCache_Globals::internalAttachCacheToHead(SkGlyphCache* cache) {
    this->internalPurge();
    fCacheCount += 1;
    cache->fRefCount += 1;
    // Access to cache->fMemoryUsed is single threaded until internalMoveToHead.
    fTotalMemoryUsed.fetch_add(cache->fMemoryUsed);

    this->internalMoveToHead(cache);

    this->validate();
    cache->validate();
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
    if (fTotalMemoryUsed.load() > fCacheSizeLimit) {
        bytesNeeded = fTotalMemoryUsed.load() - fCacheSizeLimit;
    }
    bytesNeeded = SkTMax(bytesNeeded, minBytesNeeded);
    if (bytesNeeded) {
        // no small purges!
        bytesNeeded = SkTMax(bytesNeeded, fTotalMemoryUsed.load() >> 2);
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
    while (cache != nullptr &&
           (bytesFreed < bytesNeeded || countFreed < countNeeded)) {
        SkGlyphCache* prev = cache->fPrev;
        bytesFreed += cache->fMemoryUsed;
        countFreed += 1;
        this->internalDetachCache(cache);
        if (0 == cache->fRefCount) {
            delete cache;
        }
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

void SkGlyphCache_Globals::internalMoveToHead(SkGlyphCache *cache) {
    if (cache != fHead) {
        if (cache->fPrev) {
            cache->fPrev->fNext = cache->fNext;
        }
        if (cache->fNext) {
            cache->fNext->fPrev = cache->fPrev;
        }
        cache->fNext = nullptr;
        cache->fPrev = nullptr;
        if (fHead) {
            fHead->fPrev = cache;
            cache->fNext = fHead;
        }
        fHead = cache;
    }
}

void SkGlyphCache_Globals::internalDetachCache(SkGlyphCache* cache) {
    fCacheCount -= 1;
    fTotalMemoryUsed.fetch_sub(cache->fMemoryUsed);

    if (cache->fPrev) {
        cache->fPrev->fNext = cache->fNext;
    } else {
        // If cache->fPrev == nullptr then this is the head node.
        fHead = cache->fNext;
        if (fHead != nullptr) {
            fHead->fPrev = nullptr;
        }
    }
    if (cache->fNext) {
        cache->fNext->fPrev = cache->fPrev;
    } else {
        // If cache->fNext == nullptr then this is the last node.
        if (cache->fPrev != nullptr) {
            cache->fPrev->fNext = nullptr;
        }
    }
    cache->fPrev = cache->fNext = nullptr;
    cache->fRefCount -= 1;
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
    int computedCount = 0;

    SkGlyphCache* head = fHead;
    while (head != nullptr) {
        computedCount += 1;
        head = head->fNext;
    }

    SkASSERTF(fCacheCount == computedCount, "fCacheCount: %d, computedCount: %d", fCacheCount,
              computedCount);
}

#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "SkTypefaceCache.h"

size_t SkGraphics::GetFontCacheLimit() {
    return get_globals().getCacheSizeLimit();
}

size_t SkGraphics::SetFontCacheLimit(size_t bytes) {
    return get_globals().setCacheSizeLimit(bytes);
}

size_t SkGraphics::GetFontCacheUsed() {
    return get_globals().getTotalMemoryUsed();
}

int SkGraphics::GetFontCacheCountLimit() {
    return get_globals().getCacheCountLimit();
}

int SkGraphics::SetFontCacheCountLimit(int count) {
    return get_globals().setCacheCountLimit(count);
}

int SkGraphics::GetFontCacheCountUsed() {
    return get_globals().getCacheCountUsed();
}

void SkGraphics::PurgeFontCache() {
    get_globals().purgeAll();
    SkTypefaceCache::PurgeAll();
}

// TODO(herb): clean up TLS apis.
size_t SkGraphics::GetTLSFontCacheLimit() { return 0; }
void SkGraphics::SetTLSFontCacheLimit(size_t bytes) { }

/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphCache.h"
#include "SkGlyphCache_Globals.h"
#include "SkGraphics.h"
#include "SkOncePtr.h"
#include "SkPath.h"
#include "SkTemplates.h"
#include "SkTraceMemoryDump.h"
#include "SkTypeface.h"

#include <cctype>

//#define SPEW_PURGE_STATUS

namespace {
const char gGlyphCacheDumpName[] = "skia/sk_glyph_cache";
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
    : fDesc(desc->copy())
    , fScalerContext(ctx)
    , fGlyphAlloc(kMinAllocAmount) {
    SkASSERT(typeface);
    SkASSERT(desc);
    SkASSERT(ctx);

    fPrev = fNext = nullptr;

    fScalerContext->getFontMetrics(&fFontMetrics);

    fMemoryUsed = sizeof(*this);

    fAuxProcList = nullptr;
}

SkGlyphCache::~SkGlyphCache() {
    fGlyphMap.foreach ([](SkGlyph* g) {
        if (g->fPathData) {
            delete g->fPathData->fPath;
        } } );
    SkDescriptor::Free(fDesc);
    delete fScalerContext;
    this->invokeAndRemoveAuxProcs();
}

SkGlyphCache::CharGlyphRec* SkGlyphCache::getCharGlyphRec(PackedUnicharID packedUnicharID) {
    if (nullptr == fPackedUnicharIDToPackedGlyphID.get()) {
        // Allocate the array.
        fPackedUnicharIDToPackedGlyphID.reset(kHashCount);
        // Initialize array to map character and position with the impossible glyph ID. This
        // represents no mapping.
        for (int i = 0; i <kHashCount; ++i) {
            fPackedUnicharIDToPackedGlyphID[i].fPackedUnicharID = SkGlyph::kImpossibleID;
            fPackedUnicharIDToPackedGlyphID[i].fPackedGlyphID = 0;
        }
    }

    return &fPackedUnicharIDToPackedGlyphID[SkChecksum::CheapMix(packedUnicharID) & kHashMask];
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

    if (rec.fPackedUnicharID == packedUnicharID) {
        return SkGlyph::ID2Code(rec.fPackedGlyphID);
    } else {
        return fScalerContext->charToGlyphID(charCode);
    }
}

SkUnichar SkGlyphCache::glyphToUnichar(uint16_t glyphID) {
    return fScalerContext->glyphIDToChar(glyphID);
}

unsigned SkGlyphCache::getGlyphCount() const {
    return fScalerContext->getGlyphCount();
}

int SkGlyphCache::countCachedGlyphs() const {
    return fGlyphMap.count();
}

///////////////////////////////////////////////////////////////////////////////

const SkGlyph& SkGlyphCache::getUnicharAdvance(SkUnichar charCode) {
    VALIDATE();
    return *this->lookupByChar(charCode, kJustAdvance_MetricsType);
}

const SkGlyph& SkGlyphCache::getGlyphIDAdvance(uint16_t glyphID) {
    VALIDATE();
    PackedGlyphID packedGlyphID = SkGlyph::MakeID(glyphID);
    return *this->lookupByPackedGlyphID(packedGlyphID, kJustAdvance_MetricsType);
}

///////////////////////////////////////////////////////////////////////////////

const SkGlyph& SkGlyphCache::getUnicharMetrics(SkUnichar charCode) {
    VALIDATE();
    return *this->lookupByChar(charCode, kFull_MetricsType);
}

const SkGlyph& SkGlyphCache::getUnicharMetrics(SkUnichar charCode, SkFixed x, SkFixed y) {
    VALIDATE();
    return *this->lookupByChar(charCode, kFull_MetricsType, x, y);
}

const SkGlyph& SkGlyphCache::getGlyphIDMetrics(uint16_t glyphID) {
    VALIDATE();
    PackedGlyphID packedGlyphID = SkGlyph::MakeID(glyphID);
    return *this->lookupByPackedGlyphID(packedGlyphID, kFull_MetricsType);
}

const SkGlyph& SkGlyphCache::getGlyphIDMetrics(uint16_t glyphID, SkFixed x, SkFixed y) {
    VALIDATE();
    PackedGlyphID packedGlyphID = SkGlyph::MakeID(glyphID, x, y);
    return *this->lookupByPackedGlyphID(packedGlyphID, kFull_MetricsType);
}

SkGlyph* SkGlyphCache::lookupByChar(SkUnichar charCode, MetricsType type, SkFixed x, SkFixed y) {
    PackedUnicharID id = SkGlyph::MakeID(charCode, x, y);
    CharGlyphRec* rec = this->getCharGlyphRec(id);
    if (rec->fPackedUnicharID != id) {
        // this ID is based on the UniChar
        rec->fPackedUnicharID = id;
        // this ID is based on the glyph index
        PackedGlyphID combinedID = SkGlyph::MakeID(fScalerContext->charToGlyphID(charCode), x, y);
        rec->fPackedGlyphID = combinedID;
        return this->lookupByPackedGlyphID(combinedID, type);
    } else {
        return this->lookupByPackedGlyphID(rec->fPackedGlyphID, type);
    }
}

SkGlyph* SkGlyphCache::lookupByPackedGlyphID(PackedGlyphID packedGlyphID, MetricsType type) {
    SkGlyph* glyph = fGlyphMap.find(packedGlyphID);

    if (nullptr == glyph) {
        glyph = this->allocateNewGlyph(packedGlyphID, type);
    } else {
        if (type == kFull_MetricsType && glyph->isJustAdvance()) {
           fScalerContext->getMetrics(glyph);
        }
    }
    return glyph;
}

SkGlyph* SkGlyphCache::allocateNewGlyph(PackedGlyphID packedGlyphID, MetricsType mtype) {
    fMemoryUsed += sizeof(SkGlyph);

    SkGlyph* glyphPtr;
    {
        SkGlyph glyph;
        glyph.initGlyphFromCombinedID(packedGlyphID);
        glyphPtr = fGlyphMap.set(glyph);
    }

    if (kJustAdvance_MetricsType == mtype) {
        fScalerContext->getAdvance(glyphPtr);
    } else {
        SkASSERT(kFull_MetricsType == mtype);
        fScalerContext->getMetrics(glyphPtr);
    }

    SkASSERT(glyphPtr->fID != SkGlyph::kImpossibleID);
    return glyphPtr;
}

const void* SkGlyphCache::findImage(const SkGlyph& glyph) {
    if (glyph.fWidth > 0 && glyph.fWidth < kMaxGlyphWidth) {
        if (nullptr == glyph.fImage) {
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
        if (glyph.fPathData == nullptr) {
            SkGlyph::PathData* pathData =
                    (SkGlyph::PathData* ) fGlyphAlloc.allocThrow(sizeof(SkGlyph::PathData));
            const_cast<SkGlyph&>(glyph).fPathData = pathData;
            pathData->fIntercept = nullptr;
            SkPath* path = pathData->fPath = new SkPath;
            fScalerContext->getPath(glyph, path);
            fMemoryUsed += sizeof(SkPath) + path->countPoints() * sizeof(SkPoint);
        }
    }
    return glyph.fPathData ? glyph.fPathData->fPath : nullptr;
}

#include "../pathops/SkPathOpsCubic.h"
#include "../pathops/SkPathOpsQuad.h"

static bool quad_in_bounds(const SkScalar* pts, const SkScalar bounds[2]) {
    SkScalar min = SkTMin(SkTMin(pts[0], pts[2]), pts[4]);
    if (bounds[1] < min) {
        return false;
    }
    SkScalar max = SkTMax(SkTMax(pts[0], pts[2]), pts[4]);
    return bounds[0] < max;
}

static bool cubic_in_bounds(const SkScalar* pts, const SkScalar bounds[2]) {
    SkScalar min = SkTMin(SkTMin(SkTMin(pts[0], pts[2]), pts[4]), pts[6]);
    if (bounds[1] < min) {
        return false;
    }
    SkScalar max = SkTMax(SkTMax(SkTMax(pts[0], pts[2]), pts[4]), pts[6]);
    return bounds[0] < max;
}

void SkGlyphCache::OffsetResults(const SkGlyph::Intercept* intercept, SkScalar scale,
                                 SkScalar xPos, SkScalar* array, int* count) {
    if (array) {
        array += *count;
        for (int index = 0; index < 2; index++) {
            *array++ = intercept->fInterval[index] * scale + xPos;
        }
    }
    *count += 2;
}

void SkGlyphCache::AddInterval(SkScalar val, SkGlyph::Intercept* intercept) {
    intercept->fInterval[0] = SkTMin(intercept->fInterval[0], val);
    intercept->fInterval[1] = SkTMax(intercept->fInterval[1], val);
}

void SkGlyphCache::AddPoints(const SkPoint* pts, int ptCount, const SkScalar bounds[2],
        bool yAxis, SkGlyph::Intercept* intercept) {
    for (int i = 0; i < ptCount; ++i) {
        SkScalar val = *(&pts[i].fY - yAxis);
        if (bounds[0] < val && val < bounds[1]) {
            AddInterval(*(&pts[i].fX + yAxis), intercept);
        }
    }
}

void SkGlyphCache::AddLine(const SkPoint pts[2], SkScalar axis, bool yAxis,
                     SkGlyph::Intercept* intercept) {
    SkScalar t = yAxis ? (axis - pts[0].fX) / (pts[1].fX - pts[0].fX)
            : (axis - pts[0].fY) / (pts[1].fY - pts[0].fY);
    if (0 <= t && t < 1) {   // this handles divide by zero above
        AddInterval(yAxis ? pts[0].fY + t * (pts[1].fY - pts[0].fY)
            : pts[0].fX + t * (pts[1].fX - pts[0].fX), intercept);
    }
}

void SkGlyphCache::AddQuad(const SkPoint pts[2], SkScalar axis, bool yAxis,
                     SkGlyph::Intercept* intercept) {
    SkDQuad quad;
    quad.set(pts);
    double roots[2];
    int count = yAxis ? quad.verticalIntersect(axis, roots)
            : quad.horizontalIntersect(axis, roots);
    while (--count >= 0) {
        SkPoint pt = quad.ptAtT(roots[count]).asSkPoint();
        AddInterval(*(&pt.fX + yAxis), intercept);
    }
}

void SkGlyphCache::AddCubic(const SkPoint pts[3], SkScalar axis, bool yAxis,
                      SkGlyph::Intercept* intercept) {
    SkDCubic cubic;
    cubic.set(pts);
    double roots[3];
    int count = yAxis ? cubic.verticalIntersect(axis, roots)
            : cubic.horizontalIntersect(axis, roots);
    while (--count >= 0) {
        SkPoint pt = cubic.ptAtT(roots[count]).asSkPoint();
        AddInterval(*(&pt.fX + yAxis), intercept);
    }
}

const SkGlyph::Intercept* SkGlyphCache::MatchBounds(const SkGlyph* glyph,
                                                    const SkScalar bounds[2]) {
    if (!glyph->fPathData) {
        return nullptr;
    }
    const SkGlyph::Intercept* intercept = glyph->fPathData->fIntercept;
    while (intercept) {
        if (bounds[0] == intercept->fBounds[0] && bounds[1] == intercept->fBounds[1]) {
            return intercept;
        }
        intercept = intercept->fNext;
    }
    return nullptr;
}

void SkGlyphCache::findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
        bool yAxis, SkGlyph* glyph, SkScalar* array, int* count) {
    const SkGlyph::Intercept* match = MatchBounds(glyph, bounds);

    if (match) {
        if (match->fInterval[0] < match->fInterval[1]) {
            OffsetResults(match, scale, xPos, array, count);
        }
        return;
    }

    SkGlyph::Intercept* intercept =
            (SkGlyph::Intercept* ) fGlyphAlloc.allocThrow(sizeof(SkGlyph::Intercept));
    intercept->fNext = glyph->fPathData->fIntercept;
    intercept->fBounds[0] = bounds[0];
    intercept->fBounds[1] = bounds[1];
    intercept->fInterval[0] = SK_ScalarMax;
    intercept->fInterval[1] = SK_ScalarMin;
    glyph->fPathData->fIntercept = intercept;
    const SkPath* path = glyph->fPathData->fPath;
    const SkRect& pathBounds = path->getBounds();
    if (*(&pathBounds.fBottom - yAxis) < bounds[0] || bounds[1] < *(&pathBounds.fTop - yAxis)) {
        return;
    }
    SkPath::Iter iter(*path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while (SkPath::kDone_Verb != (verb = iter.next(pts))) {
        switch (verb) {
            case SkPath::kMove_Verb:
                break;
            case SkPath::kLine_Verb:
                AddLine(pts, bounds[0], yAxis, intercept);
                AddLine(pts, bounds[1], yAxis, intercept);
                AddPoints(pts, 2, bounds, yAxis, intercept);
                break;
            case SkPath::kQuad_Verb:
                if (!quad_in_bounds(&pts[0].fY - yAxis, bounds)) {
                    break;
                }
                AddQuad(pts, bounds[0], yAxis, intercept);
                AddQuad(pts, bounds[1], yAxis, intercept);
                AddPoints(pts, 3, bounds, yAxis, intercept);
                break;
            case SkPath::kConic_Verb:
                SkASSERT(0);  // no support for text composed of conics
                break;
            case SkPath::kCubic_Verb:
                if (!cubic_in_bounds(&pts[0].fY - yAxis, bounds)) {
                    break;
                }
                AddCubic(pts, bounds[0], yAxis, intercept);
                AddCubic(pts, bounds[1], yAxis, intercept);
                AddPoints(pts, 4, bounds, yAxis, intercept);
                break;
            case SkPath::kClose_Verb:
                break;
            default:
                SkASSERT(0);
                break;
        }
    }
    if (intercept->fInterval[0] >= intercept->fInterval[1]) {
        intercept->fInterval[0] = SK_ScalarMax;
        intercept->fInterval[1] = SK_ScalarMin;
        return;
    }
    OffsetResults(intercept, scale, xPos, array, count);
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
               fGlyphMap.count());
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
    if (proc == nullptr) {
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
    rec = new AuxProcRec;
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
        delete rec;
        rec = next;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef SkAutoTExclusive<SkSpinlock> Exclusive;

size_t SkGlyphCache_Globals::setCacheSizeLimit(size_t newLimit) {
    static const size_t minLimit = 256 * 1024;
    if (newLimit < minLimit) {
        newLimit = minLimit;
    }

    Exclusive ac(fLock);

    size_t prevLimit = fCacheSizeLimit;
    fCacheSizeLimit = newLimit;
    this->internalPurge();
    return prevLimit;
}

int SkGlyphCache_Globals::setCacheCountLimit(int newCount) {
    if (newCount < 0) {
        newCount = 0;
    }

    Exclusive ac(fLock);

    int prevCount = fCacheCountLimit;
    fCacheCountLimit = newCount;
    this->internalPurge();
    return prevCount;
}

void SkGlyphCache_Globals::purgeAll() {
    Exclusive ac(fLock);
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

    SkGlyphCache_Globals& globals = get_globals();
    SkGlyphCache*         cache;

    {
        Exclusive ac(globals.fLock);

        globals.validate();

        for (cache = globals.internalGetHead(); cache != nullptr; cache = cache->fNext) {
            if (cache->fDesc->equals(*desc)) {
                globals.internalDetachCache(cache);
                if (!proc(cache, context)) {
                    globals.internalAttachCacheToHead(cache);
                    cache = nullptr;
                }
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
        if (!ctx) {
            get_globals().purgeAll();
            ctx = typeface->createScalerContext(desc, false);
            SkASSERT(ctx);
        }
        cache = new SkGlyphCache(typeface, desc, ctx);
    }

    AutoValidate av(cache);

    if (!proc(cache, context)) {   // need to reattach
        globals.attachCacheToHead(cache);
        cache = nullptr;
    }
    return cache;
}

void SkGlyphCache::AttachCache(SkGlyphCache* cache) {
    SkASSERT(cache);
    SkASSERT(cache->fNext == nullptr);

    get_globals().attachCacheToHead(cache);
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
    SkTraceMemoryDump* dump = static_cast<SkTraceMemoryDump*>(context);

    const SkTypeface* face = cache.getScalerContext()->getTypeface();
    const SkScalerContextRec& rec = cache.getScalerContext()->getRec();

    SkString fontName;
    face->getFamilyName(&fontName);
    // Replace all special characters with '_'.
    for (size_t index = 0; index < fontName.size(); ++index) {
        if (!std::isalnum(fontName[index])) {
            fontName[index] = '_';
        }
    }

    SkString dumpName = SkStringPrintf("%s/%s_%d/%p",
                                       gGlyphCacheDumpName, fontName.c_str(), rec.fFontID, &cache);

    dump->dumpNumericValue(dumpName.c_str(), "size", "bytes", cache.getMemoryUsed());
    dump->dumpNumericValue(dumpName.c_str(), "glyph_count", "objects", cache.countCachedGlyphs());
    dump->setMemoryBacking(dumpName.c_str(), "malloc", nullptr);
}

void SkGlyphCache::DumpMemoryStatistics(SkTraceMemoryDump* dump) {
    dump->dumpNumericValue(gGlyphCacheDumpName, "size", "bytes", SkGraphics::GetFontCacheUsed());
    dump->dumpNumericValue(gGlyphCacheDumpName, "budget_size", "bytes",
                           SkGraphics::GetFontCacheLimit());
    dump->dumpNumericValue(gGlyphCacheDumpName, "glyph_count", "objects",
                           SkGraphics::GetFontCacheCountUsed());
    dump->dumpNumericValue(gGlyphCacheDumpName, "budget_glyph_count", "objects",
                           SkGraphics::GetFontCacheCountLimit());

    if (dump->getRequestedDetails() == SkTraceMemoryDump::kLight_LevelOfDetail) {
        dump->setMemoryBacking(gGlyphCacheDumpName, "malloc", nullptr);
        return;
    }

    SkGlyphCache::VisitAll(sk_trace_dump_visitor, dump);
}

void SkGlyphCache::VisitAll(Visitor visitor, void* context) {
    SkGlyphCache_Globals& globals = get_globals();
    Exclusive ac(globals.fLock);
    SkGlyphCache*         cache;

    globals.validate();

    for (cache = globals.internalGetHead(); cache != nullptr; cache = cache->fNext) {
        visitor(*cache, context);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkGlyphCache_Globals::attachCacheToHead(SkGlyphCache* cache) {
    Exclusive ac(fLock);

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
    while (cache != nullptr &&
           (bytesFreed < bytesNeeded || countFreed < countNeeded)) {
        SkGlyphCache* prev = cache->fPrev;
        bytesFreed += cache->fMemoryUsed;
        countFreed += 1;

        this->internalDetachCache(cache);
        delete cache;
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
    SkASSERT(nullptr == cache->fPrev && nullptr == cache->fNext);
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
    cache->fPrev = cache->fNext = nullptr;
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
    while (head != nullptr) {
        computedBytes += head->fMemoryUsed;
        computedCount += 1;
        head = head->fNext;
    }

    SkASSERTF(fCacheCount == computedCount, "fCacheCount: %d, computedCount: %d", fCacheCount,
              computedCount);
    SkASSERTF(fTotalMemoryUsed == computedBytes, "fTotalMemoryUsed: %d, computedBytes: %d",
              fTotalMemoryUsed, computedBytes);
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

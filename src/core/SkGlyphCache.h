/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
 */

#ifndef SkGlyphCache_DEFINED
#define SkGlyphCache_DEFINED

#include "SkArenaAlloc.h"
#include "SkDescriptor.h"
#include "SkGlyph.h"
#include "SkGlyphCache_Globals.h"
#include "SkPaint.h"
#include "SkTHash.h"
#include "SkScalerContext.h"
#include "SkTemplates.h"
#include <memory>

class SkTraceMemoryDump;

class SkGlyphCache;
using SkExclusiveStrikePtr = std::unique_ptr<
    SkGlyphCache,
    SkFunctionWrapper<void, SkGlyphCache, SkGlyphCache_Globals::AttachCache>>;

/** \class SkGlyphCache

    This class represents a strike: a specific combination of typeface, size, matrix, etc., and
    holds the glyphs for that strike. Calling any of the getUnichar.../getGlyphID... methods will
    return the requested glyph, either instantly if it is already cached, or by first generating
    it and then adding it to the strike.

    The strikes are held in a global list, available to all threads. To interact with one, call
    either Find{OrCreate}Exclusive().

    The Find*Exclusive() method returns SkExclusiveStrikePtr, which releases exclusive ownership
    when they go out of scope.
*/
class SkGlyphCache {
public:
    /** Return true if glyph is cached. */
    bool isGlyphCached(SkGlyphID glyphID, SkFixed x, SkFixed y) const;

    /**  Return a glyph that has no information if it is not already filled out. */
    SkGlyph* getRawGlyphByID(SkPackedGlyphID);

    /** Return the Strike's SkArenaAlloc. */
    SkArenaAlloc* getAlloc() {
        return &fAlloc;
    }

    /** Returns a glyph with valid fAdvance and fDevKern fields. The remaining fields may be
        valid, but that is not guaranteed. If you require those, call getUnicharMetrics or
        getGlyphIDMetrics instead.
    */
    const SkGlyph& getUnicharAdvance(SkUnichar);
    const SkGlyph& getGlyphIDAdvance(SkGlyphID);

    /** Returns a glyph with all fields valid except fImage and fPath, which may be null. If they
        are null, call findImage or findPath for those. If they are not null, then they are valid.

        This call is potentially slower than the matching ...Advance call. If you only need the
        fAdvance/fDevKern fields, call those instead.
    */
    const SkGlyph& getUnicharMetrics(SkUnichar);
    const SkGlyph& getGlyphIDMetrics(SkGlyphID);

    /** These are variants that take the device position of the glyph. Call these only if you are
        drawing in subpixel mode. Passing 0, 0 is effectively the same as calling the variants
        w/o the extra params, though a tiny bit slower.
    */
    const SkGlyph& getUnicharMetrics(SkUnichar, SkFixed x, SkFixed y);
    const SkGlyph& getGlyphIDMetrics(uint16_t, SkFixed x, SkFixed y);

    /** Return the glyphID for the specified Unichar. If the char has already been seen, use the
        existing cache entry. If not, ask the scalercontext to compute it for us.
    */
    SkGlyphID unicharToGlyph(SkUnichar);

    /** Map the glyph to its Unicode equivalent. Unmappable glyphs map to a character code of zero.
    */
    SkUnichar glyphToUnichar(SkGlyphID);

    /** Returns the number of glyphs for this strike.
    */
    unsigned getGlyphCount() const;

    /** Return the number of glyphs currently cached. */
    int countCachedGlyphs() const;

    /** Return the image associated with the glyph. If it has not been generated this will
        trigger that.
    */
    const void* findImage(const SkGlyph&);

    /** If the advance axis intersects the glyph's path, append the positions scaled and offset
        to the array (if non-null), and set the count to the updated array length.
    */
    void findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
                        bool yAxis, SkGlyph* , SkScalar* array, int* count);

    /** Return the Path associated with the glyph. If it has not been generated this will trigger
        that.
    */
    const SkPath* findPath(const SkGlyph&);

    /** Return the vertical metrics for this strike.
    */
    const SkPaint::FontMetrics& getFontMetrics() const {
        return fFontMetrics;
    }

    const SkDescriptor& getDescriptor() const { return *fDesc; }

    SkMask::Format getMaskFormat() const {
        return fScalerContext->getMaskFormat();
    }

    bool isSubpixel() const {
        return fScalerContext->isSubpixel();
    }

    /** Return the approx RAM usage for this cache. */
    size_t getMemoryUsed() const { return fMemoryUsed; }

    void dump() const;

    static std::unique_ptr<SkScalerContext> CreateScalerContext(
            const SkDescriptor& desc,
            const SkScalerContextEffects& effects,
            const SkTypeface& typeface);

    SkScalerContext* getScalerContext() const { return fScalerContext.get(); }

    static SkExclusiveStrikePtr FindStrikeExclusive(const SkDescriptor& desc);

    static SkExclusiveStrikePtr FindOrCreateStrikeExclusive(
        const SkDescriptor& desc,
        const SkScalerContextEffects& effects,
        const SkTypeface& typeface);

    static SkExclusiveStrikePtr FindOrCreateStrikeExclusive(
        const SkPaint& paint,
        const SkSurfaceProps* surfaceProps,
        SkScalerContextFlags scalerContextFlags,
        const SkMatrix* deviceMatrix);

    static SkExclusiveStrikePtr FindOrCreateStrikeExclusive(const SkPaint& paint) {
        return FindOrCreateStrikeExclusive(
                paint, nullptr, SkScalerContextFlags::kFakeGammaAndBoostContrast, nullptr);
    }

    static SkExclusiveStrikePtr CreateStrikeExclusive(
        const SkDescriptor& desc,
        std::unique_ptr<SkScalerContext> scaler,
        SkPaint::FontMetrics* maybeMetrics = nullptr);

    static void Dump();

    /** Dump memory usage statistics of all the attaches caches in the process using the
        SkTraceMemoryDump interface.
    */
    static void DumpMemoryStatistics(SkTraceMemoryDump* dump);

    static void ForEachStrike(std::function<void(const SkGlyphCache&)> visitor);

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

    class AutoValidate : SkNoncopyable {
    public:
        AutoValidate(const SkGlyphCache* cache) : fCache(cache) {
            if (fCache) {
                fCache->validate();
            }
        }
        ~AutoValidate() {
            if (fCache) {
                fCache->validate();
            }
        }
        void forget() {
            fCache = nullptr;
        }
    private:
        const SkGlyphCache* fCache;
    };

private:
    friend class SkGlyphCache_Globals;

    enum MetricsType {
        kNothing_MetricsType,
        kJustAdvance_MetricsType,
        kFull_MetricsType
    };

    enum {
        kHashBits  = 8,
        kHashCount = 1 << kHashBits,
        kHashMask  = kHashCount - 1
    };

    struct CharGlyphRec {
        SkPackedUnicharID fPackedUnicharID;
        SkPackedGlyphID fPackedGlyphID;
    };

    SkGlyphCache(const SkDescriptor& desc, std::unique_ptr<SkScalerContext> scaler,
                 const SkPaint::FontMetrics&);
    ~SkGlyphCache();

    // Return the SkGlyph* associated with MakeID. The id parameter is the
    // combined glyph/x/y id generated by MakeID. If it is just a glyph id
    // then x and y are assumed to be zero.
    SkGlyph* lookupByPackedGlyphID(SkPackedGlyphID packedGlyphID, MetricsType type);

    // Return a SkGlyph* associated with unicode id and position x and y.
    SkGlyph* lookupByChar(SkUnichar id, MetricsType type, SkFixed x = 0, SkFixed y = 0);

    // Return a new SkGlyph for the glyph ID and subpixel position id. Limit the amount
    // of work using type.
    SkGlyph* allocateNewGlyph(SkPackedGlyphID packedGlyphID, MetricsType type);

    // The id arg is a combined id generated by MakeID.
    CharGlyphRec* getCharGlyphRec(SkPackedUnicharID id);

    static void OffsetResults(const SkGlyph::Intercept* intercept, SkScalar scale,
                              SkScalar xPos, SkScalar* array, int* count);
    static void AddInterval(SkScalar val, SkGlyph::Intercept* intercept);
    static void AddPoints(const SkPoint* pts, int ptCount, const SkScalar bounds[2],
                          bool yAxis, SkGlyph::Intercept* intercept);
    static void AddLine(const SkPoint pts[2], SkScalar axis, bool yAxis,
                        SkGlyph::Intercept* intercept);
    static void AddQuad(const SkPoint pts[2], SkScalar axis, bool yAxis,
                        SkGlyph::Intercept* intercept);
    static void AddCubic(const SkPoint pts[3], SkScalar axis, bool yAxis,
                         SkGlyph::Intercept* intercept);
    static const SkGlyph::Intercept* MatchBounds(const SkGlyph* glyph,
                                                 const SkScalar bounds[2]);

    SkGlyphCache*          fNext{nullptr};
    SkGlyphCache*          fPrev{nullptr};
    const std::unique_ptr<SkDescriptor> fDesc;
    const std::unique_ptr<SkScalerContext> fScalerContext;
    SkPaint::FontMetrics   fFontMetrics;

    // Map from a combined GlyphID and sub-pixel position to a SkGlyph.
    SkTHashTable<SkGlyph, SkPackedGlyphID, SkGlyph::HashTraits> fGlyphMap;

    // so we don't grow our arrays a lot
    static constexpr size_t kMinGlyphCount = 8;
    static constexpr size_t kMinGlyphImageSize = 16 /* height */ * 8 /* width */;
    static constexpr size_t kMinAllocAmount = kMinGlyphImageSize * kMinGlyphCount;

    SkArenaAlloc            fAlloc {kMinAllocAmount};

    std::unique_ptr<CharGlyphRec[]> fPackedUnicharIDToPackedGlyphID;

    // used to track (approx) how much ram is tied-up in this cache
    size_t                  fMemoryUsed;
};

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
class SkAutoGlyphCacheNoGamma : public SkExclusiveStrikePtr {
public:
    SkAutoGlyphCacheNoGamma(const SkPaint& paint,
                            const SkSurfaceProps* surfaceProps,
                            const SkMatrix* matrix)
        : INHERITED(SkGlyphCache::FindOrCreateStrikeExclusive(
            paint, surfaceProps, SkScalerContextFlags::kNone, matrix)) {}

private:
    using INHERITED = SkExclusiveStrikePtr;
};
#define SkAutoGlyphCacheNoGamma(...) SK_REQUIRE_LOCAL_VAR(SkAutoGlyphCacheNoGamma)
#endif

#endif

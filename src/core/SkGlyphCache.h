/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
 */

#ifndef SkGlyphCache_DEFINED
#define SkGlyphCache_DEFINED

#include "SkBitmap.h"
#include "SkChunkAlloc.h"
#include "SkDescriptor.h"
#include "SkGlyph.h"
#include "SkPaint.h"
#include "SkTHash.h"
#include "SkScalerContext.h"
#include "SkTemplates.h"
#include "SkTDArray.h"

class SkTraceMemoryDump;

class SkGlyphCache_Globals;

/** \class SkGlyphCache

    This class represents a strike: a specific combination of typeface, size, matrix, etc., and
    holds the glyphs for that strike. Calling any of the getUnichar.../getGlyphID... methods will
    return the requested glyph, either instantly if it is already cached, or by first generating
    it and then adding it to the strike.

    The strikes are held in a global list, available to all threads. To interact with one, call
    either VisitCache() or DetachCache().
*/
class SkGlyphCache {
public:
    /** Returns a glyph with valid fAdvance and fDevKern fields. The remaining fields may be
        valid, but that is not guaranteed. If you require those, call getUnicharMetrics or
        getGlyphIDMetrics instead.
    */
    const SkGlyph& getUnicharAdvance(SkUnichar);
    const SkGlyph& getGlyphIDAdvance(uint16_t);

    /** Returns a glyph with all fields valid except fImage and fPath, which may be null. If they
        are null, call findImage or findPath for those. If they are not null, then they are valid.

        This call is potentially slower than the matching ...Advance call. If you only need the
        fAdvance/fDevKern fields, call those instead.
    */
    const SkGlyph& getUnicharMetrics(SkUnichar);
    const SkGlyph& getGlyphIDMetrics(uint16_t);

    /** These are variants that take the device position of the glyph. Call these only if you are
        drawing in subpixel mode. Passing 0, 0 is effectively the same as calling the variants
        w/o the extra params, though a tiny bit slower.
    */
    const SkGlyph& getUnicharMetrics(SkUnichar, SkFixed x, SkFixed y);
    const SkGlyph& getGlyphIDMetrics(uint16_t, SkFixed x, SkFixed y);

    /** Return the glyphID for the specified Unichar. If the char has already been seen, use the
        existing cache entry. If not, ask the scalercontext to compute it for us.
    */
    uint16_t unicharToGlyph(SkUnichar);

    /** Map the glyph to its Unicode equivalent. Unmappable glyphs map to a character code of zero.
    */
    SkUnichar glyphToUnichar(uint16_t);

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

    /** AuxProc/Data allow a client to associate data with this cache entry. Multiple clients can
        use this, as their data is keyed with a function pointer. In addition to serving as a
        key, the function pointer is called with the data when the glyphcache object is deleted,
        so the client can cleanup their data as well.
        NOTE: the auxProc must not try to access this glyphcache in any way, since it may be in
        the process of being deleted.
    */

    //! If the proc is found, return true and set *dataPtr to its data
    bool getAuxProcData(void (*auxProc)(void*), void** dataPtr) const;

    //! Add a proc/data pair to the glyphcache. proc should be non-null
    void setAuxProc(void (*auxProc)(void*), void* auxData);

    SkScalerContext* getScalerContext() const { return fScalerContext; }

    /** Find a matching cache entry, and call proc() with it. If none is found create a new one.
        If the proc() returns true, detach the cache and return it, otherwise leave it and return
        nullptr.
    */
    static SkGlyphCache* VisitCache(SkTypeface*, const SkDescriptor* desc,
                                    bool (*proc)(const SkGlyphCache*, void*),
                                    void* context);

    /** Given a strike that was returned by either VisitCache() or DetachCache() add it back into
        the global cache list (after which the caller should not reference it anymore.
    */
    static void AttachCache(SkGlyphCache*);
    using AttachCacheFunctor = SkFunctionWrapper<void, SkGlyphCache, AttachCache>;

    /** Detach a strike from the global cache matching the specified descriptor. Once detached,
        it can be queried/modified by the current thread, and when finished, be reattached to the
        global cache with AttachCache(). While detached, if another request is made with the same
        descriptor, a different strike will be generated. This is fine. It does mean we can have
        more than 1 strike for the same descriptor, but that will eventually get purged, and the
        win is that different thread will never block each other while a strike is being used.
    */
    static SkGlyphCache* DetachCache(SkTypeface* typeface, const SkDescriptor* desc) {
        return VisitCache(typeface, desc, DetachProc, nullptr);
    }

    static void Dump();

    /** Dump memory usage statistics of all the attaches caches in the process using the
        SkTraceMemoryDump interface.
    */
    static void DumpMemoryStatistics(SkTraceMemoryDump* dump);

    typedef void (*Visitor)(const SkGlyphCache&, void* context);
    static void VisitAll(Visitor, void* context);

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
        kJustAdvance_MetricsType,
        kFull_MetricsType
    };

    enum {
        kHashBits           = 8,
        kHashCount          = 1 << kHashBits,
        kHashMask           = kHashCount - 1
    };

    typedef uint32_t PackedGlyphID;    // glyph-index + subpixel-pos
    typedef uint32_t PackedUnicharID;  // unichar + subpixel-pos

    struct CharGlyphRec {
        PackedUnicharID    fPackedUnicharID;
        PackedGlyphID      fPackedGlyphID;
    };

    struct AuxProcRec {
        AuxProcRec* fNext;
        void (*fProc)(void*);
        void* fData;
    };

    // SkGlyphCache takes ownership of the scalercontext.
    SkGlyphCache(SkTypeface*, const SkDescriptor*, SkScalerContext*);
    ~SkGlyphCache();

    // Return the SkGlyph* associated with MakeID. The id parameter is the
    // combined glyph/x/y id generated by MakeID. If it is just a glyph id
    // then x and y are assumed to be zero.
    SkGlyph* lookupByPackedGlyphID(PackedGlyphID packedGlyphID, MetricsType type);

    // Return a SkGlyph* associated with unicode id and position x and y.
    SkGlyph* lookupByChar(SkUnichar id, MetricsType type, SkFixed x = 0, SkFixed y = 0);

    // Return a new SkGlyph for the glyph ID and subpixel position id. Limit the amount
    // of work
    // using type.
    SkGlyph* allocateNewGlyph(PackedGlyphID packedGlyphID, MetricsType type);

    static bool DetachProc(const SkGlyphCache*, void*) { return true; }

    // The id arg is a combined id generated by MakeID.
    CharGlyphRec* getCharGlyphRec(PackedUnicharID id);

    void invokeAndRemoveAuxProcs();

    inline static SkGlyphCache* FindTail(SkGlyphCache* head);

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

    SkGlyphCache*          fNext;
    SkGlyphCache*          fPrev;
    SkDescriptor* const    fDesc;
    SkScalerContext* const fScalerContext;
    SkPaint::FontMetrics   fFontMetrics;

    // Map from a combined GlyphID and sub-pixel position to a SkGlyph.
    SkTHashTable<SkGlyph, PackedGlyphID, SkGlyph::HashTraits> fGlyphMap;

    SkChunkAlloc           fGlyphAlloc;

    SkAutoTArray<CharGlyphRec> fPackedUnicharIDToPackedGlyphID;

    // used to track (approx) how much ram is tied-up in this cache
    size_t                 fMemoryUsed;

    AuxProcRec*            fAuxProcList;
};

class SkAutoGlyphCache : public std::unique_ptr<SkGlyphCache, SkGlyphCache::AttachCacheFunctor> {
public:
    /** deprecated: use get() */
    SkGlyphCache* getCache() const { return this->get(); }

    SkAutoGlyphCache(SkGlyphCache* cache) : INHERITED(cache) {}
    SkAutoGlyphCache(SkTypeface* typeface, const SkDescriptor* desc)
        : INHERITED(SkGlyphCache::DetachCache(typeface, desc))
    {}
    /** deprecated: always enables fake gamma */
    SkAutoGlyphCache(const SkPaint& paint,
                     const SkSurfaceProps* surfaceProps,
                     const SkMatrix* matrix)
        : INHERITED(paint.detachCache(surfaceProps, SkPaint::FakeGamma::On, matrix))
    {}
    SkAutoGlyphCache(const SkPaint& paint,
                     const SkSurfaceProps* surfaceProps,
                     SkPaint::FakeGamma fakeGamma,
                     const SkMatrix* matrix)
        : INHERITED(paint.detachCache(surfaceProps, fakeGamma, matrix))
    {}
private:
    using INHERITED = std::unique_ptr<SkGlyphCache, SkGlyphCache::AttachCacheFunctor>;
};

class SkAutoGlyphCacheNoGamma : public SkAutoGlyphCache {
public:
    SkAutoGlyphCacheNoGamma(const SkPaint& paint,
                            const SkSurfaceProps* surfaceProps,
                            const SkMatrix* matrix)
        : SkAutoGlyphCache(paint, surfaceProps, SkPaint::FakeGamma::Off, matrix)
    {}
};
#define SkAutoGlyphCache(...) SK_REQUIRE_LOCAL_VAR(SkAutoGlyphCache)
#define SkAutoGlyphCacheNoGamma(...) SK_REQUIRE_LOCAL_VAR(SkAutoGlyphCacheNoGamma)

#endif

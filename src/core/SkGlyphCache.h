
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkGlyphCache_DEFINED
#define SkGlyphCache_DEFINED

#include "SkBitmap.h"
#include "SkChunkAlloc.h"
#include "SkDescriptor.h"
#include "SkScalerContext.h"
#include "SkTemplates.h"

class SkPaint;

class SkGlyphCache_Globals;

/** \class SkGlyphCache

    This class represents a strike: a specific combination of typeface, size,
    matrix, etc., and holds the glyphs for that strike. Calling any of the
    getUnichar.../getGlyphID... methods will return the requested glyph,
    either instantly if it is already cahced, or by first generating it and then
    adding it to the strike.

    The strikes are held in a global list, available to all threads. To interact
    with one, call either VisitCache() or DetachCache().
*/
class SkGlyphCache {
public:
    /** Returns a glyph with valid fAdvance and fDevKern fields.
        The remaining fields may be valid, but that is not guaranteed. If you
        require those, call getUnicharMetrics or getGlyphIDMetrics instead.
    */
    const SkGlyph& getUnicharAdvance(SkUnichar);
    const SkGlyph& getGlyphIDAdvance(uint16_t);

    /** Returns a glyph with all fields valid except fImage and fPath, which
        may be null. If they are null, call findImage or findPath for those.
        If they are not null, then they are valid.

        This call is potentially slower than the matching ...Advance call. If
        you only need the fAdvance/fDevKern fields, call those instead.
    */
    const SkGlyph& getUnicharMetrics(SkUnichar);
    const SkGlyph& getGlyphIDMetrics(uint16_t);

    /** These are variants that take the device position of the glyph. Call
        these only if you are drawing in subpixel mode. Passing 0, 0 is
        effectively the same as calling the variants w/o the extra params, tho
        a tiny bit slower.
    */
    const SkGlyph& getUnicharMetrics(SkUnichar, SkFixed x, SkFixed y);
    const SkGlyph& getGlyphIDMetrics(uint16_t, SkFixed x, SkFixed y);

    /** Return the glyphID for the specified Unichar. If the char has already
        been seen, use the existing cache entry. If not, ask the scalercontext
        to compute it for us.
    */
    uint16_t unicharToGlyph(SkUnichar);

    /** Map the glyph to its Unicode equivalent. Unmappable glyphs map to
        a character code of zero.
    */
    SkUnichar glyphToUnichar(uint16_t);

    /** Returns the number of glyphs for this strike.
    */
    unsigned getGlyphCount();

#ifdef SK_BUILD_FOR_ANDROID
    /** Returns the base glyph count for this strike.
    */
    unsigned getBaseGlyphCount(SkUnichar charCode) const {
        return fScalerContext->getBaseGlyphCount(charCode);
    }
#endif

    /** Return the image associated with the glyph. If it has not been generated
        this will trigger that.
    */
    const void* findImage(const SkGlyph&);
    /** Return the Path associated with the glyph. If it has not been generated
        this will trigger that.
    */
    const SkPath* findPath(const SkGlyph&);

    /** Return the vertical metrics for this strike.
    */
    const SkPaint::FontMetrics& getFontMetricsY() const {
        return fFontMetricsY;
    }

    const SkDescriptor& getDescriptor() const { return *fDesc; }

    SkMask::Format getMaskFormat() const {
        return fScalerContext->getMaskFormat();
    }

    bool isSubpixel() const {
        return fScalerContext->isSubpixel();
    }

    /*  AuxProc/Data allow a client to associate data with this cache entry.
        Multiple clients can use this, as their data is keyed with a function
        pointer. In addition to serving as a key, the function pointer is called
        with the data when the glyphcache object is deleted, so the client can
        cleanup their data as well. NOTE: the auxProc must not try to access
        this glyphcache in any way, since it may be in the process of being
        deleted.
    */

    //! If the proc is found, return true and set *dataPtr to its data
    bool getAuxProcData(void (*auxProc)(void*), void** dataPtr) const;
    //! Add a proc/data pair to the glyphcache. proc should be non-null
    void setAuxProc(void (*auxProc)(void*), void* auxData);
    //! If found, remove the proc/data pair from the glyphcache (does not
    //  call the proc)
    void removeAuxProc(void (*auxProc)(void*));

    /** Call proc on all cache entries, stopping early if proc returns true.
        The proc should not create or delete caches, since it could produce
        deadlock.
    */
    static void VisitAllCaches(bool (*proc)(SkGlyphCache*, void*), void* ctx);

    /** Find a matching cache entry, and call proc() with it. If none is found
        create a new one. If the proc() returns true, detach the cache and
        return it, otherwise leave it and return NULL.
    */
    static SkGlyphCache* VisitCache(const SkDescriptor* desc,
                                    bool (*proc)(const SkGlyphCache*, void*),
                                    void* context);

    /** Given a strike that was returned by either VisitCache() or DetachCache()
        add it back into the global cache list (after which the caller should
        not reference it anymore.
    */
    static void AttachCache(SkGlyphCache*);

    /** Detach a strike from the global cache matching the specified descriptor.
        Once detached, it can be queried/modified by the current thread, and
        when finished, be reattached to the global cache with AttachCache().
        While detached, if another request is made with the same descriptor,
        a different strike will be generated. This is fine. It does mean we
        can have more than 1 strike for the same descriptor, but that will
        eventually get purged, and the win is that different thread will never
        block each other while a strike is being used.
    */
    static SkGlyphCache* DetachCache(const SkDescriptor* desc) {
        return VisitCache(desc, DetachProc, NULL);
    }

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
            fCache = NULL;
        }
    private:
        const SkGlyphCache* fCache;
    };

private:
    SkGlyphCache(const SkDescriptor*);
    ~SkGlyphCache();

    enum MetricsType {
        kJustAdvance_MetricsType,
        kFull_MetricsType
    };

    SkGlyph* lookupMetrics(uint32_t id, MetricsType);
    static bool DetachProc(const SkGlyphCache*, void*) { return true; }

    void detach(SkGlyphCache** head) {
        if (fPrev) {
            fPrev->fNext = fNext;
        } else {
            *head = fNext;
        }
        if (fNext) {
            fNext->fPrev = fPrev;
        }
        fPrev = fNext = NULL;
    }

    void attachToHead(SkGlyphCache** head) {
        SkASSERT(NULL == fPrev && NULL == fNext);
        if (*head) {
            (*head)->fPrev = this;
            fNext = *head;
        }
        *head = this;
    }

    SkGlyphCache*       fNext, *fPrev;
    SkDescriptor*       fDesc;
    SkScalerContext*    fScalerContext;
    SkPaint::FontMetrics fFontMetricsY;

    enum {
        kHashBits   = 12,
        kHashCount  = 1 << kHashBits,
        kHashMask   = kHashCount - 1
    };
    SkGlyph*            fGlyphHash[kHashCount];
    SkTDArray<SkGlyph*> fGlyphArray;
    SkChunkAlloc        fGlyphAlloc;
    SkChunkAlloc        fImageAlloc;

    int fMetricsCount, fAdvanceCount;

    struct CharGlyphRec {
        uint32_t    fID;    // unichar + subpixel
        SkGlyph*    fGlyph;
    };
    // no reason to use the same kHashCount as fGlyphHash, but we do for now
    CharGlyphRec    fCharToGlyphHash[kHashCount];

    enum {
        // shift so that the top bits fall into kHashBits region
        kShiftForHashIndex = SkGlyph::kSubShift +
                             SkGlyph::kSubBits*2 -
                             kHashBits
    };

    static inline unsigned ID2HashIndex(uint32_t id) {
        return (id ^ (id >> kShiftForHashIndex)) & kHashMask;
    }

    // used to track (approx) how much ram is tied-up in this cache
    size_t  fMemoryUsed;

    struct AuxProcRec {
        AuxProcRec* fNext;
        void (*fProc)(void*);
        void* fData;
    };
    AuxProcRec* fAuxProcList;
    void invokeAndRemoveAuxProcs();

    // This relies on the caller to have already acquired the mutex to access the global cache
    static size_t InternalFreeCache(SkGlyphCache_Globals*, size_t bytesNeeded);

    inline static SkGlyphCache* FindTail(SkGlyphCache* head);

    friend class SkGlyphCache_Globals;
};

class SkAutoGlyphCache {
public:
    SkAutoGlyphCache(SkGlyphCache* cache) : fCache(cache) {}
    SkAutoGlyphCache(const SkDescriptor* desc) {
        fCache = SkGlyphCache::DetachCache(desc);
    }
    SkAutoGlyphCache(const SkPaint& paint, const SkMatrix* matrix) {
        fCache = paint.detachCache(matrix);
    }
    ~SkAutoGlyphCache() {
        if (fCache) {
            SkGlyphCache::AttachCache(fCache);
        }
    }

    SkGlyphCache* getCache() const { return fCache; }

    void release() {
        if (fCache) {
            SkGlyphCache::AttachCache(fCache);
            fCache = NULL;
        }
    }

private:
    SkGlyphCache*   fCache;

    static bool DetachProc(const SkGlyphCache*, void*);
};

#endif


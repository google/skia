/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
 */

#ifndef SkStrike_DEFINED
#define SkStrike_DEFINED

#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/private/SkTHash.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeInterface.h"
#include <memory>

/** \class SkGlyphCache

    This class represents a strike: a specific combination of typeface, size, matrix, etc., and
    holds the glyphs for that strike. Calling any of the getGlyphID... methods will
    return the requested glyph, either instantly if it is already cached, or by first generating
    it and then adding it to the strike.

    The strikes are held in a global list, available to all threads. To interact with one, call
    either Find{OrCreate}Exclusive().

    The Find*Exclusive() method returns SkExclusiveStrikePtr, which releases exclusive ownership
    when they go out of scope.
*/
class SkStrike final : public SkStrikeInterface {
public:
    SkStrike(const SkDescriptor& desc,
             std::unique_ptr<SkScalerContext> scaler,
             const SkFontMetrics&);

    /** Return true if glyph is cached. */
    bool isGlyphCached(SkGlyphID glyphID, SkFixed x, SkFixed y) const;

    // Return a glyph. Create it if it doesn't exist, and initialize the glyph with metrics and
    // advances.
    SkGlyph* glyph(SkPackedGlyphID id);
    SkGlyph* glyph(SkGlyphID);
    SkGlyph* glyph(SkGlyphID, SkPoint);
    SkGlyph* glyphOrNull(SkPackedGlyphID id) const;

    SkGlyph* mergeMetrics(const SkGlyph& from);
    void mergeImage(SkGlyph* glyph, const void* image, size_t size);
    void mergePath(SkGlyph* glyph, const SkPath* path);

    SkSpan<SkPoint> getAdvances(SkSpan<const SkGlyphID>, SkPoint[]);
    SkSpan<const SkGlyph*> prepareImages(
            SkSpan<const SkGlyphID> glyphIDs, SkGlyph* results[]);

    /** Returns the number of glyphs for this strike.
    */
    unsigned getGlyphCount() const;

    /** Return the number of glyphs currently cached. */
    int countCachedGlyphs() const;

    const void* ensureImage(SkGlyph*);


    /** If the advance axis intersects the glyph's path, append the positions scaled and offset
        to the array (if non-null), and set the count to the updated array length.
    */
    void findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
                        SkGlyph* , SkScalar* array, int* count);

    const SkPath* ensurePath(SkGlyph*) override;

    /** Fallback glyphs used during font remoting if the original glyph can't be found.
     */
    bool belongsToCache(const SkGlyph* glyph) const;
    /** Find any glyph in this cache with the given ID, regardless of subpixel positioning.
     *  If set and present, skip over the glyph with vetoID.
     */
    const SkGlyph* getCachedGlyphAnySubPix(SkGlyphID,
                                           SkPackedGlyphID vetoID = SkPackedGlyphID()) const;
    void initializeGlyphFromFallback(SkGlyph* glyph, const SkGlyph&);

    /** Return the vertical metrics for this strike.
    */
    const SkFontMetrics& getFontMetrics() const {
        return fFontMetrics;
    }

    SkMask::Format getMaskFormat() const {
        return fScalerContext->getMaskFormat();
    }

    bool isSubpixel() const {
        return fIsSubpixel;
    }

    SkVector rounding() const override;

    const SkGlyph& getGlyphMetrics(SkGlyphID glyphID, SkPoint position) override;

    const SkDescriptor& getDescriptor() const override;

    SkSpan<const SkGlyphPos> prepareForDrawing(const SkGlyphID glyphIDs[],
                                               const SkPoint positions[],
                                               size_t n,
                                               int maxDimension,
                                               PreparationDetail detail,
                                               SkGlyphPos results[]) override;

    void onAboutToExitScope() override;

    /** Return the approx RAM usage for this cache. */
    size_t getMemoryUsed() const { return fMemoryUsed; }

    void dump() const;

    SkScalerContext* getScalerContext() const { return fScalerContext.get(); }

#ifdef SK_DEBUG
    void forceValidate() const;
    void validate() const;
#else
    void validate() const {}
#endif

    class AutoValidate : SkNoncopyable {
    public:
        AutoValidate(const SkStrike* cache) : fCache(cache) {
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
        const SkStrike* fCache;
    };

private:
    class GlyphMapHashTraits {
    public:
        static SkPackedGlyphID GetKey(const SkGlyph* glyph) {
            return glyph->getPackedID();
        }
        static uint32_t Hash(SkPackedGlyphID glyphId) {
            return glyphId.hash();
        }
    };

    SkGlyph* makeGlyph(SkPackedGlyphID);
    SkSpan<SkGlyph*> metrics(SkSpan<const SkGlyphID>glyphIDs, SkGlyph* result[]);
    SkSpan<SkGlyphPos> metricsWithoutEmpty(
            SkSpan<const SkGlyphID>glyphIDs, const SkPoint positions[], SkGlyphPos result[]);

    const SkAutoDescriptor                 fDesc;
    const std::unique_ptr<SkScalerContext> fScalerContext;
    SkFontMetrics                          fFontMetrics;

    // Map from a combined GlyphID and sub-pixel position to a SkGlyph*.
    // The actual glyph is stored in the fAlloc. This structure provides an
    // unchanging pointer as long as the strike is alive.
    SkTHashTable<SkGlyph*, SkPackedGlyphID, GlyphMapHashTraits> fGlyphMap;

    // so we don't grow our arrays a lot
    static constexpr size_t kMinGlyphCount = 8;
    static constexpr size_t kMinGlyphImageSize = 16 /* height */ * 8 /* width */;
    static constexpr size_t kMinAllocAmount = kMinGlyphImageSize * kMinGlyphCount;

    SkArenaAlloc            fAlloc {kMinAllocAmount};

    // Tracks (approx) how much ram is tied-up in this strike.
    size_t                  fMemoryUsed;

    const bool              fIsSubpixel;
    const SkAxisAlignment   fAxisAlignment;
};

#endif  // SkStrike_DEFINED

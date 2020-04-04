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
#include "src/core/SkStrikeForGPU.h"
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
class SkStrike final : public SkStrikeForGPU {
public:
    SkStrike(const SkDescriptor& desc,
             std::unique_ptr<SkScalerContext> scaler,
             const SkFontMetrics&);

    // Return a glyph.  Create it if it doesn't exist, and initialize with the prototype.
    SkGlyph* glyphFromPrototype(const SkGlyphPrototype& p, void* image = nullptr);

    // Return a glyph or nullptr if it does not exits in the strike.
    SkGlyph* glyphOrNull(SkPackedGlyphID id) const;

    // Lookup (or create if needed) the toGlyph using toID. If that glyph is not initialized with
    // an image, then use the information in from to initialize the width, height top, left,
    // format and image of the toGlyph. This is mainly used preserving the glyph if it was
    // created by a search of desperation.
    SkGlyph* mergeGlyphAndImage(SkPackedGlyphID toID, const SkGlyph& from);

    // If the path has never been set, then add a path to glyph.
    const SkPath* preparePath(SkGlyph* glyph, const SkPath* path);

    /** Return the number of glyphs currently cached. */
    int countCachedGlyphs() const;

    /** If the advance axis intersects the glyph's path, append the positions scaled and offset
        to the array (if non-null), and set the count to the updated array length.
    */
    void findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
                        SkGlyph* , SkScalar* array, int* count);

    /** Find any glyph in this cache with the given ID, regardless of subpixel positioning.
     *  If set and present, skip over the glyph with vetoID.
     */
    const SkGlyph* getCachedGlyphAnySubPix(SkGlyphID,
                                           SkPackedGlyphID vetoID = SkPackedGlyphID()) const;

    /** Return the vertical metrics for this strike.
    */
    const SkFontMetrics& getFontMetrics() const {
        return fFontMetrics;
    }

    const SkGlyphPositionRoundingSpec& roundingSpec() const override {
        return fRoundingSpec;
    }

    const SkDescriptor& getDescriptor() const override;

    SkSpan<const SkGlyph*> metrics(SkSpan<const SkGlyphID> glyphIDs,
                                   const SkGlyph* results[]);

    SkSpan<const SkGlyph*> preparePaths(SkSpan<const SkGlyphID> glyphIDs,
                                        const SkGlyph* results[]);

    SkSpan<const SkGlyph*> prepareImages(SkSpan<const SkPackedGlyphID> glyphIDs,
                                         const SkGlyph* results[]);

    void prepareForDrawingMasksCPU(SkDrawableGlyphBuffer* drawables);

    void prepareForDrawingPathsCPU(SkDrawableGlyphBuffer* drawables);

    void prepareForMaskDrawing(
            SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) override;

    void prepareForSDFTDrawing(
            SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) override;

    void prepareForPathDrawing(
            SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) override;

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

    template <typename Fn>
    void commonFilterLoop(SkDrawableGlyphBuffer* drawables, Fn&& fn);

    // Return a glyph. Create it if it doesn't exist, and initialize the glyph with metrics and
    // advances using a scaler.
    SkGlyph* glyph(SkPackedGlyphID packedID);

    const void* prepareImage(SkGlyph* glyph);

    // If the path has never been set, then use the scaler context to add the glyph.
    const SkPath* preparePath(SkGlyph*);

    enum PathDetail {
        kMetricsOnly,
        kMetricsAndPath
    };
    // internalPrepare will only be called with a mutex already held.
    SkSpan<const SkGlyph*> internalPrepare(
            SkSpan<const SkGlyphID> glyphIDs,
            PathDetail pathDetail,
            const SkGlyph** results);

    const SkAutoDescriptor                 fDesc;
    const std::unique_ptr<SkScalerContext> fScalerContext;
    const SkFontMetrics                    fFontMetrics;

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

    const SkGlyphPositionRoundingSpec fRoundingSpec;
};

#endif  // SkStrike_DEFINED

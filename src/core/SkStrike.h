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
             const SkFontMetrics* metrics = nullptr);

    // Return a glyph.  Create it if it doesn't exist, and initialize with the prototype.
    SkGlyph* glyphFromPrototype(const SkGlyphPrototype& p, void* image = nullptr) SK_EXCLUDES(fMu);

    // Return a glyph or nullptr if it does not exits in the strike.
    SkGlyph* glyphOrNull(SkPackedGlyphID id) const SK_EXCLUDES(fMu);

    // Lookup (or create if needed) the toGlyph using toID. If that glyph is not initialized with
    // an image, then use the information in from to initialize the width, height top, left,
    // format and image of the toGlyph. This is mainly used preserving the glyph if it was
    // created by a search of desperation.
    SkGlyph* mergeGlyphAndImage(SkPackedGlyphID toID, const SkGlyph& from) SK_EXCLUDES(fMu);

    // If the path has never been set, then add a path to glyph.
    const SkPath* preparePath(SkGlyph* glyph, const SkPath* path) SK_EXCLUDES(fMu);

    /** Return the number of glyphs currently cached. */
    int countCachedGlyphs() const SK_EXCLUDES(fMu);

    /** If the advance axis intersects the glyph's path, append the positions scaled and offset
        to the array (if non-null), and set the count to the updated array length.
    */
    void findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
                        SkGlyph* , SkScalar* array, int* count) SK_EXCLUDES(fMu);


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
                                   const SkGlyph* results[]) SK_EXCLUDES(fMu);

    SkSpan<const SkGlyph*> preparePaths(SkSpan<const SkGlyphID> glyphIDs,
                                        const SkGlyph* results[]) SK_EXCLUDES(fMu);

    SkSpan<const SkGlyph*> prepareImages(SkSpan<const SkPackedGlyphID> glyphIDs,
                                         const SkGlyph* results[]) SK_EXCLUDES(fMu);

    void prepareForDrawingMasksCPU(SkDrawableGlyphBuffer* drawables) SK_EXCLUDES(fMu);

    void prepareForDrawingPathsCPU(SkDrawableGlyphBuffer* drawables) SK_EXCLUDES(fMu);

    void prepareForMaskDrawing(
            SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) override SK_EXCLUDES(fMu);

    void prepareForSDFTDrawing(
            SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) override SK_EXCLUDES(fMu);

    void prepareForPathDrawing(
            SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) override SK_EXCLUDES(fMu);

    void onAboutToExitScope() override;

    /** Return the approx RAM usage for this cache. */
    size_t getMemoryUsed() const SK_EXCLUDES(fMu) {
        SkAutoMutexExclusive lock{fMu};
        return fMemoryUsed;
    }

    void dump() const SK_EXCLUDES(fMu);

    SkScalerContext* getScalerContext() const { return fScalerContext.get(); }

#ifdef SK_DEBUG
    void forceValidate() const SK_EXCLUDES(fMu);
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

    SkGlyph* makeGlyph(SkPackedGlyphID) SK_REQUIRES(fMu);

    template <typename Fn>
    void commonFilterLoop(SkDrawableGlyphBuffer* drawables, Fn&& fn) SK_REQUIRES(fMu);

    // Return a glyph. Create it if it doesn't exist, and initialize the glyph with metrics and
    // advances using a scaler.
    SkGlyph* glyph(SkPackedGlyphID packedID) SK_REQUIRES(fMu);

    const void* prepareImage(SkGlyph* glyph) SK_REQUIRES(fMu);

    // If the path has never been set, then use the scaler context to add the glyph.
    const SkPath* preparePath(SkGlyph*) SK_REQUIRES(fMu);

    SkGlyph* internalGlyphOrNull(SkPackedGlyphID packedID) const SK_REQUIRES(fMu);

    enum PathDetail {
        kMetricsOnly,
        kMetricsAndPath
    };

    // internalPrepare will only be called with a mutex already held.
    SkSpan<const SkGlyph*> internalPrepare(
            SkSpan<const SkGlyphID> glyphIDs,
            PathDetail pathDetail,
            const SkGlyph** results) SK_REQUIRES(fMu);

    const SkAutoDescriptor                 fDesc;
    const std::unique_ptr<SkScalerContext> fScalerContext;
    const SkFontMetrics                    fFontMetrics;
    const SkGlyphPositionRoundingSpec      fRoundingSpec;

    mutable SkMutex fMu;

    // Map from a combined GlyphID and sub-pixel position to a SkGlyph*.
    // The actual glyph is stored in the fAlloc. This structure provides an
    // unchanging pointer as long as the strike is alive.
    SkTHashTable<SkGlyph*, SkPackedGlyphID, GlyphMapHashTraits> fGlyphMap SK_GUARDED_BY(fMu);

    // so we don't grow our arrays a lot
    static constexpr size_t kMinGlyphCount = 8;
    static constexpr size_t kMinGlyphImageSize = 16 /* height */ * 8 /* width */;
    static constexpr size_t kMinAllocAmount = kMinGlyphImageSize * kMinGlyphCount;

    SkArenaAlloc            fAlloc SK_GUARDED_BY(fMu) {kMinAllocAmount};

    // Tracks (approx) how much ram is tied-up in this strike.
    size_t                  fMemoryUsed SK_GUARDED_BY(fMu) {sizeof(SkStrike)};
};

#endif  // SkStrike_DEFINED

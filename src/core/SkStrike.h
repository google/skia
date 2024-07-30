/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
 */

#ifndef SkStrike_DEFINED
#define SkStrike_DEFINED

#include "include/core/SkFontMetrics.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkThreadAnnotations.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTHash.h"
#include "src/text/StrikeForGPU.h"

#include <cstddef>
#include <memory>
#include <vector>

class SkDescriptor;
class SkDrawable;
class SkPath;
class SkReadBuffer;
class SkStrikeCache;
class SkTraceMemoryDump;
class SkWriteBuffer;

class SkStrikePinner {
public:
    virtual ~SkStrikePinner() = default;
    virtual bool canDelete() = 0;
    virtual void assertValid() {}
};

// This class holds the results of an SkScalerContext, and owns a references to that scaler.
class SkStrike final : public sktext::StrikeForGPU {
public:
    SkStrike(SkStrikeCache* strikeCache,
             const SkStrikeSpec& strikeSpec,
             std::unique_ptr<SkScalerContext> scaler,
             const SkFontMetrics* metrics,
             std::unique_ptr<SkStrikePinner> pinner);

    void lock() override SK_ACQUIRE(fStrikeLock);
    void unlock() override SK_RELEASE_CAPABILITY(fStrikeLock);
    SkGlyphDigest digestFor(skglyph::ActionType, SkPackedGlyphID) override SK_REQUIRES(fStrikeLock);
    bool prepareForImage(SkGlyph* glyph) override SK_REQUIRES(fStrikeLock);
    bool prepareForPath(SkGlyph*) override SK_REQUIRES(fStrikeLock);
    bool prepareForDrawable(SkGlyph*) override SK_REQUIRES(fStrikeLock);

    bool mergeFromBuffer(SkReadBuffer& buffer) SK_EXCLUDES(fStrikeLock);
    static void FlattenGlyphsByType(SkWriteBuffer& buffer,
                                    SkSpan<SkGlyph> images,
                                    SkSpan<SkGlyph> paths,
                                    SkSpan<SkGlyph> drawables);

    // Lookup (or create if needed) the returned glyph using toID. If that glyph is not initialized
    // with an image, then use the information in fromGlyph to initialize the width, height top,
    // left, format and image of the glyph. This is mainly used preserving the glyph if it was
    // created by a search of desperation. This is deprecated.
    SkGlyph* mergeGlyphAndImage(
            SkPackedGlyphID toID, const SkGlyph& fromGlyph) SK_EXCLUDES(fStrikeLock);

    // If the path has never been set, then add a path to glyph. This is deprecated.
    const SkPath* mergePath(
            SkGlyph* glyph, const SkPath* path, bool hairline, bool modified) SK_EXCLUDES(fStrikeLock);

    // If the drawable has never been set, then add a drawable to glyph. This is deprecated.
    const SkDrawable* mergeDrawable(
            SkGlyph* glyph, sk_sp<SkDrawable> drawable) SK_EXCLUDES(fStrikeLock);

    // If the advance axis intersects the glyph's path, append the positions scaled and offset
    // to the array (if non-null), and set the count to the updated array length.
    // TODO: track memory usage.
    void findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
                        SkGlyph*, SkScalar* array, int* count) SK_EXCLUDES(fStrikeLock);

    const SkFontMetrics& getFontMetrics() const {
        return fFontMetrics;
    }

    SkSpan<const SkGlyph*> metrics(
            SkSpan<const SkGlyphID> glyphIDs, const SkGlyph* results[]) SK_EXCLUDES(fStrikeLock);

    SkSpan<const SkGlyph*> preparePaths(
            SkSpan<const SkGlyphID> glyphIDs, const SkGlyph* results[]) SK_EXCLUDES(fStrikeLock);

    SkSpan<const SkGlyph*> prepareImages(SkSpan<const SkPackedGlyphID> glyphIDs,
                                         const SkGlyph* results[]) SK_EXCLUDES(fStrikeLock);

    SkSpan<const SkGlyph*> prepareDrawables(
            SkSpan<const SkGlyphID> glyphIDs, const SkGlyph* results[]) SK_EXCLUDES(fStrikeLock);

    // SkStrikeForGPU APIs
    const SkDescriptor& getDescriptor() const override {
        return fStrikeSpec.descriptor();
    }

    const SkGlyphPositionRoundingSpec& roundingSpec() const override {
        return fRoundingSpec;
    }

    sktext::SkStrikePromise strikePromise() override {
        return sktext::SkStrikePromise(sk_ref_sp<SkStrike>(this));
    }

    // Convert all the IDs into SkPaths in the span.
    void glyphIDsToPaths(SkSpan<sktext::IDOrPath> idsOrPaths) SK_EXCLUDES(fStrikeLock);

    // Convert all the IDs into SkDrawables in the span.
    void glyphIDsToDrawables(SkSpan<sktext::IDOrDrawable> idsOrDrawables) SK_EXCLUDES(fStrikeLock);

    const SkStrikeSpec& strikeSpec() const {
        return fStrikeSpec;
    }

    void verifyPinnedStrike() const {
        if (fPinner != nullptr) {
            fPinner->assertValid();
        }
    }

    void dump() const SK_EXCLUDES(fStrikeLock);
    void dumpMemoryStatistics(SkTraceMemoryDump* dump) const SK_EXCLUDES(fStrikeLock);

    SkGlyph* glyph(SkGlyphDigest) SK_REQUIRES(fStrikeLock);

private:
    friend class SkStrikeCache;
    friend class SkStrikeTestingPeer;
    class Monitor;

    // Return a glyph. Create it if it doesn't exist, and initialize the glyph with metrics and
    // advances using a scaler.
    SkGlyph* glyph(SkPackedGlyphID) SK_REQUIRES(fStrikeLock);

    // Generate the glyph digest information and update structures to add the glyph.
    SkGlyphDigest* addGlyphAndDigest(SkGlyph* glyph) SK_REQUIRES(fStrikeLock);

    SkGlyph* mergeGlyphFromBuffer(SkReadBuffer& buffer) SK_REQUIRES(fStrikeLock);
    bool mergeGlyphAndImageFromBuffer(SkReadBuffer& buffer) SK_REQUIRES(fStrikeLock);
    bool mergeGlyphAndPathFromBuffer(SkReadBuffer& buffer) SK_REQUIRES(fStrikeLock);
    bool mergeGlyphAndDrawableFromBuffer(SkReadBuffer& buffer) SK_REQUIRES(fStrikeLock);

    // Maintain memory use statistics.
    void updateMemoryUsage(size_t increase) SK_EXCLUDES(fStrikeLock);

    enum PathDetail {
        kMetricsOnly,
        kMetricsAndPath
    };

    // internalPrepare will only be called with a mutex already held.
    SkSpan<const SkGlyph*> internalPrepare(
            SkSpan<const SkGlyphID> glyphIDs,
            PathDetail pathDetail,
            const SkGlyph** results) SK_REQUIRES(fStrikeLock);

    // The following are const and need no mutex protection.
    const SkFontMetrics               fFontMetrics;
    const SkGlyphPositionRoundingSpec fRoundingSpec;
    const SkStrikeSpec                fStrikeSpec;
    SkStrikeCache* const              fStrikeCache;

    // This mutex provides protection for this specific SkStrike.
    mutable SkMutex fStrikeLock;

    // Maps from a combined GlyphID and sub-pixel position to a SkGlyphDigest. The actual glyph is
    // stored in the fAlloc. The pointer to the glyph is stored fGlyphForIndex. The
    // SkGlyphDigest's fIndex field stores the index. This pointer provides an unchanging
    // reference to the SkGlyph as long as the strike is alive, and fGlyphForIndex
    // provides a dense index for glyphs.
    skia_private::THashTable<SkGlyphDigest, SkPackedGlyphID, SkGlyphDigest>
            fDigestForPackedGlyphID SK_GUARDED_BY(fStrikeLock);

    // Maps from a glyphIndex to a glyph
    std::vector<SkGlyph*> fGlyphForIndex SK_GUARDED_BY(fStrikeLock);

    // Context that corresponds to the glyph information in this strike.
    const std::unique_ptr<SkScalerContext> fScalerContext SK_GUARDED_BY(fStrikeLock);

    // Used while changing the strike to track memory increase.
    size_t fMemoryIncrease SK_GUARDED_BY(fStrikeLock) {0};

    // So, we don't grow our arrays a lot.
    inline static constexpr size_t kMinGlyphCount = 8;
    inline static constexpr size_t kMinGlyphImageSize = 16 /* height */ * 8 /* width */;
    inline static constexpr size_t kMinAllocAmount = kMinGlyphImageSize * kMinGlyphCount;

    SkArenaAlloc            fAlloc SK_GUARDED_BY(fStrikeLock) {kMinAllocAmount};

    // The following are protected by the SkStrikeCache's mutex.
    SkStrike*                       fNext{nullptr};
    SkStrike*                       fPrev{nullptr};
    std::unique_ptr<SkStrikePinner> fPinner;
    size_t                          fMemoryUsed{sizeof(SkStrike)};
    bool                            fRemoved{false};
};

#endif  // SkStrike_DEFINED

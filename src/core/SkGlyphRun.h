/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphRun_DEFINED
#define SkGlyphRun_DEFINED

#include <functional>
#include <memory>
#include <vector>

#include "SkDistanceFieldGen.h"
#include "SkGlyph.h"
#include "SkMask.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkScalerContext.h"
#include "SkSpan.h"
#include "SkSurfaceProps.h"
#include "SkTemplates.h"
#include "SkTextBlobPriv.h"
#include "SkTypes.h"

class SkArenaAlloc;
class SkBaseDevice;
class SkGlyphRunList;
class SkRasterClip;

class SkGlyphCacheInterface {
public:
    virtual ~SkGlyphCacheInterface() = default;
    virtual SkVector rounding() const = 0;
    virtual const SkGlyph& getGlyphMetrics(SkGlyphID glyphID, SkPoint position) = 0;
};

class SkGlyphCacheCommon {
public:
    static SkVector PixelRounding(bool isSubpixel, SkAxisAlignment axisAlignment) {
        if (!isSubpixel) {
            return {SK_ScalarHalf, SK_ScalarHalf};
        } else {
            static constexpr SkScalar kSubpixelRounding = SkFixedToScalar(SkGlyph::kSubpixelRound);
            switch (axisAlignment) {
                case kX_SkAxisAlignment:
                    return {kSubpixelRounding, SK_ScalarHalf};
                case kY_SkAxisAlignment:
                    return {SK_ScalarHalf, kSubpixelRounding};
                case kNone_SkAxisAlignment:
                    return {kSubpixelRounding, kSubpixelRounding};
            }
        }

        // Some compilers need this.
        return {0, 0};
    }

    // This assumes that position has the appropriate rounding term applied.
    static SkIPoint SubpixelLookup(SkAxisAlignment axisAlignment, SkPoint position) {
        // TODO: SkScalarFraction uses truncf to calculate the fraction. This should be floorf.
        SkFixed lookupX = SkScalarToFixed(SkScalarFraction(position.x())),
                lookupY = SkScalarToFixed(SkScalarFraction(position.y()));

        // Snap to a given axis if alignment is requested.
        if (axisAlignment == kX_SkAxisAlignment) {
            lookupY = 0;
        } else if (axisAlignment == kY_SkAxisAlignment) {
            lookupX = 0;
        }

        return {lookupX, lookupY};
    }

    // An atlas consists of plots, and plots hold glyphs. The minimum a plot can be is 256x256.
    // This means that the maximum size a glyph can be is 256x256.
    static constexpr uint16_t kSkSideTooBigForAtlas = 256;

    inline static bool GlyphTooBigForAtlas(const SkGlyph& glyph) {
        return glyph.fWidth > kSkSideTooBigForAtlas || glyph.fHeight > kSkSideTooBigForAtlas;
    }
};

class SkGlyphRun {
public:
    SkGlyphRun() = default;
    SkGlyphRun(const SkPaint& basePaint,
               const SkRunFont& runFont,
               SkSpan<const uint16_t> denseIndices,
               SkSpan<const SkPoint> positions,
               SkSpan<const SkGlyphID> glyphIDs,
               SkSpan<const SkGlyphID> uniqueGlyphIDs,
               SkSpan<const char> text,
               SkSpan<const uint32_t> clusters);

    // A function that turns an SkGlyphRun into an SkGlyphRun for each glyph.
    using PerGlyph = std::function<void (SkGlyphRun*, SkPaint*)>;
    void eachGlyphToGlyphRun(PerGlyph perGlyph);

    // The following made a ~5% speed improvement over not using a template.
    //using PerGlyphPos = std::function<void (SkGlyphID glyphID, SkPoint positions)>;
    template <typename PerGlyphPos>
    void forEachGlyphAndPosition(PerGlyphPos perGlyph) const;

    void filloutGlyphsAndPositions(SkGlyphID* glyphIDs, SkPoint* positions);

    size_t runSize() const { return fGlyphIDs.size(); }
    SkSpan<const SkPoint> positions() const { return fPositions.toConst(); }
    SkSpan<const SkGlyphID> shuntGlyphsIDs() const { return fGlyphIDs; }
    const SkPaint& paint() const { return fRunPaint; }
    SkPaint* mutablePaint() { return &fRunPaint; }
    SkSpan<const uint32_t> clusters() const { return fClusters; }
    SkSpan<const char> text() const { return fText; }

private:
    //
    const SkSpan<const uint16_t> fUniqueGlyphIDIndices;
    //
    const SkSpan<const SkPoint> fPositions;
    // This is temporary while converting from the old per glyph code to the bulk code.
    const SkSpan<const SkGlyphID> fGlyphIDs;
    // The unique glyphs from fGlyphIDs.
    const SkSpan<const SkGlyphID> fUniqueGlyphIDs;
    // Original text from SkTextBlob if present. Will be empty of not present.
    const SkSpan<const char> fText;
    // Original clusters from SkTextBlob if present. Will be empty if not present.
    const SkSpan<const uint32_t>   fClusters;
    // Paint for this run modified to have glyph encoding and left alignment.
    SkPaint fRunPaint;
};

class SkGlyphRunList {
    const SkPaint* fOriginalPaint{nullptr};  // This should be deleted soon.
    // The text blob is needed to hookup the call back that the SkTextBlob destructor calls. It
    // should be used for nothing else
    const SkTextBlob*  fOriginalTextBlob{nullptr};
    SkPoint fOrigin = {0, 0};
    SkSpan<SkGlyphRun> fGlyphRuns;

public:
    SkGlyphRunList();
    // Blob maybe null.
    SkGlyphRunList(
            const SkPaint& paint,
            const SkTextBlob* blob,
            SkPoint origin,
            SkSpan<SkGlyphRun> glyphRunList);

    SkGlyphRunList(SkGlyphRun* glyphRun);

    uint64_t uniqueID() const;
    bool anyRunsLCD() const;
    void temporaryShuntBlobNotifyAddedToCache(uint32_t cacheID) const;

    bool canCache() const { return fOriginalTextBlob != nullptr; }
    size_t runCount() const { return fGlyphRuns.size(); }
    size_t totalGlyphCount() const {
        size_t glyphCount = 0;
        for(const auto& run : fGlyphRuns) {
            glyphCount += run.runSize();
        }
        return glyphCount;
    }

    SkPoint origin() const { return fOrigin; }
    const SkPaint& paint() const { return *fOriginalPaint; }
    const SkTextBlob* blob() const { return fOriginalTextBlob; }

    auto begin() -> decltype(fGlyphRuns.begin())               { return fGlyphRuns.begin();  }
    auto end()   -> decltype(fGlyphRuns.end())                 { return fGlyphRuns.end();    }
    auto begin() const -> decltype(fGlyphRuns.cbegin())        { return fGlyphRuns.cbegin(); }
    auto end()   const -> decltype(fGlyphRuns.cend())          { return fGlyphRuns.cend();   }
    auto size()  const -> decltype(fGlyphRuns.size())          { return fGlyphRuns.size();   }
    auto empty() const -> decltype(fGlyphRuns.empty())         { return fGlyphRuns.empty();  }
    auto operator [] (size_t i) const -> decltype(fGlyphRuns[i]) { return fGlyphRuns[i];     }
};

class SkGlyphIDSet {
public:
    SkSpan<const SkGlyphID> uniquifyGlyphIDs(
            uint32_t universeSize, SkSpan<const SkGlyphID> glyphIDs,
            SkGlyphID* uniqueGlyphIDs, uint16_t* denseindices);
private:
    size_t fUniverseToUniqueSize{0};
    SkAutoTMalloc<uint16_t> fUniverseToUnique;
};

class SkGlyphRunBuilder {
public:
    void drawTextAtOrigin(const SkPaint& paint, const void* bytes, size_t byteLength);
    void drawText(
            const SkPaint& paint, const void* bytes, size_t byteLength, SkPoint origin);
    void drawPosTextH(
            const SkPaint& paint, const void* bytes, size_t byteLength,
            const SkScalar* xpos, SkScalar constY);
    void drawPosText(
            const SkPaint& paint, const void* bytes, size_t byteLength, const SkPoint* pos);
    void drawTextBlob(const SkPaint& paint, const SkTextBlob& blob, SkPoint origin);
    void drawGlyphPos(
            const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs, const SkPoint* pos);

    const SkGlyphRunList& useGlyphRunList();

private:
    void initialize(size_t totalRunSize);
    SkSpan<const SkGlyphID> textToGlyphIDs(
            const SkPaint& paint, const void* bytes, size_t byteLength);

    // Returns the span of unique glyph IDs.
    SkSpan<const SkGlyphID> addDenseAndUnique(
            const SkPaint& paint,
            SkSpan<const SkGlyphID> glyphIDs,
            uint16_t* uniqueGlyphIDIndices,
            SkGlyphID* uniqueGlyphIDs);

    void makeGlyphRun(
            const SkPaint& basePaint,
            const SkRunFont& runFont,
            SkSpan<const SkGlyphID> glyphIDs,
            SkSpan<const SkPoint> positions,
            SkSpan<const uint16_t> uniqueGlyphIDIndices,
            SkSpan<const SkGlyphID> uniqueGlyphIDs,
            SkSpan<const char> text,
            SkSpan<const uint32_t> clusters);

    void makeGlyphRunList(const SkPaint& paint, const SkTextBlob* blob, SkPoint origin);

    size_t simplifyDrawText(
            const SkPaint& paint, const SkRunFont& runFont, SkSpan<const SkGlyphID> glyphIDs,
            SkPoint origin,uint16_t* uniqueGlyphIDIndices, SkGlyphID* uniqueGlyphIDs,
            SkPoint* positions,
            SkSpan<const char> text = SkSpan<const char>{},
            SkSpan<const uint32_t> clusters = SkSpan<const uint32_t>{});
    size_t simplifyDrawPosTextH(
            const SkPaint& paint, const SkRunFont& runFont, SkSpan<const SkGlyphID> glyphIDs,
            const SkScalar* xpos, SkScalar constY,
            uint16_t* uniqueGlyphIDIndices, SkGlyphID* uniqueGlyphIDs, SkPoint* positions,
            SkSpan<const char> text = SkSpan<const char>{},
            SkSpan<const uint32_t> clusters = SkSpan<const uint32_t>{});
    size_t simplifyDrawPosText(
            const SkPaint& paint, const SkRunFont& runFont, SkSpan<const SkGlyphID> glyphIDs,
            const SkPoint* pos, uint16_t* uniqueGlyphIDIndices, SkGlyphID* uniqueGlyphIDs,
            SkSpan<const char> text = SkSpan<const char>{},
            SkSpan<const uint32_t> clusters = SkSpan<const uint32_t>{});

    size_t fMaxTotalRunSize{0};
    SkAutoTMalloc<uint16_t> fUniqueGlyphIDIndices;
    SkAutoTMalloc<SkPoint> fPositions;
    SkAutoTMalloc<SkGlyphID> fUniqueGlyphIDs;

    std::vector<SkGlyphRun> fGlyphRunListStorage;
    SkGlyphRunList fGlyphRunList;


    // Used as a temporary for preparing using utfN text. This implies that only one run of
    // glyph ids will ever be needed because blobs are already glyph based.
    std::vector<SkGlyphID> fScratchGlyphIDs;

    // Used as temporary storage for calculating positions for drawText.
    std::vector<SkPoint> fScratchAdvances;

    // Used for collecting the set of unique glyphs.
    SkGlyphIDSet fGlyphIDSet;
};

template <typename PerGlyphPos>
inline void SkGlyphRun::forEachGlyphAndPosition(PerGlyphPos perGlyph) const {
    const SkPoint* ptCursor = fPositions.data();
    for (auto glyphID : fGlyphIDs) {
        perGlyph(glyphID, *ptCursor++);
    }
}

#endif  // SkGlyphRun_DEFINED

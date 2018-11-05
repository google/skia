/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphRun_DEFINED
#define SkGlyphRun_DEFINED

#include <functional>
#include <vector>

#include "SkPaint.h"
#include "SkPoint.h"
#include "SkSpan.h"
#include "SkTemplates.h"
#include "SkTypes.h"

class SkGlyph;
class SkRunFont;

class SkGlyphRun {
public:
    SkGlyphRun() = default;
    SkGlyphRun(const SkPaint& basePaint,
               const SkRunFont& runFont,
               SkSpan<const SkPoint> positions,
               SkSpan<const SkGlyphID> glyphIDs,
               SkSpan<const char> text,
               SkSpan<const uint32_t> clusters);
    SkGlyphRun(const SkGlyphRun& glyphRun, const SkPaint& paint);

    // A function that turns an SkGlyphRun into an SkGlyphRun for each glyph.
    using PerGlyph = std::function<void (const SkGlyphRun&)>;
    void eachGlyphToGlyphRun(PerGlyph perGlyph);

    void filloutGlyphsAndPositions(SkGlyphID* glyphIDs, SkPoint* positions);

    size_t runSize() const { return fGlyphIDs.size(); }
    SkSpan<const SkPoint> positions() const { return fPositions.toConst(); }
    SkSpan<const SkGlyphID> glyphsIDs() const { return fGlyphIDs; }
    const SkPaint& paint() const { return fRunPaint; }
    SkSpan<const uint32_t> clusters() const { return fClusters; }
    SkSpan<const char> text() const { return fText; }

private:
    // Positions of each glyph.
    const SkSpan<const SkPoint> fPositions;
    // This is temporary while converting from the old per glyph code to the bulk code.
    const SkSpan<const SkGlyphID> fGlyphIDs;
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
    SkSpan<const SkGlyphRun> fGlyphRuns;

public:
    SkGlyphRunList();
    // Blob maybe null.
    SkGlyphRunList(
            const SkPaint& paint,
            const SkTextBlob* blob,
            SkPoint origin,
            SkSpan<const SkGlyphRun> glyphRunList);

    SkGlyphRunList(const SkGlyphRun& glyphRun);

    uint64_t uniqueID() const;
    bool anyRunsLCD() const;
    bool anyRunsSubpixelPositioned() const;
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

    void makeGlyphRun(
            const SkPaint& basePaint,
            const SkRunFont& runFont,
            SkSpan<const SkGlyphID> glyphIDs,
            SkSpan<const SkPoint> positions,
            SkSpan<const char> text,
            SkSpan<const uint32_t> clusters);

    void makeGlyphRunList(const SkPaint& paint, const SkTextBlob* blob, SkPoint origin);

    void simplifyDrawText(
            const SkPaint& paint, const SkRunFont& runFont, SkSpan<const SkGlyphID> glyphIDs,
            SkPoint origin, SkPoint* positions,
            SkSpan<const char> text = SkSpan<const char>{},
            SkSpan<const uint32_t> clusters = SkSpan<const uint32_t>{});
    void simplifyDrawPosTextH(
            const SkPaint& paint, const SkRunFont& runFont, SkSpan<const SkGlyphID> glyphIDs,
            const SkScalar* xpos, SkScalar constY, SkPoint* positions,
            SkSpan<const char> text = SkSpan<const char>{},
            SkSpan<const uint32_t> clusters = SkSpan<const uint32_t>{});
    void simplifyDrawPosText(
            const SkPaint& paint, const SkRunFont& runFont, SkSpan<const SkGlyphID> glyphIDs,
            const SkPoint* pos,
            SkSpan<const char> text = SkSpan<const char>{},
            SkSpan<const uint32_t> clusters = SkSpan<const uint32_t>{});

    size_t fMaxTotalRunSize{0};
    SkAutoTMalloc<SkPoint> fPositions;

    std::vector<SkGlyphRun> fGlyphRunListStorage;
    SkGlyphRunList fGlyphRunList;

    // Used as a temporary for preparing using utfN text. This implies that only one run of
    // glyph ids will ever be needed because blobs are already glyph based.
    std::vector<SkGlyphID> fScratchGlyphIDs;

    // Used as temporary storage for calculating positions for drawText.
    std::vector<SkPoint> fScratchAdvances;

    // Used for collecting the set of unique glyphs.
    SkGlyphIDSet fGlyphIDSet;
    SkAutoTMalloc<SkGlyphID> fUniqueGlyphIDs;
    SkAutoTMalloc<uint16_t> fUniqueGlyphIDIndices;
};

#endif  // SkGlyphRun_DEFINED

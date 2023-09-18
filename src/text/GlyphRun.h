/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphRun_DEFINED
#define SkGlyphRun_DEFINED

#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkZip.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <tuple>
#include <vector>

class SkPaint;
struct SkRSXform;

namespace sktext {
class GlyphRunBuilder;

class GlyphRun {
public:
    GlyphRun(const SkFont& font,
             SkSpan<const SkPoint> positions,
             SkSpan<const SkGlyphID> glyphIDs,
             SkSpan<const char> text,
             SkSpan<const uint32_t> clusters,
             SkSpan<const SkVector> scaledRotations);

    GlyphRun(const GlyphRun& glyphRun, const SkFont& font);

    size_t runSize() const { return fSource.size(); }
    SkSpan<const SkPoint> positions() const { return fSource.get<1>(); }
    SkSpan<const SkGlyphID> glyphsIDs() const { return fSource.get<0>(); }
    SkZip<const SkGlyphID, const SkPoint> source() const { return fSource; }
    const SkFont& font() const { return fFont; }
    SkSpan<const uint32_t> clusters() const { return fClusters; }
    SkSpan<const char> text() const { return fText; }
    SkSpan<const SkVector> scaledRotations() const { return fScaledRotations; }

private:
    // GlyphIDs and positions.
    const SkZip<const SkGlyphID, const SkPoint> fSource;
    // Original text from SkTextBlob if present. Will be empty of not present.
    const SkSpan<const char> fText;
    // Original clusters from SkTextBlob if present. Will be empty if not present.
    const SkSpan<const uint32_t>   fClusters;
    // Possible RSXForm information
    const SkSpan<const SkVector> fScaledRotations;
    // Font for this run modified to have glyph encoding and left alignment.
    SkFont fFont;
};

class GlyphRunList {
    SkSpan<const GlyphRun> fGlyphRuns;

public:
    // Blob maybe null.
    GlyphRunList(const SkTextBlob* blob,
                 SkRect bounds,
                 SkPoint origin,
                 SkSpan<const GlyphRun> glyphRunList,
                 GlyphRunBuilder* builder);

    GlyphRunList(const GlyphRun& glyphRun,
                 const SkRect& bounds,
                 SkPoint origin,
                 GlyphRunBuilder* builder);
    uint64_t uniqueID() const;
    bool anyRunsLCD() const;
    void temporaryShuntBlobNotifyAddedToCache(uint32_t cacheID, SkTextBlob::PurgeDelegate) const;

    bool canCache() const { return fOriginalTextBlob != nullptr; }
    size_t runCount() const { return fGlyphRuns.size(); }
    size_t totalGlyphCount() const {
        size_t glyphCount = 0;
        for (const GlyphRun& run : *this) {
            glyphCount += run.runSize();
        }
        return glyphCount;
    }
    size_t maxGlyphRunSize() const {
        size_t size = 0;
        for (const GlyphRun& run : *this) {
            size = std::max(run.runSize(), size);
        }
        return size;
    }

    bool hasRSXForm() const {
        for (const GlyphRun& run : *this) {
            if (!run.scaledRotations().empty()) { return true; }
        }
        return false;
    }

    sk_sp<SkTextBlob> makeBlob() const;

    SkPoint origin() const { return fOrigin; }
    SkRect sourceBounds() const { return fSourceBounds; }
    SkRect sourceBoundsWithOrigin() const { return fSourceBounds.makeOffset(fOrigin); }
    const SkTextBlob* blob() const { return fOriginalTextBlob; }
    GlyphRunBuilder* builder() const { return fBuilder; }

    auto begin() -> decltype(fGlyphRuns.begin())               { return fGlyphRuns.begin();      }
    auto end()   -> decltype(fGlyphRuns.end())                 { return fGlyphRuns.end();        }
    auto begin() const -> decltype(std::cbegin(fGlyphRuns))    { return std::cbegin(fGlyphRuns); }
    auto end()   const -> decltype(std::cend(fGlyphRuns))      { return std::cend(fGlyphRuns);   }
    auto size()  const -> decltype(fGlyphRuns.size())          { return fGlyphRuns.size();       }
    auto empty() const -> decltype(fGlyphRuns.empty())         { return fGlyphRuns.empty();      }
    auto operator [] (size_t i) const -> decltype(fGlyphRuns[i]) { return fGlyphRuns[i];         }

private:
    // The text blob is needed to hook up the call back that the SkTextBlob destructor calls. It
    // should be used for nothing else.
    const SkTextBlob* fOriginalTextBlob{nullptr};
    const SkRect fSourceBounds{SkRect::MakeEmpty()};
    const SkPoint fOrigin = {0, 0};
    GlyphRunBuilder* const fBuilder;
};

class GlyphRunBuilder {
public:
    GlyphRunList makeGlyphRunList(const GlyphRun& run, const SkPaint& paint, SkPoint origin);
    const GlyphRunList& textToGlyphRunList(const SkFont& font,
                                           const SkPaint& paint,
                                           const void* bytes,
                                           size_t byteLength,
                                           SkPoint origin,
                                           SkTextEncoding encoding = SkTextEncoding::kUTF8);
    const GlyphRunList& blobToGlyphRunList(const SkTextBlob& blob, SkPoint origin);
    std::tuple<SkSpan<const SkPoint>, SkSpan<const SkVector>>
            convertRSXForm(SkSpan<const SkRSXform> xforms);

    bool empty() const { return fGlyphRunListStorage.empty(); }

private:
    void initialize(const SkTextBlob& blob);
    void prepareBuffers(int positionCount, int RSXFormCount);

    SkSpan<const SkGlyphID> textToGlyphIDs(
            const SkFont& font, const void* bytes, size_t byteLength, SkTextEncoding);

    void makeGlyphRun(
            const SkFont& font,
            SkSpan<const SkGlyphID> glyphIDs,
            SkSpan<const SkPoint> positions,
            SkSpan<const char> text,
            SkSpan<const uint32_t> clusters,
            SkSpan<const SkVector> scaledRotations);

    const GlyphRunList& setGlyphRunList(
            const SkTextBlob* blob, const SkRect& bounds, SkPoint origin);

    int fMaxTotalRunSize{0};
    skia_private::AutoTMalloc<SkPoint> fPositions;
    int fMaxScaledRotations{0};
    skia_private::AutoTMalloc<SkVector> fScaledRotations;

    std::vector<GlyphRun> fGlyphRunListStorage;
    std::optional<GlyphRunList> fGlyphRunList;  // Defaults to no value;

    // Used as a temporary for preparing using utfN text. This implies that only one run of
    // glyph ids will ever be needed because blobs are already glyph based.
    std::vector<SkGlyphID> fScratchGlyphIDs;

};
}  // namespace sktext

#endif  // SkGlyphRun_DEFINED

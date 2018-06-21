/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphRunInfo_DEFINED
#define SkGlyphRunInfo_DEFINED

#include <functional>
#include <memory>
#include <vector>

#include "SkDescriptor.h"
#include "SkMask.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkTextBlob.h"
#include "SkTypes.h"

class SkBaseDevice;
class SkDrawFilter;

template <typename T>
class SkSpan {
public:
    SkSpan() : fPtr{nullptr}, fSize{0} {}
    SkSpan(T* ptr, size_t size) : fPtr{ptr}, fSize{size} {}
    explicit SkSpan(std::vector<T>& v) : fPtr{v.data()}, fSize{v.size()} {}
    SkSpan& operator=( const SkSpan& other ) = default;
    T& operator [] (ptrdiff_t i) const { return fPtr[i]; }
    T* begin() const { return fPtr; }
    T* end() const { return fPtr + fSize; }
    const T* cbegin() const { return fPtr; }
    const T* cend() const { return fPtr + fSize; }
    T* data() const { return fPtr; }
    ptrdiff_t size() const { return fSize; }
    bool empty() const { return fSize == 0; }

private:
    T* fPtr;
    size_t fSize;
};

struct SkIndexedRunInfo {
    SkIndexedRunInfo(const std::vector<uint16_t>& denseIndex,
                     const std::vector<SkPoint>& positions,
                     const std::vector<SkGlyphID>& uniqueGlyphIDs)
    : fDenseIndex{denseIndex}
    , fPositions{positions}
    , fUniqueGlyphIDs{uniqueGlyphIDs} {}

    SkSpan<const uint16_t> denseIndex(size_t start, size_t size) {
        return SkSpan<const uint16_t>(&fDenseIndex[start], size);
    }

    SkSpan<const SkPoint> positions(size_t start, size_t size) const {
        return SkSpan<const SkPoint>(&fPositions[start], size);
    }

    SkSpan<const SkGlyphID> uniqueGlyphIDs(size_t start, size_t size) const {
        return SkSpan<const SkGlyphID>(&fUniqueGlyphIDs[start], size);
    }

private:
    const std::vector<uint16_t>&  fDenseIndex;
    const std::vector<SkPoint>&   fPositions;
    const std::vector<SkGlyphID>& fUniqueGlyphIDs;
};

class SkGlyphRun {
public:
    SkGlyphRun(const SkIndexedRunInfo& runInfo,
               size_t denseOffset, size_t denseSize,
               size_t fUniqueOffset, uint16_t fUniqueSize,
               SkSpan<SkGlyphID>  scratchGlyphs,
               SkSpan<const char> text,
               SkSpan<const uint32_t>   clusters,
               SkPaint&&          runPaint);

    // The temporaryShunt calls are to allow inter-operating with existing code while glyph runs
    // are developed.
    void temporaryShuntToDrawPosText(SkBaseDevice* device, SkPoint origin);
    using TemporaryShuntCallback = std::function<void(size_t, const char*, const SkScalar*)>;
    void temporaryShuntToCallback(TemporaryShuntCallback callback);

    size_t runSize() const { return fDenseSize; }
    uint16_t uniqueSize() const { return fUniqueSize; }
    SkSpan<const SkPoint> positions() const {
        return fRunInfo.positions(fDenseOffset, fDenseSize);
    }
    SkSpan<const SkGlyphID> uniqueGlyphIDs() const {
        return fRunInfo.uniqueGlyphIDs(fUniqueOffset, fUniqueSize);
    }
    SkSpan<SkGlyphID> shuntGlyphsIDs() const {
        return fTemporaryShuntGlyphIDs;
    }

    const SkPaint& paint() const {
        return fRunPaint;
    }

    SkSpan<const uint32_t> clusters() const {
        return fClusters;
    }

    SkSpan<const char> text() const {
        return fText;
    }

private:
    const SkIndexedRunInfo& fRunInfo;
    const size_t fDenseOffset;
    const size_t fDenseSize;
    const size_t fUniqueOffset;
    const uint16_t fUniqueSize;

    // This is temporary while converting from the old per glyph code to the bulk code.
    const SkSpan<SkGlyphID>  fTemporaryShuntGlyphIDs;
    // Original text from SkTextBlob if present. Will be empty of not present.
    const SkSpan<const char> fText;
    // Original clusters from SkTextBlob if present. Will be empty if not present.
    const SkSpan<const uint32_t>   fClusters;
    // Paint for this run modified to have glyph encoding and left alignment.
    const SkPaint fRunPaint;
};

class SkGlyphRunList {
    SkSpan<SkGlyphRun> fGlyphRuns;
    SkPoint            fOrigin = {0, 0};

    // The text blob is needed to hookup the call back that the SkTextBlob destructor calls. It
    // should be used for nothing else
    const SkTextBlob*  fTemporaryTextBlobShunt{nullptr};

public:
    SkGlyphRunList() = default;
    // Blob maybe null.
    SkGlyphRunList(SkSpan<SkGlyphRun> glyphRuns, SkPoint origin, const SkTextBlob* blob);

    uint64_t uniqueID() const;
    bool anyRunsLCD() const;
    void temporaryShuntBlobnotifyAddedToCache(uint32_t cacheID) const;

    bool canCache() const { return fTemporaryTextBlobShunt != nullptr; }
    ptrdiff_t runCount() const { return fGlyphRuns.size(); }
    size_t totalGlyphCount() const {
        size_t glyphCount = 0;
        for(const auto& run : fGlyphRuns) {
            glyphCount += run.runSize();
        }
        return glyphCount;
    }

    SkPoint origin() const { return fOrigin; }

    auto begin() -> decltype(fGlyphRuns.begin())               { return fGlyphRuns.begin(); }
    auto end()   -> decltype(fGlyphRuns.end())                 { return fGlyphRuns.end();   }
    auto size()  -> decltype(fGlyphRuns.size())                { return fGlyphRuns.size();  }
    auto operator [] (ptrdiff_t i) -> decltype(fGlyphRuns[i])  { return fGlyphRuns[i];      }
};

class SkGlyphRunListIterator {
public:
    explicit SkGlyphRunListIterator(SkGlyphRunList* list) : fList{*list} {}

    bool done() const { return fIndex == fList.size(); }
    void next() { fIndex += 1;}
    uint32_t glyphCount() const { return fList[fIndex].runSize(); }
    const uint16_t* glyphs() const { return fList[fIndex].shuntGlyphsIDs().data(); }
    const SkScalar* pos() const { return (const SkScalar*)fList[fIndex].positions().data(); }
    const SkPoint& offset() const { return fZero; }
    void applyFontToPaint(SkPaint* paint) const { *paint = fList[fIndex].paint(); }
    SkTextBlob::GlyphPositioning positioning() const { return SkTextBlob::kFull_Positioning; }
    const uint32_t* clusters() const { return fList[fIndex].clusters().data(); }
    uint32_t textSize() const { return fList[fIndex].text().size(); }
    const char* text() const { return fList[fIndex].text().data(); }

    bool isLCD() const { return fList[fIndex].paint().isLCDRenderText(); }

private:
    static constexpr SkPoint fZero{0,0};
    ptrdiff_t fIndex{0};
    SkGlyphRunList& fList;
};

// A faster set implementation that does not need any initialization, and reading the set items
// is order the number of items, and not the size of the universe.
// This implementation is based on the paper by Briggs and Torczon, "An Efficient Representation
// for Sparse Sets"
//
// This implementation assumes that the unique glyphs added are appended to a vector that may
// already have unique glyph from a previous computation. This allows the packing of multiple
// UniqueID sequences in a single vector.
class SkGlyphSet {
public:
    SkGlyphSet() = default;
    uint16_t add(SkGlyphID glyphID);
    void reuse(uint32_t glyphUniverseSize, std::vector<SkGlyphID>* uniqueGlyphIDs);

private:
    uint32_t uniqueSize();
    uint32_t                    fUniverseSize{0};
    size_t                      fStartOfUniqueIDs{0};
    std::vector<uint16_t>       fIndices;
    std::vector<SkGlyphID>*     fUniqueGlyphIDs{nullptr};
};

class SkGlyphRunBuilder {
public:
    SkGlyphRunBuilder() = default;
    void prepareDrawText(
            const SkPaint& paint, const void* bytes, size_t byteLength, SkPoint origin);
    void prepareDrawPosTextH(
            const SkPaint& paint, const void* bytes, size_t byteLength,
            const SkScalar xpos[], SkScalar constY);
    void prepareDrawPosText(
            const SkPaint& paint, const void* bytes, size_t byteLength, const SkPoint pos[]);
    void prepareTextBlob(
            const SkPaint& paint, const SkTextBlob& blob, SkPoint origin, SkDrawFilter* filter);

    void draw(SkBaseDevice* device);

    // Public for testing only.
    SkGlyphRun* useGlyphRun();
    SkGlyphRunList* useGlyphRunList();

private:
    size_t runSize() const;
    size_t uniqueSize() const;
    void initialize();
    SkGlyphID* addDenseAndUnique(const SkPaint& paint, const void* bytes, size_t byteLength);
    void addGlyphRunToList(
            const SkPaint& runPaint,
            SkGlyphID* temporaryShuntGlyphIDs,
            SkSpan<const char> text,
            SkSpan<const uint32_t> clusters);

    void drawText(
            const SkPaint& paint, const void* bytes, size_t byteLength, SkPoint origin,
            SkSpan<const char> text, SkSpan<const uint32_t> clusters);
    void drawPosTextH(
            const SkPaint& paint, const void* bytes, size_t byteLength,
            const SkScalar* xpos, SkScalar constY,
            SkSpan<const char> text, SkSpan<const uint32_t> clusters);
    void drawPosText(
            const SkPaint& paint, const void* bytes, size_t byteLength, const SkPoint* pos,
            SkSpan<const char> text, SkSpan<const uint32_t> clusters);

    const SkTextBlob*      fTemporaryTextBlobShunt{nullptr};

    // The point passed into drawTextBlob. This allows the GPU back end to detect and adjust for
    // translations.
    SkPoint                fOrigin;

    std::vector<uint16_t>  fDenseIndex;
    std::vector<SkPoint>   fPositions;
    std::vector<SkGlyphID> fUniqueGlyphIDs;

    SkIndexedRunInfo       fIndexed{fDenseIndex, fPositions, fUniqueGlyphIDs};

    size_t                 fLastDenseIndex{0};
    size_t                 fLastUniqueIndex{0};

    // Used as a temporary for preparing using utfN text. This implies that only one run of
    // glyph ids will ever be needed because blobs are already glyph based.
    std::vector<SkGlyphID> fScratchGlyphIDs;

    // Used as temporary storage for calculating positions for drawText.
    std::vector<SkPoint>   fScratchAdvances;

    // Vector for accumulating runs. This is later deposited in fScratchGlyphRunList;
    std::vector<SkGlyphRun> fGlyphRuns;

    // Used as temporary glyph run for the rest of the Text stack.
    SkGlyphRunList         fScratchGlyphRunList;

    // Used for collecting the set of unique glyphs.
    SkGlyphSet             fGlyphSet;
};

#endif  // SkGlyphRunInfo_DEFINED

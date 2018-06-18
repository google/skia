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
#include "SkTypes.h"

class SkBaseDevice;

template <typename T>
class SkSpan {
public:
    SkSpan() = default;
    SkSpan(T* ptr, size_t size) : fPtr{ptr}, fSize{size} {}
    explicit SkSpan(std::vector<T>& v) : fPtr{v.data()}, fSize{v.size()} {}
    T& operator [] (ptrdiff_t i) { return fPtr[i]; }
    const T& operator [] (ptrdiff_t i) const { return fPtr[i]; }
    const T* begin() const { return fPtr; }
    const T* end() const { return fPtr + fSize; }
    const T* data() const { return fPtr; }
    ptrdiff_t size() const { return fSize; }

private:
    T* fPtr;
    size_t fSize;
};

class SkGlyphRun {
public:
    SkGlyphRun() = default;
    SkGlyphRun(SkSpan<uint16_t>  denseIndex,
               SkSpan<SkPoint>   positions,
               SkSpan<SkGlyphID> scratchGlyphs,
               SkSpan<SkGlyphID> uniqueGlyphIDs);

    // The temporaryShunt calls are to allow inter-operating with existing code while glyph runs
    // are developed.
    void temporaryShuntToDrawPosText(const SkPaint& paint, SkBaseDevice* device);
    using TemporaryShuntCallback = std::function<void(size_t, const char*, const SkScalar*)>;
    void temporaryShuntToCallback(TemporaryShuntCallback callback);

    size_t runSize() const { return fDenseIndex.size(); }
    uint16_t uniqueSize() const { return fUniqueGlyphIDs.size(); }
    SkSpan<SkPoint> positions() const { return SkSpan<SkPoint>(fPositions); }

private:
    // Indices into the unique glyph IDs. On for each original glyph.
    SkSpan<uint16_t>  fDenseIndex;
    // The base line position of all the glyphs in source space.
    SkSpan<SkPoint>   fPositions;
    // This is temporary while converting from the old per glyph code to the bulk code.
    SkSpan<SkGlyphID> fTemporaryShuntGlyphIDs;
    // The set of unique glyphs in the run.
    SkSpan<SkGlyphID> fUniqueGlyphIDs;
};

class SkGlyphRunList {
    const uint64_t     fUniqueID{0};
    SkSpan<SkGlyphRun> fGlyphRuns;

public:
    SkGlyphRunList() = default;
    SkGlyphRunList(SkSpan<SkGlyphRun> glyphRuns, uint64_t uniqueID);

    uint64_t uniqueID() const { return fUniqueID; }

    auto begin() -> decltype(fGlyphRuns.begin())      { return fGlyphRuns.begin(); }
    auto end() -> decltype(fGlyphRuns.end())          { return fGlyphRuns.end();   }
    auto size() -> decltype(fGlyphRuns.size())        { return fGlyphRuns.size();  }
    auto operator [] (ptrdiff_t i) -> decltype(fGlyphRuns[i]) { return fGlyphRuns[i]; }
};

// A faster set implementation that does not need any initialization, and reading the set items
// is order the number of items, and not the size of the universe.
// This implementation is based on the paper by Briggs and Torczon, "An Efficient Representation
// for Sparse Sets"
class SkGlyphSet {
public:
    SkGlyphSet() = default;
    uint16_t add(SkGlyphID glyphID);
    void reuse(uint32_t glyphUniverseSize, std::vector<SkGlyphID>* uniqueGlyphIDs);

private:
    uint32_t                    fUniverseSize{0};
    std::vector<uint16_t>       fIndices;
    std::vector<SkGlyphID>*     fUniqueGlyphIDs{nullptr};
};

// Currently the old code is passing around SkGlyphRunBuilder because it facilitates working in the
// old single glyph lookup style with the cache. When the lower level code is transitioned over to
// the bulk glyph cache style, then the builder will only be used in the canvas, and only runs will
// be passed around.
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
    void prepareTextBlob(const SkPaint& paint, const SkTextBlob& blob);

    SkGlyphRunList* useGlyphRunList();
    SkGlyphRun* useGlyphRun();

private:
    size_t runSize() const;
    size_t uniqueSize() const;
    void initialize();
    void addDenseAndUnique(const SkPaint& paint, const void* bytes, size_t byteLength);
    void addGlyphRunToList();

    void drawText(
            const SkPaint& paint, const void* bytes, size_t byteLength, SkPoint origin);
    void drawPosTextH(
            const SkPaint& paint, const void* bytes, size_t byteLength,
            const SkScalar* xpos, SkScalar constY);
    void drawPosText(
            const SkPaint& paint, const void* bytes, size_t byteLength, const SkPoint* pos);

    uint64_t               fUniqueID{0};

    std::vector<uint16_t>  fDenseIndex;
    std::vector<SkPoint>   fPositions;
    std::vector<SkGlyphID> fUniqueGlyphs;

    size_t                 fLastDenseIndex{0};
    size_t                 fLastUniqueIndex{0};

    // Used as a temporary for preparing using utfN text.
    std::vector<SkGlyphID> fScratchGlyphIDs;

    // Used as temporary storage for calculating positions for drawText.
    std::vector<SkPoint>   fScratchAdvances;

    std::vector<SkGlyphRun> fGlyphRuns;

    SkGlyphRunList         fScratchGlyphRunList;

    // Used as an aid to shunt from glyph runs to drawPosText. It will either be fScratchIDs or
    // the bytes passed in.
    SkGlyphID*       fTemporaryShuntGlyphIDs{nullptr};

    // Used for collecting the set of unique glyphs.
    SkGlyphSet             fGlyphSet;
};

#endif  // SkGlyphRunInfo_DEFINED

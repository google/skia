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
    SkSpan(const T* ptr, size_t size) : fPtr{ptr}, fSize{size} {}
    explicit SkSpan(const std::vector<T>& v) : fPtr{v.data()}, fSize{v.size()} {}
    const T& operator [] (ptrdiff_t i) const { return fPtr[i]; }
    const T* begin() const { return fPtr; }
    const T* end() const { return fPtr + fSize; }
    ptrdiff_t size() const { return fSize; }

private:
    const T* fPtr;
    size_t fSize;
};

class SkGlyphRun {
public:
    SkGlyphRun() = default;
    SkGlyphRun(SkSpan<uint16_t> denseIndex, SkSpan<SkPoint> positions,
               SkSpan<SkGlyphID> uniqueGlyphIDs)
            : fDenseIndex{denseIndex}
            , fPositions{positions}
            , fUniqueGlyphIDs{uniqueGlyphIDs} {}

    size_t runSize() const { return fDenseIndex.size(); }
    uint16_t uniqueSize() const { return fUniqueGlyphIDs.size(); }
    SkSpan<SkPoint> positions() const { return SkSpan<SkPoint>(fPositions); }

private:
    SkSpan<uint16_t>  fDenseIndex;
    SkSpan<SkPoint>   fPositions;
    SkSpan<SkGlyphID> fUniqueGlyphIDs;
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

    size_t runSize() const {return fDenseIndex.size();}
    size_t uniqueSize() const {return fUniqueGlyphs.size();}

    const SkGlyphRun& useGlyphRun() const;

    // The temporaryShunt calls are to allow inter-operating with existing code while glyph runs
    // are developed.
    void temporaryShuntToDrawPosText(const SkPaint& paint, SkBaseDevice* device);
    using TemporaryShuntCallback = std::function<void(size_t, const char*, const SkScalar*)>;
    void temporaryShuntToCallback(TemporaryShuntCallback callback);

private:
    void initializeDenseAndUnique(const SkPaint& paint, const void* bytes, size_t byteLength);

    std::vector<uint16_t>  fDenseIndex;
    std::vector<SkPoint>   fPositions;
    std::vector<SkGlyphID> fUniqueGlyphs;

    // Used as a temporary for preparing using utfN text.
    std::vector<SkGlyphID> fScratchGlyphIDs;

    // Used as temporary storage for calculating positions for drawText.
    std::vector<SkPoint>   fScratchAdvances;

    // Used to temporarily use of a glyph run for bulk cache API calls (just an experiment at
    // this point).
    SkGlyphRun             fScratchGlyphRun;

    // Used as an aid to shunt from glyph runs to drawPosText. It will either be fScratchIDs or
    // the bytes passed in.
    const SkGlyphID*       fTemporaryShuntGlyphIDs{nullptr};

    // Used for collecting the set of unique glyphs.
    SkGlyphSet            fGlyphSet;
};

#endif  // SkGlyphRunInfo_DEFINED

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
    SkSpan() : fPtr{nullptr}, fSize{0} {}
    SkSpan(T* ptr, ptrdiff_t size) : fPtr{ptr}, fSize{size} { SkASSERT(size >= 0); }
    template <typename U>
    explicit SkSpan(std::vector<U>& v) : fPtr{v.data()}, fSize{SkTo<ptrdiff_t>(v.size())} {}
    SkSpan(const SkSpan<T>& o) = default;
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
    ptrdiff_t fSize;
};

class SkGlyphRun {
public:
    SkGlyphRun() = default;
    SkGlyphRun(SkPaint&& runPaint,
               SkSpan<const uint16_t> denseIndices,
               SkSpan<const SkPoint> positions,
               SkSpan<const SkGlyphID> glyphIDs,
               SkSpan<const SkGlyphID> uniqueGlyphIDs,
               SkSpan<const char> text,
               SkSpan<const uint32_t> clusters);

    // The temporaryShunt calls are to allow inter-operating with existing code while glyph runs
    // are developed.
    void temporaryShuntToDrawPosText(SkBaseDevice* device);
    using TemporaryShuntCallback = std::function<void(size_t, const char*, const SkScalar*)>;
    void temporaryShuntToCallback(TemporaryShuntCallback callback);

    size_t runSize() const { return fUniqueGlyphIDIndices.size(); }
    uint16_t uniqueSize() const { return fUniqueGlyphIDs.size(); }
    SkSpan<const SkPoint> positions() const { return fPositions; }
    SkSpan<const SkGlyphID> uniqueGlyphIDs() const { return fUniqueGlyphIDs; }
    SkSpan<const SkGlyphID> shuntGlyphsIDs() const { return fTemporaryShuntGlyphIDs; }

private:
    //
    const SkSpan<const uint16_t> fUniqueGlyphIDIndices;
    //
    const SkSpan<const SkPoint> fPositions;
    // This is temporary while converting from the old per glyph code to the bulk code.
    const SkSpan<const SkGlyphID> fTemporaryShuntGlyphIDs;
    // The unique glyphs from fTemporaryShuntGlyphIDs.
    const SkSpan<const SkGlyphID> fUniqueGlyphIDs;
    // Original text from SkTextBlob if present. Will be empty of not present.
    const SkSpan<const char> fText;
    // Original clusters from SkTextBlob if present. Will be empty if not present.
    const SkSpan<const uint32_t>   fClusters;
    // Paint for this run modified to have glyph encoding and left alignment.
    const SkPaint fRunPaint;
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
    void prepareTextBlob(const SkPaint& paint, const SkTextBlob& blob, SkPoint origin);

    SkGlyphRun* useGlyphRun();

private:
    void initialize();
    void addDenseAndUnique(const SkPaint& paint, const void* bytes, size_t byteLength);
    void makeGlyphRun(
            const SkPaint& runPaint, SkSpan<const char> text, SkSpan<const uint32_t> clusters);

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

    uint64_t fUniqueID{0};

    std::vector<uint16_t> fDenseIndex;
    std::vector<SkPoint> fPositions;
    std::vector<SkGlyphID> fUniqueGlyphIDs;

    // Used as a temporary for preparing using utfN text. This implies that only one run of
    // glyph ids will ever be needed because blobs are already glyph based.
    std::vector<SkGlyphID> fScratchGlyphIDs;

    // Used as temporary storage for calculating positions for drawText.
    std::vector<SkPoint> fScratchAdvances;


    // Used as temporary glyph run for the rest of the Text stack.
    SkGlyphRun fScratchGlyphRun;

    // Used for collecting the set of unique glyphs.
    SkGlyphSet fGlyphSet;
};

#endif  // SkGlyphRunInfo_DEFINED

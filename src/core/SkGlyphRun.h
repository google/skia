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
    SkSpan(T* ptr, size_t size) : fPtr{ptr}, fSize{size} { }
    template <typename U>
    explicit SkSpan(std::vector<U>& v) : fPtr{v.data()}, fSize{v.size()} {}
    SkSpan(const SkSpan<T>& o) = default;
    SkSpan& operator=( const SkSpan& other ) = default;
    T& operator [] (size_t i) const { return fPtr[i]; }
    T* begin() const { return fPtr; }
    T* end() const { return fPtr + fSize; }
    const T* cbegin() const { return fPtr; }
    const T* cend() const { return fPtr + fSize; }
    T* data() const { return fPtr; }
    size_t size() const { return fSize; }
    bool empty() const { return fSize == 0; }

private:
    T* fPtr;
    size_t fSize;
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
    void drawText(
            const SkPaint& paint, const void* bytes, size_t byteLength,
            SkPoint origin);
    void drawPosTextH(
            const SkPaint& paint, const void* bytes, size_t byteLength,
            const SkScalar* xpos, SkScalar constY);
    void drawPosText(
            const SkPaint& paint, const void* bytes, size_t byteLength,
            const SkPoint* pos);

    SkGlyphRun* useGlyphRun();

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
            const SkPaint& runPaint,
            SkSpan<const SkGlyphID> glyphIDs,
            SkSpan<const SkPoint> positions,
            SkSpan<const uint16_t> uniqueGlyphIDIndices,
            SkSpan<const SkGlyphID> uniqueGlyphIDs,
            SkSpan<const char> text,
            SkSpan<const uint32_t> clusters);

    void simplifyDrawText(
            const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs, SkPoint origin,
            uint16_t* uniqueGlyphIDIndices, SkGlyphID* uniqueGlyphIDs, SkPoint* positions,
            SkSpan<const char> text = SkSpan<const char>{},
            SkSpan<const uint32_t> clusters = SkSpan<const uint32_t>{});
    void simplifyDrawPosTextH(
            const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs,
            const SkScalar* xpos, SkScalar constY,
            uint16_t* uniqueGlyphIDIndices, SkGlyphID* uniqueGlyphIDs, SkPoint* positions,
            SkSpan<const char> text = SkSpan<const char>{},
            SkSpan<const uint32_t> clusters = SkSpan<const uint32_t>{});
    void simplifyDrawPosText(
            const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs, const SkPoint* pos,
            uint16_t* uniqueGlyphIDIndices, SkGlyphID* uniqueGlyphIDs,
            SkSpan<const char> text = SkSpan<const char>{},
            SkSpan<const uint32_t> clusters = SkSpan<const uint32_t>{});

    uint64_t fUniqueID{0};

    size_t fMaxTotalRunSize{0};
    SkAutoTMalloc<uint16_t> fUniqueGlyphIDIndices;
    SkAutoTMalloc<SkPoint> fPositions;
    SkAutoTMalloc<SkGlyphID> fUniqueGlyphIDs;

    // Used as a temporary for preparing using utfN text. This implies that only one run of
    // glyph ids will ever be needed because blobs are already glyph based.
    std::vector<SkGlyphID> fScratchGlyphIDs;

    // Used as temporary storage for calculating positions for drawText.
    std::vector<SkPoint> fScratchAdvances;


    // Used as temporary glyph run for the rest of the Text stack.
    SkGlyphRun fScratchGlyphRun;

    // Used for collecting the set of unique glyphs.
    SkGlyphIDSet fGlyphIDSet;
};

#endif  // SkGlyphRunInfo_DEFINED

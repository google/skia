/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphRunInfo_DEFINED
#define SkGlyphRunInfo_DEFINED

#include <memory>
#include <vector>

#include "SkDescriptor.h"
#include "SkMask.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkTypes.h"

// A faster set implementation that does not need any initialization, and reading the set items
// is order the number of items, and not the size of the universe.
// This implementation is based on the paper by Briggs and Torczon, "An Efficient Representation
// for Sparse Sets"
class SkGlyphSet {
public:
    SkGlyphSet() = default;
    uint16_t add(SkGlyphID glyphID);
    std::vector<SkGlyphID> uniqueGlyphIDs();
    void reuse(uint32_t glyphUniverseSize);

private:
    uint32_t                    fUniverseSize{0};
    std::vector<uint16_t>       fIndices;
    std::vector<SkGlyphID>      fUniqueGlyphIDs;
};

class SkGlyphRun {
public:
    SkGlyphRun() = default;
    SkGlyphRun(SkGlyphRun&&) = default;
    static SkGlyphRun MakeFromDrawText(
            const SkPaint& paint, const void* bytes, size_t byteLength,
            SkPoint origin, SkGlyphSet* glyphSet);
    static SkGlyphRun MakeFromDrawPosTextH(
            const SkPaint& paint, const void* bytes, size_t byteLength,
            const SkScalar xpos[], SkScalar constY, SkGlyphSet* glyphSet);
    static SkGlyphRun MakeFromDrawPosText(
            const SkPaint& paint, const void* bytes, size_t byteLength,
            const SkPoint pos[], SkGlyphSet* glyphSet);

    size_t runSize() const { return fRunSize; }
    uint16_t uniqueSize() const { return fUniqueGlyphs.size(); }

    // copyGlyphIDs is temporary glue to work with the existing system. Don't use with new code.
    std::unique_ptr<SkGlyphID[]> copyGlyphIDs() const;
    const SkScalar* getPositions() const {
        return reinterpret_cast<const SkScalar*>(fPositions.get());
    }

private:
    SkGlyphRun(size_t runSize,
               std::unique_ptr<uint16_t[]>&& denseIndex,
               std::unique_ptr<SkPoint[]>&& positions,
               std::vector<SkGlyphID>&& uniqueGlyphIDs);

    std::unique_ptr<uint16_t[]>  fDenseIndex;
    std::unique_ptr<SkPoint[]>   fPositions;
    std::vector<SkGlyphID>       fUniqueGlyphs;
    const size_t                 fRunSize{0};
};

template <typename T>
class SkSpan {
public:
    SkSpan(const T* ptr, size_t size) : fPtr{ptr}, fSize{size} {}
    SkSpan(const std::vector<T>& v) : fPtr{v.data()}, fSize{v.size()} {}
    const T& operator [] (ptrdiff_t i) const { return fPtr[i]; }
    const T* begin() const { return fPtr; }
    const T* end() const { return fPtr + fSize; }
    ptrdiff_t size() const { return fSize; }

private:
    const T* fPtr;
    size_t fSize;
};

#endif  // SkGlyphRunInfo_DEFINED

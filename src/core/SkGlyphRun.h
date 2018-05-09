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

class SkGlyphRun {
public:
    SkGlyphRun() = default;
    SkGlyphRun(SkGlyphRun&&) = default;
    static SkGlyphRun MakeFromDrawText(
            const SkPaint& paint, const void* bytes, size_t byteLength,
            SkPoint origin);
    static SkGlyphRun MakeFromDrawPosTextH(
            const SkPaint& paint, const void* bytes, size_t byteLength,
            const SkScalar xpos[], SkScalar constY);
    static SkGlyphRun MakeFromDrawPosText(
            const SkPaint& paint, const void* bytes, size_t byteLength,
            const SkPoint pos[]);

    size_t runSize() const { return fRunSize; }
    uint16_t uniqueSize() const { return fUniqueSize; }

    // copyGlyphIDs is temporary glue to work with the existing system. Don't use with new code.
    std::unique_ptr<SkGlyphID[]> copyGlyphIDs() const;
    const SkScalar* getPositions() const {
        return reinterpret_cast<const SkScalar*>(fPositions.get());
    }

private:
    SkGlyphRun(size_t runSize,
               std::unique_ptr<uint16_t[]>&& denseIndex,
               std::unique_ptr<SkPoint[]>&& positions,
               uint16_t uniqueSize,
               std::unique_ptr<SkGlyphID[]>&& uniqueGlyphIDs);

    std::unique_ptr<uint16_t[]>  fDenseIndex;
    std::unique_ptr<SkPoint[]>   fPositions;
    std::unique_ptr<SkGlyphID[]> fUniqueGlyphs;
    const size_t                 fRunSize{0};
    const uint16_t               fUniqueSize{0};
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

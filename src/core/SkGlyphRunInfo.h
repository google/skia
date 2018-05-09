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

class SkGlyphRunInfo {
public:
    SkGlyphRunInfo() = default;
    SkGlyphRunInfo(SkGlyphRunInfo&&) = default;
    static SkGlyphRunInfo Make(const SkPaint& paint, const void* bytes, size_t byteLength);
    static SkGlyphRunInfo Make(uint16_t count, const SkGlyphID[]);
    static SkGlyphRunInfo Make(size_t byteLength, const void* utfN,
                               const SkTypeface& typeface, SkTypeface::Encoding encoding);

    uint16_t size() const { return fSize; }
    uint16_t uniqueSize() const { return SkTo<uint16_t>(fUniqueGlyphs.size()); }

    template <typename F>
    void forEachUniqueGlyphId(F f) const {
        for (auto gid : fUniqueGlyphs) {
            f(gid);
        }
    }

    template <typename F>
    void forEachGlyphId(F f) const {
        for(int i = 0; i < fSize; i++) {
            f(i, fDenseIndex[i]);
        }
    }

private:
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wuninitialized"
        struct DenseIndex {
            DenseIndex() : index(index) {}
            DenseIndex(uint16_t index_) : index(index_) {}
            operator uint16_t () const { return index; }
            uint16_t index;
        };

        struct FastPoint {
            FastPoint() : fX(fX), fY(fY) {}

            SkScalar fX, fY;
        };

    #pragma clang diagnostic pop

    SkGlyphRunInfo(uint16_t denseIndexSize, std::unique_ptr<DenseIndex[]>&& denseIndex,
                   std::vector<SkGlyphID>&& uniqueGlyphs);

    const uint16_t fSize{0};
    std::unique_ptr<DenseIndex[]> fDenseIndex;
    std::vector<SkGlyphID> fUniqueGlyphs;

    // For bitmaps, these points are 8 times the actual position to deal with sub-pixel
    // positioning. For path, and distance field the positions are unadjusted.
    std::unique_ptr<FastPoint[]> fPositions;
};

#endif  // SkGlyphRunInfo_DEFINED

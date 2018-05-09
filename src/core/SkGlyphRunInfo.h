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
    static SkGlyphRunInfo Make(const SkPaint& paint, const void* bytes, size_t byteLength);
    static SkGlyphRunInfo Make(uint16_t count, const SkGlyphID[]);
    static SkGlyphRunInfo Make(size_t byteLength, const void* utfN,
                               const SkTypeface& typeface, SkTypeface::Encoding encoding);

    void preparePositions(
        const SkPoint*, const SkPaint&, const SkMatrix&, const SkSurfaceProps* props,
        SkScalerContextFlags scFlags);

private:
    struct DrawableGlyph {
        explicit DrawableGlyph(SkGlyphID glyphID_) : glyphID{glyphID_} {}
        SkGlyphID glyphID;
        SkPoint   advance;
        SkMask    mask;
        SkPath*   path;
    };

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wuninitialized"
        struct DenseIndex {
            DenseIndex() : index(index) {}
            DenseIndex(uint16_t index_) : index(index_) {}
            operator uint16_t () const { return index; }
            uint16_t index;
        };

        struct FastPoint : public SkPoint {
            FastPoint() : fX(fX), fY(fY) {}
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

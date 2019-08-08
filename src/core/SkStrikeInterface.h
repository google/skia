/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStrikeInterface_DEFINED
#define SkStrikeInterface_DEFINED

#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkTypes.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkSpan.h"

#include <memory>

class SkDescriptor;
class SkGlyph;
class SkMaskFilter;
class SkPathEffect;
class SkTypeface;

// TODO: rename SkScalerContextEffects -> SkStrikeEffects
struct SkScalerContextEffects {
    SkScalerContextEffects() : fPathEffect(nullptr), fMaskFilter(nullptr) {}
    SkScalerContextEffects(SkPathEffect* pe, SkMaskFilter* mf)
            : fPathEffect(pe), fMaskFilter(mf) {}
    explicit SkScalerContextEffects(const SkPaint& paint)
            : fPathEffect(paint.getPathEffect())
            , fMaskFilter(paint.getMaskFilter()) {}

    SkPathEffect*   fPathEffect;
    SkMaskFilter*   fMaskFilter;
};

struct SkGlyphPos {
    size_t index;
    const SkGlyph* glyph;
    SkPoint position;
};

struct SkPathPos {
    const SkPath* path;
    SkPoint position;
};

class SkStrikeInterface {
public:
    virtual ~SkStrikeInterface() = default;
    virtual const SkDescriptor& getDescriptor() const = 0;

    enum PreparationDetail {
        kBoundsOnly,
        kImageIfNeeded,
    };

    // prepareForDrawingRemoveEmpty takes glyphIDs, and position, and returns a list of SkGlyphs
    // and positions where all the data to draw the glyph has been created. The maxDimension
    // parameter determines if the mask/SDF version will be created, or an alternate drawing
    // format should be used. For path-only drawing set maxDimension to 0, and for bitmap-device
    // drawing (where there is no upper limit to the glyph in the cache) use INT_MAX.
    // * PreparationDetail determines, in the mask case, if the mask/SDF should be generated.
    //   This does not affect the path or fallback cases.
    // prepareForDrawingRemoveEmpty should remove all empty glyphs from the returned span.
    virtual SkSpan<const SkGlyphPos>
    prepareForDrawingRemoveEmpty(const SkPackedGlyphID packedGlyphIDs[],
                                 const SkPoint positions[],
                                 size_t n,
                                 int maxDimension,
                                 PreparationDetail detail,
                                 SkGlyphPos results[]) = 0;

    // rounding() and subpixelMask are used to calculate the subpixel position of a glyph.
    // The per component (x or y) calculation is:
    //
    //   subpixelOffset = (floor((viewportPosition + rounding) & mask) >> 14) & 3
    //
    // where mask is either 0 or ~0, and rounding is either
    // 1/2 for non-subpixel or 1/8 for subpixel.
    virtual SkVector rounding() const = 0;
    virtual SkIPoint subpixelMask() const = 0;

    // Used with SkScopedStrike to take action at the end of a scope.
    virtual void onAboutToExitScope() = 0;

    struct Deleter {
        void operator()(SkStrikeInterface* ptr) const {
            ptr->onAboutToExitScope();
        }
    };
};

using SkScopedStrike = std::unique_ptr<SkStrikeInterface, SkStrikeInterface::Deleter>;

class SkStrikeCacheInterface {
public:
    virtual ~SkStrikeCacheInterface() = default;
    virtual SkScopedStrike findOrCreateScopedStrike(const SkDescriptor& desc,
                                                    const SkScalerContextEffects& effects,
                                                    const SkTypeface& typeface) = 0;
};
#endif  //SkStrikeInterface_DEFINED

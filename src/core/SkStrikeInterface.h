/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStrikeInterface_DEFINED
#define SkStrikeInterface_DEFINED

#include <memory>

#include "include/core/SkPoint.h"
#include "include/core/SkTypes.h"
#include "src/core/SkSpan.h"

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
    virtual SkVector rounding() const = 0;
    virtual const SkDescriptor& getDescriptor() const = 0;

    enum PreparationDetail {
        kBoundsOnly,
        kImageIfNeeded,
    };

    // prepareForDrawing takes glyphIDs, and position, and returns a list of SkGlyphs and
    // positions where all the data to draw the glyph has been created. The maxDimension
    // parameter determines if the mask/SDF version will be created, or an alternate drawing
    // format should be used. For path-only drawing set maxDimension to 0, and for bitmap-device
    // drawing (where there is no upper limit to the glyph in the cache) use INT_MAX.
    // * PreparationDetail determines, in the mask case, if the mask/SDF should be generated.
    //   This does not affect the path or fallback cases.
    virtual SkSpan<const SkGlyphPos> prepareForDrawing(const SkGlyphID glyphIDs[],
                                                       const SkPoint positions[],
                                                       size_t n,
                                                       int maxDimension,
                                                       PreparationDetail detail,
                                                       SkGlyphPos results[]) = 0;

    virtual const SkGlyph& getGlyphMetrics(SkGlyphID glyphID, SkPoint position) = 0;

    // If glyph does not have an existing path, then add a path to glyph using a scaler context.
    virtual const SkPath* preparePath(SkGlyph* glyph) = 0;
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

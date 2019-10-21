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
class SkDrawableGlyphBuffer;
class SkGlyph;
class SkMaskFilter;
class SkPathEffect;
class SkTypeface;
struct SkGlyphPositionRoundingSpec;
struct SkScalerContextEffects;

class SkStrikeForGPU {
public:
    virtual ~SkStrikeForGPU() = default;
    virtual const SkDescriptor& getDescriptor() const = 0;

    // prepareForDrawing takes glyphIDs, and position in the form of
    // SkDrawableGlyphBuffer, and returns a list of SkGlyphs and positions where all the data to
    // draw the glyph has been created by adjusting the SkDrawableGlyphBuffer. The maxDimension
    // parameter determines if the mask/SDF version will be created, or an alternate drawing
    // format should be used. For path-only drawing set maxDimension to 0, and for bitmap-device
    // drawing (where there is no upper limit to the glyph in the cache) use INT_MAX.
    virtual void
    prepareForDrawing(int maxGlyphDimension, SkDrawableGlyphBuffer* drawables) = 0;

    virtual const SkGlyphPositionRoundingSpec& roundingSpec() const = 0;

    // Used with SkScopedStrikeForGPU to take action at the end of a scope.
    virtual void onAboutToExitScope() = 0;

    // Common categories for glyph types used by GPU.
    static bool CanDrawAsMask(const SkGlyph& glyph);
    static bool CanDrawAsSDFT(const SkGlyph& glyph);
    static bool CanDrawAsPath(const SkGlyph& glyph);


    struct Deleter {
        void operator()(SkStrikeForGPU* ptr) const {
            ptr->onAboutToExitScope();
        }
    };
};

using SkScopedStrikeForGPU = std::unique_ptr<SkStrikeForGPU, SkStrikeForGPU::Deleter>;

class SkStrikeForGPUCacheInterface {
public:
    virtual ~SkStrikeForGPUCacheInterface() = default;
    virtual SkScopedStrikeForGPU findOrCreateScopedStrike(const SkDescriptor& desc,
                                                          const SkScalerContextEffects& effects,
                                                          const SkTypeface& typeface) = 0;
};
#endif  //SkStrikeInterface_DEFINED

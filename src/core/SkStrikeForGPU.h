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
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"

#include <memory>

class SkDescriptor;
class SkDrawableGlyphBuffer;
class SkGlyph;
class SkMaskFilter;
class SkPathEffect;
class SkSourceGlyphBuffer;
class SkStrike;
class SkStrikeSpec;
class SkTypeface;
struct SkGlyphPositionRoundingSpec;
struct SkScalerContextEffects;

namespace sktext::gpu {
class StrikeForGPU {
public:
    virtual ~StrikeForGPU() = default;
    virtual const SkDescriptor& getDescriptor() const = 0;

    virtual void prepareForMaskDrawing(
            SkDrawableGlyphBuffer* accepted, SkSourceGlyphBuffer* rejected) = 0;

    virtual void prepareForSDFTDrawing(
            SkDrawableGlyphBuffer* accepted, SkSourceGlyphBuffer* rejected) = 0;

    virtual void prepareForPathDrawing(
            SkDrawableGlyphBuffer* accepted, SkSourceGlyphBuffer* rejected) = 0;

    virtual void prepareForDrawableDrawing(
            SkDrawableGlyphBuffer* accepted, SkSourceGlyphBuffer* rejected) = 0;

    virtual const SkGlyphPositionRoundingSpec& roundingSpec() const = 0;

    // Used with SkScopedStrikeForGPU to take action at the end of a scope.
    virtual void onAboutToExitScope() = 0;

    // Return underlying SkStrike for building SubRuns while processing glyph runs.
    virtual sk_sp<SkStrike> getUnderlyingStrike() const = 0;

    // Return the maximum dimension of a span of glyphs.
    virtual SkScalar findMaximumGlyphDimension(SkSpan<const SkGlyphID> glyphs) = 0;

    // Common categories for glyph types used by GPU.
    static bool CanDrawAsMask(const SkGlyph& glyph);
    static bool CanDrawAsSDFT(const SkGlyph& glyph);
    static bool CanDrawAsPath(const SkGlyph& glyph);
    static bool FitsInAtlas(const SkGlyph& glyph);


    struct Deleter {
        void operator()(StrikeForGPU* ptr) const {
            ptr->onAboutToExitScope();
        }
    };
};

using ScopedStrikeForGPU = std::unique_ptr<StrikeForGPU, StrikeForGPU::Deleter>;

class StrikeForGPUCacheInterface {
public:
    virtual ~StrikeForGPUCacheInterface() = default;
    virtual ScopedStrikeForGPU findOrCreateScopedStrike(const SkStrikeSpec& strikeSpec) = 0;
};
}  // namespace sktext::gpu
#endif  //SkStrikeInterface_DEFINED

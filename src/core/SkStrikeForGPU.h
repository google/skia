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
class SkSourceGlyphBuffer;
class SkTypeface;
struct SkGlyphPositionRoundingSpec;
struct SkScalerContextEffects;

class SkStrikeForGPU {
public:
    virtual ~SkStrikeForGPU() = default;
    virtual const SkDescriptor& getDescriptor() const = 0;

    virtual void prepareForMaskDrawing(
            SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) = 0;

    virtual void prepareForSDFTDrawing(
            SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) = 0;

    virtual void prepareForPathDrawing(
            SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) = 0;

    virtual const SkGlyphPositionRoundingSpec& roundingSpec() const = 0;

    // Used with SkScopedStrikeForGPU to take action at the end of a scope.
    virtual void onAboutToExitScope() = 0;

    // Common categories for glyph types used by GPU.
    static bool CanDrawAsMask(const SkGlyph& glyph);
    static bool CanDrawAsSDFT(const SkGlyph& glyph);
    static bool CanDrawAsPath(const SkGlyph& glyph);
    static bool FitsInAtlas(const SkGlyph& glyph);


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

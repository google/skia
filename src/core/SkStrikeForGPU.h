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
#include "src/core/SkGlyphBuffer.h"
#include "src/core/SkSpan.h"
#include "src/core/SkZip.h"

#include <memory>
#include <vector>

class SkDescriptor;
class SkGlyph;
class SkMaskFilter;
class SkPathEffect;
class SkStrikeForGPU;
class SkTypeface;
struct SkScalerContextEffects;

struct SkGlyphPos {
    size_t index;
    const SkGlyph* glyph;
    SkPoint position;
};

struct SkPathPos {
    const SkPath* path;
    SkPoint position;
};

class SkStrikeForGPU {
public:
    virtual ~SkStrikeForGPU() = default;
    virtual const SkDescriptor& getDescriptor() const = 0;

    virtual void prepareForMaskDrawing(
            SkDrawableGlyphBuffer* drawbles, SkSourceGlyphBuffer* rejects) = 0;

    virtual void prepareForSDFTDrawing(
            SkDrawableGlyphBuffer* drawbles, SkSourceGlyphBuffer* rejects) = 0;

    virtual void prepareForPathDrawing(
            SkDrawableGlyphBuffer* drawbles, SkSourceGlyphBuffer* rejects) = 0;

    // rounding() and subpixelMask are used to calculate the subpixel position of a glyph.
    // The per component (x or y) calculation is:
    //
    //   subpixelOffset = (floor((viewportPosition + rounding) & mask) >> 14) & 3
    //
    // where mask is either 0 or ~0, and rounding is either
    // 1/2 for non-subpixel or 1/8 for subpixel.
    virtual SkVector rounding() const = 0;
    virtual SkIPoint subpixelMask() const = 0;

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

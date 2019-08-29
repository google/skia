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

struct SkGlyphIDPos {
    size_t n;
    const SkGlyphID* ids;
    const SkPoint* positions;
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

inline SkGlyphinator SkGlyphinator::Storage::makeForDeviceSpace(
        SkStrikeForGPU* strike,
        const SkMatrix& viewMatrix,
        const SkPoint& origin,
        SkGlyphIDPos glyphPos) {
    size_t runSize = glyphPos.n;
    this->ensure(runSize);

    // Add rounding and origin.
    SkMatrix matrix = viewMatrix;
    matrix.preTranslate(origin.x(), origin.y());
    SkPoint rounding = strike->rounding();
    matrix.postTranslate(rounding.x(), rounding.y());
    matrix.mapPoints(fPositions, glyphPos.positions, runSize);

    SkIPoint mask = strike->subpixelMask();

    for (size_t i = 0; i < runSize; i++) {
        SkFixed subX = SkScalarToFixed(fPositions[i].x()) & mask.x(),
                subY = SkScalarToFixed(fPositions[i].y()) & mask.y();
        fMultiBuffer[i].packedID = SkPackedGlyphID{glyphPos.ids[i], subX, subY};
    }

    return this->makeInator(runSize);
}

inline SkGlyphinator SkGlyphinator::Storage::makeForSourceSpace(const SkPoint& origin) {
    size_t runSize = fSource.size();
    SkGlyphID* glyphIDs = fSource.get<0>().data();
    SkPoint* positions = fSource.get<1>().data();

    SkMatrix::MakeTrans(origin.x(), origin.y()).mapPoints(fPositions, positions, runSize);

    for (size_t i = 0; i < fSource.size(); i++) {
        fMultiBuffer[i].packedID = SkPackedGlyphID{glyphIDs[i]};
    }

    return this->makeInator(runSize);
}
#endif  //SkStrikeInterface_DEFINED

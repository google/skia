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
#include "src/core/SkZip.h"

#include <memory>

class SkDescriptor;
class SkGlyph;
class SkMaskFilter;
class SkPathEffect;
class SkStrikeInterface;
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

struct SkGlyphIDPos {
    size_t n;
    const SkGlyphID* ids;
    const SkPoint* positions;
};

struct SkGlyphinator {
    size_t n = 0;
    union MultiElement {
        SkGlyphID glyphID;
        SkPackedGlyphID packedID;
        const SkGlyph* glyph;
        const SkPath* path;
    }* glyphs{nullptr};

    SkPoint* positions{nullptr};

    SkZip<MultiElement, SkPoint> zip() const {
        return SkZip<MultiElement, SkPoint>{n, glyphs, positions};
    }
    bool empty() const { return n == 0; }
    size_t size() const { return n; }
    SkGlyphinator first(size_t firstN) {
        return SkGlyphinator{firstN, glyphs, positions};
    }

    struct Storage {
        size_t fSize{0};
        SkAutoTMalloc<MultiElement> fMultiBuffer;
        SkAutoTMalloc<SkPoint> fPositions;
        void ensure(size_t size) {
            if (size > fSize) {
                fMultiBuffer.reset(size);
                fPositions.reset(size);
                fSize = size;
            }
        }

        SkGlyphinator makeInator(size_t size) {
            return SkGlyphinator{size, fMultiBuffer, fPositions};
        }

        SkGlyphinator makeForDeviceSpace(SkStrikeInterface* strike,
                                                const SkMatrix& viewMatrix,
                                                const SkPoint& origin,
                                                SkGlyphIDPos glyphPos);

        SkGlyphinator makeForSourceSpace(const SkPoint& origin, SkGlyphIDPos glyphPos);
    };
};

class SkStrikeInterface {
public:
    virtual ~SkStrikeInterface() = default;
    virtual const SkDescriptor& getDescriptor() const = 0;

    // prepareForDrawingRemoveEmpty takes glyphIDs, and position, and returns a list of SkGlyphs
    // and positions where all the data to draw the glyph has been created. The maxDimension
    // parameter determines if the mask/SDF version will be created, or an alternate drawing
    // format should be used. For path-only drawing set maxDimension to 0, and for bitmap-device
    // drawing (where there is no upper limit to the glyph in the cache) use INT_MAX.
    // prepareForDrawingRemoveEmpty should remove all empty glyphs from the returned span.
    virtual SkSpan<const SkGlyphPos>
    prepareForDrawingRemoveEmpty(const SkPackedGlyphID packedGlyphIDs[],
                                 const SkPoint positions[],
                                 size_t n,
                                 int maxDimension,
                                 SkGlyphPos results[]) = 0;


    virtual SkGlyphinator prepareForMaskDrawing(
            SkGlyphinator glyphPos, std::vector<size_t>& rejectIndices) = 0;

    virtual SkGlyphinator prepareForSDFTDrawing(
            SkGlyphinator glyphPos, std::vector<size_t>& rejectIndices) = 0;

    virtual SkGlyphinator prepareForPathDrawing(
            SkGlyphinator glyphPos, std::vector<size_t>& rejectIndices) = 0;

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

inline SkGlyphinator SkGlyphinator::Storage::makeForDeviceSpace(
        SkStrikeInterface* strike,
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

inline SkGlyphinator SkGlyphinator::Storage::makeForSourceSpace(
        const SkPoint& origin, SkGlyphIDPos glyphPos) {
    size_t runSize = glyphPos.n;
    this->ensure(runSize);

    SkMatrix::MakeTrans(origin.x(), origin.y()).mapPoints(fPositions, glyphPos.positions, runSize);

    for (size_t i = 0; i < glyphPos.n; i++) {
        fMultiBuffer[i].packedID = SkPackedGlyphID{glyphPos.ids[i]};
    }

    return this->makeInator(runSize);
}
#endif  //SkStrikeInterface_DEFINED

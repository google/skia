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

#if 0
struct SkGlyphinator {
    size_t n = 0;
    union MultiElement {
        SkGlyphID glyphID;
        SkPackedGlyphID packedID;
        const SkGlyph* glyph;
        const SkPath* path;
    }* glyphs{nullptr};

    SkPoint* positions{nullptr};

    SkZip<SkGlyphID, SkPoint> fSourceInput;

    SkZip<MultiElement, SkPoint> zip() const {
        return SkZip<MultiElement, SkPoint>{n, glyphs, positions};
    }
    bool empty() const { return n == 0; }
    size_t size() const { return n; }
    SkGlyphinator first(size_t firstN) {
        return SkGlyphinator{firstN, glyphs, positions};
    }

    struct Storage {

        Storage(SkZip<SkGlyphID, SkPoint> pass1) : fSource{pass1} {
            this-> ensure(pass1.size());
        }

        size_t fSize{0};
        SkAutoTMalloc<MultiElement> fMultiBuffer;
        SkAutoTMalloc<SkPoint> fPositions;
        SkAutoTMalloc<SkPoint> fSourceGlyphsBuffer;
        SkAutoTMalloc<SkPoint> fSourcePositionsBuffer;
        SkZip<SkGlyphID, SkPoint> fSource;

        void ensure(size_t size) {
            if (size > fSize) {
                fMultiBuffer.reset(size);
                fPositions.reset(size);
                fSourceGlyphsBuffer.reset(size);
                fSourcePositionsBuffer.reset(size);
                fSize = size;
            }
        }

        SkGlyphinator makeInator(size_t size) {
            return SkGlyphinator{size, fMultiBuffer, fPositions};
        }

        SkGlyphinator makeForDeviceSpace(SkStrikeForGPU* strike,
                                         const SkMatrix& viewMatrix,
                                         const SkPoint& origin);

        SkGlyphinator makeForSourceSpace(const SkPoint& origin);



        SkGlyphinator makeForOneGlyph(SkPackedGlyphID packedGlyphID, SkPoint pos) {
            this->ensure(1);
            fMultiBuffer[0].packedID = packedGlyphID;
            fPositions[0] = pos;
            return this->makeInator(1);
        }
    };
};
#endif

class SkBag {
public:
    SkBag() = default;
    SkBag& operator= (SkPackedGlyphID packedID) {
        fV.packedID = packedID;
        return *this;
    }
    SkBag& operator= (SkGlyph* glyph) {
        fV.glyph = glyph;
        return *this;
    }
    SkBag& operator= (const SkPath* path) {
        fV.path = path;
        return *this;
    }

    operator SkPackedGlyphID() { return fV.packedID; }
    operator SkGlyph*() { return fV.glyph; }
    operator const SkPath*() { return fV.path; }

private:
    union {
        SkPackedGlyphID packedID;
        SkGlyph* glyph;
        const SkPath* path;
    } fV = { .glyph = nullptr };
};

class SkPainterPassStorage {
public:
    SkPainterPassStorage() = default;

    void startSource(SkZip<SkGlyphID, SkPoint> input, SkPoint origin) {
        fSource = input;
        size_t runSize = fSource.size();
        SkGlyphID* glyphIDs = fSource.get<0>().data();
        SkPoint* positions = fSource.get<1>().data();
        SkMatrix::MakeTrans(origin.x(), origin.y()).mapPoints(fPositions, positions, runSize);

        for (size_t i = 0; i < fSource.size(); i++) {
            fMultiBuffer[i].packedID = SkPackedGlyphID{glyphIDs[i]};
        }
    }

    void startDevice(
            SkZip<SkGlyphID, SkPoint> input, SkPoint origin, const SkMatrix& viewMatrix,
            const SkStrikeForGPU& strike);

    void flipSource(SkPoint origin) {
        auto source = SkZip<SkGlyphID, SkPoint>(
                rejectedCount, fSourceGlyphsBuffer, fSourcePositionsBuffer);

        this->startSource(source, origin);
    }

    class PassForGlyphs {
        struct Iterator {
            using value_type = std::tuple<SkPackedGlyphID, SkPoint>;
            using difference_type = ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type;
            using iterator_category = std::input_iterator_tag;
            Iterator(SkBag* bag_, SkPoint* positions_)
                : bag{bag_}
                , positions{positions_} { }
            Iterator(const Iterator& that) : Iterator{that.bag, that.positions} { }
            Iterator& operator++() { ++bag; ++positions; return *this; }
            Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
            bool operator==(const Iterator& rhs) const { return bag == rhs.bag; }
            bool operator!=(const Iterator& rhs) const { return bag != rhs.bag; }
            reference operator*() {
                return value_type{*bag, *positions};
            }

            SkBag* bag;
            SkPoint* positions;
        };

    public:
        PassForGlyphs(SkPainterPassStorage* storage) : fStorage{storage} { }

        Iterator begin() { return Iterator{fStorage->fMultiBuffer, fStorage->fPositions}; }
        Iterator end() {
            size_t s = fStorage->fSource.size();
            return Iterator{fStorage->fMultiBuffer + s, fStorage->fPositions + s};
        }

        void push_back(SkGlyph* glyph) {
            fStorage->fMultiBuffer[fOutputSize].glyph = glyph;
            fStorage->fPositions[fOutputSize] = fStorage->fPositions[fProcessedGlyphs];
            fOutputSize++;
            fProcessedGlyphs++;
        }

        void reject() {
            SkGlyphID id; SkPoint pos = {0, 0};
            std::tie(id, pos) = fStorage->fSource[fProcessedGlyphs];
            fStorage->fSourceGlyphsBuffer[fRejectedSize] = id;
            fStorage->fSourcePositionsBuffer[fRejectedSize] = pos;
            fRejectedSize++;
            fProcessedGlyphs++;
        }

        SkPackedGlyphID pop_back() {
            return multiBuffer[fProcessedGlyphs].packedID;
        }

        SkZip<SkGlyph*, SkPoint> finishPass() {
            return SkZip<MultiElement, SkPoint>{
                fOutputSize,
                fStorage->fMultiBuffer,
                fStorage->fPositions};
        }

        SkPainterPassStorage* const fStorage;
        size_t fProcessedGlyphs{0};
        size_t fRejectedSize{0};
        size_t fOutputSize{0};
    };

    PassForGlyphs makePassForGlyphs() {
        return PassForGlyphs{this};
    }

private:
    size_t fSize{ 0 };
    SkAutoTMalloc<MultiElement> fMultiBuffer;
    SkAutoTMalloc<SkPoint> fPositions;
    SkAutoTMalloc<SkGlyphID> fSourceGlyphsBuffer;
    SkAutoTMalloc<SkPoint> fSourcePositionsBuffer;
    SkZip<SkGlyphID, SkPoint> fSource;

    void ensure(size_t size) {
        if (size > fSize) {
            fMultiBuffer.reset(size);
            fPositions.reset(size);
            fSourceGlyphsBuffer.reset(size);
            fSourcePositionsBuffer.reset(size);
            fSize = size;
        }
    }
};

class SkStrikeForGPU {
public:
    virtual ~SkStrikeForGPU() = default;
    virtual const SkDescriptor& getDescriptor() const = 0;

    virtual SkGlyphinator prepareForMaskDrawing(
            SkGlyphinator glyphPos, std::vector<size_t>& rejectIndices) = 0;

    virtual SkZip<SkGlyph*, SkPoint> prepareForMaskDrawing2(
            SkPainterPassStorage::PassForGlyphs* pass) = 0;

    virtual SkGlyphinator prepareForSDFTDrawing(
            SkGlyphinator glyphPos, std::vector<size_t>& rejectIndices) = 0;

    virtual SkZip<SkGlyph*, SkPoint> prepareForSDFTDrawing2(
            SkPainterPassStorage::PassForGlyphs* pass) = 0;

    virtual SkGlyphinator prepareForPathDrawing(
            SkGlyphinator glyphPos,
            std::vector<size_t>& rejectIndices,
            int* rejectedMaxDimension) = 0;

    virtual SkZip<SkGlyph*, SkPoint> prepareForPathDrawing2(
            SkPainterPassStorage::PassForGlyphs* pass, int* rejectedMaxDimension) = 0;

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

void SkPainterPassStorage::startDevice(
        SkZip<SkGlyphID, SkPoint> input, SkPoint origin, const SkMatrix& viewMatrix,
        const SkStrikeForGPU& strike) {
    fSource = input;
    size_t runSize = fSource.size();
    SkGlyphID* glyphIDs = fSource.get<0>().data();
    SkPoint* positions = fSource.get<1>().data();

    // Add rounding and origin.
    SkMatrix matrix = viewMatrix;
    matrix.preTranslate(origin.x(), origin.y());
    SkPoint rounding = strike.rounding();
    matrix.postTranslate(rounding.x(), rounding.y());
    matrix.mapPoints(fPositions, positions, runSize);

    SkIPoint mask = strike.subpixelMask();

    for (size_t i = 0; i < runSize; i++) {
        SkFixed subX = SkScalarToFixed(fPositions[i].x()) & mask.x(),
                subY = SkScalarToFixed(fPositions[i].y()) & mask.y();
        fMultiBuffer[i].packedID = SkPackedGlyphID{glyphIDs[i], subX, subY};
    }
}
#endif  //SkStrikeInterface_DEFINED

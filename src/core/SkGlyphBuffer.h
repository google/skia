/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphBuffer_DEFINED
#define SkGlyphBuffer_DEFINED

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


#endif  // SkGlyphBuffer_DEFINED

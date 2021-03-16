/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphBuffer_DEFINED
#define SkGlyphBuffer_DEFINED

#include "src/core/SkEnumerate.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkZip.h"

class SkStrikeForGPU;
struct SkGlyphPositionRoundingSpec;

// SkSourceGlyphBuffer is the source of glyphs between the different stages of character drawing.
// It starts with the glyphs and positions from the SkGlyphRun as the first source. When glyphs
// are reject by a stage they become the source for the next stage.
class SkSourceGlyphBuffer {
public:
    SkSourceGlyphBuffer() = default;

    void setSource(SkZip<const SkGlyphID, const SkPoint> source) {
        this->~SkSourceGlyphBuffer();
        new (this) SkSourceGlyphBuffer{source};
    }

    void reset();

    void reject(size_t index) {
        SkASSERT(index < fSource.size());
        if (!this->sourceIsRejectBuffers()) {
            // Need to expand the buffers for first use. All other reject sets will be fewer than
            // this one.
            auto [glyphID, pos] = fSource[index];
            fRejectedGlyphIDs.push_back(glyphID);
            fRejectedPositions.push_back(pos);
            fRejectSize++;
        } else {
            SkASSERT(fRejectSize < fRejects.size());
            fRejects[fRejectSize++] = fSource[index];
        }
    }

    void reject(size_t index, int rejectedMaxDimension) {
        fRejectedMaxDimension = std::max(fRejectedMaxDimension, rejectedMaxDimension);
        this->reject(index);
    }

    SkZip<const SkGlyphID, const SkPoint> flipRejectsToSource() {
        fRejects = SkMakeZip(fRejectedGlyphIDs, fRejectedPositions).first(fRejectSize);
        fSource = fRejects;
        fRejectSize = 0;
        fSourceMaxDimension = fRejectedMaxDimension;
        fRejectedMaxDimension = 0;
        return fSource;
    }

    SkZip<const SkGlyphID, const SkPoint> source() const { return fSource; }

    int rejectedMaxDimension() const { return fSourceMaxDimension; }

private:
    SkSourceGlyphBuffer(const SkZip<const SkGlyphID, const SkPoint>& source) {
        fSource = source;
    }
    bool sourceIsRejectBuffers() const {
        return fSource.get<0>().data() == fRejectedGlyphIDs.data();
    }

    SkZip<const SkGlyphID, const SkPoint> fSource;
    size_t fRejectSize{0};
    int fSourceMaxDimension{0};
    int fRejectedMaxDimension{0};
    SkZip<SkGlyphID, SkPoint> fRejects;
    SkSTArray<4, SkGlyphID> fRejectedGlyphIDs;
    SkSTArray<4, SkPoint> fRejectedPositions;
};

// A memory format that allows an SkPackedGlyphID, SkGlyph*, and SkPath* to occupy the same
// memory. This allows SkPackedGlyphIDs as input, and SkGlyph*/SkPath* as output using the same
// memory.
class SkGlyphVariant {
public:
    SkGlyphVariant() : fV{nullptr} { }
    SkGlyphVariant& operator= (SkPackedGlyphID packedID) {
        fV.packedID = packedID;
        SkDEBUGCODE(fTag = kPackedID);
        return *this;
    }
    SkGlyphVariant& operator= (SkGlyph* glyph) {
        fV.glyph = glyph;
        SkDEBUGCODE(fTag = kGlyph);
        return *this;

    }
    SkGlyphVariant& operator= (const SkPath* path) {
        fV.path = path;
        SkDEBUGCODE(fTag = kPath);
        return *this;
    }

    SkGlyph* glyph() const {
        SkASSERT(fTag == kGlyph);
        return fV.glyph;
    }
    const SkPath* path() const {
        SkASSERT(fTag == kPath);
        return fV.path;
    }
    SkPackedGlyphID packedID() const {
        SkASSERT(fTag == kPackedID);
        return fV.packedID;
    }

    operator SkPackedGlyphID() const { return this->packedID(); }
    operator SkGlyph*()        const { return this->glyph();    }
    operator const SkPath*()   const { return this->path();     }

private:
    union {
        SkGlyph* glyph;
        const SkPath* path;
        SkPackedGlyphID packedID;
    } fV;

#ifdef SK_DEBUG
    enum {
        kEmpty,
        kPackedID,
        kGlyph,
        kPath
    } fTag{kEmpty};
#endif
};

// A buffer for converting SkPackedGlyph to SkGlyph* or SkPath*. Initially the buffer contains
// SkPackedGlyphIDs, but those are used to lookup SkGlyph*/SkPath* which are then copied over the
// SkPackedGlyphIDs.
class SkDrawableGlyphBuffer {
public:
    void ensureSize(size_t size);

    // Load the buffer with SkPackedGlyphIDs and positions at (0, 0) ready to finish positioning
    // during drawing.
    void startSource(const SkZip<const SkGlyphID, const SkPoint>& source);

    // Load the buffer with SkPackedGlyphIDs and positions using the device transform.
    void startBitmapDevice(
            const SkZip<const SkGlyphID, const SkPoint>& source,
            SkPoint origin, const SkMatrix& viewMatrix,
            const SkGlyphPositionRoundingSpec& roundingSpec);

    // Load the buffer with SkPackedGlyphIDs, calculating positions so they can be constant.
    //
    // The positions are calculated integer positions in devices space, and the mapping of the
    // the source origin through the initial matrix is returned. It is given that these positions
    // are only reused when the blob is translated by an integral amount. Thus the shifted
    // positions are given by the following equation where (ix, iy) is the integer positions of
    // the glyph, initialMappedOrigin is (0,0) in source mapped to the device using the initial
    // matrix, and newMappedOrigin is (0,0) in source mapped to the device using the current
    // drawing matrix.
    //
    //    (ix', iy') = (ix, iy) + round(newMappedOrigin - initialMappedOrigin)
    //
    // In theory, newMappedOrigin - initialMappedOrigin should be integer, but the vagaries of
    // floating point don't guarantee that, so force it to integer.
    void startGPUDevice(
            const SkZip<const SkGlyphID, const SkPoint>& source,
            const SkMatrix& drawMatrix,
            const SkGlyphPositionRoundingSpec& roundingSpec);

    SkString dumpInput() const;

    // The input of SkPackedGlyphIDs
    SkZip<SkGlyphVariant, SkPoint> input() {
        SkASSERT(fPhase == kInput);
        SkDEBUGCODE(fPhase = kProcess);
        return SkZip<SkGlyphVariant, SkPoint>{fInputSize, fMultiBuffer, fPositions};
    }

    // Store the glyph in the next drawable slot, using the position information located at index
    // from.
    void push_back(SkGlyph* glyph, size_t from) {
        SkASSERT(fPhase == kProcess);
        SkASSERT(fDrawableSize <= from);
        fPositions[fDrawableSize] = fPositions[from];
        fMultiBuffer[fDrawableSize] = glyph;
        fDrawableSize++;
    }

    // Store the path in the next drawable slot, using the position information located at index
    // from.
    void push_back(const SkPath* path, size_t from) {
        SkASSERT(fPhase == kProcess);
        SkASSERT(fDrawableSize <= from);
        fPositions[fDrawableSize] = fPositions[from];
        fMultiBuffer[fDrawableSize] = path;
        fDrawableSize++;
    }

    // The result after a series of push_backs of drawable SkGlyph* or SkPath*.
    SkZip<SkGlyphVariant, SkPoint> drawable() {
        SkASSERT(fPhase == kProcess);
        SkDEBUGCODE(fPhase = kDraw);
        return SkZip<SkGlyphVariant, SkPoint>{fDrawableSize, fMultiBuffer, fPositions};
    }

    bool drawableIsEmpty() const {
        SkASSERT(fPhase == kProcess || fPhase == kDraw);
        return fDrawableSize == 0;
    }

    void reset();

    template <typename Fn>
    void forEachGlyphID(Fn&& fn) {
        for (auto [i, packedID, pos] : SkMakeEnumerate(this->input())) {
            fn(i, packedID.packedID(), pos);
        }
    }

private:
    size_t fMaxSize{0};
    size_t fInputSize{0};
    size_t fDrawableSize{0};
    SkAutoTMalloc<SkGlyphVariant> fMultiBuffer;
    SkAutoTMalloc<SkPoint> fPositions;

#ifdef SK_DEBUG
    enum {
        kReset,
        kInput,
        kProcess,
        kDraw
    } fPhase{kReset};
#endif
};
#endif  // SkGlyphBuffer_DEFINED

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
    // We are looking for constant values for the x,y positions for all the glyphs that are not
    // dependant on the device origin Q such that we can just add a new value to translate all the
    // glyph positions to a new device origin Q'. We want (cx,cy,0) + [Q'](0,0,1) draw the blob
    // with device origin Q'. Ultimately we show there is an integer solution for the glyph
    // positions where (ix,iy,0) + ([Q'](0,0,1) + (sx,sy,0)) both parts of the top level + are
    // integers, and preserve all the flooring properties.
    //
    // Given (px,py) the glyph origin in source space. The glyph origin in device space (x,y) is:
    //   (x,y,1) = Floor([R][V][O](px,py,1))
    // where:
    //   * R - is the rounding matrix given as translate(sampling_freq_x/2, sampling_freq_y/2).
    //   * V - is the mapping from source space to device space.
    //   * O - is the blob origin given, as translate(origin.x(), origin.y()).
    //   * (px,py,1) - is the vector of the glyph origin in source space. There is a position for
    //                 each glyph.
    //
    // The three matrices R,V, and O constitute the device origin Q. [Q] = [R][V][O]. Thus,
    //   (x,y,1) = Floor([Q](0,0,1) + [V](px,py,0)).
    //   Note: [V](px,py,0) is the vector transform without the translation portion of V. That
    //         translation of V is accounted for in [Q](0,0,1)..
    //
    // If we want to translate the blog from the device origin Q to the device position Q', we
    // can use the following translation. Restate as [Q'](0,0,1) - [Q](0,0,1).
    //   (x',y',1) = Floor([Q](0,0,1) + [V](px,py,0) + [Q'](0,0,1) - [Q](0,0,1)).
    //
    // We are given that [Q'](0,0,1) - [Q](0,0,1) is an integer translation. We can recast the
    // translation as:
    //   (x',y',1) = Floor([Q](0,0,1) + [V](px,py,0)) + [Q'](0,0,1) - [Q](0,0,1)               (1)
    //
    // We can now see that (cx, cy,0) is constructed but dropping [Q'](0,0,1) from above.
    //   (cx,cy,0) = Floor([Q](0,0,1) + [V](px,py,0)) - [Q](0,0,1)
    //
    // Notice that cx and cy are not guaranteed to be integers because [Q](0,0,1) is not
    // constrained to be integer; only [Q'](0,0,1) - [Q](0,0,1) is constrained to be an integer.
    //
    // Let Floor([Q](0,0,1)) be the integer portion and {[Q](0,0,1)} be the fractional portion
    // calculated as [Q](0,0,1) - Floor([Q](0,0,1). This vector has a zero in the third place.
    // Rewriting (1) with this substitution of Floor([Q](0,0,1)) + {[Q](0,0,1)} for [Q](0,0,1).
    //    (x',y',1) = Floor([Q](0,0,1) + [V](px,py,0)) + [Q'](0,0,1) - [Q](0,0,1)
    // becomes,
    //    (x',y',1) = Floor(Floor([Q](0,0,1)) + {[Q](0,0,1)} + [V](px,py,0)) +
    //                [Q'](0,0,1) - (Floor([Q](0,0,1)) + {[Q](0,0,1)})
    // simplifying by moving Floor([Q](0,0,1)) out of the Floor(),
    //    (x',y',1) = Floor({[Q](0,0,1)} + [V](px,py,0)) +
    //                [Q'](0,0,1) + Floor([Q](0,0,1)) - Floor([Q](0,0,1)) - {[Q](0,0,1)}
    // removing terms that result in zero gives,
    //    (x',y',1) = Floor({[Q](0,0,1)} + [V](px,py,0)) + [Q'](0,0,1) - {[Q](0,0,1)}
    // Notice that [Q'](0,0,1) - {[Q](0,0,1)} and Floor({[Q](0,0,1)} + [V](px,py,0)) are integer.
    // Let,
    //    (ix,iy,0) = Floor({[Q](0,0,1)} + [V](px,py,0)),
    //    (sx,sy,0) = -{[Q](0,0,1)}.
    // I call the (sx,sy,0) value the smidge.
    // Thus,
    //    (x',y',1) = (ix,iy,0) + ([Q](0,0,1) + (sx,sy,0)).
    //
    // As a matter of practicality, we have the following already calculated for sub-pixel
    // positioning, and use it to calculate (ix,iy,0):
    //    (fx,fy,1) = [R][V][O](px,py,1)
    //                = [Q](0,0,1) + [V](px,py,0)
    //                = Floor([Q](0,0,1)) + {[Q](0,0,1)} + [V](px,py,0)
    // So,
    //    (ix,iy,0) = Floor((fx,fy,1) - Floor([Q](0,0,1))).
    //
    // When calculating Q' we don't have the values for [R]. But we can just fold the values for
    // R into the smidge, so the full definition of smidge is:
    //    (sx,sy,0) = (rx,ry,1) - ([Q](0,0,1) - Floor([Q](0,0,1)),
    //              = Floor([Q](0,0,1) - [Q](0,0,1) + (rx,ry,1).
    //
    // Returns the smidge -- (sx,sy,0).
    SkPoint startGPUDevice(
            const SkZip<const SkGlyphID, const SkPoint>& source,
            SkPoint origin, const SkMatrix& viewMatrix,
            const SkGlyphPositionRoundingSpec& roundingSpec);

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

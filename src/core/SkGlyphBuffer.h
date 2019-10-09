/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphBuffer_DEFINED
#define SkGlyphBuffer_DEFINED

#include "src/core/SkGlyph.h"
#include "src/core/SkZip.h"

class SkStrikeForGPU;
struct SkGlyphPositionRoundingSpec;

// A memory format that allows an SkPackedGlyphID, SkGlyph*, and SkPath* to occupy the same
// memory. This allows SkPackedGlyphIDs as input, and SkGlyph*/SkPath* as output using the same
// memory.
//
// SkBag is a purposefully bad name; I'm hoping inspiration will strike.
class SkBag {
public:
    SkBag() : fV{nullptr} { }
    SkBag& operator= (SkPackedGlyphID packedID) { fV.packedID = packedID; return *this; }
    SkBag& operator= (SkGlyph* glyph)           { fV.glyph = glyph; return *this;       }
    SkBag& operator= (const SkPath* path)       { fV.path = path; return *this;         }

    SkGlyph* glyph()           const { return fV.glyph;    }
    const SkPath* path()       const { return fV.path;     }
    SkPackedGlyphID packedID() const { return fV.packedID; }

    operator SkPackedGlyphID() const { return this->packedID(); }
    operator SkGlyph*()        const { return this->glyph();    }
    operator const SkPath*()   const { return this->path();     }

private:
    union {
        SkGlyph* glyph;
        const SkPath* path;
        SkPackedGlyphID packedID;
    } fV;
};

class SkSourceGlyphBuffer {
public:
    SkSourceGlyphBuffer() = default;

    void setSource(SkZip<const SkGlyphID, const SkPoint> source) {
        // Start with rejects pointing to internal buffers.
        fRejects = SkZip<SkGlyphID, SkPoint>{
            source.size(), fRejectedGlyphIDs.get(), fRejectedPositions.get()};
        fSource = source;
    }

    void ensureSize(size_t size);
    void reset();

    void reject(size_t index) {
        fRejects[fRejectSize++] = fSource[index];
    }

    void reject(size_t index, int rejectedMaxDimension) {
        fRejectedMaxDimension = SkTMax(fRejectedMaxDimension, rejectedMaxDimension);
        this->reject(index);
    }

    SkZip<const SkGlyphID, const SkPoint> flipRejectsToSource() {
        fSource = fRejects.first(fRejectSize);
        fRejectSize = 0;
        return fSource;
    }

    SkZip<const SkGlyphID, const SkPoint> source() const { return fSource; }

    int rejectedMaxDimension() const { return fRejectedMaxDimension; }

private:
    SkZip<const SkGlyphID, const SkPoint> fSource;
    size_t fMaxSize{0};
    size_t fRejectSize{0};
    int fRejectedMaxDimension{0};
    SkZip<SkGlyphID, SkPoint> fRejects;
    SkAutoTMalloc<SkGlyphID> fRejectedGlyphIDs;
    SkAutoTMalloc<SkPoint> fRejectedPositions;
};

// A buffer for converting SkPackedGlyph to SkGlyph* or SkPath*. Initially the buffer contains
// SkPackedGlyphIDs, but those are used to lookup SkGlyph*/SkPath* which are then copied over the
// SkPackedGlyphIDs.
class SkDrawableGlyphBuffer {
public:
    // Load the buffer with SkPackedGlyphIDs and positions in source space.
    void startSource(const SkZip<const SkGlyphID, const SkPoint>& source, SkPoint origin);

    // Load the buffer with SkPackedGlyphIDs and positions using the device transform.
    void startDevice(
            const SkZip<const SkGlyphID, const SkPoint>& source,
            SkPoint origin, const SkMatrix& viewMatrix,
            const SkGlyphPositionRoundingSpec& roundingSpec);

    void ensureSize(size_t size);
    void reset();

    // Store the glyph in the next drawable slot, using the position information located at index
    // from.
    void push_back(SkGlyph* glyph, size_t from) {
        SkASSERT(fDrawableSize <= from);
        fPositions[fDrawableSize] = fPositions[from];
        fMultiBuffer[fDrawableSize] = glyph;
        fDrawableSize++;
    }

    // Store the path in the next drawable slot, using the position information located at index
    // from.
    void push_back(const SkPath* path, size_t from) {
        SkASSERT(fDrawableSize <= from);
        fPositions[fDrawableSize] = fPositions[from];
        fMultiBuffer[fDrawableSize] = path;
        fDrawableSize++;
    }

    // The result after a series of push_backs of drawable SkGlyph* or SkPath*.
    SkZip<SkBag, SkPoint> drawable() {
        return SkZip<SkBag, SkPoint>{fDrawableSize, fMultiBuffer, fPositions};
    }

    // The input of SkPackedGlyphIDs
    SkZip<SkBag, SkPoint> input() {
        return SkZip<SkBag, SkPoint>{fInputSize, fMultiBuffer, fPositions};
    }

private:
    size_t fMaxSize{0};
    size_t fInputSize{0};
    size_t fDrawableSize{0};
    SkAutoTMalloc<SkBag> fMultiBuffer;
    SkAutoTMalloc<SkPoint> fPositions;
};

#endif  // SkGlyphBuffer_DEFINED

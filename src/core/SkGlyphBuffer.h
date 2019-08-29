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

class SkBag {
public:
    SkBag() : fV{nullptr} { }
    SkBag& operator= (SkPackedGlyphID packedID) { fV.packedID = packedID; return *this; }
    SkBag& operator= (SkGlyph* glyph)           { fV.glyph = glyph; return *this;       }
    SkBag& operator= (const SkPath* path)       { fV.path = path; return *this;         }

    SkGlyph* glyph() const           { return fV.glyph;    }
    const SkPath* path() const       { return fV.path;     }
    SkPackedGlyphID packedID() const { return fV.packedID; }


    operator SkPackedGlyphID() { return this->packedID(); }
    operator SkGlyph*()        { return this->glyph();    }
    operator const SkPath*()   { return this->path();     }

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

    void reset(SkZip<const SkGlyphID, const SkPoint> source) {
        fRejectedGlyphIDs.reset(source.size());
        fRejectedPositions.reset(source.size());

        // Start with rejects pointing to internal buffers.
        fRejects = SkZip<SkGlyphID, SkPoint>{source.size(), fRejectedGlyphIDs, fRejectedPositions};
        fSource = source;
    }

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
    size_t fRejectSize{0};
    int fRejectedMaxDimension{0};
    SkZip<SkGlyphID, SkPoint> fRejects;
    SkAutoTMalloc<SkGlyphID> fRejectedGlyphIDs;
    SkAutoTMalloc<SkPoint> fRejectedPositions;
};

class SkDrawableGlyphBuffer {
public:
    void startSource(const SkZip<const SkGlyphID, const SkPoint>& source, SkPoint origin);

    void startDevice(
            const SkZip<const SkGlyphID, const SkPoint>& source,
            SkPoint origin, const SkMatrix& viewMatrix, const SkStrikeForGPU& strike);

    void push_back(SkGlyph* glyph, size_t from) {
        SkASSERT(fDrawableSize <= from);
        fPositions[fDrawableSize] = fPositions[from];
        fMultiBuffer[fDrawableSize] = glyph;
        fDrawableSize++;
    }

    void push_back(const SkPath* path, size_t from) {
        SkASSERT(fDrawableSize <= from);
        fPositions[fDrawableSize] = fPositions[from];
        fMultiBuffer[fDrawableSize] = path;
        fDrawableSize++;
    }

    SkZip<SkBag, SkPoint> drawable() {
        return SkZip<SkBag, SkPoint>{fDrawableSize, fMultiBuffer, fPositions};
    }

    SkZip<SkBag, SkPoint> input() {
        return SkZip<SkBag, SkPoint>{fInputSize, fMultiBuffer, fPositions};
    }

    bool inputEmpty() const { return fInputSize != 0; }

private:
    void reset(size_t size) {
        if (size > fBufferSize) {
            fMultiBuffer.reset(size);
            fPositions.reset(size);
            fBufferSize = size;
        }
        fInputSize = size;
        fDrawableSize = 0;
    }
    size_t fBufferSize{0};
    size_t fInputSize{0};
    size_t fDrawableSize{0};
    SkAutoTMalloc<SkBag> fMultiBuffer;
    SkAutoTMalloc<SkPoint> fPositions;
};

#endif  // SkGlyphBuffer_DEFINED

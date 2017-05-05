/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorLookUpTable_DEFINED
#define SkColorLookUpTable_DEFINED

#include "SkRefCnt.h"
#include "SkTemplates.h"

static constexpr uint8_t kMaxColorChannels = 4;

class SkColorLookUpTable : public SkRefCnt {
public:
    static constexpr uint8_t kOutputChannels = 3;

    SkColorLookUpTable(uint8_t inputChannels, const uint8_t gridPoints[kMaxColorChannels])
        : fInputChannels(inputChannels) {
        SkASSERT(inputChannels >= 1 && inputChannels <= kMaxColorChannels);
        memcpy(fGridPoints, gridPoints, fInputChannels * sizeof(uint8_t));

        for (int i = 0; i < inputChannels; i++) {
            SkASSERT(fGridPoints[i] > 1);
        }
    }

    /**
     *  If fInputChannels == kOutputChannels == 3, performs tetrahedral interpolation, otherwise
     *  performs multilinear interpolation (ie LERP for n =1, bilinear for n=2, trilinear for n=3)
     *  with fInputChannels input dimensions and kOutputChannels output dimensions.
     *  |dst| can be |src| only when fInputChannels == kOutputChannels == 3
     *  |dst| is the destination pixel, must have at least kOutputChannels elements.
     *  |src| is the source pixel, must have at least fInputChannels elements.
     */
    void interp(float* dst, const float* src) const;

    int inputChannels() const { return fInputChannels; }

    int outputChannels() const { return kOutputChannels; }

    int gridPoints(int dimension) const {
        SkASSERT(dimension >= 0 && dimension < inputChannels());
        return fGridPoints[dimension];
    }

private:
    const float* table() const {
        return SkTAddOffset<const float>(this, sizeof(SkColorLookUpTable));
    }

    /**
     *  Performs tetrahedral interpolation with 3 input and 3 output dimensions.
     *  |dst| can be |src|
     */
    void interp3D(float* dst, const float* src) const;

    // recursively LERPs one dimension at a time. Used by interp() for the general case
    float interpDimension(const float* src, int inputDimension, int outputDimension,
                          int index[kMaxColorChannels]) const;

    uint8_t fInputChannels;
    uint8_t fGridPoints[kMaxColorChannels];

public:
    // Objects of this type are created in a custom fashion using sk_malloc_throw
    // and therefore must be sk_freed.
    void* operator new(size_t size) = delete;
    void* operator new(size_t, void* p) { return p; }
    void operator delete(void* p) { sk_free(p); }
};

#endif

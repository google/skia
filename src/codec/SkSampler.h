/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkSampler_DEFINED
#define SkSampler_DEFINED

#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkTypes.h"

class SkSampler : public SkNoncopyable {
public:
    /**
     *  Update the sampler to sample every sampleX'th pixel. Returns the
     *  width after sampling.
     */
    int setSampleX(int sampleX) {
        return this->onSetSampleX(sampleX);
    }

    /**
     *  Update the sampler to sample every sampleY'th row.
     */
    void setSampleY(int sampleY) {
        fSampleY = sampleY;
    }

    /**
     *  Retrieve the value set for sampleY.
     */
    int sampleY() const {
        return fSampleY;
    }

    /**
     *  Based on fSampleY, return whether this row belongs in the output.
     *
     *  @param row Row of the image, starting with the first row in the subset.
     */
    bool rowNeeded(int row) const {
        return (row - get_start_coord(fSampleY)) % fSampleY == 0;
    }

    /**
     * Fill the remainder of the destination with a single color
     *
     * @param info
     * Contains the color type of the rows to fill.
     * Contains the width of the destination rows to fill
     * Contains the number of rows that we need to fill.
     *
     * @param dst
     * The destination row to fill from.
     *
     * @param rowBytes
     * Stride in bytes of the destination.
     *
     * @param colorOrIndex
     * If colorType is kF16, colorOrIndex is treated as a 64-bit color.
     * If colorType is kN32, colorOrIndex is treated as a 32-bit color.
     * If colorType is k565, colorOrIndex is treated as a 16-bit color.
     * If colorType is kGray, colorOrIndex is treated as an 8-bit color.
     * If colorType is kIndex, colorOrIndex is treated as an 8-bit index.
     * Other SkColorTypes are not supported.
     *
     * @param zeroInit
     * Indicates whether memory is already zero initialized.
     *
     */
    static void Fill(const SkImageInfo& info, void* dst, size_t rowBytes,
            uint64_t colorOrIndex, SkCodec::ZeroInitialized zeroInit);

    /**
     * Allow subclasses to implement unique versions of fill().
     */
    virtual void fill(const SkImageInfo& info, void* dst, size_t rowBytes,
            uint64_t colorOrIndex, SkCodec::ZeroInitialized zeroInit) {}

    SkSampler()
        : fSampleY(1)
    {}

    virtual ~SkSampler() {}
private:
    int fSampleY;

    virtual int onSetSampleX(int) = 0;
};

#endif // SkSampler_DEFINED

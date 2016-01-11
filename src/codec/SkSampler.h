/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkSampler_DEFINED
#define SkSampler_DEFINED

#include "SkCodec.h"
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
            uint32_t colorOrIndex, SkCodec::ZeroInitialized zeroInit);

    /**
     * Allow subclasses to implement unique versions of fill().
     */
    virtual void fill(const SkImageInfo& info, void* dst, size_t rowBytes,
            uint32_t colorOrIndex, SkCodec::ZeroInitialized zeroInit) {}

    virtual ~SkSampler() {}
private:

    virtual int onSetSampleX(int) = 0;
};

#endif // SkSampler_DEFINED

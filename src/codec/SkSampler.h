/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkSampler_DEFINED
#define SkSampler_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkNoncopyable.h"
#include "src/codec/SkCodecPriv.h"

#include <cstddef>

struct SkImageInfo;

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
        return (row - SkCodecPriv::GetStartCoord(fSampleY)) % fSampleY == 0;
    }

    /**
     * Fill the remainder of the destination with 0.
     *
     * 0 has a different meaning depending on the SkColorType. For color types
     * with transparency, this means transparent. For k565 and kGray, 0 is
     * black.
     *
     * @param info
     * Contains the color type of the rows to fill.
     * Contains the pixel width of the destination rows to fill
     * Contains the number of rows that we need to fill.
     *
     * @param dst
     * The destination row to fill.
     *
     * @param rowBytes
     * Stride in bytes of the destination.
     *
     * @param zeroInit
     * Indicates whether memory is already zero initialized.
     */
    static void Fill(const SkImageInfo& info, void* dst, size_t rowBytes,
                     SkCodec::ZeroInitialized zeroInit);

    virtual int fillWidth() const = 0;

    SkSampler()
        : fSampleY(1)
    {}

    virtual ~SkSampler() {}
private:
    int fSampleY;

    virtual int onSetSampleX(int) = 0;
};

#endif // SkSampler_DEFINED

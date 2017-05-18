/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMaskSwizzler_DEFINED
#define SkMaskSwizzler_DEFINED

#include "SkMasks.h"
#include "SkSampler.h"
#include "SkSwizzler.h"
#include "SkTypes.h"

/*
 *
 * Used to swizzle images whose pixel components are extracted by bit masks
 * Currently only used by bmp
 *
 */
class SkMaskSwizzler : public SkSampler {
public:

    /*
     * Create a new swizzler
     * @param masks Unowned pointer to helper class
     */
    static SkMaskSwizzler* CreateMaskSwizzler(const SkImageInfo& dstInfo,
                                              const SkImageInfo& srcInfo,
                                              SkMasks* masks,
                                              uint32_t bitsPerPixel,
                                              const SkCodec::Options& options);

    /*
     * Swizzle a row
     */
    void swizzle(void* dst, const uint8_t* SK_RESTRICT src);

    /**
     * Implement fill using a custom width.
     */
    void fill(const SkImageInfo& info, void* dst, size_t rowBytes, uint64_t colorOrIndex,
            SkCodec::ZeroInitialized zeroInit) override {
        const SkImageInfo fillInfo = info.makeWH(fDstWidth, info.height());
        SkSampler::Fill(fillInfo, dst, rowBytes, colorOrIndex, zeroInit);
    }

    /**
     *  Returns the byte offset at which we write to destination memory, taking
     *  scaling, subsetting, and partial frames into account.
     *  A similar function exists on SkSwizzler.
     */
    int swizzleWidth() const { return fDstWidth; }

private:

    /*
     * Row procedure used for swizzle
     */
    typedef void (*RowProc)(void* dstRow, const uint8_t* srcRow, int width,
            SkMasks* masks, uint32_t startX, uint32_t sampleX);

    SkMaskSwizzler(SkMasks* masks, RowProc proc, int subsetWidth, int srcOffset);

    int onSetSampleX(int) override;

    SkMasks*        fMasks;           // unowned
    const RowProc   fRowProc;

    // FIXME: Can this class share more with SkSwizzler? These variables are all the same.
    const int       fSubsetWidth;     // Width of the subset of source before any sampling.
    int             fDstWidth;        // Width of dst, which may differ with sampling.
    int             fSampleX;
    int             fSrcOffset;
    int             fX0;
};

#endif

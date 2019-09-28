/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMaskSwizzler_DEFINED
#define SkMaskSwizzler_DEFINED

#include "include/core/SkTypes.h"
#include "src/codec/SkMasks.h"
#include "src/codec/SkSampler.h"
#include "src/codec/SkSwizzler.h"

/*
 *
 * Used to swizzle images whose pixel components are extracted by bit masks
 * Currently only used by bmp
 *
 */
class SkMaskSwizzler : public SkSampler {
public:

    /*
     * @param masks Unowned pointer to helper class
     */
    static SkMaskSwizzler* CreateMaskSwizzler(const SkImageInfo& dstInfo,
                                              bool srcIsOpaque,
                                              SkMasks* masks,
                                              uint32_t bitsPerPixel,
                                              const SkCodec::Options& options);

    /*
     * Swizzle a row
     */
    void swizzle(void* dst, const uint8_t* SK_RESTRICT src);

    int fillWidth() const override {
        return fDstWidth;
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

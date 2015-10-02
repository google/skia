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
                                              uint32_t bitsPerPixel);

    /*
     * Swizzle a row
     */
    SkSwizzler::ResultAlpha swizzle(void* dst, const uint8_t* SK_RESTRICT src);

private:

    /*
     * Row procedure used for swizzle
     */
    typedef SkSwizzler::ResultAlpha (*RowProc)(
            void* dstRow, const uint8_t* srcRow, int width,
            SkMasks* masks, uint32_t startX, uint32_t sampleX);

    /*
     * Constructor for mask swizzler
     */
    SkMaskSwizzler(int width, SkMasks* masks, RowProc proc);

    int onSetSampleX(int) override;

    SkMasks*        fMasks;           // unowned
    const RowProc   fRowProc;

    // FIXME: Can this class share more with SkSwizzler? These variables are all the same.
    const int       fSrcWidth;        // Width of the source - i.e. before any sampling.
    int             fDstWidth;        // Width of dst, which may differ with sampling.
    int             fSampleX;
    int             fX0;
};

#endif

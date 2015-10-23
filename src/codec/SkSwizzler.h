/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSwizzler_DEFINED
#define SkSwizzler_DEFINED

#include "SkCodec.h"
#include "SkColor.h"
#include "SkImageInfo.h"
#include "SkSampler.h"

class SkSwizzler : public SkSampler {
public:
    /**
     *  Enum describing the config of the source data.
     */
    enum SrcConfig {
        kUnknown,  // Invalid type.
        kBit,      // A single bit to distinguish between white and black
        kGray,
        kIndex1,
        kIndex2,
        kIndex4,
        kIndex,
        kRGB,
        kBGR,
        kRGBX,
        kBGRX,
        kRGBA,
        kBGRA,
        kRGB_565,
        kCMYK,
    };

    /*
     *
     * Result code for the alpha components of a row.
     *
     */
    typedef uint16_t ResultAlpha;
    static const ResultAlpha kOpaque_ResultAlpha = 0xFFFF;
    static const ResultAlpha kTransparent_ResultAlpha = 0x0000;

    /*
     *
     * Checks if the result of decoding a row indicates that the row was
     * transparent.
     *
     */
    static bool IsTransparent(ResultAlpha r) {
        return kTransparent_ResultAlpha == r;
    }

    /*
     *
     * Checks if the result of decoding a row indicates that the row was
     * opaque.
     *
     */
    static bool IsOpaque(ResultAlpha r) {
        return kOpaque_ResultAlpha == r;
    }

    /*
     *
     * Constructs the proper result code based on accumulated alpha masks
     *
     */
    static ResultAlpha GetResult(uint8_t zeroAlpha, uint8_t maxAlpha);

    /*
     *
     * Returns bits per pixel for source config
     *
     */
    static int BitsPerPixel(SrcConfig sc) {
        switch (sc) {
            case kBit:
            case kIndex1:
                return 1;
            case kIndex2:
                return 2;
            case kIndex4:
                return 4;
            case kGray:
            case kIndex:
                return 8;
            case kRGB_565:
                return 16;
            case kRGB:
            case kBGR:
                return 24;
            case kRGBX:
            case kRGBA:
            case kBGRX:
            case kBGRA:
            case kCMYK:
                return 32;
            default:
                SkASSERT(false);
                return 0;
        }
    }

    /*
     *
     * Returns bytes per pixel for source config
     * Raises an error if each pixel is not stored in an even number of bytes
     *
     */
    static int BytesPerPixel(SrcConfig sc) {
        SkASSERT(SkIsAlign8(BitsPerPixel(sc)));
        return BitsPerPixel(sc) >> 3;
    }

    /**
     *  Create a new SkSwizzler.
     *  @param SrcConfig Description of the format of the source.
     *  @param ctable Unowned pointer to an array of up to 256 colors for an
     *                index source.
     *  @param dstInfo Describes the destination.
     *  @param options Indicates if dst is zero-initialized. The
     *                         implementation may choose to skip writing zeroes
     *                         if set to kYes_ZeroInitialized.
     *                 Contains subset information.
     *  @return A new SkSwizzler or nullptr on failure.
     */
    static SkSwizzler* CreateSwizzler(SrcConfig, const SkPMColor* ctable,
                                      const SkImageInfo& dstInfo, const SkCodec::Options&);

    /**
     *  Swizzle a line. Generally this will be called height times, once
     *  for each row of source.
     *  By allowing the caller to pass in the dst pointer, we give the caller
     *  flexibility to use the swizzler even when the encoded data does not
     *  store the rows in order.  This also improves usability for scaled and
     *  subset decodes.
     *  @param dst Where we write the output.
     *  @param src The next row of the source data.
     *  @return A result code describing if the row was fully opaque, fully
     *          transparent, or neither
     */
    ResultAlpha swizzle(void* dst, const uint8_t* SK_RESTRICT src);

    /**
     * Implement fill using a custom width.
     */
    void fill(const SkImageInfo& info, void* dst, size_t rowBytes, uint32_t colorOrIndex,
            SkCodec::ZeroInitialized zeroInit) override {
        const SkImageInfo fillInfo = info.makeWH(fDstWidth, info.height());
        SkSampler::Fill(fillInfo, dst, rowBytes, colorOrIndex, zeroInit);
    }

private:

    /**
     *  Method for converting raw data to Skia pixels.
     *  @param dstRow Row in which to write the resulting pixels.
     *  @param src Row of src data, in format specified by SrcConfig
     *  @param dstWidth Width in pixels of the destination
     *  @param bpp if bitsPerPixel % 8 == 0, deltaSrc is bytesPerPixel
     *             else, deltaSrc is bitsPerPixel
     *  @param deltaSrc bpp * sampleX
     *  @param ctable Colors (used for kIndex source).
     *  @param offset The offset before the first pixel to sample.
                        Is in bytes or bits based on what deltaSrc is in.
     */
    typedef ResultAlpha (*RowProc)(void* SK_RESTRICT dstRow,
                                   const uint8_t* SK_RESTRICT src,
                                   int dstWidth, int bpp, int deltaSrc, int offset,
                                   const SkPMColor ctable[]);

    const RowProc       fRowProc;
    const SkPMColor*    fColorTable;      // Unowned pointer
    const int           fSrcOffset;       // Offset of the src in pixels, allows for partial
                                          // scanline decodes.
    int                 fX0;              // Start coordinate for the src, may be different than
                                          // fSrcOffset if we are sampling.
    const int           fSubsetWidth;     // Width of the subset of the source before any sampling.
    int                 fDstWidth;        // Width of dst, which may differ with sampling.
    int                 fSampleX;         // step between X samples
    const int           fBPP;             // if bitsPerPixel % 8 == 0
                                          //     fBPP is bytesPerPixel
                                          // else
                                          //     fBPP is bitsPerPixel

    SkSwizzler(RowProc proc, const SkPMColor* ctable, int srcOffset, int subsetWidth, int bpp);

    int onSetSampleX(int) override;

};
#endif // SkSwizzler_DEFINED

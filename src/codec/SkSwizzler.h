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
     *  Create a new SkSwizzler.
     *  @param encodedInfo Description of the format of the encoded data.
     *  @param ctable Unowned pointer to an array of up to 256 colors for an
     *                index source.
     *  @param dstInfo Describes the destination.
     *  @param options Indicates if dst is zero-initialized. The
     *                         implementation may choose to skip writing zeroes
     *                         if set to kYes_ZeroInitialized.
     *                 Contains partial scanline information.
     *  @param frame   Is non-NULL if the source pixels are part of an image
     *                 frame that is a subset of the full image.
     *  @param skipFormatConversion Indicates that we should skip format conversion.
     *                              The swizzler only needs to sample and/or subset.
     *
     *  Note that a deeper discussion of partial scanline subsets and image frame
     *  subsets is below.  Currently, we do not support both simultaneously.  If
     *  options->fSubset is non-NULL, frame must be NULL.
     *
     *  @return A new SkSwizzler or nullptr on failure.
     */
    static SkSwizzler* CreateSwizzler(const SkEncodedInfo& encodedInfo, const SkPMColor* ctable,
                                      const SkImageInfo& dstInfo, const SkCodec::Options&,
                                      const SkIRect* frame = nullptr,
                                      bool skipFormatConversion = false);

    /**
     *  Swizzle a line. Generally this will be called height times, once
     *  for each row of source.
     *  By allowing the caller to pass in the dst pointer, we give the caller
     *  flexibility to use the swizzler even when the encoded data does not
     *  store the rows in order.  This also improves usability for scaled and
     *  subset decodes.
     *  @param dst Where we write the output.
     *  @param src The next row of the source data.
     */
    void swizzle(void* dst, const uint8_t* SK_RESTRICT src);

    /**
     * Implement fill using a custom width.
     */
    void fill(const SkImageInfo& info, void* dst, size_t rowBytes, uint64_t colorOrIndex,
            SkCodec::ZeroInitialized zeroInit) override {
        const SkImageInfo fillInfo = info.makeWH(fAllocatedWidth, info.height());
        SkSampler::Fill(fillInfo, dst, rowBytes, colorOrIndex, zeroInit);
    }

    /**
     *  If fSampleX > 1, the swizzler is sampling every fSampleX'th pixel and
     *  discarding the rest.
     *
     *  This getter is currently used by SkBmpStandardCodec for Bmp-in-Ico decodes.
     *  Ideally, the subclasses of SkCodec would have no knowledge of sampling, but
     *  this allows us to apply a transparency mask to pixels after swizzling.
     */
    int sampleX() const { return fSampleX; }

    /**
     *  Returns the actual number of pixels written to destination memory, taking
     *  scaling, subsetting, and partial frames into account.
     */
    int swizzleWidth() const { return fSwizzleWidth; }

    /**
     *  Returns the byte offset at which we write to destination memory, taking
     *  scaling, subsetting, and partial frames into account.
     */
    size_t swizzleOffsetBytes() const { return fDstOffsetBytes; }

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
    typedef void (*RowProc)(void* SK_RESTRICT dstRow,
                            const uint8_t* SK_RESTRICT src,
                            int dstWidth, int bpp, int deltaSrc, int offset,
                            const SkPMColor ctable[]);

    template <RowProc Proc>
    static void SkipLeading8888ZerosThen(void* SK_RESTRICT dstRow,
                                         const uint8_t* SK_RESTRICT src,
                                         int dstWidth, int bpp, int deltaSrc, int offset,
                                         const SkPMColor ctable[]);

    template <RowProc Proc>
    static void SkipLeadingGrayAlphaZerosThen(void* dst, const uint8_t* src, int width, int bpp,
                                              int deltaSrc, int offset, const SkPMColor ctable[]);

    // May be NULL.  We have not implemented optimized functions for all supported transforms.
    const RowProc       fFastProc;
    // Always non-NULL.  Supports sampling.
    const RowProc       fSlowProc;
    // The actual RowProc we are using.  This depends on if fFastProc is non-NULL and
    // whether or not we are sampling.
    RowProc             fActualProc;

    const SkPMColor*    fColorTable;      // Unowned pointer

    // Subset Swizzles
    // There are two types of subset swizzles that we support.  We do not
    // support both at the same time.
    // TODO: If we want to support partial scanlines for gifs (which may
    //       use frame subsets), we will need to support both subsetting
    //       modes at the same time.
    // (1) Partial Scanlines
    //         The client only wants to write a subset of the source pixels
    //         to the destination.  This subset is specified to CreateSwizzler
    //         using options->fSubset.  We will store subset information in
    //         the following fields.
    //
    //         fSrcOffset:      The starting pixel of the source.
    //         fSrcOffsetUnits: Derived from fSrcOffset with two key
    //                          differences:
    //                          (1) This takes the size of source pixels into
    //                          account by multiplying by fSrcBPP.  This may
    //                          be measured in bits or bytes depending on
    //                          which is natural for the SrcConfig.
    //                          (2) If we are sampling, this will be larger
    //                          than fSrcOffset * fSrcBPP, since sampling
    //                          implies that we will skip some pixels.
    //         fDstOffset:      Will be zero.  There is no destination offset
    //                          for this type of subset.
    //         fDstOffsetBytes: Will be zero.
    //         fSrcWidth:       The width of the desired subset of source
    //                          pixels, before any sampling is performed.
    //         fDstWidth:       Will be equal to fSrcWidth, since this is also
    //                          calculated before any sampling is performed.
    //                          For this type of subset, the destination width
    //                          matches the desired subset of the source.
    //         fSwizzleWidth:   The actual number of pixels that will be
    //                          written by the RowProc.  This is a scaled
    //                          version of fSrcWidth/fDstWidth.
    //         fAllocatedWidth: Will be equal to fSwizzleWidth.  For this type
    //                          of subset, the number of pixels written is the
    //                          same as the actual width of the destination.
    // (2) Frame Subset
    //         The client will decode the entire width of the source into a
    //         subset of destination memory.  This subset is specified to
    //         CreateSwizzler in the "frame" parameter.  We store subset
    //         information in the following fields.
    //
    //         fSrcOffset:      Will be zero.  The starting pixel of the source.
    //         fSrcOffsetUnits: Will only be non-zero if we are sampling,
    //                          since sampling implies that we will skip some
    //                          pixels.  Note that this is measured in bits
    //                          or bytes depending on which is natural for
    //                          SrcConfig.
    //         fDstOffset:      First pixel to write in destination.
    //         fDstOffsetBytes: fDstOffset * fDstBPP.
    //         fSrcWidth:       The entire width of the source pixels, before
    //                          any sampling is performed.
    //         fDstWidth:       The entire width of the destination memory,
    //                          before any sampling is performed.
    //         fSwizzleWidth:   The actual number of pixels that will be
    //                          written by the RowProc.  This is a scaled
    //                          version of fSrcWidth.
    //         fAllocatedWidth: The actual number of pixels in destination
    //                          memory.  This is a scaled version of
    //                          fDstWidth.
    //
    // If we are not subsetting, these fields are more straightforward.
    //         fSrcOffset = fDstOffet = fDstOffsetBytes = 0
    //         fSrcOffsetUnits may be non-zero (we will skip the first few pixels when sampling)
    //         fSrcWidth = fDstWidth = Full original width
    //         fSwizzleWidth = fAllcoatedWidth = Scaled width (if we are sampling)
    const int           fSrcOffset;
    const int           fDstOffset;
    int                 fSrcOffsetUnits;
    int                 fDstOffsetBytes;
    const int           fSrcWidth;
    const int           fDstWidth;
    int                 fSwizzleWidth;
    int                 fAllocatedWidth;

    int                 fSampleX;         // Step between X samples
    const int           fSrcBPP;          // Bits/bytes per pixel for the SrcConfig
                                          // if bitsPerPixel % 8 == 0
                                          //     fBPP is bytesPerPixel
                                          // else
                                          //     fBPP is bitsPerPixel
    const int           fDstBPP;          // Bytes per pixel for the destination color type

    SkSwizzler(RowProc fastProc, RowProc proc, const SkPMColor* ctable, int srcOffset,
            int srcWidth, int dstOffset, int dstWidth, int srcBPP, int dstBPP);

    int onSetSampleX(int) override;

};
#endif // SkSwizzler_DEFINED

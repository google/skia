/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBmpStandardCodec.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkScanlineDecoder.h"
#include "SkStream.h"

/*
 * Checks if the conversion between the input image and the requested output
 * image has been implemented
 */
static bool conversion_possible(const SkImageInfo& dst,
                                const SkImageInfo& src) {
    // Ensure that the profile type is unchanged
    if (dst.profileType() != src.profileType()) {
        return false;
    }

    // Ensure the alpha type is valid
    if (!valid_alpha(dst.alphaType(), src.alphaType())) {
        return false;
    }

    // Check for supported color types
    switch (dst.colorType()) {
        // Allow output to kN32 from any type of input
        case kN32_SkColorType:
            return true;
        // Allow output to kIndex_8 from compatible inputs
        case kIndex_8_SkColorType:
            return kIndex_8_SkColorType == src.colorType();
        default:
            return false;
    }
}

/*
 * Creates an instance of the decoder
 * Called only by NewFromStream
 */
SkBmpStandardCodec::SkBmpStandardCodec(const SkImageInfo& info, SkStream* stream,
                                       uint16_t bitsPerPixel, uint32_t numColors,
                                       uint32_t bytesPerColor, uint32_t offset,
                                       SkBmpCodec::RowOrder rowOrder, bool inIco)
    : INHERITED(info, stream, bitsPerPixel, rowOrder)
    , fColorTable(NULL)
    , fNumColors(this->computeNumColors(numColors))
    , fBytesPerColor(bytesPerColor)
    , fOffset(offset)
    , fSwizzler(NULL)
    , fSrcBuffer(NULL)
    , fInIco(inIco)
{}

/*
 * Initiates the bitmap decode
 */
SkCodec::Result SkBmpStandardCodec::onGetPixels(const SkImageInfo& dstInfo,
                                        void* dst, size_t dstRowBytes,
                                        const Options& opts,
                                        SkPMColor* inputColorPtr,
                                        int* inputColorCount) {
    if (!this->handleRewind(fInIco)) {
        return kCouldNotRewind;
    }
    if (opts.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }
    if (dstInfo.dimensions() != this->getInfo().dimensions()) {
        SkCodecPrintf("Error: scaling not supported.\n");
        return kInvalidScale;
    }
    if (!conversion_possible(dstInfo, this->getInfo())) {
        SkCodecPrintf("Error: cannot convert input type to output type.\n");
        return kInvalidConversion;
    }

    // Create the color table if necessary and prepare the stream for decode
    // Note that if it is non-NULL, inputColorCount will be modified
    if (!this->createColorTable(dstInfo.alphaType(), inputColorCount)) {
        SkCodecPrintf("Error: could not create color table.\n");
        return kInvalidInput;
    }

    // Copy the color table to the client if necessary
    copy_color_table(dstInfo, fColorTable, inputColorPtr, inputColorCount);

    // Initialize a swizzler if necessary
    if (!this->initializeSwizzler(dstInfo, opts)) {
        SkCodecPrintf("Error: cannot initialize swizzler.\n");
        return kInvalidConversion;
    }

    return this->decode(dstInfo, dst, dstRowBytes, opts);
}

/*
 * Process the color table for the bmp input
 */
 bool SkBmpStandardCodec::createColorTable(SkAlphaType alphaType, int* numColors) {
    // Allocate memory for color table
    uint32_t colorBytes = 0;
    SkPMColor colorTable[256];
    if (this->bitsPerPixel() <= 8) {
        // Inform the caller of the number of colors
        uint32_t maxColors = 1 << this->bitsPerPixel();
        if (NULL != numColors) {
            // We set the number of colors to maxColors in order to ensure
            // safe memory accesses.  Otherwise, an invalid pixel could
            // access memory outside of our color table array.
            *numColors = maxColors;
        }

        // Read the color table from the stream
        colorBytes = fNumColors * fBytesPerColor;
        SkAutoTDeleteArray<uint8_t> cBuffer(SkNEW_ARRAY(uint8_t, colorBytes));
        if (stream()->read(cBuffer.get(), colorBytes) != colorBytes) {
            SkCodecPrintf("Error: unable to read color table.\n");
            return false;
        }

        // Choose the proper packing function
        SkPMColor (*packARGB) (uint32_t, uint32_t, uint32_t, uint32_t);
        switch (alphaType) {
            case kOpaque_SkAlphaType:
            case kUnpremul_SkAlphaType:
                packARGB = &SkPackARGB32NoCheck;
                break;
            case kPremul_SkAlphaType:
                packARGB = &SkPreMultiplyARGB;
                break;
            default:
                // This should not be reached because conversion possible
                // should fail if the alpha type is not one of the above
                // values.
                SkASSERT(false);
                packARGB = NULL;
                break;
        }

        // Fill in the color table
        uint32_t i = 0;
        for (; i < fNumColors; i++) {
            uint8_t blue = get_byte(cBuffer.get(), i*fBytesPerColor);
            uint8_t green = get_byte(cBuffer.get(), i*fBytesPerColor + 1);
            uint8_t red = get_byte(cBuffer.get(), i*fBytesPerColor + 2);
            uint8_t alpha;
            if (kOpaque_SkAlphaType == alphaType) {
                alpha = 0xFF;
            } else {
                alpha = get_byte(cBuffer.get(), i*fBytesPerColor + 3);
            }
            colorTable[i] = packARGB(alpha, red, green, blue);
        }

        // To avoid segmentation faults on bad pixel data, fill the end of the
        // color table with black.  This is the same the behavior as the
        // chromium decoder.
        for (; i < maxColors; i++) {
            colorTable[i] = SkPackARGB32NoCheck(0xFF, 0, 0, 0);
        }

        // Set the color table
        fColorTable.reset(SkNEW_ARGS(SkColorTable, (colorTable, maxColors)));
    }

    // Bmp-in-Ico files do not use an offset to indicate where the pixel data
    // begins.  Pixel data always begins immediately after the color table.
    if (!fInIco) {
        // Check that we have not read past the pixel array offset
        if(fOffset < colorBytes) {
            // This may occur on OS 2.1 and other old versions where the color
            // table defaults to max size, and the bmp tries to use a smaller
            // color table.  This is invalid, and our decision is to indicate
            // an error, rather than try to guess the intended size of the
            // color table.
            SkCodecPrintf("Error: pixel data offset less than color table size.\n");
            return false;
        }

        // After reading the color table, skip to the start of the pixel array
        if (stream()->skip(fOffset - colorBytes) != fOffset - colorBytes) {
            SkCodecPrintf("Error: unable to skip to image data.\n");
            return false;
        }
    }

    // Return true on success
    return true;
}

bool SkBmpStandardCodec::initializeSwizzler(const SkImageInfo& dstInfo,
                                            const Options& opts) {
    // Allocate space for a row buffer
    const size_t rowBytes = SkAlign4(compute_row_bytes(dstInfo.width(), this->bitsPerPixel()));
    fSrcBuffer.reset(SkNEW_ARRAY(uint8_t, rowBytes));

    // Get swizzler configuration
    SkSwizzler::SrcConfig config;
    switch (this->bitsPerPixel()) {
        case 1:
            config = SkSwizzler::kIndex1;
            break;
        case 2:
            config = SkSwizzler::kIndex2;
            break;
        case 4:
            config = SkSwizzler::kIndex4;
            break;
        case 8:
            config = SkSwizzler::kIndex;
            break;
        case 24:
            config = SkSwizzler::kBGR;
            break;
        case 32:
            if (kOpaque_SkAlphaType == dstInfo.alphaType()) {
                config = SkSwizzler::kBGRX;
            } else {
                config = SkSwizzler::kBGRA;
            }
            break;
        default:
            SkASSERT(false);
            return false;
    }

    // Get a pointer to the color table if it exists
    const SkPMColor* colorPtr = get_color_ptr(fColorTable.get());

    // Create swizzler
    fSwizzler.reset(SkSwizzler::CreateSwizzler(config,
            colorPtr, dstInfo, opts.fZeroInitialized));

    if (NULL == fSwizzler.get()) {
        return false;
    }
    return true;
}

/*
 * Choose a fill for failures due to an incomplete image.  We will use zero as
 * the default palette index, black for opaque images, and transparent for
 * non-opaque images.
 */
static uint32_t get_fill_color_or_index(uint16_t bitsPerPixels, SkAlphaType alphaType) {
    uint32_t fillColorOrIndex;
    switch (bitsPerPixels) {
        case 1:
        case 2:
        case 4:
        case 8:
            fillColorOrIndex = 0;
            break;
        case 24:
            fillColorOrIndex = SK_ColorBLACK;
            break;
        case 32:
            if (kOpaque_SkAlphaType == alphaType) {
                fillColorOrIndex = SK_ColorBLACK;
            } else {
                fillColorOrIndex = SK_ColorTRANSPARENT;
            }
            break;
        default:
            SkASSERT(false);
            return 0;
    }
    return fillColorOrIndex;
}

/*
 * Performs the bitmap decoding for standard input format
 */
SkCodec::Result SkBmpStandardCodec::decode(const SkImageInfo& dstInfo,
                                   void* dst, size_t dstRowBytes,
                                   const Options& opts) {
    // Set constant values
    const int width = dstInfo.width();
    const int height = dstInfo.height();
    const size_t rowBytes = SkAlign4(compute_row_bytes(width, this->bitsPerPixel()));

    // Iterate over rows of the image
    for (int y = 0; y < height; y++) {
        // Read a row of the input
        if (this->stream()->read(fSrcBuffer.get(), rowBytes) != rowBytes) {
            SkCodecPrintf("Warning: incomplete input stream.\n");
            // Fill the destination image on failure
            // Get the fill color/index and check if it is 0
            uint32_t fillColorOrIndex = get_fill_color_or_index(this->bitsPerPixel(),
                    dstInfo.alphaType());
            bool zeroFill = (0 == fillColorOrIndex);

            if (kNo_ZeroInitialized == opts.fZeroInitialized || !zeroFill) {
                // Get a pointer to the color table if it exists
                const SkPMColor* colorPtr = get_color_ptr(fColorTable.get());

                void* dstStart = this->getDstStartRow(dst, dstRowBytes, y);
                SkSwizzler::Fill(dstStart, dstInfo, dstRowBytes, dstInfo.height() - y,
                        fillColorOrIndex, colorPtr);
            }
            return kIncompleteInput;
        }

        // Decode the row in destination format
        uint32_t row;
        if (SkBmpCodec::kTopDown_RowOrder == this->rowOrder()) {
            row = y;
        } else {
            row = height - 1 - y;
        }

        void* dstRow = SkTAddOffset<void>(dst, row * dstRowBytes);
        fSwizzler->swizzle(dstRow, fSrcBuffer.get());
    }

    // Finally, apply the AND mask for bmp-in-ico images
    if (fInIco) {
        // The AND mask is always 1 bit per pixel
        const size_t rowBytes = SkAlign4(compute_row_bytes(width, 1));

        SkPMColor* dstPtr = (SkPMColor*) dst;
        for (int y = 0; y < height; y++) {
            // The srcBuffer will at least be large enough
            if (stream()->read(fSrcBuffer.get(), rowBytes) != rowBytes) {
                SkCodecPrintf("Warning: incomplete AND mask for bmp-in-ico.\n");
                return kIncompleteInput;
            }

            int row;
            if (SkBmpCodec::kBottomUp_RowOrder == this->rowOrder()) {
                row = height - y - 1;
            } else {
                row = y;
            }

            SkPMColor* dstRow =
                    SkTAddOffset<SkPMColor>(dstPtr, row * dstRowBytes);

            for (int x = 0; x < width; x++) {
                int quotient;
                int modulus;
                SkTDivMod(x, 8, &quotient, &modulus);
                uint32_t shift = 7 - modulus;
                uint32_t alphaBit =
                        (fSrcBuffer.get()[quotient] >> shift) & 0x1;
                dstRow[x] &= alphaBit - 1;
            }
        }
    }

    // Finished decoding the entire image
    return kSuccess;
}

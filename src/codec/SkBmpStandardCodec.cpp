/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBmpStandardCodec.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkStream.h"

/*
 * Creates an instance of the decoder
 * Called only by NewFromStream
 */
SkBmpStandardCodec::SkBmpStandardCodec(const SkImageInfo& info, SkStream* stream,
                                       uint16_t bitsPerPixel, uint32_t numColors,
                                       uint32_t bytesPerColor, uint32_t offset,
                                       SkCodec::SkScanlineOrder rowOrder, bool inIco)
    : INHERITED(info, stream, bitsPerPixel, rowOrder)
    , fColorTable(nullptr)
    , fNumColors(this->computeNumColors(numColors))
    , fBytesPerColor(bytesPerColor)
    , fOffset(offset)
    , fSwizzler(nullptr)
    , fSrcRowBytes(SkAlign4(compute_row_bytes(this->getInfo().width(), this->bitsPerPixel())))
    , fSrcBuffer(new uint8_t [fSrcRowBytes])
    , fInIco(inIco)
{}

/*
 * Initiates the bitmap decode
 */
SkCodec::Result SkBmpStandardCodec::onGetPixels(const SkImageInfo& dstInfo,
                                        void* dst, size_t dstRowBytes,
                                        const Options& opts,
                                        SkPMColor* inputColorPtr,
                                        int* inputColorCount,
                                        int* rowsDecoded) {
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

    Result result = this->prepareToDecode(dstInfo, opts, inputColorPtr, inputColorCount);
    if (kSuccess != result) {
        return result;
    }
    int rows = this->decodeRows(dstInfo, dst, dstRowBytes, opts);
    if (rows != dstInfo.height()) {
        *rowsDecoded = rows;
        return kIncompleteInput;
    }
    if (fInIco) {
        return this->decodeIcoMask(dstInfo, dst, dstRowBytes);
    }
    return kSuccess;
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
        if (nullptr != numColors) {
            // We set the number of colors to maxColors in order to ensure
            // safe memory accesses.  Otherwise, an invalid pixel could
            // access memory outside of our color table array.
            *numColors = maxColors;
        }

        // Read the color table from the stream
        colorBytes = fNumColors * fBytesPerColor;
        SkAutoTDeleteArray<uint8_t> cBuffer(new uint8_t[colorBytes]);
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
                packARGB = nullptr;
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
        fColorTable.reset(new SkColorTable(colorTable, maxColors));
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

bool SkBmpStandardCodec::initializeSwizzler(const SkImageInfo& dstInfo, const Options& opts) {
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
    fSwizzler.reset(SkSwizzler::CreateSwizzler(config, colorPtr, dstInfo, opts));

    if (nullptr == fSwizzler.get()) {
        return false;
    }
    return true;
}

SkCodec::Result SkBmpStandardCodec::prepareToDecode(const SkImageInfo& dstInfo,
        const SkCodec::Options& options, SkPMColor inputColorPtr[], int* inputColorCount) {
    // Create the color table if necessary and prepare the stream for decode
    // Note that if it is non-NULL, inputColorCount will be modified
    if (!this->createColorTable(dstInfo.alphaType(), inputColorCount)) {
        SkCodecPrintf("Error: could not create color table.\n");
        return SkCodec::kInvalidInput;
    }

    // Copy the color table to the client if necessary
    copy_color_table(dstInfo, this->fColorTable, inputColorPtr, inputColorCount);

    // Initialize a swizzler if necessary
    if (!this->initializeSwizzler(dstInfo, options)) {
        SkCodecPrintf("Error: cannot initialize swizzler.\n");
        return SkCodec::kInvalidConversion;
    }
    return SkCodec::kSuccess;
}

/*
 * Performs the bitmap decoding for standard input format
 */
int SkBmpStandardCodec::decodeRows(const SkImageInfo& dstInfo,
                                               void* dst, size_t dstRowBytes,
                                               const Options& opts) {
    // Iterate over rows of the image
    const int height = dstInfo.height();
    for (int y = 0; y < height; y++) {
        // Read a row of the input
        if (this->stream()->read(fSrcBuffer.get(), fSrcRowBytes) != fSrcRowBytes) {
            SkCodecPrintf("Warning: incomplete input stream.\n");
            return y;
        }

        // Decode the row in destination format
        uint32_t row = this->getDstRow(y, dstInfo.height());

        void* dstRow = SkTAddOffset<void>(dst, row * dstRowBytes);
        fSwizzler->swizzle(dstRow, fSrcBuffer.get());
    }

    // Finished decoding the entire image
    return height;
}

// TODO (msarett): This function will need to be modified in order to perform row by row decodes
//                 when the Ico scanline decoder is implemented.
SkCodec::Result SkBmpStandardCodec::decodeIcoMask(const SkImageInfo& dstInfo,
        void* dst, size_t dstRowBytes) {
    // BMP in ICO have transparency, so this cannot be 565, and this mask
    // prevents us from using kIndex8. The below code depends on the output
    // being an SkPMColor.
    SkASSERT(dstInfo.colorType() == kN32_SkColorType);

    // The AND mask is always 1 bit per pixel
    const int width = this->getInfo().width();
    const size_t rowBytes = SkAlign4(compute_row_bytes(width, 1));

    SkPMColor* dstPtr = (SkPMColor*) dst;
    for (int y = 0; y < dstInfo.height(); y++) {
        // The srcBuffer will at least be large enough
        if (stream()->read(fSrcBuffer.get(), rowBytes) != rowBytes) {
            SkCodecPrintf("Warning: incomplete AND mask for bmp-in-ico.\n");
            return kIncompleteInput;
        }

        int row = this->getDstRow(y, dstInfo.height());

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
    return kSuccess;
}

uint32_t SkBmpStandardCodec::onGetFillValue(SkColorType colorType, SkAlphaType alphaType) const {
    const SkPMColor* colorPtr = get_color_ptr(fColorTable.get());
    if (colorPtr) {
        return get_color_table_fill_value(colorType, colorPtr, 0);
    }
    return INHERITED::onGetFillValue(colorType, alphaType);
}

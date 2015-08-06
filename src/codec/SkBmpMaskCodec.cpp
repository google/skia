/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBmpMaskCodec.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"

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
        // Allow output to kN32
        case kN32_SkColorType:
            return true;
        default:
            return false;
    }
}


/*
 * Creates an instance of the decoder
 */
SkBmpMaskCodec::SkBmpMaskCodec(const SkImageInfo& info, SkStream* stream,
                               uint16_t bitsPerPixel, SkMasks* masks,
                               SkBmpCodec::RowOrder rowOrder)
    : INHERITED(info, stream, bitsPerPixel, rowOrder)
    , fMasks(masks)
    , fMaskSwizzler(NULL)
    , fSrcBuffer(NULL)
{}

/*
 * Initiates the bitmap decode
 */
SkCodec::Result SkBmpMaskCodec::onGetPixels(const SkImageInfo& dstInfo,
                                            void* dst, size_t dstRowBytes,
                                            const Options& opts,
                                            SkPMColor* inputColorPtr,
                                            int* inputColorCount) {
    if (!this->handleRewind(false)) {
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

    // Initialize a the mask swizzler
    if (!this->initializeSwizzler(dstInfo)) {
        SkCodecPrintf("Error: cannot initialize swizzler.\n");
        return kInvalidConversion;
    }

    return this->decode(dstInfo, dst, dstRowBytes, opts);
}

bool SkBmpMaskCodec::initializeSwizzler(const SkImageInfo& dstInfo) {
    // Allocate space for a row buffer
    const size_t rowBytes = SkAlign4(compute_row_bytes(dstInfo.width(), this->bitsPerPixel()));
    fSrcBuffer.reset(SkNEW_ARRAY(uint8_t, rowBytes));

    // Create the swizzler
    fMaskSwizzler.reset(SkMaskSwizzler::CreateMaskSwizzler(
            dstInfo, fMasks, this->bitsPerPixel()));

    if (NULL == fMaskSwizzler.get()) {
        return false;
    }

    return true;
}

/*
 * Performs the decoding
 */
SkCodec::Result SkBmpMaskCodec::decode(const SkImageInfo& dstInfo,
                                       void* dst, size_t dstRowBytes,
                                       const Options& opts) {
    // Set constant values
    const int width = dstInfo.width();
    const int height = dstInfo.height();
    const size_t rowBytes = SkAlign4(compute_row_bytes(width, this->bitsPerPixel()));

    // Iterate over rows of the image
    uint8_t* srcRow = fSrcBuffer.get();
    for (int y = 0; y < height; y++) {
        // Read a row of the input
        if (this->stream()->read(srcRow, rowBytes) != rowBytes) {
            SkCodecPrintf("Warning: incomplete input stream.\n");
            // Fill the destination image on failure
            SkPMColor fillColor = dstInfo.alphaType() == kOpaque_SkAlphaType ?
                    SK_ColorBLACK : SK_ColorTRANSPARENT;
            if (kNo_ZeroInitialized == opts.fZeroInitialized || 0 != fillColor) {
                void* dstStart = this->getDstStartRow(dst, dstRowBytes, y);
                SkSwizzler::Fill(dstStart, dstInfo, dstRowBytes, dstInfo.height() - y, fillColor,
                        NULL);
            }
            return kIncompleteInput;
        }

        // Decode the row in destination format
        int row = SkBmpCodec::kBottomUp_RowOrder == this->rowOrder() ? height - 1 - y : y;
        void* dstRow = SkTAddOffset<void>(dst, row * dstRowBytes);
        fMaskSwizzler->swizzle(dstRow, srcRow);
    }

    // Finished decoding the entire image
    return kSuccess;
}

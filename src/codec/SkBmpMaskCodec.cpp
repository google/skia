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
 * Creates an instance of the decoder
 */
SkBmpMaskCodec::SkBmpMaskCodec(const SkImageInfo& info, SkStream* stream,
                               uint16_t bitsPerPixel, SkMasks* masks,
                               SkCodec::SkScanlineOrder rowOrder)
    : INHERITED(info, stream, bitsPerPixel, rowOrder)
    , fMasks(masks)
    , fMaskSwizzler(nullptr)
    , fSrcBuffer(new uint8_t [this->srcRowBytes()])
{}

/*
 * Initiates the bitmap decode
 */
SkCodec::Result SkBmpMaskCodec::onGetPixels(const SkImageInfo& dstInfo,
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
    return kSuccess;
}

SkCodec::Result SkBmpMaskCodec::prepareToDecode(const SkImageInfo& dstInfo,
        const SkCodec::Options& options, SkPMColor inputColorPtr[], int* inputColorCount) {
    // Initialize the mask swizzler
    fMaskSwizzler.reset(SkMaskSwizzler::CreateMaskSwizzler(dstInfo, this->getInfo(), fMasks,
            this->bitsPerPixel(), options));
    SkASSERT(fMaskSwizzler);

    return SkCodec::kSuccess;
}

/*
 * Performs the decoding
 */
int SkBmpMaskCodec::decodeRows(const SkImageInfo& dstInfo,
                                           void* dst, size_t dstRowBytes,
                                           const Options& opts) {
    // Iterate over rows of the image
    uint8_t* srcRow = fSrcBuffer.get();
    const int height = dstInfo.height();
    for (int y = 0; y < height; y++) {
        // Read a row of the input
        if (this->stream()->read(srcRow, this->srcRowBytes()) != this->srcRowBytes()) {
            SkCodecPrintf("Warning: incomplete input stream.\n");
            return y;
        }

        // Decode the row in destination format
        uint32_t row = this->getDstRow(y, height);
        void* dstRow = SkTAddOffset<void>(dst, row * dstRowBytes);
        fMaskSwizzler->swizzle(dstRow, srcRow);
    }

    // Finished decoding the entire image
    return height;
}

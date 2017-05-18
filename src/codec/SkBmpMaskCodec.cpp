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
SkBmpMaskCodec::SkBmpMaskCodec(int width, int height, const SkEncodedInfo& info, SkStream* stream,
                               uint16_t bitsPerPixel, SkMasks* masks,
                               SkCodec::SkScanlineOrder rowOrder)
    : INHERITED(width, height, info, stream, bitsPerPixel, rowOrder)
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

SkCodec::Result SkBmpMaskCodec::onPrepareToDecode(const SkImageInfo& dstInfo,
        const SkCodec::Options& options, SkPMColor inputColorPtr[], int* inputColorCount) {
    if (this->colorXform()) {
        this->resetXformBuffer(dstInfo.width());
    }

    SkImageInfo swizzlerInfo = dstInfo;
    if (this->colorXform()) {
        swizzlerInfo = swizzlerInfo.makeColorType(kXformSrcColorType);
        if (kPremul_SkAlphaType == dstInfo.alphaType()) {
            swizzlerInfo = swizzlerInfo.makeAlphaType(kUnpremul_SkAlphaType);
        }
    }

    // Initialize the mask swizzler
    fMaskSwizzler.reset(SkMaskSwizzler::CreateMaskSwizzler(swizzlerInfo, this->getInfo(),
            fMasks.get(), this->bitsPerPixel(), options));
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

        if (this->colorXform()) {
            SkImageInfo xformInfo = dstInfo.makeWH(fMaskSwizzler->swizzleWidth(), dstInfo.height());
            fMaskSwizzler->swizzle(this->xformBuffer(), srcRow);
            this->applyColorXform(xformInfo, dstRow, this->xformBuffer());
        } else {
            fMaskSwizzler->swizzle(dstRow, srcRow);
        }
    }

    // Finished decoding the entire image
    return height;
}

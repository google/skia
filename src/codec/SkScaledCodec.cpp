/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkSampledCodec.h"

// FIXME: Rename this file to SkSampledCodec.cpp

SkSampledCodec::SkSampledCodec(SkCodec* codec)
    : INHERITED(codec->getInfo())
    , fCodec(codec)
{}

SkISize SkSampledCodec::onGetSampledDimensions(int sampleSize) const {
    // Fast path for when we are not scaling.
    if (1 == sampleSize) {
        return fCodec->getInfo().dimensions();
    }

    const int width = fCodec->getInfo().width();
    const int height = fCodec->getInfo().height();

    // Check if the codec can provide the scaling natively.
    float scale = get_scale_from_sample_size(sampleSize);
    SkSize idealSize = SkSize::Make(scale * (float) width, scale * (float) height);
    SkISize nativeSize = fCodec->getScaledDimensions(scale);
    float widthDiff = SkTAbs(((float) nativeSize.width()) - idealSize.width());
    float heightDiff = SkTAbs(((float) nativeSize.height()) - idealSize.height());
    // Native scaling is preferred to sampling.  If we can scale natively to
    // within one of the ideal value, we should choose to scale natively.
    if (widthDiff < 1.0f && heightDiff < 1.0f) {
        return nativeSize;
    }

    // Provide the scaling by sampling.
    return SkISize::Make(get_scaled_dimension(width, sampleSize),
            get_scaled_dimension(height, sampleSize));
}

SkCodec::Result SkSampledCodec::onGetAndroidPixels(const SkImageInfo& info, void* pixels,
        size_t rowBytes, AndroidOptions& options) {
    // Create an Options struct for the codec.
    SkCodec::Options codecOptions;
    codecOptions.fZeroInitialized = options.fZeroInitialized;

    SkIRect* subset = options.fSubset;
    if (!subset || subset->size() == fCodec->getInfo().dimensions()) {
        if (fCodec->dimensionsSupported(info.dimensions())) {
            return fCodec->getPixels(info, pixels, rowBytes, &codecOptions, options.fColorPtr,
                    options.fColorCount);
        }

        // If the native codec does not support the requested scale, scale by sampling.
        return this->sampledDecode(info, pixels, rowBytes, options);
    }

    // We are performing a subset decode.
    int sampleSize = options.fSampleSize;
    SkISize scaledSize = this->onGetSampledDimensions(sampleSize);
    if (!fCodec->dimensionsSupported(scaledSize)) {
        // If the native codec does not support the requested scale, scale by sampling.
        return this->sampledDecode(info, pixels, rowBytes, options);
    }

    // Calculate the scaled subset bounds.
    int scaledSubsetX = subset->x() / sampleSize;
    int scaledSubsetY = subset->y() / sampleSize;
    int scaledSubsetWidth = info.width();
    int scaledSubsetHeight = info.height();

    // Start the scanline decode.
    SkIRect scanlineSubset = SkIRect::MakeXYWH(scaledSubsetX, 0, scaledSubsetWidth,
            scaledSize.height());
    codecOptions.fSubset = &scanlineSubset;
    SkCodec::Result result = fCodec->startScanlineDecode(info.makeWH(scaledSize.width(),
            scaledSize.height()), &codecOptions, options.fColorPtr, options.fColorCount);
    if (SkCodec::kSuccess != result) {
        return result;
    }

    // At this point, we are only concerned with subsetting.  Either no scale was
    // requested, or the fCodec is handling the scale.
    switch (fCodec->getScanlineOrder()) {
        case SkCodec::kTopDown_SkScanlineOrder:
        case SkCodec::kNone_SkScanlineOrder: {
            if (!fCodec->skipScanlines(scaledSubsetY)) {
                fCodec->fillIncompleteImage(info, pixels, rowBytes, options.fZeroInitialized,
                        scaledSubsetHeight, 0);
                return SkCodec::kIncompleteInput;
            }

            int decodedLines = fCodec->getScanlines(pixels, scaledSubsetHeight, rowBytes);
            if (decodedLines != scaledSubsetHeight) {
                return SkCodec::kIncompleteInput;
            }
            return SkCodec::kSuccess;
        }
        default:
            SkASSERT(false);
            return SkCodec::kUnimplemented;
    }
}


SkCodec::Result SkSampledCodec::sampledDecode(const SkImageInfo& info, void* pixels,
        size_t rowBytes, AndroidOptions& options) {
    // Create options struct for the codec.
    SkCodec::Options sampledOptions;
    sampledOptions.fZeroInitialized = options.fZeroInitialized;

    // Check if there is a subset.
    SkIRect subset;
    int subsetY = 0;
    int subsetWidth = fCodec->getInfo().width();
    int subsetHeight = fCodec->getInfo().height();
    if (options.fSubset) {
        // We will need to know about subsetting in the y-dimension in order to use the
        // scanline decoder.
        SkIRect* subsetPtr = options.fSubset;
        subsetY = subsetPtr->y();
        subsetWidth = subsetPtr->width();
        subsetHeight = subsetPtr->height();

        // The scanline decoder only needs to be aware of subsetting in the x-dimension.
        subset.setXYWH(subsetPtr->x(), 0, subsetWidth, fCodec->getInfo().height());
        sampledOptions.fSubset = &subset;
    }

    // Start the scanline decode.
    SkCodec::Result result = fCodec->startScanlineDecode(
            info.makeWH(fCodec->getInfo().width(), fCodec->getInfo().height()), &sampledOptions,
            options.fColorPtr, options.fColorCount);
    if (SkCodec::kSuccess != result) {
        return result;
    }

    SkSampler* sampler = fCodec->getSampler(true);
    if (!sampler) {
        return SkCodec::kUnimplemented;
    }

    // Since we guarantee that output dimensions are always at least one (even if the sampleSize
    // is greater than a given dimension), the input sampleSize is not always the sampleSize that
    // we use in practice.
    const int sampleX = subsetWidth / info.width();
    const int sampleY = subsetHeight / info.height();
    if (sampler->setSampleX(sampleX) != info.width()) {
        return SkCodec::kInvalidScale;
    }
    if (get_scaled_dimension(subsetHeight, sampleY) != info.height()) {
        return SkCodec::kInvalidScale;
    }

    const int samplingOffsetY = get_start_coord(sampleY);
    const int startY = samplingOffsetY + subsetY;
    int dstHeight = info.height();
    switch(fCodec->getScanlineOrder()) {
        case SkCodec::kTopDown_SkScanlineOrder: {
            if (!fCodec->skipScanlines(startY)) {
                fCodec->fillIncompleteImage(info, pixels, rowBytes, options.fZeroInitialized,
                        dstHeight, 0);
                return SkCodec::kIncompleteInput;
            }
            void* pixelPtr = pixels;
            for (int y = 0; y < dstHeight; y++) {
                if (1 != fCodec->getScanlines(pixelPtr, 1, rowBytes)) {
                    fCodec->fillIncompleteImage(info, pixels, rowBytes, options.fZeroInitialized,
                            dstHeight, y + 1);
                    return SkCodec::kIncompleteInput;
                }
                int linesToSkip = SkTMin(sampleY - 1, dstHeight - y - 1);
                if (!fCodec->skipScanlines(linesToSkip)) {
                    fCodec->fillIncompleteImage(info, pixels, rowBytes, options.fZeroInitialized,
                            dstHeight, y + 1);
                    return SkCodec::kIncompleteInput;
                }
                pixelPtr = SkTAddOffset<void>(pixelPtr, rowBytes);
            }
            return SkCodec::kSuccess;
        }
        case SkCodec::kNone_SkScanlineOrder: {
            const int linesNeeded = subsetHeight - samplingOffsetY;
            SkAutoMalloc storage(linesNeeded * rowBytes);
            uint8_t* storagePtr = static_cast<uint8_t*>(storage.get());

            if (!fCodec->skipScanlines(startY)) {
                fCodec->fillIncompleteImage(info, pixels, rowBytes, options.fZeroInitialized,
                        dstHeight, 0);
                return SkCodec::kIncompleteInput;
            }
            int scanlines = fCodec->getScanlines(storagePtr, linesNeeded, rowBytes);

            for (int y = 0; y < dstHeight; y++) {
                memcpy(pixels, storagePtr, info.minRowBytes());
                storagePtr += sampleY * rowBytes;
                pixels = SkTAddOffset<void>(pixels, rowBytes);
            }

            if (scanlines < dstHeight) {
                // fCodec has already handled filling uninitialized memory.
                return SkCodec::kIncompleteInput;
            }
            return SkCodec::kSuccess;
        }
        default:
            SkASSERT(false);
            return SkCodec::kUnimplemented;
    }
}

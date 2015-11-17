/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkMath.h"
#include "SkSampledCodec.h"

SkSampledCodec::SkSampledCodec(SkCodec* codec)
    : INHERITED(codec->getInfo())
    , fCodec(codec)
{}

SkISize SkSampledCodec::accountForNativeScaling(int* sampleSizePtr, int* nativeSampleSize) const {
    SkISize preSampledSize = fCodec->getInfo().dimensions();
    int sampleSize = *sampleSizePtr;
    SkASSERT(sampleSize > 1);

    if (nativeSampleSize) {
        *nativeSampleSize = 1;
    }

    // Only JPEG supports native downsampling.
    if (fCodec->getEncodedFormat() == kJPEG_SkEncodedFormat) {
        // See if libjpeg supports this scale directly
        switch (sampleSize) {
            case 2:
            case 4:
            case 8:
                // This class does not need to do any sampling.
                *sampleSizePtr = 1;
                return fCodec->getScaledDimensions(get_scale_from_sample_size(sampleSize));
            default:
                break;
        }

        // Check if sampleSize is a multiple of something libjpeg can support.
        int remainder;
        const int sampleSizes[] = { 8, 4, 2 };
        for (int supportedSampleSize : sampleSizes) {
            int actualSampleSize;
            SkTDivMod(sampleSize, supportedSampleSize, &actualSampleSize, &remainder);
            if (0 == remainder) {
                float scale = get_scale_from_sample_size(supportedSampleSize);

                // fCodec will scale to this size.
                preSampledSize = fCodec->getScaledDimensions(scale);

                // And then this class will sample it.
                *sampleSizePtr = actualSampleSize;
                if (nativeSampleSize) {
                    *nativeSampleSize = supportedSampleSize;
                }
                break;
            }
        }
    }

    return preSampledSize;
}

SkISize SkSampledCodec::onGetSampledDimensions(int sampleSize) const {
    const SkISize size = this->accountForNativeScaling(&sampleSize);
    return SkISize::Make(get_scaled_dimension(size.width(), sampleSize),
                         get_scaled_dimension(size.height(), sampleSize));
}

SkCodec::Result SkSampledCodec::onGetAndroidPixels(const SkImageInfo& info, void* pixels,
        size_t rowBytes, const AndroidOptions& options) {
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
    SkISize scaledSize = this->getSampledDimensions(sampleSize);
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
        size_t rowBytes, const AndroidOptions& options) {
    // We should only call this function when sampling.
    SkASSERT(options.fSampleSize > 1);

    // Create options struct for the codec.
    SkCodec::Options sampledOptions;
    sampledOptions.fZeroInitialized = options.fZeroInitialized;

    // FIXME: This was already called by onGetAndroidPixels. Can we reduce that?
    int sampleSize = options.fSampleSize;
    int nativeSampleSize;
    SkISize nativeSize = this->accountForNativeScaling(&sampleSize, &nativeSampleSize);

    // Check if there is a subset.
    SkIRect subset;
    int subsetY = 0;
    int subsetWidth = nativeSize.width();
    int subsetHeight = nativeSize.height();
    if (options.fSubset) {
        // We will need to know about subsetting in the y-dimension in order to use the
        // scanline decoder.
        // Update the subset to account for scaling done by fCodec.
        SkIRect* subsetPtr = options.fSubset;

        // Do the divide ourselves, instead of calling get_scaled_dimension. If
        // X and Y are 0, they should remain 0, rather than being upgraded to 1
        // due to being smaller than the sampleSize.
        const int subsetX = subsetPtr->x() / nativeSampleSize;
        subsetY = subsetPtr->y() / nativeSampleSize;

        subsetWidth = get_scaled_dimension(subsetPtr->width(), nativeSampleSize);
        subsetHeight = get_scaled_dimension(subsetPtr->height(), nativeSampleSize);

        // The scanline decoder only needs to be aware of subsetting in the x-dimension.
        subset.setXYWH(subsetX, 0, subsetWidth, nativeSize.height());
        sampledOptions.fSubset = &subset;
    }

    // Start the scanline decode.
    SkCodec::Result result = fCodec->startScanlineDecode(
            info.makeWH(nativeSize.width(), nativeSize.height()), &sampledOptions,
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
                if (y < dstHeight - 1) {
                    if (!fCodec->skipScanlines(sampleY - 1)) {
                        fCodec->fillIncompleteImage(info, pixels, rowBytes,
                                options.fZeroInitialized, dstHeight, y + 1);
                        return SkCodec::kIncompleteInput;
                    }
                }
                pixelPtr = SkTAddOffset<void>(pixelPtr, rowBytes);
            }
            return SkCodec::kSuccess;
        }
        case SkCodec::kOutOfOrder_SkScanlineOrder:
        case SkCodec::kBottomUp_SkScanlineOrder: {
            // Note that these modes do not support subsetting.
            SkASSERT(0 == subsetY && nativeSize.height() == subsetHeight);
            int y;
            for (y = 0; y < nativeSize.height(); y++) {
                int srcY = fCodec->nextScanline();
                if (is_coord_necessary(srcY, sampleY, dstHeight)) {
                    void* pixelPtr = SkTAddOffset<void>(pixels,
                            rowBytes * get_dst_coord(srcY, sampleY));
                    if (1 != fCodec->getScanlines(pixelPtr, 1, rowBytes)) {
                        break;
                    }
                } else {
                    if (!fCodec->skipScanlines(1)) {
                        break;
                    }
                }
            }

            if (nativeSize.height() == y) {
                return SkCodec::kSuccess;
            }

            // We handle filling uninitialized memory here instead of using fCodec.
            // fCodec does not know that we are sampling.
            const uint32_t fillValue = fCodec->getFillValue(info.colorType(), info.alphaType());
            const SkImageInfo fillInfo = info.makeWH(info.width(), 1);
            for (; y < nativeSize.height(); y++) {
                int srcY = fCodec->outputScanline(y);
                if (!is_coord_necessary(srcY, sampleY, dstHeight)) {
                    continue;
                }

                void* rowPtr = SkTAddOffset<void>(pixels, rowBytes * get_dst_coord(srcY, sampleY));
                SkSampler::Fill(fillInfo, rowPtr, rowBytes, fillValue, options.fZeroInitialized);
            }
            return SkCodec::kIncompleteInput;
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

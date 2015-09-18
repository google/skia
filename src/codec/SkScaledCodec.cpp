/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodecPriv.h"
#include "SkScaledCodec.h"
#include "SkStream.h"
#include "SkWebpCodec.h"


SkCodec* SkScaledCodec::NewFromStream(SkStream* stream) {
    bool isWebp = SkWebpCodec::IsWebp(stream);
    if (!stream->rewind()) {
        return nullptr;
    }
    if (isWebp) {
        // Webp codec supports scaling and subsetting natively
        return SkWebpCodec::NewFromStream(stream);  
    }

    SkAutoTDelete<SkScanlineDecoder> scanlineDecoder(SkScanlineDecoder::NewFromStream(stream));
    if (nullptr == scanlineDecoder) {
        return nullptr;
    }

    // wrap in new SkScaledCodec
    return new SkScaledCodec(scanlineDecoder.detach());
}

SkCodec* SkScaledCodec::NewFromData(SkData* data) {
    if (!data) {
        return nullptr;
    }
    return NewFromStream(new SkMemoryStream(data));
}

SkScaledCodec::SkScaledCodec(SkScanlineDecoder* scanlineDecoder)
    : INHERITED(scanlineDecoder->getInfo(), nullptr)
    , fScanlineDecoder(scanlineDecoder)
{}

SkScaledCodec::~SkScaledCodec() {}

static SkISize best_scaled_dimensions(const SkISize& origDims, const SkISize& nativeDims,
                                      const SkISize& scaledCodecDims, float desiredScale) {
    if (nativeDims == scaledCodecDims) {
        // does not matter which to return if equal. Return here to skip below calculations
        return nativeDims;
    }
    float idealWidth = origDims.width() * desiredScale;
    float idealHeight = origDims.height() * desiredScale;

    // calculate difference between native dimensions and ideal dimensions
    float nativeWDiff = SkTAbs(idealWidth - nativeDims.width());
    float nativeHDiff = SkTAbs(idealHeight - nativeDims.height());
    float nativeDiff = nativeWDiff + nativeHDiff;

    // Native scaling is preferred to sampling.  If we can scale natively to
    // within one of the ideal value, we should choose to scale natively.
    if (nativeWDiff < 1.0f && nativeHDiff < 1.0f) {
        return nativeDims;
    }

    // calculate difference between scaledCodec dimensions and ideal dimensions
    float scaledCodecWDiff = SkTAbs(idealWidth - scaledCodecDims.width());
    float scaledCodecHDiff = SkTAbs(idealHeight - scaledCodecDims.height());
    float scaledCodecDiff = scaledCodecWDiff + scaledCodecHDiff;

    // return dimensions closest to ideal dimensions.
    // If the differences are equal, return nativeDims, as native scaling is more efficient.
    return nativeDiff > scaledCodecDiff ? scaledCodecDims : nativeDims;
}

/*
 * Return a valid set of output dimensions for this decoder, given an input scale
 */
SkISize SkScaledCodec::onGetScaledDimensions(float desiredScale) const {
    SkISize nativeDimensions = fScanlineDecoder->getScaledDimensions(desiredScale);
    // support scaling down by integer numbers. Ex: 1/2, 1/3, 1/4 ...
    SkISize scaledCodecDimensions;
    if (desiredScale > 0.5f) {
        // sampleSize = 1
        scaledCodecDimensions = fScanlineDecoder->getInfo().dimensions();
    }
    // sampleSize determines the step size between samples
    // Ex: sampleSize = 2, sample every second pixel in x and y directions
    int sampleSize = int ((1.0f / desiredScale) + 0.5f);

    int scaledWidth = get_scaled_dimension(this->getInfo().width(), sampleSize);
    int scaledHeight = get_scaled_dimension(this->getInfo().height(), sampleSize);

    // Return the calculated output dimensions for the given scale
    scaledCodecDimensions = SkISize::Make(scaledWidth, scaledHeight);

    return best_scaled_dimensions(this->getInfo().dimensions(), nativeDimensions,
                                  scaledCodecDimensions, desiredScale);
}

// check if scaling to dstInfo size from srcInfo size using sampleSize is possible
static bool scaling_supported(const SkImageInfo& dstInfo, const SkImageInfo& srcInfo,
                              int* sampleX, int* sampleY) {
    SkScaledCodec::ComputeSampleSize(dstInfo, srcInfo, sampleX, sampleY);
    const int dstWidth = dstInfo.width();
    const int dstHeight = dstInfo.height();
    const int srcWidth = srcInfo.width();
    const int srcHeight = srcInfo.height();
     // only support down sampling, not up sampling
    if (dstWidth > srcWidth || dstHeight  > srcHeight) {
        return false;
    }
    // check that srcWidth is scaled down by an integer value
    if (get_scaled_dimension(srcWidth, *sampleX) != dstWidth) {
        return false;
    }
    // check that src height is scaled down by an integer value
    if (get_scaled_dimension(srcHeight, *sampleY) != dstHeight) {
        return false;
    }
    // sampleX and sampleY should be equal unless the original sampleSize requested was larger
    // than srcWidth or srcHeight. If so, the result of this is dstWidth or dstHeight = 1.
    // This functionality allows for tall thin images to still be scaled down by scaling factors.
    if (*sampleX != *sampleY){
        if (1 != dstWidth && 1 != dstHeight) {
            return false;
        }
    }
    return true;
}

// calculates sampleSize in x and y direction
void SkScaledCodec::ComputeSampleSize(const SkImageInfo& dstInfo, const SkImageInfo& srcInfo,
                                      int* sampleXPtr, int* sampleYPtr) {
    int srcWidth = srcInfo.width();
    int dstWidth = dstInfo.width();
    int srcHeight = srcInfo.height();
    int dstHeight = dstInfo.height();

    int sampleX = srcWidth / dstWidth;
    int sampleY = srcHeight / dstHeight;

    // only support down sampling, not up sampling
    SkASSERT(dstWidth <= srcWidth);
    SkASSERT(dstHeight <= srcHeight);

    // sampleX and sampleY should be equal unless the original sampleSize requested was
    // larger than srcWidth or srcHeight.
    // If so, the result of this is dstWidth or dstHeight = 1. This functionality
    // allows for tall thin images to still be scaled down by scaling factors.

    if (sampleX != sampleY){
        if (1 != dstWidth && 1 != dstHeight) {

            // rounding during onGetScaledDimensions can cause different sampleSizes
            // Ex: srcWidth = 79, srcHeight = 20, sampleSize = 10
            // dstWidth = 7, dstHeight = 2, sampleX = 79/7 = 11, sampleY = 20/2 = 10
            // correct for this rounding by comparing width to sampleY and height to sampleX

            if (get_scaled_dimension(srcWidth, sampleY) == dstWidth) {
                sampleX = sampleY;
            } else if (get_scaled_dimension(srcHeight, sampleX) == dstHeight) {
                sampleY = sampleX;
            }
        }
    }

    if (sampleXPtr) {
        *sampleXPtr = sampleX;
    }
    if (sampleYPtr) {
        *sampleYPtr = sampleY;
    }
}

// TODO: Implement subsetting in onGetPixels which works when and when not sampling 

SkCodec::Result SkScaledCodec::onGetPixels(const SkImageInfo& requestedInfo, void* dst,
                                           size_t rowBytes, const Options& options,
                                           SkPMColor ctable[], int* ctableCount) {

    if (options.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    } 

    Result result = fScanlineDecoder->start(requestedInfo, &options, ctable, ctableCount);
    if (kSuccess == result) {
        // native decode supported
        switch (fScanlineDecoder->getScanlineOrder()) {
            case SkScanlineDecoder::kTopDown_SkScanlineOrder:
            case SkScanlineDecoder::kBottomUp_SkScanlineOrder:
            case SkScanlineDecoder::kNone_SkScanlineOrder:
                return fScanlineDecoder->getScanlines(dst, requestedInfo.height(), rowBytes);
            case SkScanlineDecoder::kOutOfOrder_SkScanlineOrder: {
                for (int y = 0; y < requestedInfo.height(); y++) {
                    int dstY = fScanlineDecoder->getY();
                    void* dstPtr = SkTAddOffset<void>(dst, rowBytes * dstY);
                    result = fScanlineDecoder->getScanlines(dstPtr, 1, rowBytes);
                    // FIXME (msarett): Make the SkCodec base class take care of filling
                    // uninitialized pixels so we can return immediately on kIncompleteInput.
                    if (kSuccess != result && kIncompleteInput != result) {
                        return result;
                    }
                }
                return result;
            }
        }
    }

    if (kInvalidScale != result) {
        // no scaling requested
        return result;
    }
    
    // scaling requested
    int sampleX;
    int sampleY;
    if (!scaling_supported(requestedInfo, fScanlineDecoder->getInfo(), &sampleX, &sampleY)) {
        return kInvalidScale;
    }
    // set first sample pixel in y direction
    int Y0 = get_start_coord(sampleY);

    int dstHeight = requestedInfo.height();
    int srcHeight = fScanlineDecoder->getInfo().height();
    
    SkImageInfo info = requestedInfo;
    // use original height as scanlineDecoder does not support y sampling natively
    info = info.makeWH(requestedInfo.width(), srcHeight);

    // update scanlineDecoder with new info
    result = fScanlineDecoder->start(info, &options, ctable, ctableCount);
    if (kSuccess != result) {
        return result;
    }

    switch(fScanlineDecoder->getScanlineOrder()) {
        case SkScanlineDecoder::kTopDown_SkScanlineOrder: {
            result = fScanlineDecoder->skipScanlines(Y0);
            if (kSuccess != result && kIncompleteInput != result) {
                return result;
            }
            for (int y = 0; y < dstHeight; y++) {
                result = fScanlineDecoder->getScanlines(dst, 1, rowBytes);
                if (kSuccess != result && kIncompleteInput != result) {
                    return result;
                }
                if (y < dstHeight - 1) {
                    result = fScanlineDecoder->skipScanlines(sampleY - 1);
                    if (kSuccess != result && kIncompleteInput != result) {
                        return result;
                    }
                }
                dst = SkTAddOffset<void>(dst, rowBytes);
            }
            return result;
        }
        case SkScanlineDecoder::kBottomUp_SkScanlineOrder:
        case SkScanlineDecoder::kOutOfOrder_SkScanlineOrder: {
            for (int y = 0; y < srcHeight; y++) {
                int srcY = fScanlineDecoder->getY();
                if (is_coord_necessary(srcY, sampleY, dstHeight)) {
                    void* dstPtr = SkTAddOffset<void>(dst, rowBytes * get_dst_coord(srcY, sampleY));
                    result = fScanlineDecoder->getScanlines(dstPtr, 1, rowBytes);
                    if (kSuccess != result && kIncompleteInput != result) {
                        return result;
                    }
                } else {
                    result = fScanlineDecoder->skipScanlines(1);
                    if (kSuccess != result && kIncompleteInput != result) {
                        return result;
                    }
                }
            }
            return result;
        }
        case SkScanlineDecoder::kNone_SkScanlineOrder: {
            SkAutoMalloc storage(srcHeight * rowBytes);
            uint8_t* storagePtr = static_cast<uint8_t*>(storage.get());
            result = fScanlineDecoder->getScanlines(storagePtr, srcHeight, rowBytes);
            if (kSuccess != result && kIncompleteInput != result) {
                return result;
            }
            storagePtr += Y0 * rowBytes;
            for (int y = 0; y < dstHeight; y++) {
                memcpy(dst, storagePtr, rowBytes);
                storagePtr += sampleY * rowBytes;
                dst = SkTAddOffset<void>(dst, rowBytes);
            }
            return result;
        }
        default:
            SkASSERT(false);
            return kUnimplemented;
    }
}

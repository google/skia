/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodecImageGenerator.h"

SkImageGenerator* SkCodecImageGenerator::NewFromEncodedCodec(SkData* data) {
    SkCodec* codec = SkCodec::NewFromData(data);
    if (nullptr == codec) {
        return nullptr;
    }

    return new SkCodecImageGenerator(codec, data);
}

static SkImageInfo make_premul(const SkImageInfo& info) {
    if (kUnpremul_SkAlphaType == info.alphaType()) {
        return info.makeAlphaType(kPremul_SkAlphaType);
    }

    return info;
}

SkCodecImageGenerator::SkCodecImageGenerator(SkCodec* codec, SkData* data)
    : INHERITED(make_premul(codec->getInfo()))
    , fCodec(codec)
    , fData(SkRef(data))
    , fYWidth(0)
    , fUWidth(0)
    , fVWidth(0)
{}

SkData* SkCodecImageGenerator::onRefEncodedData(SK_REFENCODEDDATA_CTXPARAM) {
    return SkRef(fData.get());
}

bool SkCodecImageGenerator::onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
        SkPMColor ctable[], int* ctableCount) {

    SkCodec::Result result = fCodec->getPixels(info, pixels, rowBytes, nullptr, ctable,
            ctableCount);
    switch (result) {
        case SkCodec::kSuccess:
        case SkCodec::kIncompleteInput:
            return true;
        default:
            return false;
    }
}

bool SkCodecImageGenerator::onGetYUV8Planes(SkISize sizes[3], void* planes[3], size_t rowBytes[3],
        SkYUVColorSpace* colorSpace) {
    // TODO (msarett): Change the YUV API in ImageGenerator to match SkCodec.
    //                 This function is currently a hack to match the implementation
    //                 in SkCodec with the old API.
    SkCodec::YUVSizeInfo sizeInfo;

    // If planes is NULL, we just need to return the size.
    if (nullptr == planes) {
        bool result = fCodec->queryYUV8(&sizeInfo, colorSpace);
        if (result) {
            // Save the true widths
            fYWidth = sizeInfo.fYSize.width();
            fUWidth = sizeInfo.fUSize.width();
            fVWidth = sizeInfo.fVSize.width();

            // Set the sizes so that the client allocates enough memory
            sizes[0].fWidth = (int) sizeInfo.fYWidthBytes;
            sizes[0].fHeight = sizeInfo.fYSize.height();
            sizes[1].fWidth = (int) sizeInfo.fUWidthBytes;
            sizes[1].fHeight = sizeInfo.fUSize.height();
            sizes[2].fWidth = (int) sizeInfo.fVWidthBytes;
            sizes[2].fHeight = sizeInfo.fVSize.height();
        }
        return result;
    }

    // Set the sizeInfo with the true widths and heights
    SkASSERT(fYWidth != 0 && fUWidth != 0 && fVWidth != 0);
    sizeInfo.fYSize.set(fYWidth, sizes[0].height());
    sizeInfo.fUSize.set(fUWidth, sizes[1].height());
    sizeInfo.fVSize.set(fVWidth, sizes[2].height());

    // Set the sizeInfo with the allocated widths
    sizeInfo.fYWidthBytes = sizes[0].width();
    sizeInfo.fUWidthBytes = sizes[1].width();
    sizeInfo.fVWidthBytes = sizes[2].width();
    SkCodec::Result result = fCodec->getYUV8Planes(sizeInfo, planes);
    if ((result == SkCodec::kSuccess || result == SkCodec::kIncompleteInput) && colorSpace) {
        *colorSpace = kJPEG_SkYUVColorSpace;
    }

    switch (result) {
        case SkCodec::kSuccess:
        case SkCodec::kIncompleteInput:
            return true;
        default:
            return false;
    }
}

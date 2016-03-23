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

// FIXME: We should expose information about the encoded format on the
//        SkImageGenerator, so the client can interpret the encoded
//        format and request an output format.  For now, as a workaround,
//        we guess what output format the client wants.
static SkImageInfo fix_info(const SkCodec& codec) {
    const SkImageInfo& info = codec.getInfo();
    SkAlphaType alphaType = (kUnpremul_SkAlphaType == info.alphaType()) ? kPremul_SkAlphaType :
            info.alphaType();

    // Crudely guess that the presence of a color space means sRGB.
    SkColorProfileType profileType = (codec.getColorSpace()) ? kSRGB_SkColorProfileType :
            kLinear_SkColorProfileType;

    return SkImageInfo::Make(info.width(), info.height(), info.colorType(), alphaType, profileType);
}

SkCodecImageGenerator::SkCodecImageGenerator(SkCodec* codec, SkData* data)
    : INHERITED(fix_info(*codec))
    , fCodec(codec)
    , fData(SkRef(data))
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

bool SkCodecImageGenerator::onQueryYUV8(SkYUVSizeInfo* sizeInfo, SkYUVColorSpace* colorSpace) const
{
    return fCodec->queryYUV8(sizeInfo, colorSpace);
}

bool SkCodecImageGenerator::onGetYUV8Planes(const SkYUVSizeInfo& sizeInfo, void* planes[3]) {
    SkCodec::Result result = fCodec->getYUV8Planes(sizeInfo, planes);

    switch (result) {
        case SkCodec::kSuccess:
        case SkCodec::kIncompleteInput:
            return true;
        default:
            return false;
    }
}

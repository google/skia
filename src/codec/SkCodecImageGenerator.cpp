/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoPixmapStorage.h"
#include "SkCodecImageGenerator.h"
#include "SkMakeUnique.h"
#include "SkPixmapPriv.h"

#define kMirrorX    SkPixmapPriv::kMirrorX
#define kMirrorY    SkPixmapPriv::kMirrorY
#define kSwapXY     SkPixmapPriv::kSwapXY

const uint8_t gOrientationFlags[] = {
    0,                              // kTopLeft_Origin
    kMirrorX,                       // kTopRight_Origin
    kMirrorX | kMirrorY,            // kBottomRight_Origin
               kMirrorY,            // kBottomLeft_Origin
                          kSwapXY,  // kLeftTop_Origin
    kMirrorX            | kSwapXY,  // kRightTop_Origin
    kMirrorX | kMirrorY | kSwapXY,  // kRightBottom_Origin
               kMirrorY | kSwapXY,  // kLeftBottom_Origin
};

SkPixmapPriv::OrientFlags SkPixmapPriv::OriginToOrient(SkCodec::Origin o) {
    unsigned io = static_cast<int>(o) - 1;
    SkASSERT(io < SK_ARRAY_COUNT(gOrientationFlags));
    return static_cast<SkPixmapPriv::OrientFlags>(gOrientationFlags[io]);
}

static bool should_swap_width_height(SkCodec::Origin o) {
    return SkToBool(SkPixmapPriv::OriginToOrient(o) & kSwapXY);
}

static SkImageInfo swap_width_height(SkImageInfo info) {
    return info.makeWH(info.height(), info.width());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<SkImageGenerator> SkCodecImageGenerator::MakeFromEncodedCodec(sk_sp<SkData> data) {
    auto codec = SkCodec::MakeFromData(data);
    if (nullptr == codec) {
        return nullptr;
    }

    return std::unique_ptr<SkImageGenerator>(new SkCodecImageGenerator(std::move(codec), data));
}

static SkImageInfo adjust_info(SkCodec* codec) {
    SkImageInfo info = codec->getInfo();
    if (kUnpremul_SkAlphaType == info.alphaType()) {
        info = info.makeAlphaType(kPremul_SkAlphaType);
    }
    if (should_swap_width_height(codec->getOrigin())) {
        info = swap_width_height(info);
    }
    return info;
}

SkCodecImageGenerator::SkCodecImageGenerator(std::unique_ptr<SkCodec> codec, sk_sp<SkData> data)
    : INHERITED(adjust_info(codec.get()))
    , fCodec(std::move(codec))
    , fData(std::move(data))
{}

SkData* SkCodecImageGenerator::onRefEncodedData() {
    return SkRef(fData.get());
}

bool SkCodecImageGenerator::onGetPixels(const SkImageInfo& requestInfo, void* requestPixels,
                                        size_t requestRowBytes, const Options& opts) {
    const SkCodec::Origin origin = fCodec->getOrigin();
    const SkPixmap request(requestInfo, requestPixels, requestRowBytes);
    const SkPixmap* codecMap = &request;
    SkAutoPixmapStorage storage;    // used if we have to post-orient the output from the codec

    if (origin != SkCodec::kTopLeft_Origin) {
        SkImageInfo info = requestInfo;
        if (should_swap_width_height(origin)) {
            info = swap_width_height(info);
        }
        // need a tmp buffer to receive the pixels, so we can post-orient them
        if (!storage.tryAlloc(info)) {
            return false;
        }
        codecMap = &storage;
    }

    SkCodec::Options codecOpts;
    codecOpts.fPremulBehavior = opts.fBehavior;
    SkCodec::Result result = fCodec->getPixels(*codecMap, &codecOpts);
    switch (result) {
        case SkCodec::kSuccess:
            if (codecMap != &request) {
                return SkPixmapPriv::Orient(request, *codecMap,
                                            SkPixmapPriv::OriginToOrient(origin));
            }
            // fall through
        case SkCodec::kIncompleteInput:
        case SkCodec::kErrorInInput:
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
        case SkCodec::kErrorInInput:
            return true;
        default:
            return false;
    }
}

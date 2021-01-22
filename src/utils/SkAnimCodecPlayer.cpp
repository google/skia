/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/utils/SkAnimCodecPlayer.h"
#include "src/codec/SkCodecImageGenerator.h"
#include "src/core/SkPixmapPriv.h"
#include <algorithm>

SkAnimCodecPlayer::SkAnimCodecPlayer(std::unique_ptr<SkCodec> codec) : fCodec(std::move(codec)) {
    fImageInfo = fCodec->getInfo();
    fFrameInfos = fCodec->getFrameInfo();
    fImages.resize(fFrameInfos.size());

    // change the interpretation of fDuration to a end-time for that frame
    size_t dur = 0;
    for (auto& f : fFrameInfos) {
        dur += f.fDuration;
        f.fDuration = dur;
    }
    fTotalDuration = dur;

    if (!fTotalDuration) {
        // Static image -- may or may not have returned a single frame info.
        fFrameInfos.clear();
        fImages.clear();
        fImages.push_back(SkImage::MakeFromGenerator(
                              SkCodecImageGenerator::MakeFromCodec(std::move(fCodec))));
    }
}

SkAnimCodecPlayer::~SkAnimCodecPlayer() {}

SkISize SkAnimCodecPlayer::dimensions() const {
    if (!fCodec) {
        auto image = fImages.front();
        return image ? image->dimensions() : SkISize::MakeEmpty();
    }
    if (SkEncodedOriginSwapsWidthHeight(fCodec->getOrigin())) {
        return { fImageInfo.height(), fImageInfo.width() };
    }
    return { fImageInfo.width(), fImageInfo.height() };
}

sk_sp<SkImage> SkAnimCodecPlayer::getFrameAt(int index) {
    SkASSERT((unsigned)index < fFrameInfos.size());

    if (fImages[index]) {
        return fImages[index];
    }

    size_t rb = fImageInfo.minRowBytes();
    size_t size = fImageInfo.computeByteSize(rb);
    auto data = SkData::MakeUninitialized(size);

    SkCodec::Options opts;
    opts.fFrameIndex = index;

    const auto origin = fCodec->getOrigin();
    const auto orientedDims = this->dimensions();
    const auto originMatrix = SkEncodedOriginToMatrix(origin, orientedDims.width(),
                                                              orientedDims.height());

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);

    auto imageInfo = fImageInfo;
    if (fFrameInfos[index].fAlphaType != kOpaque_SkAlphaType && imageInfo.isOpaque()) {
        imageInfo = imageInfo.makeAlphaType(kPremul_SkAlphaType);
    }
    const int requiredFrame = fFrameInfos[index].fRequiredFrame;
    if (requiredFrame != SkCodec::kNoFrame && fImages[requiredFrame]) {
        auto requiredImage = fImages[requiredFrame];
        auto canvas = SkCanvas::MakeRasterDirect(imageInfo, data->writable_data(), rb);
        if (origin != kDefault_SkEncodedOrigin) {
            // The required frame is stored after applying the origin. Undo that,
            // because the codec decodes prior to applying the origin.
            // FIXME: Another approach would be to decode the frame's delta on top
            // of transparent black, and then draw that through the origin matrix
            // onto the required frame. To do that, SkCodec needs to expose the
            // rectangle of the delta and the blend mode, so we can handle
            // kRestoreBGColor frames and Blend::kSrc.
            SkMatrix inverse;
            SkAssertResult(originMatrix.invert(&inverse));
            canvas->concat(inverse);
        }
        canvas->drawImage(requiredImage, 0, 0, SkSamplingOptions(), &paint);
        opts.fPriorFrame = requiredFrame;
    }

    if (SkCodec::kSuccess != fCodec->getPixels(imageInfo, data->writable_data(), rb, &opts)) {
        return nullptr;
    }

    auto image = SkImage::MakeRasterData(imageInfo, std::move(data), rb);
    if (origin != kDefault_SkEncodedOrigin) {
        imageInfo = imageInfo.makeDimensions(orientedDims);
        rb = imageInfo.minRowBytes();
        size = imageInfo.computeByteSize(rb);
        data = SkData::MakeUninitialized(size);
        auto canvas = SkCanvas::MakeRasterDirect(imageInfo, data->writable_data(), rb);
        canvas->concat(originMatrix);
        canvas->drawImage(image, 0, 0, SkSamplingOptions(), &paint);
        image = SkImage::MakeRasterData(imageInfo, std::move(data), rb);
    }
    return fImages[index] = image;
}

sk_sp<SkImage> SkAnimCodecPlayer::getFrame() {
    SkASSERT(fTotalDuration > 0 || fImages.size() == 1);

    return fTotalDuration > 0
        ? this->getFrameAt(fCurrIndex)
        : fImages.front();
}

bool SkAnimCodecPlayer::seek(uint32_t msec) {
    if (!fTotalDuration) {
        return false;
    }

    msec %= fTotalDuration;

    auto lower = std::lower_bound(fFrameInfos.begin(), fFrameInfos.end(), msec,
                                  [](const SkCodec::FrameInfo& info, uint32_t msec) {
                                      return (uint32_t)info.fDuration <= msec;
                                  });
    int prevIndex = fCurrIndex;
    fCurrIndex = lower - fFrameInfos.begin();
    return fCurrIndex != prevIndex;
}



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
#include "include/core/SkPictureRecorder.h"
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
    if (SkPixmapPriv::ShouldSwapWidthHeight(fCodec->getOrigin())) {
        return { fImageInfo.height(), fImageInfo.width() };
    }
    return { fImageInfo.width(), fImageInfo.height() };
}

static sk_sp<SkImage> orient(const sk_sp<SkImage>& image, SkEncodedOrigin origin) {
    SkASSERT(image);
    auto dimensions = image->dimensions();
    if (SkPixmapPriv::ShouldSwapWidthHeight(origin)) {
        dimensions = { dimensions.height(), dimensions.width() };
    }

    SkPictureRecorder recorder;
    auto* canvas = recorder.beginRecording(SkRect::Make(dimensions), nullptr);
    canvas->concat(SkEncodedOriginToMatrix(origin, dimensions.width(), dimensions.height()));
    canvas->drawImage(image.get(), 0, 0, nullptr);

    const auto colorType = image->imageInfo().colorType();
    const auto bitDepth = colorType == kRGBA_F16_SkColorType ? SkImage::BitDepth::kF16
                                                             : SkImage::BitDepth::kU8;

    return SkImage::MakeFromPicture(recorder.finishRecordingAsPicture(), dimensions,
                                    nullptr, nullptr, bitDepth, image->refColorSpace());
}

sk_sp<SkImage> SkAnimCodecPlayer::getFrameAt(int index) {
    SkASSERT((unsigned)index < fFrameInfos.size());

    if (!fImages[index]) {
        size_t rb = fImageInfo.minRowBytes();
        size_t size = fImageInfo.computeByteSize(rb);
        auto data = SkData::MakeUninitialized(size);

        SkCodec::Options opts;
        opts.fFrameIndex = index;

        const int requiredFrame = fFrameInfos[index].fRequiredFrame;
        if (requiredFrame != SkCodec::kNoFrame) {
            auto requiredImage = fImages[requiredFrame];
            SkPixmap requiredPM;
            if (requiredImage && requiredImage->peekPixels(&requiredPM)) {
                sk_careful_memcpy(data->writable_data(), requiredPM.addr(), size);
                opts.fPriorFrame = requiredFrame;
            }
        }
        if (SkCodec::kSuccess == fCodec->getPixels(fImageInfo, data->writable_data(), rb, &opts)) {
            fImages[index] = SkImage::MakeRasterData(fImageInfo, std::move(data), rb);
        }
    }
    if (fImages[index]) {
        const auto origin = fCodec->getOrigin();
        if (origin != kDefault_SkEncodedOrigin) {
            return orient(fImages[index], origin);
        }
        return fImages[index];
    }
    return nullptr;
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
                                      return (uint32_t)info.fDuration < msec;
                                  });
    int prevIndex = fCurrIndex;
    fCurrIndex = lower - fFrameInfos.begin();
    return fCurrIndex != prevIndex;
}



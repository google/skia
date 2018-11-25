/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAndroidCodec.h"
#include "SkAnimatedImage.h"
#include "SkCanvas.h"
#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"

sk_sp<SkAnimatedImage> SkAnimatedImage::Make(std::unique_ptr<SkAndroidCodec> codec,
        SkISize scaledSize, SkIRect cropRect, sk_sp<SkPicture> postProcess) {
    if (!codec) {
        return nullptr;
    }

    SkISize decodeSize = scaledSize;
    auto decodeInfo = codec->getInfo();
    if (codec->getEncodedFormat() == SkEncodedImageFormat::kWEBP
            && scaledSize.width()  < decodeInfo.width()
            && scaledSize.height() < decodeInfo.height()) {
        // libwebp can decode to arbitrary smaller sizes.
        decodeInfo = decodeInfo.makeWH(decodeSize.width(), decodeSize.height());
    }

    auto image = sk_sp<SkAnimatedImage>(new SkAnimatedImage(std::move(codec), scaledSize,
                decodeInfo, cropRect, std::move(postProcess)));
    if (!image->fActiveFrame.fBitmap.getPixels()) {
        // tryAllocPixels failed.
        return nullptr;
    }

    return image;
}

sk_sp<SkAnimatedImage> SkAnimatedImage::Make(std::unique_ptr<SkAndroidCodec> codec) {
    if (!codec) {
        return nullptr;
    }

    const auto decodeInfo = codec->getInfo();
    const auto scaledSize = decodeInfo.dimensions();
    const auto cropRect   = SkIRect::MakeSize(scaledSize);
    auto image = sk_sp<SkAnimatedImage>(new SkAnimatedImage(std::move(codec), scaledSize,
                decodeInfo, cropRect, nullptr));

    if (!image->fActiveFrame.fBitmap.getPixels()) {
        // tryAllocPixels failed.
        return nullptr;
    }

    SkASSERT(image->fSimple);
    return image;
}

SkAnimatedImage::SkAnimatedImage(std::unique_ptr<SkAndroidCodec> codec, SkISize scaledSize,
        SkImageInfo decodeInfo, SkIRect cropRect, sk_sp<SkPicture> postProcess)
    : fCodec(std::move(codec))
    , fScaledSize(scaledSize)
    , fDecodeInfo(decodeInfo)
    , fCropRect(cropRect)
    , fPostProcess(std::move(postProcess))
    , fFrameCount(fCodec->codec()->getFrameCount())
    , fSimple(fScaledSize == fDecodeInfo.dimensions() && !fPostProcess
              && fCropRect == fDecodeInfo.bounds())
    , fFinished(false)
    , fRepetitionCount(fCodec->codec()->getRepetitionCount())
    , fRepetitionsCompleted(0)
{
    if (!fActiveFrame.fBitmap.tryAllocPixels(fDecodeInfo)) {
        return;
    }

    if (!fSimple) {
        fMatrix = SkMatrix::MakeTrans(-fCropRect.fLeft, -fCropRect.fTop);
        float scaleX = (float) fScaledSize.width()  / fDecodeInfo.width();
        float scaleY = (float) fScaledSize.height() / fDecodeInfo.height();
        fMatrix.preConcat(SkMatrix::MakeScale(scaleX, scaleY));
    }
    this->decodeNextFrame();
}

SkAnimatedImage::~SkAnimatedImage() { }

SkRect SkAnimatedImage::onGetBounds() {
    return SkRect::MakeIWH(fCropRect.width(), fCropRect.height());
}

SkAnimatedImage::Frame::Frame()
    : fIndex(SkCodec::kNone)
{}

bool SkAnimatedImage::Frame::copyTo(Frame* dst) const {
    if (dst->fBitmap.getPixels()) {
        dst->fBitmap.setAlphaType(fBitmap.alphaType());
    } else if (!dst->fBitmap.tryAllocPixels(fBitmap.info())) {
        return false;
    }

    memcpy(dst->fBitmap.getPixels(), fBitmap.getPixels(), fBitmap.computeByteSize());
    dst->fIndex = fIndex;
    dst->fDisposalMethod = fDisposalMethod;
    return true;
}

void SkAnimatedImage::reset() {
    fFinished = false;
    fRepetitionsCompleted = 0;
    if (fActiveFrame.fIndex == 0) {
        // Already showing the first frame.
        return;
    }

    if (fRestoreFrame.fIndex == 0) {
        SkTSwap(fActiveFrame, fRestoreFrame);
        // Now we're showing the first frame.
        return;
    }

    fActiveFrame.fIndex = SkCodec::kNone;
    this->decodeNextFrame();
}

static bool is_restore_previous(SkCodecAnimation::DisposalMethod dispose) {
    return SkCodecAnimation::DisposalMethod::kRestorePrevious == dispose;
}

int SkAnimatedImage::computeNextFrame(int current, bool* animationEnded) {
    SkASSERT(animationEnded != nullptr);
    *animationEnded = false;

    const int frameToDecode = current + 1;
    if (frameToDecode == fFrameCount - 1) {
        // Final frame. Check to determine whether to stop.
        fRepetitionsCompleted++;
        if (fRepetitionCount != SkCodec::kRepetitionCountInfinite
                && fRepetitionsCompleted > fRepetitionCount) {
            *animationEnded = true;
        }
    } else if (frameToDecode == fFrameCount) {
        return 0;
    }
    return frameToDecode;
}

double SkAnimatedImage::finish() {
    fFinished = true;
    fCurrentFrameDuration = kFinished;
    return kFinished;
}

int SkAnimatedImage::decodeNextFrame() {
    if (fFinished) {
        return kFinished;
    }

    bool animationEnded = false;
    int frameToDecode = this->computeNextFrame(fActiveFrame.fIndex, &animationEnded);

    SkCodec::FrameInfo frameInfo;
    if (fCodec->codec()->getFrameInfo(frameToDecode, &frameInfo)) {
        if (!frameInfo.fFullyReceived) {
            SkCodecPrintf("Frame %i not fully received\n", frameToDecode);
            return this->finish();
        }

        fCurrentFrameDuration = frameInfo.fDuration;
    } else {
        animationEnded = true;
        if (0 == frameToDecode) {
            // Static image. This is okay.
            frameInfo.fRequiredFrame = SkCodec::kNone;
            frameInfo.fAlphaType = fCodec->getInfo().alphaType();
            // These fields won't be read.
            frameInfo.fDuration = INT_MAX;
            frameInfo.fFullyReceived = true;
            fCurrentFrameDuration = kFinished;
        } else {
            SkCodecPrintf("Error getting frameInfo for frame %i\n",
                          frameToDecode);
            return this->finish();
        }
    }

    if (frameToDecode == fActiveFrame.fIndex) {
        if (animationEnded) {
            return this->finish();
        }
        return fCurrentFrameDuration;
    }

    if (frameToDecode == fRestoreFrame.fIndex) {
        SkTSwap(fActiveFrame, fRestoreFrame);
        if (animationEnded) {
            return this->finish();
        }
        return fCurrentFrameDuration;
    }

    // The following code makes an effort to avoid overwriting a frame that will
    // be used again. If frame |i| is_restore_previous, frame |i+1| will not
    // depend on frame |i|, so do not overwrite frame |i-1|, which may be needed
    // for frame |i+1|.
    // We could be even smarter about which frames to save by looking at the
    // entire dependency chain.
    SkCodec::Options options;
    options.fFrameIndex = frameToDecode;
    if (frameInfo.fRequiredFrame == SkCodec::kNone) {
        if (is_restore_previous(frameInfo.fDisposalMethod)) {
            // frameToDecode will be discarded immediately after drawing, so
            // do not overwrite a frame which could possibly be used in the
            // future.
            if (fActiveFrame.fIndex != SkCodec::kNone &&
                    !is_restore_previous(fActiveFrame.fDisposalMethod)) {
                SkTSwap(fActiveFrame, fRestoreFrame);
            }
        }
    } else {
        auto validPriorFrame = [&frameInfo, &frameToDecode](const Frame& frame) {
            if (SkCodec::kNone == frame.fIndex || is_restore_previous(frame.fDisposalMethod)) {
                return false;
            }

            return frame.fIndex >= frameInfo.fRequiredFrame && frame.fIndex < frameToDecode;
        };
        if (validPriorFrame(fActiveFrame)) {
            if (is_restore_previous(frameInfo.fDisposalMethod)) {
                // fActiveFrame is a good frame to use for this one, but we
                // don't want to overwrite it.
                fActiveFrame.copyTo(&fRestoreFrame);
            }
            options.fPriorFrame = fActiveFrame.fIndex;
        } else if (validPriorFrame(fRestoreFrame)) {
            if (!is_restore_previous(frameInfo.fDisposalMethod)) {
                SkTSwap(fActiveFrame, fRestoreFrame);
            } else if (!fRestoreFrame.copyTo(&fActiveFrame)) {
                SkCodecPrintf("Failed to restore frame\n");
                return this->finish();
            }
            options.fPriorFrame = fActiveFrame.fIndex;
        }
    }

    auto alphaType = kOpaque_SkAlphaType == frameInfo.fAlphaType ?
                     kOpaque_SkAlphaType : kPremul_SkAlphaType;
    SkBitmap* dst = &fActiveFrame.fBitmap;
    if (dst->getPixels()) {
        SkAssertResult(dst->setAlphaType(alphaType));
    } else {
        auto info = fDecodeInfo.makeAlphaType(alphaType);
        if (!dst->tryAllocPixels(info)) {
            return this->finish();
        }
    }

    auto result = fCodec->codec()->getPixels(dst->info(), dst->getPixels(), dst->rowBytes(),
                                             &options);
    if (result != SkCodec::kSuccess) {
        SkCodecPrintf("error %i, frame %i of %i\n", result, frameToDecode, fFrameCount);
        return this->finish();
    }

    fActiveFrame.fIndex = frameToDecode;
    fActiveFrame.fDisposalMethod = frameInfo.fDisposalMethod;

    if (animationEnded) {
        return this->finish();
    }
    return fCurrentFrameDuration;
}

void SkAnimatedImage::onDraw(SkCanvas* canvas) {
    if (fSimple) {
        canvas->drawBitmap(fActiveFrame.fBitmap, 0, 0);
        return;
    }

    SkRect bounds = this->getBounds();
    if (fPostProcess) {
        canvas->saveLayer(&bounds, nullptr);
    }
    {
        SkAutoCanvasRestore acr(canvas, fPostProcess);
        canvas->concat(fMatrix);
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        paint.setFilterQuality(kLow_SkFilterQuality);
        canvas->drawBitmap(fActiveFrame.fBitmap, 0, 0, &paint);
    }
    if (fPostProcess) {
        canvas->drawPicture(fPostProcess);
        canvas->restore();
    }
}

void SkAnimatedImage::setRepetitionCount(int newCount) {
    fRepetitionCount = newCount;
}

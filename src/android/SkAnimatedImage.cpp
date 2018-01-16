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

// Sentinel value for starting at the beginning.
static constexpr double kInit = -1.0;

SkAnimatedImage::SkAnimatedImage(std::unique_ptr<SkAndroidCodec> codec, SkISize scaledSize,
        SkImageInfo decodeInfo, SkIRect cropRect, sk_sp<SkPicture> postProcess)
    : fCodec(std::move(codec))
    , fScaledSize(scaledSize)
    , fDecodeInfo(decodeInfo)
    , fCropRect(cropRect)
    , fPostProcess(std::move(postProcess))
    , fSimple(fScaledSize == fDecodeInfo.dimensions() && !fPostProcess
              && fCropRect == fDecodeInfo.bounds())
    , fFinished(false)
    , fRunning(false)
    , fNowMS(kInit)
    , fRemainingMS(kInit)
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
    this->update(kInit);
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

void SkAnimatedImage::start() {
    fRunning = true;
}

void SkAnimatedImage::stop() {
    fRunning = false;
}

void SkAnimatedImage::reset() {
    this->update(kInit);
}

static bool is_restore_previous(SkCodecAnimation::DisposalMethod dispose) {
    return SkCodecAnimation::DisposalMethod::kRestorePrevious == dispose;
}

double SkAnimatedImage::update(double msecs) {
    if (fFinished) {
        return std::numeric_limits<double>::max();
    }

    const double lastUpdateMS = fNowMS;
    fNowMS = msecs;
    const double msSinceLastUpdate = fNowMS - lastUpdateMS;

    const int frameCount = fCodec->codec()->getFrameCount();
    int frameToDecode = SkCodec::kNone;
    if (kInit == msecs) {
        frameToDecode = 0;
    } else {
        if (!fRunning || lastUpdateMS == kInit) {
            return kInit;
        }
        if (msSinceLastUpdate < fRemainingMS) {
            fRemainingMS -= msSinceLastUpdate;
            return fRemainingMS + fNowMS;
        } else {
            frameToDecode = (fActiveFrame.fIndex + 1) % frameCount;
        }
    }

    SkCodec::FrameInfo frameInfo;
    if (fCodec->codec()->getFrameInfo(frameToDecode, &frameInfo)) {
        if (!frameInfo.fFullyReceived) {
            SkCodecPrintf("Frame %i not fully received\n", frameToDecode);
            fFinished = true;
            return std::numeric_limits<double>::max();
        }

        if (kInit == msecs) {
            fRemainingMS = frameInfo.fDuration;
        } else {
            // Check to see whether we should skip this frame.
            double pastUpdate = msSinceLastUpdate - fRemainingMS;
            while (pastUpdate >= frameInfo.fDuration) {
                SkCodecPrintf("Skipping frame %i\n", frameToDecode);
                pastUpdate -= frameInfo.fDuration;
                frameToDecode = (frameToDecode + 1) % frameCount;
                if (!fCodec->codec()->getFrameInfo(frameToDecode, &frameInfo)) {
                    SkCodecPrintf("Could not getFrameInfo for frame %i",
                                  frameToDecode);
                    // Prior call to getFrameInfo succeeded, so use that one.
                    frameToDecode--;
                    fFinished = true;
                    if (frameToDecode < 0) {
                        return std::numeric_limits<double>::max();
                    }
                }
            }
            fRemainingMS = frameInfo.fDuration - pastUpdate;
        }
    } else {
        fFinished = true;
        if (0 == frameToDecode) {
            // Static image. This is okay.
            frameInfo.fRequiredFrame = SkCodec::kNone;
            frameInfo.fAlphaType = fCodec->getInfo().alphaType();
            // These fields won't be read.
            frameInfo.fDuration = INT_MAX;
            frameInfo.fFullyReceived = true;
        } else {
            SkCodecPrintf("Error getting frameInfo for frame %i\n",
                          frameToDecode);
            return std::numeric_limits<double>::max();
        }
    }

    if (frameToDecode == fActiveFrame.fIndex) {
        return fRemainingMS + fNowMS;
    }

    if (frameToDecode == fRestoreFrame.fIndex) {
        SkTSwap(fActiveFrame, fRestoreFrame);
        return fRemainingMS + fNowMS;
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
                fFinished = true;
                return std::numeric_limits<double>::max();
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
            fFinished = true;
            return std::numeric_limits<double>::max();
        }
    }

    auto result = fCodec->codec()->getPixels(dst->info(), dst->getPixels(), dst->rowBytes(),
                                             &options);
    if (result != SkCodec::kSuccess) {
        SkCodecPrintf("error %i, frame %i of %i\n", result, frameToDecode, frameCount);
        // Reset to the beginning.
        fActiveFrame.fIndex = SkCodec::kNone;
        return 0.0;
    }

    fActiveFrame.fIndex = frameToDecode;
    fActiveFrame.fDisposalMethod = frameInfo.fDisposalMethod;
    return fRemainingMS + fNowMS;
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

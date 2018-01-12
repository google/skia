/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAnimatedImage.h"
#include "SkCanvas.h"
#include "SkCodec.h"
#include "SkCodecPriv.h"

sk_sp<SkAnimatedImage> SkAnimatedImage::MakeFromCodec(std::unique_ptr<SkCodec> codec) {
    if (!codec) {
        return nullptr;
    }

    auto image = sk_sp<SkAnimatedImage>(new SkAnimatedImage(std::move(codec)));
    if (!image->fActiveFrame.fBitmap.getPixels()) {
        // tryAllocPixels failed.
        return nullptr;
    }

    return image;
}

// Sentinel value for starting at the beginning.
static constexpr double kInit = -1.0;

SkAnimatedImage::SkAnimatedImage(std::unique_ptr<SkCodec> codec)
    : fCodec(std::move(codec))
    , fFinished(false)
    , fRunning(false)
    , fNowMS(kInit)
    , fRemainingMS(kInit)
{
    if (!fActiveFrame.fBitmap.tryAllocPixels(fCodec->getInfo())) {
        return;
    }

    this->update(kInit);
}

SkAnimatedImage::~SkAnimatedImage() { }

SkRect SkAnimatedImage::onGetBounds() {
    return SkRect::Make(fCodec->getInfo().bounds());
}

void SkAnimatedImage::onDraw(SkCanvas* canvas) {
    canvas->drawBitmap(fActiveFrame.fBitmap, 0, 0);
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

    const int frameCount = fCodec->getFrameCount();
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
    if (fCodec->getFrameInfo(frameToDecode, &frameInfo)) {
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
                if (!fCodec->getFrameInfo(frameToDecode, &frameInfo)) {
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
        auto info = fCodec->getInfo().makeAlphaType(alphaType);
        if (!dst->tryAllocPixels(info)) {
            fFinished = true;
            return std::numeric_limits<double>::max();
        }
    }

    auto result = fCodec->getPixels(dst->info(), dst->getPixels(), dst->rowBytes(), &options);
    if (result != SkCodec::kSuccess) {
        SkCodecPrintf("error %i, frame %i of %i\n", result, frameToDecode, fCodec->getFrameCount());
        // Reset to the beginning.
        fActiveFrame.fIndex = SkCodec::kNone;
        return 0.0;
    }

    fActiveFrame.fIndex = frameToDecode;
    fActiveFrame.fDisposalMethod = frameInfo.fDisposalMethod;
    return fRemainingMS + fNowMS;
}

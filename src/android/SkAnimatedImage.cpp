/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/SkAnimatedImage.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPixelRef.h"
#include "src/codec/SkCodecPriv.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkPixmapPriv.h"

#include <limits.h>
#include <utility>

sk_sp<SkAnimatedImage> SkAnimatedImage::Make(std::unique_ptr<SkAndroidCodec> codec,
        const SkImageInfo& requestedInfo, SkIRect cropRect, sk_sp<SkPicture> postProcess) {
    if (!codec) {
        return nullptr;
    }

    if (!requestedInfo.bounds().contains(cropRect)) {
        return nullptr;
    }

    auto image = sk_sp<SkAnimatedImage>(new SkAnimatedImage(std::move(codec), requestedInfo,
                cropRect, std::move(postProcess)));
    if (!image->fDisplayFrame.fBitmap.getPixels()) {
        // tryAllocPixels failed.
        return nullptr;
    }

    return image;
}

sk_sp<SkAnimatedImage> SkAnimatedImage::Make(std::unique_ptr<SkAndroidCodec> codec) {
    if (!codec) {
        return nullptr;
    }

    const auto& decodeInfo = codec->getInfo();
    const auto  cropRect   = SkIRect::MakeSize(decodeInfo.dimensions());
    return Make(std::move(codec), decodeInfo, cropRect, nullptr);
}

SkAnimatedImage::SkAnimatedImage(std::unique_ptr<SkAndroidCodec> codec,
        const SkImageInfo& requestedInfo, SkIRect cropRect, sk_sp<SkPicture> postProcess)
    : fCodec(std::move(codec))
    , fDecodeInfo(requestedInfo)
    , fCropRect(cropRect)
    , fPostProcess(std::move(postProcess))
    , fFrameCount(fCodec->codec()->getFrameCount())
    , fSampleSize(1)
    , fFinished(false)
    , fRepetitionCount(fCodec->codec()->getRepetitionCount())
    , fRepetitionsCompleted(0)
{
    auto scaledSize = requestedInfo.dimensions();

    // For simplicity in decoding and compositing frames, decode directly to a size and
    // orientation that fCodec can do directly, and then use fMatrix to handle crop (along with a
    // clip), orientation, and scaling outside of fCodec. The matrices are computed individually
    // and applied in the following order:
    //      [crop] X [origin] X [scale]
    const auto origin = fCodec->codec()->getOrigin();
    if (origin != SkEncodedOrigin::kDefault_SkEncodedOrigin) {
        // The origin is applied after scaling, so use scaledSize, which is the final scaled size.
        fMatrix = SkEncodedOriginToMatrix(origin, scaledSize.width(), scaledSize.height());

        if (SkEncodedOriginSwapsWidthHeight(origin)) {
            // The client asked for sizes post-rotation. Swap back to the pre-rotation sizes to pass
            // to fCodec and for the scale matrix computation.
            fDecodeInfo = SkPixmapPriv::SwapWidthHeight(fDecodeInfo);
            scaledSize = { scaledSize.height(), scaledSize.width() };
        }
    }

    auto decodeSize = scaledSize;
    fSampleSize = fCodec->computeSampleSize(&decodeSize);
    fDecodeInfo = fDecodeInfo.makeDimensions(decodeSize);

    if (!fDecodingFrame.fBitmap.tryAllocPixels(fDecodeInfo)) {
        return;
    }

    if (scaledSize != fDecodeInfo.dimensions()) {
        float scaleX = (float) scaledSize.width()  / fDecodeInfo.width();
        float scaleY = (float) scaledSize.height() / fDecodeInfo.height();
        fMatrix.preConcat(SkMatrix::Scale(scaleX, scaleY));
    }
    fMatrix.postConcat(SkMatrix::Translate(-fCropRect.fLeft, -fCropRect.fTop));
    this->decodeNextFrame();
}

SkAnimatedImage::~SkAnimatedImage() { }

SkRect SkAnimatedImage::onGetBounds() {
    return SkRect::MakeIWH(fCropRect.width(), fCropRect.height());
}

SkAnimatedImage::Frame::Frame()
    : fIndex(SkCodec::kNoFrame)
{}

bool SkAnimatedImage::Frame::init(const SkImageInfo& info, OnInit onInit) {
    if (fBitmap.getPixels()) {
        if (fBitmap.pixelRef()->unique()) {
            SkAssertResult(fBitmap.setAlphaType(info.alphaType()));
            return true;
        }

        // An SkCanvas provided to onDraw is still holding a reference.
        // Copy before we decode to ensure that we don't overwrite the
        // expected contents of the image.
        if (OnInit::kRestoreIfNecessary == onInit) {
            SkBitmap tmp;
            if (!tmp.tryAllocPixels(info)) {
                return false;
            }

            memcpy(tmp.getPixels(), fBitmap.getPixels(), fBitmap.computeByteSize());
            using std::swap;
            swap(tmp, fBitmap);
            return true;
        }
    }

    return fBitmap.tryAllocPixels(info);
}

bool SkAnimatedImage::Frame::copyTo(Frame* dst) const {
    if (!dst->init(fBitmap.info(), OnInit::kNoRestore)) {
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
    if (fDisplayFrame.fIndex != 0) {
        fDisplayFrame.fIndex = SkCodec::kNoFrame;
        this->decodeNextFrame();
    }
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
    const int frameToDecode = this->computeNextFrame(fDisplayFrame.fIndex, &animationEnded);

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
            frameInfo.fRequiredFrame = SkCodec::kNoFrame;
            frameInfo.fAlphaType = fCodec->getInfo().alphaType();
            frameInfo.fDisposalMethod = SkCodecAnimation::DisposalMethod::kKeep;
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

    if (frameToDecode == fDisplayFrame.fIndex) {
        if (animationEnded) {
            return this->finish();
        }
        return fCurrentFrameDuration;
    }

    for (Frame* frame : { &fRestoreFrame, &fDecodingFrame }) {
        if (frameToDecode == frame->fIndex) {
            using std::swap;
            swap(fDisplayFrame, *frame);
            if (animationEnded) {
                return this->finish();
            }
            return fCurrentFrameDuration;
        }
    }

    // The following code makes an effort to avoid overwriting a frame that will
    // be used again. If frame |i| is_restore_previous, frame |i+1| will not
    // depend on frame |i|, so do not overwrite frame |i-1|, which may be needed
    // for frame |i+1|.
    // We could be even smarter about which frames to save by looking at the
    // entire dependency chain.
    SkAndroidCodec::AndroidOptions options;
    options.fSampleSize = fSampleSize;
    options.fFrameIndex = frameToDecode;
    if (frameInfo.fRequiredFrame == SkCodec::kNoFrame) {
        if (is_restore_previous(frameInfo.fDisposalMethod)) {
            // frameToDecode will be discarded immediately after drawing, so
            // do not overwrite a frame which could possibly be used in the
            // future.
            if (fDecodingFrame.fIndex != SkCodec::kNoFrame &&
                    !is_restore_previous(fDecodingFrame.fDisposalMethod)) {
                using std::swap;
                swap(fDecodingFrame, fRestoreFrame);
            }
        }
    } else {
        auto validPriorFrame = [&frameInfo, &frameToDecode](const Frame& frame) {
            if (SkCodec::kNoFrame == frame.fIndex ||
                    is_restore_previous(frame.fDisposalMethod)) {
                return false;
            }

            return frame.fIndex >= frameInfo.fRequiredFrame && frame.fIndex < frameToDecode;
        };
        if (validPriorFrame(fDecodingFrame)) {
            if (is_restore_previous(frameInfo.fDisposalMethod)) {
                // fDecodingFrame is a good frame to use for this one, but we
                // don't want to overwrite it.
                fDecodingFrame.copyTo(&fRestoreFrame);
            }
            options.fPriorFrame = fDecodingFrame.fIndex;
        } else if (validPriorFrame(fDisplayFrame)) {
            if (!fDisplayFrame.copyTo(&fDecodingFrame)) {
                SkCodecPrintf("Failed to allocate pixels for frame\n");
                return this->finish();
            }
            options.fPriorFrame = fDecodingFrame.fIndex;
        } else if (validPriorFrame(fRestoreFrame)) {
            if (!is_restore_previous(frameInfo.fDisposalMethod)) {
                using std::swap;
                swap(fDecodingFrame, fRestoreFrame);
            } else if (!fRestoreFrame.copyTo(&fDecodingFrame)) {
                SkCodecPrintf("Failed to restore frame\n");
                return this->finish();
            }
            options.fPriorFrame = fDecodingFrame.fIndex;
        }
    }

    auto alphaType = kOpaque_SkAlphaType == frameInfo.fAlphaType ?
                     kOpaque_SkAlphaType : kPremul_SkAlphaType;
    auto info = fDecodeInfo.makeAlphaType(alphaType);
    SkBitmap* dst = &fDecodingFrame.fBitmap;
    if (!fDecodingFrame.init(info, Frame::OnInit::kRestoreIfNecessary)) {
        return this->finish();
    }

    auto result = fCodec->getAndroidPixels(dst->info(), dst->getPixels(), dst->rowBytes(),
                                           &options);
    if (result != SkCodec::kSuccess) {
        SkCodecPrintf("error %i, frame %i of %i\n", result, frameToDecode, fFrameCount);
        return this->finish();
    }

    fDecodingFrame.fIndex = frameToDecode;
    fDecodingFrame.fDisposalMethod = frameInfo.fDisposalMethod;

    using std::swap;
    swap(fDecodingFrame, fDisplayFrame);
    fDisplayFrame.fBitmap.notifyPixelsChanged();

    if (animationEnded) {
        return this->finish();
    } else if (fCodec->getEncodedFormat() == SkEncodedImageFormat::kHEIF) {
        // HEIF doesn't know the frame duration until after decoding. Update to
        // the correct value. Note that earlier returns in this method either
        // return kFinished, or fCurrentFrameDuration. If they return the
        // latter, it is a frame that was previously decoded, so it has the
        // updated value.
        if (fCodec->codec()->getFrameInfo(frameToDecode, &frameInfo)) {
            fCurrentFrameDuration = frameInfo.fDuration;
        } else {
            SkCodecPrintf("Failed to getFrameInfo on second attempt (HEIF)");
        }
    }
    return fCurrentFrameDuration;
}

void SkAnimatedImage::onDraw(SkCanvas* canvas) {
    auto image = this->getCurrentFrameSimple();

    if (this->simple()) {
        canvas->drawImage(image, 0, 0);
        return;
    }

    SkRect bounds = this->getBounds();
    if (fPostProcess) {
        canvas->saveLayer(&bounds, nullptr);
    }
    canvas->clipRect(bounds);
    {
        SkAutoCanvasRestore acr(canvas, fPostProcess != nullptr);
        canvas->concat(fMatrix);
        canvas->drawImage(image, 0, 0, SkSamplingOptions(SkFilterMode::kLinear), nullptr);
    }
    if (fPostProcess) {
        canvas->drawPicture(fPostProcess);
        canvas->restore();
    }
}

void SkAnimatedImage::setRepetitionCount(int newCount) {
    fRepetitionCount = newCount;
}

sk_sp<SkImage> SkAnimatedImage::getCurrentFrameSimple() {
    // This SkBitmap may be reused later to decode the following frame. But Frame::init
    // lazily copies the pixel ref if it has any other references. So it is safe to not
    // do a deep copy here.
    return SkMakeImageFromRasterBitmap(fDisplayFrame.fBitmap,
                                       kNever_SkCopyPixelsMode);
}

sk_sp<SkImage> SkAnimatedImage::getCurrentFrame() {
    if (this->simple()) return this->getCurrentFrameSimple();

    auto imageInfo = fDisplayFrame.fBitmap.info().makeDimensions(fCropRect.size());
    if (fPostProcess) {
        // Defensively use premul in case the post process adds alpha.
        imageInfo = imageInfo.makeAlphaType(kPremul_SkAlphaType);
    }

    SkBitmap dst;
    if (!dst.tryAllocPixels(imageInfo)) {
        return nullptr;
    }

    SkCanvas canvas(dst);
    this->draw(&canvas);
    return SkMakeImageFromRasterBitmap(dst, kNever_SkCopyPixelsMode);
}

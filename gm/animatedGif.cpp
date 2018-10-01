/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkCodec.h"
#include "SkColor.h"
#include "SkCommandLineFlags.h"
#include "SkPaint.h"
#include "SkString.h"
#include "Resources.h"

#include <vector>

DEFINE_string(animatedGif, "images/test640x479.gif", "Animated gif in resources folder");

namespace {
    void error(SkCanvas* canvas, const SkString& errorText) {
        constexpr SkScalar kOffset = 5.0f;
        canvas->drawColor(SK_ColorRED);
        SkPaint paint;
        SkRect bounds;
        paint.measureText(errorText.c_str(), errorText.size(), &bounds);
        canvas->drawString(errorText, kOffset, bounds.height() + kOffset,
                         paint);
    }
}

class AnimatedGifGM : public skiagm::GM {
private:
    std::unique_ptr<SkCodec>        fCodec;
    int                             fFrame;
    double                          fNextUpdate;
    int                             fTotalFrames;
    std::vector<SkCodec::FrameInfo> fFrameInfos;
    std::vector<SkBitmap>           fFrames;

    void drawFrame(SkCanvas* canvas, int frameIndex) {
        // FIXME: Create from an Image/ImageGenerator?
        if (frameIndex >= (int) fFrames.size()) {
            fFrames.resize(frameIndex + 1);
        }
        SkBitmap& bm = fFrames[frameIndex];
        if (!bm.getPixels()) {
            const SkImageInfo info = fCodec->getInfo().makeColorType(kN32_SkColorType);
            bm.allocPixels(info);

            SkCodec::Options opts;
            opts.fFrameIndex = frameIndex;
            const int requiredFrame = fFrameInfos[frameIndex].fRequiredFrame;
            if (requiredFrame != SkCodec::kNoFrame) {
                SkASSERT(requiredFrame >= 0
                         && static_cast<size_t>(requiredFrame) < fFrames.size());
                SkBitmap& requiredBitmap = fFrames[requiredFrame];
                // For simplicity, do not try to cache old frames
                if (requiredBitmap.getPixels() &&
                        sk_tool_utils::copy_to(&bm, requiredBitmap.colorType(), requiredBitmap)) {
                    opts.fPriorFrame = requiredFrame;
                }
            }

            if (SkCodec::kSuccess != fCodec->getPixels(info, bm.getPixels(),
                                                       bm.rowBytes(), &opts)) {
                SkDebugf("Could not getPixels for frame %i: %s", frameIndex, FLAGS_animatedGif[0]);
                return;
            }
        }

        canvas->drawBitmap(bm, 0, 0);
    }

public:
    AnimatedGifGM()
    : fFrame(0)
    , fNextUpdate (-1)
    , fTotalFrames (-1) {}

private:
    SkString onShortName() override {
        return SkString("animatedGif");
    }

    SkISize onISize() override {
        if (this->initCodec()) {
            SkISize dim = fCodec->getInfo().dimensions();
            // Wide enough to display all the frames.
            dim.fWidth *= fTotalFrames;
            // Tall enough to show the row of frames plus an animating version.
            dim.fHeight *= 2;
            return dim;
        }
        return SkISize::Make(640, 480);
    }

    void onDrawBackground(SkCanvas* canvas) override {
        canvas->clear(SK_ColorWHITE);
        if (this->initCodec()) {
            SkAutoCanvasRestore acr(canvas, true);
            for (int frameIndex = 0; frameIndex < fTotalFrames; frameIndex++) {
                this->drawFrame(canvas, frameIndex);
                canvas->translate(SkIntToScalar(fCodec->getInfo().width()), 0);
            }
        }
    }

    bool initCodec() {
        if (fCodec) {
            return true;
        }
        if (FLAGS_animatedGif.isEmpty()) {
            SkDebugf("Nothing specified for --animatedGif!");
            return false;
        }

        std::unique_ptr<SkStream> stream(GetResourceAsStream(FLAGS_animatedGif[0]));
        if (!stream) {
            return false;
        }

        fCodec = SkCodec::MakeFromStream(std::move(stream));
        if (!fCodec) {
            SkDebugf("Could create codec from %s", FLAGS_animatedGif[0]);
            return false;
        }

        fFrame = 0;
        fFrameInfos = fCodec->getFrameInfo();
        fTotalFrames = fFrameInfos.size();
        return true;
    }

    void onDraw(SkCanvas* canvas) override {
        if (!fCodec) {
            SkString errorText = SkStringPrintf("Nothing to draw; %s", FLAGS_animatedGif[0]);
            error(canvas, errorText);
            return;
        }

        SkAutoCanvasRestore acr(canvas, true);
        canvas->translate(0, SkIntToScalar(fCodec->getInfo().height()));
        this->drawFrame(canvas, fFrame);
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        if (!fCodec || fTotalFrames == 1) {
            return false;
        }

        double secs = timer.msec() * .1;
        if (fNextUpdate < double(0)) {
            // This is a sentinel that we have not done any updates yet.
            // I'm assuming this gets called *after* onOnceBeforeDraw, so our first frame should
            // already have been retrieved.
            SkASSERT(fFrame == 0);
            fNextUpdate = secs + fFrameInfos[fFrame].fDuration;

            return true;
        }

        if (secs < fNextUpdate) {
            return true;
        }

        while (secs >= fNextUpdate) {
            // Retrieve the next frame.
            fFrame++;
            if (fFrame == fTotalFrames) {
                fFrame = 0;
            }

            // Note that we loop here. This is not safe if we need to draw the intermediate frame
            // in order to draw correctly.
            fNextUpdate += fFrameInfos[fFrame].fDuration;
        }

        return true;
    }
};
DEF_GM(return new AnimatedGifGM);


class AnimCodecPlayer {
    std::unique_ptr<SkCodec>        fCodec;
    SkImageInfo                     fImageInfo;
    std::vector<SkCodec::FrameInfo> fFrameInfos;
    std::vector<sk_sp<SkImage> >    fImages;

public:
    AnimCodecPlayer(std::unique_ptr<SkCodec> codec);

    SkISize dimensions();
    int     countFrames();
    sk_sp<SkImage> getFrame(int index);
};

AnimCodecPlayer::AnimCodecPlayer(std::unique_ptr<SkCodec> codec) : fCodec(std::move(codec)) {
    fImageInfo = fCodec->getInfo();
    fFrameInfos = fCodec->getFrameInfo();
    fImages.resize(fFrameInfos.size());
}

SkISize AnimCodecPlayer::dimensions() {
    return { fImageInfo.width(), fImageInfo.height() };
}

int AnimCodecPlayer::countFrames() {
    return SkToInt(fFrameInfos.size());
}

sk_sp<SkImage> AnimCodecPlayer::getFrame(int index) {
    SkASSERT((unsigned)index < (unsigned)this->countFrames());

    if (fImages[index]) {
        return fImages[index];
    }

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
        return fImages[index] = SkImage::MakeRasterData(fImageInfo, std::move(data), rb);
    }
    return nullptr;
}

static std::unique_ptr<SkCodec> load_codec(const char filename[]) {
    auto data = SkData::MakeFromFileName(filename);
    return SkCodec::MakeFromData(data);
}

class AnimCodecPlayerGM : public skiagm::GM {
private:
    AnimCodecPlayer fPlayer;

public:
    AnimCodecPlayerGM() : fPlayer(load_codec("/Users/reed/Downloads/claws.gif")) {
    }

private:
    SkString onShortName() override {
        return SkString("AnimCodecPlayer");
    }

    SkISize onISize() override {
        return fPlayer.dimensions();
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawImage(fPlayer.getFrame(0), 0, 0, nullptr);
    }

#if 0
    bool onAnimate(const SkAnimTimer& timer) override {
        if (!fCodec || fTotalFrames == 1) {
            return false;
        }

        double secs = timer.msec() * .1;
        if (fNextUpdate < double(0)) {
            // This is a sentinel that we have not done any updates yet.
            // I'm assuming this gets called *after* onOnceBeforeDraw, so our first frame should
            // already have been retrieved.
            SkASSERT(fFrame == 0);
            fNextUpdate = secs + fFrameInfos[fFrame].fDuration;

            return true;
        }

        if (secs < fNextUpdate) {
            return true;
        }

        while (secs >= fNextUpdate) {
            // Retrieve the next frame.
            fFrame++;
            if (fFrame == fTotalFrames) {
                fFrame = 0;
            }

            // Note that we loop here. This is not safe if we need to draw the intermediate frame
            // in order to draw correctly.
            fNextUpdate += fFrameInfos[fFrame].fDuration;
        }

        return true;
    }
#endif
};
DEF_GM(return new AnimCodecPlayerGM);

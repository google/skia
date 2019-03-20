/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CommandLineFlags.h"
#include "Resources.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkCodec.h"
#include "SkColor.h"
#include "SkFont.h"
#include "SkPaint.h"
#include "SkString.h"
#include "gm.h"
#include "sk_tool_utils.h"

#include <vector>

DEFINE_string(animatedGif, "images/test640x479.gif", "Animated gif in resources folder");

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
            return false;
        }

        fFrame = 0;
        fFrameInfos = fCodec->getFrameInfo();
        fTotalFrames = fFrameInfos.size();
        return true;
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (!this->initCodec()) {
            errorMsg->printf("Could not create codec from %s", FLAGS_animatedGif[0]);
            return DrawResult::kFail;
        }

        canvas->save();
        for (int frameIndex = 0; frameIndex < fTotalFrames; frameIndex++) {
            this->drawFrame(canvas, frameIndex);
            canvas->translate(SkIntToScalar(fCodec->getInfo().width()), 0);
        }
        canvas->restore();

        SkAutoCanvasRestore acr(canvas, true);
        canvas->translate(0, SkIntToScalar(fCodec->getInfo().height()));
        this->drawFrame(canvas, fFrame);
        return DrawResult::kOk;
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


#include "SkAnimCodecPlayer.h"
#include "SkOSFile.h"
#include "SkMakeUnique.h"

static std::unique_ptr<SkCodec> load_codec(const char filename[]) {
    return SkCodec::MakeFromData(SkData::MakeFromFileName(filename));
}

class AnimCodecPlayerGM : public skiagm::GM {
private:
    std::vector<std::unique_ptr<SkAnimCodecPlayer> > fPlayers;
    uint32_t          fBaseMSec = 0;

public:
    AnimCodecPlayerGM() {
        const char* root = "/skia/anim/";
        SkOSFile::Iter iter(root);
        SkString path;
        while (iter.next(&path)) {
            SkString completepath;
            completepath.printf("%s%s", root, path.c_str());
            auto codec = load_codec(completepath.c_str());
            if (codec) {
                fPlayers.push_back(skstd::make_unique<SkAnimCodecPlayer>(std::move(codec)));
            }
        }
    }

private:
    SkString onShortName() override {
        return SkString("AnimCodecPlayer");
    }

    SkISize onISize() override {
        return { 1024, 768 };
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(0.25f, 0.25f);
        for (auto& p : fPlayers) {
            canvas->drawImage(p->getFrame(), 0, 0, nullptr);
            canvas->translate(p->dimensions().width(), 0);
        }
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        if (fBaseMSec == 0) {
            fBaseMSec = timer.msec();
        }
        for (auto& p : fPlayers) {
            (void)p->seek(timer.msec() - fBaseMSec);
        }
        return true;
    }
};
DEF_GM(return new AnimCodecPlayerGM);

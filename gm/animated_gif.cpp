/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/utils/SkAnimCodecPlayer.h"
#include "src/core/SkOSFile.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/timer/TimeUtils.h"

#include <memory>
#include <utility>
#include <vector>

static DEFINE_string(animatedGif, "images/test640x479.gif", "Animated gif in resources folder");

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
                    ToolUtils::copy_to(&bm, requiredBitmap.colorType(), requiredBitmap)) {
                    opts.fPriorFrame = requiredFrame;
                }
            }

            if (SkCodec::kSuccess != fCodec->getPixels(info, bm.getPixels(),
                                                       bm.rowBytes(), &opts)) {
                SkDebugf("Could not getPixels for frame %i: %s", frameIndex, FLAGS_animatedGif[0]);
                return;
            }
        }

        canvas->drawImage(bm.asImage(), 0, 0);
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

    bool onAnimate(double nanos) override {
        if (!fCodec || fTotalFrames == 1) {
            return false;
        }

        double secs = TimeUtils::NanosToMSec(nanos) * .1;
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
                fPlayers.push_back(std::make_unique<SkAnimCodecPlayer>(std::move(codec)));
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
            canvas->drawImage(p->getFrame(), 0, 0);
            canvas->translate(p->dimensions().width(), 0);
        }
    }

    bool onAnimate(double nanos) override {
        if (fBaseMSec == 0) {
            fBaseMSec = TimeUtils::NanosToMSec(nanos);
        }
        for (auto& p : fPlayers) {
            (void)p->seek(TimeUtils::NanosToMSec(nanos) - fBaseMSec);
        }
        return true;
    }
};
DEF_GM(return new AnimCodecPlayerGM);

class AnimCodecPlayerExifGM : public skiagm::GM {
    const char* fPath;
    SkISize fSize = SkISize::MakeEmpty();
    std::unique_ptr<SkAnimCodecPlayer> fPlayer;
    std::vector<SkCodec::FrameInfo> fFrameInfos;

    void init() {
        if (!fPlayer) {
            auto data = GetResourceAsData(fPath);
            if (!data) return;

            auto codec = SkCodec::MakeFromData(std::move(data));
            fFrameInfos = codec->getFrameInfo();
            fPlayer = std::make_unique<SkAnimCodecPlayer>(std::move(codec));
            if (!fPlayer) return;

            // We'll draw one of each frame, so make it big enough to hold them all
            // in a grid. The grid will be roughly square, with "factor" frames per
            // row and up to "factor" rows.
            const size_t count = fFrameInfos.size();
            const float root = sqrt((float) count);
            const int factor = sk_float_ceil2int(root);

            auto imageSize = fPlayer->dimensions();
            fSize.fWidth  = imageSize.fWidth  * factor;
            fSize.fHeight = imageSize.fHeight * sk_float_ceil2int((float) count / (float) factor);
        }
    }

    SkString onShortName() override {
        return SkStringPrintf("AnimCodecPlayerExif_%s", strrchr(fPath, '/') + 1);
    }

    SkISize onISize() override {
        this->init();
        return fSize;
    }

    void onDraw(SkCanvas* canvas) override {
        this->init();
        if (!fPlayer) return;

        const float root = sqrt((float) fFrameInfos.size());
        const int factor = sk_float_ceil2int(root);
        auto dimensions = fPlayer->dimensions();

        uint32_t duration = 0;
        for (int frame = 0; duration < fPlayer->duration(); frame++) {
            SkAutoCanvasRestore acr(canvas, true);
            const int xTranslate = (frame % factor) * dimensions.width();
            const int yTranslate = (frame / factor) * dimensions.height();
            canvas->translate(SkIntToScalar(xTranslate), SkIntToScalar(yTranslate));


            auto image = fPlayer->getFrame();
            canvas->drawImage(image, 0, 0);
            duration += fFrameInfos[frame].fDuration;
            fPlayer->seek(duration);
        }
    }
public:
    AnimCodecPlayerExifGM(const char* path)
        : fPath(path)
    {}

    ~AnimCodecPlayerExifGM() override = default;
};

DEF_GM(return new AnimCodecPlayerExifGM("images/required.webp");)
DEF_GM(return new AnimCodecPlayerExifGM("images/required.gif");)
DEF_GM(return new AnimCodecPlayerExifGM("images/stoplight_h.webp");)

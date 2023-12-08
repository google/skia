/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/SkAnimatedImage.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/timer/TimeUtils.h"
#include "tools/viewer/Slide.h"

static constexpr char kPauseKey = 'p';
static constexpr char kResetKey = 'r';

class AnimatedImageSlide : public Slide {
    sk_sp<SkAnimatedImage>  fImage;
    sk_sp<SkDrawable>       fDrawable;
    SkScalar                fYOffset = 0;
    bool                    fRunning = false;
    double                  fCurrentTime = 0.0;
    double                  fLastWallTime = 0.0;
    double                  fTimeToShowNextFrame = 0.0;

public:
    AnimatedImageSlide() { fName = "AnimatedImage"; }

    void draw(SkCanvas* canvas) override {
        SkFont font = ToolUtils::DefaultFont();
        font.setSize(20);

        SkString str = SkStringPrintf("Press '%c' to start/pause; '%c' to reset.",
                                      kPauseKey, kResetKey);
        const char* text = str.c_str();
        SkRect bounds;
        font.measureText(text, strlen(text), SkTextEncoding::kUTF8, &bounds);
        fYOffset = bounds.height();

        canvas->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8, 5, fYOffset, font, SkPaint());
        fYOffset *= 2;

        if (!fImage) {
            return;
        }

        canvas->translate(0, fYOffset);

        canvas->drawDrawable(fImage.get());
        canvas->drawDrawable(fDrawable.get(), fImage->getBounds().width(), 0);
    }

    bool animate(double nanos) override {
        if (!fImage) {
            return false;
        }

        const double lastWallTime = fLastWallTime;
        fLastWallTime = TimeUtils::NanosToMSec(nanos);

        if (fRunning) {
            fCurrentTime += fLastWallTime - lastWallTime;
            if (fCurrentTime > fTimeToShowNextFrame) {
                fTimeToShowNextFrame += fImage->decodeNextFrame();
                if (fImage->isFinished()) {
                    fRunning = false;
                }
            }
        }

        return true;
    }

    void load(SkScalar w, SkScalar h) override {
        sk_sp<SkData> file(GetResourceAsData("images/alphabetAnim.gif"));
        std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(file));
        if (!codec) {
            return;
        }

        fImage = SkAnimatedImage::Make(SkAndroidCodec::MakeFromCodec(std::move(codec)));
        if (!fImage) {
            return;
        }

        fTimeToShowNextFrame = fImage->currentFrameDuration();
        SkPictureRecorder recorder;
        auto canvas = recorder.beginRecording(fImage->getBounds());
        canvas->drawDrawable(fImage.get());
        fDrawable = recorder.finishRecordingAsDrawable();
    }

    bool onChar(SkUnichar uni) override {
        if (fImage) {
            switch (uni) {
                case kPauseKey:
                    fRunning = !fRunning;
                    if (!fImage->isFinished()) {
                        return true;
                    }
                    [[fallthrough]];
                case kResetKey:
                    fImage->reset();
                    fCurrentTime = fLastWallTime;
                    fTimeToShowNextFrame = fCurrentTime + fImage->currentFrameDuration();
                    return true;
                default:
                    break;
            }
        }
        return false;
    }
};

DEF_SLIDE( return new AnimatedImageSlide(); )

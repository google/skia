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
#include "tools/timer/AnimTimer.h"

#include "samplecode/Sample.h"
#include "tools/Resources.h"

static constexpr char kPauseKey = 'p';
static constexpr char kResetKey = 'r';

class SampleAnimatedImage : public Sample {
public:
    SampleAnimatedImage()
        : INHERITED()
        , fYOffset(0)
    {}

protected:
    void onDrawBackground(SkCanvas* canvas) override {
        SkFont font;
        font.setSize(20);

        SkString str = SkStringPrintf("Press '%c' to start/pause; '%c' to reset.",
                kPauseKey, kResetKey);
        const char* text = str.c_str();
        SkRect bounds;
        font.measureText(text, strlen(text), SkTextEncoding::kUTF8, &bounds);
        fYOffset = bounds.height();

        canvas->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8, 5, fYOffset, font, SkPaint());
        fYOffset *= 2;
    }

    void onDrawContent(SkCanvas* canvas) override {
        if (!fImage) {
            return;
        }

        canvas->translate(0, fYOffset);

        canvas->drawDrawable(fImage.get());
        canvas->drawDrawable(fDrawable.get(), fImage->getBounds().width(), 0);
    }

    bool onAnimate(const AnimTimer& animTimer) override {
        if (!fImage) {
            return false;
        }

        const double lastWallTime = fLastWallTime;
        fLastWallTime = animTimer.msec();

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

    void onOnceBeforeDraw() override {
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

    SkString name() override { return SkString("AnimatedImage"); }

    bool onChar(SkUnichar uni) override {
        if (fImage) {
            switch (uni) {
                case kPauseKey:
                    fRunning = !fRunning;
                    if (fImage->isFinished()) {
                        // fall through
                    } else {
                        return true;
                    }
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

private:
    sk_sp<SkAnimatedImage>  fImage;
    sk_sp<SkDrawable>       fDrawable;
    SkScalar                fYOffset;
    bool                    fRunning = false;
    double                  fCurrentTime = 0.0;
    double                  fLastWallTime = 0.0;
    double                  fTimeToShowNextFrame = 0.0;
    typedef Sample INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new SampleAnimatedImage(); )

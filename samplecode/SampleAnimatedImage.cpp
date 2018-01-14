/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAndroidCodec.h"
#include "SkAnimatedImage.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPictureRecorder.h"
#include "SkRect.h"
#include "SkScalar.h"
#include "SkString.h"

#include "SampleCode.h"
#include "Resources.h"

static constexpr char kPauseKey = 'p';
static constexpr char kResetKey = 'r';

class SampleAnimatedImage : public SampleView {
public:
    SampleAnimatedImage()
        : INHERITED()
        , fRunning(false)
        , fYOffset(0)
    {}

protected:
    void onDrawBackground(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        constexpr SkScalar kTextSize = 20;
        paint.setTextSize(kTextSize);

        SkString str = SkStringPrintf("Press '%c' to start/pause; '%c' to reset.",
                kPauseKey, kResetKey);
        const char* text = str.c_str();
        SkRect bounds;
        paint.measureText(text, strlen(text), &bounds);
        fYOffset = bounds.height();

        canvas->drawText(text, strlen(text), 5, fYOffset, paint);
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

    bool onAnimate(const SkAnimTimer& animTimer) override {
        if (!fImage) {
            return false;
        }

        fImage->update(animTimer.msec());
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

        SkPictureRecorder recorder;
        auto canvas = recorder.beginRecording(fImage->getBounds());
        canvas->drawDrawable(fImage.get());
        fDrawable = recorder.finishRecordingAsDrawable();
    }

    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "AnimatedImage");
            return true;
        }

        SkUnichar uni;
        if (fImage && SampleCode::CharQ(*evt, &uni)) {
            switch (uni) {
                case kPauseKey:
                    if (fRunning) {
                        fImage->stop();
                        fRunning = false;
                    } else {
                        fImage->start();
                        fRunning = true;
                    }
                    return true;
                case kResetKey:
                    fImage->reset();
                    return true;
                default:
                    break;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

private:
    sk_sp<SkAnimatedImage>  fImage;
    sk_sp<SkDrawable>       fDrawable;
    bool                    fRunning;
    SkScalar                fYOffset;
    typedef SampleView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() {
    return new SampleAnimatedImage;
}

static SkViewRegister reg(MyFactory);

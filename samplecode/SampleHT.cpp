/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkPictureRecorder.h"
#include "include/utils/SkInterpolator.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "src/core/SkPointPriv.h"
#include "tools/timer/AnimTimer.h"

const SkRect gUnitSquare = { -1, -1, 1, 1 };

static void color_to_floats(SkColor c, SkScalar f[4]) {
    f[0] = SkIntToScalar(SkColorGetA(c));
    f[1] = SkIntToScalar(SkColorGetR(c));
    f[2] = SkIntToScalar(SkColorGetG(c));
    f[3] = SkIntToScalar(SkColorGetB(c));
}

static SkColor floats_to_color(const SkScalar f[4]) {
    return SkColorSetARGB(SkScalarRoundToInt(f[0]),
                          SkScalarRoundToInt(f[1]),
                          SkScalarRoundToInt(f[2]),
                          SkScalarRoundToInt(f[3]));
}

static bool oval_contains(const SkRect& r, SkScalar x, SkScalar y) {
    SkMatrix m;
    m.setRectToRect(r, gUnitSquare, SkMatrix::kFill_ScaleToFit);
    SkPoint pt;
    m.mapXY(x, y, &pt);
    return SkPointPriv::LengthSqd(pt) <= 1;
}

static SkColor rand_opaque_color(uint32_t seed) {
    SkRandom rand(seed);
    return rand.nextU() | (0xFF << 24);
}

class HTDrawable : public SkDrawable {
    SkRect          fR;
    SkColor         fColor;
    SkInterpolator* fInterp;
    SkMSec          fTime;

public:
    HTDrawable(SkRandom& rand) {
        fR = SkRect::MakeXYWH(rand.nextRangeF(0, 640), rand.nextRangeF(0, 480),
                              rand.nextRangeF(20, 200), rand.nextRangeF(20, 200));
        fColor = rand_opaque_color(rand.nextU());
        fInterp = nullptr;
        fTime = 0;
    }

    void spawnAnimation(SkMSec now) {
        this->setTime(now);

        delete fInterp;
        fInterp = new SkInterpolator(5, 3);
        SkScalar values[5];
        color_to_floats(fColor, values); values[4] = 0;
        fInterp->setKeyFrame(0, now, values);
        values[0] = 0; values[4] = 180;
        fInterp->setKeyFrame(1, now + 1000, values);
        color_to_floats(rand_opaque_color(fColor), values); values[4] = 360;
        fInterp->setKeyFrame(2, now + 2000, values);

        fInterp->setMirror(true);
        fInterp->setRepeatCount(3);

        this->notifyDrawingChanged();
    }

    bool hitTest(SkScalar x, SkScalar y) {
        return oval_contains(fR, x, y);
    }

    void setTime(SkMSec time) { fTime = time; }

    void onDraw(SkCanvas* canvas) override {
        SkAutoCanvasRestore acr(canvas, false);

        SkPaint paint;
        paint.setAntiAlias(true);

        if (fInterp) {
            SkScalar values[5];
            SkInterpolator::Result res = fInterp->timeToValues(fTime, values);
            fColor = floats_to_color(values);

            canvas->save();
            canvas->rotate(values[4], fR.centerX(), fR.centerY());

            switch (res) {
                case SkInterpolator::kFreezeEnd_Result:
                    delete fInterp;
                    fInterp = nullptr;
                    break;
                default:
                    break;
            }
        }
        paint.setColor(fColor);
        canvas->drawRect(fR, paint);
    }

    SkRect onGetBounds() override { return fR; }
};

class HTView : public Sample {
public:
    enum {
        N = 50,
        W = 640,
        H = 480,
    };

    struct Rec {
        HTDrawable* fDrawable;
    };
    Rec fArray[N];
    sk_sp<SkDrawable> fRoot;
    SkMSec fTime;

    HTView() {
        SkRandom rand;

        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkRect::MakeWH(W, H));
        for (int i = 0; i < N; ++i) {
            fArray[i].fDrawable = new HTDrawable(rand);
            canvas->drawDrawable(fArray[i].fDrawable);
            fArray[i].fDrawable->unref();
        }
        fRoot = recorder.finishRecordingAsDrawable();
    }

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "HT");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawDrawable(fRoot.get());
    }

    bool onAnimate(const AnimTimer& timer) override {
        fTime = timer.msec();
        for (int i = 0; i < N; ++i) {
            fArray[i].fDrawable->setTime(fTime);
        }
        return true;
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        // search backwards to find the top-most
        for (int i = N - 1; i >= 0; --i) {
            if (fArray[i].fDrawable->hitTest(x, y)) {
                fArray[i].fDrawable->spawnAnimation(fTime);
                break;
            }
        }
        return nullptr;
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new HTView(); )

/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkPictureRecorder.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "src/core/SkPointPriv.h"
#include "tools/timer/TimeUtils.h"

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
    SkPoint pt = SkMatrix::RectToRect(r, gUnitSquare).mapXY(x, y);
    return SkPointPriv::LengthSqd(pt) <= 1;
}

static SkColor rand_opaque_color(uint32_t seed) {
    SkRandom rand(seed);
    return rand.nextU() | (0xFF << 24);
}

class HTDrawable : public SkDrawable ;

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
    SkString name() override { return SkString("HT"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawDrawable(fRoot.get());
    }

    bool onAnimate(double nanos) override {
        fTime = TimeUtils::NanosToMSec(nanos);
        for (int i = 0; i < N; ++i) {
            fArray[i].fDrawable->setTime(fTime);
        }
        return true;
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
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
    using INHERITED = Sample;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new HTView(); )

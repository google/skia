/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkParsePath.h"
#include "SkDashPathEffect.h"

/*
 *  Inspired by http://code.google.com/p/chromium/issues/detail?id=112145
 */
static void flower(SkCanvas* canvas, const SkPath& path, SkScalar intervals[2],
                   SkPaint::Join join) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeJoin(join);
    paint.setStrokeWidth(42);
    canvas->drawPath(path, paint);

    paint.setColor(SK_ColorRED);
    paint.setStrokeWidth(21);
    paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
    canvas->drawPath(path, paint);

    paint.setColor(SK_ColorGREEN);
    paint.setPathEffect(nullptr);
    paint.setStrokeWidth(0);
    canvas->drawPath(path, paint);
}

DEF_SIMPLE_GM(dashcubics, canvas, 865, 750) {
        SkPath path;
        const char* d = "M 337,98 C 250,141 250,212 250,212 C 250,212 250,212 250,212"
        "C 250,212 250,212 250,212 C 250,212 250,141 163,98 C 156,195 217,231 217,231"
        "C 217,231 217,231 217,231 C 217,231 217,231 217,231 C 217,231 156,195 75,250"
        "C 156,305 217,269 217,269 C 217,269 217,269 217,269 C 217,269 217,269 217,269"
        "C 217,269 156,305 163,402 C 250,359 250,288 250,288 C 250,288 250,288 250,288"
        "C 250,288 250,288 250,288 C 250,288 250,359 338,402 C 345,305 283,269 283,269"
        "C 283,269 283,269 283,269 C 283,269 283,269 283,269 C 283,269 345,305 425,250"
        "C 344,195 283,231 283,231 C 283,231 283,231 283,231 C 283,231 283,231 283,231"
        "C 283,231 344,195 338,98";

        SkParsePath::FromSVGString(d, &path);
            canvas->translate(-35.f, -55.f);
        for (int x = 0; x < 2; ++x) {
            for (int y = 0; y < 2; ++y) {
                canvas->save();
                canvas->translate(x * 430.f, y * 355.f);
                SkScalar intervals[] = { 5 + (x ? 0 : 0.0001f + 0.0001f), 10 };
                flower(canvas, path, intervals, y ? SkPaint::kDefault_Join : SkPaint::kRound_Join);
                canvas->restore();
            }
        }
}

#include "SkTrimPathEffect.h"
class TrimGM : public skiagm::GM {
public:
    TrimGM() {}

protected:
    SkString onShortName() override { return SkString("trimpatheffect"); }

    SkISize onISize() override { return SkISize::Make(1240, 390); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setPathEffect(SkTrimPathEffect::Make(0.25 + fOffset, 0.75));
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(10);

        SkPath path;
        path.moveTo(50, 300);
        path.cubicTo(100, 50, 150, 550, 200, 300);

        paint.setColor(0xFF888888);
        canvas->drawPath(path, paint);
        paint.setPathEffect(nullptr);
        paint.setStrokeWidth(0);
        paint.setColor(0xFF000000);
        canvas->drawPath(path, paint);
    }

    bool onAnimate(const SkAnimTimer&) override {
   //     fOffset += 1;
        return true;
    }
private:
    SkScalar fOffset = 0;
    typedef skiagm::GM INHERITED;
};
DEF_GM(return new TrimGM;)


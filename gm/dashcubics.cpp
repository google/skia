/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkTrimPathEffect.h"
#include "include/private/SkTArray.h"
#include "include/utils/SkParsePath.h"
#include "tools/timer/TimeUtils.h"

#include <math.h>
#include <utility>

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

class TrimGM : public skiagm::GM {
public:
    TrimGM() {
        SkAssertResult(SkParsePath::FromSVGString(
            "M   0,100 C  10, 50 190, 50 200,100"
            "M 200,100 C 210,150 390,150 400,100"
            "M 400,100 C 390, 50 210, 50 200,100"
            "M 200,100 C 190,150  10,150   0,100",
            &fPaths.push_back()));

        SkAssertResult(SkParsePath::FromSVGString(
            "M   0, 75 L 200, 75"
            "M 200, 91 L 200, 91"
            "M 200,108 L 200,108"
            "M 200,125 L 400,125",
            &fPaths.push_back()));

        SkAssertResult(SkParsePath::FromSVGString(
            "M   0,100 L  50, 50"
            "M  50, 50 L 150,150"
            "M 150,150 L 250, 50"
            "M 250, 50 L 350,150"
            "M 350,150 L 400,100",
            &fPaths.push_back()));

    }

protected:
    SkString onShortName() override { return SkString("trimpatheffect"); }

    SkISize onISize() override {
        return SkISize::Make(1400, 1000);
    }

    void onDraw(SkCanvas* canvas) override {
        static constexpr SkSize kCellSize = { 440, 150 };
        static constexpr SkScalar kOffsets[][2] = {
            { -0.33f, -0.66f },
            {  0    ,  1    },
            {  0    ,  0.25f},
            {  0.25f,  0.75f},
            {  0.75f,  1    },
            {  1    ,  0.75f},
        };

        SkPaint hairlinePaint;
        hairlinePaint.setAntiAlias(true);
        hairlinePaint.setStyle(SkPaint::kStroke_Style);
        hairlinePaint.setStrokeCap(SkPaint::kRound_Cap);
        hairlinePaint.setStrokeWidth(2);
        SkPaint normalPaint = hairlinePaint;
        normalPaint.setStrokeWidth(10);
        normalPaint.setColor(0x8000ff00);
        SkPaint invertedPaint = normalPaint;
        invertedPaint.setColor(0x80ff0000);

        for (const auto& offset : kOffsets) {
            auto start = offset[0] + fOffset,
                 stop  = offset[1] + fOffset;

            auto normalMode   = SkTrimPathEffect::Mode::kNormal,
                 invertedMode = SkTrimPathEffect::Mode::kInverted;
            if (fOffset) {
                start -= SkScalarFloorToScalar(start);
                stop  -= SkScalarFloorToScalar(stop);
                if (start > stop) {
                    using std::swap;
                    swap(start, stop);
                    swap(normalMode, invertedMode);
                }
            }

            normalPaint.setPathEffect(SkTrimPathEffect::Make(start, stop, normalMode));
            invertedPaint.setPathEffect(SkTrimPathEffect::Make(start, stop, invertedMode));

            {
                SkAutoCanvasRestore acr(canvas, true);
                for (const auto& path : fPaths) {
                    canvas->drawPath(path, normalPaint);
                    canvas->drawPath(path, invertedPaint);
                    canvas->drawPath(path, hairlinePaint);
                    canvas->translate(kCellSize.width(), 0);
                }
            }

            canvas->translate(0, kCellSize.height());
        }
    }

    bool onAnimate(double nanos) override {
        fOffset = TimeUtils::NanosToMSec(nanos) / 2000.0f;
        fOffset -= floorf(fOffset);
        return true;
    }

private:
    SkTArray<SkPath> fPaths;
    SkScalar         fOffset = 0;

    typedef skiagm::GM INHERITED;
};
DEF_GM(return new TrimGM;)


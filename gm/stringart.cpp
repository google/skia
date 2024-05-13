/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "tools/ToolUtils.h"
#include "tools/timer/TimeUtils.h"

// Reproduces https://code.google.com/p/chromium/issues/detail?id=279014

constexpr int kWidth = 440;
constexpr int kHeight = 440;
constexpr SkScalar kAngle = 0.305f;
constexpr int kMaxNumSteps = 140;

// Renders a string art shape.
// The particular shape rendered can be controlled by adjusting kAngle, from 0 to 1

class StringArtGM : public skiagm::GM {
public:
    StringArtGM() : fNumSteps(kMaxNumSteps) {}

protected:
    SkString getName() const override { return SkString("stringart"); }

    SkISize getISize() override { return SkISize::Make(kWidth, kHeight); }

    void onDraw(SkCanvas* canvas) override {
        SkScalar angle = kAngle*SK_ScalarPI + SkScalarHalf(SK_ScalarPI);
        SkScalar size = SkIntToScalar(std::min(kWidth, kHeight));
        SkPoint center = SkPoint::Make(SkScalarHalf(kWidth), SkScalarHalf(kHeight));
        SkScalar length = 5;
        SkScalar step = angle;

        SkPath path;
        path.moveTo(center);

        for (int i = 0; i < fNumSteps && length < (SkScalarHalf(size) - 10.f); ++i) {
            SkPoint rp = SkPoint::Make(length*SkScalarCos(step) + center.fX,
                                       length*SkScalarSin(step) + center.fY);
            path.lineTo(rp);
            length += angle / SkScalarHalf(SK_ScalarPI);
            step += angle;
        }

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(ToolUtils::color_to_565(0xFF007700));

        canvas->drawPath(path, paint);
    }

    bool onAnimate(double nanos) override {
        constexpr SkScalar kDesiredDurationSecs = 3.0f;

        // Make the animation ping-pong back and forth but start in the fully drawn state
        SkScalar fraction = 1.0f - TimeUtils::Scaled(1e-9 * nanos, 2.0f/kDesiredDurationSecs, 2.0f);
        if (fraction <= 0.0f) {
            fraction = -fraction;
        }

        SkASSERT(fraction >= 0.0f && fraction <= 1.0f);

        fNumSteps = (int) (fraction * kMaxNumSteps);
        return true;
    }

private:
    int fNumSteps;

    using INHERITED = GM;
};

DEF_GM( return new StringArtGM; )

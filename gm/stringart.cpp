/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

// Reproduces https://code.google.com/p/chromium/issues/detail?id=279014

static const int kWidth = 640;
static const int kHeight = 480;
static const SkScalar kAngle = 0.305f;

// Renders a string art shape.
// The particular shape rendered can be controlled by adjusting kAngle, from 0 to 1

class StringArtGM : public skiagm::GM {
public:
    StringArtGM() {}

protected:

    SkString onShortName() override {
        return SkString("stringart");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar angle = kAngle*SK_ScalarPI + SkScalarHalf(SK_ScalarPI);
        SkScalar size = SkIntToScalar(SkMin32(kWidth, kHeight));
        SkPoint center = SkPoint::Make(SkScalarHalf(kWidth), SkScalarHalf(kHeight));
        SkScalar length = 5;
        SkScalar step = angle;

        SkPath path;
        path.moveTo(center);

        while (length < (SkScalarHalf(size) - 10.f))
        {
            SkPoint rp = SkPoint::Make(length*SkScalarCos(step) + center.fX,
                                       length*SkScalarSin(step) + center.fY);
            path.lineTo(rp);
            length += angle / SkScalarHalf(SK_ScalarPI);
            step += angle;
        }
        path.close();

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(sk_tool_utils::color_to_565(0xFF007700));

        canvas->drawPath(path, paint);
    }

private:
    typedef GM INHERITED;
};

DEF_GM( return new StringArtGM; )

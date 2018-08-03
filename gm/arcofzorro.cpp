/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkBitmap.h"
#include "SkPath.h"
#include "SkRandom.h"

#include "Resources.h"

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(128, 128);
    }

    void onDraw(SkCanvas* canvas) override {

        sk_tool_utils::draw_checkerboard(canvas, SK_ColorLTGRAY, SK_ColorDKGRAY, 8);

        SkImageInfo ii = SkImageInfo::MakeN32Premul(128, 128);
        SkBitmap blueRect;
        {
            blueRect.allocPixels(ii);
            SkCanvas c1(blueRect);
            c1.clear(SK_ColorTRANSPARENT);
            SkPaint p;
            p.setColor(SK_ColorBLUE);
            c1.drawRect(SkRect::MakeXYWH(16, 40, 64, 64), p);
        }

        SkBitmap redCircle;
        {
            redCircle.allocPixels(ii);
            SkCanvas c1(redCircle);
            c1.clear(SK_ColorTRANSPARENT);
            SkPaint p;
            p.setColor(SK_ColorRED);
            c1.drawCircle(72, 40, 32, p);
        }


//        canvas->saveLayer(nullptr, nullptr);

        SkPaint p;
        canvas->drawBitmap(redCircle, 0, 0, &p);
        p.setBlendMode(SkBlendMode::kDstIn);
        canvas->drawBitmap(blueRect, 0, 0, &p);

#if 0
        SkPaint paint;
        paint.setColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAntiAlias(true);
        paint.setBlendMode(SkBlendMode::kSrc);

        // kSrcIn - works
        // kDst - works
        // kDstATop - works
        SkPath path = sk_tool_utils::make_star(SkRect::MakeXYWH(64, 64, 128, 128));
        path.setFillType(SkPath::kInverseEvenOdd_FillType);

        canvas->drawPath(path, paint);
#endif

//        canvas->restore();
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}

/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkRandom.h"

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(1000, 150);
    }

    void drawClippedRect(SkCanvas* canvas, const SkRect& r) {
        SkPaint grey;
        grey.setColor(0xFFEEEEEE);

        bool isAA = grey.isAntiAlias();

        canvas->save();

//        canvas->clipRect(r);         // non-AA

        canvas->drawRect(r, grey);     // non-AA

        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->save();

        canvas->concat(SkMatrix::MakeTrans(493.5f, 8.0f));

        canvas->drawRect(SkRect::MakeWH(201, 100), SkPaint());

        this->drawClippedRect(canvas, SkRect::MakeLTRB(0, 83, 17, 100));
        this->drawClippedRect(canvas, SkRect::MakeLTRB(17, 83, 67, 100));
        this->drawClippedRect(canvas, SkRect::MakeLTRB(67, 83, 167, 100));
        this->drawClippedRect(canvas, SkRect::MakeLTRB(167, 83, 184, 100));
        this->drawClippedRect(canvas, SkRect::MakeLTRB(184, 83, 201, 100));

        canvas->restore();
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}

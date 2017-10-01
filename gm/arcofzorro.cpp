/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkRandom.h"

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
        return SkISize::Make(1000, 1000);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorWHITE);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0x8055aaff);
        SkRect clipRect = { 0, 0, 90.5f, 120.5f };
        for (auto alias: { false, true } ) {
            canvas->save();
            canvas->clipRect(clipRect, SkClipOp::kIntersect, alias);
            canvas->drawCircle(70, 100, 60, paint);
            canvas->restore();
            canvas->save();
            canvas->clipRect(clipRect, SkClipOp::kDifference, alias);
            canvas->drawCircle(70, 100, 60, paint);
            canvas->restore();
            canvas->translate(120, 0);
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}

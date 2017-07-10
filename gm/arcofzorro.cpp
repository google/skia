/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkRandom.h"

#include "SkImage.h"
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
        return SkISize::Make(512, 512);
    }

    void onDraw(SkCanvas* canvas) override {
		sk_sp<SkImage> image = GetResourceAsImage("mandrill_256.png");

		canvas->clipRect(SkRect::MakeXYWH(1, 1, 255, 255));

		canvas->save();

		canvas->rotate(45.0f);
		canvas->clipRect(SkRect::MakeXYWH(300, -128, 128, 128));

		SkPaint paint;

		canvas->drawImageRect(image, SkIRect::MakeWH(128, 128), SkRect::MakeXYWH(300, -128, 256, 256), &paint);

		canvas->restore();
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}

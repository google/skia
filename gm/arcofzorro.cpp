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
        return SkISize::Make(200, 100);
    }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkTypeface> typeface = sk_tool_utils::emoji_typeface();
        const char* text = sk_tool_utils::emoji_sample_text();

        SkPaint paint;
        paint.setTypeface(typeface);

        canvas->drawString(text, 0, 50, paint);

//        SkUnichar uc;
//        kUTF32_TextEncoding;
        //icu
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}

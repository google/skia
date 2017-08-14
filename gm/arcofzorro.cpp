/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkRandom.h"
#include "SkUtils.h"

void get_row(SkUnichar base, SkUnichar glyphs[16]) {
//    const char* str = "\xF0\x9F\x9A\xBB" "\xF0\x9F\x9A\xBC" "\xF0\x9F\x9A\xBD";
//     SkUnichar unichar = SkUTF8_ToUnichar(str);

    for (int i = 0x0; i <= 0xF; ++i) {
        glyphs[i] = base;
        glyphs[i] |= i;
    }
}

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
        return SkISize::Make(512, 2000);
    }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkTypeface> typeface = sk_tool_utils::emoji_typeface();

        SkUnichar rows[] = {
            0x1F300,
            0x1F310,
            0x1F320,
            0x1F330,
            0x1F340,
            0x1F350,
            0x1F360,
            0x1F370,
            0x1F380,
            0x1F390,
            0x1F3A0,
            0x1F3B0,
            0x1F3C0,
            0x1F3D0,
            0x1F3E0,
            0x1F3F0,
            0x1F400,
            0x1F410,
            0x1F420,
            0x1F430,
            0x1F440,
            0x1F450,
            0x1F460,
            0x1F470,
            0x1F480,
            0x1F490,
            0x1F4A0,
            0x1F4B0,
            0x1F4C0,
            0x1F4D0,
            0x1F4E0,
            0x1F4F0,
            0x1F500,
            0x1F510,
            0x1F520,
            0x1F530,
            0x1F550,
            0x1F600,
            0x1F610,
            0x1F620,
            0x1F630,	
            0x1F640,
            0x1F680,
            0x1F690,
            0x1F6A0,
            0x1F6B0,
        };

        static const SkScalar kTextSize = 20;

        SkPaint paint;
        paint.setTypeface(typeface);
        paint.setTextSize(kTextSize);
        paint.setTextEncoding(SkPaint::kUTF32_TextEncoding);

        SkPaint::FontMetrics metrics;
        paint.getFontMetrics(&metrics);

        SkUnichar glyphs[16];

        SkScalar y = 0;
        for (size_t i = 0; i < SK_ARRAY_COUNT(rows); ++i) {
            get_row(rows[i], glyphs);

            y += -metrics.fAscent;
//            canvas->drawString((const char*) glyphs, 0, y, paint);
            canvas->drawText(glyphs, sizeof(glyphs), 0, y, paint);

            y += metrics.fDescent + metrics.fLeading;
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}

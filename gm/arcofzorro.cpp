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
        SkPaint paints[4];
        paints[0].setColor(SK_ColorGREEN);
        paints[1].setColor(SK_ColorGRAY);
        paints[2].setColor(SK_ColorGREEN);
        paints[3].setColor(SK_ColorGRAY);

        const SkRect topRow[4] = {
            {   8, 26,  58, 51 },
            {  58, 26, 133, 51 },
            { 158, 26, 258, 51 },
            { 258, 26, 383, 51 },
        };

        for (int i = 0; i < 4; ++i) {
            canvas->saveLayer(&topRow[i], nullptr);
//            canvas->saveLayer(nullptr, nullptr);
                canvas->clipRect(topRow[i], false);
                canvas->drawRect(topRow[i], paints[i]);
            canvas->restore();
        }

        const SkRect botRow[4] = {
            {   8, 80,  58, 105 },
            {  58, 80, 133, 105 },
            { 158, 80, 258, 105 },
            { 258, 80, 383, 105 },
        };

        for (int i = 0; i < 4; ++i) {
            canvas->drawRect(botRow[i], paints[i]);
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}

/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

namespace skiagm {

// Draw various width thin rects at 1/8 horizontal pixel increments
class ThinRectsGM : public GM {
public:
    ThinRectsGM() {
        this->setBGColor(0xFF000000);
    }

protected:
    SkString onShortName() override {
        return SkString("thinrects");
    }

    SkISize onISize() override {
        return SkISize::Make(240, 320);
    }

    void onDraw(SkCanvas* canvas) override {

        SkPaint white;
        white.setColor(SK_ColorWHITE);
        white.setAntiAlias(true);

        SkPaint green;
        green.setColor(SK_ColorGREEN);
        green.setAntiAlias(true);

        for (int i = 0; i < 8; ++i) {
            canvas->save();
                canvas->translate(i*0.125f, i*40.0f);
                DrawVertRects(canvas, white);

                canvas->translate(40.0f, 0.0f);
                DrawVertRects(canvas, green);
            canvas->restore();

            canvas->save();
                canvas->translate(80.0f, i*40.0f + i*0.125f);
                DrawHorizRects(canvas, white);

                canvas->translate(40.0f, 0.0f);
                DrawHorizRects(canvas, green);
            canvas->restore();

            canvas->save();
                canvas->translate(160.0f + i*0.125f,
                                  i*40.0f + i*0.125f);
                DrawSquares(canvas, white);

                canvas->translate(40.0f, 0.0f);
                DrawSquares(canvas, green);
            canvas->restore();
        }
    }

private:
    static void DrawVertRects(SkCanvas* canvas, const SkPaint& p) {
        constexpr SkRect vertRects[] = {
            { 1,  1,    5.0f, 21 }, // 4 pix wide
            { 8,  1,   10.0f, 21 }, // 2 pix wide
            { 13, 1,   14.0f, 21 }, // 1 pix wide
            { 17, 1,   17.5f, 21 }, // 1/2 pix wide
            { 21, 1,  21.25f, 21 }, // 1/4 pix wide
            { 25, 1, 25.125f, 21 }, // 1/8 pix wide
            { 29, 1,   29.0f, 21 }  // 0 pix wide
        };

        for (size_t j = 0; j < SK_ARRAY_COUNT(vertRects); ++j) {
            canvas->drawRect(vertRects[j], p);
        }
    }

    static void DrawHorizRects(SkCanvas* canvas, const SkPaint& p) {
        constexpr SkRect horizRects[] = {
            { 1, 1,  21,    5.0f }, // 4 pix high
            { 1, 8,  21,   10.0f }, // 2 pix high
            { 1, 13, 21,   14.0f }, // 1 pix high
            { 1, 17, 21,   17.5f }, // 1/2 pix high
            { 1, 21, 21,  21.25f }, // 1/4 pix high
            { 1, 25, 21, 25.125f }, // 1/8 pix high
            { 1, 29, 21,   29.0f }  // 0 pix high
        };

        for (size_t j = 0; j < SK_ARRAY_COUNT(horizRects); ++j) {
            canvas->drawRect(horizRects[j], p);
        }
    }

    static void DrawSquares(SkCanvas* canvas, const SkPaint& p) {
        constexpr SkRect squares[] = {
            { 1,  1,     5.0f,    5.0f }, // 4 pix
            { 8,  8,    10.0f,   10.0f }, // 2 pix
            { 13, 13,   14.0f,   14.0f }, // 1 pix
            { 17, 17,   17.5f,   17.5f }, // 1/2 pix
            { 21, 21,  21.25f,  21.25f }, // 1/4 pix
            { 25, 25, 25.125f, 25.125f }, // 1/8 pix
            { 29, 29,   29.0f,   29.0f }  // 0 pix
        };

        for (size_t j = 0; j < SK_ARRAY_COUNT(squares); ++j) {
            canvas->drawRect(squares[j], p);
        }
    }

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ThinRectsGM; }
static GMRegistry reg(MyFactory);

}

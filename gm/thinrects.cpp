/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"

namespace skiagm {

// Draw various width thin rects at 1/8 horizontal pixel increments
class ThinRectsGM : public GM {
public:
    ThinRectsGM(bool round) : fRound(round) {
        this->setBGColor(0xFF000000);
    }

protected:
    SkString onShortName() override {
        return SkString(fRound ? "thinroundrects" : "thinrects");
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
                this->drawVertRects(canvas, white);

                canvas->translate(40.0f, 0.0f);
                this->drawVertRects(canvas, green);
            canvas->restore();

            canvas->save();
                canvas->translate(80.0f, i*40.0f + i*0.125f);
                this->drawHorizRects(canvas, white);

                canvas->translate(40.0f, 0.0f);
                this->drawHorizRects(canvas, green);
            canvas->restore();

            canvas->save();
                canvas->translate(160.0f + i*0.125f,
                                  i*40.0f + i*0.125f);
                this->drawSquares(canvas, white);

                canvas->translate(40.0f, 0.0f);
                this->drawSquares(canvas, green);
            canvas->restore();
        }
    }

private:
    void drawVertRects(SkCanvas* canvas, const SkPaint& p) {
        constexpr SkRect vertRects[] = {
            { 1,  1,    5.0f, 21 }, // 4 pix wide
            { 8,  1,   10.0f, 21 }, // 2 pix wide
            { 13, 1,   14.0f, 21 }, // 1 pix wide
            { 17, 1,   17.5f, 21 }, // 1/2 pix wide
            { 21, 1,  21.25f, 21 }, // 1/4 pix wide
            { 25, 1, 25.125f, 21 }, // 1/8 pix wide
            { 29, 1,   29.0f, 21 }  // 0 pix wide
        };

        static constexpr SkVector radii[4] = {{1/32.f, 2/32.f}, {3/32.f, 1/32.f}, {2/32.f, 3/32.f},
                                              {1/32.f, 3/32.f}};
        SkRRect rrect;
        for (size_t j = 0; j < SK_ARRAY_COUNT(vertRects); ++j) {
            if (fRound) {
                rrect.setRectRadii(vertRects[j], radii);
                canvas->drawRRect(rrect, p);
            } else {
                canvas->drawRect(vertRects[j], p);
            }
        }
    }

    void drawHorizRects(SkCanvas* canvas, const SkPaint& p) {
        constexpr SkRect horizRects[] = {
            { 1, 1,  21,    5.0f }, // 4 pix high
            { 1, 8,  21,   10.0f }, // 2 pix high
            { 1, 13, 21,   14.0f }, // 1 pix high
            { 1, 17, 21,   17.5f }, // 1/2 pix high
            { 1, 21, 21,  21.25f }, // 1/4 pix high
            { 1, 25, 21, 25.125f }, // 1/8 pix high
            { 1, 29, 21,   29.0f }  // 0 pix high
        };

        SkRRect rrect;
        for (size_t j = 0; j < SK_ARRAY_COUNT(horizRects); ++j) {
            if (fRound) {
                rrect.setNinePatch(horizRects[j], 1/32.f, 2/32.f, 3/32.f, 4/32.f);
                canvas->drawRRect(rrect, p);
            } else {
                canvas->drawRect(horizRects[j], p);
            }
        }
    }

    void drawSquares(SkCanvas* canvas, const SkPaint& p) {
        constexpr SkRect squares[] = {
            { 1,  1,     5.0f,    5.0f }, // 4 pix
            { 8,  8,    10.0f,   10.0f }, // 2 pix
            { 13, 13,   14.0f,   14.0f }, // 1 pix
            { 17, 17,   17.5f,   17.5f }, // 1/2 pix
            { 21, 21,  21.25f,  21.25f }, // 1/4 pix
            { 25, 25, 25.125f, 25.125f }, // 1/8 pix
            { 29, 29,   29.0f,   29.0f }  // 0 pix
        };

        SkRRect rrect;
        for (size_t j = 0; j < SK_ARRAY_COUNT(squares); ++j) {
            if (fRound) {
                rrect.setRectXY(squares[j], 1/32.f, 2/32.f);
                canvas->drawRRect(rrect, p);
            } else {
                canvas->drawRect(squares[j], p);
            }
        }
    }

    const bool fRound;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ThinRectsGM(false); )
DEF_GM( return new ThinRectsGM(true); )

}  // namespace skiagm

DEF_SIMPLE_GM_CAN_FAIL(clipped_thinrect, canvas, errorMsg, 256, 256) {
    auto zoomed = canvas->makeSurface(canvas->imageInfo().makeWH(10, 10));
    if (!zoomed) {
        errorMsg->printf("makeSurface not supported");
        return skiagm::DrawResult::kSkip;
    }
    auto zoomedCanvas = zoomed->getCanvas();

    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kFill_Style);
    zoomedCanvas->save();
    zoomedCanvas->clipRect(SkRect::MakeXYWH(0, 5, 256, 10), true /*doAntialias*/);
    zoomedCanvas->drawRect(SkRect::MakeXYWH(0, 0, 100, 5.5), p);
    zoomedCanvas->restore();

    // Zoom-in. Should see one line of red representing zoomed in 1/2px coverage and *not*
    // two lines of varying coverage from hairline rendering.
    auto img = zoomed->makeImageSnapshot();
    canvas->drawImageRect(img, SkRect::MakeXYWH(0, 10, 200, 200), SkSamplingOptions());
    return skiagm::DrawResult::kOk;
}

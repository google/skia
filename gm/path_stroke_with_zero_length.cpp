/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPath.h"
#include "SkStream.h"
#include "gm.h"


// Test how short paths are stroked with various caps
class StrokeZeroGM : public skiagm::GM {
    SkPath fPaths[8];
    SkPath fClipL, fClipR, fClipS;

protected:
    void onOnceBeforeDraw() override {
        fClipL.moveTo(0, 0);
        fClipL.lineTo(3, 0);
        fClipL.lineTo(2.5f, 1);
        fClipL.lineTo(3.5f, 2.5f);
        fClipL.lineTo(2.5f, 4);
        fClipL.lineTo(3, 5);
        fClipL.lineTo(0, 5);
        fClipL.close();

        fClipR.moveTo(34, 0);
        fClipR.lineTo(34, 5);
        fClipR.lineTo(31, 5);
        fClipR.lineTo(30.5, 4);
        fClipR.lineTo(31.5, 2.5);
        fClipR.lineTo(30.5, 1);
        fClipR.lineTo(31, 0);
        fClipR.close();

        fClipS.addRect(SkRect::MakeIWH(4, 5));

        fPaths[0].moveTo(30, 0);  // single line segment
        fPaths[0].rLineTo(30, 0);

        fPaths[1].moveTo(90, 0);  // single line segment with close (does not draw caps)
        fPaths[1].rLineTo(30, 0);
        fPaths[1].close();

        fPaths[2].moveTo(150, 0);  // zero-length line
        fPaths[2].rLineTo(0, 0);

        fPaths[3].moveTo(180, 0);  // zero-length line with close (expected not to draw)
        fPaths[3].rLineTo(0, 0);
        fPaths[3].close();

        fPaths[4].moveTo(210, 0);  // close only, no line
        fPaths[4].close();

        fPaths[5].moveTo(30, 90);  // all combos below should draw two caps
        fPaths[5].rLineTo(0, 0);
        fPaths[5].moveTo(60, 90);
        fPaths[5].rLineTo(0, 0);

        fPaths[6].moveTo(90, 90);
        fPaths[6].close();
        fPaths[6].moveTo(120, 90);
        fPaths[6].close();

        fPaths[7].moveTo(150, 90);
        fPaths[7].rLineTo(0, 0);
        fPaths[7].moveTo(180, 90);
        fPaths[7].close();
    }


    SkString onShortName() override {
        return SkString("path_stroke_with_zero_length");
    }

    SkISize onISize() override {
        return SkISize::Make(1120, 840);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint bkgrnd;
        bkgrnd.setColor(SK_ColorWHITE);
        canvas->drawRect(SkRect::MakeIWH(onISize().fWidth, onISize().fHeight), bkgrnd);

         auto drawPaths = [&](SkPaint& paint, int indexMask) {
            canvas->translate(0, 30.0f);
            int index = 0;
            for (const SkPath& path : fPaths) {
                if (indexMask & (1 << index)) {
                    canvas->drawPath(path, paint);
                }
                if (paint.getStrokeWidth() < 2) {
                    drawFat(canvas, path, paint, index);
                }
                ++index;
            }
        };

        if (false) { // debugging variant that draws a single element
            SkScalar width = 0;
            bool antialias = true;

            SkPaint butt;
            butt.setAntiAlias(antialias);
            butt.setStyle(SkPaint::kStroke_Style);
            butt.setStrokeWidth(width);

            SkPaint round(butt);
            round.setStrokeCap(SkPaint::kRound_Cap);
            drawPaths(round, 1 << 7);
            return;
        }

        SkScalar widths[] = { 0, .999f, 1, 1.001f, 20 };
        bool aliases[] = { false, true };
        for (bool antialias : aliases) {
            canvas->save();
            for (SkScalar width : widths) {
                canvas->save();
                SkPaint butt;
                butt.setAntiAlias(antialias);
                butt.setStyle(SkPaint::kStroke_Style);
                butt.setStrokeWidth(width);
                drawPaths(butt, -1);

                SkPaint round(butt);
                round.setStrokeCap(SkPaint::kRound_Cap);
                drawPaths(round, -1);

                SkPaint square(butt);
                square.setStrokeCap(SkPaint::kSquare_Cap);
                drawPaths(square, -1);
                canvas->restore();
                canvas->translate(220, 0);
            }
            canvas->restore();
            canvas->translate(0, 210);
        }
    }

private:
    void drawFat(SkCanvas* canvas, const SkPath& path, const SkPaint& paint, int index) {
        const SkScalar scale = 10;
        SkRect bounds = path.getBounds();
        SkBitmap offscreen;
        offscreen.allocN32Pixels(SkScalarRoundToInt(bounds.width() + 4),
                SkScalarRoundToInt(bounds.height() + 4));
        offscreen.eraseColor(SK_ColorWHITE);
        SkScalar pathX = bounds.fLeft - 2;
        SkScalar pathY = bounds.fTop - 2;
        SkMatrix cMatrix = canvas->getTotalMatrix();
        if (!canvas->readPixels(&offscreen, SkScalarRoundToInt(pathX + cMatrix.getTranslateX()),
                SkScalarRoundToInt(pathY + cMatrix.getTranslateY()))) {
            return;
        }

        canvas->save();
        SkMatrix clipM;
        clipM.reset();
        clipM.preScale(scale, scale);
        clipM.postTranslate(bounds.fLeft - 17, bounds.fTop - 24.5f + 420);
        SkPath clip;
        if (index < 2) {
            fClipL.transform(clipM, &clip);
        } else {
            fClipS.transform(clipM, &clip);
        }
        canvas->clipPath(clip, SkRegion::kIntersect_Op, true);
        canvas->scale(scale, scale);
        canvas->drawBitmap(offscreen, (bounds.fLeft - 17) / scale,
                    (bounds.fTop - 20 + 420) / scale);
        canvas->restore();

        if (bounds.width() > 20) {
            canvas->save();
            clipM.reset();
            clipM.preScale(scale, scale);
            clipM.postTranslate(bounds.fLeft - 17 - 275, bounds.fTop - 24.5f + 420);
            SkPath clip;
            fClipR.transform(clipM, &clip);
            canvas->clipPath(clip, SkRegion::kIntersect_Op, true);
            canvas->scale(10.f, 10.f);
            canvas->drawBitmap(offscreen, (bounds.fLeft - 17 - 275
                    + (index >= 5 ? 5 : 0)) / scale, (bounds.fTop - 20 + 420) / scale);
            canvas->restore();
        }
    }

};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new StrokeZeroGM(); )


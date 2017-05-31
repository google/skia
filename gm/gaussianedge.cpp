/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColorFilter.h"
#include "SkGaussianEdgeShader.h"
#include "SkRRect.h"

//#define VIZ 1

#ifdef VIZ
#include "SkStroke.h"

static void draw_stroke(SkCanvas* canvas, const SkRRect& rr, const SkPaint& p, SkColor color) {
    SkPath output;

    if (SkPaint::kFill_Style == p.getStyle()) {
        output.addRRect(rr);
    } else {
        SkPath input;
        input.addRRect(rr);

        SkStroke stroke(p);
        stroke.strokePath(input, &output);
    }

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(color);

    canvas->drawPath(output, paint);
}

static void extract_pts(const SkBitmap& bm, SkTDArray<SkPoint>* pts,
                        int xOff, int width) {
    pts->rewind();

    for (int x = 0; x < width; ++x) {
        SkColor color = bm.getColor(xOff+x, 0);    

        pts->append()->set(SkIntToScalar(x), 255.0f-SkColorGetB(color));
        if (x > 0 && x < width-1) {
            pts->append()->set(SkIntToScalar(x), 255.0f-SkColorGetB(color));
        }
    }
}

static void draw_row(SkCanvas* canvas, int row, int width) {
    SkPaint paint;
    paint.setAntiAlias(true);

    SkBitmap readback;

    if (!canvas->readPixels(SkIRect::MakeXYWH(0, row, width, 1), &readback)) {
        return;
    }

    SkTDArray<SkPoint> pts;
    pts.setReserve(width/3);

    extract_pts(readback, &pts, 0, width/3);
    paint.setColor(SK_ColorRED);
    canvas->drawPoints(SkCanvas::kLines_PointMode, pts.count(), pts.begin(), paint);

    extract_pts(readback, &pts, width/3, width/3);
    paint.setColor(SK_ColorGREEN);
    canvas->drawPoints(SkCanvas::kLines_PointMode, pts.count(), pts.begin(), paint);

    extract_pts(readback, &pts, 2*width/3, width/3);
    paint.setColor(SK_ColorBLUE);
    canvas->drawPoints(SkCanvas::kLines_PointMode, pts.count(), pts.begin(), paint);
}
#endif

namespace skiagm {

// This GM exercises the SkGaussianEdgeShader.
// It draws three columns showing filled, stroked, and stroke and filled rendering.
// It draws three rows showing a blur radius smaller than, equal to
// and, finally, double the RRect's corner radius
// In VIZ mode an extra column is drawn showing the blur ramps (they should all line up).
class GaussianEdgeGM : public GM {
public:
    GaussianEdgeGM() {
        this->setBGColor(SK_ColorWHITE);
    }

protected:

    SkString onShortName() override {
        return SkString("gaussianedge");
    }

    SkISize onISize() override {
        int numCols = kNumBaseCols;
#ifdef VIZ
        numCols++;
#endif

        return SkISize::Make(kPad + numCols*(kCellWidth+kPad),
                             kPad + kNumRows*(kCellWidth+kPad));
    }

    static void DrawRow(SkCanvas* canvas, int blurRad, int midLine) {
        SkAutoCanvasRestore acr(canvas, true);

        SkRRect rrects[kNumBaseCols];
        SkPaint paints[kNumBaseCols];

        {
            const SkRect r = SkRect::MakeIWH(kRRSize, kRRSize);
            const SkRRect baseRR = SkRRect::MakeRectXY(r,
                                                       SkIntToScalar(kRRRad),
                                                       SkIntToScalar(kRRRad));

            SkPaint basePaint;
            basePaint.setAntiAlias(true);
            basePaint.setColor(SkColorSetARGB(255, (4 * blurRad) >> 8, (4 * blurRad) & 0xff, 0));
            basePaint.setShader(SkGaussianEdgeShader::Make());
            basePaint.setColorFilter(SkColorFilter::MakeModeFilter(SK_ColorRED,
                                                                   SkBlendMode::kModulate));

            //----
            paints[0] = basePaint;
            rrects[0] = baseRR;

            //----
            paints[1] = basePaint;
            paints[1].setStyle(SkPaint::kStroke_Style);

            rrects[1] = baseRR;
            if (blurRad/2.0f < kRRRad) {
                rrects[1].inset(blurRad/2.0f, blurRad/2.0f);
                paints[1].setStrokeWidth(SkIntToScalar(blurRad));
            } else {
                SkScalar inset = kRRRad - 0.5f;
                rrects[1].inset(inset, inset);
                SkScalar pad = blurRad/2.0f - inset;
                paints[1].setStrokeWidth(blurRad + 2.0f * pad);                
                paints[1].setColor(SkColorSetARGB(255, (4 * blurRad) >> 8, (4 * blurRad) & 0xff,
                                                  (int)(4.0f*pad)));
            }

            //----
            paints[2] = basePaint;
            paints[2].setStyle(SkPaint::kStrokeAndFill_Style);

            rrects[2] = baseRR;
            if (blurRad/2.0f < kRRRad) {
                rrects[2].inset(blurRad/2.0f, blurRad/2.0f);
                paints[2].setStrokeWidth(SkIntToScalar(blurRad));
            } else {
                SkScalar inset = kRRRad - 0.5f;
                rrects[2].inset(inset, inset);
                SkScalar pad = blurRad/2.0f - inset;
                paints[2].setStrokeWidth(blurRad + 2.0f * pad);                
                paints[2].setColor(SkColorSetARGB(255, (4 * blurRad) >> 8, (4 * blurRad) & 0xff,
                                                  (int)(4.0f*pad)));
            }
        }

        //----
        canvas->save();
            // draw the shadows
            for (int i = 0; i < kNumBaseCols; ++i) {
                canvas->drawRRect(rrects[i], paints[i]);
                canvas->translate(SkIntToScalar(kCellWidth+kPad), 0.0f);
            }

#ifdef VIZ
            // draw the visualization of the shadow ramps
            draw_row(canvas, midLine, 3*(kRRSize+kPad));
#endif
        canvas->restore();

#ifdef VIZ
        const SkColor colors[kNumBaseCols] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };

        // circle back and draw the stroked geometry (they would mess up the viz otherwise)
        for (int i = 0; i < kNumBaseCols; ++i) {
            draw_stroke(canvas, rrects[i], paints[i], colors[i]);
            canvas->translate(SkIntToScalar(kCellWidth+kPad), 0.0f);
        }
#endif
    }

    void onDraw(SkCanvas* canvas) override {
        GrRenderTargetContext* renderTargetContext =
            canvas->internal_private_accessTopLayerRenderTargetContext();
        if (!renderTargetContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        const int blurRadii[kNumRows] = { kRRRad/2, kRRRad, 2*kRRRad };

        canvas->translate(SkIntToScalar(kPad), SkIntToScalar(kPad));
        for (int i = 0; i < kNumRows; ++i) {
            DrawRow(canvas, blurRadii[i], kPad+(i*kRRSize)+kRRSize/2);
            canvas->translate(0.0f, SkIntToScalar(kCellWidth+kPad));
        }
    }

private:
    static const int kNumRows = 3;
    static const int kNumBaseCols = 3;
    static const int kPad = 5;
    static const int kRRSize = 256;
    static const int kRRRad = 64;
    static const int kCellWidth = kRRSize;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new GaussianEdgeGM;)
}

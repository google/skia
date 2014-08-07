
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrTest.h"
#include "SkPatch.h"

static void draw_control_points(SkCanvas* canvas, const SkPatch& patch) {
    //draw control points
    SkPaint paint;
    SkPoint bottom[SkPatch::kNumPtsCubic];
    patch.getBottomPoints(bottom);
    SkPoint top[SkPatch::kNumPtsCubic];
    patch.getTopPoints(top);
    SkPoint left[SkPatch::kNumPtsCubic];
    patch.getLeftPoints(left);
    SkPoint right[SkPatch::kNumPtsCubic];
    patch.getRightPoints(right);

    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(0.5);
    SkPoint corners[4] = { bottom[0], bottom[3], top[0], top[3] };
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, bottom, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, bottom+1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, top, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, left, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, right, paint);

    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, top+1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, left+1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, right+1, paint);

    paint.setStrokeWidth(2);

    paint.setColor(SK_ColorRED);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 4, corners, paint);

    paint.setColor(SK_ColorBLUE);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, bottom+1, paint);

    paint.setColor(SK_ColorCYAN);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, top+1, paint);

    paint.setColor(SK_ColorYELLOW);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, left+1, paint);

    paint.setColor(SK_ColorGREEN);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, right+1, paint);
}

namespace skiagm {
/**
 * This GM draws a SkPatch.
 */
class SkPatchGM : public GM {
    
public:
    SkPatchGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("patch_primitive");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(800, 800);
    }

    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        return kSkipTiled_Flag;
    }
    
    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        
        SkPaint paint;
        
        // The order of the colors and points is clockwise starting at upper-left corner.
        SkColor colors[SkPatch::kNumColors] = {
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorCYAN
        };
        SkPoint points[SkPatch::kNumCtrlPts] = {
            //top points
            {50,50},{75,20},{125,80}, {150,50},
            //right points
            {120,75},{180,125},
            //bottom points
            {150,150},{125,120},{75,180},{50,150},
            //left points
            {20,125},{80,75}
        };
        
        SkPatch patch(points, colors);
        static const SkScalar kScale = 0.5f;
        canvas->translate(100, 100);
        canvas->save();
        for (SkScalar x = 0; x < 4; x++) {
            canvas->save();
            canvas->scale(kScale * (x + 1), kScale * (x + 1));
            canvas->translate(x * 100, 0);
            canvas->drawPatch(patch, paint);
            draw_control_points(canvas, patch);
            canvas->restore();
        }
        
        canvas->translate(0, 270);
        
        static const SkScalar kSkew = 0.2f;
        for (SkScalar x = 0; x < 4; x++) {
            canvas->save();
            canvas->scale(kScale * (x + 1), kScale * (x + 1));
            canvas->translate(x * 100, 0);
            canvas->skew(kSkew * (x + 1), kSkew * (x + 1));
            canvas->drawPatch(patch, paint);
            draw_control_points(canvas, patch);
            canvas->restore();
        }
        canvas->restore();
    }

private:
    typedef GM INHERITED;
};

DEF_GM(return SkNEW(SkPatchGM); )

}

#endif

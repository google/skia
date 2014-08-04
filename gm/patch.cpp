
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

static void draw_control_points(SkCanvas* canvas, SkPatch& patch, SkPaint& paint) {
    //draw control points
    SkPaint copy(paint);
    SkPoint bottom[4];
    patch.getBottomPoints(bottom);
    SkPoint top[4];
    patch.getTopPoints(top);
    SkPoint left[4];
    patch.getLeftPoints(left);
    SkPoint right[4];
    patch.getRightPoints(right);

    copy.setColor(SK_ColorBLACK);
    copy.setStrokeWidth(0.5);
    SkPoint corners[4] = { bottom[0], bottom[3], top[0], top[3] };
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, bottom, copy);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, bottom+1, copy);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, top, copy);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, left, copy);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, right, copy);

    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, top+1, copy);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, left+1, copy);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, right+1, copy);

    copy.setStrokeWidth(2);

    copy.setColor(SK_ColorRED);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 4, corners, copy);

    copy.setColor(SK_ColorBLUE);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, bottom+1, copy);

    copy.setColor(SK_ColorCYAN);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, top+1, copy);

    copy.setColor(SK_ColorYELLOW);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, left+1, copy);

    copy.setColor(SK_ColorGREEN);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, right+1, copy);
}

static void draw_random_patch(SkPoint points[12], SkColor colors[4], SkCanvas* canvas,
                              SkPaint& paint, SkRandom* rnd) {
    SkPoint ptsCpy[12];
    memcpy(ptsCpy, points, 12 * sizeof(SkPoint));
    for (int i = 0; i < 5; i++) {
        int index = rnd->nextRangeU(0, 11);
        SkScalar dx = rnd->nextRangeScalar(-50, 50), dy = rnd->nextRangeScalar(-50, 50);
        ptsCpy[index].offset(dx, dy);
    }
    SkPatch patch(ptsCpy, colors);
    canvas->drawPatch(patch, paint);
    draw_control_points(canvas, patch, paint);
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
        return kSkipTiled_Flag | kSkipPipe_Flag | kSkipPicture_Flag;
    }
    
    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        
        SkPaint paint;
        SkColor colors[4] = {
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorCYAN
        };
        SkPoint points[12] = {
            {50,50},{75,20},{125,80}, {150,50},
            {120,75},{180,125},{150,150},
            {125,120},{75,180},{50,150},
            {20,125},{80,75}
        };
        
        SkRandom rnd;
        SkScalar scale = 0.5f;
        canvas->save();
        for (SkScalar x = 0; x < 4; x++) {
            canvas->save();
            canvas->scale(scale * (x + 1), scale * (x + 1));
            canvas->translate(x * 100, 0);
            draw_random_patch(points, colors, canvas, paint, &rnd);
            canvas->restore();
        }
        canvas->translate(0, 270);
        SkScalar skew = 0.1f;
        for (SkScalar x = 0; x < 4; x++) {
            canvas->save();
            canvas->scale(scale * (x + 1), scale * (x + 1));
            canvas->skew(skew * (x + 1), skew * (x + 1));
            canvas->translate(x * 100, 0);
            draw_random_patch(points, colors, canvas, paint, &rnd);
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


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
        return kGPUOnly_Flag;
    }


    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {

        SkPaint paint;
        SkColor colors[4] = {
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorCYAN
        };
        SkPoint points[] = {
            {100,100},{130,50},{500,70}, {650,60},
            {350,125},{490,555},{600,700},
            {515,595},{140,550},{110,590},
            {125,400},{70,150}
            
        };

        SkPatch coons(points, colors);
        
        SkPatch::VertexData data;
        coons.getVertexData(&data, 10);

        canvas->drawVertices(SkCanvas::kTriangles_VertexMode,data.fVertexCount,
            data.fPoints, data.fTexCoords, data.fColors, NULL, data.fIndices,
            data.fIndexCount, paint);

        //draw control points
        SkPoint bottom[4];
        coons.getBottomPoints(bottom);
        SkPoint top[4];
        coons.getTopPoints(top);
        SkPoint left[4];
        coons.getLeftPoints(left);
        SkPoint right[4];
        coons.getRightPoints(right);

        SkPoint corners[4] = { bottom[0], bottom[3], top[0], top[3] };
        canvas->drawPoints(SkCanvas::kLines_PointMode, 4, bottom, paint);
        canvas->drawPoints(SkCanvas::kLines_PointMode, 2, bottom+1, paint);
        canvas->drawPoints(SkCanvas::kLines_PointMode, 4, top, paint);
        canvas->drawPoints(SkCanvas::kLines_PointMode, 4, left, paint);
        canvas->drawPoints(SkCanvas::kLines_PointMode, 4, right, paint);

        canvas->drawPoints(SkCanvas::kLines_PointMode, 2, top+1, paint);
        canvas->drawPoints(SkCanvas::kLines_PointMode, 2, left+1, paint);
        canvas->drawPoints(SkCanvas::kLines_PointMode, 2, right+1, paint);

        paint.setStrokeWidth(10);

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

private:
    typedef GM INHERITED;
};

DEF_GM( return SkNEW(SkPatchGM); )

}

#endif

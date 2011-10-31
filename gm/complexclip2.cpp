/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkRandom.h"

namespace skiagm {

class ComplexClip2GM : public GM {
public:
    ComplexClip2GM() {
        this->setBGColor(SkColorSetRGB(0xDD,0xA0,0xDD));
        
        SkScalar xA =  0 * SK_Scalar1;
        SkScalar xB = 10 * SK_Scalar1;
        SkScalar xC = 20 * SK_Scalar1;
        SkScalar xD = 30 * SK_Scalar1;
        SkScalar xE = 40 * SK_Scalar1;
        SkScalar xF = 50 * SK_Scalar1;

        SkScalar yA =  0 * SK_Scalar1;
        SkScalar yB = 10 * SK_Scalar1;
        SkScalar yC = 20 * SK_Scalar1;
        SkScalar yD = 30 * SK_Scalar1;
        SkScalar yE = 40 * SK_Scalar1;
        SkScalar yF = 50 * SK_Scalar1;

        fWidth = xF - xA;
        fHeight = yF - yA;

        fRects[0].set(xB, yB, xE, yE);
        fRectColors[0] = SK_ColorRED;

        fRects[1].set(xA, yA, xD, yD);
        fRectColors[1] = SK_ColorGREEN;

        fRects[2].set(xC, yA, xF, yD);
        fRectColors[2] = SK_ColorBLUE;

        fRects[3].set(xA, yC, xD, yF);
        fRectColors[3] = SK_ColorYELLOW;

        fRects[4].set(xC, yC, xF, yF);
        fRectColors[4] = SK_ColorCYAN;

        fTotalWidth = kCols * fWidth + SK_Scalar1 * (kCols + 1) * kPadX;
        fTotalHeight = kRows * fHeight + SK_Scalar1 * (kRows + 1) * kPadY;

        SkRegion::Op ops[] = {
            SkRegion::kDifference_Op,
            SkRegion::kIntersect_Op,
            SkRegion::kUnion_Op,
            SkRegion::kXOR_Op,
            SkRegion::kReverseDifference_Op,
            SkRegion::kReplace_Op,
        };

        SkRandom r;
        for (int i = 0; i < kRows; ++i) {
            for (int j = 0; j < kCols; ++j) {
                for (int k = 0; k < 5; ++k) {
                    fOps[j*kRows+i][k] = ops[r.nextU() % SK_ARRAY_COUNT(ops)];
                }
            }
        }
    }

protected:

    static const int kRows = 5;
    static const int kCols = 5;
    static const int kPadX = 20;
    static const int kPadY = 20;

    virtual SkString onShortName() {
        return SkString("complexclip2");
    }

    virtual SkISize onISize() {
        return make_isize(SkScalarRoundToInt(fTotalWidth),
                          SkScalarRoundToInt(fTotalHeight));
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint rectPaint;
        rectPaint.setStyle(SkPaint::kStroke_Style);
        rectPaint.setStrokeWidth(-1);

        SkPaint fillPaint;
        fillPaint.setColor(SkColorSetRGB(0xA0,0xDD,0xA0));

        for (int i = 0; i < kRows; ++i) {
            for (int j = 0; j < kCols; ++j) {
                canvas->save();
                canvas->translate(kPadX * SK_Scalar1 + (fWidth + kPadX * SK_Scalar1)*j,
                                  kPadY * SK_Scalar1 + (fHeight + kPadY * SK_Scalar1)*i);
                canvas->save();
                for (int k = 0; k < 5; ++k) {
                    canvas->clipRect(fRects[k], fOps[j*kRows+i][k]);
                }
                canvas->drawRect(SkRect::MakeWH(fWidth, fHeight), fillPaint);
                canvas->restore();
                for (int k = 0; k < 5; ++k) {
                    rectPaint.setColor(fRectColors[k]);
                    canvas->drawRect(fRects[k], rectPaint);
                }
                canvas->restore();
            }
        }
    }
private:
    SkRect fRects[5];
    SkColor fRectColors[5];
    SkRegion::Op fOps[kRows * kCols][5];
    SkScalar fWidth;
    SkScalar fHeight;
    SkScalar fTotalWidth;
    SkScalar fTotalHeight;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ComplexClip2GM; }
static GMRegistry reg(MyFactory);

}

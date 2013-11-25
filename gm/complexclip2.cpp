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
    ComplexClip2GM(bool doPaths, bool antiAlias)
    : fDoPaths(doPaths)
    , fAntiAlias(antiAlias) {
        this->setBGColor(SkColorSetRGB(0xDD,0xA0,0xDD));

        // offset the rects a bit so we get antialiasing even in the rect case
        SkScalar xA = 0.65f;
        SkScalar xB = 10.65f;
        SkScalar xC = 20.65f;
        SkScalar xD = 30.65f;
        SkScalar xE = 40.65f;
        SkScalar xF = 50.65f;

        SkScalar yA = 0.65f;
        SkScalar yB = 10.65f;
        SkScalar yC = 20.65f;
        SkScalar yD = 30.65f;
        SkScalar yE = 40.65f;
        SkScalar yF = 50.65f;

        fWidth = xF - xA;
        fHeight = yF - yA;

        fRects[0].set(xB, yB, xE, yE);
        fPaths[0].addRoundRect(fRects[0], SkIntToScalar(5), SkIntToScalar(5));
        fRectColors[0] = SK_ColorRED;

        fRects[1].set(xA, yA, xD, yD);
        fPaths[1].addRoundRect(fRects[1], SkIntToScalar(5), SkIntToScalar(5));
        fRectColors[1] = SK_ColorGREEN;

        fRects[2].set(xC, yA, xF, yD);
        fPaths[2].addRoundRect(fRects[2], SkIntToScalar(5), SkIntToScalar(5));
        fRectColors[2] = SK_ColorBLUE;

        fRects[3].set(xA, yC, xD, yF);
        fPaths[3].addRoundRect(fRects[3], SkIntToScalar(5), SkIntToScalar(5));
        fRectColors[3] = SK_ColorYELLOW;

        fRects[4].set(xC, yC, xF, yF);
        fPaths[4].addRoundRect(fRects[4], SkIntToScalar(5), SkIntToScalar(5));
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

        SkLCGRandom r;
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
        if (!fDoPaths && !fAntiAlias) {
            return SkString("complexclip2");
        }

        SkString str;
        str.printf("complexclip2_%s_%s",
                    fDoPaths ? "path" : "rect",
                    fAntiAlias ? "aa" : "bw");
        return str;
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

                // draw the original shapes first so we can see the
                // antialiasing on the clipped draw
                for (int k = 0; k < 5; ++k) {
                    rectPaint.setColor(fRectColors[k]);
                    if (fDoPaths) {
                        canvas->drawPath(fPaths[k], rectPaint);
                    } else {
                        canvas->drawRect(fRects[k], rectPaint);
                    }
                }

                for (int k = 0; k < 5; ++k) {
                    if (fDoPaths) {
                        canvas->clipPath(fPaths[k],
                                         fOps[j*kRows+i][k],
                                         fAntiAlias);
                    } else {
                        canvas->clipRect(fRects[k],
                                         fOps[j*kRows+i][k],
                                         fAntiAlias);
                    }
                }
                canvas->drawRect(SkRect::MakeWH(fWidth, fHeight), fillPaint);
                canvas->restore();
            }
        }
    }
private:
    bool fDoPaths;
    bool fAntiAlias;
    SkRect fRects[5];
    SkPath fPaths[5];
    SkColor fRectColors[5];
    SkRegion::Op fOps[kRows * kCols][5];
    SkScalar fWidth;
    SkScalar fHeight;
    SkScalar fTotalWidth;
    SkScalar fTotalHeight;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

// bw rects
static GM* MyFactory(void*) { return new ComplexClip2GM(false, false); }
static GMRegistry reg(MyFactory);

// bw paths
static GM* MyFactory2(void*) { return new ComplexClip2GM(true, false); }
static GMRegistry reg2(MyFactory2);

// aa rects
static GM* MyFactory3(void*) { return new ComplexClip2GM(false, true); }
static GMRegistry reg3(MyFactory3);

// aa paths
static GM* MyFactory4(void*) { return new ComplexClip2GM(true, true); }
static GMRegistry reg4(MyFactory4);

}

/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/utils/SkRandom.h"

namespace skiagm {

class ComplexClip2GM : public GM {
public:
    enum Clip {
        kRect_Clip,
        kRRect_Clip,
        kPath_Clip
    };

    ComplexClip2GM(Clip clip, bool antiAlias)
    : fClip(clip)
    , fAntiAlias(antiAlias) {
        SkScalar xA = 0.65f;
        SkScalar xF = 50.65f;

        SkScalar yA = 0.65f;
        SkScalar yF = 50.65f;

        fWidth = xF - xA;
        fHeight = yF - yA;

        fTotalWidth = kCols * fWidth + SK_Scalar1 * (kCols + 1) * kPadX;
        fTotalHeight = kRows * fHeight + SK_Scalar1 * (kRows + 1) * kPadY;
    }

protected:
    void onOnceBeforeDraw() override {
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

        fRects[0].setLTRB(xB, yB, xE, yE);
        fRRects[0].setRectXY(fRects[0], 7, 7);
        fPaths[0] = SkPath::RRect(fRects[0], 5, 5);
        fRectColors[0] = SK_ColorRED;

        fRects[1].setLTRB(xA, yA, xD, yD);
        fRRects[1].setRectXY(fRects[1], 7, 7);
        fPaths[1] = SkPath::RRect(fRects[1], 5, 5);
        fRectColors[1] = SK_ColorGREEN;

        fRects[2].setLTRB(xC, yA, xF, yD);
        fRRects[2].setRectXY(fRects[2], 7, 7);
        fPaths[2] = SkPath::RRect(fRects[2], 5, 5);
        fRectColors[2] = SK_ColorBLUE;

        fRects[3].setLTRB(xA, yC, xD, yF);
        fRRects[3].setRectXY(fRects[3], 7, 7);
        fPaths[3] = SkPath::RRect(fRects[3], 5, 5);
        fRectColors[3] = SK_ColorYELLOW;

        fRects[4].setLTRB(xC, yC, xF, yF);
        fRRects[4].setRectXY(fRects[4], 7, 7);
        fPaths[4] = SkPath::RRect(fRects[4], 5, 5);
        fRectColors[4] = SK_ColorCYAN;

        const SkClipOp ops[] = {
            SkClipOp::kDifference,
            SkClipOp::kIntersect,
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

    inline static constexpr int kRows = 5;
    inline static constexpr int kCols = 5;
    inline static constexpr int kPadX = 20;
    inline static constexpr int kPadY = 20;

    static const char* ClipStr(Clip clip) {
        switch (clip) {
        case kRect_Clip:
            return "rect";
        case kRRect_Clip:
            return "rrect";
        case kPath_Clip:
            return "path";
        }
        SkDEBUGFAIL("Unknown clip type.");
        return "";
    }

    SkString onShortName() override {
        if (kRect_Clip == fClip && !fAntiAlias) {
            return SkString("complexclip2");
        }

        SkString str;
        str.printf("complexclip2_%s_%s",
                    ClipStr(fClip),
                    fAntiAlias ? "aa" : "bw");
        return str;
    }

    SkISize onISize() override {
        return SkISize::Make(SkScalarRoundToInt(fTotalWidth),
                             SkScalarRoundToInt(fTotalHeight));
    }

    void onDraw(SkCanvas* canvas) override {
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
                    switch (fClip) {
                        case kRect_Clip:
                            canvas->drawRect(fRects[k], rectPaint);
                            break;
                        case kRRect_Clip:
                            canvas->drawRRect(fRRects[k], rectPaint);
                            break;
                        case kPath_Clip:
                            canvas->drawPath(fPaths[k], rectPaint);
                            break;
                    }
                }

                for (int k = 0; k < 5; ++k) {
                    switch (fClip) {
                        case kRect_Clip:
                            canvas->clipRect(fRects[k],
                                             fOps[j*kRows+i][k],
                                             fAntiAlias);
                            break;
                        case kRRect_Clip:
                            canvas->clipRRect(fRRects[k],
                                              fOps[j*kRows+i][k],
                                              fAntiAlias);
                            break;
                        case kPath_Clip:
                            canvas->clipPath(fPaths[k],
                                             fOps[j*kRows+i][k],
                                             fAntiAlias);
                            break;
                    }
                }
                canvas->drawRect(SkRect::MakeWH(fWidth, fHeight), fillPaint);
                canvas->restore();
            }
        }
    }
private:
    Clip fClip;
    bool fAntiAlias;
    SkRect fRects[5];
    SkRRect fRRects[5];
    SkPath fPaths[5];
    SkColor fRectColors[5];
    SkClipOp fOps[kRows * kCols][5];
    SkScalar fWidth;
    SkScalar fHeight;
    SkScalar fTotalWidth;
    SkScalar fTotalHeight;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

// bw
DEF_GM( return new ComplexClip2GM(ComplexClip2GM::kRect_Clip, false); )
DEF_GM( return new ComplexClip2GM(ComplexClip2GM::kRRect_Clip, false); )
DEF_GM( return new ComplexClip2GM(ComplexClip2GM::kPath_Clip, false); )

// aa
DEF_GM( return new ComplexClip2GM(ComplexClip2GM::kRect_Clip, true); )
DEF_GM( return new ComplexClip2GM(ComplexClip2GM::kRRect_Clip, true); )
DEF_GM( return new ComplexClip2GM(ComplexClip2GM::kPath_Clip, true); )

}  // namespace skiagm

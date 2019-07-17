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
#include "src/core/SkClipOpPriv.h"

namespace {

class ComplexClip2GM : public skiagm::GM {
    static constexpr SkScalar xA = 0.65f;
    static constexpr SkScalar xB = 10.65f;
    static constexpr SkScalar xC = 20.65f;
    static constexpr SkScalar xD = 30.65f;
    static constexpr SkScalar xE = 40.65f;
    static constexpr SkScalar xF = 50.65f;

    static constexpr SkScalar yA = 0.65f;
    static constexpr SkScalar yB = 10.65f;
    static constexpr SkScalar yC = 20.65f;
    static constexpr SkScalar yD = 30.65f;
    static constexpr SkScalar yE = 40.65f;
    static constexpr SkScalar yF = 50.65f;

    static constexpr int fWidth  = (int)(xF - xA);
    static constexpr int fHeight = (int)(yF - yA);

    static constexpr int kRows = 5;
    static constexpr int kCols = 5;
    static constexpr int kPadX = 20;
    static constexpr int kPadY = 20;
    static constexpr int fTotalWidth  = kCols * fWidth  + (kCols + 1) * kPadX;
    static constexpr int fTotalHeight = kRows * fHeight + (kRows + 1) * kPadY;

public:
    enum Clip {
        kRect_Clip,
        kRRect_Clip,
        kPath_Clip
    };

    ComplexClip2GM(Clip c, bool a, const char* n) : fClip(c), fAntiAlias(a), fName(n) {}

private:
    void onOnceBeforeDraw() override {
        this->setBGColor(SkColorSetRGB(0xDD,0xA0,0xDD));

        // offset the rects a bit so we get antialiasing even in the rect case
        fRects[0].set(xB, yB, xE, yE);
        fRRects[0].setRectXY(fRects[0], 7, 7);
        fPaths[0].addRoundRect(fRects[0], 5, 5);
        fRectColors[0] = SK_ColorRED;

        fRects[1].set(xA, yA, xD, yD);
        fRRects[1].setRectXY(fRects[1], 7, 7);
        fPaths[1].addRoundRect(fRects[1], 5, 5);
        fRectColors[1] = SK_ColorGREEN;

        fRects[2].set(xC, yA, xF, yD);
        fRRects[2].setRectXY(fRects[2], 7, 7);
        fPaths[2].addRoundRect(fRects[2], 5, 5);
        fRectColors[2] = SK_ColorBLUE;

        fRects[3].set(xA, yC, xD, yF);
        fRRects[3].setRectXY(fRects[3], 7, 7);
        fPaths[3].addRoundRect(fRects[3], 5, 5);
        fRectColors[3] = SK_ColorYELLOW;

        fRects[4].set(xC, yC, xF, yF);
        fRRects[4].setRectXY(fRects[4], 7, 7);
        fPaths[4].addRoundRect(fRects[4], 5, 5);
        fRectColors[4] = SK_ColorCYAN;

        const SkClipOp ops[] = {
            kDifference_SkClipOp,
            kIntersect_SkClipOp,
            kUnion_SkClipOp,
            kXOR_SkClipOp,
            kReverseDifference_SkClipOp,
            kReplace_SkClipOp,
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

    SkString onShortName() override { return SkASSERT(fName), SkString(fName); }

    SkISize onISize() override { return {fTotalWidth, fTotalHeight}; }

    void onDraw(SkCanvas* canvas) override {
        SkPaint rectPaint;
        rectPaint.setStyle(SkPaint::kStroke_Style);
        rectPaint.setStrokeWidth(-1);

        SkPaint fillPaint;
        fillPaint.setColor(SkColorSetRGB(0xA0,0xDD,0xA0));

        for (int i = 0; i < kRows; ++i) {
            for (int j = 0; j < kCols; ++j) {
                canvas->save();

                canvas->translate((float)(kPadX + (fWidth  + kPadX) * j),
                                  (float)(kPadY + (fHeight + kPadY) * i));

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
                canvas->drawRect(SkRect{0, 0, (float)fWidth, (float)fHeight}, fillPaint);
                canvas->restore();
            }
        }
    }

    Clip fClip;
    bool fAntiAlias;
    const char* fName = nullptr;
    SkRect fRects[5];
    SkRRect fRRects[5];
    SkPath fPaths[5];
    SkColor fRectColors[5];
    SkClipOp fOps[kRows * kCols][5];
};
}  // namespace

// bw
DEF_GM( return new ComplexClip2GM(ComplexClip2GM::kRect_Clip,  false, "complexclip2"         ); )
DEF_GM( return new ComplexClip2GM(ComplexClip2GM::kRRect_Clip, false, "complexclip2_rrect_bw"); )
DEF_GM( return new ComplexClip2GM(ComplexClip2GM::kPath_Clip,  false, "complexclip2_path_bw" ); )

// aa
DEF_GM( return new ComplexClip2GM(ComplexClip2GM::kRect_Clip,  true,  "complexclip2_rect_aa" ); )
DEF_GM( return new ComplexClip2GM(ComplexClip2GM::kRRect_Clip, true,  "complexclip2_rrect_aa"); )
DEF_GM( return new ComplexClip2GM(ComplexClip2GM::kPath_Clip,  true,  "complexclip2_path_aa" ); )

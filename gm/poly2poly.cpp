/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/utils/SkTextUtils.h"
#include "tools/Resources.h"

#include <stdint.h>

class Poly2PolyGM : public skiagm::GM {
public:
    Poly2PolyGM() {}

protected:

    SkString onShortName() override {
        return SkString("poly2poly");
    }

    SkISize onISize() override {
        return SkISize::Make(835, 840);
    }

    static void doDraw(SkCanvas* canvas, const SkFont& font, SkPaint* paint, const int isrc[],
                       const int idst[], int count) {
        SkMatrix matrix;
        SkPoint src[4], dst[4];

        for (int i = 0; i < count; i++) {
            src[i].set(SkIntToScalar(isrc[2*i+0]), SkIntToScalar(isrc[2*i+1]));
            dst[i].set(SkIntToScalar(idst[2*i+0]), SkIntToScalar(idst[2*i+1]));
        }

        canvas->save();
        matrix.setPolyToPoly(src, dst, count);
        canvas->concat(matrix);

        paint->setColor(SK_ColorGRAY);
        paint->setStyle(SkPaint::kStroke_Style);
        const SkScalar D = 64;
        canvas->drawRect(SkRect::MakeWH(D, D), *paint);
        canvas->drawLine(0, 0, D, D, *paint);
        canvas->drawLine(0, D, D, 0, *paint);

        SkFontMetrics fm;
        font.getMetrics(&fm);
        paint->setColor(SK_ColorRED);
        paint->setStyle(SkPaint::kFill_Style);
        SkScalar x = D/2;
        SkScalar y = D/2 - (fm.fAscent + fm.fDescent)/2;
        uint16_t glyphID = 3; // X
        SkTextUtils::Draw(canvas, &glyphID, sizeof(glyphID), SkTextEncoding::kGlyphID, x, y,
                          font, *paint, SkTextUtils::kCenter_Align);
        canvas->restore();
    }

    void onOnceBeforeDraw() override {
        fEmFace = MakeResourceAsTypeface("fonts/Em.ttf");
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStrokeWidth(SkIntToScalar(4));
        SkFont font(fEmFace, 40);

        canvas->save();
        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));
        // translate (1 point)
        const int src1[] = { 0, 0 };
        const int dst1[] = { 5, 5 };
        doDraw(canvas, font, &paint, src1, dst1, 1);
        canvas->restore();

        canvas->save();
        canvas->translate(SkIntToScalar(160), SkIntToScalar(10));
        // rotate/uniform-scale (2 points)
        const int src2[] = { 32, 32, 64, 32 };
        const int dst2[] = { 32, 32, 64, 48 };
        doDraw(canvas, font, &paint, src2, dst2, 2);
        canvas->restore();

        canvas->save();
        canvas->translate(SkIntToScalar(10), SkIntToScalar(110));
        // rotate/skew (3 points)
        const int src3[] = { 0, 0, 64, 0, 0, 64 };
        const int dst3[] = { 0, 0, 96, 0, 24, 64 };
        doDraw(canvas, font, &paint, src3, dst3, 3);
        canvas->restore();

        canvas->save();
        canvas->translate(SkIntToScalar(160), SkIntToScalar(110));
        // perspective (4 points)
        const int src4[] = { 0, 0, 64, 0, 64, 64, 0, 64 };
        const int dst4[] = { 0, 0, 96, 0, 64, 96, 0, 64 };
        doDraw(canvas, font, &paint, src4, dst4, 4);
        canvas->restore();
    }

private:
    typedef skiagm::GM INHERITED;
    sk_sp<SkTypeface> fEmFace;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new Poly2PolyGM; )

/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkString.h"
#include "include/core/SkTime.h"
#include "include/utils/SkRandom.h"
#include "include/utils/SkTextUtils.h"
#include "samplecode/Sample.h"

class PolyToPolyView : public Sample {
public:
    PolyToPolyView() {
        // tests
        {
            SkPoint src[] = { { 0, 0 },
                              { SK_Scalar1, 0 },
                              { 0, SK_Scalar1 } };
            SkPoint dst[] = { { 0, 0 },
                              { 2*SK_Scalar1, 0 },
                              { 0, 2*SK_Scalar1 } };
            SkMatrix m1, m2;

            (void) m1.setPolyToPoly(src, dst, 3);

            m2.reset();
            m2.set(SkMatrix::kMScaleX, dst[1].fX - dst[0].fX);
            m2.set(SkMatrix::kMSkewX,  dst[2].fX - dst[0].fX);
            m2.set(SkMatrix::kMTransX, dst[0].fX);
            m2.set(SkMatrix::kMSkewY,  dst[1].fY - dst[0].fY);
            m2.set(SkMatrix::kMScaleY, dst[2].fY - dst[0].fY);
            m2.set(SkMatrix::kMTransY, dst[0].fY);

            m1.reset();

            const SkScalar src1[] = {
                0, 0, 0, 427, 316, 427, 316, 0
            };
            const SkScalar dst1[] = {
                158, 177.5f, 158, 249.5f,
                158, 604.5f, 158, -177.5f
            };

            (void) m2.setPolyToPoly((const SkPoint*)src1, (SkPoint*)dst1, 4);

            {
                const SkPoint src[] = {
                    { SkIntToScalar(1), SkIntToScalar(0) },
                    { SkIntToScalar(4), SkIntToScalar(7) },
                    { SkIntToScalar(10), SkIntToScalar(2) }
                };
                const SkPoint dst[] = {
                    { SkIntToScalar(4), SkIntToScalar(2) },
                    { SkIntToScalar(45), SkIntToScalar(26) },
                    { SkIntToScalar(32), SkIntToScalar(17) }
                };

                SkMatrix m0;
                m0.setPolyToPoly(src, dst, 3);
            }
        }
    }

protected:
    virtual bool onQuery(Sample::Event* evt)  {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "PolyToPolyView");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    static void doDraw(SkCanvas* canvas, SkPaint* paint, const SkFont& font, const int isrc[],
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
        const SkScalar D = SkIntToScalar(64);
        canvas->drawRect(SkRect::MakeWH(D, D), *paint);
        canvas->drawLine(0, 0, D, D, *paint);
        canvas->drawLine(0, D, D, 0, *paint);

        SkFontMetrics fm;
        font.getMetrics(&fm);
        paint->setColor(SK_ColorRED);
        paint->setStyle(SkPaint::kFill_Style);
        SkScalar x = D/2;
        float y = D/2 - (fm.fAscent + fm.fDescent)/2;
        SkString str;
        str.appendS32(count);
        SkTextUtils::DrawString(canvas, str.c_str(), x, y, font, *paint, SkTextUtils::kCenter_Align);

        canvas->restore();
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStrokeWidth(SkIntToScalar(4));

        SkFont font;
        font.setSize(40);

        canvas->save();
        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));
        // translate (1 point)
        const int src1[] = { 0, 0 };
        const int dst1[] = { 5, 5 };
        doDraw(canvas, &paint, font, src1, dst1, 1);
        canvas->restore();

        canvas->save();
        canvas->translate(SkIntToScalar(160), SkIntToScalar(10));
        // rotate/uniform-scale (2 points)
        const int src2[] = { 32, 32, 64, 32 };
        const int dst2[] = { 32, 32, 64, 48 };
        doDraw(canvas, &paint, font, src2, dst2, 2);
        canvas->restore();

        canvas->save();
        canvas->translate(SkIntToScalar(10), SkIntToScalar(110));
        // rotate/skew (3 points)
        const int src3[] = { 0, 0, 64, 0, 0, 64 };
        const int dst3[] = { 0, 0, 96, 0, 24, 64 };
        doDraw(canvas, &paint, font, src3, dst3, 3);
        canvas->restore();

        canvas->save();
        canvas->translate(SkIntToScalar(160), SkIntToScalar(110));
        // perspective (4 points)
        const int src4[] = { 0, 0, 64, 0, 64, 64, 0, 64 };
        const int dst4[] = { 0, 0, 96, 0, 64, 96, 0, 64 };
        doDraw(canvas, &paint, font, src4, dst4, 4);
        canvas->restore();
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new PolyToPolyView(); )

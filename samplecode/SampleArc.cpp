/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/effects/Sk1DPathEffect.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkRandom.h"
#include "include/utils/SkTextUtils.h"
#include "samplecode/Sample.h"
#include "src/utils/SkUTF.h"

#include "include/utils/SkParsePath.h"
static void testparse() {
    SkRect r;
    r.setLTRB(0, 0, 10, 10.5f);
    SkPath p, p2;
    SkString str, str2;

    p.addRect(r);
    SkParsePath::ToSVGString(p, &str);
    SkParsePath::FromSVGString(str.c_str(), &p2);
    SkParsePath::ToSVGString(p2, &str2);
}

class ArcsView : public Sample {
    class MyDrawable : public SkDrawable {
        SkRect   fR;
        SkScalar fSweep;
    public:
        MyDrawable(const SkRect& r) : fR(r), fSweep(0) {}

        void setSweep(SkScalar sweep) {
            if (fSweep != sweep) {
                fSweep = sweep;
                this->notifyDrawingChanged();
            }
        }

        void onDraw(SkCanvas* canvas) override {
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setStrokeWidth(SkIntToScalar(2));

            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(0x800000FF);
            canvas->drawArc(fR, 0, fSweep, true, paint);

            paint.setColor(0x800FF000);
            canvas->drawArc(fR, 0, fSweep, false, paint);

            paint.setStyle(SkPaint::kStroke_Style);
            paint.setColor(SK_ColorRED);
            canvas->drawArc(fR, 0, fSweep, true, paint);

            paint.setStrokeWidth(0);
            paint.setColor(SK_ColorBLUE);
            canvas->drawArc(fR, 0, fSweep, false, paint);
        }

        SkRect onGetBounds() override {
            SkRect r(fR);
            r.outset(2, 2);
            return r;
        }
    };

    SkRect fRect = {20, 20, 220, 220};
    sk_sp<MyDrawable> fAnimatingDrawable;
    sk_sp<SkDrawable> fRootDrawable;

    SkString name() override { return SkString("Arcs"); }

    static void DrawRectWithLines(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
        canvas->drawRect(r, p);
        canvas->drawLine(r.fLeft, r.fTop, r.fRight, r.fBottom, p);
        canvas->drawLine(r.fLeft, r.fBottom, r.fRight, r.fTop, p);
        canvas->drawLine(r.fLeft, r.centerY(), r.fRight, r.centerY(), p);
        canvas->drawLine(r.centerX(), r.fTop, r.centerX(), r.fBottom, p);
    }

    static void DrawLabel(SkCanvas* canvas, const SkRect& rect, SkScalar start, SkScalar sweep) {
        SkFont font;
        SkString    str;
        str.appendScalar(start);
        str.append(", ");
        str.appendScalar(sweep);
        SkTextUtils::DrawString(canvas, str.c_str(), rect.centerX(),
                         rect.fBottom + font.getSize() * 5/4, font, SkPaint(),
                                SkTextUtils::kCenter_Align);
    }

    static void DrawArcs(SkCanvas* canvas) {
        SkPaint paint;
        SkRect  r;
        SkScalar w = 75;
        SkScalar h = 50;

        r.setWH(w, h);
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);

        canvas->save();
        canvas->translate(SkIntToScalar(10), SkIntToScalar(300));

        paint.setStrokeWidth(SkIntToScalar(1));

        static const SkScalar gAngles[] = {
            0, 360,
            0, 45,
            0, -45,
            720, 135,
            -90, 269,
            -90, 270,
            -90, 271,
            -180, -270,
            225, 90
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(gAngles); i += 2) {
            paint.setColor(SK_ColorBLACK);
            DrawRectWithLines(canvas, r, paint);

            paint.setColor(SK_ColorRED);
            canvas->drawArc(r, gAngles[i], gAngles[i+1], false, paint);

            DrawLabel(canvas, r, gAngles[i], gAngles[i+1]);

            canvas->translate(w * 8 / 7, 0);
        }

        canvas->restore();
    }

    void drawRoot(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStrokeWidth(SkIntToScalar(2));
        paint.setStyle(SkPaint::kStroke_Style);

        DrawRectWithLines(canvas, fRect, paint);

        canvas->drawDrawable(fAnimatingDrawable.get());

        DrawArcs(canvas);
    }

    void onOnceBeforeDraw() override {
        testparse();
        this->setBGColor(0xFFDDDDDD);

        fAnimatingDrawable = sk_make_sp<MyDrawable>(fRect);

        SkPictureRecorder recorder;
        this->drawRoot(recorder.beginRecording(SkRect::MakeWH(800, 500)));
        fRootDrawable = recorder.finishRecordingAsDrawable();
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawDrawable(fRootDrawable.get());
    }

    bool onAnimate(double nanos) override {
        SkScalar angle = SkDoubleToScalar(fmod(1e-9 * nanos * 360 / 24, 360));
        if (fAnimatingDrawable) {
            fAnimatingDrawable->setSweep(angle);
        }
        return true;
    }
};

DEF_SAMPLE( return new ArcsView(); )

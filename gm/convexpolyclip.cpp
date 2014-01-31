
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkBitmap.h"
#include "SkGradientShader.h"
#include "SkTLList.h"

static SkBitmap make_bmp(int w, int h) {
    SkBitmap bmp;
    bmp.allocN32Pixels(w, h, true);

    SkCanvas canvas(bmp);
    SkScalar wScalar = SkIntToScalar(w);
    SkScalar hScalar = SkIntToScalar(h);

    SkPoint     pt = { wScalar / 2, hScalar / 2 };

    SkScalar    radius = 3 * SkMaxScalar(wScalar, hScalar);

    SkColor     colors[] = { SK_ColorDKGRAY, 0xFF222255,
                             0xFF331133, 0xFF884422,
                             0xFF000022, SK_ColorWHITE,
                             0xFFAABBCC};

    SkScalar    pos[] = {0,
                         SK_Scalar1 / 6,
                         2 * SK_Scalar1 / 6,
                         3 * SK_Scalar1 / 6,
                         4 * SK_Scalar1 / 6,
                         5 * SK_Scalar1 / 6,
                         SK_Scalar1};

    SkPaint paint;
    paint.setShader(SkGradientShader::CreateRadial(
                    pt, radius,
                    colors, pos,
                    SK_ARRAY_COUNT(colors),
                    SkShader::kRepeat_TileMode))->unref();
    SkRect rect = SkRect::MakeWH(wScalar, hScalar);
    SkMatrix mat = SkMatrix::I();
    for (int i = 0; i < 4; ++i) {
        paint.getShader()->setLocalMatrix(mat);
        canvas.drawRect(rect, paint);
        rect.inset(wScalar / 8, hScalar / 8);
        mat.preTranslate(6 * wScalar, 6 * hScalar);
        mat.postScale(SK_Scalar1 / 3, SK_Scalar1 / 3);
    }

    paint.setAntiAlias(true);
    paint.setTextSize(wScalar / 2.2f);
    paint.setShader(0);
    paint.setColor(SK_ColorLTGRAY);
    static const char kTxt[] = "Skia";
    SkPoint texPos = { wScalar / 17, hScalar / 2 + paint.getTextSize() / 2.5f };
    canvas.drawText(kTxt, SK_ARRAY_COUNT(kTxt)-1, texPos.fX, texPos.fY, paint);
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SK_Scalar1);
    canvas.drawText(kTxt, SK_ARRAY_COUNT(kTxt)-1, texPos.fX, texPos.fY, paint);
    return bmp;
}

namespace skiagm {
/**
 * This GM tests convex polygon clips.
 */
class ConvexPolyClip : public GM {
public:
    ConvexPolyClip() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("convex_poly_clip");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return make_isize(435, 440);
    }

    virtual void onOnceBeforeDraw() SK_OVERRIDE {
        SkPath tri;
        tri.moveTo(5.f, 5.f);
        tri.lineTo(100.f, 20.f);
        tri.lineTo(15.f, 100.f);

        fPaths.addToTail(tri);

        SkPath hexagon;
        static const SkScalar kRadius = 45.f;
        const SkPoint center = { kRadius, kRadius };
        for (int i = 0; i < 6; ++i) {
            SkScalar angle = 2 * SK_ScalarPI * i / 6;
            SkPoint point;
            point.fY = SkScalarSinCos(angle, &point.fX);
            point.scale(kRadius);
            point = center + point;
            if (0 == i) {
                hexagon.moveTo(point);
            } else {
                hexagon.lineTo(point);
            }
        }
        fPaths.addToTail(hexagon);

        SkMatrix scaleM;
        scaleM.setScale(1.1f, 0.4f, kRadius, kRadius);
        hexagon.transform(scaleM);
        fPaths.addToTail(hexagon);

        SkPath rotRect;
        SkRect rect = SkRect::MakeLTRB(10.f, 12.f, 80.f, 86.f);
        rotRect.addRect(rect);
        SkMatrix rotM;
        rotM.setRotate(23.f, rect.centerX(), rect.centerY());
        rotRect.transform(rotM);
        fPaths.addToTail(rotRect);

        fBmp = make_bmp(100, 100);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        const SkPath* path;
        SkScalar y = 0;
        static const SkScalar kMargin = 10.f;

        SkPaint bgPaint;
        bgPaint.setAlpha(0x15);
        SkISize size = canvas->getDeviceSize();
        SkRect dstRect = SkRect::MakeWH(SkIntToScalar(size.fWidth),
                                        SkIntToScalar(size.fHeight));
        canvas->drawBitmapRectToRect(fBmp, NULL, dstRect, &bgPaint);

        for (SkTLList<SkPath>::Iter iter(fPaths, SkTLList<SkPath>::Iter::kHead_IterStart);
             NULL != (path = iter.get());
             iter.next()) {
            SkScalar x = 0;
            for (int aa = 0; aa < 2; ++aa) {
                canvas->save();
                canvas->translate(x, y);
                canvas->clipPath(*path, SkRegion::kIntersect_Op, SkToBool(aa));
                canvas->drawBitmap(fBmp, 0, 0);
                canvas->restore();
                x += fBmp.width() + kMargin;
            }
            for (int aa = 0; aa < 2; ++aa) {
                static const char kTxt[] = "Clip Me!";
                SkPaint txtPaint;
                txtPaint.setTextSize(23.f);
                txtPaint.setAntiAlias(true);
                txtPaint.setColor(SK_ColorDKGRAY);

                SkPaint clipOutlinePaint;
                clipOutlinePaint.setAntiAlias(true);
                clipOutlinePaint.setColor(0x50505050);
                clipOutlinePaint.setStyle(SkPaint::kStroke_Style);
                clipOutlinePaint.setStrokeWidth(0);

                canvas->save();
                canvas->translate(x, y);
                SkPath closedClipPath = *path;
                closedClipPath.close();
                canvas->drawPath(closedClipPath, clipOutlinePaint);
                canvas->clipPath(*path, SkRegion::kIntersect_Op, SkToBool(aa));
                canvas->scale(1.f, 1.8f);
                canvas->drawText(kTxt, SK_ARRAY_COUNT(kTxt)-1,
                                 0, 1.5f * txtPaint.getTextSize(),
                                 txtPaint);
                canvas->restore();
                x += fBmp.width() + kMargin;
            }

            y += fBmp.height() + kMargin;
        }
    }

private:
    SkTLList<SkPath> fPaths;
    SkBitmap         fBmp;

    typedef GM INHERITED;
};

DEF_GM( return SkNEW(ConvexPolyClip); )

}

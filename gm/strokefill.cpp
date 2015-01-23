/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkTypeface.h"

namespace skiagm {

class StrokeFillGM : public GM {
public:
    StrokeFillGM() {

    }

protected:

    SkString onShortName() SK_OVERRIDE {
        return SkString("stroke-fill");
    }

    SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(640, 480);
    }

    static void show_bold(SkCanvas* canvas, const void* text, int len,
                          SkScalar x, SkScalar y, const SkPaint& paint) {
        SkPaint p(paint);
        canvas->drawText(text, len, x, y, p);
        p.setFakeBoldText(true);
        canvas->drawText(text, len, x, y + SkIntToScalar(120), p);
    }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkScalar x = SkIntToScalar(100);
        SkScalar y = SkIntToScalar(88);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(100));
        paint.setStrokeWidth(SkIntToScalar(5));

        sk_tool_utils::set_portable_typeface(&paint, "Papyrus");
        show_bold(canvas, "Hello", 5, x, y, paint);

        sk_tool_utils::set_portable_typeface(&paint, "Hiragino Maru Gothic Pro");
        const unsigned char hyphen[] = { 0xE3, 0x83, 0xBC };
        show_bold(canvas, hyphen, SK_ARRAY_COUNT(hyphen), x + SkIntToScalar(300), y, paint);

        paint.setStyle(SkPaint::kStrokeAndFill_Style);

        SkPath path;
        path.setFillType(SkPath::kWinding_FillType);
        path.addCircle(x, y + SkIntToScalar(200), SkIntToScalar(50), SkPath::kCW_Direction);
        path.addCircle(x, y + SkIntToScalar(200), SkIntToScalar(40), SkPath::kCCW_Direction);
        canvas->drawPath(path, paint);

        SkPath path2;
        path2.setFillType(SkPath::kWinding_FillType);
        path2.addCircle(x + SkIntToScalar(120), y + SkIntToScalar(200), SkIntToScalar(50), SkPath::kCCW_Direction);
        path2.addCircle(x + SkIntToScalar(120), y + SkIntToScalar(200), SkIntToScalar(40), SkPath::kCW_Direction);
        canvas->drawPath(path2, paint);

        path2.reset();
        path2.addCircle(x + SkIntToScalar(240), y + SkIntToScalar(200), SkIntToScalar(50), SkPath::kCCW_Direction);
        canvas->drawPath(path2, paint);
        SkASSERT(path2.cheapIsDirection(SkPath::kCCW_Direction));

        path2.reset();
        SkASSERT(!path2.cheapComputeDirection(NULL));
        path2.addCircle(x + SkIntToScalar(360), y + SkIntToScalar(200), SkIntToScalar(50), SkPath::kCW_Direction);
        SkASSERT(path2.cheapIsDirection(SkPath::kCW_Direction));
        canvas->drawPath(path2, paint);

        SkRect r = SkRect::MakeXYWH(x - SkIntToScalar(50), y + SkIntToScalar(280),
                                    SkIntToScalar(100), SkIntToScalar(100));
        SkPath path3;
        path3.setFillType(SkPath::kWinding_FillType);
        path3.addRect(r, SkPath::kCW_Direction);
        r.inset(SkIntToScalar(10), SkIntToScalar(10));
        path3.addRect(r, SkPath::kCCW_Direction);
        canvas->drawPath(path3, paint);

        r = SkRect::MakeXYWH(x + SkIntToScalar(70), y + SkIntToScalar(280), 
                             SkIntToScalar(100), SkIntToScalar(100));
        SkPath path4;
        path4.setFillType(SkPath::kWinding_FillType);
        path4.addRect(r, SkPath::kCCW_Direction);
        r.inset(SkIntToScalar(10), SkIntToScalar(10));
        path4.addRect(r, SkPath::kCW_Direction);
        canvas->drawPath(path4, paint);

        r = SkRect::MakeXYWH(x + SkIntToScalar(190), y + SkIntToScalar(280), 
                             SkIntToScalar(100), SkIntToScalar(100));
        path4.reset();
        SkASSERT(!path4.cheapComputeDirection(NULL));
        path4.addRect(r, SkPath::kCCW_Direction);
        SkASSERT(path4.cheapIsDirection(SkPath::kCCW_Direction));
        path4.moveTo(0, 0); // test for crbug.com/247770
        canvas->drawPath(path4, paint);

        r = SkRect::MakeXYWH(x + SkIntToScalar(310), y + SkIntToScalar(280), 
                             SkIntToScalar(100), SkIntToScalar(100));
        path4.reset();
        SkASSERT(!path4.cheapComputeDirection(NULL));
        path4.addRect(r, SkPath::kCW_Direction);
        SkASSERT(path4.cheapIsDirection(SkPath::kCW_Direction));
        path4.moveTo(0, 0); // test for crbug.com/247770
        canvas->drawPath(path4, paint);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return SkNEW(StrokeFillGM);)

}

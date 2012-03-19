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
    virtual SkString onShortName() {
        return SkString("stroke-fill");
    }

    virtual SkISize onISize() {
        return make_isize(640, 480);
    }

    static void show_bold(SkCanvas* canvas, const void* text, int len,
                          SkScalar x, SkScalar y, const SkPaint& paint) {
        SkPaint p(paint);
        canvas->drawText(text, len, x, y, p);
        p.setFakeBoldText(true);
        canvas->drawText(text, len, x, y + SkIntToScalar(120), p);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkScalar x = SkIntToScalar(100);
        SkScalar y = SkIntToScalar(88);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(100));
        paint.setStrokeWidth(SkIntToScalar(5));
        
        SkTypeface* face = SkTypeface::CreateFromName("Papyrus", SkTypeface::kNormal);
        SkSafeUnref(paint.setTypeface(face));
        show_bold(canvas, "Hello", 5, x, y, paint);

        face = SkTypeface::CreateFromName("Hiragino Maru Gothic Pro", SkTypeface::kNormal);
        SkSafeUnref(paint.setTypeface(face));
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
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new StrokeFillGM; }
static GMRegistry reg(MyFactory);

}

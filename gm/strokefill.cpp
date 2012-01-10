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

static void test10(SkCanvas* canvas) {
    SkPaint paint;
    const char text[] = "Hello"; // "Hello";
    const size_t len = sizeof(text) - 1;
    paint.setAntiAlias(true);
    paint.setTextSize(SkIntToScalar(100));
    SkTypeface* hira = SkTypeface::CreateFromName("Hiragino Maru Gothic Pro",
                                                  SkTypeface::kNormal);
    paint.setTypeface(hira);
    SkScalar x = 180;
    SkScalar y = 88;
    
    canvas->drawText(text, len, x, y, paint);
    paint.setFakeBoldText(true);
    canvas->drawText(text, len, x, y + 100, paint);
    paint.setStyle(SkPaint::kStrokeAndFill_Style);
    paint.setStrokeWidth(5);
    
    SkPath path;
    path.setFillType(SkPath::kWinding_FillType);
    path.addCircle(x, y + 200, 50, SkPath::kCW_Direction);
    path.addCircle(x, y + 200, 40, SkPath::kCCW_Direction);
    canvas->drawPath(path, paint);
    
    SkPath path2;
    path2.setFillType(SkPath::kWinding_FillType);
    path2.addCircle(x + 120, y + 200, 50, SkPath::kCCW_Direction);
    path2.addCircle(x + 120, y + 200, 40, SkPath::kCW_Direction);
    canvas->drawPath(path2, paint);
    
    path2.reset();
    path2.addCircle(x + 240, y + 200, 50, SkPath::kCCW_Direction);
    canvas->drawPath(path2, paint);
    SkASSERT(path2.cheapIsDirection(SkPath::kCCW_Direction));
    
    path2.reset();
    SkASSERT(!path2.cheapComputeDirection(NULL));
    path2.addCircle(x + 360, y + 200, 50, SkPath::kCW_Direction);
    SkASSERT(path2.cheapIsDirection(SkPath::kCW_Direction));
    canvas->drawPath(path2, paint);
}

static void test_path(SkCanvas* canvas, const SkPath& path) {
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawPath(path, paint);
    
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorRED);
    canvas->drawPath(path, paint);
}

static void test_rev(SkCanvas* canvas, const SkPath& path) {
    test_path(canvas, path);

    SkPath rev;
    rev.reverseAddPath(path);
    canvas->save();
    canvas->translate(150, 0);
    test_path(canvas, rev);
    canvas->restore();
}

static void test_rev(SkCanvas* canvas) {
    SkRect r = { 10, 10, 100, 60 };

    SkPath path;

    path.addRect(r); test_rev(canvas, path);

    canvas->translate(0, 100);
    path.offset(20, 20);
    path.addRect(r); test_rev(canvas, path);

    canvas->translate(0, 100);
    path.reset();
    path.moveTo(10, 10); path.lineTo(30, 30);
    path.addOval(r);
    r.offset(50, 20);
    path.addOval(r);
    test_rev(canvas, path);

    SkPaint paint;
    paint.setTextSize(SkIntToScalar(100));
    SkTypeface* hira = SkTypeface::CreateFromName("Hiragino Maru Gothic Pro", SkTypeface::kNormal);
    SkSafeUnref(paint.setTypeface(hira));
    path.reset();
    paint.getTextPath("e", 1, 50, 50, &path);
    canvas->translate(0, 100);
    test_rev(canvas, path);
}

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

    virtual void onDraw(SkCanvas* canvas) {
        test10(canvas);
    //    test_rev(canvas);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new StrokeFillGM; }
static GMRegistry reg(MyFactory);

}

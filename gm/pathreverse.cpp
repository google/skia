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

/* The hiragino_maru_goth_pro_e path was generated with Mac-specific code:
 *
 * paint.setTextSize(SkIntToScalar(100));
 * paint.setTypeface(SkTypeface::CreateFromName("Hiragino Maru Gothic Pro"));
 * paint.getTextPath("e", 1, 50, 50, &path);
 * 
 * The path data is duplicated here to allow the test to
 * run on all platforms and to remove the bug dependency
 * should future Macs edit or delete the font.
 */
static SkPath hiragino_maru_goth_pro_e() {
    SkPath path;
    path.moveTo(98.6f, 24.7f);
    path.cubicTo(101.7f, 24.7f, 103.6f, 22.8f, 103.6f, 19.2f);
    path.cubicTo(103.6f, 18.9f, 103.6f, 18.7f, 103.6f, 18.4f);
    path.cubicTo(102.6f, 5.3f, 94.4f, -6.1f, 79.8f, -6.1f);
    path.cubicTo(63.5f, -6.1f, 54.5f, 6, 54.5f, 23.3f);
    path.cubicTo(54.5f, 40.6f, 64, 52.2f, 80.4f, 52.2f);
    path.cubicTo(93.4f, 52.2f, 99.2f, 45.6f, 102.4f, 39);
    path.cubicTo(102.8f, 38.4f, 102.9f, 37.8f, 102.9f, 37.2f);
    path.cubicTo(102.9f, 35.4f, 101.5f, 34.2f, 99.8f, 33.7f);
    path.cubicTo(99.1f, 33.5f, 98.4f, 33.3f, 97.7f, 33.3f);
    path.cubicTo(96.3f, 33.3f, 95, 34, 94.1f, 35.8f);
    path.cubicTo(91.7f, 41.1f, 87.7f, 44.7f, 80.5f, 44.7f);
    path.cubicTo(69.7f, 44.7f, 63.6f, 37, 63.4f, 24.7f);
    path.lineTo(98.6f, 24.7f);
    path.close();
    path.moveTo(63.7f, 17.4f);
    path.cubicTo(65, 7.6f, 70.2f, 1.2f, 79.8f, 1.2f);
    path.cubicTo(89, 1.2f, 93.3f, 8.5f, 94.5f, 15.6f);
    path.cubicTo(94.5f, 15.8f, 94.5f, 16, 94.5f, 16.1f);
    path.cubicTo(94.5f, 17, 94.1f, 17.4f, 93, 17.4f);
    path.lineTo(63.7f, 17.4f);
    path.close();
    return path;
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

namespace skiagm {

class PathReverseGM : public GM {
public:
    PathReverseGM() {

    }

protected:

    SkString onShortName() override {
        return SkString("path-reverse");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
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

        path = hiragino_maru_goth_pro_e();
        canvas->translate(0, 100);
        test_rev(canvas, path);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new PathReverseGM; }
static GMRegistry reg(MyFactory);

}

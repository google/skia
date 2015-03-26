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
    sk_tool_utils::set_portable_typeface(&paint, "Hiragino Maru Gothic Pro");
    path.reset();
    paint.getTextPath("e", 1, 50, 50, &path);
    canvas->translate(0, 100);
    test_rev(canvas, path);
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
        if (false) test_rev(canvas); // avoid bit rot, suppress warning
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
        sk_tool_utils::set_portable_typeface(&paint, "Hiragino Maru Gothic Pro");
        path.reset();
        paint.getTextPath("e", 1, 50, 50, &path);
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

/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPath.h"
#include "SkScan.h"

#define W   800
#define H   800

class AnalyticAntiAliasConvexGM : public skiagm::GM {
public:
    AnalyticAntiAliasConvexGM() {}

protected:

    SkString onShortName() override {
        return SkString("analytic_antialias_convex");
    }

    SkISize onISize() override {
        return SkISize::Make(W, H);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);

        canvas->clear(0xFFFFFFFF);

        canvas->save();

        SkScalar y = 0;

        canvas->translate(0, y);
        canvas->rotate(1);
        canvas->drawRectCoords(20, 20, 200, 200, p);
        canvas->restore();

        y += 200;

        canvas->translate(0, y);
        canvas->rotate(1);
        canvas->drawRectCoords(20, 20, 20.2f, 200, p);
        canvas->drawRectCoords(20, 200, 200, 200.1f, p);
        canvas->drawCircle(100, 100, 30, p);
        canvas->restore();

        // The following path is empty but it'll reveal bug chrome:662914
        SkPath path;
        path.moveTo(SkBits2Float(0x429b9d5c), SkBits2Float(0x4367a041));  // 77.8073f, 231.626f
        // 77.8075f, 231.626f, 77.8074f, 231.625f, 77.8073f, 231.625f
        path.cubicTo(SkBits2Float(0x429b9d71), SkBits2Float(0x4367a022),
                SkBits2Float(0x429b9d64), SkBits2Float(0x4367a009),
                SkBits2Float(0x429b9d50), SkBits2Float(0x43679ff2));
        path.lineTo(SkBits2Float(0x429b9d5c), SkBits2Float(0x4367a041));  // 77.8073f, 231.626f
        path.close();
        canvas->drawPath(path, p);
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return new AnalyticAntiAliasConvexGM; )

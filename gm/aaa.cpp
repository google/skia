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
        canvas->drawRect({ 20, 20, 200, 200 }, p);
        canvas->restore();

        y += 200;

        canvas->translate(0, y);
        canvas->rotate(1);
        canvas->drawRect({ 20, 20, 20.2f, 200 }, p);
        canvas->drawRect({ 20, 200, 200, 200.1f }, p);
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

        // The following path reveals a subtle SkAnalyticQuadraticEdge::updateQuadratic bug:
        // we should not use any snapped y for the intermediate values whose error may accumulate;
        // snapping should only be allowed once before updateLine.
        path.reset();
        path.moveTo(SkBits2Float(0x434ba71e), SkBits2Float(0x438a06d0));  // 203.653f, 276.053f
        path.lineTo(SkBits2Float(0x43492a74), SkBits2Float(0x4396d70d));  // 201.166f, 301.68f
        // 200.921f, 304.207f, 196.939f, 303.82f, 0.707107f
        path.conicTo(SkBits2Float(0x4348ebaf), SkBits2Float(0x43981a75),
                SkBits2Float(0x4344f079), SkBits2Float(0x4397e900), SkBits2Float(0x3f3504f3));
        path.close();
        // Manually setting convexity is required. Otherwise, this path will be considered concave.
        path.setConvexity(SkPath::kConvex_Convexity);
        canvas->drawPath(path, p);
    }

private:
    typedef skiagm::GM INHERITED;
};

class AnalyticAntiAliasGeneralGM : public skiagm::GM {
public:
    AnalyticAntiAliasGeneralGM() {}

protected:

    SkString onShortName() override {
        return SkString("analytic_antialias_general");
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
        canvas->rotate(1);
        const SkScalar R = 115.2f, C = 128.0f;
        SkPath path;
        path.moveTo(C + R, C);
        for (int i = 1; i < 8; ++i) {
            SkScalar a = 2.6927937f * i;
            SkScalar cosine;
            SkScalar sine = SkScalarSinCos(a, &cosine);
            path.lineTo(C + R * cosine, C + R * sine);
        }
        canvas->drawPath(path, p);
        canvas->restore();

        canvas->save();
        canvas->translate(200, 0);
        canvas->rotate(1);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(5);
        canvas->drawPath(path, p);
        canvas->restore();
    }

private:
    typedef skiagm::GM INHERITED;
};

class AnalyticAntiAliasInverseGM : public skiagm::GM {
public:
    AnalyticAntiAliasInverseGM() {}

protected:

    SkString onShortName() override {
        return SkString("analytic_antialias_inverse");
    }

    SkISize onISize() override {
        return SkISize::Make(W, H);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);

        canvas->save();

        SkPath path;
        path.addCircle(100, 100, 30);
        path.setFillType(SkPath::kInverseWinding_FillType);
        canvas->drawPath(path, p);
        canvas->restore();
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return new AnalyticAntiAliasConvexGM; )
DEF_GM( return new AnalyticAntiAliasGeneralGM; )
DEF_GM( return new AnalyticAntiAliasInverseGM; )

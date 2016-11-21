/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkAnimTimer.h"
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
            path.lineTo(C + R * cos(a), C + R * sin(a));
        }
        canvas->drawPath(path, p);
        canvas->restore();

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

class AnalyticAntiAliasTestGM : public skiagm::GM {
public:
    AnalyticAntiAliasTestGM() {}

protected:

    SkString onShortName() override {
        return SkString("aaatest");
    }

    SkISize onISize() override {
        return SkISize::Make(W, H);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);

        canvas->save();

        // canvas->clipRect(SkRect::MakeLTRB(0, 144, 44, 244));
        SkPath path;
        path.moveTo(SkBits2Float(0x41100000), SkBits2Float(0x43530000));  // 9, 211
        // path.cubicTo(SkBits2Float(0x41100000), SkBits2Float(0x43530000), SkBits2Float(0xc1f8eb2e), SkBits2Float(0x431af649), SkBits2Float(0xc2038e5d), SkBits2Float(0x4323d05e));  // 9, 211, -31.1148f, 154.962f, -32.889f, 163.814f
        // path.cubicTo(SkBits2Float(0xc20aa723), SkBits2Float(0x432caa73), SkBits2Float(0xc2364d20), SkBits2Float(0x435d9e05), SkBits2Float(0xc240d8de), SkBits2Float(0x4330a4f5));  // -34.6632f, 172.666f, -45.5753f, 221.617f, -48.2118f, 176.644f
        // path.quadTo(SkBits2Float(0xc240d8de), SkBits2Float(0x4330a4f5), SkBits2Float(0xc12eacb8), SkBits2Float(0x433940ac));  // -48.2118f, 176.644f, -10.9172f, 185.253f
        // path.quadTo(SkBits2Float(0x41d30504), SkBits2Float(0x4341dc63), SkBits2Float(0xc1a15186), SkBits2Float(0x4316433b));  // 26.3774f, 193.861f, -20.1648f, 150.263f
        // path.quadTo(SkBits2Float(0xc2856a04), SkBits2Float(0x42d55428), SkBits2Float(0x420c29f0), SkBits2Float(0x436d8448));  // -66.7071f, 106.664f, 35.041f, 237.517f
        path.cubicTo(SkBits2Float(0x420c29f0), SkBits2Float(0x436d8448), SkBits2Float(0x4273d87e), SkBits2Float(0x43a1a4ae), SkBits2Float(0x429f8caa), SkBits2Float(0x43a60c06));  // 35.041f, 237.517f, 60.9614f, 323.287f, 79.7747f, 332.094f
        path.lineTo(SkBits2Float(0x429f8caa), SkBits2Float(0x436223c4));  // 79.7747f, 226.14f
        path.lineTo(SkBits2Float(0x429f8caa), SkBits2Float(0x432c0eee));  // 79.7747f, 172.058f
        // path.lineTo(SkBits2Float(0x429f8caa), SkBits2Float(0x432da9c1));  // 79.7747f, 173.663f
        // path.conicTo(SkBits2Float(0x429d637c), SkBits2Float(0x431088af), SkBits2Float(0x4287d566), SkBits2Float(0x43012206), SkBits2Float(0x3f5db3d7));  // 78.6943f, 144.534f, 67.9168f, 129.133f, 0.866025f
        // path.conicTo(SkBits2Float(0x42648ea4), SkBits2Float(0x42e376bc), SkBits2Float(0x423dc4da), SkBits2Float(0x42feeb8e), SkBits2Float(0x3f5db3d7));  // 57.1393f, 113.732f, 47.4422f, 127.46f, 0.866025f
        // path.conicTo(SkBits2Float(0x4216fb0e), SkBits2Float(0x430d3031), SkBits2Float(0x421b4d6c), SkBits2Float(0x432a5142), SkBits2Float(0x3f5db3d7));  // 37.7452f, 141.188f, 38.8256f, 170.317f, 0.866025f
        // path.conicTo(SkBits2Float(0x429b21c0), SkBits2Float(0x42fb58ee), SkBits2Float(0x42ac89ac), SkBits2Float(0x42c01b3a), SkBits2Float(0x3f59d69d));  // 77.5659f, 125.674f, 86.2689f, 96.0532f, 0.850931f
        // path.conicTo(SkBits2Float(0x42bdf198), SkBits2Float(0x4284dd86), SkBits2Float(0x4280108e), SkBits2Float(0x42a90d94), SkBits2Float(0x3f59d69d));  // 94.9719f, 66.4327f, 64.0323f, 84.5265f, 0.850931f
        // path.conicTo(SkBits2Float(0x42045f02), SkBits2Float(0x42cd3da2), SkBits2Float(0xc055e900), SkBits2Float(0x43147583), SkBits2Float(0x3f59d69d));  // 33.0928f, 102.62f, -3.34235f, 148.459f, 0.850931f
        // path.conicTo(SkBits2Float(0xc13028c4), SkBits2Float(0x42eb4c21), SkBits2Float(0xc231c2b5), SkBits2Float(0x42fa2920), SkBits2Float(0x3f38d4a6));  // -11.01f, 117.649f, -44.4401f, 125.08f, 0.721995f
        // path.conicTo(SkBits2Float(0xc29bbd9d), SkBits2Float(0x43048310), SkBits2Float(0xc292187c), SkBits2Float(0x4323f46d), SkBits2Float(0x3f38d4a6));  // -77.8703f, 132.512f, -73.0478f, 163.955f, 0.721995f
        // path.conicTo(SkBits2Float(0xc288735c), SkBits2Float(0x434365c9), SkBits2Float(0xc20989ee), SkBits2Float(0x433ea455), SkBits2Float(0x3f38d4a6));  // -68.2253f, 195.398f, -34.3847f, 190.642f, 0.721995f
        path.moveTo(SkBits2Float(0xc25cf010), SkBits2Float(0x436d655e));  // -55.2344f, 237.396f
        path.lineTo(SkBits2Float(0x41fdb604), SkBits2Float(0x435b70d3));  // 31.7139f, 219.441f
        path.moveTo(SkBits2Float(0x42962f4c), SkBits2Float(0x4382d41d));  // 75.0924f, 261.657f
        path.lineTo(SkBits2Float(0x42ea4f1e), SkBits2Float(0x43877100));  // 117.155f, 270.883f
        path.cubicTo(SkBits2Float(0x42ea4f1e), SkBits2Float(0x43877100), SkBits2Float(0xc2535964), SkBits2Float(0x435faf08), SkBits2Float(0xc1ae786a), SkBits2Float(0x433aeeb8));  // 117.155f, 270.883f, -52.8373f, 223.684f, -21.8088f, 186.932f
        path.cubicTo(SkBits2Float(0x411383e8), SkBits2Float(0x43162e68), SkBits2Float(0x42250bc4), SkBits2Float(0x431c4baa), SkBits2Float(0x4208cdb0), SkBits2Float(0x4349de4f));  // 9.2197f, 150.181f, 41.2615f, 156.296f, 34.2009f, 201.868f
        path.cubicTo(SkBits2Float(0x41d91f38), SkBits2Float(0x437770f5), SkBits2Float(0x40a9b4e0), SkBits2Float(0x43706828), SkBits2Float(0xc2118253), SkBits2Float(0x432fe0ec));  // 27.1402f, 247.441f, 5.30333f, 240.407f, -36.3773f, 175.879f
        // path.conicTo(SkBits2Float(0xc1bb7858), SkBits2Float(0x43781a63), SkBits2Float(0x42506f78), SkBits2Float(0x43693b64), SkBits2Float(0x3f3ec826));  // -23.4338f, 248.103f, 52.1089f, 233.232f, 0.745242f
        // p.setStyle(SkPaint::kStroke_Style);
        canvas->drawPath(path, p);
        canvas->restore();
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return new AnalyticAntiAliasConvexGM; )
DEF_GM( return new AnalyticAntiAliasGeneralGM; )
DEF_GM( return new AnalyticAntiAliasInverseGM; )
DEF_GM( return new AnalyticAntiAliasTestGM; )

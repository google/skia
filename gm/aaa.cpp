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

        SkPath path;
        path.moveTo(SkBits2Float(0x439e3000), SkBits2Float(0x4307ce14));  // 316.375f, 135.805f
        path.lineTo(SkBits2Float(0x439e3000), SkBits2Float(0x4307d020));  // 316.375f, 135.813f
        path.lineTo(SkBits2Float(0x439e3000), SkBits2Float(0x4307ce14));  // 316.375f, 135.805f
        path.lineTo(SkBits2Float(0x439e0000), SkBits2Float(0x430607f0));  // 316, 134.031f
        path.lineTo(SkBits2Float(0x439e3000), SkBits2Float(0x4307ce14));  // 316.375f, 135.805f
        path.close();
        path.moveTo(SkBits2Float(0x439b3b02), SkBits2Float(0x4300e7f0));  // 310.461f, 128.906f
        path.lineTo(SkBits2Float(0x439b3b02), SkBits2Float(0x4300e7f0));  // 310.461f, 128.906f
        path.lineTo(SkBits2Float(0x439b3b02), SkBits2Float(0x4300e7f0));  // 310.461f, 128.906f
        path.lineTo(SkBits2Float(0x439a5810), SkBits2Float(0x430147f0));  // 308.688f, 129.281f
        path.lineTo(SkBits2Float(0x439b3b02), SkBits2Float(0x4300e7f0));  // 310.461f, 128.906f
        path.close();
        path.moveTo(SkBits2Float(0x439b3b02), SkBits2Float(0x4300e7f0));  // 310.461f, 128.906f
        path.cubicTo(SkBits2Float(0x439b5b02), SkBits2Float(0x43021604), SkBits2Float(0x439bb4fe), SkBits2Float(0x430309fc), SkBits2Float(0x439c2c08), SkBits2Float(0x4303a5e4));  // 310.711f, 130.086f, 311.414f, 131.039f, 312.344f, 131.648f
        path.cubicTo(SkBits2Float(0x439ca4fe), SkBits2Float(0x430441cb), SkBits2Float(0x439d3916), SkBits2Float(0x430481cb), SkBits2Float(0x439dd000), SkBits2Float(0x430441cb));  // 313.289f, 132.257f, 314.446f, 132.507f, 315.625f, 132.257f
        path.lineTo(SkBits2Float(0x439e3000), SkBits2Float(0x4307cdd3));  // 316.375f, 135.804f
        path.cubicTo(SkBits2Float(0x439c070a), SkBits2Float(0x4308b9db), SkBits2Float(0x4399e9fc), SkBits2Float(0x4305f9db), SkBits2Float(0x439973f8), SkBits2Float(0x4301a9ba));  // 312.055f, 136.726f, 307.828f, 133.976f, 306.906f, 129.663f
        path.lineTo(SkBits2Float(0x439b3b02), SkBits2Float(0x4300e7f0));  // 310.461f, 128.906f
        path.close();
        path.moveTo(SkBits2Float(0x439c87f0), SkBits2Float(0x42f06042));  // 313.062f, 120.188f
        path.lineTo(SkBits2Float(0x439c87f0), SkBits2Float(0x42f06042));  // 313.062f, 120.188f
        path.lineTo(SkBits2Float(0x439c87f0), SkBits2Float(0x42f06042));  // 313.062f, 120.188f
        path.lineTo(SkBits2Float(0x439cb6ea), SkBits2Float(0x42f3f021));  // 313.429f, 121.969f
        path.lineTo(SkBits2Float(0x439c87f0), SkBits2Float(0x42f06042));  // 313.062f, 120.188f
        path.close();
        path.moveTo(SkBits2Float(0x439ce7f0), SkBits2Float(0x42f78000));  // 313.812f, 123.75f
        path.cubicTo(SkBits2Float(0x439c50e6), SkBits2Float(0x42f80000), SkBits2Float(0x439bd7f0), SkBits2Float(0x42f963d7), SkBits2Float(0x439b89fc), SkBits2Float(0x42fb4000));  // 312.632f, 124, 311.687f, 124.695f, 311.078f, 125.625f
        path.cubicTo(SkBits2Float(0x439b3b02), SkBits2Float(0x42fd23d7), SkBits2Float(0x439b1c08), SkBits2Float(0x42ff7021), SkBits2Float(0x439b3b02), SkBits2Float(0x4300e7f0));  // 310.461f, 126.57f, 310.219f, 127.719f, 310.461f, 128.906f
        path.lineTo(SkBits2Float(0x439973f8), SkBits2Float(0x4301a9fc));  // 306.906f, 129.664f
        path.cubicTo(SkBits2Float(0x43990000), SkBits2Float(0x42faac08), SkBits2Float(0x439a5df4), SkBits2Float(0x42f237cf), SkBits2Float(0x439c87f0), SkBits2Float(0x42f05fbf));  // 306, 125.336f, 308.734f, 121.109f, 313.062f, 120.187f
        path.lineTo(SkBits2Float(0x439ce7f0), SkBits2Float(0x42f78000));  // 313.812f, 123.75f
        path.close();
        path.moveTo(SkBits2Float(0x43a143f8), SkBits2Float(0x42fcb021));  // 322.531f, 126.344f
        path.lineTo(SkBits2Float(0x43a143f8), SkBits2Float(0x42fcb021));  // 322.531f, 126.344f
        path.lineTo(SkBits2Float(0x43a143f8), SkBits2Float(0x42fcb021));  // 322.531f, 126.344f
        path.lineTo(SkBits2Float(0x43a06000), SkBits2Float(0x42fd7021));  // 320.75f, 126.719f
        path.lineTo(SkBits2Float(0x43a143f8), SkBits2Float(0x42fcb021));  // 322.531f, 126.344f
        path.close();
        path.moveTo(SkBits2Float(0x439f7d0e), SkBits2Float(0x42fe3021));  // 318.977f, 127.094f
        path.cubicTo(SkBits2Float(0x439f5c08), SkBits2Float(0x42fbd3f8), SkBits2Float(0x439f0312), SkBits2Float(0x42f9f021), SkBits2Float(0x439e8a1c), SkBits2Float(0x42f8b43a));  // 318.719f, 125.914f, 318.024f, 124.969f, 317.079f, 124.352f
        path.cubicTo(SkBits2Float(0x439e120c), SkBits2Float(0x42f78001), SkBits2Float(0x439d7f1a), SkBits2Float(0x42f70001), SkBits2Float(0x439ce810), SkBits2Float(0x42f78001));  // 316.141f, 123.75f, 314.993f, 123.5f, 313.813f, 123.75f
        path.lineTo(SkBits2Float(0x439c8810), SkBits2Float(0x42f06043));  // 313.063f, 120.188f
        path.cubicTo(SkBits2Float(0x439eb106), SkBits2Float(0x42ee9064), SkBits2Float(0x43a0ce14), SkBits2Float(0x42f40c4b), SkBits2Float(0x43a14418), SkBits2Float(0x42fcb022));  // 317.383f, 119.282f, 321.61f, 122.024f, 322.532f, 126.344f
        path.lineTo(SkBits2Float(0x439f7d0e), SkBits2Float(0x42fe3021));  // 318.977f, 127.094f
        path.close();
        path.moveTo(SkBits2Float(0x439dd000), SkBits2Float(0x4304420c));  // 315.625f, 132.258f
        path.cubicTo(SkBits2Float(0x439e670a), SkBits2Float(0x43040000), SkBits2Float(0x439ee106), SkBits2Float(0x43035020), SkBits2Float(0x439f2efa), SkBits2Float(0x43025df3));  // 316.805f, 132, 317.758f, 131.313f, 318.367f, 130.367f
        path.cubicTo(SkBits2Float(0x439f7cee), SkBits2Float(0x43016dd2), SkBits2Float(0x439f9c08), SkBits2Float(0x430045e3), SkBits2Float(0x439f7cee), SkBits2Float(0x42fe301f));  // 318.976f, 129.429f, 319.219f, 128.273f, 318.976f, 127.094f
        path.lineTo(SkBits2Float(0x43a143f8), SkBits2Float(0x42fcb01f));  // 322.531f, 126.344f
        path.cubicTo(SkBits2Float(0x43a1b7f0), SkBits2Float(0x4302a9fb), SkBits2Float(0x43a057f0), SkBits2Float(0x4306e418), SkBits2Float(0x439e3000), SkBits2Float(0x4307ce14));  // 323.437f, 130.664f, 320.687f, 134.891f, 316.375f, 135.805f
        path.lineTo(SkBits2Float(0x439dd000), SkBits2Float(0x4304420c));  // 315.625f, 132.258f
        path.close();
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

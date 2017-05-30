/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkBlurDrawLooper.h"
#include "SkBlurMask.h"
#include "SkColorFilter.h"
#include "SkMaskFilter.h"
#include "SkPath.h"

namespace skiagm {

///////////////////////////////////////////////////////////////////////////////

static void setup(SkPaint* paint, SkColor c, SkScalar strokeWidth) {
    paint->setColor(c);
    if (strokeWidth < 0) {
        paint->setStyle(SkPaint::kFill_Style);
    } else {
        paint->setStyle(SkPaint::kStroke_Style);
        paint->setStrokeWidth(strokeWidth);
    }
}

class ShadowsGM : public GM {
public:
    SkPath fCirclePath;
    SkRect fRect;
    SkBitmap fBitmap;

protected:
    void onOnceBeforeDraw() override {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
        fCirclePath.addCircle(SkIntToScalar(20), SkIntToScalar(20), SkIntToScalar(10) );
        fRect.set(SkIntToScalar(10), SkIntToScalar(10),
                  SkIntToScalar(30), SkIntToScalar(30));
        fBitmap.allocPixels(SkImageInfo::Make(20, 20, SkColorType::kAlpha_8_SkColorType,
                            kPremul_SkAlphaType));
        SkCanvas canvas(fBitmap);
        canvas.clear(0x0);
        SkPaint p;
        canvas.drawRect(SkRect::MakeXYWH(10, 0, 10, 10), p);
        canvas.drawRect(SkRect::MakeXYWH(0, 10, 10, 10), p);
    }

    SkString onShortName() override {
        return SkString("shadows");
    }

    SkISize onISize() override {
        return SkISize::Make(200, 200);
    }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkDrawLooper> shadowLoopers[] = {
              SkBlurDrawLooper::Make(SK_ColorBLUE,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(10)),
                                     SkIntToScalar(5), SkIntToScalar(10)),
              SkBlurDrawLooper::Make(SK_ColorBLUE,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(10)),
                                     SkIntToScalar(5), SkIntToScalar(10)),
              SkBlurDrawLooper::Make(SK_ColorBLACK,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(5)),
                                     SkIntToScalar(5),
                                     SkIntToScalar(10)),
              SkBlurDrawLooper::Make(0x7FFF0000,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(5)),
                                     SkIntToScalar(-5), SkIntToScalar(-10)),
            SkBlurDrawLooper::Make(SK_ColorBLACK, SkIntToScalar(0),
                                     SkIntToScalar(5), SkIntToScalar(5)),
        };

        constexpr struct {
            SkColor fColor;
            SkScalar fStrokeWidth;
        } gRec[] = {
            { SK_ColorRED,      -SK_Scalar1 },
            { SK_ColorGREEN,    SkIntToScalar(4) },
            { SK_ColorBLUE,     SkIntToScalar(0)},
        };

        SkPaint paint;
        paint.setAntiAlias(true);
        for (size_t i = 0; i < SK_ARRAY_COUNT(shadowLoopers); ++i) {
            SkAutoCanvasRestore acr(canvas, true);

            paint.setLooper(shadowLoopers[i]);

            canvas->translate(SkIntToScalar((unsigned int)i*40), SkIntToScalar(0));
            setup(&paint, gRec[0].fColor, gRec[0].fStrokeWidth);
            canvas->drawRect(fRect, paint);

            canvas->translate(SkIntToScalar(0), SkIntToScalar(40));
            setup(&paint, gRec[1].fColor, gRec[1].fStrokeWidth);
            canvas->drawPath(fCirclePath, paint);

            canvas->translate(SkIntToScalar(0), SkIntToScalar(40));
            setup(&paint, gRec[2].fColor, gRec[2].fStrokeWidth);
            canvas->drawPath(fCirclePath, paint);

            // see bug.skia.org/562 (reference, draws correct)
            canvas->translate(0, 40);
            paint.setColor(SK_ColorBLACK);
            canvas->drawBitmap(fBitmap, 10, 10, &paint);

            canvas->translate(0, 40);
            paint.setShader(SkShader::MakeBitmapShader(
                                          fBitmap, SkShader::kRepeat_TileMode,
                                          SkShader::kRepeat_TileMode));

            // see bug.skia.org/562 (shows bug as reported)
            paint.setStyle(SkPaint::kFill_Style);
            canvas->drawRect(SkRect::MakeXYWH(10, 10, 20, 20), paint);
            paint.setShader(nullptr);
        }
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ShadowsGM; }
static GMRegistry reg(MyFactory);

///////////////////////////////////////////////////////////////////////////////

class DistShadowGM : public GM {
public:
    static constexpr int W = 512;
    static constexpr int H = 512;

    DistShadowGM() {
        this->genPoints();
    }

    SkString onShortName() override {
        return SkString("DistShadow");
    }

    SkISize onISize() override {
        return SkISize::Make(W, H);
    }

    bool onHandleKey(SkUnichar uni) override {
        switch (uni) {
            case '+':
                fBlurRadius += 1.0f;
                return true;
            case '-':
                fBlurRadius = SkTMax(1.0f, fBlurRadius - 1.0f);
                return true;
            case '>':
                if (fMode == 1) {
                    fN++;
                    this->genPoints();
                    return true;
                }
                return false;
            case '<':
                if (fMode == 1) {
                    fN = SkTMax(3, fN - 1);
                    this->genPoints();
                    return true;
                }
                return false;
            case '{':
                fOffset = SkTMax(fOffset - 1.0f, .0f);
                return true;
            case '}':
                fOffset += 1.0f;
                return true;
            case 'M':
                fMode = (fMode + 1) % MAX_MODE;
                this->genPoints();
                return true;
        }
        return false;
    }

    void onDraw(SkCanvas* canvas) override {
        SkBitmap bitmap;
        SkImageInfo info = SkImageInfo::MakeA8(W, H);
        SkASSERT(bitmap.tryAllocPixels(info));
        uint8_t* addr = bitmap.getAddr8(0, 0);

        int n = fN;
        SkPoint* points = fPoints.begin();

        // Draw outline
        SkPath outline;
        outline.moveTo(points[0]);
        for(int i = 0; i < n; ++i) {
            outline.lineTo(points[i]);
        }
        outline.close();

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(true);
        paint.setAlpha(0); // hide outline
        canvas->drawPath(outline, paint);

        // Draw shadow
        SkTArray<Line> lines;
        for(int i = 0; i < n; ++i) {
            lines.push_back(Line(points[i], points[i + 1]));
        }

        SkScalar r = fBlurRadius;
        for(int x = 0; x < W; ++x) {
            for(int y = 0; y < H; ++y) {
                SkPoint point;
                point.fX = x;
                point.fY = y;

                SkScalar dist = 0;
                for(int i = 0; i < n; ++i) {
                    dist += sqr(SkTMax(.0f, lines[i].dist(point) + r));
                }
                dist = (-r + sqrt(dist) - fOffset) / r;

                SkScalar alpha = SkTPin((1.0f - dist) * 0.5f, 0.0f, 1.0f);
                SkASSERT(alpha >= 0 && alpha <= 1);
                SkScalar factor = 1.0f - alpha; // somehow it's inverted...
                factor = exp(-factor * factor * 4.0f) - 0.018f; // gaussian

                addr[y * W + x] = (int)(factor * 255 + 0.5);
            }
        }

        canvas->drawBitmap(bitmap, 0, 0);
    }

private:
    SkScalar fBlurRadius = 10.0f;
    SkScalar fOffset = .0f;
    SkTArray<SkPoint> fPoints;
    int fN;
    int fMode = 0;

    static constexpr int MAX_MODE = 2;

    static constexpr double PI = 3.141592653589793;

    void genPoints() {
        switch (fMode) {
            case 0:
                // Some polygon
                fN = 4;
                fPoints.reset();
                fPoints.push_back({20, 100});
                fPoints.push_back({200, 20});
                fPoints.push_back({240, 100});
                fPoints.push_back({200, 200});
                fPoints.push_back({20, 100});
                // SkPoint points[] = {{20, 100}, {200, 20}, {240, 100}, {200, 200}, {20, 100}};
                break;
            case 1:
                // Points along a circle
                fN = SkTMax(3, fN);
                constexpr SkScalar R = 50;
                constexpr SkScalar CX = 100;
                constexpr SkScalar CY = 100;
                fPoints.reset();
                SkScalar angle = .0f;
                for(int i = 0; i <= fN; ++i) {
                    fPoints.push_back(SkPoint::Make(CX + R * cos(angle), CY + R * sin(angle)));
                    angle += 2 * PI / fN;
                }
        }

    }

    struct Line {
        SkScalar a, b, c; // a x + b y + c = 0

        Line(const SkPoint& p0 = {0, 0}, const SkPoint& p1 = {0, 1}) {
            b = p0.fX - p1.fX;
            a = p1.fY - p0.fY;
            c = p1.fX * p0.fY - p0.fX * p1.fY;

            // Normalize for dist
            SkScalar norm = sqrt(a * a + b * b);
            a /= norm;
            b /= norm;
            c /= norm;
        }

        SkScalar dist(const SkPoint& p) {
            return a * p.fX + b * p.fY + c;
        }
    };

    inline SkScalar sqr(SkScalar a) {
        return a * a;
    }
};

DEF_GM(return new DistShadowGM;)

}

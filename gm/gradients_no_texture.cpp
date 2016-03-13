/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkGradientShader.h"

using namespace skiagm;

struct GradData {
    int             fCount;
    const SkColor*  fColors;
    const SkScalar* fPos;
};

static const SkColor gColors[] = {
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE,
};

static const GradData gGradData[] = {
    { 1, gColors, nullptr },
    { 2, gColors, nullptr },
    { 3, gColors, nullptr },
    { 4, gColors, nullptr },
};

static SkShader* MakeLinear(const SkPoint pts[2], const GradData& data, SkShader::TileMode tm) {
    return SkGradientShader::CreateLinear(pts, data.fColors, data.fPos, data.fCount, tm);
}

static SkShader* MakeRadial(const SkPoint pts[2], const GradData& data, SkShader::TileMode tm) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::CreateRadial(center, center.fX, data.fColors,
                                          data.fPos, data.fCount, tm);
}

static SkShader* MakeSweep(const SkPoint pts[2], const GradData& data, SkShader::TileMode) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::CreateSweep(center.fX, center.fY, data.fColors, data.fPos, data.fCount);
}

static SkShader* Make2Radial(const SkPoint pts[2], const GradData& data, SkShader::TileMode tm) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointConical(
        center1, (pts[1].fX - pts[0].fX) / 7,
        center0, (pts[1].fX - pts[0].fX) / 2,
        data.fColors, data.fPos, data.fCount, tm);
}

static SkShader* Make2Conical(const SkPoint pts[2], const GradData& data, SkShader::TileMode tm) {
    SkPoint center0, center1;
    SkScalar radius0 = (pts[1].fX - pts[0].fX) / 10;
    SkScalar radius1 = (pts[1].fX - pts[0].fX) / 3;
    center0.set(pts[0].fX + radius0, pts[0].fY + radius0);
    center1.set(pts[1].fX - radius1, pts[1].fY - radius1);
    return SkGradientShader::CreateTwoPointConical(center1, radius1,
                                                   center0, radius0,
                                                   data.fColors, data.fPos,
                                                   data.fCount, tm);
}


typedef SkShader* (*GradMaker)(const SkPoint pts[2], const GradData& data, SkShader::TileMode tm);

static const GradMaker gGradMakers[] = {
    MakeLinear, MakeRadial, MakeSweep, Make2Radial, Make2Conical,
};

///////////////////////////////////////////////////////////////////////////////

class GradientsNoTextureGM : public GM {
public:
    GradientsNoTextureGM(bool dither) : fDither(dither) {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
    }

protected:

    SkString onShortName() override {
        return SkString(fDither ? "gradients_no_texture" : "gradients_no_texture_nodither");
    }

    SkISize onISize() override { return SkISize::Make(640, 615); }

    void onDraw(SkCanvas* canvas) override {
        static const SkPoint kPts[2] = { { 0, 0 },
                                         { SkIntToScalar(50), SkIntToScalar(50) } };
        static const SkShader::TileMode kTM = SkShader::kClamp_TileMode;
        SkRect kRect = { 0, 0, SkIntToScalar(50), SkIntToScalar(50) };
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setDither(fDither);

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        static const uint8_t kAlphas[] = { 0xff, 0x40 };
        for (size_t a = 0; a < SK_ARRAY_COUNT(kAlphas); ++a) {
            for (size_t i = 0; i < SK_ARRAY_COUNT(gGradData); ++i) {
                canvas->save();
                for (size_t j = 0; j < SK_ARRAY_COUNT(gGradMakers); ++j) {
                    SkShader* shader = gGradMakers[j](kPts, gGradData[i], kTM);
                    paint.setShader(shader)->unref();
                    paint.setAlpha(kAlphas[a]);
                    canvas->drawRect(kRect, paint);
                    canvas->translate(0, SkIntToScalar(kRect.height() + 20));
                }
                canvas->restore();
                canvas->translate(SkIntToScalar(kRect.width() + 20), 0);
            }
        }
    }

private:
    bool fDither;

    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

struct ColorPos {
    SkColor*    fColors;
    SkScalar*   fPos;
    int         fCount;

    ColorPos() : fColors(nullptr), fPos(nullptr), fCount(0) {}
    ~ColorPos() {
        delete[] fColors;
        delete[] fPos;
    }

    void construct(const SkColor colors[], const SkScalar pos[], int count) {
        fColors = new SkColor[count];
        memcpy(fColors, colors, count * sizeof(SkColor));
        if (pos) {
            fPos = new SkScalar[count];
            memcpy(fPos, pos, count * sizeof(SkScalar));
            fPos[0] = 0;
            fPos[count - 1] = 1;
        }
        fCount = count;
    }
};

static void make0(ColorPos* rec) {
#if 0
    From http://jsfiddle.net/3fe2a/

background-image: -webkit-linear-gradient(left, #22d1cd 1%, #22d1cd 0.9510157507590116%, #df4b37 2.9510157507590113%, #df4b37 23.695886056604927%, #22d1cd 25.695886056604927%, #22d1cd 25.39321881940624%, #e6de36 27.39321881940624%, #e6de36 31.849399922570655%, #3267ff 33.849399922570655%, #3267ff 44.57735802921938%, #9d47d1 46.57735802921938%, #9d47d1 53.27185850805876%, #3267ff 55.27185850805876%, #3267ff 61.95718972227316%, #5cdd9d 63.95718972227316%, #5cdd9d 69.89166004442%, #3267ff 71.89166004442%, #3267ff 74.45795382765857%, #9d47d1 76.45795382765857%, #9d47d1 82.78364610713776%, #3267ff 84.78364610713776%, #3267ff 94.52743647737229%, #e3d082 96.52743647737229%, #e3d082 96.03934633331295%);
height: 30px;
#endif

    const SkColor colors[] = {
        0xFF22d1cd, 0xFF22d1cd, 0xFFdf4b37, 0xFFdf4b37, 0xFF22d1cd, 0xFF22d1cd, 0xFFe6de36, 0xFFe6de36,
        0xFF3267ff, 0xFF3267ff, 0xFF9d47d1, 0xFF9d47d1, 0xFF3267ff, 0xFF3267ff, 0xFF5cdd9d, 0xFF5cdd9d,
        0xFF3267ff, 0xFF3267ff, 0xFF9d47d1, 0xFF9d47d1, 0xFF3267ff, 0xFF3267ff, 0xFFe3d082, 0xFFe3d082
    };
    const double percent[] = {
        1, 0.9510157507590116, 2.9510157507590113, 23.695886056604927,
        25.695886056604927, 25.39321881940624, 27.39321881940624, 31.849399922570655,
        33.849399922570655, 44.57735802921938, 46.57735802921938, 53.27185850805876,
        55.27185850805876, 61.95718972227316, 63.95718972227316, 69.89166004442,
        71.89166004442, 74.45795382765857, 76.45795382765857, 82.78364610713776,
        84.78364610713776, 94.52743647737229, 96.52743647737229, 96.03934633331295,
    };
    const int N = SK_ARRAY_COUNT(percent);
    SkScalar pos[N];
    for (int i = 0; i < N; ++i) {
        pos[i] = SkDoubleToScalar(percent[i] / 100);
    }
    rec->construct(colors, pos, N);
}

static void make1(ColorPos* rec) {
    const SkColor colors[] = {
        SK_ColorBLACK, SK_ColorWHITE, SK_ColorBLACK, SK_ColorWHITE,
        SK_ColorBLACK, SK_ColorWHITE, SK_ColorBLACK, SK_ColorWHITE,
        SK_ColorBLACK,
    };
    rec->construct(colors, nullptr, SK_ARRAY_COUNT(colors));
}

static void make2(ColorPos* rec) {
    const SkColor colors[] = {
        SK_ColorBLACK, SK_ColorWHITE, SK_ColorBLACK, SK_ColorWHITE,
        SK_ColorBLACK, SK_ColorWHITE, SK_ColorBLACK, SK_ColorWHITE,
        SK_ColorBLACK,
    };
    const int N = SK_ARRAY_COUNT(colors);
    SkScalar pos[N];
    for (int i = 0; i < N; ++i) {
        pos[i] = SK_Scalar1 * i / (N - 1);
    }
    rec->construct(colors, pos, N);
}

static void make3(ColorPos* rec) {
    const SkColor colors[] = {
        SK_ColorRED, SK_ColorBLUE, SK_ColorBLUE, SK_ColorGREEN, SK_ColorGREEN, SK_ColorBLACK,
    };
    const SkScalar pos[] = {
        0, 0, 0.5f, 0.5, 1, 1,
    };
    rec->construct(colors, pos, SK_ARRAY_COUNT(colors));
}

class GradientsManyColorsGM : public GM {
    enum {
        W = 800,
    };
    SkAutoTUnref<SkShader> fShader;

    typedef void (*Proc)(ColorPos*);
public:
    GradientsManyColorsGM(bool dither) : fDither(dither) {}

protected:

    SkString onShortName() override {
        return SkString(fDither ? "gradients_many" : "gradients_many_nodither");
    }

    SkISize onISize() override { return SkISize::Make(880, 400); }

    void onDraw(SkCanvas* canvas) override {
        const Proc procs[] = {
            make0, make1, make2, make3,
        };
        const SkPoint pts[] = {
            { 0, 0 },
            { SkIntToScalar(W), 0 },
        };
        const SkRect r = SkRect::MakeWH(SkIntToScalar(W), 30);

        SkPaint paint;
        paint.setDither(fDither);

        canvas->translate(40, 20);

        for (int i = 0; i <= 8; ++i) {
            SkScalar x = r.width() * i / 8;
            canvas->drawLine(x, 0, x, 10000, paint);
        }

        // expand the drawing rect so we exercise clampping in the gradients
        const SkRect drawR = r.makeOutset(20, 0);
        for (size_t i = 0; i < SK_ARRAY_COUNT(procs); ++i) {
            ColorPos rec;
            procs[i](&rec);
            SkShader* s = SkGradientShader::CreateLinear(pts, rec.fColors, rec.fPos, rec.fCount,
                                                         SkShader::kClamp_TileMode);
            paint.setShader(s)->unref();
            canvas->drawRect(drawR, paint);

            canvas->save();
            canvas->translate(r.centerX(), r.height() + 4);
            canvas->scale(-1, 1);
            canvas->translate(-r.centerX(), 0);
            canvas->drawRect(drawR, paint);
            canvas->restore();

            canvas->translate(0, r.height() + 2*r.height() + 8);
        }
    }

private:
    bool fDither;

    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM(return new GradientsNoTextureGM(true);)
DEF_GM(return new GradientsNoTextureGM(false);)
DEF_GM(return new GradientsManyColorsGM(true);)
DEF_GM(return new GradientsManyColorsGM(false);)

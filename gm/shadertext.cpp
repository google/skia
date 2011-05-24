#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkUnitMappers.h"

namespace skiagm {

static void makebm(SkBitmap* bm, SkBitmap::Config config, int w, int h) {
    bm->setConfig(config, w, h);
    bm->allocPixels();
    bm->eraseColor(0);

    SkCanvas    canvas(*bm);
    SkScalar    s = SkIntToScalar(SkMin32(w, h));
    SkPoint     pts[] = { { 0, 0 }, { s, s } };
    SkColor     colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    SkScalar    pos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    SkPaint     paint;

    SkUnitMapper*   um = NULL;

    um = new SkCosineMapper;

    SkAutoUnref au(um);

    paint.setDither(true);
    paint.setShader(SkGradientShader::CreateLinear(pts, colors, pos,
                SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode, um))->unref();
    canvas.drawPaint(paint);
}

SkShader* MakeBitmapShader(SkShader::TileMode tx, SkShader::TileMode ty,
                           int w, int h) {
    static SkBitmap bmp;
    if (bmp.isNull()) {
        makebm(&bmp, SkBitmap::kARGB_8888_Config, w/2, h/4);
    }
    return SkShader::CreateBitmapShader(bmp, tx, ty);
}

///////////////////////////////////////////////////////////////////////////////

struct GradData {
    int             fCount;
    const SkColor*  fColors;
    const SkScalar* fPos;
};

static const SkColor gColors[] = {
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK
};

static const GradData gGradData[] = {
    { 2, gColors, NULL },
    { 5, gColors, NULL },
};

static SkShader* MakeLinear(const SkPoint pts[2], const GradData& data,
                            SkShader::TileMode tm, SkUnitMapper* mapper) {
    return SkGradientShader::CreateLinear(pts, data.fColors, data.fPos,
                                          data.fCount, tm, mapper);
}

static SkShader* MakeRadial(const SkPoint pts[2], const GradData& data,
                            SkShader::TileMode tm, SkUnitMapper* mapper) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::CreateRadial(center, center.fX, data.fColors,
                                          data.fPos, data.fCount, tm, mapper);
}

static SkShader* MakeSweep(const SkPoint pts[2], const GradData& data,
                           SkShader::TileMode tm, SkUnitMapper* mapper) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::CreateSweep(center.fX, center.fY, data.fColors,
                                         data.fPos, data.fCount, mapper);
}

static SkShader* Make2Radial(const SkPoint pts[2], const GradData& data,
                           SkShader::TileMode tm, SkUnitMapper* mapper) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointRadial(
                            center1, (pts[1].fX - pts[0].fX) / 7,
                            center0, (pts[1].fX - pts[0].fX) / 2,
                            data.fColors, data.fPos, data.fCount, tm, mapper);
}

typedef SkShader* (*GradMaker)(const SkPoint pts[2], const GradData& data,
                     SkShader::TileMode tm, SkUnitMapper* mapper);
static const GradMaker gGradMakers[] = {
    MakeLinear, MakeRadial, MakeSweep, Make2Radial
};

///////////////////////////////////////////////////////////////////////////////

class ShaderTextGM : public GM {
public:
	ShaderTextGM() {}

protected:

    SkString onShortName() {
        return SkString("shadertext");
    }

	SkISize onISize() { return make_isize(950, 500); }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        const char text[] = "Shaded Text";
        const int textLen = SK_ARRAY_COUNT(text) - 1;
        const int pointSize = 48;

        int w = pointSize * textLen;
        int h = pointSize;

        SkPoint pts[2] = {
            { 0, 0 },
            { SkIntToScalar(w), SkIntToScalar(h) }
        };
        SkScalar textBase = SkIntToScalar(h/2);

        SkShader::TileMode tileModes[] = {
            SkShader::kClamp_TileMode,
            SkShader::kRepeat_TileMode,
            SkShader::kMirror_TileMode
        };

        static const int gradCount = SK_ARRAY_COUNT(gGradData) *
                                     SK_ARRAY_COUNT(gGradMakers);
        static const int bmpCount = SK_ARRAY_COUNT(tileModes) *
                                    SK_ARRAY_COUNT(tileModes);
        SkShader* shaders[gradCount + bmpCount];

        int shdIdx = 0;
        for (size_t d = 0; d < SK_ARRAY_COUNT(gGradData); ++d) {
            for (size_t m = 0; m < SK_ARRAY_COUNT(gGradMakers); ++m) {
                shaders[shdIdx++] = gGradMakers[m](pts,
                                                   gGradData[d],
                                                   SkShader::kClamp_TileMode,
                                                   NULL);
            }
        }
        for (size_t tx = 0; tx < SK_ARRAY_COUNT(tileModes); ++tx) {
            for (size_t ty = 0; ty < SK_ARRAY_COUNT(tileModes); ++ty) {
                shaders[shdIdx++] = MakeBitmapShader(tileModes[tx],
                                                     tileModes[ty],
                                                     w/8, h);
            }
        }

        SkPaint paint;
        paint.setDither(true);
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(pointSize));

        canvas->save();
        canvas->translate(SkIntToScalar(20), SkIntToScalar(10));

        static const int testsPerCol = 8;
        static const int rowHeight = 60;
        static const int colWidth = 300;
        canvas->save();
        for (size_t s = 0; s < SK_ARRAY_COUNT(shaders); s++) {
            canvas->save();
            canvas->translate(SkIntToScalar((s / testsPerCol) * colWidth),
                              SkIntToScalar((s % testsPerCol) * rowHeight));
                              paint.setShader(shaders[s])->ref();
            canvas->drawText(text, textLen, 0, textBase, paint);
            canvas->restore();
        }
        canvas->restore();
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ShaderTextGM; }
static GMRegistry reg(MyFactory);

}

/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"

namespace skiagm {

static void makebm(SkBitmap* bm, int w, int h) {
    bm->allocN32Pixels(w, h);
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas    canvas(*bm);
    SkScalar    s = SkIntToScalar(SkMin32(w, h));
    SkPoint     pts[] = { { 0, 0 }, { s, s } };
    SkColor     colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    SkScalar    pos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    SkPaint     paint;

    paint.setDither(true);
    paint.setShader(SkGradientShader::CreateLinear(pts, colors, pos,
                SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode))->unref();
    canvas.drawPaint(paint);
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

static SkShader* Make2Conical(const SkPoint pts[2], const GradData& data, SkShader::TileMode tm) {
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

typedef SkShader* (*GradMaker)(const SkPoint pts[2], const GradData& data, SkShader::TileMode tm);

static const GradMaker gGradMakers[] = {
    MakeLinear, MakeRadial, MakeSweep, Make2Conical
};

///////////////////////////////////////////////////////////////////////////////

class ShaderTextGM : public GM {
public:
    ShaderTextGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
    }

protected:

    SkString onShortName() override {
        return SkString("shadertext");
    }

    SkISize onISize() override { return SkISize::Make(1450, 500); }

    void onDraw(SkCanvas* canvas) override {
        const char text[] = "Shaded Text";
        const int textLen = SK_ARRAY_COUNT(text) - 1;
        const int pointSize = 36;

        const int w = pointSize * textLen;
        const int h = pointSize;

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
                                                   SkShader::kClamp_TileMode);
            }
        }
        
        SkBitmap bm;
        makebm(&bm, w/16, h/4);
        for (size_t tx = 0; tx < SK_ARRAY_COUNT(tileModes); ++tx) {
            for (size_t ty = 0; ty < SK_ARRAY_COUNT(tileModes); ++ty) {
                shaders[shdIdx++] = SkShader::CreateBitmapShader(bm, tileModes[tx], tileModes[ty]);
            }
        }

        SkPaint paint;
        paint.setDither(true);
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setTextSize(SkIntToScalar(pointSize));

        canvas->save();
        canvas->translate(SkIntToScalar(20), SkIntToScalar(10));

        SkPath path;
        path.arcTo(SkRect::MakeXYWH(SkIntToScalar(-40), SkIntToScalar(15),
                                    SkIntToScalar(300), SkIntToScalar(90)),
                                    SkIntToScalar(225), SkIntToScalar(90),
                                    false);
        path.close();

        static const int testsPerCol = 8;
        static const int rowHeight = 60;
        static const int colWidth = 300;
        canvas->save();
        for (int s = 0; s < static_cast<int>(SK_ARRAY_COUNT(shaders)); s++) {
            canvas->save();
            int i = 2*s;
            canvas->translate(SkIntToScalar((i / testsPerCol) * colWidth),
                              SkIntToScalar((i % testsPerCol) * rowHeight));
            paint.setShader(shaders[s])->unref();
            canvas->drawText(text, textLen, 0, textBase, paint);
            canvas->restore();
            canvas->save();
            ++i;
            canvas->translate(SkIntToScalar((i / testsPerCol) * colWidth),
                              SkIntToScalar((i % testsPerCol) * rowHeight));
            canvas->drawTextOnPath(text, textLen, path, NULL, paint);
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

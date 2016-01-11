
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkBlurMask.h"
#include "SkCanvas.h"
#include "SkView.h"
#include "Sk1DPathEffect.h"
#include "Sk2DPathEffect.h"
#include "SkBlurMaskFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkColorPriv.h"
#include "SkCornerPathEffect.h"
#include "SkDashPathEffect.h"
#include "SkDiscretePathEffect.h"
#include "SkEmbossMaskFilter.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkGradientShader.h"
#include "SkImageDecoder.h"
#include "SkLayerRasterizer.h"
#include "SkMath.h"
#include "SkPath.h"
#include "SkPictureRecorder.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkComposeShader.h"
#include "SkCornerPathEffect.h"
#include "SkPathMeasure.h"
#include "SkPicture.h"
#include "SkRandom.h"
#include "SkTypeface.h"
#include "SkUtils.h"
#include "SkXfermode.h"

#include <math.h>

static inline SkPMColor rgb2gray(SkPMColor c) {
    unsigned r = SkGetPackedR32(c);
    unsigned g = SkGetPackedG32(c);
    unsigned b = SkGetPackedB32(c);

    unsigned x = (r * 5 + g * 7 + b * 4) >> 4;

    return SkPackARGB32(0, x, x, x) | (c & (SK_A32_MASK << SK_A32_SHIFT));
}

class SkGrayScaleColorFilter : public SkColorFilter {
public:
    virtual void filterSpan(const SkPMColor src[], int count,
                            SkPMColor result[]) const override {
        for (int i = 0; i < count; i++)
            result[i] = rgb2gray(src[i]);
    }
};

class SkChannelMaskColorFilter : public SkColorFilter {
public:
    SkChannelMaskColorFilter(U8CPU redMask, U8CPU greenMask, U8CPU blueMask) {
        fMask = SkPackARGB32(0xFF, redMask, greenMask, blueMask);
    }

    virtual void filterSpan(const SkPMColor src[], int count,
                            SkPMColor result[]) const override {
        SkPMColor mask = fMask;
        for (int i = 0; i < count; i++) {
            result[i] = src[i] & mask;
        }
    }

private:
    SkPMColor   fMask;
};

///////////////////////////////////////////////////////////

static void r0(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    p.setMaskFilter(SkBlurMaskFilter::Create(kNormal_SkBlurStyle,
                                             SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(3)),
                                             SkBlurMaskFilter::kNone_BlurFlag))->unref();
    rastBuilder->addLayer(p, SkIntToScalar(3), SkIntToScalar(3));

    p.setMaskFilter(nullptr);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rastBuilder->addLayer(p);

    p.setAlpha(0x11);
    p.setStyle(SkPaint::kFill_Style);
    p.setXfermodeMode(SkXfermode::kSrc_Mode);
    rastBuilder->addLayer(p);
}

static void r1(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    rastBuilder->addLayer(p);

    p.setAlpha(0x40);
    p.setXfermodeMode(SkXfermode::kSrc_Mode);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*2);
    rastBuilder->addLayer(p);
}

static void r2(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    p.setStyle(SkPaint::kStrokeAndFill_Style);
    p.setStrokeWidth(SK_Scalar1*4);
    rastBuilder->addLayer(p);

    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*3/2);
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p);
}

static void r3(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*3);
    rastBuilder->addLayer(p);

    p.setAlpha(0x20);
    p.setStyle(SkPaint::kFill_Style);
    p.setXfermodeMode(SkXfermode::kSrc_Mode);
    rastBuilder->addLayer(p);
}

static void r4(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    p.setAlpha(0x60);
    rastBuilder->addLayer(p, SkIntToScalar(3), SkIntToScalar(3));

    p.setAlpha(0xFF);
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p, SK_Scalar1*3/2, SK_Scalar1*3/2);

    p.setXfermode(nullptr);
    rastBuilder->addLayer(p);
}

static void r5(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    rastBuilder->addLayer(p);

    p.setPathEffect(SkDiscretePathEffect::Create(SK_Scalar1*4, SK_Scalar1*3))->unref();
    p.setXfermodeMode(SkXfermode::kSrcOut_Mode);
    rastBuilder->addLayer(p);
}

static void r6(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    rastBuilder->addLayer(p);

    p.setAntiAlias(false);
    SkLayerRasterizer::Builder rastBuilder2;
    r5(&rastBuilder2, p);
    p.setRasterizer(rastBuilder2.detachRasterizer())->unref();
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p);
}

class Dot2DPathEffect : public Sk2DPathEffect {
public:
    Dot2DPathEffect(SkScalar radius, const SkMatrix& matrix)
        : Sk2DPathEffect(matrix), fRadius(radius) {}

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(Dot2DPathEffect)

protected:
    void next(const SkPoint& loc, int u, int v, SkPath* dst) const override {
        dst->addCircle(loc.fX, loc.fY, fRadius);
    }

    void flatten(SkWriteBuffer& buffer) const override {
        this->INHERITED::flatten(buffer);
        buffer.writeScalar(fRadius);
    }

private:
    SkScalar fRadius;

    typedef Sk2DPathEffect INHERITED;
};

static void r7(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1*6, SK_Scalar1*6, 0, 0);
    lattice.postSkew(SK_Scalar1/3, 0, 0, 0);
    p.setPathEffect(new Dot2DPathEffect(SK_Scalar1*4, lattice))->unref();
    rastBuilder->addLayer(p);
}

static void r8(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    rastBuilder->addLayer(p);

    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1*6, SK_Scalar1*6, 0, 0);
    lattice.postSkew(SK_Scalar1/3, 0, 0, 0);
    p.setPathEffect(new Dot2DPathEffect(SK_Scalar1*2, lattice))->unref();
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p);

    p.setPathEffect(nullptr);
    p.setXfermode(nullptr);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rastBuilder->addLayer(p);
}

static void r9(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    rastBuilder->addLayer(p);

    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1, SK_Scalar1*6, 0, 0);
    lattice.postRotate(SkIntToScalar(30), 0, 0);
    p.setPathEffect(SkLine2DPathEffect::Create(SK_Scalar1*2, lattice))->unref();
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p);

    p.setPathEffect(nullptr);
    p.setXfermode(nullptr);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rastBuilder->addLayer(p);
}

typedef void (*raster_proc)(SkLayerRasterizer::Builder*, SkPaint&);

static const raster_proc gRastProcs[] = {
    r0, r1, r2, r3, r4, r5, r6, r7, r8, r9
};

static const struct {
    SkColor fMul, fAdd;
} gLightingColors[] = {
    { 0x808080, 0x800000 }, // general case
    { 0x707070, 0x707070 }, // no-pin case
    { 0xFFFFFF, 0x800000 }, // just-add case
    { 0x808080, 0x000000 }, // just-mul case
    { 0xFFFFFF, 0x000000 }  // identity case
};

static void apply_shader(SkPaint* paint, int index) {
    raster_proc proc = gRastProcs[index];
    if (proc) {
        SkPaint p;
        SkLayerRasterizer::Builder rastBuilder;

        p.setAntiAlias(true);
        proc(&rastBuilder, p);
        paint->setRasterizer(rastBuilder.detachRasterizer())->unref();
    }

#if 1
    SkScalar dir[] = { SK_Scalar1, SK_Scalar1, SK_Scalar1 };
    paint->setMaskFilter(SkBlurMaskFilter::CreateEmboss(
                SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(3)), dir,
                SK_Scalar1/4, SkIntToScalar(4)))->unref();
    paint->setColor(SK_ColorBLUE);
#endif
}

class DemoView : public SampleView {
public:
    DemoView() {}

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Demo");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual bool onClick(Click* click) {
        return this->INHERITED::onClick(click);
    }

    void makePath(SkPath& path) {
        path.addCircle(SkIntToScalar(20), SkIntToScalar(20), SkIntToScalar(20),
            SkPath::kCCW_Direction);
        for (int index = 0; index < 10; index++) {
            SkScalar x = (float) cos(index / 10.0f * 2 * 3.1415925358f);
            SkScalar y = (float) sin(index / 10.0f * 2 * 3.1415925358f);
            x *= index & 1 ? 7 : 14;
            y *= index & 1 ? 7 : 14;
            x += SkIntToScalar(20);
            y += SkIntToScalar(20);
            if (index == 0)
                path.moveTo(x, y);
            else
                path.lineTo(x, y);
        }
        path.close();
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        canvas->save();
        this->drawPicture(canvas, 0);
        canvas->restore();

        {
            SkPictureRecorder recorder;
            {
                SkCanvas* record = recorder.beginRecording(320, 480, nullptr, 0);
                this->drawPicture(record, 120);
            }
            SkAutoTUnref<SkPicture> picture(recorder.endRecording());

            canvas->translate(0, SkIntToScalar(120));

            SkRect clip;
            clip.set(0, 0, SkIntToScalar(160), SkIntToScalar(160));
            do {
                canvas->save();
                canvas->clipRect(clip);
                picture->playback(canvas);
                canvas->restore();
                if (clip.fRight < SkIntToScalar(320))
                    clip.offset(SkIntToScalar(160), 0);
                else if (clip.fBottom < SkIntToScalar(480))
                    clip.offset(-SkIntToScalar(320), SkIntToScalar(160));
                else
                    break;
            } while (true);
        }
    }

    void drawPicture(SkCanvas* canvas, int spriteOffset) {
        SkMatrix matrix; matrix.reset();
        SkPaint paint;
        SkPath path;
        SkPoint start = {0, 0};
        SkPoint stop = { SkIntToScalar(40), SkIntToScalar(40) };
        SkRect rect = {0, 0, SkIntToScalar(40), SkIntToScalar(40) };
        SkRect rect2 = {0, 0, SkIntToScalar(65), SkIntToScalar(20) };
        SkScalar left = 0, top = 0, x = 0, y = 0;
        int index;

        char ascii[] = "ascii...";
        int asciiLength = sizeof(ascii) - 1;
        char utf8[] = "utf8" "\xe2\x80\xa6";
        short utf16[] = {'u', 't', 'f', '1', '6', 0x2026 };
        short utf16simple[] = {'u', 't', 'f', '1', '6', '!' };

        makePath(path);
        SkTDArray<SkPoint>(pos);
        pos.setCount(asciiLength);
        for (index = 0;  index < asciiLength; index++)
            pos[index].set(SkIntToScalar((unsigned int)index * 10),
                                       SkIntToScalar((unsigned int)index * 2));
        SkTDArray<SkPoint>(pos2);
        pos2.setCount(asciiLength);
        for (index = 0;  index < asciiLength; index++)
            pos2[index].set(SkIntToScalar((unsigned int)index * 10),
                                        SkIntToScalar(20));

        // shaders
        SkPoint linearPoints[] = { { 0, 0, }, { SkIntToScalar(40), SkIntToScalar(40) } };
        SkColor linearColors[] = { SK_ColorRED, SK_ColorBLUE };
        SkScalar* linearPos = nullptr;
        int linearCount = 2;
        SkShader::TileMode linearMode = SkShader::kMirror_TileMode;
        SkShader* linear = SkGradientShader::CreateLinear(linearPoints,
            linearColors, linearPos, linearCount, linearMode);

        SkPoint radialCenter = { SkIntToScalar(25), SkIntToScalar(25) };
        SkScalar radialRadius = SkIntToScalar(25);
        SkColor radialColors[] = { SK_ColorGREEN, SK_ColorGRAY, SK_ColorRED };
        SkScalar radialPos[] = { 0, SkIntToScalar(3) / 5, SkIntToScalar(1)};
        int radialCount = 3;
        SkShader::TileMode radialMode = SkShader::kRepeat_TileMode;
        SkShader* radial = SkGradientShader::CreateRadial(radialCenter,
            radialRadius, radialColors, radialPos, radialCount,
            radialMode);

        SkEmbossMaskFilter::Light light;
        light.fDirection[0] = SK_Scalar1/2;
        light.fDirection[1] = SK_Scalar1/2;
        light.fDirection[2] = SK_Scalar1/3;
        light.fAmbient        = 0x48;
        light.fSpecular        = 0x80;
        SkScalar sigma = SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(12)/5);
        SkMaskFilter* embossFilter = SkEmbossMaskFilter::Create(sigma, light);

        SkXfermode* xfermode = SkXfermode::Create(SkXfermode::kXor_Mode);
        SkColorFilter* lightingFilter = SkColorMatrixFilter::CreateLightingFilter(
            0xff89bc45, 0xff112233);

        canvas->save();
        canvas->translate(SkIntToScalar(0), SkIntToScalar(5));
        paint.setAntiAlias(true);
        paint.setFilterQuality(kLow_SkFilterQuality);
        // !!! draw through a clip
        paint.setColor(SK_ColorLTGRAY);
        paint.setStyle(SkPaint::kFill_Style);
        SkRect clip = {0, 0, SkIntToScalar(320), SkIntToScalar(120)};
        canvas->clipRect(clip);
        paint.setShader(SkShader::CreateBitmapShader(fTx,
            SkShader::kMirror_TileMode, SkShader::kRepeat_TileMode))->unref();
        canvas->drawPaint(paint);
        canvas->save();

        // line (exercises xfermode, colorShader, colorFilter, filterShader)
        paint.setColor(SK_ColorGREEN);
        paint.setStrokeWidth(SkIntToScalar(10));
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setXfermode(xfermode)->unref();
        paint.setColorFilter(lightingFilter)->unref();
        canvas->drawLine(start.fX, start.fY, stop.fX, stop.fY, paint); // should not be green
        paint.setXfermode(nullptr);
        paint.setColorFilter(nullptr);

        // rectangle
        paint.setStyle(SkPaint::kFill_Style);
        canvas->translate(SkIntToScalar(50), 0);
        paint.setColor(SK_ColorYELLOW);
        paint.setShader(linear)->unref();
        paint.setPathEffect(pathEffectTest())->unref();
        canvas->drawRect(rect, paint);
        paint.setPathEffect(nullptr);

        // circle w/ emboss & transparent (exercises 3dshader)
        canvas->translate(SkIntToScalar(50), 0);
        paint.setMaskFilter(embossFilter)->unref();
        canvas->drawOval(rect, paint);
        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));
//        paint.setShader(transparentShader)->unref();
        canvas->drawOval(rect, paint);
        canvas->translate(0, SkIntToScalar(-10));

        // path
        canvas->translate(SkIntToScalar(50), 0);
        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(5));
        paint.setShader(radial)->unref();
        paint.setMaskFilter(nullptr);
        canvas->drawPath(path, paint);

        paint.setShader(nullptr);
        // bitmap
        canvas->translate(SkIntToScalar(50), 0);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawBitmap(fBug, left, top, &paint);

        canvas->translate(-SkIntToScalar(30), SkIntToScalar(30));
        paint.setShader(shaderTest())->unref(); // test compose shader
        canvas->drawRect(rect2, paint);
        paint.setShader(nullptr);

        canvas->restore();
        // text
        canvas->translate(0, SkIntToScalar(60));
        canvas->save();
        paint.setColor(SK_ColorGRAY);
        canvas->drawPosText(ascii, asciiLength, pos.begin(), paint);
        canvas->drawPosText(ascii, asciiLength, pos2.begin(), paint);

        canvas->translate(SkIntToScalar(50), 0);
        paint.setColor(SK_ColorCYAN);
        canvas->drawText(utf8, sizeof(utf8) - 1, x, y, paint);

        canvas->translate(SkIntToScalar(30), 0);
        paint.setColor(SK_ColorMAGENTA);
        paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
        matrix.setTranslate(SkIntToScalar(10), SkIntToScalar(10));
        canvas->drawTextOnPath((void*) utf16, sizeof(utf16), path, &matrix, paint);
        canvas->translate(0, SkIntToScalar(20));
        canvas->drawTextOnPath((void*) utf16simple, sizeof(utf16simple), path, &matrix, paint);
        canvas->restore();

        canvas->translate(0, SkIntToScalar(60));
        paint.setTextEncoding(SkPaint::kUTF8_TextEncoding);
        canvas->restore();
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) {
        fClickPt.set(x, y);
        this->inval(nullptr);
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    SkPathEffect* pathEffectTest() {
        static const int gXY[] = { 1, 0, 0, -1, 2, -1, 3, 0, 2, 1, 0, 1 };
        SkScalar gPhase = 0;
        SkPath path;
        path.moveTo(SkIntToScalar(gXY[0]), SkIntToScalar(gXY[1]));
        for (unsigned i = 2; i < SK_ARRAY_COUNT(gXY); i += 2)
            path.lineTo(SkIntToScalar(gXY[i]), SkIntToScalar(gXY[i+1]));
        path.close();
        path.offset(SkIntToScalar(-6), 0);
        SkPathEffect* outer = SkPath1DPathEffect::Create(path, SkIntToScalar(12),
            gPhase, SkPath1DPathEffect::kRotate_Style);
        SkPathEffect* inner = SkDiscretePathEffect::Create(SkIntToScalar(2),
            SkIntToScalar(1)/10); // SkCornerPathEffect(SkIntToScalar(2));
        SkPathEffect* result = SkComposePathEffect::Create(outer, inner);
        outer->unref();
        inner->unref();
        return result;
    }

    SkShader* shaderTest() {
        SkPoint pts[] = { { 0, 0, }, { SkIntToScalar(100), 0 } };
        SkColor colors[] = { SK_ColorRED, SK_ColorBLUE };
        SkShader* shaderA = SkGradientShader::CreateLinear(pts, colors, nullptr,
            2, SkShader::kClamp_TileMode);
        pts[1].set(0, SkIntToScalar(100));
        SkColor colors2[] = {SK_ColorBLACK,  SkColorSetARGB(0x80, 0, 0, 0)};
        SkShader* shaderB = SkGradientShader::CreateLinear(pts, colors2, nullptr,
            2, SkShader::kClamp_TileMode);
        SkXfermode* mode = SkXfermode::Create(SkXfermode::kDstIn_Mode);
        SkShader* result = new SkComposeShader(shaderA, shaderB, mode);
        shaderA->unref();
        shaderB->unref();
        mode->unref();
        return result;
    }

    virtual void startTest() {
        SkImageDecoder::DecodeFile("/Users/caryclark/Desktop/bugcirc.gif", &fBug);
        SkImageDecoder::DecodeFile("/Users/caryclark/Desktop/tbcirc.gif", &fTb);
        SkImageDecoder::DecodeFile("/Users/caryclark/Desktop/05psp04.gif", &fTx);
    }

    void drawRaster(SkCanvas* canvas)  {
        for (size_t index = 0; index < SK_ARRAY_COUNT(gRastProcs); index++)
            drawOneRaster(canvas);
    }

    void drawOneRaster(SkCanvas* canvas) {
        canvas->save();

        SkScalar    x = SkIntToScalar(20);
        SkScalar    y = SkIntToScalar(40);
        SkPaint     paint;

        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(48));
        paint.setTypeface(SkTypeface::CreateFromName("sans-serif",
                                                     SkTypeface::kBold));

        SkString str("GOOGLE");

        for (size_t i = 0; i < SK_ARRAY_COUNT(gRastProcs); i++) {
            apply_shader(&paint, (int)i);

          //  paint.setMaskFilter(nullptr);
          //  paint.setColor(SK_ColorBLACK);

#if 01
            int index = i % SK_ARRAY_COUNT(gLightingColors);
            paint.setColorFilter(SkColorMatrixFilter::CreateLightingFilter(
                                    gLightingColors[index].fMul,
                                    gLightingColors[index].fAdd))->unref();
#endif

            canvas->drawText(str.c_str(), str.size(), x, y, paint);
            SkRect  oval = { x, y - SkIntToScalar(40), x + SkIntToScalar(40), y };
            paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawOval(oval, paint);
            paint.setStyle(SkPaint::kFill_Style);

            y += paint.getFontSpacing();
        }

        canvas->restore();
    }

private:
    SkPoint fClickPt;
    SkBitmap fBug, fTb, fTx;
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DemoView; }
static SkViewRegister reg(MyFactory);

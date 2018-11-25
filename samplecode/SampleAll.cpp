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
#include "SkMath.h"
#include "SkPath.h"
#include "SkPictureRecorder.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkCornerPathEffect.h"
#include "SkPathMeasure.h"
#include "SkPicture.h"
#include "SkRandom.h"
#include "SkTypeface.h"
#include "SkUtils.h"

#include <math.h>
#include "DecodeFile.h"

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
            sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

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
        SkTDArray<SkPoint> pos;
        pos.setCount(asciiLength);
        for (index = 0;  index < asciiLength; index++)
            pos[index].set(SkIntToScalar((unsigned int)index * 10),
                                       SkIntToScalar((unsigned int)index * 2));
        SkTDArray<SkPoint> pos2;
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
        auto linear = SkGradientShader::MakeLinear(linearPoints,
            linearColors, linearPos, linearCount, linearMode);

        SkPoint radialCenter = { SkIntToScalar(25), SkIntToScalar(25) };
        SkScalar radialRadius = SkIntToScalar(25);
        SkColor radialColors[] = { SK_ColorGREEN, SK_ColorGRAY, SK_ColorRED };
        SkScalar radialPos[] = { 0, SkIntToScalar(3) / 5, SkIntToScalar(1)};
        int radialCount = 3;
        SkShader::TileMode radialMode = SkShader::kRepeat_TileMode;
        auto radial = SkGradientShader::MakeRadial(radialCenter,
            radialRadius, radialColors, radialPos, radialCount,
            radialMode);

        SkEmbossMaskFilter::Light light;
        light.fDirection[0] = SK_Scalar1/2;
        light.fDirection[1] = SK_Scalar1/2;
        light.fDirection[2] = SK_Scalar1/3;
        light.fAmbient        = 0x48;
        light.fSpecular        = 0x80;

        auto lightingFilter = SkColorMatrixFilter::MakeLightingFilter(
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
        paint.setShader(SkShader::MakeBitmapShader(fTx,
            SkShader::kMirror_TileMode, SkShader::kRepeat_TileMode));
        canvas->drawPaint(paint);
        canvas->save();

        // line (exercises xfermode, colorShader, colorFilter, filterShader)
        paint.setColor(SK_ColorGREEN);
        paint.setStrokeWidth(SkIntToScalar(10));
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setBlendMode(SkBlendMode::kXor);
        paint.setColorFilter(lightingFilter);
        canvas->drawLine(start, stop, paint); // should not be green
        paint.setBlendMode(SkBlendMode::kSrcOver);
        paint.setColorFilter(nullptr);

        // rectangle
        paint.setStyle(SkPaint::kFill_Style);
        canvas->translate(SkIntToScalar(50), 0);
        paint.setColor(SK_ColorYELLOW);
        paint.setShader(linear);
        paint.setPathEffect(pathEffectTest());
        canvas->drawRect(rect, paint);
        paint.setPathEffect(nullptr);

        // circle w/ emboss & transparent (exercises 3dshader)
        canvas->translate(SkIntToScalar(50), 0);
        paint.setMaskFilter(SkEmbossMaskFilter::Make(
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(12)/5), light));
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
        paint.setShader(radial);
        paint.setMaskFilter(nullptr);
        canvas->drawPath(path, paint);

        paint.setShader(nullptr);
        // bitmap
        canvas->translate(SkIntToScalar(50), 0);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawBitmap(fBug, left, top, &paint);

        canvas->translate(-SkIntToScalar(30), SkIntToScalar(30));
        paint.setShader(shaderTest()); // test compose shader
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
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    sk_sp<SkPathEffect> pathEffectTest() {
        static const int gXY[] = { 1, 0, 0, -1, 2, -1, 3, 0, 2, 1, 0, 1 };
        SkScalar gPhase = 0;
        SkPath path;
        path.moveTo(SkIntToScalar(gXY[0]), SkIntToScalar(gXY[1]));
        for (unsigned i = 2; i < SK_ARRAY_COUNT(gXY); i += 2)
            path.lineTo(SkIntToScalar(gXY[i]), SkIntToScalar(gXY[i+1]));
        path.close();
        path.offset(SkIntToScalar(-6), 0);
        auto outer = SkPath1DPathEffect::Make(path, SkIntToScalar(12),
            gPhase, SkPath1DPathEffect::kRotate_Style);
        auto inner = SkDiscretePathEffect::Make(SkIntToScalar(2),
            SkIntToScalar(1)/10); // SkCornerPathEffect(SkIntToScalar(2));
        return SkPathEffect::MakeCompose(outer, inner);
    }

    sk_sp<SkShader> shaderTest() {
        SkPoint pts[] = { { 0, 0, }, { SkIntToScalar(100), 0 } };
        SkColor colors[] = { SK_ColorRED, SK_ColorBLUE };
        auto shaderA = SkGradientShader::MakeLinear(pts, colors, nullptr,
            2, SkShader::kClamp_TileMode);
        pts[1].set(0, SkIntToScalar(100));
        SkColor colors2[] = {SK_ColorBLACK,  SkColorSetARGB(0x80, 0, 0, 0)};
        auto shaderB = SkGradientShader::MakeLinear(pts, colors2, nullptr,
            2, SkShader::kClamp_TileMode);
        return SkShader::MakeComposeShader(std::move(shaderA), std::move(shaderB),
                                           SkBlendMode::kDstIn);
    }

    virtual void startTest() {
        decode_file("/Users/caryclark/Desktop/bugcirc.gif", &fBug);
        decode_file("/Users/caryclark/Desktop/tbcirc.gif", &fTb);
        decode_file("/Users/caryclark/Desktop/05psp04.gif", &fTx);
    }

private:
    SkPoint fClickPt;
    SkBitmap fBug, fTb, fTx;
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DemoView; }
static SkViewRegister reg(MyFactory);

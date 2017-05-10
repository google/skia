/*
 * Copyright 2012 Intel Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkBlurDrawLooper.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
#include "SkMatrix.h"
#include "SkRandom.h"
#include "SkTArray.h"

namespace skiagm {

class CircleGM : public GM {
public:
    CircleGM() {
        this->setBGColor(0xFF000000);
        this->makePaints();
        this->makeMatrices();
    }

protected:

    SkString onShortName() override {
        return SkString("circles");
    }

    SkISize onISize() override {
        return SkISize::Make(1200, 900);
    }

    void makePaints() {
        {
        // no AA
        SkPaint p;
        fPaints.push_back(p);
        }

        {
        // AA
        SkPaint p;
        p.setAntiAlias(true);
        fPaints.push_back(p);
        }

        {
        // AA with mask filter
        SkPaint p;
        p.setAntiAlias(true);
        p.setMaskFilter(SkBlurMaskFilter::Make(
                               kNormal_SkBlurStyle,
                               SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(5)),
                               SkBlurMaskFilter::kHighQuality_BlurFlag));
        fPaints.push_back(p);
        }

        {
        // AA with radial shader
        SkPaint p;
        p.setAntiAlias(true);
        SkPoint center = SkPoint::Make(SkIntToScalar(40), SkIntToScalar(40));
        SkColor colors[] = { SK_ColorBLUE, SK_ColorRED, SK_ColorGREEN };
        SkScalar pos[] = { 0, SK_ScalarHalf, SK_Scalar1 };
        p.setShader(SkGradientShader::MakeRadial(center, 20, colors, pos, SK_ARRAY_COUNT(colors),
                                                 SkShader::kClamp_TileMode));
        fPaints.push_back(p);
        }

        {
        // AA with blur
        SkPaint p;
        p.setAntiAlias(true);
        p.setLooper(SkBlurDrawLooper::Make(SK_ColorBLUE,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(10)),
                                     SkIntToScalar(5), SkIntToScalar(10)));
        fPaints.push_back(p);
        }

        {
        // AA with stroke style
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(SkIntToScalar(3));
        fPaints.push_back(p);
        }

        {
        // AA with stroke style, width = 0
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        fPaints.push_back(p);
        }

        {
        // AA with stroke and fill style
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStrokeAndFill_Style);
        p.setStrokeWidth(SkIntToScalar(2));
        fPaints.push_back(p);
        }
    }

    void makeMatrices() {
        {
        SkMatrix m;
        m.setScale(SkIntToScalar(2), SkIntToScalar(3));
        fMatrices.push_back(m);
        }

        {
        SkMatrix m;
        m.setScale(SkIntToScalar(2), SkIntToScalar(2));
        fMatrices.push_back(m);
        }

        {
        SkMatrix m;
        m.setSkew(SkIntToScalar(2), SkIntToScalar(3));
        fMatrices.push_back(m);
        }

        {
        SkMatrix m;
        m.setSkew(SkIntToScalar(2), SkIntToScalar(2));
        fMatrices.push_back(m);
        }

        {
        SkMatrix m;
        m.setRotate(SkIntToScalar(30));
        fMatrices.push_back(m);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        // Draw a giant AA circle as the background.
        SkISize size = this->getISize();
        SkScalar giantRadius = SkTMin(SkIntToScalar(size.fWidth),
                                      SkIntToScalar(size.fHeight)) / 2.f;
        SkPoint giantCenter = SkPoint::Make(SkIntToScalar(size.fWidth/2),
                                            SkIntToScalar(size.fHeight/2));
        SkPaint giantPaint;
        giantPaint.setAntiAlias(true);
        giantPaint.setColor(0x80808080);
        canvas->drawCircle(giantCenter.fX, giantCenter.fY, giantRadius, giantPaint);

        SkRandom rand;
        canvas->translate(20 * SK_Scalar1, 20 * SK_Scalar1);
        int i;
        for (i = 0; i < fPaints.count(); ++i) {
            canvas->save();
            // position the path, and make it at off-integer coords.
            canvas->translate(SK_Scalar1 * 200 * (i % 5) + SK_Scalar1 / 4,
                              SK_Scalar1 * 200 * (i / 5) + 3 * SK_Scalar1 / 4);
            SkColor color = rand.nextU();
            color |= 0xff000000;
            fPaints[i].setColor(color);

            canvas->drawCircle(SkIntToScalar(40), SkIntToScalar(40),
                               SkIntToScalar(20),
                               fPaints[i]);
            canvas->restore();
        }

        for (int j = 0; j < fMatrices.count(); ++j, ++i) {
            canvas->save();

            canvas->translate(SK_Scalar1 * 200 * (i % 5) + SK_Scalar1 / 4,
                              SK_Scalar1 * 200 * (i / 5) + 3 * SK_Scalar1 / 4);

            canvas->concat(fMatrices[j]);

            SkPaint paint;
            paint.setAntiAlias(true);

            SkColor color = rand.nextU();
            color |= 0xff000000;
            paint.setColor(color);

            canvas->drawCircle(SkIntToScalar(40), SkIntToScalar(40),
                               SkIntToScalar(20),
                               paint);

            canvas->restore();
        }
    }

private:
    typedef GM INHERITED;
    SkTArray<SkPaint> fPaints;
    SkTArray<SkMatrix> fMatrices;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new CircleGM; }
static GMRegistry reg(MyFactory);

}

DEF_SIMPLE_GM(readpixels, canvas, 256,256) {
    canvas->clear(0x8055aaff);
    for (SkAlphaType alphaType : { kPremul_SkAlphaType, kUnpremul_SkAlphaType } ) {
        uint32_t pixel = 0;
        SkImageInfo info = SkImageInfo::Make(1, 1, kBGRA_8888_SkColorType, alphaType);
        if (canvas->readPixels(info, &pixel, 4, 0, 0)) {
            SkDebugf("pixel = %08x\n", pixel);
        }
    }
}

DEF_SIMPLE_GM(readpixels2, canvas, 256,256) {
    canvas->clear(0x8055aaff);
    SkBitmap bitmap;
    bitmap.setInfo(canvas->imageInfo());
    canvas->readPixels(bitmap, 1, 1);
    SkDebugf("pixel = %08x\n", bitmap.getAddr32(0, 0)[0]);
 }

DEF_SIMPLE_GM(writepixels, canvas, 256,256) {
    SkImageInfo imageInfo = SkImageInfo::MakeN32Premul(2, 2);
    SkBitmap bitmap;
    bitmap.setInfo(imageInfo);
    uint32_t pixels[] = {0x3399dd33, 0x6699dd33, 0x9999dd33, 0xCC99dd33}; 
    bitmap.setPixels(pixels);
    for (int y = 0; y < 256; y += 2) {
    for (int x = 0; x < 256;  x += 2) {

    canvas->writePixels(bitmap, x, y);
}
}
    canvas->drawColor(0x80eebb99);
}

DEF_SIMPLE_GM(savelcd, canvas, 256,256) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setLCDRenderText(true);
    paint.setTextSize(20);
    canvas->clear(SK_ColorWHITE);
    for (auto preserve : { false, true } ) {
        preserve ? canvas->saveLayerPreserveLCDTextRequests(nullptr, nullptr)
                 : canvas->saveLayer(nullptr, nullptr);
            canvas->drawText("Hamburgefons", 12, 30, 60, paint);

        SkPaint p;
        p.setColor(0xFFCCCCCC);
        canvas->drawRect(SkRect::MakeLTRB(25, 70, 200, 100), p);
        canvas->drawText("Hamburgefons", 12, 30, 90, paint);

        canvas->restore();
        canvas->translate(0, 80);
    }
}

DEF_SIMPLE_GM(accessTopLayerPixels, canvas, 256,256) {
  SkPaint paint;
  paint.setTextSize(100);
  canvas->clear(SK_ColorWHITE);
  canvas->drawText("ABC", 3, 20, 160, paint);
  SkRect layerBounds = SkRect::MakeXYWH(96, 96, 64, 64);
  canvas->saveLayerAlpha(&layerBounds, 160);
  canvas->clear(SK_ColorWHITE);
  canvas->drawText("DEF", 3, 20, 160, paint);
  SkImageInfo imageInfo;
  size_t rowBytes;
  SkIPoint origin;
  uint32_t* access = (uint32_t*) canvas->accessTopLayerPixels(&imageInfo, &rowBytes, &origin);
  if (access) {
    int h = imageInfo.height();
    int v = imageInfo.width();
    int rowWords = rowBytes / sizeof(uint32_t);
    for (int y = 0; y < h; ++y) {
        int newY = (y - h / 2) * 2 + h / 2;
        if (newY < 0 || newY >= h) {
            continue;
        }
        for (int x = 0; x < v; ++x) {
            int newX = (x - v / 2) * 2 + v / 2;
            if (newX < 0 || newX >= v) {
                continue;
            }
            if (access[y * rowWords + x] == SK_ColorBLACK) {
                access[newY * rowWords + newX] = SK_ColorGRAY;
            }
        }
    }

  }
  canvas->restore();
}

#include "SkColorMatrixFilter.h"
#include "SkImageFilter.h"
#include "SkLayerDrawLooper.h"
#include "SkLayerRasterizer.h"
#include "SkRegion.h"

DEF_SIMPLE_GM(everydarnthing, canvas, 256, 256) {
    SkLayerDrawLooper::LayerInfo info;
    info.fPaintBits = (SkLayerDrawLooper::BitFlags) SkLayerDrawLooper::kColorFilter_Bit;
    info.fColorMode = SkBlendMode::kSrc;
    SkLayerDrawLooper::Builder looperBuilder;
    SkPaint* loopPaint = looperBuilder.addLayer(info);
    loopPaint->setColor(SK_ColorRED);
    info.fOffset.set(20, 20);
    loopPaint = looperBuilder.addLayer(info);
    loopPaint->setColor(SK_ColorBLUE);
    SkPaint paint;
    paint.setDrawLooper(looperBuilder.detach());
    loopPaint->setMaskFilter(SkBlurMaskFilter::Make(kSolid_SkBlurStyle, 3));
    loopPaint->setColorFilter(SkColorMatrixFilter::MakeLightingFilter(0xFFFFFF, 0xFF0000));
    canvas->drawCircle(50, 50, 50, paint);
    canvas->translate(0, 120);
}

DEF_SIMPLE_GM(colorfilter, canvas, 256, 256) {
    SkPaint paint;
    paint.setColorFilter(SkColorMatrixFilter::MakeLightingFilter(0xFFFFFF, 0xFF0000));
    for (SkColor c : { SK_ColorBLACK, SK_ColorGREEN } ) {
        paint.setColor(c);
        canvas->drawRect(SkRect::MakeXYWH(50, 50, 50, 50), paint);
        paint.setAlpha(0x80);
        canvas->drawRect(SkRect::MakeXYWH(100, 100, 50, 50), paint);
        canvas->translate(100, 0);
    }
}

DEF_SIMPLE_GM(rastermask, canvas, 256, 256) {
        SkLayerRasterizer::Builder layerBuilder;
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2);
        layerBuilder.addLayer(paint);
        paint.reset();
        paint.setAntiAlias(true);
        paint.setTextSize(50);
        paint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, 3));
        paint.setRasterizer(layerBuilder.detach());
        canvas->clear(SK_ColorWHITE);
        canvas->drawText("blurry out", 10, 0, 50, paint);
}

DEF_SIMPLE_GM(rastermask2, canvas, 256, 256) {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2);
        SkRegion region;
        region.op( 10, 10, 50, 50, SkRegion::kUnion_Op);
        region.op( 10, 50, 90, 90, SkRegion::kUnion_Op);
        canvas->clear(SK_ColorWHITE);
        paint.setImageFilter(SkImageFilter::MakeBlur(5.0f, 5.0f, nullptr));
        canvas->drawRegion(region, paint);
        paint.setImageFilter(nullptr);
        paint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, 5));
        canvas->translate(100, 100);
        canvas->drawRegion(region, paint);
}

DEF_SIMPLE_GM(lottafilters, canvas, 256, 256) {
        SkPaint paint;
     paint.setColorFilter(SkColorMatrixFilter::MakeLightingFilter(0x0000FF, 0xFF0000));
   SkPoint center = { 50, 50 };
   SkScalar radius = 50;
   const SkColor colors[] = { 0xFF00FFFF, 0xFFFFFF00 };
    paint.setShader(SkGradientShader::MakeRadial(center, radius, colors,
        nullptr, SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode));
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2);
        SkRegion region;
        region.op( 10, 10, 50, 50, SkRegion::kUnion_Op);
        region.op( 10, 50, 90, 90, SkRegion::kUnion_Op);
        canvas->clear(SK_ColorWHITE);
        paint.setImageFilter(SkImageFilter::MakeBlur(5.0f, 5.0f, nullptr));
        canvas->drawRegion(region, paint);
        paint.setImageFilter(nullptr);
        paint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, 5));
        canvas->translate(0, 110);
        canvas->drawRegion(region, paint);
        canvas->translate(110, 0);
        paint.setImageFilter(SkImageFilter::MakeBlur(5.0f, 5.0f, nullptr));
        canvas->drawRegion(region, paint);
}


DEF_SIMPLE_GM(restoreloop, canvas, 256, 256) {
    canvas->clear(SK_ColorWHITE);
    SkPaint redPaint, bluePaint;
    redPaint.setAntiAlias(true);
    redPaint.setColor(SK_ColorRED);
    bluePaint.setColor(SK_ColorBLUE);
    for (int i = 0; i < 2; ++i) {
    canvas->drawCircle(21, 21, 8, redPaint);
    canvas->drawCircle(31, 21, 8, bluePaint);
    SkMatrix matrix;
    matrix.setScale(4, 4);
    auto scaler = SkImageFilter::MakeMatrixFilter(matrix, kNone_SkFilterQuality, nullptr);
        SkLayerDrawLooper::LayerInfo info;
        info.fPaintBits = (SkLayerDrawLooper::BitFlags) SkLayerDrawLooper::kColorFilter_Bit;
        info.fColorMode = SkBlendMode::kSrc;
        info.fOffset.set(10, 0);
        SkLayerDrawLooper::Builder looperBuilder;
        SkPaint* loopPaint = looperBuilder.addLayer(info);
        loopPaint->setAlpha(64);
        if (i == 0) {
        info.fOffset.set(0, -10);
        loopPaint = looperBuilder.addLayer(info);
        loopPaint->setAlpha(64);
        }
        SkPaint paint;
        paint.setDrawLooper(looperBuilder.detach());
        SkRect bounds = { 0, 0, 250, 250 };
    SkCanvas::SaveLayerRec saveLayerRec(&bounds, &paint, scaler.get(), 0); 
    canvas->saveLayer(saveLayerRec);
    canvas->drawCircle(125, 85, 8, redPaint);
    canvas->restore();
    canvas->translate(250, 0);
    }
}

#include "SkPath.h"

DEF_SIMPLE_GM(localbounds, canvas, 256, 256) {
    SkCanvas local(256, 256);
  //  canvas = &local;
    SkRect bounds = canvas->getLocalClipBounds();
    SkDebugf("left:%g  top:%g  right:%g  bottom:%g\n",
            bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
    SkPoint clipPoints[]  = {{30, 130}, {120, 130}, {120, 230} }; 
    SkPath clipPath;
    clipPath.addPoly(clipPoints, SK_ARRAY_COUNT(clipPoints), true);
    canvas->clipPath(clipPath);
    bounds = canvas->getLocalClipBounds();
    SkDebugf("left:%g  top:%g  right:%g  bottom:%g\n",
            bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
}

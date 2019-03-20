/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGradientShader.h"
#include "SkImage.h"
#include "SkPath.h"
#include "SkSurface.h"
#include "ToolUtils.h"
#include "gm.h"

static sk_sp<SkImage> make_image(SkCanvas* origCanvas, int w, int h) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);
    auto        surface(ToolUtils::makeSurface(origCanvas, info));
    SkCanvas* canvas = surface->getCanvas();

    ToolUtils::draw_checkerboard(canvas, SK_ColorRED, SK_ColorGREEN, w / 10);
    return surface->makeImageSnapshot();
}

namespace skiagm {

class PerspShadersGM : public GM {
public:
    PerspShadersGM(bool doAA) : fDoAA(doAA) { }

protected:
    SkString onShortName() override {
        SkString name;
        name.printf("persp_shaders_%s",
                     fDoAA ? "aa" : "bw");
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(kCellSize*kNumCols, kCellSize*kNumRows);
    }

    void onOnceBeforeDraw() override {
        fBitmap = ToolUtils::create_checkerboard_bitmap(
                kCellSize, kCellSize, SK_ColorBLUE, SK_ColorYELLOW, kCellSize / 10);

        fBitmapShader = SkShader::MakeBitmapShader(fBitmap, SkShader::kClamp_TileMode,
                                                   SkShader::kClamp_TileMode);
        SkPoint pts1[] = {
            { 0, 0 },
            { SkIntToScalar(kCellSize), SkIntToScalar(kCellSize) }
        };
        SkPoint pts2[] = {
            { 0, 0 },
            { 0, SkIntToScalar(kCellSize) }
        };
        constexpr SkColor colors[] = {
            SK_ColorRED, SK_ColorGREEN, SK_ColorRED, SK_ColorGREEN, SK_ColorRED
        };
        constexpr SkScalar pos[] = { 0, 0.25f, 0.5f, 0.75f, SK_Scalar1 };

        fLinearGrad1 = SkGradientShader::MakeLinear(pts1, colors, pos, SK_ARRAY_COUNT(colors),
                                                    SkShader::kClamp_TileMode);
        fLinearGrad2 = SkGradientShader::MakeLinear(pts2, colors, pos, SK_ARRAY_COUNT(colors),
                                                    SkShader::kClamp_TileMode);

        fPerspMatrix.reset();
        fPerspMatrix.setPerspY(SK_Scalar1 / 50);

        fPath.moveTo(0, 0);
        fPath.lineTo(0, SkIntToScalar(kCellSize));
        fPath.lineTo(kCellSize/2.0f, kCellSize/2.0f);
        fPath.lineTo(SkIntToScalar(kCellSize), SkIntToScalar(kCellSize));
        fPath.lineTo(SkIntToScalar(kCellSize), 0);
        fPath.close();
    }

    void drawRow(SkCanvas* canvas, SkFilterQuality filterQ) {
        SkPaint filterPaint;
        filterPaint.setFilterQuality(filterQ);
        filterPaint.setAntiAlias(fDoAA);

        SkPaint pathPaint;
        pathPaint.setShader(fBitmapShader);
        pathPaint.setFilterQuality(filterQ);
        pathPaint.setAntiAlias(fDoAA);

        SkPaint gradPaint1;
        gradPaint1.setShader(fLinearGrad1);
        gradPaint1.setAntiAlias(fDoAA);
        SkPaint gradPaint2;
        gradPaint2.setShader(fLinearGrad2);
        gradPaint2.setAntiAlias(fDoAA);

        SkRect r = SkRect::MakeWH(SkIntToScalar(kCellSize), SkIntToScalar(kCellSize));

        canvas->save();

        canvas->save();
        canvas->concat(fPerspMatrix);
        canvas->drawBitmapRect(fBitmap, r, &filterPaint);
        canvas->restore();

        canvas->translate(SkIntToScalar(kCellSize), 0);
        canvas->save();
        canvas->concat(fPerspMatrix);
        canvas->drawImage(fImage.get(), 0, 0, &filterPaint);
        canvas->restore();

        canvas->translate(SkIntToScalar(kCellSize), 0);
        canvas->save();
        canvas->concat(fPerspMatrix);
        canvas->drawRect(r, pathPaint);
        canvas->restore();

        canvas->translate(SkIntToScalar(kCellSize), 0);
        canvas->save();
        canvas->concat(fPerspMatrix);
        canvas->drawPath(fPath, pathPaint);
        canvas->restore();

        canvas->translate(SkIntToScalar(kCellSize), 0);
        canvas->save();
        canvas->concat(fPerspMatrix);
        canvas->drawRect(r, gradPaint1);
        canvas->restore();

        canvas->translate(SkIntToScalar(kCellSize), 0);
        canvas->save();
        canvas->concat(fPerspMatrix);
        canvas->drawPath(fPath, gradPaint2);
        canvas->restore();

        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        if (!fImage || !fImage->isValid(canvas->getGrContext())) {
            fImage = make_image(canvas, kCellSize, kCellSize);
        }

        this->drawRow(canvas, kNone_SkFilterQuality);
        canvas->translate(0, SkIntToScalar(kCellSize));
        this->drawRow(canvas, kLow_SkFilterQuality);
        canvas->translate(0, SkIntToScalar(kCellSize));
        this->drawRow(canvas, kMedium_SkFilterQuality);
        canvas->translate(0, SkIntToScalar(kCellSize));
        this->drawRow(canvas, kHigh_SkFilterQuality);
        canvas->translate(0, SkIntToScalar(kCellSize));
    }
private:
    static constexpr int kCellSize = 50;
    static constexpr int kNumRows = 4;
    static constexpr int kNumCols = 6;

    bool            fDoAA;
    SkPath          fPath;
    sk_sp<SkShader> fBitmapShader;
    sk_sp<SkShader> fLinearGrad1;
    sk_sp<SkShader> fLinearGrad2;
    SkMatrix        fPerspMatrix;
    sk_sp<SkImage>  fImage;
    SkBitmap        fBitmap;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new PerspShadersGM(true);)
DEF_GM(return new PerspShadersGM(false);)
}

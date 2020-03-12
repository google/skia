/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "tools/ToolUtils.h"

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
        fBitmap.setImmutable();

        fBitmapShader = fBitmap.makeShader();
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
                                                    SkTileMode::kClamp);
        fLinearGrad2 = SkGradientShader::MakeLinear(pts2, colors, pos, SK_ARRAY_COUNT(colors),
                                                    SkTileMode::kClamp);

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
DEF_GM(return new PerspShadersGM(true);)
DEF_GM(return new PerspShadersGM(false);)
}

//////////////////////////////////////////////////////////////////////////////

#include "tools/Resources.h"

static SkPath make_path() {
    SkRandom rand;
    auto rand_pt = [&rand]() {
        auto x = rand.nextF();
        auto y = rand.nextF();
        return SkPoint{x * 400, y * 400};
    };

    SkPath path;
    for (int i = 0; i < 4; ++i) {
        SkPoint pts[6];
        for (auto& p : pts) {
            p = rand_pt();
        }
        path.moveTo(pts[0]).quadTo(pts[1], pts[2]).quadTo(pts[3], pts[4]).lineTo(pts[5]);
    }
    return path;
}

DEF_SIMPLE_GM(perspective_clip, canvas, 800, 800) {
    SkPath path = make_path();
    auto shader = GetResourceAsImage("images/mandrill_128.png")
                                    ->makeShader(SkMatrix::MakeScale(3, 3));

    SkPaint paint;
    paint.setColor({0.75, 0.75, 0.75, 1});
    canvas->drawPath(path, paint);

    // This is a crazy perspective matrix, derived from halfplanes3, to draw a shape where
    // part of it is "behind" the viewer, hence showing the need for "half-plane" clipping
    // when in perspective.
    SkMatrix mx;
    const SkScalar array[] = {
        -1.7866f,  1.3357f, 273.0295f,
        -1.0820f,  1.3186f, 135.5196f,
        -0.0047f, -0.0015f,  2.1485f,
    };
    mx.set9(array);

    paint.setShader(shader);
    canvas->concat(mx);
    canvas->drawPath(path, paint);
}

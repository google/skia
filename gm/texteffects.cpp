/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkFlattenableBuffers.h"
#include "SkLayerRasterizer.h"

static void r0(SkLayerRasterizer* rast, SkPaint& p) {
    p.setMaskFilter(SkBlurMaskFilter::Create(SkBlurMaskFilter::kNormal_BlurStyle,
                              SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(3))))->unref();
    rast->addLayer(p, SkIntToScalar(3), SkIntToScalar(3));

    p.setMaskFilter(NULL);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rast->addLayer(p);

    p.setAlpha(0x11);
    p.setStyle(SkPaint::kFill_Style);
    p.setXfermodeMode(SkXfermode::kSrc_Mode);
    rast->addLayer(p);
}

static void r1(SkLayerRasterizer* rast, SkPaint& p) {
    rast->addLayer(p);

    p.setAlpha(0x40);
    p.setXfermodeMode(SkXfermode::kSrc_Mode);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*2);
    rast->addLayer(p);
}

static void r2(SkLayerRasterizer* rast, SkPaint& p) {
    p.setStyle(SkPaint::kStrokeAndFill_Style);
    p.setStrokeWidth(SK_Scalar1*4);
    rast->addLayer(p);

    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*3/2);
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rast->addLayer(p);
}

static void r3(SkLayerRasterizer* rast, SkPaint& p) {
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*3);
    rast->addLayer(p);

    p.setAlpha(0x20);
    p.setStyle(SkPaint::kFill_Style);
    p.setXfermodeMode(SkXfermode::kSrc_Mode);
    rast->addLayer(p);
}

static void r4(SkLayerRasterizer* rast, SkPaint& p) {
    p.setAlpha(0x60);
    rast->addLayer(p, SkIntToScalar(3), SkIntToScalar(3));

    p.setAlpha(0xFF);
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rast->addLayer(p, SK_Scalar1*3/2, SK_Scalar1*3/2);

    p.setXfermode(NULL);
    rast->addLayer(p);
}

#include "SkDiscretePathEffect.h"

static void r5(SkLayerRasterizer* rast, SkPaint& p) {
    rast->addLayer(p);

    p.setPathEffect(new SkDiscretePathEffect(SK_Scalar1*4, SK_Scalar1*3))->unref();
    p.setXfermodeMode(SkXfermode::kSrcOut_Mode);
    rast->addLayer(p);
}

static void r6(SkLayerRasterizer* rast, SkPaint& p) {
    rast->addLayer(p);

    p.setAntiAlias(false);
    SkLayerRasterizer* rast2 = new SkLayerRasterizer;
    r5(rast2, p);
    p.setRasterizer(rast2)->unref();
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rast->addLayer(p);
}

#include "Sk2DPathEffect.h"

static SkPathEffect* MakeDotEffect(SkScalar radius, const SkMatrix& matrix) {
    SkPath path;
    path.addCircle(0, 0, radius);
    return new SkPath2DPathEffect(matrix, path);
}

static void r7(SkLayerRasterizer* rast, SkPaint& p) {
    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1*6, SK_Scalar1*6, 0, 0);
    lattice.postSkew(SK_Scalar1/3, 0, 0, 0);
    p.setPathEffect(MakeDotEffect(SK_Scalar1*4, lattice))->unref();
    rast->addLayer(p);
}

static void r8(SkLayerRasterizer* rast, SkPaint& p) {
    rast->addLayer(p);

    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1*6, SK_Scalar1*6, 0, 0);
    lattice.postSkew(SK_Scalar1/3, 0, 0, 0);
    p.setPathEffect(MakeDotEffect(SK_Scalar1*2, lattice))->unref();
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rast->addLayer(p);

    p.setPathEffect(NULL);
    p.setXfermode(NULL);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rast->addLayer(p);
}

static void r9(SkLayerRasterizer* rast, SkPaint& p) {
    rast->addLayer(p);

    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1, SK_Scalar1*6, 0, 0);
    lattice.postRotate(SkIntToScalar(30), 0, 0);
    p.setPathEffect(new SkLine2DPathEffect(SK_Scalar1*2, lattice))->unref();
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rast->addLayer(p);

    p.setPathEffect(NULL);
    p.setXfermode(NULL);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rast->addLayer(p);
}

typedef void (*raster_proc)(SkLayerRasterizer*, SkPaint&);

static const raster_proc gRastProcs[] = {
    r0, r1, r2, r3, r4, r5, r6, r7, r8, r9
};

#include "SkXfermode.h"

static void apply_shader(SkPaint* paint, int index) {
    raster_proc proc = gRastProcs[index];
    if (proc)
    {
        SkPaint p;
        SkLayerRasterizer*  rast = new SkLayerRasterizer;

        p.setAntiAlias(true);
        proc(rast, p);
        paint->setRasterizer(rast)->unref();
    }

#if 0
    SkScalar dir[] = { SK_Scalar1, SK_Scalar1, SK_Scalar1 };
    paint->setMaskFilter(SkBlurMaskFilter::CreateEmboss(dir, SK_Scalar1/4, SkIntToScalar(4), SkIntToScalar(3)))->unref();
#endif
    paint->setColor(SK_ColorBLUE);
}

class TextEffectsGM : public skiagm::GM {
public:
    TextEffectsGM() {}

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("texteffects");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(460, 680);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        canvas->save();

        SkPaint     paint;
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(56));

        SkScalar    x = SkIntToScalar(20);
        SkScalar    y = paint.getTextSize();

        SkString str("Hamburgefons");

        for (int i = 0; i < static_cast<int>(SK_ARRAY_COUNT(gRastProcs)); i++) {
            apply_shader(&paint, i);

            //  paint.setMaskFilter(NULL);
            //  paint.setColor(SK_ColorBLACK);

            canvas->drawText(str.c_str(), str.size(), x, y, paint);

            y += paint.getFontSpacing();
        }

        canvas->restore();
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new TextEffectsGM; }
static skiagm::GMRegistry reg(MyFactory);

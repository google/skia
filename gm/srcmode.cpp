/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkSurface.h"

#if SK_SUPPORT_GPU
    #include "SkGpuDevice.h"
#endif

#define W   SkIntToScalar(80)
#define H   SkIntToScalar(60)

typedef void (*PaintProc)(SkPaint*);

static void identity_paintproc(SkPaint* paint) {
    paint->setShader(NULL);
}

static void gradient_paintproc(SkPaint* paint) {
    const SkColor colors[] = { SK_ColorGREEN, SK_ColorBLUE };
    const SkPoint pts[] = { { 0, 0 }, { W, H } };
    SkShader* s = SkGradientShader::CreateLinear(pts, colors, NULL,
                                                 SK_ARRAY_COUNT(colors),
                                                 SkShader::kClamp_TileMode);
    paint->setShader(s)->unref();
}

typedef void (*Proc)(SkCanvas*, const SkPaint&);

static void draw_hair(SkCanvas* canvas, const SkPaint& paint) {
    SkPaint p(paint);
    p.setStrokeWidth(0);
    canvas->drawLine(0, 0, W, H, p);
}

static void draw_thick(SkCanvas* canvas, const SkPaint& paint) {
    SkPaint p(paint);
    p.setStrokeWidth(H/5);
    canvas->drawLine(0, 0, W, H, p);
}

static void draw_rect(SkCanvas* canvas, const SkPaint& paint) {
    canvas->drawRect(SkRect::MakeWH(W, H), paint);
}

static void draw_oval(SkCanvas* canvas, const SkPaint& paint) {
    canvas->drawOval(SkRect::MakeWH(W, H), paint);
}

static void draw_text(SkCanvas* canvas, const SkPaint& paint) {
    SkPaint p(paint);
    p.setTextSize(H/4);
    canvas->drawText("Hamburge", 8, 0, H*2/3, p);
}

class SrcModeGM : public skiagm::GM {
    SkPath fPath;
public:
    SrcModeGM() {
        this->setBGColor(SK_ColorBLACK);
    }

protected:
    virtual SkString onShortName() {
        return SkString("srcmode");
    }

    virtual SkISize onISize() {
        return SkISize::Make(640, 760);
    }

    void drawContent(SkCanvas* canvas) {
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        SkPaint paint;
        paint.setColor(0x80FF0000);

        const Proc procs[] = {
            draw_hair, draw_thick, draw_rect, draw_oval, draw_text
        };

        const SkXfermode::Mode modes[] = {
            SkXfermode::kSrcOver_Mode, SkXfermode::kSrc_Mode, SkXfermode::kClear_Mode
        };

        const PaintProc paintProcs[] = {
            identity_paintproc, gradient_paintproc
        };

        for (int aa = 0; aa <= 1; ++aa) {
            paint.setAntiAlias(SkToBool(aa));
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(paintProcs); ++i) {
                paintProcs[i](&paint);
                for (size_t x = 0; x < SK_ARRAY_COUNT(modes); ++x) {
                    paint.setXfermodeMode(modes[x]);
                    canvas->save();
                    for (size_t y = 0; y < SK_ARRAY_COUNT(procs); ++y) {
                        procs[y](canvas, paint);
                        canvas->translate(0, H * 5 / 4);
                    }
                    canvas->restore();
                    canvas->translate(W * 5 / 4, 0);
                }
            }
            canvas->restore();
            canvas->translate(0, (H * 5 / 4) * SK_ARRAY_COUNT(procs));
        }
    }

    static SkSurface* compat_surface(SkCanvas* canvas, const SkISize& size,
                                     bool skipGPU) {
        SkImageInfo info = {
            size.width(),
            size.height(),
            kPMColor_SkColorType,
            kPremul_SkAlphaType
        };
#if SK_SUPPORT_GPU
        SkBaseDevice* dev = canvas->getDevice();
        if (!skipGPU && dev->accessRenderTarget()) {
            SkGpuDevice* gd = (SkGpuDevice*)dev;
            GrContext* ctx = gd->context();
            return SkSurface::NewRenderTarget(ctx, info, 0);
        }
#endif
        return SkSurface::NewRaster(info);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkAutoTUnref<SkSurface> surf(compat_surface(canvas, this->getISize(),
                                                    this->isCanvasDeferred()));
        surf->getCanvas()->drawColor(SK_ColorWHITE);
        this->drawContent(surf->getCanvas());
        surf->draw(canvas, 0, 0, NULL);
    }

private:
    typedef skiagm::GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM(return new SrcModeGM;)

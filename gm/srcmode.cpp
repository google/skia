/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkSurface.h"

#define W   SkIntToScalar(80)
#define H   SkIntToScalar(60)

typedef void (*PaintProc)(SkPaint*);

static void identity_paintproc(SkPaint* paint) {
    paint->setShader(nullptr);
}

static void gradient_paintproc(SkPaint* paint) {
    const SkColor colors[] = { SK_ColorGREEN, SK_ColorBLUE };
    const SkPoint pts[] = { { 0, 0 }, { W, H } };
    paint->setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                                                  SkShader::kClamp_TileMode));
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
    canvas->drawString("Hamburge", 0, H*2/3, p);
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
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setColor(0x80F60000);

        const Proc procs[] = {
            draw_hair, draw_thick, draw_rect, draw_oval, draw_text
        };

        const SkBlendMode modes[] = {
            SkBlendMode::kSrcOver, SkBlendMode::kSrc, SkBlendMode::kClear
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
                    paint.setBlendMode(modes[x]);
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

    static sk_sp<SkSurface> compat_surface(SkCanvas* canvas, const SkISize& size, bool skipGPU) {
        SkImageInfo info = SkImageInfo::MakeN32Premul(size);

        bool callNewSurface = true;
#if SK_SUPPORT_GPU
        if (canvas->getGrContext() && skipGPU) {
            callNewSurface = false;
        }
#endif
        sk_sp<SkSurface> surface = callNewSurface ? canvas->makeSurface(info) : nullptr;
        if (nullptr == surface) {
            // picture canvas will return null, so fall-back to raster
            surface = SkSurface::MakeRaster(info);
        }
        return surface;
    }

    virtual void onDraw(SkCanvas* canvas) {
        auto surf(compat_surface(canvas, this->getISize(), this->isCanvasDeferred()));
        surf->getCanvas()->drawColor(SK_ColorWHITE);
        this->drawContent(surf->getCanvas());
        surf->draw(canvas, 0, 0, nullptr);
    }

private:
    typedef skiagm::GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM(return new SrcModeGM;)

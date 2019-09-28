/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
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
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "tools/ToolUtils.h"

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
                                                  SkTileMode::kClamp));
}

typedef void (*Proc)(SkCanvas*, const SkPaint&, const SkFont&);

static void draw_hair(SkCanvas* canvas, const SkPaint& paint, const SkFont&) {
    SkPaint p(paint);
    p.setStrokeWidth(0);
    canvas->drawLine(0, 0, W, H, p);
}

static void draw_thick(SkCanvas* canvas, const SkPaint& paint, const SkFont&) {
    SkPaint p(paint);
    p.setStrokeWidth(H/5);
    canvas->drawLine(0, 0, W, H, p);
}

static void draw_rect(SkCanvas* canvas, const SkPaint& paint, const SkFont&) {
    canvas->drawRect(SkRect::MakeWH(W, H), paint);
}

static void draw_oval(SkCanvas* canvas, const SkPaint& paint, const SkFont&) {
    canvas->drawOval(SkRect::MakeWH(W, H), paint);
}

static void draw_text(SkCanvas* canvas, const SkPaint& paint, const SkFont& font) {
    canvas->drawString("Hamburge", 0, H*2/3, font, paint);
}

class SrcModeGM : public skiagm::GM {
    SkPath fPath;

    void onOnceBeforeDraw() override { this->setBGColor(SK_ColorBLACK); }

    SkString onShortName() override { return SkString("srcmode"); }

    SkISize onISize() override { return {640, 760}; }

    void drawContent(SkCanvas* canvas) {
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        SkPaint paint;
        SkFont  font(ToolUtils::create_portable_typeface(), H / 4);
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
            font.setEdging(SkToBool(aa) ? SkFont::Edging::kAntiAlias : SkFont::Edging::kAlias);
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(paintProcs); ++i) {
                paintProcs[i](&paint);
                for (size_t x = 0; x < SK_ARRAY_COUNT(modes); ++x) {
                    paint.setBlendMode(modes[x]);
                    canvas->save();
                    for (size_t y = 0; y < SK_ARRAY_COUNT(procs); ++y) {
                        procs[y](canvas, paint, font);
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
        if (canvas->getGrContext() && skipGPU) {
            callNewSurface = false;
        }
        sk_sp<SkSurface> surface = callNewSurface ? canvas->makeSurface(info) : nullptr;
        if (nullptr == surface) {
            // picture canvas will return null, so fall-back to raster
            surface = SkSurface::MakeRaster(info);
        }
        return surface;
    }

    void onDraw(SkCanvas* canvas) override {
        auto surf(compat_surface(canvas, this->getISize(), this->isCanvasDeferred()));
        surf->getCanvas()->drawColor(SK_ColorWHITE);
        this->drawContent(surf->getCanvas());
        surf->draw(canvas, 0, 0, nullptr);
    }
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM(return new SrcModeGM;)

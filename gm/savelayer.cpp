/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkShaderMaskFilter.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkCanvasPriv.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <string.h>
#include <initializer_list>

// Test kInitWithPrevious_SaveLayerFlag by drawing an image, save a layer with the flag, which
// should seed the layer with the image (from below). Then we punch a hole in the layer and
// restore with kPlus mode, which should show the mandrill super-bright on the outside, but
// normal where we punched the hole.
DEF_SIMPLE_GM(savelayer_initfromprev, canvas, 256, 256) {
    canvas->drawImage(GetResourceAsImage("images/mandrill_256.png"), 0, 0);

    SkCanvas::SaveLayerRec rec;
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kPlus);
    rec.fSaveLayerFlags = SkCanvas::kInitWithPrevious_SaveLayerFlag;
    rec.fPaint = &paint;
    canvas->saveLayer(rec);
    paint.setBlendMode(SkBlendMode::kClear);
    canvas->drawCircle(128, 128, 96, paint);
    canvas->restore();
};

DEF_SIMPLE_GM(savelayer_coverage, canvas, 500, 500) {
    canvas->saveLayer(nullptr, nullptr);

    SkRect r = { 0, 0, 200, 200 };
    SkPaint layerPaint;
    layerPaint.setBlendMode(SkBlendMode::kModulate);

    auto image = GetResourceAsImage("images/mandrill_128.png");

    auto proc = [layerPaint](SkCanvas* canvas, SkCanvas::SaveLayerRec& rec) {
        SkPaint paint;
        paint.setColor(SK_ColorRED);

        canvas->saveLayer(rec);
        canvas->drawCircle(100, 100, 50, paint);
        paint.setColor(0x8800FF00);
        canvas->drawRect({10, 90, 190, 110}, paint);
        canvas->restore();
    };

    const int yflags[] = { 0, SkCanvas::kInitWithPrevious_SaveLayerFlag };
    for (int y = 0; y <= 1; ++y) {
        const int xflags[] = { 0, SkCanvas::kMaskAgainstCoverage_EXPERIMENTAL_DONT_USE_SaveLayerFlag };
        for (int x = 0; x <= 1; ++x) {
            canvas->save();
            canvas->translate(x * 200.f, y * 200.f);

            SkCanvas::SaveLayerRec rec(&r, &layerPaint, yflags[y] | xflags[x]);
            canvas->drawImageRect(image, r, SkSamplingOptions(), nullptr);
            proc(canvas, rec);

            canvas->restore();
        }
    }

    canvas->restore();
}

static void draw_cell(SkCanvas* canvas, sk_sp<SkTextBlob> blob, SkColor c, SkScalar w, SkScalar h,
                      bool useDrawBehind) {
    SkRect r = SkRect::MakeWH(w, h);
    SkPaint p;
    p.setColor(c);
    p.setBlendMode(SkBlendMode::kSrc);
    canvas->drawRect(r, p);
    p.setBlendMode(SkBlendMode::kSrcOver);

    const SkScalar margin = 80;
    r.fLeft = w - margin;

    // save the behind image
    SkDEBUGCODE(int sc0 =) canvas->getSaveCount();
    SkDEBUGCODE(int sc1 =) SkCanvasPriv::SaveBehind(canvas, &r);
    SkDEBUGCODE(int sc2 =) canvas->getSaveCount();
    SkASSERT(sc0 == sc1);
    SkASSERT(sc0 + 1 == sc2);

    // draw the foreground (including over the 'behind' section)
    p.setColor(SK_ColorBLACK);
    canvas->drawTextBlob(blob, 10, 30, p);

    // draw the treatment
    const SkPoint pts[] = { {r.fLeft,0}, {r.fRight, 0} };
    const SkColor colors[] = { 0x88000000, 0x0 };
    auto sh = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
    p.setShader(sh);
    p.setBlendMode(SkBlendMode::kDstIn);

    if (useDrawBehind) {
        SkCanvasPriv::DrawBehind(canvas, p);
    } else {
        canvas->drawRect(r, p);
    }

    // this should restore the behind image
    canvas->restore();
    SkDEBUGCODE(int sc3 =) canvas->getSaveCount();
    SkASSERT(sc3 == sc0);

    // just outline where we expect the treatment to appear
    p.reset();
    p.setStyle(SkPaint::kStroke_Style);
    p.setAlphaf(0.25f);
}

static void draw_list(SkCanvas* canvas, sk_sp<SkTextBlob> blob, bool useDrawBehind) {
    SkAutoCanvasRestore acr(canvas, true);

    SkRandom rand;
    SkScalar w = 400;
    SkScalar h = 40;
    for (int i = 0; i < 8; ++i) {
        SkColor c = rand.nextU();   // ensure we're opaque
        c = (c & 0xFFFFFF) | 0x80000000;
        draw_cell(canvas, blob, c, w, h, useDrawBehind);
        canvas->translate(0, h);
    }
}

DEF_SIMPLE_GM(save_behind, canvas, 830, 670) {
    SkFont font;
    font.setTypeface(ToolUtils::create_portable_typeface());
    font.setSize(30);

    const char text[] = "This is a very long line of text";
    auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);

    for (bool useDrawBehind : {false, true}) {
        canvas->save();

        draw_list(canvas, blob, useDrawBehind);
        canvas->translate(0, 350);
        canvas->saveLayer({0, 0, 400, 320}, nullptr);
        draw_list(canvas, blob, useDrawBehind);
        canvas->restore();

        canvas->restore();
        canvas->translate(430, 0);
    }
}

#include "include/effects/SkGradientShader.h"

DEF_SIMPLE_GM(savelayer_f16, canvas, 900, 300) {
    int n = 15;
    SkRect r{0, 0, 300, 300};
    SkPaint paint;

    const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorRED };
    paint.setShader(SkGradientShader::MakeSweep(r.centerX(), r.centerY(),
                                                colors, nullptr, SK_ARRAY_COUNT(colors)));

    canvas->drawOval(r, paint);

    paint.setAlphaf(1.0f/n);
    paint.setBlendMode(SkBlendMode::kPlus);

    for (auto flags : {0, (int)SkCanvas::kF16ColorType}) {
        canvas->translate(r.width(), 0);

        SkCanvas::SaveLayerRec rec;
        rec.fSaveLayerFlags = flags;
        canvas->saveLayer(rec);
        for (int i = 0; i < n; ++i) {
            canvas->drawOval(r, paint);
        }
        canvas->restore();
    }
}

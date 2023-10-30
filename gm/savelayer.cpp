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
#include "include/core/SkRSXform.h"
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
#include "include/core/SkVertices.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkShaderMaskFilter.h"
#include "src/base/SkRandom.h"
#include "src/core/SkCanvasPriv.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>
#include <initializer_list>

// Test kInitWithPrevious_SaveLayerFlag by drawing an image, save a layer with the flag, which
// should seed the layer with the image (from below). Then we punch a hole in the layer and
// restore with kPlus mode, which should show the mandrill super-bright on the outside, but
// normal where we punched the hole.
DEF_SIMPLE_GM(savelayer_initfromprev, canvas, 256, 256) {
    canvas->drawImage(ToolUtils::GetResourceAsImage("images/mandrill_256.png"), 0, 0);

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
    SkFont font = ToolUtils::DefaultPortableFont();
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
                                                colors, nullptr, std::size(colors)));

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

static void draw_atlas(SkCanvas* canvas, SkImage* image) {
    SkRSXform xforms[] = {{1, 0, 0, 0}, {1, 0, 50, 50}};
    SkRect tex[] = {{0, 0, 100, 100}, {0, 0, 100, 100}};
    SkColor colors[] = {0xffffffff, 0xffffffff};
    SkPaint paint;

    canvas->drawAtlas(image,
                      xforms,
                      tex,
                      colors,
                      2,
                      SkBlendMode::kSrcIn,
                      SkFilterMode::kNearest,
                      nullptr,
                      &paint);
}

static void draw_vertices(SkCanvas* canvas, SkImage* image) {
    SkPoint pts[] = {{0, 0}, {0, 100}, {100, 100}, {100, 0}, {100, 100}, {0, 100}};
    sk_sp<SkVertices> verts =
            SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode, 6, pts, nullptr, nullptr);

    SkPaint paint;
    paint.setShader(image->makeShader(SkFilterMode::kNearest));

    canvas->drawVertices(verts, SkBlendMode::kSrc, paint);
}

static void draw_points(SkCanvas* canvas, SkImage* image) {
    SkPoint pts[] = {{50, 50}, {75, 75}};
    SkPaint paint;
    paint.setShader(image->makeShader(SkFilterMode::kNearest));
    paint.setStrokeWidth(100);
    paint.setStrokeCap(SkPaint::kSquare_Cap);

    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, pts, paint);
}

static void draw_image_set(SkCanvas* canvas, SkImage* image) {
    SkRect r = SkRect::MakeWH(100, 100);
    SkCanvas::ImageSetEntry entries[] = {
            SkCanvas::ImageSetEntry(sk_ref_sp(image), r, r, 1.0f, SkCanvas::kNone_QuadAAFlags),
            SkCanvas::ImageSetEntry(
                    sk_ref_sp(image), r, r.makeOffset(50, 50), 1.0f, SkCanvas::kNone_QuadAAFlags),
    };

    SkPaint paint;
    canvas->experimental_DrawEdgeAAImageSet(
            entries, 2, nullptr, nullptr, SkFilterMode::kNearest, &paint);
}

/*
  Picture optimization looks for single drawing operations inside a saveLayer with alpha. It tries
  to push the alpha into the drawing operation itself. That's only valid if the draw logically
  touches each pixel once. A handful of draws do not behave like that. They instead act like
  repeated, independent draws. This GM tests this with several operations.
  */
DEF_SIMPLE_GM(skbug_14554, canvas, 310, 630) {
    sk_sp<SkImage> image = ToolUtils::GetResourceAsImage("images/mandrill_128.png");
    SkPictureRecorder rec;

    using DrawProc = void(*)(SkCanvas*, SkImage*);

    for (DrawProc proc : {draw_atlas, draw_vertices, draw_points, draw_image_set}) {
        canvas->save();
        for (bool injectExtraOp : {false, true}) {
            auto c = rec.beginRecording(SkRect::MakeWH(150, 150));
            c->saveLayerAlphaf(nullptr, 0.6f);
            proc(c, image.get());
            // For the second draw of each test-case, we inject an extra (useless) operation, which
            // inhibits the optimization and produces the correct result.
            if (injectExtraOp) {
                c->translate(1, 0);
            }
            c->restore();

            auto pic = rec.finishRecordingAsPicture();

            canvas->drawPicture(pic);
            canvas->translate(160, 0);
        }
        canvas->restore();
        canvas->translate(0, 160);
    }
}

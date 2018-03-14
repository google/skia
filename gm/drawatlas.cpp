/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkAutoMalloc.h"
#include "SkCanvas.h"
#include "SkRSXform.h"
#include "SkSurface.h"
#include "sk_tool_utils.h"

class DrawAtlasGM : public skiagm::GM {
    static sk_sp<SkImage> MakeAtlas(SkCanvas* caller, const SkRect& target) {
        SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
        auto surface(sk_tool_utils::makeSurface(caller, info));
        SkCanvas* canvas = surface->getCanvas();
        // draw red everywhere, but we don't expect to see it in the draw, testing the notion
        // that drawAtlas draws a subset-region of the atlas.
        canvas->clear(SK_ColorRED);

        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kClear);
        SkRect r(target);
        r.inset(-1, -1);
        // zero out a place (with a 1-pixel border) to land our drawing.
        canvas->drawRect(r, paint);
        paint.setBlendMode(SkBlendMode::kSrcOver);
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(true);
        canvas->drawOval(target, paint);
        return surface->makeImageSnapshot();
    }

public:
    DrawAtlasGM() {}

protected:

    SkString onShortName() override {
        return SkString("draw-atlas");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkRect target = { 50, 50, 80, 90 };
        auto atlas = MakeAtlas(canvas, target);

        const struct {
            SkScalar fScale;
            SkScalar fDegrees;
            SkScalar fTx;
            SkScalar fTy;

            void apply(SkRSXform* xform) const {
                const SkScalar rad = SkDegreesToRadians(fDegrees);
                xform->fSCos = fScale * SkScalarCos(rad);
                xform->fSSin = fScale * SkScalarSin(rad);
                xform->fTx   = fTx;
                xform->fTy   = fTy;
            }
        } rec[] = {
            { 1, 0, 10, 10 },       // just translate
            { 2, 0, 110, 10 },      // scale + translate
            { 1, 30, 210, 10 },     // rotate + translate
            { 2, -30, 310, 30 },    // scale + rotate + translate
        };

        const int N = SK_ARRAY_COUNT(rec);
        SkRSXform xform[N];
        SkRect tex[N];
        SkColor colors[N];

        for (int i = 0; i < N; ++i) {
            rec[i].apply(&xform[i]);
            tex[i] = target;
            colors[i] = 0x80FF0000 + (i * 40 * 256);
        }

        SkPaint paint;
        paint.setFilterQuality(kLow_SkFilterQuality);
        paint.setAntiAlias(true);

        canvas->drawAtlas(atlas.get(), xform, tex, N, nullptr, &paint);
        canvas->translate(0, 100);
        canvas->drawAtlas(atlas.get(), xform, tex, colors, N, SkBlendMode::kSrcIn, nullptr, &paint);
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new DrawAtlasGM; )

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkPath.h"
#include "SkPathMeasure.h"

static void draw_text_on_path(SkCanvas* canvas, const void* text, size_t length,
                              const SkPoint xy[], const SkPath& path, const SkPaint& paint,
                              float baseline_offset, bool useRSX) {
    SkPathMeasure meas(path, false);

    int count = paint.countText(text, length);
    size_t size = count * (sizeof(SkRSXform) + sizeof(SkScalar));
    SkAutoSMalloc<512> storage(size);
    SkRSXform* xform = (SkRSXform*)storage.get();
    SkScalar* widths = (SkScalar*)(xform + count);

    // Compute a conservative bounds so we can cull the draw
    const SkRect font = paint.getFontBounds();
    const SkScalar max = SkTMax(SkTMax(SkScalarAbs(font.fLeft), SkScalarAbs(font.fRight)),
                                SkTMax(SkScalarAbs(font.fTop), SkScalarAbs(font.fBottom)));
    const SkRect bounds = path.getBounds().makeOutset(max, max);

    if (useRSX) {
        paint.getTextWidths(text, length, widths);

        for (int i = 0; i < count; ++i) {
            // we want to position each character on the center of its advance
            const SkScalar offset = SkScalarHalf(widths[i]);
            SkPoint pos;
            SkVector tan;
            if (!meas.getPosTan(xy[i].x() + offset, &pos, &tan)) {
                pos = xy[i];
                tan.set(1, 0);
            }
            pos += SkVector::Make(-tan.fY, tan.fX) * baseline_offset;

            xform[i].fSCos = tan.x();
            xform[i].fSSin = tan.y();
            xform[i].fTx   = pos.x() - tan.y() * xy[i].y() - tan.x() * offset;
            xform[i].fTy   = pos.y() + tan.x() * xy[i].y() - tan.y() * offset;
        }

        SkPaint p(paint);
        p.setTextAlign(SkPaint::kLeft_Align);
        canvas->drawTextRSXform(text, length, &xform[0], &bounds, p);
    } else {
        canvas->drawTextOnPathHV(text, length, path, 0, baseline_offset, paint);
    }

    if (false) {
        SkPaint p;
        p.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(bounds, p);
    }
}

static void drawTextPath(SkCanvas* canvas, bool useRSX, bool doStroke) {
    const char text0[] = "ABCDFGHJKLMNOPQRSTUVWXYZ";
    const int N = sizeof(text0) - 1;
    SkPoint pos[N];

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(100);
    if (doStroke) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2.25f);
        paint.setStrokeJoin(SkPaint::kRound_Join);
    }

    SkScalar x = 0;
    for (int i = 0; i < N; ++i) {
        pos[i].set(x, 0);
        x += paint.measureText(&text0[i], 1);
    }

    SkPath path;
    const float baseline_offset = -5;

    const SkPath::Direction dirs[] = {
        SkPath::kCW_Direction, SkPath::kCCW_Direction,
    };
    for (auto d : dirs) {
        path.reset();
        path.addOval(SkRect::MakeXYWH(160, 160, 540, 540), d);
        draw_text_on_path(canvas, text0, N, pos, path, paint, baseline_offset, useRSX);
    }

    paint.reset();
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
}

DEF_SIMPLE_GM(drawTextRSXform, canvas, 860, 860) {
    canvas->scale(0.5f, 0.5f);
    const bool doStroke[] = { false, true };
    for (auto st : doStroke) {
        canvas->save();
        drawTextPath(canvas, false, st);
        canvas->translate(860, 0);
        drawTextPath(canvas, true, st);
        canvas->restore();
        canvas->translate(0, 860);
    }
}

#include "Resources.h"
#include "SkColorFilter.h"
#include "SkVertices.h"

static sk_sp<SkVertices> make_vertices(sk_sp<SkImage> image, const SkRect& r,
                                       SkColor color) {
    SkPoint pos[4];
    r.toQuad(pos);
    SkColor colors[4] = { color, color, color, color };
    return SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode, 4,
                                pos, pos, colors);
}

/*
 *  drawAtlas and drawVertices have several things in common:
 *  - can create compound "shaders", combining texture and colors
 *      - these are combined via an explicit blendmode
 *  - like drawImage, they only respect parts of the paint
 *      - colorfilter, imagefilter, blendmode, alpha
 *
 *  This GM produces a series of pairs of images (atlas | vertices).
 *  Each pair should look the same, and each set shows a different combination
 *  of alpha | colorFilter | mode
 */
DEF_SIMPLE_GM(compare_atlas_vertices, canvas, 560, 585) {
    const SkRect tex = SkRect::MakeWH(128, 128);
    const SkRSXform xform = SkRSXform::Make(1, 0, 0, 0);
    const SkColor color = 0x884488CC;

    auto image = GetResourceAsImage("images/mandrill_128.png");
    auto verts = make_vertices(image, tex, color);
    const sk_sp<SkColorFilter> filters[] = {
        nullptr,
        SkColorFilter::MakeModeFilter(0xFF00FF88, SkBlendMode::kModulate),
    };
    const SkBlendMode modes[] = {
        SkBlendMode::kSrcOver,
        SkBlendMode::kPlus,
    };

    canvas->translate(10, 10);
    SkPaint paint;
    for (SkBlendMode mode : modes) {
        for (int alpha : { 0xFF, 0x7F }) {
            paint.setAlpha(alpha);
            canvas->save();
            for (auto cf : filters) {
                paint.setColorFilter(cf);
                canvas->drawAtlas(image, &xform, &tex, &color, 1,
                                  mode, &tex, &paint);
                canvas->translate(128, 0);
                paint.setShader(image->makeShader());
                canvas->drawVertices(verts, mode, paint);
                paint.setShader(nullptr);
                canvas->translate(145, 0);
            }
            canvas->restore();
            canvas->translate(0, 145);
        }
    }
}

static void draw_text_on_path_rigid(SkCanvas* canvas, const void* text, size_t length,
                                    const SkPath& path, const SkPaint& paint) {
    SkAutoTArray<SkScalar> storage(length * 3);
    SkScalar* widths = storage.get();
    SkPoint* pos = (SkPoint*)(widths + length);
    int count = paint.getTextWidths(text, length, widths, nullptr);

    SkScalar w = 0;
    for (int i = 0; i < count; ++i) {
        pos[i] = { w, 0 };
        w += widths[i];
    }

    if (paint.getTextAlign() != SkPaint::kLeft_Align) {
        SkPathMeasure meas(path, false);
        SkScalar offset = meas.getLength() - w;
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            offset *= 0.5f;
        }
        for (int i = 0; i < count; ++i) {
            pos[i].fX += offset;
        }
    }

    draw_text_on_path(canvas, text, length, pos, path, paint, 0, true);
}

DEF_SIMPLE_GM(textonpath_line, canvas, 400, 400) {
    float deltaFontSize  = 0.9f;
    float centerFontSize = 3.f;

    float pixelFontSize  = 30.f;
    float pixelLineHeight = 40.f;

    SkPaint fontFill;
    fontFill.setAntiAlias(true);
    fontFill.setTextAlign(SkPaint::kCenter_Align);

    SkPath path;

    float w = 400;
    float h = 800;

    float scale = 1.f;

    for(float n = h/2.f; n > pixelFontSize; n -= pixelLineHeight)
        scale /= deltaFontSize;

    for(float y = pixelFontSize; y < h; y += pixelLineHeight)
    {

        float fontSize = centerFontSize / scale;
        float matrixScale = pixelFontSize / fontSize;

        fontFill.setTextSize(fontSize);

        path.rewind();
        path.moveTo(0.f, y / matrixScale);
        path.lineTo(w / matrixScale , y / matrixScale);

        canvas->save();
        canvas->scale(matrixScale, matrixScale);

        SkString str;
        str.printf("Font Size: %10f", fontSize);
        canvas->drawTextOnPathHV(str.c_str(), str.size(), path, 0.f, 0.f, fontFill);

        SkPaint rigidPaint = fontFill;
        rigidPaint.setColor(0x80FF0000);
        draw_text_on_path_rigid(canvas, str.c_str(), str.size(), path, rigidPaint);

        rigidPaint.setColor(0xFF0000FF);
        rigidPaint.setStyle(SkPaint::kStroke_Style);
        canvas->drawPath(path, rigidPaint);

        canvas->restore();

        scale *= deltaFontSize;
    }
}

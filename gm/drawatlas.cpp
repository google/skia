/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathMeasure.h"
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
#include "include/private/base/SkTemplates.h"
#include "src/base/SkAutoMalloc.h"
#include "src/core/SkFontPriv.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <initializer_list>

using namespace skia_private;

class DrawAtlasGM : public skiagm::GM {
    static sk_sp<SkImage> MakeAtlas(SkCanvas* caller, const SkRect& target) {
        SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
        auto        surface(ToolUtils::makeSurface(caller, info));
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
    SkString getName() const override { return SkString("draw-atlas"); }

    SkISize getISize() override { return SkISize::Make(640, 480); }

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

        const int N = std::size(rec);
        SkRSXform xform[N];
        SkRect tex[N];
        SkColor colors[N];

        for (int i = 0; i < N; ++i) {
            rec[i].apply(&xform[i]);
            tex[i] = target;
            colors[i] = 0x80FF0000 + (i * 40 * 256);
        }

        SkPaint paint;
        paint.setAntiAlias(true);
        SkSamplingOptions sampling(SkFilterMode::kLinear);

        canvas->drawAtlas(atlas.get(), xform, tex, nullptr, N, SkBlendMode::kDst,
                          sampling, nullptr, &paint);
        canvas->translate(0, 100);
        canvas->drawAtlas(atlas.get(), xform, tex, colors, N, SkBlendMode::kSrcIn,
                          sampling, nullptr, &paint);
    }

private:
    using INHERITED = GM;
};
DEF_GM( return new DrawAtlasGM; )

///////////////////////////////////////////////////////////////////////////////////////////////////

static void draw_text_on_path(SkCanvas* canvas, const void* text, size_t length,
                              const SkPoint xy[], const SkPath& path, const SkFont& font, const SkPaint& paint,
                              float baseline_offset) {
    SkPathMeasure meas(path, false);

    int count = font.countText(text, length, SkTextEncoding::kUTF8);
    size_t size = count * (sizeof(SkRSXform) + sizeof(SkScalar));
    SkAutoSMalloc<512> storage(size);
    SkRSXform* xform = (SkRSXform*)storage.get();
    SkScalar* widths = (SkScalar*)(xform + count);

    // Compute a conservative bounds so we can cull the draw
    const SkRect fontb = SkFontPriv::GetFontBounds(font);
    const SkScalar max = std::max(std::max(SkScalarAbs(fontb.fLeft), SkScalarAbs(fontb.fRight)),
                                std::max(SkScalarAbs(fontb.fTop), SkScalarAbs(fontb.fBottom)));
    const SkRect bounds = path.getBounds().makeOutset(max, max);

    AutoTArray<SkGlyphID> glyphs(count);
    font.textToGlyphs(text, length, SkTextEncoding::kUTF8, glyphs.get(), count);
    font.getWidths(glyphs.get(), count, widths);

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

    canvas->drawTextBlob(SkTextBlob::MakeFromRSXform(glyphs.get(), count * sizeof(SkGlyphID),
                                         &xform[0], font, SkTextEncoding::kGlyphID),
                         0, 0, paint);

    if (true) {
        SkPaint p;
        p.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(bounds, p);
    }
}

static sk_sp<SkShader> make_shader() {
    SkPoint pts[2] = {{0, 0}, {220, 0}};
    SkColor colors[2] = {SK_ColorRED, SK_ColorBLUE};
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kMirror);
}

static void drawTextPath(SkCanvas* canvas, bool doStroke) {
    const char text0[] = "ABCDFGHJKLMNOPQRSTUVWXYZ";
    const int N = sizeof(text0) - 1;
    SkPoint pos[N];

    SkFont font;
    font.setSize(100);

    SkPaint paint;
    paint.setShader(make_shader());
    paint.setAntiAlias(true);
    if (doStroke) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2.25f);
        paint.setStrokeJoin(SkPaint::kRound_Join);
    }

    SkScalar x = 0;
    for (int i = 0; i < N; ++i) {
        pos[i].set(x, 0);
        x += font.measureText(&text0[i], 1, SkTextEncoding::kUTF8, nullptr, &paint);
    }

    SkPath path;
    const float baseline_offset = -5;

    const SkPathDirection dirs[] = {
        SkPathDirection::kCW, SkPathDirection::kCCW,
    };
    for (auto d : dirs) {
        path.reset();
        path.addOval(SkRect::MakeXYWH(160, 160, 540, 540), d);
        draw_text_on_path(canvas, text0, N, pos, path, font, paint, baseline_offset);
    }

    paint.reset();
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
}

DEF_SIMPLE_GM(drawTextRSXform, canvas, 430, 860) {
    canvas->scale(0.5f, 0.5f);
    const bool doStroke[] = { false, true };
    for (auto st : doStroke) {
        drawTextPath(canvas, st);
        canvas->translate(0, 860);
    }
}

// Exercise xform blob and its bounds
DEF_SIMPLE_GM(blob_rsxform, canvas, 500, 100) {
    SkFont font;
    font.setTypeface(ToolUtils::create_portable_typeface());
    font.setSize(50);

    const char text[] = "CrazyXform";
    constexpr size_t len = sizeof(text) - 1;

    SkRSXform xforms[len];
    SkScalar scale = 1;
    SkScalar x = 0, y = 0;
    for (size_t i = 0; i < len; ++i) {
        scale = SkScalarSin(i * SK_ScalarPI / (len-1)) * 0.75f + 0.5f;
        xforms[i] = SkRSXform::Make(scale, 0, x, y);
        x += 50 * scale;
    }

    auto blob = SkTextBlob::MakeFromRSXform(text, len, xforms, font);

    SkPoint offset = { 20, 70 };
    SkPaint paint;
    paint.setColor(0xFFCCCCCC);
    canvas->drawRect(blob->bounds().makeOffset(offset.fX, offset.fY), paint);
    paint.setColor(SK_ColorBLACK);
    canvas->drawTextBlob(blob, offset.fX, offset.fY, paint);
}

// Exercise xform blob and its tight bounds
DEF_SIMPLE_GM(blob_rsxform_distortable, canvas, 500, 100) {
    sk_sp<SkTypeface> typeface;
    std::unique_ptr<SkStreamAsset> distortable(GetResourceAsStream("fonts/Distortable.ttf"));
    if (distortable) {
        sk_sp<SkFontMgr> fm = SkFontMgr::RefDefault();
        const SkFontArguments::VariationPosition::Coordinate position[] = {
            { SkSetFourByteTag('w','g','h','t'), 1.618033988749895f }
        };
        SkFontArguments params;
        params.setVariationDesignPosition({position, std::size(position)});
        typeface = fm->makeFromStream(std::move(distortable), params);
    }

    SkFont font(typeface, 50);

    const char text[] = "abcabcabc";
    constexpr size_t len = sizeof(text) - 1;

    SkRSXform xforms[len];
    SkScalar scale = 1;
    SkScalar x = 0, y = 0;
    for (size_t i = 0; i < len; ++i) {
        scale = SkScalarSin(i * SK_ScalarPI / (len-1)) * 0.75f + 0.5f;
        xforms[i] = SkRSXform::Make(scale, 0, x, y);
        x += 50 * scale;
    }

    auto blob = SkTextBlob::MakeFromRSXform(text, len, xforms, font);

    SkPoint offset = { 20, 70 };
    SkPaint paint;
    paint.setColor(0xFFCCCCCC);
    canvas->drawRect(blob->bounds().makeOffset(offset.fX, offset.fY), paint);
    paint.setColor(SK_ColorBLACK);
    canvas->drawTextBlob(blob, offset.fX, offset.fY, paint);
}

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
        SkColorFilters::Blend(0xFF00FF88, SkBlendMode::kModulate),
    };
    const SkBlendMode modes[] = {
        SkBlendMode::kSrcOver,
        SkBlendMode::kPlus,
    };

    canvas->translate(10, 10);
    SkPaint paint;
    for (SkBlendMode mode : modes) {
        for (float alpha : { 1.0f, 0.5f }) {
            paint.setAlphaf(alpha);
            canvas->save();
            for (const sk_sp<SkColorFilter>& cf : filters) {
                paint.setColorFilter(cf);
                canvas->drawAtlas(image.get(), &xform, &tex, &color, 1, mode,
                                  SkSamplingOptions(), &tex, &paint);
                canvas->translate(128, 0);
                paint.setShader(image->makeShader(SkSamplingOptions()));
                canvas->drawVertices(verts, mode, paint);
                paint.setShader(nullptr);
                canvas->translate(145, 0);
            }
            canvas->restore();
            canvas->translate(0, 145);
        }
    }
}

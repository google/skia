/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "tools/ToolUtils.h"

// Exercises RSX text blobs + shader with various local matrix combinations.
// Yellow grid should stay aligned for text vs. background.
class RSXShaderGM : public skiagm::GM {
public:
private:
    SkString onShortName() override {
        return SkString("rsx_blob_shader");
    }

    SkISize onISize() override {
        return SkISize::Make(kSZ*kScale*2.1f, kSZ*kScale*2.1f);
    }

    void onOnceBeforeDraw() override {
        const SkFontStyle style(SkFontStyle::kExtraBlack_Weight,
                                SkFontStyle::kNormal_Width,
                                SkFontStyle::kUpright_Slant);
        SkFont font(ToolUtils::create_portable_typeface(nullptr, style), kFontSZ);
        font.setEdging(SkFont::Edging::kAntiAlias);

        static constexpr char txt[] = "TEST";
        SkGlyphID glyphs[16];
        float     widths[16];
        const auto glyph_count = font.textToGlyphs(txt, strlen(txt), SkTextEncoding::kUTF8,
                                                   glyphs, SK_ARRAY_COUNT(glyphs));
        font.getWidths(glyphs, glyph_count, widths);

        SkTextBlobBuilder builder;
        const auto& buf = builder.allocRunRSXform(font, glyph_count);
        std::copy(glyphs, glyphs + glyph_count, buf.glyphs);

        float x = 0;
        for (int i = 0; i < glyph_count; ++i) {
            buf.xforms()[i] = {
                1, 0,
                x, 0,
            };
            x += widths[i];
        }

        fBlob = builder.make();
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(kScale, kScale);
        this->draw_one(canvas,
            {0, 0}, SkMatrix::I(), SkMatrix::I());
        this->draw_one(canvas,
            {kSZ*1.1f, 0}, SkMatrix::Scale(2, 2), SkMatrix::I());
        this->draw_one(canvas,
            {0, kSZ*1.1f}, SkMatrix::I(), SkMatrix::RotateDeg(45));
        this->draw_one(canvas,
            {kSZ*1.1f, kSZ*1.1f}, SkMatrix::Scale(2, 2), SkMatrix::RotateDeg(45));
    }

    void draw_one(SkCanvas* canvas, SkPoint pos, const SkMatrix& lm,
                  const SkMatrix& outer_lm) const {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->translate(pos.fX, pos.fY);

        SkPaint p;
        p.setShader(make_shader(lm, outer_lm));
        p.setAlphaf(0.75f);
        canvas->drawRect(SkRect::MakeWH(kSZ, kSZ), p);

        p.setAlphaf(1);
        canvas->drawTextBlob(fBlob, 0, kFontSZ*1, p);
        canvas->drawTextBlob(fBlob, 0, kFontSZ*2, p);
    }

    static sk_sp<SkShader> make_shader(const SkMatrix& lm, const SkMatrix& outer_lm) {
        static constexpr SkISize kTileSize = { 30, 30 };
        auto surface = SkSurface::MakeRasterN32Premul(kTileSize.width(), kTileSize.height());

        SkPaint p;
        p.setColor(0xffffff00);
        surface->getCanvas()->drawPaint(p);
        p.setColor(0xff008000);
        surface->getCanvas()
               ->drawRect({0, 0, kTileSize.width()*0.9f, kTileSize.height()*0.9f}, p);

        return surface->makeImageSnapshot()
                ->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                             SkSamplingOptions(SkFilterMode::kLinear), &lm)
                ->makeWithLocalMatrix(outer_lm);
    }

    inline static constexpr float kSZ     = 300,
                                  kFontSZ = kSZ * 0.38,
                                  kScale  = 1.4f;

    sk_sp<SkTextBlob> fBlob;
};

DEF_GM(return new RSXShaderGM;)

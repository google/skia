/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPoint.h"
#include "SkShader.h"
#include "SkTextBlob.h"
#include "SkTDArray.h"
#include "SkTypeface.h"

// This GM exercises drawTextBlob offset vs. shader space behavior.
class TextBlobShaderGM : public skiagm::GM {
public:
    TextBlobShaderGM(const char* txt) {
        SkPaint p;
        sk_tool_utils::set_portable_typeface(&p);
        size_t txtLen = strlen(txt);
        fGlyphs.append(p.textToGlyphs(txt, txtLen, nullptr));
        p.textToGlyphs(txt, txtLen, fGlyphs.begin());
    }

protected:

    void onOnceBeforeDraw() override {
        SkPaint p;
        p.setAntiAlias(true);
        p.setSubpixelText(true);
        p.setTextSize(30);
        p.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        sk_tool_utils::set_portable_typeface(&p);

        SkTextBlobBuilder builder;
        int glyphCount = fGlyphs.count();
        const SkTextBlobBuilder::RunBuffer* run;

        run = &builder.allocRun(p, glyphCount, 10, 10, nullptr);
        memcpy(run->glyphs, fGlyphs.begin(), glyphCount * sizeof(uint16_t));

        run = &builder.allocRunPosH(p, glyphCount,  80, nullptr);
        memcpy(run->glyphs, fGlyphs.begin(), glyphCount * sizeof(uint16_t));
        for (int i = 0; i < glyphCount; ++i) {
            run->pos[i] = p.getTextSize() * i * .75f;
        }

        run = &builder.allocRunPos(p, glyphCount, nullptr);
        memcpy(run->glyphs, fGlyphs.begin(), glyphCount * sizeof(uint16_t));
        for (int i = 0; i < glyphCount; ++i) {
            run->pos[i * 2] = p.getTextSize() * i * .75f;
            run->pos[i * 2 + 1] = 150 + 5 * sinf((float)i * 8 / glyphCount);
        }

        fBlob.reset(builder.build());

        SkColor  colors[2];
        colors[0] = SK_ColorRED;
        colors[1] = SK_ColorGREEN;

        SkScalar pos[SK_ARRAY_COUNT(colors)];
        for (unsigned i = 0; i < SK_ARRAY_COUNT(pos); ++i) {
            pos[i] = (float)i / (SK_ARRAY_COUNT(pos) - 1);
        }

        SkISize sz = this->onISize();
        fShader = SkGradientShader::MakeRadial(SkPoint::Make(SkIntToScalar(sz.width() / 2),
                                               SkIntToScalar(sz.height() / 2)),
                                               sz.width() * .66f, colors, pos,
                                               SK_ARRAY_COUNT(colors),
                                               SkShader::kRepeat_TileMode);
    }

    SkString onShortName() override {
        return SkString("textblobshader");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setStyle(SkPaint::kFill_Style);
        p.setShader(fShader);

        SkISize sz = this->onISize();
        static const int kXCount = 4;
        static const int kYCount = 3;
        for (int i = 0; i < kXCount; ++i) {
            for (int j = 0; j < kYCount; ++j) {
                canvas->drawTextBlob(fBlob,
                                     SkIntToScalar(i * sz.width() / kXCount),
                                     SkIntToScalar(j * sz.height() / kYCount),
                                     p);
            }
        }
    }

private:
    SkTDArray<uint16_t>            fGlyphs;
    SkAutoTUnref<const SkTextBlob> fBlob;
    sk_sp<SkShader>                fShader;

    typedef skiagm::GM INHERITED;
};

DEF_GM(return new TextBlobShaderGM("Blobber");)

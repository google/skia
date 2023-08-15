/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkTDArray.h"
#include "tools/ToolUtils.h"

#include <math.h>
#include <string.h>

// This GM exercises drawTextBlob offset vs. shader space behavior.
class TextBlobShaderGM : public skiagm::GM {
public:
    TextBlobShaderGM() {}

private:
    void onOnceBeforeDraw() override {
        {
            SkFont      font(ToolUtils::create_portable_typeface());
            const char* txt = "Blobber";
            size_t txtLen = strlen(txt);
            fGlyphs.append(font.countText(txt, txtLen, SkTextEncoding::kUTF8));
            font.textToGlyphs(txt, txtLen, SkTextEncoding::kUTF8, fGlyphs.begin(), fGlyphs.size());
        }

        SkFont font;
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setSize(30);
        font.setTypeface(ToolUtils::create_portable_typeface());

        SkTextBlobBuilder builder;
        int glyphCount = fGlyphs.size();
        const SkTextBlobBuilder::RunBuffer* run;

        run = &builder.allocRun(font, glyphCount, 10, 10, nullptr);
        memcpy(run->glyphs, fGlyphs.begin(), glyphCount * sizeof(uint16_t));

        run = &builder.allocRunPosH(font, glyphCount,  80, nullptr);
        memcpy(run->glyphs, fGlyphs.begin(), glyphCount * sizeof(uint16_t));
        for (int i = 0; i < glyphCount; ++i) {
            run->pos[i] = font.getSize() * i * .75f;
        }

        run = &builder.allocRunPos(font, glyphCount, nullptr);
        memcpy(run->glyphs, fGlyphs.begin(), glyphCount * sizeof(uint16_t));
        for (int i = 0; i < glyphCount; ++i) {
            run->pos[i * 2] = font.getSize() * i * .75f;
            run->pos[i * 2 + 1] = 150 + 5 * sinf((float)i * 8 / glyphCount);
        }

        fBlob = builder.make();

        SkColor  colors[2];
        colors[0] = SK_ColorRED;
        colors[1] = SK_ColorGREEN;

        SkScalar pos[std::size(colors)];
        for (unsigned i = 0; i < std::size(pos); ++i) {
            pos[i] = (float)i / (std::size(pos) - 1);
        }

        SkISize sz = this->getISize();
        fShader = SkGradientShader::MakeRadial(SkPoint::Make(SkIntToScalar(sz.width() / 2),
                                               SkIntToScalar(sz.height() / 2)),
                                               sz.width() * .66f, colors, pos,
                                               std::size(colors),
                                               SkTileMode::kRepeat);
    }

    SkString getName() const override { return SkString("textblobshader"); }

    SkISize getISize() override { return SkISize::Make(640, 480); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kFill_Style);
        p.setShader(fShader);

        SkISize sz = this->getISize();
        constexpr int kXCount = 4;
        constexpr int kYCount = 3;
        for (int i = 0; i < kXCount; ++i) {
            for (int j = 0; j < kYCount; ++j) {
                canvas->drawTextBlob(fBlob,
                                     SkIntToScalar(i * sz.width() / kXCount),
                                     SkIntToScalar(j * sz.height() / kYCount),
                                     p);
            }
        }
    }

    SkTDArray<uint16_t> fGlyphs;
    sk_sp<SkTextBlob>   fBlob;
    sk_sp<SkShader>     fShader;

    using INHERITED = skiagm::GM;
};

DEF_GM(return new TextBlobShaderGM;)

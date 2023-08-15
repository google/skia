/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// GM to stress the GPU font cache
// It's not necessary to run this with CPU configs

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tools/ToolUtils.h"

using MaskFormat = skgpu::MaskFormat;

static SkScalar draw_string(SkCanvas* canvas, const SkString& text, SkScalar x,
                           SkScalar y, const SkFont& font) {
    SkPaint paint;
    canvas->drawString(text, x, y, font, paint);
    return x + font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8);
}

class FontCacheGM : public skiagm::GM {
public:
    FontCacheGM(GrContextOptions::Enable allowMultipleTextures)
        : fAllowMultipleTextures(allowMultipleTextures) {
        this->setBGColor(SK_ColorLTGRAY);
    }

    void modifyGrContextOptions(GrContextOptions* options) override {
        options->fGlyphCacheTextureMaximumBytes = 0;
        options->fAllowMultipleGlyphCacheTextures = fAllowMultipleTextures;
    }

protected:
    SkString getName() const override {
        SkString name("fontcache");
        if (GrContextOptions::Enable::kYes == fAllowMultipleTextures) {
            name.append("-mt");
        }
        return name;
    }

    SkISize getISize() override { return SkISize::Make(kSize, kSize); }

    void onOnceBeforeDraw() override {
        fTypefaces[0] = ToolUtils::create_portable_typeface("serif", SkFontStyle::Italic());
        fTypefaces[1] = ToolUtils::create_portable_typeface("sans-serif", SkFontStyle::Italic());
        fTypefaces[2] = ToolUtils::create_portable_typeface("serif", SkFontStyle::Normal());
        fTypefaces[3] = ToolUtils::create_portable_typeface("sans-serif", SkFontStyle::Normal());
        fTypefaces[4] = ToolUtils::create_portable_typeface("serif", SkFontStyle::Bold());
        fTypefaces[5] = ToolUtils::create_portable_typeface("sans-serif", SkFontStyle::Bold());
    }

    void onDraw(SkCanvas* canvas) override {
        this->drawText(canvas);
        //  Debugging tool for GPU.
        static const bool kShowAtlas = false;
        if (kShowAtlas) {
            if (auto dContext = GrAsDirectContext(canvas->recordingContext())) {
                auto img = dContext->priv().testingOnly_getFontAtlasImage(MaskFormat::kA8);
                canvas->drawImage(img, 0, 0);
            }
        }
    }

private:
    void drawText(SkCanvas* canvas) {
        static const int kSizes[] = {8, 9, 10, 11, 12, 13, 18, 20, 25};

        static const SkString kTexts[] = {SkString("ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
                                          SkString("abcdefghijklmnopqrstuvwxyz"),
                                          SkString("0123456789"),
                                          SkString("!@#$%^&*()<>[]{}")};
        SkFont font;
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setSubpixel(true);

        static const SkScalar kSubPixelInc = 1 / 2.f;
        SkScalar x = 0;
        SkScalar y = 10;
        SkScalar subpixelX = 0;
        SkScalar subpixelY = 0;
        bool offsetX = true;

        if (GrContextOptions::Enable::kYes == fAllowMultipleTextures) {
            canvas->scale(10, 10);
        }

        do {
            for (auto s : kSizes) {
                auto size = 2 * s;
                font.setSize(size);
                for (const auto& typeface : fTypefaces) {
                    font.setTypeface(typeface);
                    for (const auto& text : kTexts) {
                        x = size + draw_string(canvas, text, x + subpixelX, y + subpixelY, font);
                        x = SkScalarCeilToScalar(x);
                        if (x + 100 > kSize) {
                            x = 0;
                            y += SkScalarCeilToScalar(size + 3);
                            if (y > kSize) {
                                return;
                            }
                        }
                    }
                }
                (offsetX ? subpixelX : subpixelY) += kSubPixelInc;
                offsetX = !offsetX;
            }
        } while (true);
    }

    inline static constexpr SkScalar kSize = 1280;

    GrContextOptions::Enable fAllowMultipleTextures;
    sk_sp<SkTypeface> fTypefaces[6];
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new FontCacheGM(GrContextOptions::Enable::kNo))
DEF_GM(return new FontCacheGM(GrContextOptions::Enable::kYes))

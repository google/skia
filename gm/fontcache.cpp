/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// GM to stress the GPU font cache
// It's not necessary to run this with CPU configs

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrContextOptions.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkImage.h"
#include "SkTypeface.h"
#include "gm.h"
#include "sk_tool_utils.h"

static SkScalar draw_string(SkCanvas* canvas, const SkString& text, SkScalar x,
                           SkScalar y, const SkPaint& paint) {
    canvas->drawString(text, x, y, paint);
    return x + paint.measureText(text.c_str(), text.size());
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
    SkString onShortName() override {
        SkString name("fontcache");
        if (GrContextOptions::Enable::kYes == fAllowMultipleTextures) {
            name.append("-mt");
        }
        return name;
    }

    SkISize onISize() override { return SkISize::Make(kSize, kSize); }

    void onOnceBeforeDraw() override {
        fTypefaces[0] = sk_tool_utils::create_portable_typeface("serif", SkFontStyle::Italic());
        fTypefaces[1] = sk_tool_utils::create_portable_typeface("sans-serif",SkFontStyle::Italic());
        fTypefaces[2] = sk_tool_utils::create_portable_typeface("serif", SkFontStyle::Normal());
        fTypefaces[3] =
                sk_tool_utils::create_portable_typeface("sans-serif", SkFontStyle::Normal());
        fTypefaces[4] = sk_tool_utils::create_portable_typeface("serif", SkFontStyle::Bold());
        fTypefaces[5] = sk_tool_utils::create_portable_typeface("sans-serif", SkFontStyle::Bold());
    }

    void onDraw(SkCanvas* canvas) override {
        GrRenderTargetContext* renderTargetContext =
            canvas->internal_private_accessTopLayerRenderTargetContext();
        if (!renderTargetContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        canvas->clear(SK_ColorLTGRAY);
        this->drawText(canvas);
        //  Debugging tool for GPU.
        static const bool kShowAtlas = false;
        if (kShowAtlas) {
            if (auto ctx = canvas->getGrContext()) {
                auto img = ctx->getFontAtlasImage_ForTesting(kA8_GrMaskFormat);
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
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(false);
        paint.setSubpixelText(true);

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
                paint.setTextSize(size);
                for (const auto& typeface : fTypefaces) {
                    paint.setTypeface(typeface);
                    for (const auto& text : kTexts) {
                        x = size + draw_string(canvas, text, x + subpixelX, y + subpixelY, paint);
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

    static constexpr SkScalar kSize = 1280;

    GrContextOptions::Enable fAllowMultipleTextures;
    sk_sp<SkTypeface> fTypefaces[6];
    typedef GM INHERITED;
};

constexpr SkScalar FontCacheGM::kSize;

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new FontCacheGM(GrContextOptions::Enable::kNo))
DEF_GM(return new FontCacheGM(GrContextOptions::Enable::kYes))

#endif

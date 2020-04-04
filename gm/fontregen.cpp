/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// GM to stress TextBlob regeneration and the GPU font cache
// It's not necessary to run this with CPU configs
//
// The point here is to draw a set of text that will fit in one Plot, and then some large
// text. After a flush we draw the first set of text again with a slightly different color,
// and then enough new large text to spill the entire atlas. What *should* happen is that
// the Plot with the first set of text will not get overwritten by the new large text.

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkTemplates.h"
#include "src/gpu/GrContextPriv.h"
#include "tools/ToolUtils.h"

class GrRenderTargetContext;

static sk_sp<SkTextBlob> make_blob(const SkString& text, const SkFont& font) {
    size_t len = text.size();
    SkAutoTArray<SkScalar>  pos(len);
    SkAutoTArray<SkGlyphID> glyphs(len);

    font.textToGlyphs(text.c_str(), len, SkTextEncoding::kUTF8, glyphs.get(), len);
    font.getXPos(glyphs.get(), len, pos.get());
    return SkTextBlob::MakeFromPosTextH(text.c_str(), len, pos.get(), 0, font);
}

class FontRegenGM : public skiagm::GpuGM {

    void modifyGrContextOptions(GrContextOptions* options) override {
        options->fGlyphCacheTextureMaximumBytes = 0;
        options->fAllowMultipleGlyphCacheTextures = GrContextOptions::Enable::kNo;
    }

    SkString onShortName() override { return SkString("fontregen"); }

    SkISize onISize() override { return {kSize, kSize}; }

    void onOnceBeforeDraw() override {
        this->setBGColor(SK_ColorLTGRAY);

        auto tf = ToolUtils::create_portable_typeface("sans-serif", SkFontStyle::Normal());

        static const SkString kTexts[] = {
            SkString("abcdefghijklmnopqrstuvwxyz"),
            SkString("ABCDEFGHI"),
            SkString("NOPQRSTUV")
        };

        SkFont font;
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setSubpixel(false);
        font.setSize(80);
        font.setTypeface(tf);

        fBlobs[0] = make_blob(kTexts[0], font);
        font.setSize(162);
        fBlobs[1] = make_blob(kTexts[1], font);
        fBlobs[2] = make_blob(kTexts[2], font);
    }

    void onDraw(GrContext* context, GrRenderTargetContext*, SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        canvas->drawTextBlob(fBlobs[0], 10, 80, paint);
        canvas->drawTextBlob(fBlobs[1], 10, 225, paint);
        context->flush();

        paint.setColor(0xFF010101);
        canvas->drawTextBlob(fBlobs[0], 10, 305, paint);
        canvas->drawTextBlob(fBlobs[2], 10, 465, paint);

        //  Debugging tool for GPU.
        static const bool kShowAtlas = false;
        if (kShowAtlas) {
            auto img = context->priv().testingOnly_getFontAtlasImage(kA8_GrMaskFormat);
            canvas->drawImage(img, 200, 0);
        }
    }

private:
    static constexpr int kSize = 512;

    sk_sp<SkTextBlob> fBlobs[3];
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new FontRegenGM())

///////////////////////////////////////////////////////////////////////////////

class BadAppleGM : public skiagm::GpuGM {

    SkString onShortName() override { return SkString("badapple"); }

    SkISize onISize() override { return {kSize, kSize}; }

    void onOnceBeforeDraw() override {
        this->setBGColor(SK_ColorWHITE);
        auto fm = SkFontMgr::RefDefault();

        static const SkString kTexts[] = {
                SkString("Meet"),
                SkString("iPad Pro"),
        };

        SkFont font;
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        font.setSubpixel(true);
        font.setSize(256);

        fBlobs[0] = make_blob(kTexts[0], font);
        fBlobs[1] = make_blob(kTexts[1], font);
    }

    void onDraw(GrContext* context, GrRenderTargetContext*, SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(0xFF111111);
        canvas->drawTextBlob(fBlobs[0], 10, 260, paint);
        canvas->drawTextBlob(fBlobs[1], 10, 500, paint);
        context->flush();
    }

private:
    static constexpr int kSize = 512;

    sk_sp<SkTextBlob> fBlobs[3];
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BadAppleGM())

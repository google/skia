/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradientShader.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skshaper/include/SkShaper_skunicode.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/base/SkRandom.h"
#include "src/base/SkTime.h"
#include "src/base/SkUTF.h"
#include "src/core/SkOSFile.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/viewer/Slide.h"

#if defined(SK_SHAPER_CORETEXT_AVAILABLE)
#include "modules/skshaper/include/SkShaper_coretext.h"
#endif

#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE)
#include "modules/skshaper/include/SkShaper_harfbuzz.h"
#endif

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu.h"
#endif

#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_libgrapheme.h"
#endif

#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu4x.h"
#endif

typedef std::unique_ptr<SkShaper> (*ShaperFactory)();

static const char gText[] =
    "When in the Course of human events it becomes necessary for one people "
    "to dissolve the political bands which have connected them with another "
    "and to assume among the powers of the earth, the separate and equal "
    "station to which the Laws of Nature and of Nature's God entitle them, "
    "a decent respect to the opinions of mankind requires that they should "
    "declare the causes which impel them to the separation.";

namespace {
sk_sp<SkUnicode> get_unicode() {
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::ICU::Make()) {
        return unicode;
    }
#endif  // defined(SK_UNICODE_ICU_IMPLEMENTATION)
#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::Libgrapheme::Make()) {
        return unicode;
    }
#endif
#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::ICU4X::Make()) {
        return unicode;
    }
#endif
    return nullptr;
}
}

using MakeBidiIteratorCallback = std::unique_ptr<SkShaper::BiDiRunIterator> (*)(sk_sp<SkUnicode> unicode,
                                                                                const char* utf8,
                                                                                size_t utf8Bytes,
                                                                                uint8_t bidiLevel);
using MakeScriptRunCallback = std::unique_ptr<SkShaper::ScriptRunIterator> (*)(
        const char* utf8, size_t utf8Bytes, SkFourByteTag script);

static std::unique_ptr<SkShaper::BiDiRunIterator> make_trivial_bidi(sk_sp<SkUnicode>,
                                                                    const char*,
                                                                    size_t utf8Bytes,
                                                                    uint8_t bidiLevel) {
    return std::make_unique<SkShaper::TrivialBiDiRunIterator>(bidiLevel, utf8Bytes);
}

#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
static std::unique_ptr<SkShaper::BiDiRunIterator> make_unicode_bidi(sk_sp<SkUnicode> unicode,
                                                                    const char* utf8,
                                                                    size_t utf8Bytes,
                                                                    uint8_t bidiLevel) {
    if (auto bidi = SkShapers::unicode::BidiRunIterator(unicode, utf8, utf8Bytes, bidiLevel)) {
        return bidi;
    }
    return make_trivial_bidi(unicode, utf8, utf8Bytes, bidiLevel);
}
#endif

static std::unique_ptr<SkShaper::ScriptRunIterator> make_trivial_script_runner(
        const char*, size_t utf8Bytes, SkFourByteTag scriptTag) {
    return std::make_unique<SkShaper::TrivialScriptRunIterator>(scriptTag, utf8Bytes);
}

#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
static std::unique_ptr<SkShaper::ScriptRunIterator> make_harfbuzz_script_runner(
        const char* utf8, size_t utf8Bytes, SkFourByteTag scriptTag) {
    std::unique_ptr<SkShaper::ScriptRunIterator> script =
            SkShapers::HB::ScriptRunIterator(utf8, utf8Bytes, scriptTag);
    if (script) {
        return script;
    }
    return std::make_unique<SkShaper::TrivialScriptRunIterator>(scriptTag, utf8Bytes);
}
#endif

class TextBoxSlide : public Slide {
public:
    TextBoxSlide(ShaperFactory fact,
                 MakeBidiIteratorCallback bidi,
                 MakeScriptRunCallback script,
                 const char suffix[])
            : fShaper(fact()), fBidiCallback(bidi), fScriptRunCallback(script) {
        fName = SkStringPrintf("TextBox_%s", suffix);
    }

    void load(SkScalar w, SkScalar h) override { fSize = {w, h}; }

    void resize(SkScalar w, SkScalar h) override { fSize = {w, h}; }

    void draw(SkCanvas* canvas) override {
        SkScalar width = fSize.width() / 3;
        drawTest(canvas, width, fSize.height(), SK_ColorBLACK, SK_ColorWHITE);
        canvas->translate(width, 0);
        drawTest(canvas, width, fSize.height(), SK_ColorWHITE, SK_ColorBLACK);
        canvas->translate(width, 0);
        drawTest(canvas, width, fSize.height()/2, SK_ColorGRAY, SK_ColorWHITE);
        canvas->translate(0, fSize.height()/2);
        drawTest(canvas, width, fSize.height()/2, SK_ColorGRAY, SK_ColorBLACK);
    }

private:
    void drawTest(SkCanvas* canvas, SkScalar w, SkScalar h, SkColor fg, SkColor bg) {
        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(bg);

        SkScalar margin = 20;

        SkPaint paint;
        paint.setColor(fg);

        for (int i = 9; i < 24; i += 2) {
#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
            SkShapers::HB::PurgeCaches();
#endif
            SkTextBlobBuilderRunHandler builder(gText, { margin, margin });
            SkFont srcFont(nullptr, SkIntToScalar(i));
            srcFont.setEdging(SkFont::Edging::kSubpixelAntiAlias);
            srcFont.setSubpixel(true);

            const char* utf8 = gText;
            size_t utf8Bytes = sizeof(gText) - 1;

            auto unicode = get_unicode();
            std::unique_ptr<SkShaper::BiDiRunIterator> bidi =
                    fBidiCallback(unicode, utf8, utf8Bytes, 0xfe);
            if (!bidi) {
                return;
            }

            std::unique_ptr<SkShaper::LanguageRunIterator> language(
                    SkShaper::MakeStdLanguageRunIterator(utf8, utf8Bytes));
            if (!language) {
                return;
            }

            SkFourByteTag undeterminedScript = SkSetFourByteTag('Z','y','y','y');
            std::unique_ptr<SkShaper::ScriptRunIterator> script =
                    fScriptRunCallback(utf8, utf8Bytes, undeterminedScript);
            if (!script) {
                return;
            }

            std::unique_ptr<SkShaper::FontRunIterator> font(
                    SkShaper::MakeFontMgrRunIterator(utf8,
                                                     utf8Bytes,
                                                     srcFont,
                                                     ToolUtils::TestFontMgr(),
                                                     "Arial",
                                                     SkFontStyle::Bold(),
                                                     &*language));
            if (!font) {
                return;
            }

            fShaper->shape(utf8,
                           utf8Bytes,
                           *font,
                           *bidi,
                           *script,
                           *language,
                           nullptr,
                           0,
                           w - margin,
                           &builder);
            canvas->drawTextBlob(builder.makeBlob(), 0, 0, paint);

            canvas->translate(0, builder.endPoint().y());
        }
    }

    SkSize fSize;
    std::unique_ptr<SkShaper> fShaper;
    MakeBidiIteratorCallback fBidiCallback;
    MakeScriptRunCallback fScriptRunCallback;
};

DEF_SLIDE(return new TextBoxSlide(SkShapers::Primitive::PrimitiveText,
                                  make_trivial_bidi,
                                  make_trivial_script_runner,
                                  "primitive"););

#if defined(SK_SHAPER_CORETEXT_AVAILABLE)
DEF_SLIDE(return new TextBoxSlide(SkShapers::CT::CoreText,
                                  make_trivial_bidi,
                                  make_trivial_script_runner,
                                  "coretext"););
#endif

#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
DEF_SLIDE(return new TextBoxSlide(
                         []() {
                             return SkShapers::HB::ShaperDrivenWrapper(get_unicode(),
                                                                       SkFontMgr::RefEmpty());
                         },
                         make_unicode_bidi,
                         make_harfbuzz_script_runner,
                         "harfbuzz"););
#endif

class ShaperSlide : public Slide {
public:
    ShaperSlide() { fName = "shaper"; }

    void draw(SkCanvas* canvas) override {
        canvas->translate(10, 30);

        const char text[] = "world";

        for (SkScalar size = 30; size <= 30; size += 10) {
            this->drawTest(canvas,
                           text,
                           size,
                           SkShapers::Primitive::PrimitiveText(),
                           make_trivial_bidi,
                           make_trivial_script_runner);
            canvas->translate(0, size + 5);
#if defined(SK_SHAPER_CORETEXT_AVAILABLE)
            this->drawTest(canvas,
                           text,
                           size,
                           SkShapers::CT::CoreText(),
                           make_trivial_bidi,
                           make_trivial_script_runner);
#endif
            canvas->translate(0, size + 5);
#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
            auto unicode = get_unicode();
            this->drawTest(
                    canvas,
                    text,
                    size,
                    SkShapers::HB::ShaperDrivenWrapper(unicode, SkFontMgr::RefEmpty()),
                    make_unicode_bidi,
                    make_harfbuzz_script_runner);
#endif
            canvas->translate(0, size*2);
        }
    }

private:
    void drawTest(SkCanvas* canvas,
                  const char str[],
                  SkScalar size,
                  std::unique_ptr<SkShaper> shaper,
                  MakeBidiIteratorCallback bidiCallback,
                  MakeScriptRunCallback scriptRunCallback) {
        if (!shaper) return;

        SkTextBlobBuilderRunHandler builder(str, {0, 0});
        SkFont srcFont;
        srcFont.setSize(size);
        srcFont.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        srcFont.setSubpixel(true);

        size_t len = strlen(str);

        auto unicode = get_unicode();
        std::unique_ptr<SkShaper::BiDiRunIterator> bidi =
                bidiCallback(unicode, str, len, 0xfe);
        if (!bidi) {
            return;
        }

        std::unique_ptr<SkShaper::LanguageRunIterator> language(
                SkShaper::MakeStdLanguageRunIterator(str, len));
        if (!language) {
            return;
        }

        SkFourByteTag undeterminedScript = SkSetFourByteTag('Z','y','y','y');
        std::unique_ptr<SkShaper::ScriptRunIterator> script(
                scriptRunCallback(str, len, undeterminedScript));
        if (!script) {
            return;
        }

        std::unique_ptr<SkShaper::FontRunIterator> font(
                SkShaper::MakeFontMgrRunIterator(str,
                                                 len,
                                                 srcFont,
                                                 ToolUtils::TestFontMgr(),
                                                 "Arial",
                                                 SkFontStyle::Bold(),
                                                 &*language));
        if (!font) {
            return;
        }

        shaper->shape(str, len, *font, *bidi, *script, *language, nullptr, 0, 2000, &builder);

        canvas->drawTextBlob(builder.makeBlob(), 0, 0, SkPaint());
    }
};

DEF_SLIDE( return new ShaperSlide; );

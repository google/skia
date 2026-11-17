/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/OneLineShaper.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/tests/SkParagraphTestUtils.h"
#include "modules/skparagraph/utils/TestFontCollection.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/core/SkTaskGroup.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>
#include <memory>
#include <utility>
#include <vector>

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu.h"
#endif

#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu4x.h"
#endif

using namespace skia_private;

using namespace skia::textlayout;
using namespace skia::textlayout::test;

// TODO: Fix underline
UNIX_ONLY_TEST(SkParagraph_EmojiParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_EmojiParagraph.png");
    const char* text =
            "😀😃😄😁😆😅😂🤣☺😇🙂😍😡😟😢😻👽💩👍👎🙏👌👋👄👁👦👼👨‍🚀👨‍🚒🙋‍♂️👳👨‍👨‍👧‍👧\
      💼👡👠☂🐶🐰🐻🐼🐷🐒🐵🐔🐧🐦🐋🐟🐡🕸🐌🐴🐊🐄🐪🐘🌸🌏🔥🌟🌚🌝💦💧\
      ❄🍕🍔🍟🥝🍱🕶🎩🏈⚽🚴‍♀️🎻🎼🎹🚨🚎🚐⚓🛳🚀🚁🏪🏢🖱⏰📱💾💉📉🛏🔑🔓\
      📁🗓📊❤💯🚫🔻♠♣🕓❗🏳🏁🏳️‍🌈🇮🇹🇱🇷🇺🇸🇬🇧🇨🇳🇧🇴";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    text_style.setFontSize(50);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 0, 0);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->lines().size() == 8);

    for (auto& line : impl->lines()) {
        if (&line != impl->lines().data() + impl->lines().size() - 1) {
            // The actual value is 50_size / 109_ppemX * 136_advance = ~62.385319
            // FreeType reports advances in 24.6 fixed point, so each is 62.390625
            REPORTER_ASSERT(
                    reporter,
                    line.width() == 998.25f || (998.1 < line.width() && line.width() < 998.2),
                    "width: %f",
                    line.width());
        } else {
            REPORTER_ASSERT(reporter, line.width() < 998.25f, "width: %f", line.width());
        }
        REPORTER_ASSERT(reporter, line.height() == 59, "height: %f", line.height());
    }
}

// Checked: NO DIFF (number of runs only)
UNIX_ONLY_TEST(SkParagraph_FontFallbackParagraph, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    TestCanvas canvas("SkParagraph_FontFallbackParagraph.png");

    const char* text1 = "Roboto 字典 ";         // Roboto + unresolved
    const char* text2 = "Homemade Apple 字典";  // Homemade Apple + Noto Sans...
    const char* text3 = "Chinese 字典";         // Homemade Apple + Source Han

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    TextStyle text_style;
    text_style.setFontFamilies({
            SkString("Not a real font"),
            SkString("Also a fake font"),
            SkString("So fake it is obvious"),
            SkString("Next one should be a real font..."),
            SkString("Roboto"),
            SkString("another fake one in between"),
            SkString("Homemade Apple"),
    });
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text1, strlen(text1));

    text_style.setFontFamilies({
            SkString("Not a real font"),
            SkString("Also a fake font"),
            SkString("So fake it is obvious"),
            SkString("Homemade Apple"),
            SkString("Next one should be a real font..."),
            SkString("Roboto"),
            SkString("another fake one in between"),
            SkString("Noto Sans CJK JP"),
            SkString("Source Han Serif CN"),
    });
    builder.pushStyle(text_style);
    builder.addText(text2, strlen(text2));

    text_style.setFontFamilies({
            SkString("Not a real font"),
            SkString("Also a fake font"),
            SkString("So fake it is obvious"),
            SkString("Homemade Apple"),
            SkString("Next one should be a real font..."),
            SkString("Roboto"),
            SkString("another fake one in between"),
            SkString("Source Han Serif CN"),
            SkString("Noto Sans CJK JP"),
    });
    builder.pushStyle(text_style);
    builder.addText(text3, strlen(text3));

    builder.pop();

    auto paragraph = builder.Build();
    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == -1);  // Not shaped yet
    paragraph->layout(TestCanvasWidth);
    paragraph->paint(canvas.get(), 10.0, 15.0);

    size_t spaceRun = 1;
    REPORTER_ASSERT(reporter,
                    paragraph->unresolvedGlyphs() ==
                            2);  // From the text1 ("字典" - excluding the last space)

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 6 + spaceRun);
    if (impl->runs().size() == 6 + spaceRun) {
        // Font resolution in Skia produces 6 runs because 2 parts of "Roboto 字典 " have different
        // script (Minikin merges the first 2 into one because of unresolved)
        // [Apple + Unresolved + ' '] 0, 1, 2
        // [Apple + Noto] 3, 4
        // [Apple + Han] 5, 6
        auto robotoAdvance = impl->runs()[0].advance().fX + impl->runs()[1].advance().fX;
        robotoAdvance += impl->runs()[2].advance().fX;

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(robotoAdvance, 64.199f, EPSILON50));
        REPORTER_ASSERT(
                reporter,
                SkScalarNearlyEqual(impl->runs()[2 + spaceRun].advance().fX, 139.125f, EPSILON100));
        REPORTER_ASSERT(
                reporter,
                SkScalarNearlyEqual(impl->runs()[3 + spaceRun].advance().fX, 27.999f, EPSILON100));
        REPORTER_ASSERT(
                reporter,
                SkScalarNearlyEqual(impl->runs()[4 + spaceRun].advance().fX, 62.248f, EPSILON100));
        REPORTER_ASSERT(
                reporter,
                SkScalarNearlyEqual(impl->runs()[5 + spaceRun].advance().fX, 27.999f, EPSILON100));

        // When a different font is resolved, then the metrics are different.
        REPORTER_ASSERT(reporter,
                        impl->runs()[3 + spaceRun].correctAscent() !=
                                impl->runs()[5 + spaceRun].correctAscent());
        REPORTER_ASSERT(reporter,
                        impl->runs()[3 + spaceRun].correctDescent() !=
                                impl->runs()[5 + spaceRun].correctDescent());
    }
}

UNIX_ONLY_TEST(SkParagraph_Fallbacks, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr(), "Arial");
    fontCollection->enableFontFallback();
    TestCanvas canvas("SkParagraph_Fallbacks.png");

    const char* multiScript =
            "A1!aÀàĀāƁƀḂⱠꜲꬰəͲἀἏЀЖԠꙐꙮՁخ‎ࡔࠇܦআਉઐଘஇఘಧൺඣᭆᯔᮯ᳇ꠈᜅᩌꪈ༇ꥄꡙꫤ᧰៘꧁꧂ᜰᨏᯤᢆᣭᗗꗃⵞ𐒎߷ጩꬤ𖠺‡₩℻Ⅷ↹⋇⏳ⓖ╋▒◛⚧⑆שׁ🅕㊼龜ポ"
            "䷤🂡\n";
    const size_t len = strlen(multiScript);

    const char* androidFonts[] = {
            "sans-serif",
            "sans-serif-condensed",
            "serif",
            "monospace",
            "serif-monospace",
            "casual",
            "cursive",
            "sans-serif-smallcaps",
    };

    for (auto& font : androidFonts) {

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setLocale(SkString("en_US"));
        text_style.setFontSize(20);

        text_style.setFontFamilies({SkString(font)});
        builder.pushStyle(text_style);
        builder.addText(multiScript, len);

        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(TestCanvasWidth);
        paragraph->paint(canvas.get(), 0, 0);
        canvas.get()->translate(0, paragraph->getHeight() + 10);
    }
}

UNIX_ONLY_TEST(SkParagraph_FontResolutions, reporter) {
    TestCanvas canvas("SkParagraph_FontResolutions.png");

    sk_sp<TestFontCollection> fontCollection =
            sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), false);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    if (!fontCollection->addFontFromFile("abc/abc.ttf", "abc")) {
        ERRORF(reporter, "abc/abc.ttf not found");
        return;
    }
    if (!fontCollection->addFontFromFile("abc/abc+grave.ttf", "abc+grave")) {
        ERRORF(reporter, "abc/abc+grave.ttf not found");
        return;
    }
    if (!fontCollection->addFontFromFile("abc/abc+agrave.ttf", "abc+agrave")) {
        ERRORF(reporter, "abc/abc+agrave.ttf not found");
        return;
    }

    TextStyle text_style;
    text_style.setFontFamilies({SkString("abc")});
    text_style.setFontSize(50);

    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());

    text_style.setFontFamilies({SkString("abc"), SkString("abc+grave")});
    text_style.setColor(SK_ColorBLUE);
    builder.pushStyle(text_style);
    builder.addText(u"a\u0300");
    text_style.setColor(SK_ColorMAGENTA);
    builder.pushStyle(text_style);
    builder.addText(u"à");

    text_style.setFontFamilies({SkString("abc"), SkString("abc+agrave")});

    text_style.setColor(SK_ColorRED);
    builder.pushStyle(text_style);
    builder.addText(u"a\u0300");
    text_style.setColor(SK_ColorGREEN);
    builder.pushStyle(text_style);
    builder.addText(u"à");

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 2);
    if (impl->runs().size() >= 2) {
        REPORTER_ASSERT(reporter, impl->runs().front().size() == 4);
        if (impl->runs().front().size() >= 4) {
            REPORTER_ASSERT(reporter,
                            impl->runs().front().glyphs()[0] == impl->runs().front().glyphs()[2]);
            REPORTER_ASSERT(reporter,
                            impl->runs().front().glyphs()[1] == impl->runs().front().glyphs()[3]);
        }

        REPORTER_ASSERT(reporter, impl->runs().back().size() == 2);
        if (impl->runs().back().size() >= 2) {
            REPORTER_ASSERT(reporter,
                            impl->runs().back().glyphs()[0] == impl->runs().back().glyphs()[1]);
        }
    }

    paragraph->paint(canvas.get(), 100, 100);
}

[[maybe_unused]] static void SkParagraph_EmojiFontResolution(sk_sp<SkUnicode> icu,
                                                             skiatest::Reporter* reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    const char* text = "♻️🏴󠁧󠁢󠁳󠁣󠁴󠁿";
    const char* text1 = "♻️";
    const size_t len = strlen(text);
    const size_t len1 = strlen(text1);

    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("")});
    builder.pushStyle(text_style);
    builder.addText(text, len);
    auto paragraph = builder.Build();
    paragraph->layout(SK_ScalarMax);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1, "size: %zu", impl->runs().size());

    ParagraphBuilderImpl builder1(paragraph_style, fontCollection, get_unicode());
    builder1.pushStyle(text_style);
    builder1.addText(text1, len1);
    auto paragraph1 = builder1.Build();
    paragraph1->layout(SK_ScalarMax);

    auto impl1 = static_cast<ParagraphImpl*>(paragraph1.get());
    REPORTER_ASSERT(reporter, impl1->runs().size() == 1, "size: %zu", impl1->runs().size());
    if (impl1->runs().size() == 1) {
        SkString ff;
        impl->run(0).font().getTypeface()->getFamilyName(&ff);
        SkString ff1;
        impl1->run(0).font().getTypeface()->getFamilyName(&ff1);
        REPORTER_ASSERT(reporter, ff.equals(ff1));
    }
}

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
UNIX_ONLY_TEST(SkParagraph_ICU_EmojiFontResolution, reporter) {
    SkParagraph_EmojiFontResolution(SkUnicodes::ICU::Make(), reporter);
}
#endif

#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
UNIX_ONLY_TEST(SkParagraph_ICU4X_EmojiFontResolution, reporter) {
    SkParagraph_EmojiFontResolution(SkUnicodes::ICU4X::Make(), reporter);
}
#endif

[[maybe_unused]] static void SkUnicode_Emoji(sk_sp<SkUnicode> icu, skiatest::Reporter* reporter) {
    auto test = [&](const char* text, SkUnichar expected) {
        SkString str(text);
        if ((false)) {
            SkDebugf("'%s'\n", text);
            const char* begin = str.data();
            const char* end = str.data() + str.size();
            while (begin != end) {
                auto unicode = SkUTF::NextUTF8WithReplacement(&begin, end);
                SkDebugf("  %d: %s %s\n",
                         unicode,
                         icu->isEmoji(unicode) ? "isEmoji" : "",
                         icu->isEmojiComponent(unicode) ? "isEmojiComponent" : "");
            }

            SkDebugf("Graphemes:");
            skia_private::TArray<SkUnicode::CodeUnitFlags, true> codeUnitProperties;
            icu->computeCodeUnitFlags(str.data(), str.size(), false, &codeUnitProperties);
            int index = 0;
            for (auto& cp : codeUnitProperties) {
                if (SkUnicode::hasGraphemeStartFlag(cp)) {
                    SkDebugf(" %d", index);
                }
                ++index;
            }
            SkDebugf("\n");
        }

        SkSpan<const char> textSpan(str.data(), str.size());
        const char* begin = str.data();
        const char* end = begin + str.size();
        auto emojiStart = OneLineShaper::getEmojiSequenceStart(icu.get(), &begin, end);
        REPORTER_ASSERT(reporter, expected == emojiStart);
    };

    test("", -1);
    test("0", -1);
    test("2nd", -1);
    test("99", -1);
    test("0️⃣", 48);
    test("0️⃣12", 48);
    test("#", -1);
    test("#️⃣", 35);
    test("#️⃣#", 35);
    test("#️⃣#️⃣", 35);
    test("*", -1);
    test("*️⃣", 42);
    test("*️⃣abc", 42);
    test("*️⃣😊", 42);
    test("😊", 128522);
    test("😊abc", 128522);
    test("😊*️⃣", 128522);
    test("👨‍👩‍👦‍👦", 128104);

    // These 2 have emoji components as the first codepoint
    test("🇷🇺", 127479);  // Flag sequence
    test("0️⃣", 48);       // Keycap sequence

    // These have a simple emoji as a first codepoint
    test("🏴󠁧󠁢󠁥󠁮󠁧󠁿", 127988);  // Tag sequence
    test("👋🏼", 128075);                          // Modifier sequence
    test("👨‍👩‍👧‍👦", 128104);     // ZWJ sequence
}

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
UNIX_ONLY_TEST(SkParagraph_ICU_EmojiRuns, reporter) {
    SkUnicode_Emoji(SkUnicodes::ICU::Make(), reporter);
}
#endif

#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
UNIX_ONLY_TEST(SkParagraph_ICU4X_EmojiRuns, reporter) {
    SkUnicode_Emoji(SkUnicodes::ICU4X::Make(), reporter);
}
#endif

namespace {
// A font manager that only implements matchFamilyStyle (not matchFamily) to
// verify that FontCollection resolves families through the style API.
class StyleOnlyFontMgr : public TypefaceFontProvider {
public:
    explicit StyleOnlyFontMgr(sk_sp<SkTypeface> typeface) : fTypeface(std::move(typeface)) {}

    int matchFamilyCalls() const { return fMatchFamilyCalls; }
    int matchFamilyStyleCalls() const { return fMatchFamilyStyleCalls; }

private:
    sk_sp<SkFontStyleSet> onMatchFamily(const char[]) const override {
        ++fMatchFamilyCalls;
        return nullptr;
    }

    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle&) const override {
        ++fMatchFamilyStyleCalls;
        return familyName && strcmp(familyName, "StyleOnly") == 0 ? fTypeface : nullptr;
    }

    sk_sp<SkTypeface> fTypeface;
    mutable int fMatchFamilyCalls = 0;
    mutable int fMatchFamilyStyleCalls = 0;
};
}  // namespace

DEF_TEST(SkParagraph_FindTypefacesUsesMatchFamilyStyle, reporter) {
    auto typeface =
            ToolUtils::TestFontMgr()->makeFromStream(GetResourceAsStream("fonts/abc/abc.ttf"), {});
    if (!typeface) {
        return;
    }

    sk_sp<SkTypeface> expected = typeface;
    auto manager = sk_make_sp<StyleOnlyFontMgr>(std::move(typeface));
    auto collection = sk_make_sp<FontCollection>();
    collection->setDefaultFontManager(manager);

    auto faces = collection->findTypefaces({SkString("StyleOnly")}, SkFontStyle());
    REPORTER_ASSERT(reporter, faces.size() == 1);
    if (faces.size() == 1) {
        REPORTER_ASSERT(reporter, faces[0] == expected);
    }
    REPORTER_ASSERT(reporter, manager->matchFamilyStyleCalls() == 1);
    REPORTER_ASSERT(reporter, manager->matchFamilyCalls() == 0);
}

namespace {
// Counts fallback requests and finds no fallback typeface
class CountingFallbackFontMgr : public TypefaceFontProvider {
public:
    int fallbackCount() const { return fFallbackCount; }

private:
    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(
            const char[], const SkFontStyle&, const char*[], int, SkUnichar) const override {
        ++fFallbackCount;
        return nullptr;
    }

    mutable int fFallbackCount = 0;
};
}  // namespace

// Verify that CR LF, which is one grapheme (UAX #29 GB3) but two glyphs, does not ask for a
// fallback font, the same as a lone CR or LF
DEF_TEST(SkParagraph_CrLfDoesNotRequestFallback, reporter) {
    sk_sp<SkFontMgr> mgr = ToolUtils::TestFontMgr();
    // abc.ttf has no glyphs for CR and LF
    auto abcTypeface = mgr->makeFromStream(GetResourceAsStream("fonts/abc/abc.ttf"), {});
    // Portable font manager returns null for stream-backed typefaces when native fonts are disabled
    if (!abcTypeface) {
        return;
    }

    auto fontProvider = sk_make_sp<TypefaceFontProvider>();
    fontProvider->registerTypeface(abcTypeface, SkString("AbcFont"));

    auto check = [&](const char* name, const char* text, size_t expectedLines) {
        auto fallbackMgr = sk_make_sp<CountingFallbackFontMgr>();
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setAssetFontManager(fontProvider);
        fontCollection->setDefaultFontManager(fallbackMgr);
        fontCollection->enableFontFallback();

        ParagraphStyle paragraphStyle;
        TextStyle style;
        style.setFontFamilies({SkString("AbcFont")});
        style.setFontSize(14.0f);
        paragraphStyle.setTextStyle(style);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(style);
        builder.addText(text);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);

        REPORTER_ASSERT(reporter,
                        fallbackMgr->fallbackCount() == 0,
                        "%s: %d fallback requests",
                        name,
                        fallbackMgr->fallbackCount());
        REPORTER_ASSERT(reporter,
                        paragraph->lineNumber() == expectedLines,
                        "%s: %zu lines",
                        name,
                        paragraph->lineNumber());
        return paragraph->getHeight();
    };

    check("CR", "a\rb", 1);
    auto lfHeight = check("LF", "a\nb", 2);
    auto crLfHeight = check("CR LF", "a\r\nb", 2);
    REPORTER_ASSERT(reporter, crLfHeight == lfHeight);
    auto lfLfHeight = check("LF LF", "a\n\nb", 3);
    auto crLfCrLfHeight = check("CR LF CR LF", "a\r\n\r\nb", 3);
    REPORTER_ASSERT(reporter, crLfCrLfHeight == lfLfHeight);
}

namespace {
std::unique_ptr<Paragraph> layoutWith(sk_sp<FontCollection> fontCollection, const char* text) {
    ParagraphStyle paragraphStyle;
    TextStyle textStyle;
    textStyle.setFontSize(30.0f);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(1000.0f);
    return paragraph;
}
// Forwards to another font manager and counts the fallback queries.
class CountingFontMgr : public SkFontMgr {
public:
    explicit CountingFontMgr(sk_sp<SkFontMgr> fontMgr) : fFontMgr(std::move(fontMgr)) {}
    mutable int fCharacterMatches = 0;
    mutable int fFamilyStyleMatches = 0;
    // Counted apart from fFamilyStyleMatches because matchTypeface also calls matchFamilyStyle
    mutable int fEmojiFamilyMatches = 0;

protected:
    int onCountFamilies() const override { return fFontMgr->countFamilies(); }
    void onGetFamilyName(int index, SkString* familyName) const override {
        fFontMgr->getFamilyName(index, familyName);
    }
    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override {
        return fFontMgr->createStyleSet(index);
    }
    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override {
        return fFontMgr->matchFamily(familyName);
    }
    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& style) const override {
        ++fFamilyStyleMatches;
        if (familyName && std::strcmp(familyName, "Apple Color Emoji") == 0) {
            ++fEmojiFamilyMatches;
        }
        return fFontMgr->matchFamilyStyle(familyName, style);
    }
    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[],
                                                  const SkFontStyle& style,
                                                  const char* bcp47[],
                                                  int bcp47Count,
                                                  SkUnichar character) const override {
        ++fCharacterMatches;
        return fFontMgr->matchFamilyStyleCharacter(familyName, style, bcp47, bcp47Count, character);
    }
    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        return fFontMgr->makeFromData(std::move(data), ttcIndex);
    }
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override {
        return fFontMgr->makeFromStream(std::move(stream), ttcIndex);
    }
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                           const SkFontArguments& args) const override {
        return fFontMgr->makeFromStream(std::move(stream), args);
    }
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        return fFontMgr->makeFromFile(path, ttcIndex);
    }
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[],
                                           SkFontStyle style) const override {
        return fFontMgr->legacyMakeTypeface(familyName, style);
    }

private:
    sk_sp<SkFontMgr> fFontMgr;
};
}  // namespace

// Paragraphs with different text must share the system fallback results
DEF_TEST(SkParagraph_FallbackCacheAcrossParagraphs, reporter) {
    auto fontMgr = sk_make_sp<CountingFontMgr>(ToolUtils::TestFontMgr());
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(fontMgr);
    auto queries = [&] { return fontMgr->fCharacterMatches + fontMgr->fFamilyStyleMatches; };

    auto first = layoutWith(fontCollection, "a 一 ❯ \U0001F600");
    const int firstQueries = queries();
    REPORTER_ASSERT(reporter, firstQueries > 0);
    auto second = layoutWith(fontCollection, "b 一 ❯ \U0001F600");
    REPORTER_ASSERT(reporter, queries() == firstQueries);
    // A cached fallback must give the same font as a new query
    auto family = [](Paragraph* paragraph, TextIndex index) {
        SkString name;
        if (SkTypeface* typeface = paragraph->getFontAt(index).getTypeface()) {
            typeface->getFamilyName(&name);
        }
        return name;
    };
    for (TextIndex index = 2; index < 14; ++index) {
        REPORTER_ASSERT(reporter, family(first.get(), index) == family(second.get(), index));
    }

    fontCollection->clearCaches();
    layoutWith(fontCollection, "c 一 ❯ \U0001F600");
    REPORTER_ASSERT(reporter, queries() == 2 * firstQueries);

    auto otherFontMgr = sk_make_sp<CountingFontMgr>(ToolUtils::TestFontMgr());
    fontCollection->setDefaultFontManager(otherFontMgr);
    layoutWith(fontCollection, "d 一 ❯ \U0001F600");
    REPORTER_ASSERT(
            reporter,
            otherFontMgr->fCharacterMatches + otherFontMgr->fFamilyStyleMatches == firstQueries);

    // Distinct emoji codepoints must not re-query family-based Apple Color Emoji fallback
    layoutWith(fontCollection, "e 一 ❯ \U0001F602");
    REPORTER_ASSERT(reporter, otherFontMgr->fEmojiFamilyMatches <= 1);

    // Emoji fallback must work when only a non-default font manager is configured
    auto testFontMgr = sk_make_sp<CountingFontMgr>(ToolUtils::TestFontMgr());
    auto testCollection = sk_make_sp<FontCollection>();
    testCollection->setTestFontManager(testFontMgr);
    testCollection->defaultEmojiFallback(0x1F600, SkFontStyle(), SkString());
    testCollection->defaultEmojiFallback(0x1F602, SkFontStyle(), SkString());
    REPORTER_ASSERT(reporter, testFontMgr->fFamilyStyleMatches <= 1);

    // Styles with different families[0] in one paragraph must not share fallback entries
    auto layoutTwoFamilies = [&](const char* firstFamily, const char* secondFamily) {
        ParagraphStyle paragraphStyle;
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        TextStyle firstStyle;
        firstStyle.setFontFamilies({SkString(firstFamily)});
        builder.pushStyle(firstStyle);
        builder.addText("一");
        builder.pop();
        TextStyle secondStyle;
        secondStyle.setFontFamilies({SkString(secondFamily)});
        builder.pushStyle(secondStyle);
        builder.addText("一");
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);
    };
    const int beforeFamilies = otherFontMgr->fCharacterMatches;
    layoutTwoFamilies("FamilyA", "FamilyB");
    REPORTER_ASSERT(reporter, otherFontMgr->fCharacterMatches == beforeFamilies + 2);
    layoutTwoFamilies("FamilyB", "FamilyA");
    REPORTER_ASSERT(reporter, otherFontMgr->fCharacterMatches == beforeFamilies + 2);
}

// The fallback cache must not grow with each new codepoint
DEF_TEST(SkParagraph_FallbackCacheIsBounded, reporter) {
    auto fallbackMgr = sk_make_sp<CountingFallbackFontMgr>();
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(fallbackMgr);
    auto queriesAfter = [&](SkUnichar unicode) {
        fontCollection->defaultFallback(unicode, {}, SkFontStyle(), SkString(), std::nullopt);
        return fallbackMgr->fallbackCount();
    };

    // All 65536 codepoints of Supplementary Private Use Area-A, more than the cache keeps
    for (SkUnichar unicode = 0xF0000; unicode <= 0xFFFFF; ++unicode) {
        queriesAfter(unicode);
    }
    const int queries = fallbackMgr->fallbackCount();
    REPORTER_ASSERT(reporter, queries == 0x10000);
    REPORTER_ASSERT(reporter, queriesAfter(0xFFFFF) == queries);
    REPORTER_ASSERT(reporter, queriesAfter(0xF0000) == queries + 1);
}

// Threads that share one FontCollection must get the same fallback results as one thread
DEF_TEST(SkParagraph_FallbackCacheConcurrently, reporter) {
    constexpr int kThreadCount = 8;
    constexpr int kRounds = 10;
    std::vector<SkUnichar> codepoints = {'a', 0x276F};
    for (SkUnichar unicode = 0x4E00; unicode < 0x4E00 + 100; ++unicode) {
        codepoints.push_back(unicode);
    }
    auto familyOf = [](const sk_sp<SkTypeface>& typeface) {
        SkString name;
        if (typeface) {
            typeface->getFamilyName(&name);
        }
        return name;
    };
    // cloneTypeface() locks fVariationCache after the fallback cache lock is released
    const FontArguments fontArgs(SkFontArguments().setCollectionIndex(0));
    auto resolve = [&](FontCollection* fontCollection, size_t index) {
        if (index % 2) {
            return familyOf(
                    fontCollection->defaultEmojiFallback(0x1F600, SkFontStyle(), SkString()));
        }
        return familyOf(fontCollection->defaultFallback(
                codepoints[index / 2], {}, SkFontStyle(), SkString(), fontArgs));
    };
    const size_t kLookups = 2 * codepoints.size();

    auto expectedCollection = sk_make_sp<FontCollection>();
    expectedCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    std::vector<SkString> expected(kLookups);
    for (size_t i = 0; i < kLookups; ++i) {
        expected[i] = resolve(expectedCollection.get(), i);
    }

    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    std::vector<int> mismatches(kThreadCount);
    auto executor = SkFunctionalExecutors::MakeFIFOThreadPool(kThreadCount, false);
    SkTaskGroup tasks(*executor);
    for (int thread = 0; thread < kThreadCount; ++thread) {
        tasks.add([&, thread] {
            // Most lookups are hits, and a hit also changes the LRU list
            for (size_t n = 0; n < kRounds * kLookups; ++n) {
                size_t i = (n + thread * kLookups / kThreadCount) % kLookups;
                if (resolve(fontCollection.get(), i) != expected[i]) {
                    ++mismatches[thread];
                }
            }
        });
    }
    tasks.wait();
    for (int count : mismatches) {
        REPORTER_ASSERT(reporter, count == 0);
    }
}

// Changing how fonts are resolved must not leave typefaces or paragraphs cached from before
DEF_TEST(SkParagraph_FontCollectionSettersResetCaches, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(SkFontMgr::RefEmpty());
    REPORTER_ASSERT(reporter, layoutWith(fontCollection, "Hello")->getMaxIntrinsicWidth() == 0);
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    REPORTER_ASSERT(reporter, layoutWith(fontCollection, "Hello")->getMaxIntrinsicWidth() > 0);

    auto reference = sk_make_sp<FontCollection>();
    reference->setDefaultFontManager(ToolUtils::TestFontMgr());
    reference->disableFontFallback();
    const SkScalar widthWithoutFallback = layoutWith(reference, "Hello")->getMaxIntrinsicWidth();

    auto toggled = sk_make_sp<FontCollection>();
    toggled->setDefaultFontManager(ToolUtils::TestFontMgr());
    const SkScalar widthWithFallback = layoutWith(toggled, "Hello")->getMaxIntrinsicWidth();
    REPORTER_ASSERT(reporter, widthWithoutFallback != widthWithFallback);
    toggled->disableFontFallback();
    REPORTER_ASSERT(reporter,
                    layoutWith(toggled, "Hello")->getMaxIntrinsicWidth() == widthWithoutFallback);
}

// Repeating the current font settings must keep the cached paragraphs
DEF_TEST(SkParagraph_FontCollectionSameSettingsKeepCaches, reporter) {
    auto fontMgr = ToolUtils::TestFontMgr();
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(fontMgr);
    layoutWith(fontCollection, "Hello");
    REPORTER_ASSERT(reporter, fontCollection->getParagraphCache()->count() == 1);

    fontCollection->setDefaultFontManager(fontMgr);
    fontCollection->setAssetFontManager(nullptr);
    fontCollection->enableFontFallback();
    REPORTER_ASSERT(reporter, fontCollection->getParagraphCache()->count() == 1);
}

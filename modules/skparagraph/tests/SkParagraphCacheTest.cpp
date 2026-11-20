/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkColor.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/tests/SkParagraphTestUtils.h"
#include "src/core/SkTaskGroup.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>
#include <chrono>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

UNIX_ONLY_TEST(SkParagraph_CachePreservesUnresolvedGlyphs, reporter) {
    sk_sp<ResourceFontCollection> fonts = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fonts)
    fonts->disableFontFallback();
    ParagraphStyle style;
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(14);
    style.setTextStyle(textStyle);
    auto make = [&](SkScalar width) {
        ParagraphBuilderImpl builder(style, fonts, get_unicode());
        builder.addText("missing \xF3\xB0\x80\x80");  // U+F0000 has no Roboto glyph.
        auto paragraph = builder.Build();
        paragraph->layout(width);
        return paragraph;
    };
    auto cold = make(120);
    const auto unresolved = cold->unresolvedCodepoints();
    REPORTER_ASSERT(reporter, cold->unresolvedGlyphs() == 1);
    REPORTER_ASSERT(reporter, unresolved.size() == 1 && unresolved.count(0xF0000) == 1);
    auto cached = make(180);
    REPORTER_ASSERT(reporter, cached->unresolvedGlyphs() == cold->unresolvedGlyphs());
    REPORTER_ASSERT(reporter, cached->unresolvedCodepoints() == unresolved);
    // Shaping again after a cache hit must not change the set the cache shares
    fonts->getParagraphCache()->turnOn(false);
    cached->markDirty();
    cached->layout(150);
    REPORTER_ASSERT(reporter, cached->unresolvedCodepoints() == unresolved);
    REPORTER_ASSERT(reporter, cold->unresolvedCodepoints() == unresolved);
    REPORTER_ASSERT(reporter, make(120)->unresolvedCodepoints() == unresolved);
}

UNIX_ONLY_TEST(SkParagraph_CacheText, reporter) {
    ParagraphCache cache;
    cache.turnOn(true);
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);

    auto test = [&](const char* text, int count, bool expectedToBeFound) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        builder.pushStyle(text_style);
        builder.addText(text, strlen(text));
        builder.pop();
        auto paragraph = builder.Build();

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, count == cache.count());
        auto found = cache.findParagraph(impl);
        REPORTER_ASSERT(reporter, found == expectedToBeFound);
        auto added = cache.updateParagraph(impl);
        REPORTER_ASSERT(reporter, added != expectedToBeFound);
    };

    test("text1", 0, false);
    test("text1", 1, true);
    test("text2", 1, false);
    test("text2", 2, true);
    test("text3", 2, false);
}

UNIX_ONLY_TEST(SkParagraph_CacheFonts, reporter) {
    ParagraphCache cache;
    cache.turnOn(true);
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();

    TextStyle text_style;
    text_style.setColor(SK_ColorBLACK);

    const char* text = "text";
    const size_t len = strlen(text);

    auto test = [&](int count, bool expectedToBeFound) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        builder.pushStyle(text_style);
        builder.addText(text, len);
        builder.pop();
        auto paragraph = builder.Build();
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());

        REPORTER_ASSERT(reporter, count == cache.count());
        auto found = cache.findParagraph(impl);
        REPORTER_ASSERT(reporter, found == expectedToBeFound);
        auto added = cache.updateParagraph(impl);
        REPORTER_ASSERT(reporter, added != expectedToBeFound);
    };

    text_style.setFontFamilies({SkString("Roboto")});
    test(0, false);
    test(1, true);
    text_style.setFontFamilies({SkString("Homemade Apple")});
    test(1, false);
    test(2, true);
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    test(2, false);
}

UNIX_ONLY_TEST(SkParagraph_CacheFontRanges, reporter) {
    ParagraphCache cache;
    cache.turnOn(true);
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);

    auto test = [&](const char* text1,
                    const char* text2,
                    const char* font1,
                    const char* font2,
                    int count,
                    bool expectedToBeFound) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        text_style.setFontFamilies({SkString(font1)});
        builder.pushStyle(text_style);
        builder.addText(text1, strlen(text1));
        builder.pop();
        text_style.setFontFamilies({SkString(font2)});
        builder.pushStyle(text_style);
        builder.addText(text2, strlen(text2));
        builder.pop();
        auto paragraph = builder.Build();
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());

        REPORTER_ASSERT(reporter, count == cache.count());
        auto found = cache.findParagraph(impl);
        REPORTER_ASSERT(reporter, found == expectedToBeFound);
        auto added = cache.updateParagraph(impl);
        REPORTER_ASSERT(reporter, added != expectedToBeFound);
    };

    test("text", "", "Roboto", "Homemade Apple", 0, false);
    test("t", "ext", "Roboto", "Homemade Apple", 1, false);
    test("te", "xt", "Roboto", "Homemade Apple", 2, false);
    test("tex", "t", "Roboto", "Homemade Apple", 3, false);
    test("text", "", "Roboto", "Homemade Apple", 4, true);
}

UNIX_ONLY_TEST(SkParagraph_CacheStyles, reporter) {
    ParagraphCache cache;
    cache.turnOn(true);
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);

    const char* text = "text";
    const size_t len = strlen(text);

    auto test = [&](int count, bool expectedToBeFound) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        builder.pushStyle(text_style);
        builder.addText(text, len);
        builder.pop();
        auto paragraph = builder.Build();
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());

        REPORTER_ASSERT(reporter, count == cache.count());
        auto found = cache.findParagraph(impl);
        REPORTER_ASSERT(reporter, found == expectedToBeFound);
        auto added = cache.updateParagraph(impl);
        REPORTER_ASSERT(reporter, added != expectedToBeFound);
    };

    test(0, false);
    test(1, true);
    text_style.setLetterSpacing(10);
    test(1, false);
    test(2, true);
    text_style.setWordSpacing(10);
    test(2, false);
}

namespace {
struct CacheProbe {
    std::function<void(ParagraphStyle&, TextStyle& text, TextStyle& placeholder)> fMutate;
    const char* fName;
};

// Lays out a paragraph with text and a placeholder, reporting whether the cache was hit
bool layoutHitsCache(sk_sp<FontCollection> fontCollection, const CacheProbe* probe) {
    ParagraphStyle paragraphStyle;
    StrutStyle strutStyle;
    strutStyle.setStrutEnabled(true);
    strutStyle.setFontSize(30.0f);
    strutStyle.setHeightOverride(true);
    strutStyle.setHeight(2.0f);
    paragraphStyle.setStrutStyle(strutStyle);
    TextStyle textStyle;
    textStyle.setFontSize(30.0f);
    textStyle.setHeightOverride(true);
    textStyle.setHeight(2.0f);
    TextStyle placeholderTextStyle = textStyle;
    if (probe) {
        probe->fMutate(paragraphStyle, textStyle, placeholderTextStyle);
    }

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(textStyle);
    builder.addText("Hello world");
    builder.pop();
    builder.pushStyle(placeholderTextStyle);
    builder.addPlaceholder(
            PlaceholderStyle(20, 20, PlaceholderAlignment::kTop, TextBaseline::kAlphabetic, 0));
    builder.pop();
    auto paragraph = builder.Build();

    bool found = false;
    fontCollection->getParagraphCache()->setChecker(
            [&found](ParagraphImpl*, const char* event, bool) {
                found |= strcmp(event, "foundParagraph") == 0;
            });
    paragraph->layout(1000.0f);
    fontCollection->getParagraphCache()->setChecker([](ParagraphImpl*, const char*, bool) {});
    return found;
}
}  // namespace

// Changing any shaping input must miss ParagraphCache, or stale shaped runs are reused
DEF_TEST(SkParagraph_CacheKeyCoversShapingInputs, reporter) {
    auto makeCollection = []() {
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
        return fontCollection;
    };

    auto same = makeCollection();
    layoutHitsCache(same, nullptr);
    REPORTER_ASSERT(reporter, layoutHitsCache(same, nullptr));

    // Without the override the shaper ignores the height value
    CacheProbe ignoredHeight = {[](ParagraphStyle&, TextStyle& t, TextStyle& p) {
                                    t.setHeightOverride(false);
                                    p.setHeightOverride(false);
                                },
                                "ignoredHeight"};
    CacheProbe otherIgnoredHeight = {[](ParagraphStyle&, TextStyle& t, TextStyle& p) {
                                         t.setHeightOverride(false);
                                         t.setHeight(3.0f);
                                         p.setHeightOverride(false);
                                         p.setHeight(3.0f);
                                     },
                                     "otherIgnoredHeight"};
    auto heights = makeCollection();
    layoutHitsCache(heights, &ignoredHeight);
    REPORTER_ASSERT(reporter, layoutHitsCache(heights, &otherIgnoredHeight));

    const CacheProbe probes[] = {
            {[](ParagraphStyle&, TextStyle& t, TextStyle&) { t.setHalfLeading(true); },
             "halfLeading"},
            {[](ParagraphStyle&, TextStyle& t, TextStyle&) { t.setHeightOverride(false); },
             "heightOverride"},
            {[](ParagraphStyle&, TextStyle& t, TextStyle&) {
                 t.setFontEdging(SkFont::Edging::kAlias);
             },
             "edging"},
            {[](ParagraphStyle&, TextStyle& t, TextStyle&) {
                 t.setFontHinting(SkFontHinting::kFull);
             },
             "hinting"},
            {[](ParagraphStyle&, TextStyle& t, TextStyle&) { t.setSubpixel(false); }, "subpixel"},
            {[](ParagraphStyle& s, TextStyle&, TextStyle&) { s.setFakeMissingFontStyles(false); },
             "fakeMissingFontStyles"},
            {[](ParagraphStyle& s, TextStyle&, TextStyle&) { s.setLetterSpacingByCSSSpec(true); },
             "letterSpacingByCSSSpec"},
            {[](ParagraphStyle& s, TextStyle&, TextStyle&) {
                 StrutStyle strut = s.getStrutStyle();
                 strut.setHalfLeading(true);
                 s.setStrutStyle(strut);
             },
             "strutHalfLeading"},
            {[](ParagraphStyle&, TextStyle&, TextStyle& p) { p.setFontSize(80.0f); },
             "placeholderFontSize"},
    };
    for (const CacheProbe& probe : probes) {
        auto fontCollection = makeCollection();
        layoutHitsCache(fontCollection, nullptr);
        REPORTER_ASSERT(reporter, !layoutHitsCache(fontCollection, &probe), "%s", probe.fName);
    }
}

// Toggling the leading distribution on the same text must change the laid out baseline
DEF_TEST(SkParagraph_CachedLayoutFollowsHalfLeading, reporter) {
    auto baselineOf = [](sk_sp<FontCollection> fontCollection, bool halfLeading) {
        TextStyle textStyle;
        textStyle.setFontSize(13.0f);
        textStyle.setHeightOverride(true);
        textStyle.setHeight(22.5f / 13.0f);
        textStyle.setHalfLeading(halfLeading);
        ParagraphStyle paragraphStyle;
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(textStyle);
        builder.addText("Hello");
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);
        return paragraph->getAlphabeticBaseline();
    };
    auto makeCollection = []() {
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
        return fontCollection;
    };

    for (bool first : {false, true}) {
        const SkScalar expected = baselineOf(makeCollection(), !first);
        auto shared = makeCollection();
        const SkScalar previous = baselineOf(shared, first);
        REPORTER_ASSERT(reporter, !SkScalarNearlyEqual(previous, expected, EPSILON100));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(baselineOf(shared, !first), expected, EPSILON100),
                        "halfLeading %d after %d",
                        !first,
                        first);
    }
}

// A paragraph restored from ParagraphCache must report the same font resolution
DEF_TEST(SkParagraph_CacheRestoresFontResolution, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    bool found = false;
    auto layout = [&]() {
        ParagraphStyle paragraphStyle;
        TextStyle textStyle;
        textStyle.setFontSize(30.0f);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(textStyle);
        builder.addText("Hello \xE2\xBF\xB0 \xF0\x9F\x98\x80");
        builder.pop();
        auto paragraph = builder.Build();
        found = false;
        fontCollection->getParagraphCache()->setChecker(
                [&found](ParagraphImpl*, const char* event, bool) {
                    found |= strcmp(event, "foundParagraph") == 0;
                });
        paragraph->layout(1000.0f);
        return paragraph;
    };

    auto shaped = layout();
    REPORTER_ASSERT(reporter, !found);
    auto cached = layout();
    REPORTER_ASSERT(reporter, found);
    fontCollection->getParagraphCache()->setChecker([](ParagraphImpl*, const char*, bool) {});

    auto* shapedImpl = static_cast<ParagraphImpl*>(shaped.get());
    auto* cachedImpl = static_cast<ParagraphImpl*>(cached.get());
    REPORTER_ASSERT(reporter,
                    cachedImpl->resolvedFonts().size() == shapedImpl->resolvedFonts().size());
}

UNIX_ONLY_TEST(SkParagraph_MemoryLeak, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    std::string text;
    for (size_t i = 0; i < 10; i++) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);

        TextStyle textStyle;
        textStyle.setForegroundColor(paint);
        textStyle.setFontFamilies({SkString("Roboto")});

        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextStyle(textStyle);

        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        text += "Text ";
        builder.addText(text.c_str());

        auto paragraph = builder.Build();
        paragraph->layout(100);

        //used to add a delay so I can monitor memory usage
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

// This test does not produce an image
UNIX_ONLY_TEST(SkParagraph_NoCache1, reporter) {

    ParagraphCache cache;
    cache.turnOn(true);

    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>(true);
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    // Long arabic text with english spaces
    const char* text =
            "من أسر وإعلان الخاصّة وهولندا،, عل قائمة الضغوط بالمطالبة تلك. الصفحة "
            "من أسر وإعلان الخاصّة وهولندا،, عل قائمة الضغوط بالمطالبة تلك. الصفحة "
            "من أسر وإعلان الخاصّة وهولندا،, عل قائمة الضغوط بالمطالبة تلك. الصفحة "
            "من أسر وإعلان الخاصّة وهولندا،, عل قائمة الضغوط بالمطالبة تلك. الصفحة "
            "من أسر وإعلان الخاصّة وهولندا،, عل قائمة الضغوط بالمطالبة تلك. الصفحة "
            "عل بمباركة التقليدية قام عن. تصفح";

    SkString str;

    ParagraphStyle paragraph_style;
    paragraph_style.setTextDirection(TextDirection::kLtr);
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(14);
    text_style.setColor(SK_ColorBLACK);

    auto test = [&](const char* test, const char* text, bool editing) {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
        //SkDebugf("test %s:\n", test);
        builder.pushStyle(text_style);
        builder.addText(text);
        builder.pop();

        auto cache = fontCollection->getParagraphCache();
        auto countBefore = cache->count();
        auto paragraph = builder.Build();
        paragraph->layout(TestCanvasWidth);
        auto countAfter = cache->count();

        if (test == nullptr) {
            return;
        }

        REPORTER_ASSERT(reporter, (countBefore == countAfter) == editing);
    };

    str.append(text);
    test("Long arabic text", str.c_str(), false);

    str.append("عل");
    test("+2 character at the end", str.c_str(), true);

    str = SkString(text);
    test("-2 characters from the end", str.c_str(), true);

    str.insert(0, "عل");
    test("+2 character at the start", str.c_str(), true);

    test("-2 characters from the start", text, true);

    // Make sure that different strings are not flagged as editing
    test("different strings", "0123456789 0123456789 0123456789 0123456789 0123456789", false);
}

static void performGetRectsForRangeConcurrently(skiatest::Reporter* reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    auto const text = std::u16string(42000, 'x');
    ParagraphStyle paragraphStyle;
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(14);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(
            SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(std::numeric_limits<float>::max());

    RectHeightStyle heightStyle = RectHeightStyle::kMax;
    RectWidthStyle widthStyle = RectWidthStyle::kMax;
    auto t1 = std::thread([&] {
        auto result = paragraph->getRectsForRange(0, 2, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, !result.empty());
    });
    auto t2 = std::thread([&] {
        auto result = paragraph->getRectsForRange(5, 10, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, !result.empty());
    });
    t1.join();
    t2.join();
}

UNIX_ONLY_TEST(SkParagraph_GetRectsForRangeConcurrently, reporter) {
    auto const threads_count = 100;
    std::thread threads[threads_count];
    for (auto& thread : threads) {
        thread = std::thread(performGetRectsForRangeConcurrently, reporter);
    }
    for (auto& thread : threads) {
        thread.join();
    }
}

// Concurrent layout, findTypefaces, and defaultFallback on a shared FontCollection must not race
// when FaceCache and VariationCache grow and rehash across threads
UNIX_ONLY_TEST(SkParagraph_FontCollectionConcurrently, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)

    constexpr int kThreadCount = 100;
    SkTypefaceID sharedTypefaceIDs[kThreadCount] = {};
    FontArguments sharedArgs(SkFontArguments().setCollectionIndex(0));
    std::unique_ptr<SkFunctionalExecutor> executor =
            SkFunctionalExecutors::MakeFIFOThreadPool(kThreadCount, false);
    SkTaskGroup tasks(*executor);

    for (int i = 0; i < kThreadCount; ++i) {
        tasks.add([&, i] {
            auto sharedFaces =
                    fontCollection->findTypefaces({SkString("Roboto")}, SkFontStyle(), sharedArgs);
            if (!sharedFaces.empty() && sharedFaces[0]) {
                sharedTypefaceIDs[i] = sharedFaces[0]->uniqueID();
            }

            SkFontArguments skFontArgs = SkFontArguments().setCollectionIndex(i + 1);
            FontArguments fontArgs(skFontArgs);

            ParagraphStyle paragraphStyle;
            TextStyle textStyle;
            textStyle.setFontFamilies({SkString("Roboto")});
            textStyle.setFontSize(14);
            textStyle.setColor(SK_ColorBLACK);
            textStyle.setFontArguments(skFontArgs);

            ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
            builder.pushStyle(textStyle);
            builder.addText(u"Concurrent layout text");
            builder.pop();

            auto paragraph = builder.Build();
            paragraph->layout(300);
            REPORTER_ASSERT(reporter, paragraph->getHeight() > 0);

            fontCollection->findTypefaces({SkString("Roboto")}, SkFontStyle::Bold(), fontArgs);
            fontCollection->defaultFallback(
                    0x276F, {SkString("Roboto")}, SkFontStyle(), SkString("en"), fontArgs);
        });
    }
    tasks.wait();
    REPORTER_ASSERT(reporter, sharedTypefaceIDs[0] != 0);
    for (SkTypefaceID id : sharedTypefaceIDs) {
        REPORTER_ASSERT(reporter, id == sharedTypefaceIDs[0]);
    }
}

// Paragraphs that get their shaped runs from ParagraphCache share the glyph data of the runs.
// Justification of one of them must not change the others.
DEF_TEST(SkParagraph_CacheSharedGlyphData, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->disableFontFallback();
    // The reference does not use the cache
    sk_sp<ResourceFontCollection> referenceFonts = sk_make_sp<ResourceFontCollection>();
    referenceFonts->disableFontFallback();
    referenceFonts->getParagraphCache()->turnOn(false);

    int found = 0;
    fontCollection->getParagraphCache()->setChecker(
            [&found](ParagraphImpl*, const char* event, bool) {
                found += strcmp(event, "foundParagraph") == 0;
            });

    auto build = [&](sk_sp<FontCollection> fonts, TextAlign align) {
        TextStyle style;
        style.setFontFamilies({SkString("Roboto")});
        style.setFontSize(20);
        style.setColor(SK_ColorBLACK);
        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextStyle(style);
        paragraphStyle.setTextAlign(align);
        ParagraphBuilderImpl builder(paragraphStyle, fonts, get_unicode());
        builder.pushStyle(style);
        builder.addText("The quick brown fox jumps over the lazy dog and runs far away");
        auto paragraph = builder.Build();
        paragraph->layout(200);
        return paragraph;
    };
    auto boxes = [](Paragraph* paragraph) {
        std::vector<SkRect> rects;
        for (auto& box :
             paragraph->getRectsForRange(0, 62, RectHeightStyle::kTight, RectWidthStyle::kTight)) {
            rects.push_back(box.rect);
        }
        return rects;
    };

    auto referenceLeft = boxes(build(referenceFonts, TextAlign::kLeft).get());
    auto referenceJustified = boxes(build(referenceFonts, TextAlign::kJustify).get());
    REPORTER_ASSERT(reporter, referenceLeft != referenceJustified);

    auto justified = build(fontCollection, TextAlign::kJustify);
    REPORTER_ASSERT(reporter, found == 0);
    auto left = build(fontCollection, TextAlign::kLeft);
    REPORTER_ASSERT(reporter, found == 1);
    REPORTER_ASSERT(reporter, boxes(justified.get()) == referenceJustified);
    REPORTER_ASSERT(reporter, boxes(left.get()) == referenceLeft);

    // The runs of the paragraph that was shaped first are gone, the others still have the data
    justified.reset();
    auto again = build(fontCollection, TextAlign::kJustify);
    REPORTER_ASSERT(reporter, found == 2);
    REPORTER_ASSERT(reporter, boxes(again.get()) == referenceJustified);
    REPORTER_ASSERT(reporter, boxes(left.get()) == referenceLeft);
    fontCollection->getParagraphCache()->setChecker([](ParagraphImpl*, const char*, bool) {});
}

// A paragraph with a NaN style value must hit the cache like any other: SkLRUCache finds the
// entry it evicts by key equality, so a key unequal to itself crashed on eviction
DEF_TEST(SkParagraph_CacheKeyWithNaNStyleValue, reporter) {
    struct Styles {
        ParagraphStyle paragraph;
        TextStyle text;
        std::optional<PlaceholderStyle> placeholder;
    };
    struct Case {
        const char* name;
        void (*poison)(Styles&, SkScalar nan);
    };
    const Case kCases[] = {
            {"font size", [](Styles& s, SkScalar nan) { s.text.setFontSize(nan); }},
            {"height",
             [](Styles& s, SkScalar nan) {
                 s.text.setHeightOverride(true);
                 s.text.setHeight(nan);
             }},
            {"letter spacing", [](Styles& s, SkScalar nan) { s.text.setLetterSpacing(nan); }},
            {"word spacing", [](Styles& s, SkScalar nan) { s.text.setWordSpacing(nan); }},
            {"baseline shift", [](Styles& s, SkScalar nan) { s.text.setBaselineShift(nan); }},
            {"font variation",
             [](Styles& s, SkScalar nan) {
                 const SkFontArguments::VariationPosition::Coordinate coordinate = {
                         SkSetFourByteTag('w', 'g', 'h', 't'), nan};
                 s.text.setFontArguments(
                         SkFontArguments().setVariationDesignPosition({&coordinate, 1}));
             }},
            {"strut font size",
             [](Styles& s, SkScalar nan) {
                 StrutStyle strut;
                 strut.setStrutEnabled(true);
                 strut.setFontSize(nan);
                 s.paragraph.setStrutStyle(strut);
             }},
            // StrutStyle::operator== compares the values of a disabled strut too
            {"disabled strut leading",
             [](Styles& s, SkScalar nan) {
                 StrutStyle strut;
                 strut.setLeading(nan);
                 s.paragraph.setStrutStyle(strut);
             }},
            {"placeholder width",
             [](Styles& s, SkScalar nan) {
                 s.placeholder = PlaceholderStyle(nan,
                                                  10,
                                                  PlaceholderAlignment::kBaseline,
                                                  TextBaseline::kAlphabetic,
                                                  0);
             }},
            {"placeholder baseline offset",
             [](Styles& s, SkScalar nan) {
                 s.placeholder = PlaceholderStyle(10,
                                                  10,
                                                  PlaceholderAlignment::kBaseline,
                                                  TextBaseline::kAlphabetic,
                                                  nan);
             }},
    };

    auto fonts = sk_make_sp<FontCollection>();
    auto build = [&](const Styles& styles, const char* text) {
        ParagraphBuilderImpl builder(styles.paragraph, fonts, get_unicode());
        builder.pushStyle(styles.text);
        builder.addText(text);
        if (styles.placeholder) {
            builder.addPlaceholder(*styles.placeholder);
        }
        builder.pop();
        return builder.Build();
    };
    constexpr int kCacheSize = 128;  // ParagraphCache::Cache::kMaxEntries
    for (const Case& c : kCases) {
        skiatest::ReporterContext context(reporter, SkString(c.name));
        ParagraphCache cache;
        Styles poisoned;
        c.poison(poisoned, SK_ScalarNaN);
        auto paragraph = build(poisoned, "text");
        cache.updateParagraph(static_cast<ParagraphImpl*>(paragraph.get()));
        REPORTER_ASSERT(reporter, cache.count() == 1, "got %d entries", cache.count());

        // 0/0 on x86 gives a NaN with the sign bit set, so the bits differ from SK_ScalarNaN
        Styles samePoison;
        c.poison(samePoison, -SK_ScalarNaN);
        auto same = build(samePoison, "text");
        REPORTER_ASSERT(reporter, cache.findParagraph(static_cast<ParagraphImpl*>(same.get())));
        cache.updateParagraph(static_cast<ParagraphImpl*>(same.get()));
        REPORTER_ASSERT(reporter, cache.count() == 1, "got %d entries", cache.count());

        // The poisoned entry is the oldest one, so it is evicted first
        for (int i = 0; i < kCacheSize; ++i) {
            auto filler = build(Styles(), SkStringPrintf("filler %d", i).c_str());
            cache.updateParagraph(static_cast<ParagraphImpl*>(filler.get()));
        }
        REPORTER_ASSERT(reporter, cache.count() == kCacheSize, "got %d entries", cache.count());
    }
}

// FaceCache and VariationCache are hash maps keyed by FontArguments: a NaN axis value must find
// its own entry instead of cloning a new typeface (and adding an entry) on every lookup
UNIX_ONLY_TEST(SkParagraph_FontCollectionCachesNaNVariation, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    auto find = [&](SkScalar nan) {
        const SkFontArguments::VariationPosition::Coordinate coordinate = {
                SkSetFourByteTag('w', 'g', 'h', 't'), nan};
        FontArguments args(SkFontArguments().setVariationDesignPosition({&coordinate, 1}));
        return fontCollection->findTypefaces({SkString("Roboto")}, SkFontStyle(), args);
    };
    auto first = find(SK_ScalarNaN);
    // The sign bit gives a NaN with other bits, as 0/0 does on x86
    auto second = find(-SK_ScalarNaN);
    REPORTER_ASSERT(reporter, first.size() == 1, "got %zu typefaces", first.size());
    REPORTER_ASSERT(reporter, second.size() == 1, "got %zu typefaces", second.size());
    if (first.size() != 1 || second.size() != 1) {
        return;
    }
    REPORTER_ASSERT(reporter, first[0]->uniqueID() == second[0]->uniqueID(),
                    "got typefaces %u and %u", first[0]->uniqueID(), second[0]->uniqueID());
}

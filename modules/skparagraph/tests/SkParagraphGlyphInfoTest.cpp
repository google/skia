/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/DartTypes.h"
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
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>
#include <memory>
#include <string>
#include <vector>

using namespace skia::textlayout;
using namespace skia::textlayout::test;

UNIX_ONLY_TEST(SkParagraph_GlyphCluster_Ligature, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    const char* text = "fi";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(20);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    Paragraph::GlyphClusterInfo glyphInfo;
    REPORTER_ASSERT(reporter, paragraph->getGlyphClusterAt(0, &glyphInfo));
    REPORTER_ASSERT(reporter, glyphInfo.fClusterTextRange.start == 0);
    REPORTER_ASSERT(reporter, glyphInfo.fClusterTextRange.end == len);
}

UNIX_ONLY_TEST(SkParagraph_GlyphInfo_LigatureDiacritics, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    // Ligature + diacritics
    // text = lam + hamza + alef + hamza
    // lam and alef form a laam-alif ligature and the 2 hamza are diacritical
    // marks the combines into the ligated base glyphs.
    const char* text = "لٔأ";
    const size_t len = strlen(text);

    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection, get_unicode());
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Katibeh")});
    text_style.setFontSize(100);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);
    TestCanvas canvas("SkParagraph_laam_alif_diacritics.png");
    paragraph->paint(canvas.get(), 50, 50);

    Paragraph::GlyphInfo glyphInfo;

    const auto boxes =
            paragraph->getRectsForRange(0, len, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, boxes.size() == 1);
    const SkRect fullRect = boxes[0].rect;

    REPORTER_ASSERT(reporter, paragraph->getGlyphInfoAtUTF16Offset(0, &glyphInfo));
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.start == 0);
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.end == 2);
    const SkRect rect0 = glyphInfo.fGraphemeLayoutBounds;

    REPORTER_ASSERT(reporter, paragraph->getGlyphInfoAtUTF16Offset(1, &glyphInfo));
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.start == 0);
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.end == 2);
    const SkRect rect1 = glyphInfo.fGraphemeLayoutBounds;
    REPORTER_ASSERT(reporter, rect0 == rect1);

    REPORTER_ASSERT(reporter, paragraph->getGlyphInfoAtUTF16Offset(2, &glyphInfo));
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.start == 2);
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.end == 4);
    const SkRect rect2 = glyphInfo.fGraphemeLayoutBounds;
    // The latter half of the text forms a different grapheme.
    REPORTER_ASSERT(reporter, rect2 != rect1);

    REPORTER_ASSERT(reporter, paragraph->getGlyphInfoAtUTF16Offset(3, &glyphInfo));
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.start == 2);
    REPORTER_ASSERT(reporter, glyphInfo.fGraphemeClusterTextRange.end == 4);
    const SkRect rect3 = glyphInfo.fGraphemeLayoutBounds;
    REPORTER_ASSERT(reporter, rect2 == rect3);

    // RTL text.
    REPORTER_ASSERT(reporter, fullRect.left() == rect2.left());
    REPORTER_ASSERT(reporter, fullRect.right() == rect0.right());

    // Currently it seems the 2 grapheme rects do not overlap each other.
    // REPORTER_ASSERT(reporter, rect2.right() < rect0.left());
}

// Verify compound clusters with zero-width codepoints split into separate clusters
DEF_TEST(SkParagraph_ZeroWidthClusterSplit, reporter) {
    sk_sp<SkFontMgr> mgr = ToolUtils::TestFontMgr();
    auto abcTypeface = mgr->makeFromStream(GetResourceAsStream("fonts/abc/abc.ttf"), {});
    // Portable font manager returns null for stream-backed typefaces when native fonts are disabled
    if (!abcTypeface) {
        return;
    }
    auto fontCollection = sk_make_sp<FontCollection>();
    auto fontProvider = sk_make_sp<TypefaceFontProvider>();
    fontProvider->registerTypeface(abcTypeface, SkString("AbcFont"));
    fontCollection->setAssetFontManager(fontProvider);
    fontCollection->setDefaultFontManager(mgr);

    ParagraphStyle paragraphStyle;
    TextStyle style;
    style.setFontFamilies({SkString("AbcFont")});
    style.setFontSize(14.0f);
    paragraphStyle.setTextStyle(style);

    {
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(style);
        builder.addText("b\u200B");
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(100.0f);

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        auto clusterBIndex = impl->clusterIndex(0);
        auto clusterZWSPIndex = impl->clusterIndex(1);
        REPORTER_ASSERT(reporter, clusterBIndex != clusterZWSPIndex);

        const auto& clusterB = impl->cluster(clusterBIndex);
        const auto& clusterZWSP = impl->cluster(clusterZWSPIndex);
        REPORTER_ASSERT(reporter, clusterB.textRange() == TextRange(0, 1));
        REPORTER_ASSERT(reporter, clusterB.width() > 0);
        REPORTER_ASSERT(reporter, clusterB.size() == 1);
        REPORTER_ASSERT(reporter, clusterZWSP.textRange() == TextRange(1, 4));
        REPORTER_ASSERT(reporter, clusterZWSP.width() == 0);
        REPORTER_ASSERT(reporter, clusterZWSP.size() == 0);
    }

    {
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(style);
        builder.addText("a\u200Eb");
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(100.0f);

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        auto clusterAIndex = impl->clusterIndex(0);
        auto clusterLRMIndex = impl->clusterIndex(1);
        auto clusterBIndex = impl->clusterIndex(4);
        REPORTER_ASSERT(reporter, clusterAIndex != clusterLRMIndex);
        REPORTER_ASSERT(reporter, clusterLRMIndex != clusterBIndex);

        const auto& clusterA = impl->cluster(clusterAIndex);
        const auto& clusterLRM = impl->cluster(clusterLRMIndex);
        const auto& clusterB = impl->cluster(clusterBIndex);
        REPORTER_ASSERT(reporter, clusterA.textRange() == TextRange(0, 1));
        REPORTER_ASSERT(reporter, clusterA.width() > 0);
        REPORTER_ASSERT(reporter, clusterA.size() == 1);
        REPORTER_ASSERT(reporter, clusterLRM.textRange() == TextRange(1, 4));
        REPORTER_ASSERT(reporter, clusterLRM.width() == 0);
        REPORTER_ASSERT(reporter, clusterLRM.size() == 0);
        REPORTER_ASSERT(reporter, clusterB.textRange() == TextRange(4, 5));
        REPORTER_ASSERT(reporter, clusterB.width() > 0);
        REPORTER_ASSERT(reporter, clusterB.size() == 1);
    }

    {
        ParagraphStyle rtlStyle;
        rtlStyle.setTextDirection(TextDirection::kRtl);
        TextStyle rtlStyleText;
        rtlStyleText.setFontSize(14.0f);
        rtlStyle.setTextStyle(rtlStyleText);

        auto defaultCollection = sk_make_sp<FontCollection>();
        defaultCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

        ParagraphBuilderImpl builder(rtlStyle, defaultCollection, get_unicode());
        builder.pushStyle(rtlStyleText);
        builder.addText("\u0633\u200F");
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(100.0f);

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        for (size_t i = 0; i <= impl->text().size(); ++i) {
            REPORTER_ASSERT(reporter, impl->clusterIndex(i) != EMPTY_INDEX);
        }
    }
}

// ExtendedVisitorInfo::positions can be changed: the change stays in the glyph data. If the
// paragraph was not painted or visited yet, the next paint() draws the glyphs at the changed
// positions. paint() and visit() build the text blobs one time for each layout().
DEF_TEST(SkParagraph_ExtendedVisitPositionsAreWritable, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->disableFontFallback();

    TextStyle style;
    style.setFontFamilies({SkString("Roboto")});
    style.setFontSize(20);
    style.setColor(SK_ColorBLACK);
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextStyle(style);

    auto build = [&] {
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        builder.pushStyle(style);
        builder.addText("Hello world");
        auto paragraph = builder.Build();
        paragraph->layout(300);
        return paragraph;
    };
    // The first row that has a pixel which is not white
    auto firstInkRow = [](Paragraph* paragraph) {
        SkBitmap bitmap;
        bitmap.allocN32Pixels(300, 100);
        SkCanvas canvas(bitmap);
        canvas.clear(SK_ColorWHITE);
        paragraph->paint(&canvas, 0, 0);
        for (int y = 0; y < bitmap.height(); ++y) {
            for (int x = 0; x < bitmap.width(); ++x) {
                if (bitmap.getColor(x, y) != SK_ColorWHITE) {
                    return y;
                }
            }
        }
        return -1;
    };

    auto reference = build();
    int referenceRow = firstInkRow(reference.get());
    REPORTER_ASSERT(reporter, referenceRow >= 0);

    auto paragraph = build();
    int glyphs = 0;
    SkPoint* firstPosition = nullptr;
    paragraph->extendedVisit([&](int, const Paragraph::ExtendedVisitorInfo* info) {
        for (int i = 0; info && i < info->count; ++i) {
            info->positions[i].fY += 30;
            ++glyphs;
        }
        if (info && !firstPosition) {
            firstPosition = info->positions;
        }
    });
    REPORTER_ASSERT(reporter, glyphs == 11);

    // The pointer stays valid, and the paragraph keeps the change
    int checked = 0;
    paragraph->extendedVisit([&](int, const Paragraph::ExtendedVisitorInfo* info) {
        if (info && checked++ == 0) {
            REPORTER_ASSERT(reporter, info->positions == firstPosition);
        }
        for (int i = 0; info && i < info->count; ++i) {
            REPORTER_ASSERT(reporter, info->positions[i].fY == 30);
        }
    });
    REPORTER_ASSERT(reporter, firstInkRow(paragraph.get()) == referenceRow + 30);
}

namespace {
// Vocalized Arabic: every harakah is a mark that HarfBuzz places with a non-zero glyph offset
constexpr char16_t kVocalizedArabic[] =
        u"\u0645\u064E\u0631\u0652\u062D\u064E\u0628\u064B\u0627 "
        u"\u0634\u064F\u0643\u0652\u0631\u064B\u0627 "
        u"\u0645\u064E\u0631\u0652\u062D\u064E\u0628\u064B\u0627";

std::unique_ptr<Paragraph> buildVocalizedArabic(sk_sp<FontCollection> fontCollection,
                                                const ParagraphStyle& paragraphStyle,
                                                SkScalar width) {
    TextStyle arabic;
    arabic.setFontFamilies({SkString("Noto Naskh Arabic")});
    arabic.setFontSize(20);
    arabic.setColor(SK_ColorBLACK);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(arabic);
    builder.addText(kVocalizedArabic);
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(width);
    return paragraph;
}

std::vector<SkPoint> drawnPositions(Paragraph* paragraph) {
    std::vector<SkPoint> positions;
    paragraph->visit([&](int, const Paragraph::VisitorInfo* info) {
        for (int i = 0; info && i < info->count; ++i) {
            positions.push_back(info->positions[i]);
        }
    });
    return positions;
}

bool hasNonZeroOffset(const Run& run) {
    bool nonZero = false;
    for (auto offset : run.offsets()) {
        nonZero |= offset != SkPoint{0, 0};
    }
    return nonZero;
}
}  // namespace

// The ellipsis is shaped outside OneLineShaper, and its marks still need their offsets
DEF_TEST(SkParagraph_EllipsisKeepsGlyphOffsets, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->disableFontFallback();

    ParagraphStyle paragraphStyle;
    paragraphStyle.setMaxLines(1);
    paragraphStyle.setEllipsis(std::u16string(u"\u0628\u064E\u0628\u064E"));
    paragraphStyle.setTextDirection(TextDirection::kRtl);
    auto paragraph = buildVocalizedArabic(fontCollection, paragraphStyle, 120);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);
    const Run* ellipsis = impl->lines()[0].ellipsis();
    REPORTER_ASSERT(reporter, ellipsis != nullptr);
    if (ellipsis) {
        REPORTER_ASSERT(reporter, hasNonZeroOffset(*ellipsis));
    }
}

// A paragraph built from the cache shares the glyph data, offsets included, with the
// paragraph that filled the cache, so both draw the same glyph positions
DEF_TEST(SkParagraph_CachedRunsKeepGlyphOffsets, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->disableFontFallback();
    fontCollection->getParagraphCache()->reset();
    fontCollection->getParagraphCache()->turnOn(true);

    ParagraphStyle paragraphStyle;
    auto cold = buildVocalizedArabic(fontCollection, paragraphStyle, 1000);
    REPORTER_ASSERT(reporter, fontCollection->getParagraphCache()->count() == 1);
    auto cached = buildVocalizedArabic(fontCollection, paragraphStyle, 1000);

    auto cachedImpl = static_cast<ParagraphImpl*>(cached.get());
    bool anyOffsets = false;
    for (auto& run : cachedImpl->runs()) {
        anyOffsets |= hasNonZeroOffset(run);
    }
    REPORTER_ASSERT(reporter, anyOffsets);
    REPORTER_ASSERT(reporter, drawnPositions(cold.get()) == drawnPositions(cached.get()));
    fontCollection->getParagraphCache()->turnOn(false);
}

// A run keeps glyph offsets only if one of them is not zero, and the drawn glyphs use them
DEF_TEST(SkParagraph_GlyphOffsetsOnlyWhenNeeded, reporter) {
    sk_sp<ResourceFontCollection> fontCollection = sk_make_sp<ResourceFontCollection>();
    SKIP_IF_FONTS_NOT_FOUND(reporter, fontCollection)
    fontCollection->disableFontFallback();

    TextStyle latin;
    latin.setFontFamilies({SkString("Roboto")});
    latin.setFontSize(20);
    latin.setColor(SK_ColorBLACK);
    TextStyle arabic = latin;
    // U+2605 is not in Noto Naskh Arabic, so the Arabic run is cut into pieces around it
    arabic.setFontFamilies({SkString("Noto Naskh Arabic"), SkString("Noto Sans CJK JP")});
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextStyle(latin);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    builder.pushStyle(latin);
    builder.addText("Hello ");
    builder.pop();
    builder.pushStyle(arabic);
    builder.addText(
            u"\u0645\u064E\u0631\u0652\u062D\u064E\u0628\u064B\u0627\u2605"
            u"\u0634\u064F\u0643\u0652\u0631\u064B\u0627");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(1000);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    int runsWithOffsets = 0;
    for (auto& run : impl->runs()) {
        SkString family;
        run.font().getTypeface()->getFamilyName(&family);
        if (family.equals("Roboto")) {
            REPORTER_ASSERT(reporter, run.offsets().empty());
        } else if (family.equals("Noto Naskh Arabic")) {
            bool nonZero = false;
            for (auto offset : run.offsets()) {
                nonZero |= offset != SkPoint{0, 0};
            }
            REPORTER_ASSERT(reporter, nonZero);
            runsWithOffsets += nonZero;
        }
    }
    REPORTER_ASSERT(reporter, runsWithOffsets == 2);

    // The marks are drawn above and below the letters
    bool movedUpOrDown = false;
    paragraph->visit([&](int, const Paragraph::VisitorInfo* info) {
        for (int i = 0; info && i < info->count; ++i) {
            movedUpOrDown |= info->positions[i].fY != 0;
        }
    });
    REPORTER_ASSERT(reporter, movedUpOrDown);
}

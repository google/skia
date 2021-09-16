// Copyright 2021 Google LLC.
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/src/Paint.h"

#include <string.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct GrContextOptions;

#define VeryLongCanvasWidth 1000000
#define TestCanvasWidth 1000
#define TestCanvasHeight 600

#if !defined(SK_BUILD_FOR_UNIX)
#undef DEF_TEST
#define DEF_TEST(name, reporter)                                                          \
    static void test_##name(skiatest::Reporter* reporter, const GrContextOptions&);       \
    skiatest::TestRegistry name##TestRegistry(skiatest::Test(#name, false, test_##name)); \
    void test_##name(skiatest::Reporter* reporter, const GrContextOptions&) {             \
        /* SkDebugf("Disabled:"#name "\n"); */                                            \
    }                                                                                     \
    void disabled_##name(skiatest::Reporter* reporter, const GrContextOptions&)
#endif

using namespace skia::text;

namespace {
    bool operator==(SkSpan<const char16_t> a, SkSpan<const char16_t> b) {
        if (a.size() != b.size()) {
            return false;
        }
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i] != b[i]) {
                return false;
            }
        }
        return true;
    }
}

DEF_TEST(SkText_ShapedText_LTR, reporter) {
    TrivialFontChain* fontChain = new TrivialFontChain("Roboto", 40.0f, SkFontStyle::Normal());
    if (fontChain->empty()) return;

    std::u16string utf16(u"Hello world\nHello world");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), sk_ref_sp<FontChain>(fontChain));
    auto shapedText = unicodeText.shape(SkSpan<FontBlock>(&fontBlock, 1), TextDirection::kLtr);
    auto logicalRuns = shapedText->getLogicalRuns();

    auto newLine = utf16.find_first_of(u"\n");
    REPORTER_ASSERT(reporter, logicalRuns.size() == 3);
    REPORTER_ASSERT(reporter, logicalRuns[1].getRunType() == LogicalRunType::kLineBreak);
    REPORTER_ASSERT(reporter, logicalRuns[1].getTextRange() == TextRange(newLine, newLine + 1));
}

DEF_TEST(SkText_ShapedText_RTL, reporter) {
    sk_sp<TrivialFontChain> fontChain = sk_make_sp<TrivialFontChain>("Roboto", 40.0f, SkFontStyle::Normal());
    if (fontChain->empty()) return;

    std::u16string utf16(u"\u202EHELLO WORLD\nHELLO WORLD");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), fontChain);
    auto shapedText = unicodeText.shape(SkSpan<FontBlock>(&fontBlock, 1), TextDirection::kLtr);
    auto logicalRuns = shapedText->getLogicalRuns();

    auto newLine = utf16.find_first_of(u"\n");
    REPORTER_ASSERT(reporter, logicalRuns.size() == 3);
    REPORTER_ASSERT(reporter, logicalRuns[1].getRunType() == LogicalRunType::kLineBreak);
    REPORTER_ASSERT(reporter, logicalRuns[1].getTextRange() == TextRange(newLine, newLine + 1));
}

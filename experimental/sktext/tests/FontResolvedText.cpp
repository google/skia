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

using namespace skia::text;

UNIX_ONLY_TEST(SkText_FontResolution1, reporter) {
    TrivialFontChain* fontChain = new TrivialFontChain("Roboto", 40.0f, SkFontStyle::Normal());
    if (fontChain->empty()) return;

    std::u16string utf16(u"Hello world");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), sk_ref_sp<FontChain>(fontChain));
    auto fontResolvedText = unicodeText.resolveFonts(SkSpan<FontBlock>(&fontBlock, 1));

    auto resolvedFonts = fontResolvedText->resolvedFonts();

    REPORTER_ASSERT(reporter, resolvedFonts.size() == 1);
    REPORTER_ASSERT(reporter, resolvedFonts.front().textRange.width() == utf16.size());
    REPORTER_ASSERT(reporter, resolvedFonts.front().typeface->uniqueID() == fontChain->getTypeface()->uniqueID());
    REPORTER_ASSERT(reporter, resolvedFonts.front().size == 40.0f);
    REPORTER_ASSERT(reporter, resolvedFonts.front().style == SkFontStyle::Normal());
}

UNIX_ONLY_TEST(SkText_FontResolution3, reporter) {
    MultipleFontChain* fontChain = new MultipleFontChain({ "Roboto", "Noto Color Emoji", "Noto Serif CJK JP" }, 40.0f, SkFontStyle::Normal());
    if (fontChain->count() < 3) return;

    std::u16string utf16(u"English English 字典 字典 😀😃😄 😀😃😄");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), sk_ref_sp<FontChain>(fontChain));
    auto fontResolvedText = unicodeText.resolveFonts(SkSpan<FontBlock>(&fontBlock, 1));

    auto resolvedFonts = fontResolvedText->resolvedFonts();

    TextIndex prev = 0;
    for (auto& rf : resolvedFonts) {
        REPORTER_ASSERT(reporter, prev == rf.textRange.fStart);
        REPORTER_ASSERT(reporter, rf.textRange.width() > 0.0f);
        prev = rf.textRange.fEnd;
    }

    REPORTER_ASSERT(reporter, resolvedFonts.size() == 8 /* 1English 3English spaces + 2Emoji + 2JP */);
    REPORTER_ASSERT(reporter, resolvedFonts[0].textRange.fStart == 0);
    REPORTER_ASSERT(reporter, resolvedFonts[7].textRange.fEnd == utf16.size());
    REPORTER_ASSERT(reporter, resolvedFonts[0].size == 40.0f);
    REPORTER_ASSERT(reporter, resolvedFonts[0].style == SkFontStyle::Normal());
}

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

struct TestLine {
    size_t index;
    TextRange lineText;
    bool hardBreak;
    SkRect bounds;
    GlyphRange trailingSpaces;
    Range<RunIndex> runRange;
};

struct TestRun {
    const SkFont& font;
    TextRange textRange;        // Currently we make sure that the run edges are the grapheme cluster edges
    SkRect bounds;              // bounds contains the physical boundaries of the run
    int trailingSpaces;         // Depending of TextDirection it goes right to the end (LTR) or left to the start (RTL)
    SkSpan<const uint16_t> glyphs;
    SkSpan<const SkPoint> positions;
    SkSpan<const TextIndex> clusters;
};

class TestVisitor : public Visitor {
public:
    void onBeginLine(size_t index, TextRange lineText, bool hardBreak, SkRect bounds) override {
        SkASSERT(fTestLines.size() == index);
        fTestLines.push_back({ index, lineText, hardBreak, bounds, EMPTY_RANGE, Range<RunIndex>(fTestRuns.size(), fTestRuns.size()) });
    }
    void onEndLine(size_t index, TextRange lineText, GlyphRange trailingSpaces, size_t glyphCount) override {
        SkASSERT(fTestLines.size() == index + 1);
        fTestLines.back().trailingSpaces = trailingSpaces;
        fTestLines.back().runRange.fEnd = fTestRuns.size();
    }
    void onGlyphRun(const SkFont& font,
                    TextRange textRange,        // Currently we make sure that the run edges are the grapheme cluster edges
                    SkRect bounds,              // bounds contains the physical boundaries of the run
                    int trailingSpaces,         // Depending of TextDirection it goes right to the end (LTR) or left to the start (RTL)
                    int glyphCount,             // Just the number of glyphs
                    const uint16_t glyphs[],
                    const SkPoint positions[],        // Positions relative to the line
                    const TextIndex clusters[]) override
    {
        fTestRuns.push_back({font, textRange, bounds, trailingSpaces,
                            SkSpan<const uint16_t>(&glyphs[0], glyphCount),
                            SkSpan<const SkPoint>(&positions[0], glyphCount + 1),
                            SkSpan<const TextIndex>(&clusters[0], glyphCount + 1),
                            });
    }
    void onPlaceholder(TextRange, const SkRect& bounds) override { }

    std::vector<TestLine> fTestLines;
    std::vector<TestRun> fTestRuns;
};


UNIX_ONLY_TEST(SkText_WrappedText_Spaces, reporter) {
    sk_sp<TrivialFontChain> fontChain = sk_make_sp<TrivialFontChain>("Roboto", 40.0f, SkFontStyle::Normal());
    if (fontChain->empty()) return;

    std::u16string utf16(u"    Leading spaces\nTrailing spaces    \nLong text with collapsed      spaces inside wrapped into few lines");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), fontChain);
    auto fontResolvedText = unicodeText.resolveFonts(SkSpan<FontBlock>(&fontBlock, 1));
    auto shapedText = fontResolvedText->shape(&unicodeText, TextDirection::kLtr);
    auto wrappedText = shapedText->wrap(&unicodeText, 440.0f, 500.0f);

    TestVisitor testVisitor;
    wrappedText->visit(&testVisitor);

    REPORTER_ASSERT(reporter, testVisitor.fTestLines.size() == 5);
    REPORTER_ASSERT(reporter, testVisitor.fTestRuns.size() == 5);

    REPORTER_ASSERT(reporter, testVisitor.fTestLines[0].trailingSpaces.width() == 0);
    REPORTER_ASSERT(reporter, testVisitor.fTestLines[1].trailingSpaces.width() == 4);
    REPORTER_ASSERT(reporter, testVisitor.fTestLines[2].trailingSpaces.width() == 6);
    REPORTER_ASSERT(reporter, testVisitor.fTestLines[3].trailingSpaces.width() == 1);
    REPORTER_ASSERT(reporter, testVisitor.fTestLines[4].trailingSpaces.width() == 0);

    auto break1 = utf16.find_first_of(u"\n");
    auto break2 = utf16.find_last_of(u"\n");

    REPORTER_ASSERT(reporter, testVisitor.fTestLines[0].lineText.width() == break1);
    REPORTER_ASSERT(reporter, testVisitor.fTestLines[1].lineText.width() == break2 - break1 - 1);

    RunIndex runIndex = 0;
    SkScalar verticalOffset = 0.0f;
    for (int lineIndex = 0; lineIndex < testVisitor.fTestLines.size(); ++lineIndex) {
        auto& line = testVisitor.fTestLines[lineIndex];
        REPORTER_ASSERT(reporter, line.runRange == Range<RunIndex>(runIndex, runIndex + 1));
        REPORTER_ASSERT(reporter, line.runRange.width() == 1);
        auto& run = testVisitor.fTestRuns[runIndex];
        REPORTER_ASSERT(reporter, line.lineText == run.textRange);
        REPORTER_ASSERT(reporter, runIndex <= 1 ? line.hardBreak : !line.hardBreak);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(verticalOffset, line.bounds.fTop));

        // There is only one line that is wrapped and it has enough trailing spaces to exceed the line width
        REPORTER_ASSERT(reporter, (line.index == 2 ? line.bounds.width() > 440.0f: line.bounds.width() < 440.0f));
        verticalOffset = line.bounds.fBottom;
        ++runIndex;
    }
}

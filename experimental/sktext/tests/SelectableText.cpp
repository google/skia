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
    size_t glyphCount;
};

struct TestRun {
    const SkFont& font;
    DirTextRange dirTextRange;  // Currently we make sure that the run edges are the grapheme cluster edges
    SkRect bounds;              // bounds contains the physical boundaries of the run
    size_t trailingSpaces;      // Depending of TextDirection it goes right to the end (LTR) or left to the start (RTL)
    SkSpan<const uint16_t> glyphs;
    SkSpan<const SkPoint> positions;
    SkSpan<const TextIndex> clusters;
};

class TestVisitor : public Visitor {
public:
    void onBeginLine(size_t index, TextRange lineText, bool hardBreak, SkRect bounds) override {
        SkASSERT(fTestLines.size() == index);
        fTestLines.push_back({ index, lineText, hardBreak, bounds, EMPTY_RANGE, Range<RunIndex>(fTestRuns.size(), fTestRuns.size()), 0 });
    }
    void onEndLine(size_t index, TextRange lineText, GlyphRange trailingSpaces, size_t glyphCount) override {
        SkASSERT(fTestLines.size() == index + 1);
        fTestLines.back().trailingSpaces = trailingSpaces;
        fTestLines.back().runRange.fEnd = fTestRuns.size();
        fTestLines.back().glyphCount = glyphCount;
    }
    void onGlyphRun(const SkFont& font,
                    DirTextRange dirTextRange,
                    SkRect bounds,
                    TextIndex trailingSpaces,
                    size_t glyphCount,          // Just the number of glyphs
                    const uint16_t glyphs[],
                    const SkPoint positions[],        // Positions relative to the line
                    const TextIndex clusters[]) override
    {
        fTestRuns.push_back({font, dirTextRange, bounds, trailingSpaces,
                            SkSpan<const uint16_t>(&glyphs[0], glyphCount),
                            SkSpan<const SkPoint>(&positions[0], glyphCount + 1),
                            SkSpan<const TextIndex>(&clusters[0], glyphCount + 1),
                            });
    }
    void onPlaceholder(TextRange, const SkRect& bounds) override { }

    std::vector<TestLine> fTestLines;
    std::vector<TestRun> fTestRuns;
};

UNIX_ONLY_TEST(SkText_SelectableText_Bounds, reporter) {
    sk_sp<TrivialFontChain> fontChain = sk_make_sp<TrivialFontChain>("Roboto", 40.0f, SkFontStyle::Normal());
    if (fontChain->empty()) return;

    std::u16string utf16(u"    Leading spaces\nTrailing spaces    \nLong text with collapsed      spaces inside wrapped into few lines");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), fontChain);
    auto fontResolvedText = unicodeText.resolveFonts(SkSpan<FontBlock>(&fontBlock, 1));
    auto shapedText = fontResolvedText->shape(&unicodeText, TextDirection::kLtr);
    auto wrappedText = shapedText->wrap(&unicodeText, 440.0f, 500.0f);
    auto selectableText = wrappedText->prepareToEdit(&unicodeText);

    TestVisitor testVisitor;
    wrappedText->visit(&testVisitor);

    REPORTER_ASSERT(reporter, selectableText->countLines() == 5);
    for (LineIndex lineIndex = 0; lineIndex < selectableText->countLines(); ++lineIndex) {
        auto& testLine = testVisitor.fTestLines[lineIndex];
        auto boxLine = selectableText->getLine(lineIndex);
        SkScalar left = boxLine.fBounds.fLeft;
        for (auto& box : boxLine.fBoxGlyphs) {
            REPORTER_ASSERT(reporter, boxLine.fBounds.contains(box) || box.isEmpty());
            REPORTER_ASSERT(reporter, left <= box.fLeft);
            left = box.fRight;
        }

        GlyphIndex trailingSpaces = boxLine.fBoxGlyphs.size() - 1;
        for (RunIndex runIndex = testLine.runRange.fEnd; runIndex > testLine.runRange.fStart; --runIndex) {
            auto& testRun = testVisitor.fTestRuns[runIndex - 1];
            if (testRun.trailingSpaces == 0) {
                trailingSpaces -= testRun.glyphs.size();
            } else {
                trailingSpaces -= (testRun.glyphs.size() - testRun.trailingSpaces);
                break;
            }
        }

        REPORTER_ASSERT(reporter, boxLine.fTrailingSpacesEnd == testLine.trailingSpaces.fEnd);
        REPORTER_ASSERT(reporter, boxLine.fTextEnd == trailingSpaces);
        REPORTER_ASSERT(reporter, boxLine.fTextRange == testLine.lineText);
        REPORTER_ASSERT(reporter, boxLine.fIndex == lineIndex);
        REPORTER_ASSERT(reporter, boxLine.fIsHardBreak == testLine.hardBreak);
        REPORTER_ASSERT(reporter, boxLine.fBounds == testLine.bounds);
    }
}

UNIX_ONLY_TEST(SkText_SelectableText_Navigation_FirstLast, reporter) {
    sk_sp<TrivialFontChain> fontChain = sk_make_sp<TrivialFontChain>("Roboto", 40.0f, SkFontStyle::Normal());
    if (fontChain->empty()) return;

    std::u16string utf16(u"    Leading spaces\nTrailing spaces    \nLong text with collapsed      spaces inside wrapped into few lines");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), fontChain);
    auto fontResolvedText = unicodeText.resolveFonts(SkSpan<FontBlock>(&fontBlock, 1));
    auto shapedText = fontResolvedText->shape(&unicodeText, TextDirection::kLtr);
    auto wrappedText = shapedText->wrap(&unicodeText, 440.0f, 500.0f);
    auto selectableText = wrappedText->prepareToEdit(&unicodeText);

    TestVisitor testVisitor;
    wrappedText->visit(&testVisitor);

    // First position
    auto firstLine = testVisitor.fTestLines.front();
    auto firstRun = testVisitor.fTestRuns.front();
    auto firstPosition = selectableText->firstPosition(PositionType::kGraphemeCluster);
    REPORTER_ASSERT(reporter, firstPosition.fLineIndex == 0);
    REPORTER_ASSERT(reporter, firstPosition.fTextRange == TextRange(0, 0));
    REPORTER_ASSERT(reporter, firstPosition.fGlyphRange == GlyphRange(0, 0));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(firstPosition.fBoundaries.fLeft, 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(firstPosition.fBoundaries.fTop, 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(firstPosition.fBoundaries.width(), 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(firstPosition.fBoundaries.height(), firstLine.bounds.height()));

    // Last position
    auto lastLine = testVisitor.fTestLines.back();
    auto lastRun = testVisitor.fTestRuns.back();
    auto lastPosition = selectableText->lastPosition(PositionType::kGraphemeCluster);
    REPORTER_ASSERT(reporter, lastPosition.fLineIndex == testVisitor.fTestLines.size() - 1);
    REPORTER_ASSERT(reporter, lastPosition.fTextRange == TextRange(utf16.size(), utf16.size()));
    REPORTER_ASSERT(reporter, lastPosition.fGlyphRange == GlyphRange(lastRun.glyphs.size(), lastRun.glyphs.size()));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lastPosition.fBoundaries.fLeft, lastRun.positions[lastRun.glyphs.size()].fX));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lastPosition.fBoundaries.fTop, lastLine.bounds.fTop));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lastPosition.fBoundaries.width(), 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lastPosition.fBoundaries.height(), lastLine.bounds.height()));
}

UNIX_ONLY_TEST(SkText_SelectableText_ScanRightByGraphemeClusters, reporter) {
    sk_sp<TrivialFontChain> fontChain = sk_make_sp<TrivialFontChain>("Roboto", 40.0f, SkFontStyle::Normal());
    if (fontChain->empty()) return;

    std::u16string utf16(u"    Small Text   \n");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), fontChain);
    auto fontResolvedText = unicodeText.resolveFonts(SkSpan<FontBlock>(&fontBlock, 1));
    auto shapedText = fontResolvedText->shape(&unicodeText, TextDirection::kLtr);
    auto wrappedText = shapedText->wrap(&unicodeText, 440.0f, 500.0f);
    auto selectableText = wrappedText->prepareToEdit(&unicodeText);

    TestVisitor testVisitor;
    wrappedText->visit(&testVisitor);

    auto firstPosition = selectableText->firstPosition(PositionType::kGraphemeCluster);
    auto lastPosition = selectableText->lastPosition(PositionType::kGraphemeCluster);

    auto position = firstPosition;
    while (!(position.fGlyphRange == lastPosition.fGlyphRange)) {
        auto next = selectableText->nextPosition(position);
        REPORTER_ASSERT(reporter, position.fTextRange.fEnd == next.fTextRange.fStart);
        if (position.fLineIndex == next.fLineIndex - 1) {
            auto line = selectableText->getLine(next.fLineIndex);
            REPORTER_ASSERT(reporter, next.fGlyphRange.fStart == 0);
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(next.fBoundaries.fLeft, 0.0f));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(next.fBoundaries.fTop, line.fBounds.fTop));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(next.fBoundaries.height(), line.fBounds.height()));
        } else {
            REPORTER_ASSERT(reporter, position.fLineIndex == next.fLineIndex);
            REPORTER_ASSERT(reporter, position.fGlyphRange.fEnd == next.fGlyphRange.fStart);
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(position.fBoundaries.fRight, next.fBoundaries.fLeft));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(position.fBoundaries.fTop, next.fBoundaries.fTop));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(position.fBoundaries.height(), next.fBoundaries.height()));
        }
        position = next;
    }
}

UNIX_ONLY_TEST(SkText_SelectableText_ScanLeftByGraphemeClusters, reporter) {
    sk_sp<TrivialFontChain> fontChain = sk_make_sp<TrivialFontChain>("Roboto", 40.0f, SkFontStyle::Normal());
    if (fontChain->empty()) return;

    std::u16string utf16(u"    Small Text   \n");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), fontChain);
    auto fontResolvedText = unicodeText.resolveFonts(SkSpan<FontBlock>(&fontBlock, 1));
    auto shapedText = fontResolvedText->shape(&unicodeText, TextDirection::kLtr);
    auto wrappedText = shapedText->wrap(&unicodeText, 440.0f, 500.0f);
    auto selectableText = wrappedText->prepareToEdit(&unicodeText);

    TestVisitor testVisitor;
    wrappedText->visit(&testVisitor);

    auto firstPosition = selectableText->firstPosition(PositionType::kGraphemeCluster);
    auto lastPosition = selectableText->lastPosition(PositionType::kGraphemeCluster);

    auto position = lastPosition;
    while (!(position.fGlyphRange == firstPosition.fGlyphRange)) {
        auto prev = selectableText->previousPosition(position);
        REPORTER_ASSERT(reporter, position.fTextRange.fEnd == prev.fTextRange.fStart);
        if (position.fLineIndex == prev.fLineIndex + 1) {
            auto line = selectableText->getLine(prev.fLineIndex);
            REPORTER_ASSERT(reporter, prev.fGlyphRange.fEnd == line.fBoxGlyphs.size());
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prev.fBoundaries.fRight, line.fBounds.fRight));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prev.fBoundaries.fTop, line.fBounds.fTop));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(prev.fBoundaries.height(), line.fBounds.height()));
        } else {
            REPORTER_ASSERT(reporter, position.fLineIndex == prev.fLineIndex);
            REPORTER_ASSERT(reporter, position.fGlyphRange.fStart == prev.fGlyphRange.fEnd);
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(position.fBoundaries.fLeft, prev.fBoundaries.fRight));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(position.fBoundaries.fTop, prev.fBoundaries.fTop));
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(position.fBoundaries.height(), prev.fBoundaries.height()));
        }
        position = prev;
    }
}

UNIX_ONLY_TEST(SkText_SelectableText_Navigation_UpDown, reporter) {
    sk_sp<TrivialFontChain> fontChain = sk_make_sp<TrivialFontChain>("Roboto", 40.0f, SkFontStyle::Normal());
    if (fontChain->empty()) return;

    std::u16string utf16(u"    Leading spaces\nTrailing spaces    \nLong text with collapsed      spaces inside wrapped into few lines");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    if (!unicodeText.getUnicode()) return;

    FontBlock fontBlock(utf16.size(), fontChain);
    auto fontResolvedText = unicodeText.resolveFonts(SkSpan<FontBlock>(&fontBlock, 1));
    auto shapedText = fontResolvedText->shape(&unicodeText, TextDirection::kLtr);
    auto wrappedText = shapedText->wrap(&unicodeText, 440.0f, 500.0f);
    auto selectableText = wrappedText->prepareToEdit(&unicodeText);

    TestVisitor testVisitor;
    wrappedText->visit(&testVisitor);

    // Upper position
    auto position = selectableText->lastInLinePosition(PositionType::kGraphemeCluster, 0);
    while (position.fLineIndex > 0) {
        auto down = selectableText->downPosition(position);
        REPORTER_ASSERT(reporter, position.fLineIndex + 1 == down.fLineIndex);
        REPORTER_ASSERT(reporter, position.fBoundaries.centerX() >= down.fBoundaries.centerX());
        position = down;
    }

    // Down position
    position = selectableText->lastInLinePosition(PositionType::kGraphemeCluster, selectableText->countLines() - 1);
    while (position.fLineIndex < selectableText->countLines() - 1) {
        auto down = selectableText->downPosition(position);
        REPORTER_ASSERT(reporter, position.fLineIndex - 1 == down.fLineIndex);
        REPORTER_ASSERT(reporter, position.fBoundaries.centerX() >= down.fBoundaries.centerX());
        position = down;
    }
}

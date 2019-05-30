/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ParagraphImpl_DEFINED
#define ParagraphImpl_DEFINED

#include "TextLine.h"
#include "Run.h"
#include "include/core/SkPicture.h"
#include "include/private//SkTHash.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"

class SkCanvas;

namespace skia {
namespace textlayout {

template <typename T> inline bool operator==(const SkSpan<T>& a, const SkSpan<T>& b) {
    return a.size() == b.size() && a.begin() == b.begin();
}

template <typename T> inline bool operator<=(const SkSpan<T>& a, const SkSpan<T>& b) {
    return a.begin() >= b.begin() && a.end() <= b.end();
}

inline bool operator&&(const SkSpan<const char>& a, const SkSpan<const char>& b) {
    if (a.empty() || b.empty()) {
        return false;
    }
    return SkTMax(a.begin(), b.begin()) < SkTMin(a.end(), b.end());
}

class TextBreaker {
public:
    TextBreaker() : fPos(-1) {}

    bool initialize(SkSpan<const char> text, UBreakIteratorType type) {
        UErrorCode status = U_ZERO_ERROR;

        fSize = text.size();
        UText utf8UText = UTEXT_INITIALIZER;
        utext_openUTF8(&utf8UText, text.begin(), text.size(), &status);
        fAutoClose =
                std::unique_ptr<UText, SkFunctionWrapper<UText*, UText, utext_close>>(&utf8UText);
        if (U_FAILURE(status)) {
            SkDebugf("Could not create utf8UText: %s", u_errorName(status));
            return false;
        }
        fIterator = ubrk_open(type, "en", nullptr, 0, &status);
        if (U_FAILURE(status)) {
            SkDebugf("Could not create line break iterator: %s", u_errorName(status));
            SK_ABORT("");
        }

        ubrk_setUText(fIterator, &utf8UText, &status);
        if (U_FAILURE(status)) {
            SkDebugf("Could not setText on break iterator: %s", u_errorName(status));
            return false;
        }

        fPos = 0;
        return true;
    }

    size_t first() {
        fPos = ubrk_first(fIterator);
        return eof() ? fSize : fPos;
    }

    size_t next() {
        fPos = ubrk_next(fIterator);
        return eof() ? fSize : fPos;
    }

    int32_t status() { return ubrk_getRuleStatus(fIterator); }

    bool eof() { return fPos == icu::BreakIterator::DONE; }

    ~TextBreaker() = default;

private:
    std::unique_ptr<UText, SkFunctionWrapper<UText*, UText, utext_close>> fAutoClose;
    UBreakIterator* fIterator;
    int32_t fPos;
    size_t fSize;
};

class ParagraphImpl final : public Paragraph {
public:
    ParagraphImpl(const std::string& text,
                    ParagraphStyle style,
                    std::vector<Block>
                            blocks,
                    sk_sp<FontCollection>
                            fonts)
            : Paragraph(std::move(style), std::move(fonts))
            , fUtf8(text.data(), text.size())
            , fPicture(nullptr) {
        fTextStyles.reserve(blocks.size());
        for (auto& block : blocks) {
            fTextStyles.emplace_back(
                    SkSpan<const char>(fUtf8.begin() + block.fStart, block.fEnd - block.fStart),
                    block.fStyle);
        }
    }

    ParagraphImpl(const std::u16string& utf16text,
                    ParagraphStyle style,
                    std::vector<Block>
                            blocks,
                    sk_sp<FontCollection>
                            fonts)
            : Paragraph(std::move(style), std::move(fonts)), fPicture(nullptr) {
        icu::UnicodeString unicode((UChar*)utf16text.data(), SkToS32(utf16text.size()));
        std::string str;
        unicode.toUTF8String(str);
        fUtf8 = SkSpan<const char>(str.data(), str.size());

        fTextStyles.reserve(blocks.size());
        for (auto& block : blocks) {
            fTextStyles.emplace_back(
                    SkSpan<const char>(fUtf8.begin() + block.fStart, block.fEnd - block.fStart),
                    block.fStyle);
        }
    }

    ~ParagraphImpl() override;

    void layout(SkScalar width) override;
    void paint(SkCanvas* canvas, SkScalar x, SkScalar y) override;
    std::vector<TextBox> getRectsForRange(unsigned start,
                                          unsigned end,
                                          RectHeightStyle rectHeightStyle,
                                          RectWidthStyle rectWidthStyle) override;
    PositionWithAffinity getGlyphPositionAtCoordinate(SkScalar dx, SkScalar dy) override;
    SkRange<size_t> getWordBoundary(unsigned offset) override;
    bool didExceedMaxLines() override {
        return !fParagraphStyle.unlimited_lines() && fLines.size() > fParagraphStyle.getMaxLines();
    }

    size_t lineNumber() override { return fLines.size(); }

    TextLine& addLine(SkVector offset, SkVector advance, SkSpan<const char> text,
                    SkSpan<const Cluster> clusters, size_t start, size_t end,
                    LineMetrics sizes);

    inline SkSpan<const char> text() const { return fUtf8; }
    inline SkSpan<Run> runs() { return SkSpan<Run>(fRuns.data(), fRuns.size()); }
    inline SkSpan<TextBlock> styles() {
        return SkSpan<TextBlock>(fTextStyles.data(), fTextStyles.size());
    }
    inline SkSpan<TextLine> lines() { return SkSpan<TextLine>(fLines.data(), fLines.size()); }
    inline ParagraphStyle paragraphStyle() const { return fParagraphStyle; }
    void formatLines(SkScalar maxWidth);

    inline bool strutEnabled() const { return paragraphStyle().getStrutStyle().fStrutEnabled; }
    inline bool strutForceHeight() const {
        return paragraphStyle().getStrutStyle().fForceStrutHeight;
    }
    inline LineMetrics strutMetrics() const { return fStrutMetrics; }

private:
    friend class ParagraphBuilder;

    void resetContext();
    void resolveStrut();
    void buildClusterTable();
    void shapeTextIntoEndlessLine();
    void breakShapedTextIntoLines(SkScalar maxWidth);
    void paintLinesIntoPicture();

    SkSpan<const TextBlock> findAllBlocks(SkSpan<const char> text);

    // Input
    SkTArray<TextBlock, true> fTextStyles;
    SkSpan<const char> fUtf8;

    // Internal structures
    SkTArray<Run> fRuns;
    SkTArray<Cluster, true> fClusters;
    SkTArray<TextLine> fLines;
    LineMetrics fStrutMetrics;

    // Painting
    sk_sp<SkPicture> fPicture;
};
}  // namespace textlayout
}  // namespace skia

#endif  // ParagraphImpl_DEFINED
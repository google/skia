// Copyright 2021 Google LLC.
#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/src/LogicalRun.h"
#include "experimental/sktext/src/VisualRun.h"
#include <memory>
#include <stack>
namespace skia {
namespace text {
UnicodeText::UnicodeText(std::unique_ptr<SkUnicode> unicode, SkSpan<uint16_t> utf16)
    : fText16(std::u16string((char16_t*)utf16.data(), utf16.size()))
    , fUnicode(std::move(unicode)) {
    initialize(utf16);
}

UnicodeText::UnicodeText(std::unique_ptr<SkUnicode> unicode, const SkString& utf8)
    : fUnicode(std::move(unicode)) {
    fText16 = fUnicode->convertUtf8ToUtf16(utf8);
    initialize(SkSpan<uint16_t>((uint16_t*)fText16.data(), fText16.size()));
}

bool UnicodeText::isWhitespaces(TextRange range) const {
    if (range.leftToRight()) {
        for (auto i = range.fStart; i < range.fEnd; ++i) {
            if (!this->hasProperty(i, CodeUnitFlags::kPartOfWhiteSpace)) {
                return false;
            }
        }
    } else {
        for (auto i = range.fStart; i > range.fEnd; --i) {
            if (!this->hasProperty(i, CodeUnitFlags::kPartOfWhiteSpace)) {
                return false;
            }
        }
    }
    return true;
}

void UnicodeText::initialize(SkSpan<uint16_t> utf16) {
    if (!fUnicode) {
        SkASSERT(fUnicode);
        return;
    }
    // Get white spaces
    fCodeUnitProperties.push_back_n(utf16.size() + 1, CodeUnitFlags::kNoCodeUnitFlag);
    this->fUnicode->forEachCodepoint((char16_t*)utf16.data(), utf16.size(),
       [this](SkUnichar unichar, int32_t start, int32_t end) {
            if (this->fUnicode->isWhitespace(unichar)) {
                for (auto i = start; i < end; ++i) {
                    fCodeUnitProperties[i] |=  CodeUnitFlags::kPartOfWhiteSpace;
                }
            }
       });
    // Get graphemes
    this->fUnicode->forEachBreak((char16_t*)utf16.data(), utf16.size(), SkUnicode::BreakType::kGraphemes,
                           [this](SkBreakIterator::Position pos, SkBreakIterator::Status) {
                                fCodeUnitProperties[pos]|= CodeUnitFlags::kGraphemeStart;
                            });
    // Get line breaks
    this->fUnicode->forEachBreak((char16_t*)utf16.data(), utf16.size(), SkUnicode::BreakType::kLines,
                           [this](SkBreakIterator::Position pos, SkBreakIterator::Status status) {
                                if (status == (SkBreakIterator::Status)SkUnicode::LineBreakType::kHardLineBreak) {
                                    // Hard line breaks clears off all the other flags
                                    // TODO: Treat \n as a formatting mark and do not pass it to SkShaper
                                    fCodeUnitProperties[pos - 1] = CodeUnitFlags::kHardLineBreakBefore;
                                } else {
                                    fCodeUnitProperties[pos] |= CodeUnitFlags::kSoftLineBreakBefore;
                                }
                            });
}

std::unique_ptr<FontResolvedText> UnicodeText::resolveFonts(SkSpan<FontBlock> blocks) {

    auto fontResolvedText = std::make_unique<FontResolvedText>();

    TextRange adjustedBlock(0, 0);
    TextIndex index = 0;
    for (auto& block : blocks) {

        index += block.charCount;
        adjustedBlock.fStart = adjustedBlock.fEnd;
        adjustedBlock.fEnd = index;
        if (adjustedBlock.fStart >= adjustedBlock.fEnd) {
            // The last block adjustment went over the entire block
            continue;
        }

        // Move the end of the block to the right until it's on the grapheme edge
        while (adjustedBlock.fEnd < this->fText16.size() &&  !this->hasProperty(adjustedBlock.fEnd, CodeUnitFlags::kGraphemeStart)) {
            ++adjustedBlock.fEnd;
        }
        SkASSERT(block.type == BlockType::kFontChain);
        fontResolvedText->resolveChain(this, adjustedBlock, *block.chain);
    }

    std::sort(fontResolvedText->fResolvedFonts.begin(), fontResolvedText->fResolvedFonts.end(),
              [](const ResolvedFontBlock& a, const ResolvedFontBlock& b) {
                return a.textRange.fStart < b.textRange.fStart;
              });
/*
    SkDebugf("Resolved:\n");
    for (auto& f : fontResolvedText->fResolvedFonts) {
        SkDebugf("[%d:%d)\n", f.textRange.fStart, f.textRange.fEnd);
    }
*/
    return std::move(fontResolvedText);
}

bool FontResolvedText::resolveChain(UnicodeText* unicodeText, TextRange textRange, const FontChain& fontChain) {

    std::deque<TextRange> unresolvedTexts;
    unresolvedTexts.push_back(textRange);
    for (auto fontIndex = 0; fontIndex < fontChain.count(); ++fontIndex) {
        auto typeface = fontChain[fontIndex];

        std::deque<TextRange> newUnresolvedTexts;
        // Check all text range that have not been resolved yet
        while (!unresolvedTexts.empty()) {
            // Take the first unresolved
            auto unresolvedText = unresolvedTexts.front();
            unresolvedTexts.pop_front();

            // Resolve font for the entire grapheme
            auto start = newUnresolvedTexts.size();
            unicodeText->forEachGrapheme(unresolvedText, [&](TextRange grapheme) {
                auto count = typeface->textToGlyphs(unicodeText->getText16().data() + grapheme.fStart, grapheme.width() * 2, SkTextEncoding::kUTF16, nullptr, 0);
                SkAutoTArray<SkGlyphID> glyphs(count);
                typeface->textToGlyphs(unicodeText->getText16().data() + grapheme.fStart, grapheme.width() * 2, SkTextEncoding::kUTF16, glyphs.data(), count);
                for (auto i = 0; i < count; ++i) {
                    if (glyphs[i] == 0) {
                        if (newUnresolvedTexts.empty() || newUnresolvedTexts.back().fEnd < grapheme.fStart) {
                            // It's a new unresolved block
                            newUnresolvedTexts.push_back(grapheme);
                        } else {
                            // Let's extend the last unresolved block
                            newUnresolvedTexts.back().fEnd = grapheme.fEnd;
                        }
                        break;
                    }
                }
            });
            // Let's fill the resolved blocks with the current font
            TextRange resolvedText(unresolvedText.fStart, unresolvedText.fStart);
            for (auto newUnresolvedText : newUnresolvedTexts) {
                if (start > 0) {
                    --start;
                    continue;
                }
                resolvedText.fEnd = newUnresolvedText.fStart;
                if (resolvedText.width() > 0) {
                    // Add another resolved block
                    fResolvedFonts.emplace_back(resolvedText, typeface, fontChain.fontSize(), SkFontStyle::Normal());
                }
                resolvedText.fStart = newUnresolvedText.fEnd;
            }
            resolvedText.fEnd = unresolvedText.fEnd;
            if (resolvedText.width() > 0) {
                // Add the last resolved block
                fResolvedFonts.emplace_back(resolvedText, typeface, fontChain.fontSize(), SkFontStyle::Normal());
            }
        }

        // Try the next font in chain
        SkASSERT(unresolvedTexts.empty());
        unresolvedTexts = std::move(newUnresolvedTexts);
    }

    return unresolvedTexts.empty();
}

// Font iterator that finds all formatting marks
// and breaks runs on them (so we can select and interpret them later)
class FormattingFontIterator final : public SkShaper::FontRunIterator {
public:
    FormattingFontIterator(TextIndex textCount,
                           SkSpan<ResolvedFontBlock> fontBlocks,
                           SkSpan<TextIndex> marks)
            : fTextCount(textCount)
            , fFontBlocks(fontBlocks)
            , fFormattingMarks(marks)
            , fCurrentBlock(fontBlocks.begin())
            , fCurrentMark(marks.begin())
            , fCurrentFontIndex(fCurrentBlock->textRange.fEnd) {
        fCurrentFont = this->createFont(*fCurrentBlock);
    }
    void consume() override {
        SkASSERT(fCurrentBlock < fFontBlocks.end());
        SkASSERT(fCurrentMark < fFormattingMarks.end());
        if (fCurrentFontIndex > *fCurrentMark) {
            ++fCurrentMark;
            return;
        }
        if (fCurrentFontIndex == *fCurrentMark) {
            ++fCurrentMark;
        }
        ++fCurrentBlock;
        if (fCurrentBlock < fFontBlocks.end()) {
            fCurrentFontIndex = fCurrentBlock->textRange.fEnd;
            fCurrentFont = this->createFont(*fCurrentBlock);
        }
    }
    size_t endOfCurrentRun() const override {
        SkASSERT(fCurrentMark != fFormattingMarks.end() || fCurrentBlock != fFontBlocks.end());
        if (fCurrentMark == fFormattingMarks.end()) {
            return fCurrentFontIndex;
        } else if (fCurrentBlock == fFontBlocks.end()) {
            return *fCurrentMark;
        } else {
            return std::min(fCurrentFontIndex, *fCurrentMark);
        }
    }
    bool atEnd() const override {
        return (fCurrentBlock == fFontBlocks.end() || fCurrentFontIndex == fTextCount) &&
               (fCurrentMark == fFormattingMarks.end() || *fCurrentMark == fTextCount);
    }
    const SkFont& currentFont() const override { return fCurrentFont; }
    SkFont createFont(const ResolvedFontBlock& resolvedFont) const {
        SkFont font(resolvedFont.typeface, resolvedFont.size);
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setHinting(SkFontHinting::kSlight);
        font.setSubpixel(true);
        return font;
    }
private:
    TextIndex const fTextCount;
    SkSpan<ResolvedFontBlock> fFontBlocks;
    SkSpan<TextIndex> fFormattingMarks;
    ResolvedFontBlock* fCurrentBlock;
    TextIndex* fCurrentMark;
    TextIndex fCurrentFontIndex;
    SkFont fCurrentFont;
};

std::unique_ptr<ShapedText> FontResolvedText::shape(UnicodeText* unicodeText,
                                                    TextDirection textDirection) {
    // Get utf8 <-> utf16 conversion tables.
    // We need to pass to SkShaper indices in utf8 and then convert them back to utf16 for SkText
    auto text16 = unicodeText->getText16();
    auto text8 = SkUnicode::convertUtf16ToUtf8(std::u16string(text16.data(), text16.size()));
    size_t utf16Index = 0;
    SkTArray<size_t, true> UTF16FromUTF8;
    SkTArray<size_t, true> UTF8FromUTF16;
    UTF16FromUTF8.push_back_n(text8.size() + 1, utf16Index);
    UTF8FromUTF16.push_back_n(text16.size() + 1, utf16Index);
    unicodeText->getUnicode()->forEachCodepoint(text8.c_str(), text8.size(),
    [&](SkUnichar unichar, int32_t start, int32_t end, int32_t count) {
        // utf8 index group of 1, 2 or 3 can be represented with one utf16 index group
        for (auto i = start; i < end; ++i) {
            UTF16FromUTF8[i] = utf16Index;
        }
        // utf16 index group of 1 or 2  can refer to the same group of utf8 indices
        for (; count != 0; --count) {
            UTF8FromUTF16[utf16Index++] = start;
        }
    });
    UTF16FromUTF8[text8.size()] = text16.size();
    UTF8FromUTF16[text16.size()] = text8.size();
    // Break text into pieces by font blocks and by formatting marks
    // Formatting marks: \n (and possibly some other later)
    std::vector<size_t> formattingMarks;
    for (size_t i = 0; i < text16.size(); ++i) {
        if (unicodeText->isHardLineBreak(i)) {
            formattingMarks.emplace_back(UTF8FromUTF16[i]);
            formattingMarks.emplace_back(UTF8FromUTF16[i + 1]);
            ++i;
        }
    }
    formattingMarks.emplace_back(text8.size()/* UTF8FromUTF16[text16.size() */);
    // Convert fontBlocks from utf16 to utf8
    SkTArray<ResolvedFontBlock, true> fontBlocks8;
    TextIndex index8 = 0;
    for (auto& fb : fResolvedFonts) {
        TextRange text8(UTF8FromUTF16[fb.textRange.fStart], UTF8FromUTF16[fb.textRange.fEnd]);
        fontBlocks8.emplace_back(text8, fb.typeface, fb.size, fb.style);
    }
    auto shapedText = std::make_unique<ShapedText>();
    // Shape the text
    FormattingFontIterator fontIter(text8.size(),
                                    SkSpan<ResolvedFontBlock>(fontBlocks8.data(), fontBlocks8.size()),
                                    SkSpan<TextIndex>(&formattingMarks[0], formattingMarks.size()));
    SkShaper::TrivialLanguageRunIterator langIter(text8.c_str(), text8.size());
    std::unique_ptr<SkShaper::BiDiRunIterator> bidiIter(
        SkShaper::MakeSkUnicodeBidiRunIterator(
            unicodeText->getUnicode(), text8.c_str(), text8.size(), textDirection == TextDirection::kLtr ? 0 : 1));
    std::unique_ptr<SkShaper::ScriptRunIterator> scriptIter(
        SkShaper::MakeSkUnicodeHbScriptRunIterator(unicodeText->getUnicode(), text8.c_str(), text8.size()));
    auto shaper = SkShaper::MakeShapeDontWrapOrReorder();
    if (shaper == nullptr) {
        // For instance, loadICU does not work. We have to stop the process
        return nullptr;
    }
    shaper->shape(
            text8.c_str(), text8.size(),
            fontIter, *bidiIter, *scriptIter, langIter,
            std::numeric_limits<SkScalar>::max(), shapedText.get());
    if (shapedText->fLogicalRuns.empty()) {
        // Create a fake run for an empty text (to avoid all the checks)
        SkShaper::RunHandler::RunInfo emptyInfo {
            fontIter.createFont(fResolvedFonts.front()),
            0,
            SkVector::Make(0.0f, 0.0f),
            0,
            SkShaper::RunHandler::Range(0, 0)
        };
        shapedText->fLogicalRuns.emplace_back(emptyInfo, 0, 0.0f);
        shapedText->fLogicalRuns.front().commit();
    }
    // Fill out all code unit properties
    for (auto& logicalRun : shapedText->fLogicalRuns) {
        // Convert utf8 range into utf16 range
        logicalRun.convertUtf16Range([&](unsigned long index8) {
            return UTF16FromUTF8[index8];
        });
        // Convert all utf8 indexes into utf16 indexes (and also shift them to be on the entire text scale, too)
        logicalRun.convertClusterIndexes([&](TextIndex clusterIndex8) {
            return UTF16FromUTF8[clusterIndex8];
        });
        // Detect and mark line break runs
        if (logicalRun.getTextRange().width() == 1 &&
            logicalRun.size() == 1 &&
            unicodeText->isHardLineBreak(logicalRun.getTextRange().fStart)) {
            logicalRun.setRunType(LogicalRunType::kLineBreak);
        }
    }
    return std::move(shapedText);
}

// TODO: Implement the vertical restriction (height) and add ellipsis
std::unique_ptr<WrappedText> ShapedText::wrap(UnicodeText* unicodeText, float width, float height) {
    auto wrappedText = std::unique_ptr<WrappedText>(new WrappedText());
    // line + spaces + clusters
    Stretch line;
    Stretch spaces;
    Stretch clusters;
    Stretch cluster;
    for (size_t runIndex = 0; runIndex < this->fLogicalRuns.size(); ++runIndex ) {
        auto& run = this->fLogicalRuns[runIndex];
        if (run.getRunType() == LogicalRunType::kLineBreak) {
            // This is the end of the word, the end of the line
            if (!clusters.isEmpty()) {
                line.moveTo(spaces);
                line.moveTo(clusters);
                spaces = clusters;
            }
            this->addLine(wrappedText.get(), unicodeText->getUnicode(), line, spaces, true);
            line = spaces;
            clusters = spaces;
            cluster = Stretch();
            continue;
        }
        TextMetrics runMetrics(run.fFont);
        if (!run.leftToRight()) {
            cluster.setTextRange({ run.fUtf16Range.fStart, run.fUtf16Range.fEnd});
        }
        // Let's wrap the text
        for (size_t glyphIndex = 0; glyphIndex < run.fPositions.size(); ++glyphIndex) {
            auto textIndex = run.fClusters[glyphIndex];
            if (cluster.textRange() == EMPTY_RANGE) {
                // The beginning of a new line (or the first one)
                cluster = Stretch(GlyphPos(runIndex, glyphIndex), textIndex, runMetrics);
                line = cluster;
                spaces = cluster;
                clusters = cluster;
                continue;
            }
            // The entire cluster belongs to a single run
            SkASSERT(cluster.glyphStart().runIndex() == runIndex);
            auto clusterWidth = run.calculateWidth(cluster.glyphStartIndex(), glyphIndex);
            cluster.finish(glyphIndex, textIndex, clusterWidth);
            auto isSoftLineBreak = unicodeText->isSoftLineBreak(cluster.textStart());
            auto isWhitespaces = unicodeText->isWhitespaces(cluster.textRange());
            auto isEndOfText = run.leftToRight() ? textIndex == run.fUtf16Range.fEnd : textIndex == run.fUtf16Range.fStart;
            // line + spaces + clusters + cluster
            if (isWhitespaces) {
                // This is the end of the word
                if (!clusters.isEmpty()) {
                    line.moveTo(spaces);
                    line.moveTo(clusters);
                    spaces = clusters;
                }
                spaces.moveTo(cluster);
                clusters = cluster;
                // Whitespaces do not extend the line width so no wrapping
                continue;
            } else if (!SkScalarIsFinite(width)) {
                // No wrapping - the endless line
                clusters.moveTo(cluster);
                continue;
            }
            // Now let's find out if we can add the cluster to the line
            auto currentWidth = line.width() + spaces.width() + clusters.width() + cluster.width();
            if (currentWidth > width) {
                // Finally, the wrapping case
                if (line.isEmpty()) {
                    if (spaces.isEmpty() && clusters.isEmpty()) {
                        // There is only this cluster and it's too long; we are drawing it anyway
                        line.moveTo(cluster);
                    } else {
                        // We break the only one word on the line by this cluster
                        line.moveTo(clusters);
                    }
                } else {
                  // We move clusters + cluster on the next line
                  // TODO: Parametrise possible ways of breaking too long word
                  //  (start it from a new line or squeeze the part of it on this line)
                }
                this->addLine(wrappedText.get(), unicodeText->getUnicode(), line, spaces, false);
                line = spaces;
            }
            clusters.moveTo(cluster);
            cluster = Stretch(GlyphPos(runIndex, glyphIndex), textIndex, runMetrics);
        }
    }
    // Deal with the last line
    if (!clusters.isEmpty()) {
        line.moveTo(spaces);
        line.moveTo(clusters);
        spaces = clusters;
    } else if (wrappedText->fVisualLines.empty()) {
        // Empty text; we still need a line to avoid checking for empty lines every time
        line.moveTo(cluster);
        spaces.moveTo(cluster);
    }
    this->addLine(wrappedText.get(), unicodeText->getUnicode(), line, spaces, false);
    wrappedText->fActualSize.fWidth = width;
    return std::move(wrappedText);
}

SkTArray<int32_t> ShapedText::getVisualOrder(SkUnicode* unicode, RunIndex startRun, RunIndex endRun) {
    auto numRuns = endRun - startRun + 1;
    SkTArray<int32_t> results;
    results.push_back_n(numRuns);
    if (numRuns == 0) {
        return results;
    }
    SkTArray<SkUnicode::BidiLevel> runLevels;
    runLevels.push_back_n(numRuns);
    size_t runLevelsIndex = 0;
    for (RunIndex runIndex = startRun; runIndex <= endRun; ++runIndex) {
        runLevels[runLevelsIndex++] = fLogicalRuns[runIndex].bidiLevel();
    }
    SkASSERT(runLevelsIndex == numRuns);
    unicode->reorderVisual(runLevels.data(), numRuns, results.data());
    return results;
}

// TODO: Fill line fOffset.fY
void ShapedText::addLine(WrappedText* wrappedText, SkUnicode* unicode, Stretch& stretch, Stretch& spaces, bool hardLineBreak) {
    auto spacesStart = spaces.glyphStart();
    Stretch lineStretch = stretch;
    lineStretch.moveTo(spaces);
    auto startRun = lineStretch.glyphStart().runIndex();
    auto endRun = lineStretch.glyphEnd().runIndex();
    // Reorder and cut (if needed) runs so they fit the line
    auto visualOrder = std::move(this->getVisualOrder(unicode, startRun, endRun));
    // Walk through the line's runs in visual order
    auto firstRunIndex = startRun;
    auto runStart = wrappedText->fVisualRuns.size();
    SkScalar runOffsetInLine = 0.0f;
    for (auto visualIndex : visualOrder) {
        auto& logicalRun = fLogicalRuns[firstRunIndex + visualIndex];
        if (logicalRun.getRunType() == LogicalRunType::kLineBreak) {
            SkASSERT(false);
        }
        bool isFirstRun = startRun == (firstRunIndex + visualIndex);
        bool isLastRun = endRun == (firstRunIndex + visualIndex);
        bool isSpaceRun = spacesStart.runIndex() == (firstRunIndex + visualIndex);
        auto glyphStart = isFirstRun ? lineStretch.glyphStart().glyphIndex() : 0;
        auto glyphEnd = isLastRun ? lineStretch.glyphEnd().glyphIndex() : logicalRun.size();
        auto glyphSize = glyphEnd - glyphStart;
        auto glyphSpaces = isSpaceRun ? spacesStart.glyphIndex() : glyphEnd;
        if (glyphSpaces > glyphStart) {
            auto textStart = isFirstRun ? lineStretch.textRange().fStart : logicalRun.fUtf16Range.fStart;
            auto textEnd = isLastRun ? lineStretch.textRange().fEnd : logicalRun.fUtf16Range.fEnd;
            wrappedText->fVisualRuns.emplace_back(TextRange(textStart, textEnd),
                                                  glyphSpaces - glyphStart,
                                                  logicalRun.fTextMetrics,
                                                  runOffsetInLine,
                                                  SkSpan<SkPoint>(&logicalRun.fPositions[glyphStart], glyphSize + 1),
                                                  SkSpan<SkGlyphID>(&logicalRun.fGlyphs[glyphStart], glyphSize),
                                                  SkSpan<uint32_t>((uint32_t*)&logicalRun.fClusters[glyphStart], glyphSize + 1));
        }
        runOffsetInLine += logicalRun.calculateWidth(glyphStart, glyphEnd);
    }
    auto runRange = wrappedText->fVisualRuns.size() == runStart
                    ? SkSpan<VisualRun>(nullptr, 0)
                    : SkSpan<VisualRun>(&wrappedText->fVisualRuns[runStart], wrappedText->fVisualRuns.size() - runStart);
    wrappedText->fVisualLines.emplace_back(lineStretch.textRange(), hardLineBreak, wrappedText->fActualSize.fHeight, runRange);
    wrappedText->fActualSize.fHeight += lineStretch.textMetrics().height();
    wrappedText->fActualSize.fWidth = lineStretch.width();
    stretch.clean();
    spaces.clean();
}

void WrappedText::format(TextAlign textAlign, TextDirection textDirection) {
    if (fAligned == textAlign) {
        return;
    }
    SkScalar verticalOffset = 0.0f;
    for (auto& line : this->fVisualLines) {
        if (textAlign == TextAlign::kLeft) {
            // Good by default
        } else if (textAlign == TextAlign::kCenter) {
            line.fOffset.fX = (this->fActualSize.width() - line.fActualWidth) / 2.0f;
        } else {
            // TODO: Implement all formatting features
        }
        line.fOffset.fY = verticalOffset;
        verticalOffset += line.fTextMetrics.height();
    }
}

void WrappedText::visit(Visitor* visitor) const {
    size_t lineIndex = 0;
    SkScalar verticalOffset = 0.0f;
    for (auto& line : fVisualLines) {
        visitor->onBeginLine(lineIndex, line.text(), line.isHardBreak(), SkRect::MakeXYWH(0, verticalOffset, line.fActualWidth, line.fTextMetrics.height()));
        // Select the runs that are on the line
        size_t glyphCount = 0ul;
        for (auto& run : line.fRuns) {
            auto diff = line.fTextMetrics.above() - run.fTextMetrics.above();
            SkRect boundingRect = SkRect::MakeXYWH(line.fOffset.fX + run.fPositions[0].fX, line.fOffset.fY + diff, run.width(), run.fTextMetrics.height());
            visitor->onGlyphRun(run.fFont, run.textRange(), boundingRect, run.trailingSpacesStart(),
                                run.size(), run.fGlyphs.data(), run.fPositions.data(), run.fClusters.data());
            glyphCount += run.size();
        }
        visitor->onEndLine(lineIndex, line.text(), line.trailingSpaces(), glyphCount);
        verticalOffset += line.fTextMetrics.height();
        ++lineIndex;
    }
}

void WrappedText::visit(UnicodeText* unicodeText, Visitor* visitor, PositionType positionType, SkSpan<TextIndex> textChunks) const {
    // Decor blocks have to be sorted by text cannot intersect but can skip some parts of the text
    // (in which case we use default text style from paragraph style)
    // The edges of the decor blocks don't have to match glyph, grapheme or even unicode code point edges
    // It's out responsibility to adjust them to some reasonable values
    // [a:b) -> [c:d) where
    // c is closest GG cluster edge to a from the left and d is closest GG cluster edge to b from the left
    SkScalar verticalOffset = 0.0f;
    LineIndex lineIndex = 0;
    size_t glyphCount = 0ul;
    for (auto& line : fVisualLines) {
        visitor->onBeginLine(lineIndex, line.text(), line.isHardBreak(), SkRect::MakeXYWH(0, verticalOffset, line.fActualWidth, line.fTextMetrics.height()));
        RunIndex runIndex = 0;
        for (auto& run : fVisualRuns) {
            run.forEachTextChunkInGlyphRange(textChunks, [&](TextRange textRange) {
                // TODO: Translate text range into glyph range (with all the appropriate adjustments)
                GlyphRange glyphRange = this->textToGlyphs(unicodeText, positionType, runIndex, textRange);
                auto diff = line.fTextMetrics.above() - run.fTextMetrics.above();
                SkRect boundingRect = SkRect::MakeXYWH(line.fOffset.fX + run.fPositions[glyphRange.fStart].fX, line.fOffset.fY + diff, glyphRange.width(), run.fTextMetrics.height());
                visitor->onGlyphRun(run.fFont, textRange, boundingRect, run.trailingSpacesStart(),
                                    glyphRange.width(), &run.fGlyphs[glyphRange.fStart], &run.fPositions[glyphRange.fStart], &run.fClusters[glyphRange.fStart]);
            });
            ++runIndex;
            glyphCount += run.size();
        }
        visitor->onEndLine(lineIndex, line.text(), line.trailingSpaces(), glyphCount);
        verticalOffset += line.fTextMetrics.height();
        ++lineIndex;
    }
}

// TODO: Implement more effective search
GlyphRange WrappedText::textToGlyphs(UnicodeText* unicodeText, PositionType positionType, RunIndex runIndex, TextRange textRange) const {
    SkASSERT(runIndex < fVisualRuns.size());
    auto& run = fVisualRuns[runIndex];
    SkASSERT(run.fUtf16Range.contains(textRange));
    GlyphRange glyphRange(0, run.size());
    for (GlyphIndex glyph = 0; glyph < run.size(); ++glyph) {
        auto textIndex = run.fClusters[glyph];
        if (positionType == PositionType::kGraphemeCluster && unicodeText->hasProperty(textIndex, CodeUnitFlags::kGraphemeStart)) {
            if (textIndex <= textRange.fStart) {
                glyphRange.fStart = glyph;
            } else if (textIndex <= textRange.fEnd) {
                glyphRange.fEnd = glyph;
            } else {
                return glyphRange;
            }
        }
    }
    SkASSERT(false);
    return glyphRange;
}

std::unique_ptr<DrawableText> WrappedText::prepareToDraw(UnicodeText* unicodeText, PositionType positionType, SkSpan<TextIndex> blocks) const {
    auto drawableText = std::make_unique<DrawableText>();
    this->visit(unicodeText, drawableText.get(), positionType, blocks);
    return std::move(drawableText);
}

std::unique_ptr<SelectableText> WrappedText::prepareToEdit(UnicodeText* unicodeText) const {
    auto selectableText = std::make_unique<SelectableText>();
    this->visit(selectableText.get());
    selectableText->fGlyphUnitProperties.push_back_n(unicodeText->getText16().size() + 1, GlyphUnitFlags::kNoGlyphUnitFlag);
    for (auto index = 0; index < unicodeText->getText16().size(); ++index) {
        if (unicodeText->hasProperty(index, CodeUnitFlags::kHardLineBreakBefore)) {
            selectableText->fGlyphUnitProperties[index] = GlyphUnitFlags::kGraphemeClusterStart;
        }
    }
    for (const auto& run : fVisualRuns) {
        for (auto& cluster : run.fClusters) {
            if (unicodeText->hasProperty(cluster, CodeUnitFlags::kGraphemeStart)) {
                selectableText->fGlyphUnitProperties[cluster] = GlyphUnitFlags::kGraphemeClusterStart;
            }
        }
    }
    return selectableText;
}

void SelectableText::onBeginLine(size_t index, TextRange lineText, bool hardBreak, SkRect bounds) {
    SkASSERT(fBoxLines.size() == index);
    fBoxLines.emplace_back(index, lineText, hardBreak, bounds);
}

void SelectableText::onEndLine(size_t index, TextRange lineText, GlyphRange trailingSpaces, size_t glyphCount) {
    auto& line = fBoxLines.back();
    line.fTextEnd = trailingSpaces.fStart;
    line.fTrailingSpacesEnd = trailingSpaces.fEnd;
    SkASSERT(line.fTextByGlyph.size() == glyphCount);
    line.fBoxGlyphs.emplace_back(SkRect::MakeXYWH(line.fBounds.fRight, line.fBounds.fTop, 0.0f, line.fBounds.height()));
    if (line.fTextByGlyph.empty()) {
        // Let's create an empty fake box to avoid all the checks
        line.fTextByGlyph.emplace_back(lineText.fEnd);
    }
    line.fTextByGlyph.emplace_back(lineText.fEnd);
}

void SelectableText::onGlyphRun(const SkFont& font,
                                TextRange textRange,
                                SkRect boundingRect,
                                int trailingSpacesStart,
                                int glyphCount,
                                const uint16_t glyphs[],
                                const SkPoint positions[],
                                const TextIndex clusters[]) {
    auto& line = fBoxLines.back();
    auto start = line.fTextByGlyph.size();
    line.fBoxGlyphs.push_back_n(glyphCount);
    line.fTextByGlyph.push_back_n(glyphCount);
    for (auto i = 0; i < glyphCount; ++i) {
        auto pos = positions[i];
        auto pos1 = positions[i + 1];
        line.fBoxGlyphs[start + i] = SkRect::MakeXYWH(pos.fX, boundingRect.fTop + pos.fY, pos1.fX - pos.fX, boundingRect.height());
        line.fTextByGlyph[start + i] = clusters[i];
    }
}

// TODO: Do something (logN) that is not a linear search
Position SelectableText::findPosition(PositionType positionType, const BoxLine& line, SkScalar x) const {
    Position position(positionType);
    position.fGlyphRange = GlyphRange(0, line.fBoxGlyphs.size() - 1);
    position.fTextRange = line.fTextRange;
    position.fBoundaries.fTop = line.fBounds.fTop;
    position.fBoundaries.fBottom = line.fBounds.fBottom;
    // We look for the narrowest glyph range adjusted to positionType that contains the point.
    // So far we made sure that one unit of any positionType does not cross the run edges
    // Therefore it's going to be represented by a single text range only
    for (; position.fGlyphRange.fStart < position.fGlyphRange.fEnd; ++position.fGlyphRange.fStart) {
        auto glyphBox = line.fBoxGlyphs[position.fGlyphRange.fStart];
        if (glyphBox.fLeft > x) {
            break;
        }
        if (position.fPositionType == PositionType::kGraphemeCluster) {
            auto textIndex = line.fTextByGlyph[position.fGlyphRange.fStart];
            if (this->hasProperty(textIndex, GlyphUnitFlags::kGraphemeClusterStart)) {
                position.fTextRange.fStart = textIndex;
            }
        } else {
            // TODO: Implement
            SkASSERT(false);
        }
    }
    for (; position.fGlyphRange.fEnd > position.fGlyphRange.fStart ; --position.fGlyphRange.fEnd) {
        auto glyphBox = line.fBoxGlyphs[position.fGlyphRange.fStart];
        if (glyphBox.fRight <= x) {
            break;
        }
        if (position.fPositionType == PositionType::kGraphemeCluster) {
            auto textIndex = line.fTextByGlyph[position.fGlyphRange.fEnd];
            if (this->hasProperty(textIndex, GlyphUnitFlags::kGraphemeClusterStart)) {
                position.fTextRange.fEnd = textIndex;
                break;
            }
        } else {
            // TODO: Implement
            SkASSERT(false);
        }
    }
    position.fLineIndex = line.fIndex;
    position.fBoundaries.fLeft = line.fBoxGlyphs[position.fGlyphRange.fStart].fLeft;
    position.fBoundaries.fRight = line.fBoxGlyphs[position.fGlyphRange.fEnd].fRight;
    return position;
}

Position SelectableText::adjustedPosition(PositionType positionType, SkPoint xy) const {
    xy.fX = std::min(xy.fX, this->fActualSize.fWidth);
    xy.fY = std::min(xy.fY, this->fActualSize.fHeight);
    Position position(positionType);
    for (auto& line : fBoxLines) {
        if (line.fBounds.fTop > xy.fY) {
            // We are past the point vertically
            break;
        } else if (line.fBounds.fBottom <= xy.fY) {
            // We haven't reached the point vertically yet
            continue;
        }
        return this->findPosition(positionType, line, xy.fX);
    }
    return this->lastPosition(positionType);
}

Position SelectableText::previousPosition(Position current) const {
    const BoxLine* currentLine = &fBoxLines[current.fLineIndex];
    if (this->isFirstOnTheLine(current)) {
        // Go to the previous line
        if (current.fLineIndex == 0) {
            // We reached the end; there is nowhere to move
            current.fGlyphRange = GlyphRange(0, 0);
            return current;
        } else {
            current.fLineIndex -= 1;
            currentLine = &fBoxLines[current.fLineIndex];
            current.fGlyphRange.fStart = currentLine->fBoxGlyphs.size();
        }
    }
    auto position = this->findPosition(current.fPositionType, *currentLine, currentLine->fBoxGlyphs[current.fGlyphRange.fStart].centerX());
    if (current.fPositionType == PositionType::kGraphemeCluster) {
        // Either way we found us a grapheme cluster (just make sure of it)
        SkASSERT(this->hasProperty(current.fTextRange.fStart, GlyphUnitFlags::kGraphemeClusterStart));
    }
    return position;
}

Position SelectableText::nextPosition(Position current) const {
    const BoxLine* currentLine = &fBoxLines[current.fLineIndex];
    if (this->isLastOnTheLine(current)) {
        // Go to the next line
        if (current.fLineIndex == this->fBoxLines.size() - 1) {
            // We reached the end; there is nowhere to move
            current.fGlyphRange = GlyphRange(currentLine->fBoxGlyphs.size(), currentLine->fBoxGlyphs.size());
            return current;
        } else {
            current.fLineIndex += 1;
            currentLine = &fBoxLines[current.fLineIndex];
            current.fGlyphRange.fEnd = 0;
        }
    }
    auto position = this->findPosition(current.fPositionType, *currentLine, currentLine->fBoxGlyphs[current.fGlyphRange.fStart].centerX());
    if (current.fPositionType == PositionType::kGraphemeCluster) {
        // Either way we found us a grapheme cluster (just make sure of it)
        SkASSERT(this->hasProperty(current.fTextRange.fEnd, GlyphUnitFlags::kGraphemeClusterStart));
    }
    return position;
}

Position SelectableText::upPosition(Position current) const {

    if (current.fLineIndex == 0) {
        // We are on the first line; just move to the first position
        return this->firstPosition(current.fPositionType);
    }

    // Go to the previous line
    const BoxLine* currentLine = &fBoxLines[current.fLineIndex];
    auto position = this->findPosition(current.fPositionType, fBoxLines[current.fLineIndex - 1], currentLine->fBoxGlyphs[current.fGlyphRange.fStart].centerX());
    if (current.fPositionType == PositionType::kGraphemeCluster) {
        // Either way we found us a grapheme cluster (just make sure of it)
        SkASSERT(this->hasProperty(current.fTextRange.fEnd, GlyphUnitFlags::kGraphemeClusterStart));
    }
    return position;
}

Position SelectableText::downPosition(Position current) const {

    if (current.fLineIndex == this->countLines() - 1) {
        // We are on the last line; just move to the last position
        return this->lastPosition(current.fPositionType);
    }

    // Go to the next line
    const BoxLine* currentLine = &fBoxLines[current.fLineIndex];
    auto position = this->findPosition(current.fPositionType, fBoxLines[current.fLineIndex + 1], currentLine->fBoxGlyphs[current.fGlyphRange.fStart].centerX());
    if (current.fPositionType == PositionType::kGraphemeCluster) {
        // Either way we found us a grapheme cluster (just make sure of it)
        SkASSERT(this->hasProperty(current.fTextRange.fEnd, GlyphUnitFlags::kGraphemeClusterStart));
    }
    return position;
}

Position SelectableText::firstPosition(PositionType positionType) const {
    auto firstLine = fBoxLines.front();
    auto firstGlyph = firstLine.fBoxGlyphs.front();
    Position beginningOfText(positionType);
    // Set the glyph range after the last glyph
    beginningOfText.fGlyphRange = GlyphRange { 0, 0};
    beginningOfText.fLineIndex = 0;
    beginningOfText.fBoundaries = SkRect::MakeXYWH(firstGlyph.fLeft, firstGlyph.fTop, 0, firstGlyph.height());
    beginningOfText.fTextRange = this->glyphsToText(beginningOfText);
    beginningOfText.fLineIndex = 0;
    return beginningOfText;
}

Position SelectableText::lastPosition(PositionType positionType) const {
    auto lastLine = fBoxLines.back();
    auto lastGlyph = lastLine.fBoxGlyphs.back();
    Position endOfText(positionType);
    endOfText.fLineIndex = lastLine.fIndex;
    endOfText.fGlyphRange = GlyphRange(lastLine.fBoxGlyphs.size() - 1, lastLine.fBoxGlyphs.size() - 1);
    endOfText.fBoundaries = SkRect::MakeXYWH(lastGlyph.fRight, lastGlyph.fTop, 0, lastGlyph.height());
    endOfText.fTextRange = this->glyphsToText(endOfText);
    endOfText.fLineIndex = lastLine.fIndex;
    return endOfText;
}

Position SelectableText::firstInLinePosition(PositionType positionType, LineIndex lineIndex) const {
    SkASSERT(lineIndex >= 0 && lineIndex < fBoxLines.size());
    auto& line = fBoxLines[lineIndex];
    return this->findPosition(positionType, line, line.fBounds.left());
}

Position SelectableText::lastInLinePosition(PositionType positionType, LineIndex lineIndex) const {
    auto& line = fBoxLines[lineIndex];
    return this->findPosition(positionType, line, line.fBounds.right());
}


TextRange SelectableText::glyphsToText(Position position) const {
    SkASSERT(position.fPositionType != PositionType::kRandomText);
    auto line = this->getLine(position.fLineIndex);
    TextRange textRange = EMPTY_RANGE;
    for (auto glyph = position.fGlyphRange.fStart; glyph <= position.fGlyphRange.fEnd; ++glyph) {
        if (textRange.fStart == EMPTY_INDEX) {
            textRange.fStart = line.fTextByGlyph[glyph];
        }
        textRange.fEnd = line.fTextByGlyph[glyph];
    }
    return textRange;
}
} // namespace text
} // namespace skia

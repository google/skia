// Copyright 2021 Google LLC.

#include "experimental/sktext/include/Text.h"
#include <stack>

namespace skia {
namespace text {

std::unique_ptr<UnicodeText> Text::parse(SkSpan<uint16_t> utf16) {

    auto unicodeText = std::unique_ptr<UnicodeText>(new UnicodeText());
    unicodeText->fUnicode = std::move(SkUnicode::Make());
    if (nullptr == unicodeText->fUnicode) {
        return nullptr;
    }

    // Create utf8 -> utf16 conversion table
    unicodeText->fText8 = unicodeText->fUnicode->convertUtf16ToUtf8(std::u16string((char16_t*)utf16.data(), utf16.size()));
    size_t utf16Index = 0;
    unicodeText->fUTF16FromUTF8.push_back_n(unicodeText->fText8.size() + 1, utf16Index);

    // Fill out all code unit properties
    unicodeText->fCodeUnitProperties.push_back_n(utf16.size() + 1, CodeUnitFlags::kNoCodeUnitFlag);
    unicodeText->fUnicode->forEachCodepoint(unicodeText->fText8.c_str(), unicodeText->fText8.size(),
        [&unicodeText, &utf16Index](SkUnichar unichar, int32_t start, int32_t end) {
            for (auto i = start; i < end; ++i) {
                unicodeText->fUTF16FromUTF8[i] = utf16Index;
            }
            ++utf16Index;
       });
    unicodeText->fUTF16FromUTF8[unicodeText->fText8.size()] = utf16Index;

    // Get white spaces
    unicodeText->fUnicode->forEachCodepoint(unicodeText->fText8.c_str(), unicodeText->fText8.size(),
       [&unicodeText](SkUnichar unichar, int32_t start, int32_t end) {
            if (unicodeText->fUnicode->isWhitespace(unichar)) {
                for (auto i = start; i < end; ++i) {
                    unicodeText->fCodeUnitProperties[i] |=  CodeUnitFlags::kPartOfWhiteSpace;
                }
            }
       });

    // Get line breaks
    unicodeText->fUnicode->forEachBreak((char16_t*)utf16.data(), utf16.size(), SkUnicode::BreakType::kLines,
                           [&unicodeText](SkBreakIterator::Position pos, SkBreakIterator::Status status){
                                unicodeText->fCodeUnitProperties[pos] |= (status == (SkBreakIterator::Status)SkUnicode::LineBreakType::kHardLineBreak
                                                               ? CodeUnitFlags::kHardLineBreakBefore
                                                               : CodeUnitFlags::kSoftLineBreakBefore);
                            });

    // Get graphemes
    unicodeText->fUnicode->forEachBreak((char16_t*)utf16.data(), utf16.size(), SkUnicode::BreakType::kGraphemes,
                           [&unicodeText](SkBreakIterator::Position pos, SkBreakIterator::Status){
                                unicodeText->fCodeUnitProperties[pos]|= CodeUnitFlags::kGraphemeStart;
                            });

   return std::move(unicodeText);
}

std::unique_ptr<ShapedText> UnicodeText::shape(SkSpan<Block> blocks,
                                               TextDirection textDirection) {

    fShapedText = std::unique_ptr<ShapedText>(new ShapedText());

    for (auto& block : blocks) {
        SkFont font(this->createFont(block));
        SkShaper::TrivialFontRunIterator fontIter(font, fText8.size());
        SkShaper::TrivialLanguageRunIterator langIter(fText8.c_str(), fText8.size());
        std::unique_ptr<SkShaper::BiDiRunIterator> bidiIter(
            SkShaper::MakeSkUnicodeBidiRunIterator(
                this->fUnicode.get(), fText8.c_str(), fText8.size(), textDirection == TextDirection::kLtr ? 0 : 1));
        std::unique_ptr<SkShaper::ScriptRunIterator> scriptIter(
            SkShaper::MakeSkUnicodeHbScriptRunIterator(this->fUnicode.get(), fText8.c_str(), fText8.size()));
        auto shaper = SkShaper::MakeShapeDontWrapOrReorder();
        if (shaper == nullptr) {
            // For instance, loadICU does not work. We have to stop the process
            return nullptr;
        }

        shaper->shape(fText8.c_str(), fText8.size(),
                fontIter, *bidiIter, *scriptIter, langIter,
                std::numeric_limits<SkScalar>::max(), this);
    }

    fShapedText->fGlyphUnitProperties.push_back_n(this->fCodeUnitProperties.size(), GlyphUnitFlags::kNoGlyphUnitFlag);
    for (size_t i = 0; i < this->fCodeUnitProperties.size(); ++i) {
        fShapedText->fGlyphUnitProperties[i] = (GlyphUnitFlags)this->fCodeUnitProperties[i];
    }
    for (auto& run : fShapedText->fRuns) {
        for (auto index : run.fClusters) {
            if (fShapedText->hasProperty(index, GlyphUnitFlags::kGraphemeStart)) {
                fShapedText->fGlyphUnitProperties[index] |= GlyphUnitFlags::kGlyphClusterStart;
            }
        }
    }

    return std::move(fShapedText);
}

void UnicodeText::commitRunBuffer(const RunInfo&) {
    fCurrentRun->commit();

    // Convert all utf8 indexes into utf16 indexes
    for (size_t i = 0; i < fCurrentRun->fClusters.size(); ++i) {
        auto element = &fCurrentRun->fClusters[i];
        *element = this->fUTF16FromUTF8[*element];
    }

    fCurrentRun->fUtf16Range =
            TextRange(this->fUTF16FromUTF8[fCurrentRun->fUtf8Range.fBegin], this->fUTF16FromUTF8[fCurrentRun->fUtf8Range.end()]);
    fShapedText->fRuns.emplace_back(std::move(*fCurrentRun));
}

SkFont UnicodeText::createFont(const Block& block) {

    if (block.chain->count() == 0) {
        return SkFont();
    }
    sk_sp<SkTypeface> typeface = block.chain->operator[](0);

    SkFont font(std::move(typeface), block.chain->size());
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setHinting(SkFontHinting::kSlight);
    font.setSubpixel(true);

    return font;
}

std::unique_ptr<WrappedText> ShapedText::wrap(SkScalar width, SkScalar heightCurrentlyIgnored, SkUnicode* unicode) {

    auto wrappedText = std::unique_ptr<WrappedText>(new WrappedText());
    wrappedText->fRuns = this->fRuns;
    wrappedText->fGlyphUnitProperties = this->fGlyphUnitProperties; // copy

    // line : spaces : clusters
    Stretch line;
    Stretch spaces;
    Stretch clusters;

    for (size_t runIndex = 0; runIndex < this->fRuns.size(); ++runIndex ) {

        auto& run = this->fRuns[runIndex];
        TextMetrics runMetrics(run.fFont);
        Stretch cluster;
        if (!run.leftToRight()) {
            cluster.setTextRange({ run.fUtf16Range.fStart, run.fUtf16Range.fEnd});
        }

        for (size_t glyphIndex = 0; glyphIndex < run.fPositions.size(); ++glyphIndex) {
            auto textIndex = run.fClusters[glyphIndex];

            if (cluster.isEmpty()) {
                cluster = Stretch(GlyphPos(runIndex, glyphIndex), textIndex, runMetrics);
                continue;
          }

          // The entire cluster belongs to a single run
          SkASSERT(cluster.glyphStart().runIndex() == runIndex);

          auto clusterWidth = run.calculateWidth(cluster.glyphStartIndex(), glyphIndex);
          cluster.finish(glyphIndex, textIndex, clusterWidth);

          auto isHardLineBreak = this->isHardLineBreak(cluster.textStart());
          auto isSoftLineBreak = this->isSoftLineBreak(cluster.textStart());
          auto isWhitespaces = this->isWhitespaces(cluster.textRange());
          auto isEndOfText = run.leftToRight() ? textIndex == run.fUtf16Range.fEnd : textIndex == run.fUtf16Range.fStart;

          if (isWhitespaces || isHardLineBreak) {
              // This is the end of the word
              if (!clusters.isEmpty()) {
                  line.moveTo(spaces);
                  line.moveTo(clusters);
                  spaces = clusters;
              }
          }

          // line + spaces + clusters + cluster
          if (isWhitespaces) {
              // Whitespaces do not extend the line width
              spaces.moveTo(cluster);
              clusters = cluster;
              continue;
          } else if (isHardLineBreak) {
              // Hard line break ends the line but does not extend the width
              // Same goes for the end of the text
              wrappedText->addLine(line, spaces, unicode);
              break;
          } else if (!SkScalarIsFinite(width)) {
              clusters.moveTo(cluster);
              continue;
          }

          // Now let's find out if we can add the cluster to the line
          if ((line.width() + spaces.width() + clusters.width() + cluster.width()) <= width) {
              clusters.moveTo(cluster);
          } else {
              if (line.isEmpty()) {
                  if (spaces.isEmpty() && clusters.isEmpty()) {
                      // There is only this cluster and it's too long;
                      // we are drawing it anyway
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
              wrappedText->addLine(line, spaces, unicode);
              line = spaces;
              clusters.moveTo(cluster);
          }

          cluster = Stretch(GlyphPos(runIndex, glyphIndex), textIndex, runMetrics);
        }
    }

    if (!clusters.isEmpty()) {
        line.moveTo(spaces);
        line.moveTo(clusters);
        spaces = clusters;
    }
    wrappedText->addLine(line, spaces, unicode);

    wrappedText->fSize.fWidth = width;
    return std::move(wrappedText);
}

void WrappedText::addLine(Stretch& stretch, Stretch& spaces, SkUnicode* unicode) {

    // This is just chosen to catch the common/fast cases. Feel free to tweak.
    constexpr int kPreallocCount = 4;
    auto start = stretch.glyphStart().runIndex();
    auto end = spaces.glyphEnd().runIndex();
    auto numRuns = end - start + 1;
    SkAutoSTArray<kPreallocCount, SkUnicode::BidiLevel> runLevels(numRuns);
    size_t runLevelsIndex = 0;
    for (auto runIndex = start; runIndex <= end; ++runIndex) {
        auto& run = fRuns[runIndex];
        runLevels[runLevelsIndex++] = run.bidiLevel();
    }
    SkASSERT(runLevelsIndex == numRuns);
    SkAutoSTArray<kPreallocCount, int32_t> logicalOrder(numRuns);
    SkSTArray<1, size_t, true> visualOrder;
    unicode->reorderVisual(runLevels.data(), numRuns, logicalOrder.data());
    auto firstRunIndex = start;
    for (auto index : logicalOrder) {
        visualOrder.push_back(firstRunIndex + index);
    }
    this->fLines.emplace_back(stretch, spaces, std::move(visualOrder));
    fSize.fHeight += stretch.textMetrics().height();

    stretch.clean();
    spaces.clean();
}

sk_sp<FormattedText> WrappedText::format(TextAlign textAlign, TextDirection textDirection) {
    auto formattedText = sk_sp<FormattedText>(new FormattedText());

    formattedText->fRuns = this->fRuns;
    formattedText->fLines = this->fLines;
    formattedText->fGlyphUnitProperties = this->fGlyphUnitProperties;
    formattedText->fSize = this->fSize;

    return std::move(formattedText);
}

void FormattedText::visit(Visitor* visitor) const {

    SkPoint offset = SkPoint::Make(0 , 0);
    for (auto& line : this->fLines) {
        offset.fX = 0;
        visitor->onBeginLine(line.fText, line.fTextMetrics.baseline());
        for (auto& runIndex : line.fRunsInVisualOrder) {
            auto& run = this->fRuns[runIndex];
            auto startGlyph = runIndex == line.fTextStart.runIndex() ? line.fTextStart.glyphIndex() : 0;
            auto endGlyph = runIndex == line.fTextEnd.runIndex() ? line.fTextEnd.glyphIndex() : run.fGlyphs.size();
            TextRange textRange(run.fClusters[startGlyph], run.fClusters[endGlyph]);
            auto count = endGlyph - startGlyph;
            SkScalar runWidth = run.calculateWidth(Range<GlyphIndex>(startGlyph, endGlyph));

            // Update positions
            SkAutoSTMalloc<256, SkPoint> positions(count + 1);
            SkPoint shift = SkPoint::Make(-run.fPositions[startGlyph].fX, line.fTextMetrics.baseline());
            for (size_t i = startGlyph; i <= endGlyph; ++i) {
                positions[i - startGlyph] = run.fPositions[i] + shift + offset;
            }
            offset.fX += runWidth;
            visitor->onGlyphRun(run.fFont, textRange, count, run.fGlyphs.data() + startGlyph, positions.data(), run.fOffsets.data() + startGlyph);
        }
        visitor->onEndLine(line.fText, line.fTextMetrics.baseline());
        offset.fY += line.fTextMetrics.height();
    }
}

void FormattedText::visit(Visitor* visitor, SkSpan<size_t> chunks) const {
    // Decor blocks have to be sorted by text cannot intersect but can skip some parts of the text
    // (in which case we use default text style from paragraph style)
    // The edges of the decor blocks don't have to match glyph, grapheme or even unicode code point edges
    // It's out responsibility to adjust them to some reasonable values
    // [a:b) -> [c:d) where
    // c is closest GG cluster edge to a from the left and d is closest GG cluster edge to b from the left

    size_t* currentBlock = &chunks[0];
    size_t currentStart = 0;
    SkPoint offset = SkPoint::Make(0 , 0);
    for (auto& line : this->fLines) {
        offset.fX = 0;
        visitor->onBeginLine(line.fText, line.fTextMetrics.baseline());
        for (auto& runIndex : line.fRunsInVisualOrder) {
            auto& run = this->fRuns[runIndex];
            // The run edges are good (aligned to GGC)
            // "ABCdef" -> "defCBA"
            // "AB": red
            // "Cd": green
            // "ef": blue
            // green[d] blue[ef] green [C] red [BA]
            auto startGlyph = runIndex == line.fTextStart.runIndex() ? line.fTextStart.glyphIndex() : 0;
            auto endGlyph = runIndex == line.fTextEnd.runIndex() ? line.fTextEnd.glyphIndex() : run.fGlyphs.size();

            TextRange textRange(run.fClusters[startGlyph], run.fClusters[endGlyph]);
            GlyphRange glyphRange(startGlyph, endGlyph);
            SkScalar runWidth = run.calculateWidth(glyphRange);
            size_t currentEnd = *currentBlock;
            for (auto glyphIndex = startGlyph; glyphIndex <= endGlyph; ++glyphIndex) {

                SkASSERT(currentBlock < chunks.end());
                auto textIndex = run.fClusters[glyphIndex];
                if (glyphIndex == endGlyph) {
                    // last piece of the text
                } else if (run.leftToRight() && textIndex < currentEnd) {
                    continue;
                } else if (!run.leftToRight() && textIndex >= currentStart) {
                    continue;
                }
                textRange.fEnd = textIndex;
                glyphRange.fEnd = glyphIndex;

                // Update positions
                SkAutoSTMalloc<256, SkPoint> positions(glyphRange.width() + 1);
                SkPoint shift = SkPoint::Make(-run.fPositions[startGlyph].fX, line.fTextMetrics.baseline());
                for (size_t i = glyphRange.fStart; i <= glyphRange.fEnd; ++i) {
                    positions[i - glyphRange.fStart] = run.fPositions[i] + shift + offset;
                }
                visitor->onGlyphRun(run.fFont, textRange, glyphRange.width(), run.fGlyphs.data() + startGlyph, positions.data(), run.fOffsets.data() + startGlyph);

                textRange.fStart = textIndex;
                glyphRange.fStart = glyphIndex;

                if (glyphIndex != endGlyph) {
                    // We are here because we reached the end of the block
                    ++currentBlock;
                }
            }
            offset.fX += runWidth;
        }
        visitor->onEndLine(line.fText, line.fTextMetrics.baseline());
        offset.fY += line.fTextMetrics.height();
    }
}

std::tuple<const Line*, const TextRun*, GlyphIndex, SkRect> FormattedText::indexToAdjustedGraphemePosition(TextIndex textIndex) const {

    SkRect rect = SkRect::MakeEmpty();
    for (auto& line : fLines) {
        rect.fTop = rect.fBottom;
        rect.fBottom += line.fTextMetrics.height();
        if (!line.fText.contains(textIndex)) {
            continue;
        }

        for (auto runIndex : line.fRunsInVisualOrder) {
            auto& run = fRuns[runIndex];
            GlyphIndex start = runIndex == line.fTextStart.runIndex() ? line.fTextStart.glyphIndex() : 0;
            GlyphIndex end = runIndex == line.fTextEnd.runIndex() ? line.fTextEnd.glyphIndex() : run.fGlyphs.size();
            TextRange textRange(run.fClusters[start], run.fClusters[end]);
            if (textRange.contains(textIndex)) {
                auto glyphIndex = run.findGlyph(textIndex);
                rect.fLeft += run.calculateWidth(start, glyphIndex);
                rect.fRight += run.calculateWidth(start, glyphIndex + 1);
                return std::make_tuple(&line, &run, glyphIndex, rect);
            }
            rect.fLeft += run.calculateWidth(GlyphRange(start, end));
            rect.fRight = rect.fLeft;
        }
        // IF we found the index in the line we should have found it in one of the runs on the line
        SkASSERT(false);
    }

    // We are right from the end of the text;
    const auto lastLine = &fLines.back();
    const auto lastRun = &fRuns[lastLine->fTextEnd.runIndex()];
    rect.fLeft =
    rect.fRight = lastLine->fTextWidth;
    return std::make_tuple(lastLine, lastRun, lastRun->fGlyphs.size(), rect);
}

TextIndex FormattedText::positionToAdjustedGraphemeIndex(SkPoint xy) const {
    if (xy.fX >= this->fSize.fWidth) {
        xy.fX = this->fSize.fWidth;
    }
    if (xy.fY >= this->fSize.fHeight) {
        xy.fY = this->fSize.fHeight;
    }

    SkScalar height = 0;
    for (auto& line : fLines) {
        if (height > xy.fY) {
            break;
        } else if (height + line.fTextMetrics.height() <= xy.fY) {
            height += line.fTextMetrics.height();
            continue;
        }

        SkScalar shift = 0;
        for (auto runIndex : line.fRunsInVisualOrder) {
            auto& run = fRuns[runIndex];
            GlyphIndex start = runIndex == line.fTextStart.runIndex() ? line.fTextStart.glyphIndex() : 0;
            GlyphIndex end = runIndex == line.fTextEnd.runIndex() ? line.fTextEnd.glyphIndex() : run.fGlyphs.size();
            auto runWidth = run.calculateWidth(GlyphRange(start, end));
            if (shift > xy.fX) {
                break;
            } else if (shift + runWidth < xy.fX) {
                shift += runWidth;
                continue;
            }
            SkScalar startPos = run.fPositions[start].fX;
            GlyphIndex found = start;
            for (auto i = start; i <= end; ++i) {
                auto currentPos = run.fPositions[i].fX - startPos;
                if (currentPos > xy.fX) {
                    break;
                }
                found = i;
            }
            return run.fClusters[found];
        }
        height += line.fTextMetrics.height();
    }
    return EMPTY_INDEX;
}

} // namespace text
} // namespace skia

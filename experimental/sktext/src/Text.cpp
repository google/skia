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
    unicodeText->fText16 = std::u16string((char16_t*)utf16.data(), utf16.size());
    unicodeText->fText8 = unicodeText->fUnicode->convertUtf16ToUtf8(unicodeText->fText16);
    size_t utf16Index = 0;
    unicodeText->fUTF16FromUTF8.push_back_n(unicodeText->fText8.size() + 1, utf16Index);
    unicodeText->fUTF8FromUTF16.push_back_n(unicodeText->fText16.size() + 1, utf16Index);

    // Fill out all code unit properties
    unicodeText->fCodeUnitProperties.push_back_n(utf16.size() + 1, CodeUnitFlags::kNoCodeUnitFlag);
    unicodeText->fUnicode->forEachCodepoint(unicodeText->fText8.c_str(), unicodeText->fText8.size(),
    [&unicodeText, &utf16Index](SkUnichar unichar, int32_t start, int32_t end) {
        for (auto i = start; i < end; ++i) {
            unicodeText->fUTF16FromUTF8[i] = utf16Index;
        }
        unicodeText->fUTF8FromUTF16[utf16Index] = start;
        ++utf16Index;
    });
    unicodeText->fUTF16FromUTF8[unicodeText->fText8.size()] = unicodeText->fText16.size();
    unicodeText->fUTF8FromUTF16[unicodeText->fText16.size()] = unicodeText->fText8.size();

        // Get white spaces
    // TODO: It's a bug. We need to operate on utf16 indexes everywhere
    unicodeText->fUnicode->forEachCodepoint(unicodeText->fText8.c_str(), unicodeText->fText8.size(),
       [&unicodeText](SkUnichar unichar, int32_t start, int32_t end) {
            if (unicodeText->fUnicode->isWhitespace(unichar)) {
                for (auto i = start; i < end; ++i) {
                    unicodeText->fCodeUnitProperties[i] |=  CodeUnitFlags::kPartOfWhiteSpace;
                }
            }
       });

    // Get graphemes
    unicodeText->fUnicode->forEachBreak((char16_t*)utf16.data(), utf16.size(), SkUnicode::BreakType::kGraphemes,
                           [&unicodeText](SkBreakIterator::Position pos, SkBreakIterator::Status){
                                unicodeText->fCodeUnitProperties[pos]|= CodeUnitFlags::kGraphemeStart;
                            });

    // Get line breaks
    unicodeText->fUnicode->forEachBreak((char16_t*)utf16.data(), utf16.size(), SkUnicode::BreakType::kLines,
                           [&unicodeText](SkBreakIterator::Position pos, SkBreakIterator::Status status) {
                                if (status == (SkBreakIterator::Status)SkUnicode::LineBreakType::kHardLineBreak) {
                                    // Hard line breaks clears off all the other flags
                                    // TODO: Treat \n as a formatting mark and do not pass it to SkShaper
                                    unicodeText->fCodeUnitProperties[pos - 1] = CodeUnitFlags::kHardLineBreakBefore;
                                } else {
                                    unicodeText->fCodeUnitProperties[pos] |= CodeUnitFlags::kSoftLineBreakBefore;
                                }
                            });

   return std::move(unicodeText);
}

// Break text into pieces by font blocks and by formatting marks
// Formatting marks: \n (and possibly some other later)
std::unique_ptr<ShapedText> UnicodeText::shape(SkSpan<FontBlock> fontBlocks,
                                               TextDirection textDirection) {
    // TODO: Deal with all the cases
    // A run can start
    // 1. From the beginning of the text
    // 2. From the beginning of the paragraph after newline
    // 3. From the beginning of the font block
    // 4. One shaping call can produce multiple runs (for which we should apply 1 - 3)
    fRunGlyphStart = 0.0f;
    fParagraphTextStart = 0;
    std::vector<TextIndex> formattingMarks;
    formattingMarks.emplace_back(fText8.size());

    // Copy flags and find all the formatting marks
    fShapedText = std::unique_ptr<ShapedText>(new ShapedText());
    fShapedText->fGlyphUnitProperties.push_back_n(this->fCodeUnitProperties.size(), GlyphUnitFlags::kNoGlyphUnitFlag);
    for (size_t i = 0; i < this->fCodeUnitProperties.size(); ++i) {
        fShapedText->fGlyphUnitProperties[i] = (GlyphUnitFlags)this->fCodeUnitProperties[i];
        if (this->isHardLineBreak(i)) {
            formattingMarks.emplace_back(i);
        }
    }

    // TODO: Deal with placeholders (have to be treated the same way to avoid all trouble with bidi)

    // Break the text into a list of paragraphs by \n
    // and shape each paragraph separately
    // We start each paragraph from a new line
    fRunGlyphStart = 0.0f;

    FormattingFontIterator fontIter(fText8.size(), fontBlocks, SkSpan<TextIndex>(&formattingMarks[0], 1));
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

    shaper->shape(
            fText8.c_str(), fText8.size(),
            fontIter, *bidiIter, *scriptIter, langIter,
            std::numeric_limits<SkScalar>::max(), this);

    // Create a fake run for an empty text (to avoid all the checks)
    if (fShapedText->fRuns.empty()) {
        // We still need one empty run to keep the metrics
        SkShaper::RunHandler::RunInfo emptyInfo {
            this->createFont(fontBlocks.front()),
            0,
            SkVector::Make(0.0f, 0.0f),
            0,
            Range(0, 0)
        };
        fShapedText->fRuns.emplace_back(emptyInfo, 0, 0.0f);
        fShapedText->fRuns.front().commit();
    }

    return std::move(fShapedText);
}

void UnicodeText::commitRunBuffer(const RunInfo&) {
    fCurrentRun->commit();

    // Convert utf8 range into utf16 range
    fCurrentRun->convertUtf16Range([this](unsigned long index) {
        return this->fUTF16FromUTF8[index + fParagraphTextStart];
    });

    // Convert all utf8 indexes into utf16 indexes (and also shift them to be on the entire text scale, too)
    fCurrentRun->convertClusterIndexes([this](TextIndex clusterIndex) {
        auto converted = this->fUTF16FromUTF8[clusterIndex + fParagraphTextStart];
        fShapedText->fGlyphUnitProperties[converted] |= GlyphUnitFlags::kGlyphClusterStart;
        if (this->hasProperty(converted, CodeUnitFlags::kGraphemeStart)) {
            fShapedText->fGlyphUnitProperties[converted] |= GlyphUnitFlags::kGraphemeClusterStart;
        }
        return converted;
    });

    fShapedText->fRuns.emplace_back(std::move(*fCurrentRun));

    fRunGlyphStart += fCurrentRun->width();
}

SkFont UnicodeText::createFont(const FontBlock& fontBlock) {

    if (fontBlock.chain->count() == 0) {
        return SkFont();
    }
    sk_sp<SkTypeface> typeface = fontBlock.chain->operator[](0);

    SkFont font(std::move(typeface), fontBlock.chain->size());
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
    Stretch cluster;

    for (size_t runIndex = 0; runIndex < this->fRuns.size(); ++runIndex ) {

        auto& run = this->fRuns[runIndex];
        TextMetrics runMetrics(run.fFont);
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
              spaces.moveTo(cluster);
              wrappedText->addLine(line, spaces, unicode, true);
              line = spaces;
              clusters = spaces;
              continue;
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
              wrappedText->addLine(line, spaces, unicode, false);
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
    } else if (wrappedText->fLines.empty()) {
        // Empty text; we still need a line to avoid checking for empty lines every time
        line.moveTo(cluster);
        spaces.moveTo(cluster);
    }

    wrappedText->addLine(line, spaces, unicode, false);

    wrappedText->fActualSize.fWidth = width;
    return std::move(wrappedText);
}

void WrappedText::addLine(Stretch& stretch, Stretch& spaces, SkUnicode* unicode, bool hardLineBreak) {

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
    this->fLines.emplace_back(stretch, spaces, std::move(visualOrder), fActualSize.fHeight, hardLineBreak);
    fActualSize.fHeight += stretch.textMetrics().height();

    stretch.clean();
    spaces.clean();
}

sk_sp<FormattedText> WrappedText::format(TextAlign textAlign, TextDirection textDirection) {
    auto formattedText = sk_sp<FormattedText>(new FormattedText());

    formattedText->fRuns = this->fRuns;
    formattedText->fLines = this->fLines;
    formattedText->fGlyphUnitProperties = this->fGlyphUnitProperties;
    formattedText->fActualSize = this->fActualSize;
    if (textAlign == TextAlign::kLeft) {
        // Good by default
    } else if (textAlign == TextAlign::kCenter) {
        for (auto& line : formattedText->fLines) {
            line.fHorizontalOffset = (this->fActualSize.width() - line.fTextWidth) / 2.0f;
        }
        formattedText->fActualSize.fWidth = this->fActualSize.fWidth;
    } else {
        // TODO: Implement all formatting features
    }

    return std::move(formattedText);
}

void FormattedText::visit(Visitor* visitor) const {

    SkPoint offset = SkPoint::Make(0 , 0);
    for (auto& line : this->fLines) {
        offset.fX = 0;
        visitor->onBeginLine(line.text());
        for (auto index = 0; index < line.runsNumber(); ++index) {
            auto runIndex = line.visualRun(index);
            auto& run = this->fRuns[runIndex];

            GlyphRange glyphRange(line.glyphRange(runIndex, run.size(), false /* excluding trailing spaces */));
            // Update positions
            SkAutoSTMalloc<256, SkPoint> positions(glyphRange.width() + 1);
            SkPoint shift = SkPoint::Make(-run.fPositions[glyphRange.fStart].fX, line.baseline());
            for (size_t i = glyphRange.fStart; i <= glyphRange.fEnd; ++i) {
                positions[i - glyphRange.fStart] = run.fPositions[i] + shift + offset;
            }
            SkRect boundingRect = SkRect::MakeXYWH(shift.fX + offset.fX, offset.fY, run.fPositions[glyphRange.fEnd].fX  , run.fTextMetrics.height());
            visitor->onGlyphRun(run.fFont, run.getTextRange(glyphRange), boundingRect, glyphRange.width(), run.fGlyphs.data() + glyphRange.fStart, positions.data(), run.fOffsets.data() + glyphRange.fStart);
            offset.fX += run.calculateWidth(glyphRange);
        }
        visitor->onEndLine(line.text());
        offset.fY += line.height();
    }
}

void FormattedText::visit(Visitor* visitor, SkSpan<TextIndex> textChunks) const {
    // Decor blocks have to be sorted by text cannot intersect but can skip some parts of the text
    // (in which case we use default text style from paragraph style)
    // The edges of the decor blocks don't have to match glyph, grapheme or even unicode code point edges
    // It's out responsibility to adjust them to some reasonable values
    // [a:b) -> [c:d) where
    // c is closest GG cluster edge to a from the left and d is closest GG cluster edge to b from the left
    SkPoint offset = SkPoint::Make(0 , 0);
    for (auto& line : this->fLines) {
        offset.fX = 0;
        visitor->onBeginLine(line.text());
        for (auto index = 0; index < line.runsNumber(); ++index) {
            auto runIndex = line.visualRun(index);
            auto& run = this->fRuns[runIndex];
            if (run.size() == 0) {
                continue;
            }

            GlyphRange runGlyphRange(line.glyphRange(runIndex, run.size(), false /* excluding trailing spaces */));
            SkPoint shift = SkPoint::Make(-run.fPositions[runGlyphRange.fStart].fX, line.baseline());
            // The run edges are good (aligned to GGC)
            // "ABCdef" -> "defCBA"
            // "AB": red
            // "Cd": green
            // "ef": blue
            // green[d] blue[ef] green [C] red [BA]

            // chunks[text index] -> chunks [glyph index] -> walk through in glyph index order (the visual order)
            run.forEachTextChunkInGlyphRange(textChunks, runGlyphRange, [&](TextRange textRange, GlyphRange glyphRange){
                // Update positions & calculate the bounding rect
                SkAutoSTMalloc<256, SkPoint> positions(glyphRange.width() + 1);
                for (GlyphIndex i = glyphRange.fStart; i <= glyphRange.fEnd; ++i) {
                    positions[i - glyphRange.fStart] = run.fPositions[i] + shift + offset + SkPoint::Make(line.horizontalOffset(), 0.0f);
                }
                SkRect boundingRect = SkRect::MakeXYWH(positions[0].fX + line.horizontalOffset(), offset.fY, positions[glyphRange.width()].fX - positions[0].fX, run.fTextMetrics.height());
                visitor->onGlyphRun(run.fFont, run.getTextRange(glyphRange), boundingRect, glyphRange.width(), run.fGlyphs.data() + glyphRange.fStart, positions.data(), run.fOffsets.data() + glyphRange.fStart);

            });

            offset.fX += run.calculateWidth(runGlyphRange);
        }
        visitor->onEndLine(line.text());
        offset.fY += line.height();
    }
}

// Find the element that includes textIndex
Position FormattedText::adjustedPosition(PositionType positionType, TextIndex textIndex) const {
    Position position(positionType);
    SkScalar shift = 0;
    for (auto& line : fLines) {
        position.fBoundaries.fTop = position.fBoundaries.fBottom;
        position.fBoundaries.fBottom = line.verticalOffset() + line.height();
        if (!line.text().contains(textIndex) && !line.whitespaces().contains(textIndex) ) {
            continue;
        }

        shift = 0;
        for (auto index = 0; index < line.runsNumber(); ++index) {
            auto runIndex = line.visualRun(index);
            auto& run = fRuns[runIndex];

            GlyphRange runGlyphRange(line.glyphRange(runIndex, run.size(), true /* including trailing spaces */));
            auto runWidth = run.calculateWidth(runGlyphRange);

            if (!run.fUtf16Range.contains(textIndex)) {
                shift += runWidth;
                continue;
            }

            // Find the position left
            GlyphIndex found = run.findGlyphIndexLeftOf(runGlyphRange, textIndex);

            position.fLineIndex = lineIndex(&line);
            position.fRun = &run;
            position.fGlyphRange = GlyphRange(found, found == runGlyphRange.fEnd ? found : found + 1);
            position.fTextRange = position.fRun->getTextRange(position.fGlyphRange);
            this->adjustTextRange(&position);
            position.fBoundaries.fLeft += shift + run.calculateWidth(runGlyphRange.fStart, position.fGlyphRange.fStart);
            position.fBoundaries.fRight += shift + run.calculateWidth(runGlyphRange.fStart, position.fGlyphRange.fEnd);
            return position;
        }
        // The cursor is not on the text anymore; position it after the last element
        break;
    }

    position.fLineIndex = fLines.size() - 1;
    position.fRun = this->visuallyLastRun(position.fLineIndex);
    position.fGlyphRange = GlyphRange(position.fRun->size(), position.fRun->size());
    position.fTextRange = position.fRun->getTextRange(position.fGlyphRange);
    this->adjustTextRange(&position);
    position.fBoundaries.fLeft = fLines.back().withWithTrailingSpaces();
    position.fBoundaries.fRight = fLines.back().withWithTrailingSpaces();
    return position;
}

Position FormattedText::adjustedPosition(PositionType positionType, SkPoint xy) const {

    xy.fX = std::min(xy.fX, this->fActualSize.fWidth);
    xy.fY = std::min(xy.fY, this->fActualSize.fHeight);

    Position position(positionType);
    position.fRun = this->visuallyLastRun(this->fLines.size() - 1);
    for (auto& line : fLines) {
        position.fBoundaries.fTop = position.fBoundaries.fBottom;
        position.fBoundaries.fBottom = line.verticalOffset() + line.height();
        position.fLineIndex = this->lineIndex(&line);
        if (position.fBoundaries.fTop > xy.fY) {
            // We are past the point vertically
            break;
        } else if (position.fBoundaries.fBottom <= xy.fY) {
            // We haven't reached the point vertically yet
            continue;
        }

        // We found the line that contains the point;
        // let's walk through all its runs and find the element
        SkScalar runOffsetInLine = 0;
        for (auto index = 0; index < line.runsNumber(); ++index) {
            auto runIndex = line.visualRun(index);
            auto& run = fRuns[runIndex];
            GlyphRange runGlyphRangeInLine = line.glyphRange(runIndex, run.size(), true /* including trailing space */);
            SkScalar runWidthInLine = run.calculateWidth(runGlyphRangeInLine);
            position.fRun = &run;
            if (runOffsetInLine > xy.fX) {
                // We are past the point horizontally
                break;
            } else if (runOffsetInLine + runWidthInLine < xy.fX) {
                // We haven't reached the point horizontally yet
                runOffsetInLine += runWidthInLine;
                continue;
            }

            // We found the run that contains the point
            // let's find the glyph
            GlyphIndex found = runGlyphRangeInLine.fStart;
            for (auto i = runGlyphRangeInLine.fStart; i <= runGlyphRangeInLine.fEnd; ++i) {
                if (runOffsetInLine + run.calculateWidth(runGlyphRangeInLine.fStart, i) > xy.fX) {
                    break;
                }
                found = i;
            }

            position.fGlyphRange = GlyphRange(found, found == runGlyphRangeInLine.fEnd ? found : found + 1);
            position.fTextRange = position.fRun->getTextRange(position.fGlyphRange);
            this->adjustTextRange(&position);
            position.fBoundaries.fLeft = runOffsetInLine + run.calculateWidth(runGlyphRangeInLine.fStart, position.fGlyphRange.fStart);
            position.fBoundaries.fRight = runOffsetInLine + run.calculateWidth(runGlyphRangeInLine.fStart, position.fGlyphRange.fEnd);
            return position;
        }
        // The cursor is not on the text anymore; position it after the last element of the last visual run of the current line
        break;
    }

    position.fRun = this->visuallyLastRun(position.fLineIndex);
    auto line = this->line(position.fLineIndex);
    position.fGlyphRange.fStart =
    position.fGlyphRange.fEnd = line->glyphRange(this->runIndex(position.fRun), position.fRun->size(), true /* including trailing spaces */).fEnd;
    position.fTextRange = position.fRun->getTextRange(position.fGlyphRange);
    this->adjustTextRange(&position);
    position.fBoundaries.fLeft =
    position.fBoundaries.fRight = line->withWithTrailingSpaces();
    return position;
}

// Adjust the text positions to the position type
// (assuming for now that a grapheme cannot cross run edges; it's actually not true)
void FormattedText::adjustTextRange(Position* position) const {
    // The textRange is aligned on a glyph cluster
    if (position->fPositionType == PositionType::kGraphemeCluster) {
        // Move left to the beginning of the run
        while (position->fTextRange.fStart > position->fRun->fUtf8Range.begin() &&
               !this->hasProperty(position->fTextRange.fStart, GlyphUnitFlags::kGraphemeStart)) {
            --position->fTextRange.fStart;
        }
        // Update glyphRange, too
        while (position->fRun->fClusters[position->fGlyphRange.fStart] > position->fTextRange.fStart) {
            --position->fGlyphRange.fStart;
        }

        // Move right to the end of the run updating glyphRange, too
        while (position->fTextRange.fEnd < position->fRun->fUtf8Range.end() &&
               !this->hasProperty(position->fTextRange.fEnd, GlyphUnitFlags::kGraphemeStart)) {
            ++position->fTextRange.fEnd;
        }
        // Update glyphRange, too
        while (position->fRun->fClusters[position->fGlyphRange.fEnd] < position->fTextRange.fEnd) {
            ++position->fGlyphRange.fEnd;
        }
    } else {
        // TODO: Implement all the other position types
        SkASSERT(false);
    }
    position->fTextRange = TextRange(position->fRun->fClusters[position->fGlyphRange.fStart], position->fRun->fClusters[position->fGlyphRange.fEnd]);
}

// The range is guaranteed on the same line
bool FormattedText::recalculateBoundaries(Position& position) const {

    auto line = this->line(position.fLineIndex);
    auto runIndex = this->runIndex(position.fRun);

    GlyphIndex start = runIndex == line->glyphStart().runIndex() ? line->glyphStart().glyphIndex() : 0;
    GlyphIndex end = runIndex == line->glyphTrailingEnd().runIndex() ? line->glyphTrailingEnd().glyphIndex() : position.fRun->fGlyphs.size();

    SkASSERT (start <= position.fGlyphRange.fStart && end >= position.fGlyphRange.fEnd);
    auto left = position.fRun->calculateWidth(start, position.fGlyphRange.fStart);
    auto width = position.fRun->calculateWidth(position.fGlyphRange);
    position.fBoundaries = SkRect::MakeXYWH(left, line->verticalOffset(), width, line->getMetrics().height());
    return true;
}

const TextRun* FormattedText::visuallyPreviousRun(size_t lineIndex, const TextRun* run) const {
    auto line = this->line(lineIndex);
    auto runIndex = this->runIndex(run);
    for (auto i = 0; i < line->runsNumber(); ++i) {
        if (line->visualRun(i) == runIndex) {
            if (i == 0) {
                // Go to the previous line
                if (lineIndex == 0) {
                    // This is the first line
                    return nullptr;
                }
                // Previous line, last visual run
                line = this->line(--lineIndex);
                i = line->runsNumber() - 1;
            }
            return &fRuns[line->visualRun(i)];
        }
    }
    SkASSERT(false);
    return nullptr;
}

const TextRun* FormattedText::visuallyNextRun(size_t lineIndex, const TextRun* run) const {
    auto line = this->line(lineIndex);
    auto runIndex = this->runIndex(run);
    for (auto i = 0; i < line->runsNumber(); ++i) {
        if (line->visualRun(i) == runIndex) {
            if (i == line->runsNumber() - 1) {
                // Go to the next line
                if (lineIndex == fLines.size() - 1) {
                    // This is the last line
                    return nullptr;
                }
                // Next line, first visual run
                line = this->line(++lineIndex);
                i = 0;
            }
            return &fRuns[line->visualRun(i)];
        }
    }
    SkASSERT(false);
    return nullptr;
}

const TextRun* FormattedText::visuallyFirstRun(size_t lineIndex) const {
    auto line = this->line(lineIndex);
    return &fRuns[line->visualRun(0)];
}

const TextRun* FormattedText::visuallyLastRun(size_t lineIndex) const {
    auto line = this->line(lineIndex);
    return &fRuns[line->visualRun(line->runsNumber() - 1)];
}

Position FormattedText::previousElement(Position element) const {

    if (element.fGlyphRange.fStart == 0) {
        if (this->isVisuallyFirst(element.fLineIndex, element.fRun)) {
            element.fGlyphRange = GlyphRange { 0, 0};
            return element;
        }
        // We need to go to the visually previous run
        // (skipping all the empty runs if there are any)
        element.fRun = this->visuallyPreviousRun(element.fLineIndex, element.fRun);
        // Set the glyph range after the last glyph
        element.fGlyphRange = GlyphRange { element.fRun->fGlyphs.size(), element.fRun->fGlyphs.size()};
        if (element.fRun == nullptr) {
            return element;
        }
    }

    auto& clusters = element.fRun->fClusters;
    element.fGlyphRange = GlyphRange(element.fGlyphRange.fStart, element.fGlyphRange.fStart);
    element.fTextRange = TextRange(clusters[element.fGlyphRange.fStart],
                                   clusters[element.fGlyphRange.fStart]);
    while (element.fGlyphRange.fStart > 0) {
        // Shift left visually
        element.fTextRange.fStart = clusters[--element.fGlyphRange.fStart];
        if (element.fPositionType == PositionType::kGraphemeCluster) {
            if (this->hasProperty(element.fTextRange.fStart, GlyphUnitFlags::kGraphemeStart)) {
                break;
            }
        }
    }

    // Update the line
    auto line = this->line(element.fLineIndex);
    if (line->glyphStart().runIndex() == this->runIndex(element.fRun) &&
        line->glyphStart().glyphIndex() > element.fGlyphRange.fStart) {
        --element.fLineIndex;
    }

    // Either way we found us a grapheme cluster (just make sure of it)
    SkASSERT(this->hasProperty(element.fTextRange.fStart, GlyphUnitFlags::kGraphemeStart));
    return element;
}

Position FormattedText::nextElement(Position element) const {

    if (element.fGlyphRange.fEnd == element.fRun->size()) {
        // We need to go to the visually next run
        // (skipping all the empty runs if there are any)
        if (this->isVisuallyLast(element.fLineIndex, element.fRun)) {
            element.fGlyphRange = GlyphRange { element.fRun->size(), element.fRun->size() };
            return element;
        }
        element.fRun = this->visuallyNextRun(element.fLineIndex, element.fRun);
        // Set the glyph range after the last glyph
        element.fGlyphRange = GlyphRange { 0, 0};
        if (element.fRun == nullptr) {
            return element;
        }
    }

    auto& clusters = element.fRun->fClusters;
    element.fGlyphRange = GlyphRange(element.fGlyphRange.fEnd, element.fGlyphRange.fEnd);
    element.fTextRange = TextRange(clusters[element.fGlyphRange.fEnd],
                                   clusters[element.fGlyphRange.fEnd]);
    while (element.fGlyphRange.fEnd < element.fRun->size()) {
        // Shift left visually
        element.fTextRange.fEnd = clusters[++element.fGlyphRange.fEnd];
        if (element.fPositionType == PositionType::kGraphemeCluster) {
            if (this->hasProperty(element.fTextRange.fEnd, GlyphUnitFlags::kGraphemeStart)) {
                break;
            }
        }
    }
    // Update the line
    auto line = this->line(element.fLineIndex);
    if (line->glyphTrailingEnd().runIndex() == this->runIndex(element.fRun) &&
        line->glyphTrailingEnd().glyphIndex() < element.fGlyphRange.fEnd) {
        ++element.fLineIndex;
    }

    // Either way we found us a grapheme cluster (just make sure of it)
    SkASSERT(this->hasProperty(element.fTextRange.fEnd, GlyphUnitFlags::kGraphemeStart));
    return element;
}

bool FormattedText::isFirstOnTheLine(Position element) const {
    auto lineStart = this->line(element.fLineIndex)->glyphStart();
    return lineStart.runIndex() == this->runIndex(element.fRun) &&
           lineStart.glyphIndex() == element.fGlyphRange.fStart;
}

bool FormattedText::isLastOnTheLine(Position element) const {
    auto lineEnd = this->line(element.fLineIndex)->glyphEnd();
    return lineEnd.runIndex() == this->runIndex(element.fRun) &&
           lineEnd.glyphIndex() == element.fGlyphRange.fStart;
}

Position FormattedText::firstElement(PositionType positionType) const {

    Position beginningOfText(positionType);
    // We need to go to the visually next run
    // (skipping all the empty runs if there are any)
    beginningOfText.fRun = this->visuallyFirstRun(0);
    // Set the glyph range after the last glyph
    beginningOfText.fGlyphRange = GlyphRange { 0, 0};
    beginningOfText.fLineIndex = 0;

    return beginningOfText;// this->nextElement(beginningOfText);
}

Position FormattedText::lastElement(PositionType positionType) const {

    Position endOfText(positionType);
    // We need to go to the visually next run
    // (skipping all the empty runs if there are any)
    endOfText.fRun = this->visuallyLastRun(fLines.size() - 1);
    // Set the glyph range after the last glyph
    endOfText.fGlyphRange = GlyphRange { endOfText.fRun->size(), endOfText.fRun->size() };
    endOfText.fLineIndex = this->countLines() - 1;

    return endOfText; // this->previousElement(endOfText);
}

bool FormattedText::isVisuallyFirst(size_t lineIndex, const TextRun* run) const {
    auto firstLine = this->line(lineIndex);
    return this->runIndex(run) == firstLine->visualRun(0);
}

bool FormattedText::isVisuallyLast(size_t lineIndex, const TextRun* run) const {
    auto lastLine = this->line(lineIndex);
    return this->runIndex(run) == lastLine->visualRun(lastLine->runsNumber() - 1);
}
} // namespace text
} // namespace skia

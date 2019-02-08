/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShaped.h"
#include "SkFontMetrics.h"
#include "SkDashPathEffect.h"
#include "SkDiscretePathEffect.h"

ShapedParagraph::ShapedParagraph(SkTextBlobBuilder* builder,
                                 SkParagraphStyle style,
                                 std::vector<Block> blocks)
    : _currentWord(builder), _style(style), _blocks(std::move(blocks)) {
  _alphabeticBaseline = 0;
  _ideographicBaseline = 0;
  _height = 0;
  _width = 0;
  _maxIntrinsicWidth = 0;
  _minIntrinsicWidth = 0;
  _linesNumber = 0;
  _exceededLimits = false;
}

void ShapedParagraph::layout(SkScalar maxWidth, size_t maxLines) {
  _maxWidth = maxWidth;
  _maxLines = maxLines;
  _block = _blocks.begin();
  _linesNumber = 0;

  if (!_blocks.empty()) {
    auto start = _blocks.begin()->start;
    auto end = _blocks.empty() ? start - 1 : std::prev(_blocks.end())->end;

    if (start < end) {
      MultipleFontRunIterator font(start, end - start,
                                   _blocks.begin(),
                                   _blocks.end(),
                                   _style.getTextStyle());
      SkShaper shaper;
      shaper.shape(this, &font, start, end - start, true, {0, 0}, maxWidth);
      return;
    }

    // Shaper does not shape empty lines
    SkFontMetrics metrics;
    _block->textStyle.getFontMetrics(metrics);
    _alphabeticBaseline = -metrics.fAscent;
    _ideographicBaseline = -metrics.fAscent;
    _height = metrics.fDescent + metrics.fLeading - metrics.fAscent;
    _width = 0;
    _maxIntrinsicWidth = 0;
    _minIntrinsicWidth = 0;
    _linesNumber = 1;
    return;
  }

  // Shaper does not shape empty lines
  _height = 0;
  _width = 0;
  _maxIntrinsicWidth = 0;
  _minIntrinsicWidth = 0;
}

void ShapedParagraph::printBlocks(size_t linenum) {
  SkDebugf("Paragraph #%d\n", linenum);
  if (!_blocks.empty()) {
    SkDebugf("Lost blocks\n");
    for (auto& block : _blocks) {
      std::string str(block.start, block.end - block.start);
      SkDebugf("Block: '%s'\n", str.c_str());
    }
  }
  int i = 0;
  for (auto& line : _lines) {
    SkDebugf("Line: %d (%d)\n", i, line.blocks.size());
    for (auto& block : line.blocks) {
      std::string str(block.start, block.end - block.start);
      SkDebugf("Block: '%s'\n", str.c_str());
    }
    ++i;
  }
}

void ShapedParagraph::format() {

  size_t lineIndex = 0;
  for (auto& line : _lines) {

    ++lineIndex;
    SkScalar delta = _maxWidth - line.size.width();
    if (delta <= 0) {
      // Delta can be < 0 if there are extra whitespaces at the end of the line;
      // This is a limitation of a current version
      continue;
    }

    switch (_style.effective_align()) {
      case SkTextAlign::left:
        break;
      case SkTextAlign::right:
        for (auto& block : line.blocks) {
          block.shift += delta;
        }
        line.size.fWidth = _maxWidth;
        _width = _maxWidth;
        break;
      case SkTextAlign::center: {
        auto half = delta / 2;
        for (auto& block : line.blocks) {
          block.shift += half;
        }
        line.size.fWidth = _maxWidth;
        _width = _maxWidth;
        break;
      }
      case SkTextAlign::justify: {
        if (&line == &_lines.back()) {
          break;
        }
        SkScalar step = delta / (line.blocks.size() - 1);
        SkScalar shift = 0;
        for (auto& block : line.blocks) {
          block.shift += shift;
          if (&block != &line.blocks.back()) {
            block.rect.fRight += step;
            line.size.fWidth = _maxWidth;
            _width = _maxWidth;
          }
          shift += step;
        }
        break;
      }
      default:
        break;
    }
  }
}

void ShapedParagraph::paint(SkCanvas* textCanvas, SkPoint& point) {

  for (auto& line : _lines) {
    for (auto& block : line.blocks) {
      SkPaint paint;
      if (block.textStyle.hasForeground()) {
        paint = block.textStyle.getForeground();
      } else {
        paint.reset();
        paint.setColor(block.textStyle.getColor());
      }
      paint.setAntiAlias(true);

      SkPoint start = SkPoint::Make(point.x() + block.shift, point.y());
      block.PaintBackground(textCanvas, start);
      block.PaintShadow(textCanvas, start);

      textCanvas->drawTextBlob(block.blob, start.x(), start.y(), paint);
    }

    std::vector<Block>::const_iterator start = line.blocks.begin();
    SkScalar width = 0;
    for (auto block = line.blocks.begin(); block != line.blocks.end();
         ++block) {
      if (start == block) {
        width += block->rect.width();
      } else if (start->textStyle == block->textStyle) {
        width += block->rect.width();
      } else {
        PaintDecorations(textCanvas, start, block, point, width);
        start = block;
        width = block->rect.width();
      }
    }

    if (start != line.blocks.end()) {
      PaintDecorations(textCanvas, start, line.blocks.end(), point, width);
    }
  }
  point.fY += _height;
}

void ShapedParagraph::addWord(const char* startWord,
                              const char* endWord,
                              SkPoint point,
                              SkVector advance,
                              SkScalar baseline) {
  if (_linesNumber >= _maxLines) {
    return;
  }

//SkString word(startWord, endWord - startWord);
//SkDebugf("Word: '%s'\n", word.c_str());

  _minIntrinsicWidth = SkMaxScalar(_minIntrinsicWidth, advance.fX);
  _ideographicBaseline = baseline;
  _alphabeticBaseline = baseline;

// Find the first style attached to that text
  while (_block != _blocks.end() && startWord >= _block->end) {
    ++_block;
  }

  SkASSERT(_block != _blocks.end());
  _block->blob = _currentWord->make();
  _block->rect = SkRect::MakeXYWH(point.fX, point.fY, advance.fX, advance.fY);

  std::string str(startWord, endWord - startWord);
  if (_block->end > endWord) {
// One block (style) can have few runs (words); let's break them here
// TODO: deal with the trimmed values
    auto oldEnd = _block->end;
    _block->end = endWord;
    _block = _blocks.emplace(std::next(_block),
                             endWord,
                             oldEnd,
                             _block->blob,
                             _block->rect,
                             _block->textStyle);
  } else if (_block->end == endWord) {
// Just move on
    ++_block;
  } else {
// One word is covered by many styles
// We have 3 solutions here:
// 1. Stop it by separating runs by any style, not only font-related
// 2. Take the first style for the entire word (implemented)
// 3. TODO: Make each run a glyph, not a "word" and deal with it appropriately
// Remove all the other styles that cover only this word
    int n = 0;
    while (_block != _blocks.end() && _block->end <= endWord) {
      _block = _blocks.erase(_block);
      ++n;
    }
  }
}

void ShapedParagraph::addLine(const char* start,
                              const char* end,
                              SkPoint point,
                              SkVector advance,
                              size_t lineIndex,
                              bool lastLine) {
  if (_linesNumber >= _maxLines || _exceededLimits) {
// We already exceeded the limits - just ignore everything
    return;
  }
  _linesNumber = lineIndex;
//SkString line(start, end - start);
//SkDebugf("addLine #%d: %f '%s'\n", lineIndex, advance.fY, line.c_str());

  _advance = advance;
  _height = advance.fY;
  _width = SkMaxScalar(_width, advance.fX);
  _maxIntrinsicWidth += advance.fX;

// Check if we can and should use ellipsis
  if (!isinf(_maxWidth) &&
      !lastLine &&                      // We still have some text to shape
      _style.ellipsized() &&                   // We have ellipsis
      (_style.unlimited_lines() ||
          _style.getMaxLines() == lineIndex)) {// The last line
// This is the last allowed line
// The line width exceeds the constraint
    while (_block != _blocks.begin()) {
// Create ellipsis blob to measure it and possibly to add
// We cannot create an ellipsis here because it must match the font of the last run
// We cannot find out the last run until we measure the ellipsis :)
      auto ellipsisStyle = std::prev(_block)->textStyle;
      const std::string& ellipsis = _style.getEllipsis();
      SkTextBlobBuilderRunHandler handler;
      SkFont font(ellipsisStyle.getTypeface(), ellipsisStyle.getFontSize());
      font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
      SkShaper shaper;
      SkPoint ellipsisStart = SkPoint::Make(_advance.fX, point.fY);
      auto ellipsisEnd = shaper.shape(&handler,
                                      font,
                                      ellipsis.data(),
                                      ellipsis.size(),
                                      true,
                                      ellipsisStart,
                                      _maxWidth);
      auto size = ellipsisEnd - ellipsisStart;

      if (ellipsisEnd.fX <= _maxWidth || _blocks.size() == 1) {
        _block = _blocks.emplace(_block, _block->start, _block->start,
                                 handler.makeBlob(),
                                 SkRect::MakeXYWH(ellipsisStart.x(),
                                                  ellipsisStart.y(),
                                                  size.x(),
                                                  size.y()),
                                 ellipsisStyle);
        ++_block;
        _exceededLimits = true;
        break;
      }
      --_block;
      _advance.fX -= _block->rect.width();
    }
  }

// Move shaped blocks to the line
  std::vector<Block> first;
  std::move(_blocks.begin(), _block, std::back_inserter(first));
  _blocks.erase(_blocks.begin(), _block);
// Add one more line and start from the it's first block again
  _lines.emplace_back(first, advance);

  _block = _blocks.begin();
}

SkScalar ShapedParagraph::ComputeDecorationThickness(SkTextStyle textStyle) {

  SkScalar thickness = 1.0f;

  SkFontMetrics metrics;
  textStyle.getFontMetrics(metrics);

  switch (textStyle.getDecoration()) {
    case SkTextDecoration::kUnderline:
      if (!metrics.hasUnderlineThickness(&thickness)) {
        thickness = 1.f;
      }
      break;
    case SkTextDecoration::kOverline:
      break;
    case SkTextDecoration::kLineThrough:
      if (!metrics.hasStrikeoutThickness(&thickness)) {
        thickness = 1.f;
      }
      break;
    default:
      SkASSERT(false);
  }

  thickness = SkMaxScalar(thickness, textStyle.getFontSize() / 14.f);

  return thickness * textStyle.getDecorationThicknessMultiplier();
}

SkScalar
ShapedParagraph::ComputeDecorationPosition(Block block, SkScalar thickness) {

  SkFontMetrics metrics;
  block.textStyle.getFontMetrics(metrics);

  SkScalar position;

  switch (block.textStyle.getDecoration()) {
    case SkTextDecoration::kUnderline:
      if (metrics.hasUnderlinePosition(&position)) {
        return position - metrics.fAscent;
      } else {
        position = metrics.fDescent - metrics.fAscent;
        if (block.textStyle.getDecorationStyle() == SkTextDecorationStyle::kWavy
            ||
                block.textStyle.getDecorationStyle()
                    == SkTextDecorationStyle::kDouble
            ) {
          return position - thickness * 3;
        } else {
          return position - thickness;
        }
      }

      break;
    case SkTextDecoration::kOverline:
      return 0;
      break;
    case SkTextDecoration::kLineThrough: {
      SkScalar delta = block.rect.height()
          - (metrics.fDescent - metrics.fAscent + metrics.fLeading);
      position = SkTMax(0.0f, delta) + (metrics.fDescent - metrics.fAscent) / 2;
      break;
    }
    default:
      position = 0;
      SkASSERT(false);
      break;
  }

  return position;
}

void ShapedParagraph::ComputeDecorationPaint(Block block,
                                             SkPaint& paint,
                                             SkPath& path,
                                             SkScalar width) {

  paint.setStyle(SkPaint::kStroke_Style);
  if (block.textStyle.getDecorationColor() == SK_ColorTRANSPARENT) {
    paint.setColor(block.textStyle.getColor());
  } else {
    paint.setColor(block.textStyle.getDecorationColor());
  }
  paint.setAntiAlias(true);

  SkScalar scaleFactor = block.textStyle.getFontSize() / 14.f;

  switch (block.textStyle.getDecorationStyle()) {
    case SkTextDecorationStyle::kSolid:
      break;

    case SkTextDecorationStyle::kDouble:
      break;

      // Note: the intervals are scaled by the thickness of the line, so it is
      // possible to change spacing by changing the decoration_thickness
      // property of TextStyle.
    case SkTextDecorationStyle::kDotted: {
      const SkScalar intervals[] =
          {1.0f * scaleFactor, 1.5f * scaleFactor, 1.0f * scaleFactor,
           1.5f * scaleFactor};
      size_t count = sizeof(intervals) / sizeof(intervals[0]);
      paint.setPathEffect(SkPathEffect::MakeCompose(
          SkDashPathEffect::Make(intervals, count, 0.0f),
          SkDiscretePathEffect::Make(0, 0)));
      break;
    }
      // Note: the intervals are scaled by the thickness of the line, so it is
      // possible to change spacing by changing the decoration_thickness
      // property of TextStyle.
    case SkTextDecorationStyle::kDashed: {
      const SkScalar intervals[] =
          {4.0f * scaleFactor, 2.0f * scaleFactor, 4.0f * scaleFactor,
           2.0f * scaleFactor};
      size_t count = sizeof(intervals) / sizeof(intervals[0]);
      paint.setPathEffect(SkPathEffect::MakeCompose(
          SkDashPathEffect::Make(intervals, count, 0.0f),
          SkDiscretePathEffect::Make(0, 0)));
      break;
    }
    case SkTextDecorationStyle::kWavy: {

      int wave_count = 0;
      double x_start = 0;
      double wavelength = 2 * scaleFactor;

      path.moveTo(0, 0);
      while (x_start + wavelength * 2 < width) {
        path.rQuadTo(wavelength, wave_count % 2 != 0 ? wavelength : -wavelength,
                     wavelength * 2, 0);
        x_start += wavelength * 2;
        ++wave_count;
      }
      break;
    }
  }
}

void ShapedParagraph::PaintDecorations(SkCanvas* canvas,
                                       std::vector<Block>::const_iterator begin,
                                       std::vector<Block>::const_iterator end,
                                       SkPoint offset,
                                       SkScalar width) {

  if (begin == end) {
    return;
  }

  auto block = *begin;
  if (block.textStyle.getDecoration() == SkTextDecoration::kNone) {
    return;
  }

  // Decoration thickness
  SkScalar thickness = ComputeDecorationThickness(block.textStyle);

  // Decoration position
  SkScalar position = ComputeDecorationPosition(block, thickness);

  // Decoration paint (for now) and/or path
  SkPaint paint;
  SkPath path;
  ComputeDecorationPaint(block, paint, path, width);
  paint.setStrokeWidth(thickness);

  // Draw the decoration
  SkScalar x = offset.x() + block.rect.left() + block.shift;
  SkScalar y = offset.y() + block.rect.top() + position;
  switch (block.textStyle.getDecorationStyle()) {
    case SkTextDecorationStyle::kWavy:
      path.offset(x, y);
      canvas->drawPath(path, paint);
      break;
    case SkTextDecorationStyle::kDouble: {
      canvas->drawLine(x, y, x + width, y, paint);
      SkScalar bottom = y + thickness * 2;
      canvas->drawLine(x, bottom, x + width, bottom, paint);
      break;
    }
    case SkTextDecorationStyle::kDashed:
    case SkTextDecorationStyle::kDotted:
    case SkTextDecorationStyle::kSolid:
      canvas->drawLine(x, y, x + width, y, paint);
      break;
    default:
      break;
  }
}

void ShapedParagraph::GetRectsForRange(const char* start,
                                       const char* end,
                                       std::vector<SkTextBox>& result) {
  for (auto& line : _lines) {
    for (auto& block : line.blocks) {
      if (block.end <= start || block.start >= end) {
        continue;
      }
      result.emplace_back(block.rect,
                          SkTextDirection::ltr); // TODO: the right direction
    }
  }
}
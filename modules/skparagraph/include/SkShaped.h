/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <vector>
#include "uchar.h"
#include "SkColor.h"
#include "SkCanvas.h"
#include "SkShaper.h"
#include "SkTextStyle.h"
#include "SkParagraphStyle.h"
#include "SkTextBlobPriv.h"

// The smallest part of the text that is painted separately
struct Block {
  Block(
      const char* start,
      const char* end,
      sk_sp<SkTextBlob> blob, SkRect rect, SkTextStyle style)
      : start(start)
      , end(end)
      , textStyle(style)
      , blob(blob)
      , rect(rect)
      , shift(0)
  {}
  Block(const char* start, const char* end, SkTextStyle style)
      : start(start)
      , end(end)
      , textStyle(style)
      , shift(0)
  {}
  const char* start;
  const char*  end;
  SkTextStyle textStyle;
  sk_sp<SkTextBlob> blob;
  SkRect rect;
  SkScalar shift;


  void PaintBackground(SkCanvas* canvas, SkPoint offset) {

    if (!textStyle.hasBackground()) {
      return;
    }
    rect.offset(offset.fY, offset.fY);
    canvas->drawRect(rect, textStyle.getBackground());
  }

  void PaintShadow(SkCanvas* canvas, SkPoint offset) {
    if (textStyle.getShadowNumber() == 0) {
      return;
    }

    for (SkTextShadow shadow : textStyle.getShadows()) {
      if (!shadow.hasShadow()) {
        continue;
      }

      SkPaint paint;
      paint.setColor(shadow.color);
      if (shadow.blur_radius != 0.0) {
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, shadow.blur_radius, false));
      }
      canvas->drawTextBlob(blob, offset.x() + shadow.offset.x(), offset.y() + shadow.offset.y(), paint);
    }
  }
};

// Comes from the paragraph
struct StyledText {

  StyledText(size_t start, size_t end, SkTextStyle textStyle)
      : start(start), end(end), textStyle(textStyle) { };

  bool operator==(const StyledText& rhs) const {
    return start == rhs.start &&
        end == rhs.end &&
        textStyle == rhs.textStyle;
  }
  size_t start;
  size_t end;
  SkTextStyle textStyle;
};

struct Line {
  Line(std::vector<Block> blocks, SkVector advance)
      : blocks(std::move(blocks)) {
    size.fHeight = advance.fY;
    size.fWidth = advance.fX;
  }
  std::vector<Block> blocks;
  SkSize size;
  size_t Length() const { return blocks.empty() ? 0 :  blocks.back().end - blocks.front().start; }
  bool IsEmpty() const { return blocks.empty() || Length() == 0; }
};

class MultipleFontRunIterator final : public FontRunIterator {
 public:
  MultipleFontRunIterator(const char* utf8,
                          size_t utf8Bytes,
                          std::vector<Block>::iterator begin,
                          std::vector<Block>::iterator end,
                          SkTextStyle defaultStyle)
      : fCurrent(utf8)
      , fEnd(fCurrent + utf8Bytes)
      , fCurrentStyle(SkTextStyle())
      , fDefaultStyle(defaultStyle)
      , fIterator(begin)
      , fNext(begin)
      , fLast(end)
  {
    fCurrentTypeface = SkTypeface::MakeDefault();
    MoveToNext();
  }

  void consume() override {

    if (fIterator == fLast) {
      fCurrent = fEnd;
      fCurrentStyle = fDefaultStyle;
    } else {
      fCurrent = fNext == fLast ? fEnd : std::next(fCurrent, fNext->start - fIterator->start);
      fCurrentStyle = fIterator->textStyle;
    }

    fCurrentTypeface = fCurrentStyle.getTypeface();
    fFont = SkFont(fCurrentTypeface, fCurrentStyle.getFontSize());

    MoveToNext();
  }
  const char* endOfCurrentRun() const override {
    return fCurrent;
  }
  bool atEnd() const override {
    return fCurrent == fEnd;
  }

  const SkFont* currentFont() const override {
    return &fFont;
  }

  void MoveToNext() {

    fIterator = fNext;
    if (fIterator == fLast) {
      return;
    }
    // This is a semi-solution allows flutter to run correctly:
    // we break runs on every style change even if the font is still the same
    ++fNext;
  }

  /*
  SkTextStyle currentTextStyle() const {
    return fCurrentStyle;
  }
  SkTextStyle currentDefaultStyle() const {
    return fDefaultStyle;
  }
  sk_sp<SkTypeface> currentTypeface() const {
    return fCurrentTypeface;
  }
  */
 private:
  const char* fCurrent;
  const char* fEnd;
  SkFont fFont;
  SkTextStyle fCurrentStyle;
  SkTextStyle fDefaultStyle;
  std::vector<Block>::iterator fIterator;
  std::vector<Block>::iterator fNext;
  std::vector<Block>::iterator fLast;
  sk_sp<SkTypeface> fCurrentTypeface;
};

class ShapedParagraph final : public SkShaper::RunHandler {
 public:

  ShapedParagraph() { }

  ShapedParagraph(SkTextBlobBuilder* builder, SkParagraphStyle style, std::vector<Block> blocks);

  SkShaper::RunHandler::Buffer newRunBuffer(const RunInfo&, const SkFont& font, int glyphCount, int textCount) override {
    const auto& runBuffer = SkTextBlobBuilderPriv::AllocRunTextPos
                                (_currentWord, font, glyphCount, textCount, SkString());
    return { runBuffer.glyphs,
             runBuffer.points(),
             runBuffer.utf8text,
             runBuffer.clusters };
  }

  size_t lineNumber() const { return _lines.size(); }

  void layout(SkScalar maxWidth, size_t maxLines);

  void printBlocks(size_t linenum);

  void format();

  void paint(SkCanvas* textCanvas, SkPoint& point);

  void commitLine() override { }

  void addWord(const char* startWord, const char* endWord, SkPoint point, SkVector advance, SkScalar baseline) override;

  void addLine(const char* start, const char* end, SkPoint point, SkVector advance, size_t lineIndex, bool lastLine) override;

  SkScalar alphabeticBaseline() { return _alphabeticBaseline; }
  SkScalar height() { return _height; }
  SkScalar width() { return _width; }
  SkScalar ideographicBaseline() { return _ideographicBaseline; }
  SkScalar maxIntrinsicWidth() { return _maxIntrinsicWidth; }
  SkScalar minIntrinsicWidth() { return _minIntrinsicWidth; }

  SkScalar ComputeDecorationThickness(SkTextStyle textStyle);

  SkScalar ComputeDecorationPosition(Block block, SkScalar thickness);

  void ComputeDecorationPaint(Block block, SkPaint& paint, SkPath& path, SkScalar width);

  void PaintDecorations(SkCanvas* canvas,
                        std::vector<Block>::const_iterator begin,
                        std::vector<Block>::const_iterator end,
                        SkPoint offset,
                        SkScalar width);


  const char* start() { return _blocks.front().start; }

  const char* end() { return _blocks.back().end; }

  void GetRectsForRange(const char* start, const char* end, std::vector<SkTextBox>& result);

 private:
  // TODO: we create a text blob in one callback from Shaper and use it in another. Fix it!
  SkTextBlobBuilder* _currentWord;

  // Constrains
  SkScalar _maxWidth;
  size_t _maxLines;

  // Output to Flutter
  size_t _linesNumber;
  SkScalar _alphabeticBaseline;
  SkScalar _ideographicBaseline;
  SkScalar _height;
  SkScalar _width;
  SkScalar _maxIntrinsicWidth;
  SkScalar _minIntrinsicWidth;

  // Internal structures
  SkVector _advance;
  bool     _exceededLimits; // Lines number exceed the limit and there is an ellipse
  SkParagraphStyle _style;
  std::vector<Line> _lines;
  std::vector<Block> _blocks;
  std::vector<Block>::iterator _block;
};
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <unicode/brkiter.h>
#include <unicode/ubidi.h>
#include "include/core/SkBlurTypes.h"
#include "include/core/SkFontMgr.h"
#include "src/core/SkSpan.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkCanvas.h"
#include "src/utils/SkUTF.h"
#include "SkParagraphImpl.h"

namespace {
/*
  std::string toString(SkSpan<const char> text) {
    icu::UnicodeString
        utf16 = icu::UnicodeString(text.begin(), SkToS32(text.size()));
    std::string str;
    utf16.toUTF8String(str);
    return str;
  }
  */
  SkSpan<const char> operator*(const SkSpan<const char>& a, const SkSpan<const char>& b) {
    auto begin = SkTMax(a.begin(), b.begin());
    auto end = SkTMin(a.end(), b.end());
    return SkSpan<const char>(begin, end > begin ? end - begin : 0);
  }
  void print(const SkCluster& cluster) {
    /*
    auto type = cluster.breakType() == SkCluster::BreakType::HardLineBreak
                ? "!"
                : (cluster.isSoftBreak() ? "?" : " ");
    SkDebugf("Cluster %s%s", type, cluster.isWhitespaces() ? "*" : " ");
    SkDebugf("[%d:%d) ", cluster.startPos(), cluster.endPos());
    SkDebugf(" %f ", cluster.width());

    SkDebugf("'");
    for (auto ch = cluster.text().begin(); ch != cluster.text().end(); ++ch) {
      SkDebugf("%c", *ch);
    }
    SkDebugf("'");

    if (cluster.text().size() != 1) {
      SkDebugf("(%d)\n", cluster.text().size());
    } else {
      if (cluster.endPos() - cluster.startPos() == 1) {
        SkDebugf(" %f + %f", cluster.run()->positionX(cluster.startPos()), cluster.run()->offset(cluster.startPos()));
      }
      SkDebugf("\n");
    }

    if (cluster.endPos() - cluster.startPos() > 1) {
      SkDebugf("Offsets: ");
      for (size_t i = cluster.startPos(); i < cluster.endPos(); ++i) {
        SkDebugf("%f ", cluster.run()->offset(i));
      }
      SkDebugf("\n");
    }
     */
  }
  /*
  SkSpan<const char> operator*(const SkSpan<const char>& a, const SkSpan<const char>& b) {
    auto begin = SkTMax(a.begin(), b.begin());
    auto end = SkTMin(a.end(), b.end());
    return SkSpan<const char>(begin, end > begin ? end - begin : 0);
  }
*/
  inline SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
  }
}

SkParagraphImpl::~SkParagraphImpl() = default;

void SkParagraphImpl::layout(SkScalar width) {

  this->resetContext();

  this->resolveStrut();

  this->shapeTextIntoEndlessLine();

  this->buildClusterTable();

  this->breakShapedTextIntoLines(width);
}

void SkParagraphImpl::resolveStrut() {

  auto strutStyle = this->paragraphStyle().getStrutStyle();
  if (!strutStyle.fStrutEnabled) {
    return;
  }

  sk_sp<SkTypeface> typeface;
  for (auto& fontFamily : strutStyle.fFontFamilies) {
    typeface = fFontCollection->matchTypeface(fontFamily, strutStyle.fFontStyle);
    if (typeface.get() != nullptr) {
      break;
    }
  }
  if (typeface.get() == nullptr) {
    typeface = SkTypeface::MakeDefault();
  }

  SkFont font(typeface, strutStyle.fFontSize);
  SkFontMetrics metrics;
  font.getMetrics(&metrics);

  fStrutMetrics = SkLineMetrics(
    metrics.fAscent * strutStyle.fHeight,
    metrics.fDescent* strutStyle.fHeight,
    strutStyle.fLeading < 0 ? metrics.fLeading : strutStyle.fLeading * strutStyle.fFontSize
  );
}

void SkParagraphImpl::paint(SkCanvas* canvas, SkScalar x, SkScalar y) {

  if (nullptr == fPicture) {
    // Build the picture lazily not until we actually have to paint (or never)
    this->formatLines(fWidth);
    this->paintLinesIntoPicture();
  }

  SkMatrix matrix = SkMatrix::MakeTrans(x, y);
  canvas->drawPicture(fPicture, &matrix, nullptr);
}

void SkParagraphImpl::resetContext() {

  fAlphabeticBaseline = 0;
  fHeight = 0;
  fWidth = 0;
  fIdeographicBaseline = 0;
  fMaxIntrinsicWidth = 0;
  fMinIntrinsicWidth = 0;
  fMaxLineWidth = 0;

  fPicture = nullptr;
  fRuns.reset();
  fClusters.reset();
  fLines.reset();
  fTextWrapper.reset();
}

// Clusters in the order of the input text
void SkParagraphImpl::buildClusterTable() {

  // Find all possible (soft) line breaks
  SkTextBreaker breaker;
  if (!breaker.initialize(fUtf8, UBRK_LINE)) {
    return;
  }
  size_t currentPos = breaker.first();
  SkTHashMap<const char*, bool> softLineBreaks;
  while (!breaker.eof()) {
    currentPos = breaker.next();
    const char* ch = currentPos + fUtf8.begin();
    softLineBreaks.set(ch, breaker.status() == UBRK_LINE_HARD);
  }

  SkBlock* currentStyle = this->fTextStyles.begin();
  SkScalar shift = 0;
  // Cannot set SkSpan<SkCluster> until the array is done - can be moved around
  std::vector<std::tuple<SkRun*, size_t, size_t>> toUpdate;

  // Walk through all the run in the direction of input text
  for (auto& run : fRuns) {

    auto runStart = fClusters.size();
    // Walk through the glyph in the direction of input text
    run.iterateThroughClustersInTextOrder(
      [&run, this, &softLineBreaks, &currentStyle, &shift]
      (size_t glyphStart,
       size_t glyphEnd,
       size_t charStart,
       size_t charEnd,
       SkScalar width,
       SkScalar height) {

        SkASSERT(charEnd >= charStart);
        SkSpan<const char> text(fUtf8.begin() + charStart, charEnd - charStart);

        auto& cluster = fClusters.emplace_back(&run, glyphStart, glyphEnd, text, width, height);

        // Mark the line breaks
        auto found = softLineBreaks.find(cluster.text().end());
        if (found) {
          cluster.setBreakType(*found
                               ? SkCluster::BreakType::HardLineBreak
                               : SkCluster::BreakType::SoftLineBreak);
        }
        cluster.setIsWhiteSpaces();

        // Shift the cluster
        run.shift(&cluster, shift);

        // Synchronize styles (one cluster can be covered by few styles)
        while (!cluster.startsIn(currentStyle->text())) {
          currentStyle++;
          SkASSERT(currentStyle != this->fTextStyles.end());
        }

        // Take spacing styles in account
        if (currentStyle->style().getWordSpacing() != 0 &&
            fParagraphStyle.getTextAlign() != SkTextAlign::justify) {
          if (cluster.isWhitespaces() && cluster.isSoftBreak()) {
            shift += run.addSpacesAtTheEnd(currentStyle->style().getWordSpacing(), &cluster);
          }
        }
        if (currentStyle->style().getLetterSpacing() != 0) {
          shift += run.addSpacesEvenly(currentStyle->style().getLetterSpacing(), &cluster);
        }

        print(cluster);
      });

    toUpdate.emplace_back(&run, runStart, fClusters.size() - runStart);
  }

  // Set SkSpan<SkCluster> ranges for all the runs
  for (auto update : toUpdate) {
    auto run = std::get<0>(update);
    auto start = std::get<1>(update);
    auto size = std::get<2>(update);
    run->setClusters(SkSpan<SkCluster>(&fClusters[start], size));
  }
}

void SkParagraphImpl::shapeTextIntoEndlessLine() {

 class MultipleFontRunIterator final : public SkShaper::FontRunIterator {
   public:
    MultipleFontRunIterator(
        SkSpan<const char> utf8,
        SkSpan<SkBlock> styles,
        sk_sp<SkFontCollection> fonts,
        bool hintingOn)
        : fText(utf8)
        , fCurrentChar(utf8.begin())
        , fCurrentStyle(styles.begin())
        , fStyles(styles)
        , fFontCollection(std::move(fonts))
        , fHintingOn(hintingOn) {

      resolveFonts();
    }

    bool existsInFont(sk_sp<SkTypeface> typeface, SkUnichar u) {
      return typeface->unicharToGlyph(u) != 0;
    }

    // TODO: Resolve fonts via harfbuzz
    void resolveFonts() {

      fResolvedFont.reset();

      sk_sp<SkTypeface> prevTypeface;
      SkScalar prevFontSize = 0;
      SkScalar prevLineHeight = 0;
      while (fCurrentChar != fText.end()) {

        SkFontStyle currentFontStyle = fCurrentStyle->style().getFontStyle();
        SkScalar currentFontSize = fCurrentStyle->style().getFontSize();
        SkScalar currentLineHeight = fCurrentStyle->style().getHeight();

        const char* current = fCurrentChar;
        SkUnichar u = utf8_next(&fCurrentChar, fText.end());

        // Find the first typeface from the collection that exists in the current text style
        sk_sp<SkTypeface> typeface = nullptr;
        for (auto& fontFamily : fCurrentStyle->style().getFontFamilies()) {
          typeface = fFontCollection->matchTypeface(fontFamily, currentFontStyle);
          if (typeface.get() != nullptr && existsInFont(typeface, u)) {
            if (!SkTypeface::Equal(typeface.get(), prevTypeface.get()) ||
                prevFontSize != currentFontSize ||
                prevLineHeight != currentLineHeight) {
              fResolvedFont.set(current, makeFont(typeface, currentFontSize, currentLineHeight));
              prevTypeface = typeface;
              prevFontSize = currentFontSize;
              prevLineHeight = currentLineHeight;
            }
            break;
          }
        }
        if (prevTypeface.get() == nullptr) {
          // We didn't find anything
          SkTypeface* candidate = fFontCollection->geFallbackManager()->matchFamilyStyleCharacter(
                  nullptr, currentFontStyle, nullptr, 0, u);
          if (candidate) {
            // We found a font with this character
            prevTypeface.reset(candidate);
          } else if (typeface != nullptr && typeface.get() != nullptr) {
            // We cannot resolve the character we may as well continue with the current font
            prevTypeface = typeface;
          } else {
            // Anything...
            prevTypeface = SkTypeface::MakeDefault();
          }
          fResolvedFont.set(current, makeFont(prevTypeface, currentFontSize, currentLineHeight));
          prevFontSize = currentFontSize;
          prevLineHeight = currentLineHeight;
        }

        // Find the text style that contains the next character
        while (fCurrentStyle != fStyles.end() && fCurrentStyle->text().end() <= fCurrentChar) {
          ++fCurrentStyle;
        }
      }

      fCurrentChar = fText.begin();
      fCurrentStyle = fStyles.begin();
    }

    void consume() override {

      SkASSERT(fCurrentChar < fText.end());

      auto found = fResolvedFont.find(fCurrentChar);
      if (found == nullptr) {
        fFont = SkFont();
        fLineHeight = 1;
      } else {
        // Get the font
        fFont = found->first;
        fLineHeight = found->second;
      }

      // Find the first character (index) that cannot be resolved with the current font
      while (++fCurrentChar != fText.end()) {
        found = fResolvedFont.find(fCurrentChar);
        if (found != nullptr) {
          break;
        }
      }
    }

    std::pair<SkFont, SkScalar> makeFont(sk_sp<SkTypeface> typeface, SkScalar size, SkScalar height) {
      SkFont font(typeface, size);
      font.setEdging(SkFont::Edging::kAntiAlias);
      if (!fHintingOn) {
        font.setHinting(SkFontHinting::kSlight);
        font.setSubpixel(true);
      }
      return std::make_pair(font, height);
    }

    size_t endOfCurrentRun() const override { return fCurrentChar - fText.begin(); }
    bool atEnd() const override { return fCurrentChar == fText.end(); }
    const SkFont& currentFont() const override { return fFont; }
    SkScalar lineHeight() const { return fLineHeight; }

   private:
    SkSpan<const char> fText;
    const char* fCurrentChar;
    SkFont fFont;
    SkScalar fLineHeight;
    SkBlock* fCurrentStyle;
    SkSpan<SkBlock> fStyles;
    sk_sp<SkFontCollection> fFontCollection;
    SkTHashMap<const char*, std::pair<SkFont, SkScalar>> fResolvedFont;
    bool fHintingOn;
  };

 class ShapeHandler final : public SkShaper::RunHandler {

   public:
    explicit ShapeHandler(SkParagraphImpl& paragraph, MultipleFontRunIterator* fontIterator)
        : fParagraph(&paragraph)
        , fFontIterator(fontIterator)
        , fAdvance(SkVector::Make(0, 0)) {}

    inline SkVector advance() const { return fAdvance; }

   private:

    void beginLine() override { }

    void runInfo(const RunInfo&) override { }

    void commitRunInfo() override { }

    Buffer runBuffer(const RunInfo& info) override {

      auto& run = fParagraph->fRuns.emplace_back(
          fParagraph->text(),
          info,
          fFontIterator->lineHeight(),
          fParagraph->fRuns.count(),
          fAdvance.fX);
      return run.newRunBuffer();
    }

    void commitRunBuffer(const RunInfo&) override {
      auto& run = fParagraph->fRuns.back();
      if (run.size() == 0) {
        fParagraph->fRuns.pop_back();
        return;
      }
      // Carve out the line text out of the entire run text
      fAdvance.fX += run.advance().fX;
      fAdvance.fY = SkMaxScalar(fAdvance.fY, run.descent() - run.ascent());
    }

    void commitLine() override { }

    SkParagraphImpl* fParagraph;
    MultipleFontRunIterator* fFontIterator;
    SkVector fAdvance;
 };

  SkSpan<SkBlock> styles(fTextStyles.begin(), fTextStyles.size());
  MultipleFontRunIterator font(fUtf8, styles, fFontCollection, fParagraphStyle.hintingIsOn());
  ShapeHandler handler(*this, &font);
  std::unique_ptr<SkShaper> shaper = SkShaper::MakeShapeDontWrapOrReorder();

  auto bidi = SkShaper::MakeIcuBiDiRunIterator(fUtf8.begin(), fUtf8.size(),
                          fParagraphStyle.getTextDirection() == SkTextDirection::ltr ? (uint8_t)2 : (uint8_t)1);
  auto script = SkShaper::MakeHbIcuScriptRunIterator(fUtf8.begin(), fUtf8.size());
  auto lang = SkShaper::MakeStdLanguageRunIterator(fUtf8.begin(), fUtf8.size());

  shaper->shape(fUtf8.begin(), fUtf8.size(),
               font,
               *bidi,
               *script,
               *lang,
               std::numeric_limits<SkScalar>::max(),
               &handler);

  fMaxIntrinsicWidth = handler.advance().fX;
}

void SkParagraphImpl::breakShapedTextIntoLines(SkScalar maxWidth) {

  fTextWrapper.formatText(
      SkSpan<SkCluster>(fClusters.begin(), fClusters.size()),
      maxWidth,
      fParagraphStyle.getMaxLines(),
      fParagraphStyle.getEllipsis());
  fHeight = fTextWrapper.height();
  fWidth = maxWidth; //fTextWrapper.width();
  fMinIntrinsicWidth = fTextWrapper.intrinsicWidth();
  fAlphabeticBaseline = fLines.empty() ? 0 : fLines.front().alphabeticBaseline();
  fIdeographicBaseline = fLines.empty() ? 0 : fLines.front().ideographicBaseline();
}

void SkParagraphImpl::formatLines(SkScalar maxWidth) {

  auto effectiveAlign = fParagraphStyle.effective_align();
  for (auto& line : fLines) {
    line.format(effectiveAlign, maxWidth, &line != &fLines.back());
  }
}

void SkParagraphImpl::paintLinesIntoPicture() {

  SkPictureRecorder recorder;
  SkCanvas* textCanvas = recorder.beginRecording(fWidth, fHeight, nullptr, 0);

  for (auto& line : fLines) {
    line.paint(textCanvas);
  }

  fPicture = recorder.finishRecordingAsPicture();
}

SkSpan<const SkBlock> SkParagraphImpl::findAllBlocks(SkSpan<const char> text) {

  const SkBlock* begin = nullptr;
  const SkBlock* end = nullptr;
  for (auto& block : fTextStyles) {
    if (block.text().end() <= text.begin()) {
      continue;
    }
    if (block.text().begin() >= text.end()) {
      break;
    }
    if (begin == nullptr) {
      begin = &block;
    }
    end = &block;
  }

  return SkSpan<const SkBlock>(begin, end - begin + 1);
}

SkLine& SkParagraphImpl::addLine(
    SkVector offset,
    SkVector advance,
    SkSpan<const char> text,
    SkSpan<const SkCluster> clusters,
    SkSpan<const SkCluster> end,
    SkLineMetrics sizes) {

  // Define a list of styles that covers the line
  auto blocks = findAllBlocks(text);

  return fLines.emplace_back
      (offset,
       advance,
       blocks,
       text,
       clusters,
       end,
       sizes);
}

// Returns a vector of bounding boxes that enclose all text between
// start and end glyph indexes, including start and excluding end
std::vector<SkTextBox> SkParagraphImpl::getRectsForRange(
    unsigned start,
    unsigned end,
    RectHeightStyle rectHeightStyle,
    RectWidthStyle rectWidthStyle) {

  std::vector<SkTextBox> results;
  if (start >= end) {
    return results;
  }

  // Calculate the utf8 substring
  const char* first = fUtf8.begin();
  for (unsigned i = 0; i < start; ++i) {
    utf8_next(&first, fUtf8.end());
  }
  const char* last = first;
  for (unsigned i = start; i < end; ++i) {
    utf8_next(&last, fUtf8.end());
  }
  SkSpan<const char> text(first, last - first);

  for (auto& line : fLines) {
    auto intersect = line.text() * text;
    if (intersect.empty() && (!line.text().empty() || line.text().begin() != text.begin())) {
      continue;
    }

    SkScalar runOffset = 0;
    if (line.text().begin() != intersect.begin()) {
      SkSpan<const char> before(line.text().begin(), intersect.begin() - line.text().begin());
      runOffset = line.iterateThroughRuns
          (before, 0, true, [](SkRun*, size_t, size_t, SkRect, SkScalar, bool) { return true; });
    }
    auto firstBox = results.size();
    line.iterateThroughRuns(
      intersect,
      runOffset,
      true,
      [&results,&line]
      (SkRun* run, size_t pos, size_t size, SkRect clip, SkScalar shift, bool clippingNeeded) {
        clip.offset(line.offset());
        results.emplace_back(clip, run->leftToRight() ? SkTextDirection::ltr : SkTextDirection::rtl);
        return true;
      });

    if (rectHeightStyle != RectHeightStyle::kTight) {
      // Align all the rectangles
      for (auto i = firstBox; i < results.size(); ++i) {
        auto& rect = results[i].rect;
        if (rectHeightStyle == RectHeightStyle::kMax) {
          rect.fTop = line.offset().fY + line.roundingDelta();
          rect.fBottom = line.offset().fY + line.height();

        } else if (rectHeightStyle == RectHeightStyle::kIncludeLineSpacingTop) {
          rect.fTop = line.offset().fY;

        } else if (rectHeightStyle == RectHeightStyle::kIncludeLineSpacingMiddle) {
          auto delta = (line.height() - rect.height()) / 2;
          rect.fTop = line.offset().fY + delta;
          rect.fBottom = line.offset().fY + line.height() - delta;

        } else if (rectHeightStyle == RectHeightStyle::kIncludeLineSpacingBottom) {
          rect.fBottom = line.offset().fY + line.height();
        }
      }
    } else {
      // Just leave the boxes the way they are
    }

    if (rectWidthStyle == RectWidthStyle::kMax) {
      for (auto& i = firstBox; i < results.size(); ++i) {
        auto clip = results[i].rect;
        auto dir = results[i].direction;
        if (clip.fLeft > line.offset().fX) {
          SkRect left = SkRect::MakeXYWH(0, clip.fTop, clip.fLeft -line.offset().fX, clip.fBottom);
          results.insert(results.begin() + i, { left, dir });
          ++i;
        }
        if (clip.fRight < line.offset().fX + line.width()) {
          SkRect right = SkRect::MakeXYWH(0, clip.fTop, line.offset().fX + line.width() - clip.fRight, clip.fBottom);
          results.insert(results.begin() + i, { right, dir });
          ++i;
        }
      }
    }
  }

  return results;
}
// TODO: Deal with RTL here
SkPositionWithAffinity SkParagraphImpl::getGlyphPositionAtCoordinate(SkScalar dx, SkScalar dy) {

  SkPositionWithAffinity result(0, Affinity::DOWNSTREAM);
  for (auto& line : fLines) {

    // This is so far the the line vertically closest to our coordinates
    // (or the first one, or the only one - all the same)
    line.iterateThroughRuns(
      line.text(),
      0,
      true,
      [&]
      (SkRun* run, size_t pos, size_t size, SkRect clip, SkScalar shift, bool clippingNeeded) {

        if (dx < clip.fLeft) {
          // All the other runs are placed right of this one
          result = { SkToS32(run->fClusterIndexes[pos]), DOWNSTREAM };
          return false;
        }

        if (dx >= clip.fRight) {
          // We have to keep looking but just in case keep the last one as the closes so far
          result = { SkToS32(run->fClusterIndexes[pos + size]), UPSTREAM };
          return true;
        }

        // So we found the run that contains our coordinates
        size_t found = pos;
        for (size_t i = pos; i < pos + size; ++i) {
          if (run->positionX(i) + shift > dx) {
            break;
          }
          found = i;
        }

        if (found == pos) {
          result = { SkToS32(run->fClusterIndexes[found]), DOWNSTREAM };
        } else if (found == pos + size - 1) {
          result = { SkToS32(run->fClusterIndexes[found]), UPSTREAM };
        } else {
          auto center = (run->positionX(found + 1) + run->positionX(found)) / 2;
          if ((dx <= center + shift) == run->leftToRight()) {
            result = { SkToS32(run->fClusterIndexes[found]), DOWNSTREAM };
          } else {
            result = { SkToS32(run->fClusterIndexes[found + 1]), UPSTREAM };
          }
        }
        // No need to continue
        return false;
    });

    // Let's figure out if we can stop looking
    auto offsetY = line.offset().fY;
    if (dy < offsetY) {
      // The closest position on this line; next line is going to be even lower
      break;
    }
    if (dy >= offsetY + line.height()) {
      // We have the closest position on the lowest line so far, but we have to continue
      continue;
    }

    // We hit the line; nothing else to do
    break;
  }

  //SkDebugf("getGlyphPositionAtCoordinate(%f,%f) = %d\n", dx, dy, result.position);
  return result;
}

// Finds the first and last glyphs that define a word containing
// the glyph at index offset.
// By "glyph" they mean a character index - indicated by Minikin's code
SkRange<size_t> SkParagraphImpl::getWordBoundary(unsigned offset) {

  SkTextBreaker breaker;
  if (!breaker.initialize(fUtf8, UBRK_WORD)) {
    return {0, 0};
  }

  size_t currentPos = breaker.first();
  while (true) {
    auto start = currentPos;
    currentPos = breaker.next();
    if (breaker.eof()) {
      break;
    }
    if (start <= offset && currentPos > offset) {
      return {start, currentPos};
    }
  }
  return {0, 0};
}

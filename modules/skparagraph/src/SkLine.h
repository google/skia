/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "SkDartTypes.h"
#include "SkSpan.h"
#include "SkTArray.h"
#include "SkRun.h"

class SkWord {

 public:

  SkWord() { }

  SkWord(SkSpan<const char> text)
      : fText(text)
      , fShift(0)
      , fAdvance(SkVector::Make(0, 0)) {
    setIsWhiteSpaces();
  }

  inline SkSpan<const char> text() const { return fText; }
  inline SkVector advance() const { return fAdvance; }
  inline SkScalar offset() const { return fShift; }
  inline void shift(SkScalar shift) { fShift += shift; }
  inline void expand(SkScalar step) { fAdvance.fX += step; }
  inline bool empty() const { return fText.empty(); }
  inline bool isWhiteSpace() const { return fWhiteSpaces; }

 private:

  friend class SkParagraphImpl;

  void setIsWhiteSpaces() {
    fWhiteSpaces = false;
    auto pos = fText.end();
    while (--pos >= fText.begin()) {
      auto ch = *pos;
      if (!u_isspace(ch) &&
          u_charType(ch) != U_CONTROL_CHAR &&
          u_charType(ch) != U_NON_SPACING_MARK) {
        return;
      }
    }
    fWhiteSpaces = true;
  }

  SkSpan<const char> fText;
  SkScalar fShift;    // For justification
  SkVector fAdvance;  // Size
  bool fWhiteSpaces;
};

class SkLine {

 public:

  SkLine() { }

  SkLine(SkVector offset, SkVector advance, SkSpan<const char> text, SkRun* ellipsis, SkFontSizes sizes)
      : fText(text)
      , fShift(0)
      , fAdvance(advance)
      , fOffset(offset)
      , fEllipsis(ellipsis)
      , fSizes(sizes) { }

  inline SkSpan<const char> text() const { return fText; }
  inline SkVector advance() const { return fAdvance; }
  inline SkVector offset() const { return fOffset + SkVector::Make(fShift, 0); }
  inline SkRun* ellipsis() const { return fEllipsis; }
  inline SkFontSizes sizes() const { return fSizes; }
  inline bool empty() const { return fText.empty(); }
  void breakLineByWords(UBreakIteratorType type, std::function<void(SkWord& word)> apply);

 private:

  friend class SkParagraphImpl;

  SkSpan<const char> fText;
  SkTArray<SkWord, true> fWords; // Text broken into words by ICU word breaker
  SkScalar fShift;    // Shift to left - right - center
  SkVector fAdvance;  // Text on the line size
  SkVector fOffset;   // Text position on the screen
  SkRun* fEllipsis;   // In case the line ends with the ellipsis
  SkFontSizes fSizes;
};


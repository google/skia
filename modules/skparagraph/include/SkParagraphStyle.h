/*
 * Copyright 2019 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string>

#include "include/core/SkFontStyle.h"
#include "SkDartTypes.h"
#include "SkTextStyle.h"

struct SkStrutStyle {

  SkStrutStyle();
  SkFontStyle fFontStyle;
  std::vector<std::string> fFontFamilies;
  SkScalar fFontSize;
  SkScalar fHeight;
  SkScalar fLeading;
  bool fForceStrutHeight;
  bool fStrutEnabled;
};

struct SkParagraphStyle {
  SkParagraphStyle();

  bool operator==(const SkParagraphStyle& rhs) const {

    return this->fHeight == rhs.fHeight &&
        this->fEllipsis == rhs.fEllipsis &&
        this->fTextDirection == rhs.fTextDirection &&
        this->fTextAlign == rhs.fTextAlign &&
        this->fDefaultTextStyle == rhs.fDefaultTextStyle;
  }

  SkStrutStyle& getStrutStyle() { return fStrutStyle; }
  SkTextStyle& getTextStyle() { return fDefaultTextStyle; }
  inline size_t getMaxLines() const { return fLinesLimit; }
  inline SkTextDirection getTextDirection() const { return fTextDirection; }
  inline std::string getEllipsis() const { return fEllipsis; }

  void setStrutStyle(SkStrutStyle strutStyle) { fStrutStyle = std::move(strutStyle); }
  void setTextStyle(const SkTextStyle& textStyle) { fDefaultTextStyle = textStyle; }
  void setTextAlign(SkTextAlign align) { fTextAlign = align; }
  SkTextAlign getTextAlign() const { return fTextAlign; }
  void setTextDirection(SkTextDirection direction) {

    fTextDirection = direction;
  }
  void setMaxLines(size_t maxLines) { fLinesLimit = maxLines; }
  void setEllipsis(const std::u16string& ellipsis);
  void setHeight(SkScalar height) { fHeight = height; }
  SkScalar getHeight() const { return fHeight; }

  inline bool unlimited_lines() const { return fLinesLimit == std::numeric_limits<size_t>::max(); }
  inline bool ellipsized() const { return !fEllipsis.empty(); }
  SkTextAlign effective_align() const;

  bool hintingIsOn() const { return fHintingIsOn; }
  void turnHintingOff() { fHintingIsOn = false; }

 private:
  SkStrutStyle fStrutStyle;
  SkTextStyle fDefaultTextStyle;
  SkTextAlign fTextAlign;
  SkTextDirection fTextDirection;
  size_t fLinesLimit;
  std::string fEllipsis;
  SkScalar fHeight;
  bool fHintingIsOn;
};


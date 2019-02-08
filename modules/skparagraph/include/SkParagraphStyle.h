/*
 * Copyright 2018 Google Inc.
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

#include <climits>
#include <string>

#include "SkDartTypes.h"
#include "SkFontStyle.h"
#include "SkTextStyle.h"

// TODO: Introduce line break strategy later
class SkParagraphStyle {
 public:

  SkParagraphStyle();

  bool operator==(const SkParagraphStyle& rhs) const {
    return this->_line_height == rhs._line_height &&
           this->_ellipsis == rhs._ellipsis &&
           this->_textDirection == rhs._textDirection &&
           this->_textAlign == rhs._textAlign &&
           this->_textStyle == rhs._textStyle;
  }

  SkTextStyle& getTextStyle() { return _textStyle; }
  size_t getMaxLines() const { return _maxLines; }
  SkTextDirection getTextDirection() const { return _textDirection; }
  std::string getEllipsis() const { return _ellipsis; }

  void setTextStyle(SkTextStyle textStyle) {
    _textStyle = textStyle;
  }
  void setTextAlign(SkTextAlign align) { _textAlign = align; }
  void setTextDirection(SkTextDirection direction) { _textDirection = direction; }
  void setMaxLines(size_t maxLines) { _maxLines = maxLines; }
  void setEllipsis(const std::u16string& ellipsis);
  void setLineHeight(SkScalar line_height) { _line_height = line_height; }

  bool unlimited_lines() const;
  bool ellipsized() const;
  SkTextAlign effective_align() const;

 private:

  SkTextStyle _textStyle;
  SkTextAlign _textAlign;
  SkTextDirection _textDirection;
  size_t _maxLines;
  std::string _ellipsis;
  SkScalar _line_height;
};

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
#include <list>

#include "SkParagraphBuilder.h"
#include "SkParagraphStyle.h"
#include "SkPaint.h"


SkParagraphBuilder::SkParagraphBuilder(
    SkParagraphStyle style,
    sk_sp<SkFontCollection> font_collection)
    : _fontCollection(std::move(font_collection)) {
  SetParagraphStyle(style);
}

SkParagraphBuilder::~SkParagraphBuilder() = default;

void SkParagraphBuilder::SetParagraphStyle(const SkParagraphStyle& style) {
  _style = style;
  auto& textStyle = _style.getTextStyle();
  _fontCollection->findTypeface(textStyle);
  _styles.push(textStyle);
  _runs.emplace_back(_text.size(), _text.size(), textStyle);
}

void SkParagraphBuilder::PushStyle(const SkTextStyle& style) {
  EndRunIfNeeded();

  _styles.push(style);
  if (!_runs.empty() && _runs.back().end == _text.size() && _runs.back().textStyle == style) {
    // Just continue with the same style
  } else {
    // Resolve the new style and go with it
    auto& textStyle = _styles.top();
    _fontCollection->findTypeface(textStyle);
    _runs.emplace_back(_text.size(), _text.size(), textStyle);
  }
}

void SkParagraphBuilder::Pop() {

  EndRunIfNeeded();
  if (_styles.size() > 1) {
    _styles.pop();
  } else {
    // In this case we use paragraph style and skip Pop operation
    SkDebugf("SkParagraphBuilder.Pop() called too many times.\n");
  }

  auto top = _styles.top();
  _runs.emplace_back(_text.size(), _text.size(), top);
}

SkTextStyle SkParagraphBuilder::PeekStyle() {

  EndRunIfNeeded();
  if (!_styles.empty()) {
    return _styles.top();
  } else {
    SkDebugf("SkParagraphBuilder._styles is empty.\n");
    return _style.getTextStyle();
  }
}

void SkParagraphBuilder::AddText(const std::u16string& text) {

  icu::UnicodeString unicode;
  unicode.setTo((UChar*)text.data());
  unicode.toUTF8String(_text);
}

void SkParagraphBuilder::AddText(const std::string& text) {

  icu::UnicodeString unicode;
  unicode.setTo(text.data());
  unicode.toUTF8String(_text);
}

void SkParagraphBuilder::AddText(const char* text) {
  icu::UnicodeString unicode;
  unicode.setTo(text);
  unicode.toUTF8String(_text);
}

void SkParagraphBuilder::EndRunIfNeeded() {
  if (_runs.empty()) {
    return;
  }

  auto& last = _runs.back();
  if (last.start == _text.size()) {
    _runs.pop_back();
  } else {
    last.end = _text.size();
  }
}

std::unique_ptr<SkParagraph> SkParagraphBuilder::Build() {
  EndRunIfNeeded();

  std::unique_ptr<SkParagraph> paragraph = std::make_unique<SkParagraph>();

  paragraph->SetText(_text);
  paragraph->SetRuns(std::move(_runs));
  paragraph->SetParagraphStyle(_style);

  return paragraph;
}


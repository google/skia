/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGText.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkString.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

SkSVGText::SkSVGText() : INHERITED(SkSVGTag::kText) {}

void SkSVGText::setX(const SkSVGLength& x) { fX = x; }

void SkSVGText::setY(const SkSVGLength& y) { fY = y; }

void SkSVGText::setFontFamily(const SkSVGStringType& font_family) {
  if (fFontFamily != font_family) {
    fFontFamily = font_family;

    this->loadFont();
  }
}

void SkSVGText::loadFont() {
  SkFontStyle style;
  if (fFontWeight.equals("bold")) {
    if (fFontStyle.equals("italic")) {
      style = SkFontStyle::BoldItalic();
    } else {
      style = SkFontStyle::Bold();
    }
  } else if (fFontStyle.equals("italic")) {
    style = SkFontStyle::Italic();
  }

  fTypeface = SkTypeface::MakeFromName(fFontFamily.c_str(), style);
}

void SkSVGText::setFontSize(const SkSVGLength& size) { fFontSize = size; }

void SkSVGText::setFontStyle(const SkSVGStringType& font_style) {
  if (fFontStyle != font_style) {
    fFontStyle = font_style;

    this->loadFont();
  }
}

void SkSVGText::setFontWeight(const SkSVGStringType& font_weight) {
  if (fFontWeight != font_weight) {
    fFontWeight = font_weight;

    this->loadFont();
  }
}

void SkSVGText::setText(const SkSVGStringType& text) { fText = text; }

void SkSVGText::setTextAnchor(const SkSVGStringType& text_anchor) {
  if (strcmp(text_anchor.c_str(), "start") == 0) {
    fTextAlign = SkTextUtils::Align::kLeft_Align;
  } else if (strcmp(text_anchor.c_str(), "middle") == 0) {
    fTextAlign = SkTextUtils::Align::kCenter_Align;
  } else if (strcmp(text_anchor.c_str(), "end") == 0) {
    fTextAlign = SkTextUtils::Align::kRight_Align;
  }
}

void SkSVGText::onDraw(SkCanvas* canvas, const SkSVGLengthContext& lctx,
                       const SkPaint& paint, SkPathFillType) const {
  SkFont font(fTypeface, fFontSize.value());
  SkTextUtils::DrawString(canvas, fText.c_str(), fX.value(), fY.value(),
                          font, paint, fTextAlign);
}

SkPath SkSVGText::onAsPath(const SkSVGRenderContext& ctx) const {
  SkPath path;
  return path;
}

void SkSVGText::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
  switch (attr) {
    case SkSVGAttribute::kX:
      if (const auto* x = v.as<SkSVGLengthValue>()) {
        this->setX(*x);
      }
      break;
    case SkSVGAttribute::kY:
      if (const auto* y = v.as<SkSVGLengthValue>()) {
        this->setY(*y);
      }
      break;
    case SkSVGAttribute::kText:
      if (const auto* text = v.as<SkSVGStringValue>()) {
        this->setText(*text);
      }
      break;
    case SkSVGAttribute::kTextAnchor:
      if (const auto* text_anchor = v.as<SkSVGStringValue>()) {
        this->setTextAnchor(*text_anchor);
      }
      break;
    case SkSVGAttribute::kFontFamily:
      if (const auto* font_family = v.as<SkSVGStringValue>()) {
        this->setFontFamily(*font_family);
      }
      break;
    case SkSVGAttribute::kFontWeight:
      if (const auto* font_weight = v.as<SkSVGStringValue>()) {
        this->setFontWeight(*font_weight);
      }
      break;
    case SkSVGAttribute::kFontSize:
      if (const auto* font_size = v.as<SkSVGLengthValue>()) {
        this->setFontSize(*font_size);
      }
      break;
    case SkSVGAttribute::kFontStyle:
      if (const auto* font_style = v.as<SkSVGStringValue>()) {
        this->setFontStyle(*font_style);
      }
      break;
    default:
      this->INHERITED::onSetAttribute(attr, v);
  }
}

/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGText.h"

#include "experimental/svg/model/SkSVGRenderContext.h"
#include "experimental/svg/model/SkSVGValue.h"
#include "include/core/SkCanvas.h"

SkSVGText::SkSVGText() : INHERITED(SkSVGTag::kText) {}

void SkSVGText::setX(const SkSVGLength& x) { fX = x; }

void SkSVGText::setY(const SkSVGLength& y) { fY = y; }

void SkSVGText::setFontFamily(const SkSVGStringType& font_family) {
  fTypeface =
      SkTypeface::MakeFromName(font_family.value().c_str(), SkFontStyle());
}

void SkSVGText::setFontSize(const SkSVGLength& size) { fFontSize = size; }

void SkSVGText::setText(const SkSVGStringType& text) { fText = text; }

void SkSVGText::setTextAnchor(const SkSVGStringType& text_anchor) {
  if (strcmp(text_anchor.value().c_str(), "start") == 0) {
    fTextAlign = SkTextUtils::Align::kLeft_Align;
  } else if (strcmp(text_anchor.value().c_str(), "middle") == 0) {
    fTextAlign = SkTextUtils::Align::kCenter_Align;
  } else if (strcmp(text_anchor.value().c_str(), "end") == 0) {
    fTextAlign = SkTextUtils::Align::kRight_Align;
  }
}

void SkSVGText::onDraw(SkCanvas* canvas, const SkSVGLengthContext& lctx,
                       const SkPaint& paint, SkPathFillType) const {
  SkFont font(fTypeface, fFontSize.value());
  SkTextUtils::DrawString(canvas, fText.value().c_str(), fX.value(), fY.value(),
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
    case SkSVGAttribute::kFontSize:
      if (const auto* font_size = v.as<SkSVGLengthValue>()) {
        this->setFontSize(*font_size);
      }
      break;
    default:
      this->INHERITED::onSetAttribute(attr, v);
  }
}

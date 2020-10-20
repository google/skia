/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGText.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkString.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"

SkSVGText::SkSVGText() : INHERITED(SkSVGTag::kText) {}

void SkSVGText::setX(const SkSVGLength& x) { fX = x; }

void SkSVGText::setY(const SkSVGLength& y) { fY = y; }

//void SkSVGText::setFontFamily(const SkSVGStringType& font_family) {
//  if (fFontFamily != font_family) {
//    fFontFamily = font_family;

//    this->loadFont();
//  }
//}

//void SkSVGText::loadFont() {
//  SkFontStyle style;
//  if (fFontWeight.equals("bold")) {
//    if (fFontStyle.equals("italic")) {
//      style = SkFontStyle::BoldItalic();
//    } else {
//      style = SkFontStyle::Bold();
//    }
//  } else if (fFontStyle.equals("italic")) {
//    style = SkFontStyle::Italic();
//  }

//  fTypeface = SkTypeface::MakeFromName(fFontFamily.c_str(), style);
//}

//void SkSVGText::setFontSize(const SkSVGLength& size) { fFontSize = size; }

//void SkSVGText::setFontStyle(const SkSVGStringType& font_style) {
//  if (fFontStyle != font_style) {
//    fFontStyle = font_style;

//    this->loadFont();
//  }
//}

//void SkSVGText::setFontWeight(const SkSVGStringType& font_weight) {
//  if (fFontWeight != font_weight) {
//    fFontWeight = font_weight;

//    this->loadFont();
//  }
//}

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

void SkSVGText::onRender(const SkSVGRenderContext& ctx) const {
    auto weight = [](const SkSVGFontWeight& w) {
        switch (w.type()) {
            case SkSVGFontWeight::Type::k100:     return SkFontStyle::kThin_Weight;
            case SkSVGFontWeight::Type::k200:     return SkFontStyle::kExtraLight_Weight;
            case SkSVGFontWeight::Type::k300:     return SkFontStyle::kLight_Weight;
            case SkSVGFontWeight::Type::k400:     return SkFontStyle::kNormal_Weight;
            case SkSVGFontWeight::Type::k500:     return SkFontStyle::kMedium_Weight;
            case SkSVGFontWeight::Type::k600:     return SkFontStyle::kSemiBold_Weight;
            case SkSVGFontWeight::Type::k700:     return SkFontStyle::kBold_Weight;
            case SkSVGFontWeight::Type::k800:     return SkFontStyle::kExtraBold_Weight;
            case SkSVGFontWeight::Type::k900:     return SkFontStyle::kBlack_Weight;
            case SkSVGFontWeight::Type::kNormal:  return SkFontStyle::kNormal_Weight;
            case SkSVGFontWeight::Type::kBold:    return SkFontStyle::kBold_Weight;
            case SkSVGFontWeight::Type::kBolder:  return SkFontStyle::kExtraBold_Weight;
            case SkSVGFontWeight::Type::kLighter: return SkFontStyle::kLight_Weight;
            case SkSVGFontWeight::Type::kInherit: {
                // TODO: SkASSERT(false);
                return SkFontStyle::kNormal_Weight;
            }
        }
    };

    auto slant = [](const SkSVGFontStyle& s) {
        switch (s.type()) {
            case SkSVGFontStyle::Type::kNormal:  return SkFontStyle::kUpright_Slant;
            case SkSVGFontStyle::Type::kItalic:  return SkFontStyle::kItalic_Slant;
            case SkSVGFontStyle::Type::kOblique: return SkFontStyle::kOblique_Slant;
            case SkSVGFontStyle::Type::kInherit: {
                // TODO: SkASSERT(false);
                return SkFontStyle::kUpright_Slant;
            }
        }
    };


    const auto& family = ctx.presentationContext().fInherited.fFontFamily->family();
    const SkFontStyle style(weight(*ctx.presentationContext().fInherited.fFontWeight),
                            SkFontStyle::kNormal_Width,
                            slant(*ctx.presentationContext().fInherited.fFontStyle));

    sk_sp<SkTypeface> tf(SkFontMgr::RefDefault()->matchFamilyStyle(family.c_str(), style));

    const auto size =
            ctx.lengthContext().resolve(ctx.presentationContext().fInherited.fFontSize->size(),
                                        SkSVGLengthContext::LengthType::kHorizontal);
    SkFont font(std::move(tf), size);
    font.setSubpixel(true);
    font.setLinearMetrics(true);
    font.setEdging(SkFont::Edging::kAntiAlias);

    printf("*** fam: %s, sz: %f, weight: %d, txt: >%s<\n",
           family.c_str(), size, style.weight(), fText.c_str());

    if (const SkPaint* fillPaint = ctx.fillPaint()) {
        SkTextUtils::DrawString(ctx.canvas(), fText.c_str(), fX.value(), fY.value(), font,
                                *fillPaint);
    }

    if (const SkPaint* strokePaint = ctx.strokePaint()) {
        SkTextUtils::DrawString(ctx.canvas(), fText.c_str(), fX.value(), fY.value(), font,
                                *strokePaint);
    }
}

void SkSVGText::appendChild(sk_sp<SkSVGNode>) {
    // TODO
}

//void SkSVGText::onDraw(SkCanvas* canvas, const SkSVGLengthContext& lctx,
//                       const SkPaint& paint, SkPathFillType) const {
//  SkFont font(fTypeface, fFontSize.value());
//  SkTextUtils::DrawString(canvas, fText.c_str(), fX.value(), fY.value(),
//                          font, paint, fTextAlign);
//}

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
      break;
    default:
      this->INHERITED::onSetAttribute(attr, v);
  }
}

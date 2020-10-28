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

SkFont SkSVGText::resolveFont(const SkSVGRenderContext& ctx) const {
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
                SkASSERT(false);
                return SkFontStyle::kNormal_Weight;
            }
        }
        SkUNREACHABLE;
    };

    auto slant = [](const SkSVGFontStyle& s) {
        switch (s.type()) {
            case SkSVGFontStyle::Type::kNormal:  return SkFontStyle::kUpright_Slant;
            case SkSVGFontStyle::Type::kItalic:  return SkFontStyle::kItalic_Slant;
            case SkSVGFontStyle::Type::kOblique: return SkFontStyle::kOblique_Slant;
            case SkSVGFontStyle::Type::kInherit: {
                SkASSERT(false);
                return SkFontStyle::kUpright_Slant;
            }
        }
        SkUNREACHABLE;
    };

    const auto& family = ctx.presentationContext().fInherited.fFontFamily->family();
    const SkFontStyle style(weight(*ctx.presentationContext().fInherited.fFontWeight),
                            SkFontStyle::kNormal_Width,
                            slant(*ctx.presentationContext().fInherited.fFontStyle));

    const auto size =
            ctx.lengthContext().resolve(ctx.presentationContext().fInherited.fFontSize->size(),
                                        SkSVGLengthContext::LengthType::kVertical);

    // TODO: allow clients to pass an external fontmgr.
    SkFont font(SkTypeface::MakeFromName(family.c_str(), style), size);
    font.setHinting(SkFontHinting::kNone);
    font.setSubpixel(true);
    font.setLinearMetrics(true);
    font.setBaselineSnap(false);
    font.setEdging(SkFont::Edging::kAntiAlias);

    return font;
}

void SkSVGText::onRender(const SkSVGRenderContext& ctx) const {
    const auto font = this->resolveFont(ctx);

    const auto text_align = [](const SkSVGTextAnchor& anchor) {
        switch (anchor.type()) {
            case SkSVGTextAnchor::Type::kStart : return SkTextUtils::Align::kLeft_Align;
            case SkSVGTextAnchor::Type::kMiddle: return SkTextUtils::Align::kCenter_Align;
            case SkSVGTextAnchor::Type::kEnd   : return SkTextUtils::Align::kRight_Align;
            case SkSVGTextAnchor::Type::kInherit:
                SkASSERT(false);
                return SkTextUtils::Align::kLeft_Align;
        }
        SkUNREACHABLE;
    };

    const auto align = text_align(*ctx.presentationContext().fInherited.fTextAnchor);
    if (const SkPaint* fillPaint = ctx.fillPaint()) {
        SkTextUtils::DrawString(ctx.canvas(), fText.c_str(), fX.value(), fY.value(), font,
                                *fillPaint, align);
    }

    if (const SkPaint* strokePaint = ctx.strokePaint()) {
        SkTextUtils::DrawString(ctx.canvas(), fText.c_str(), fX.value(), fY.value(), font,
                                *strokePaint, align);
    }
}

void SkSVGText::appendChild(sk_sp<SkSVGNode>) {
    // TODO
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
    default:
      this->INHERITED::onSetAttribute(attr, v);
  }
}

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

namespace {

static SkFont ResolveFont(const SkSVGRenderContext& ctx) {
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

    // TODO: we likely want matchFamilyStyle here, but switching away from legacyMakeTypeface
    // changes all the results when using the default fontmgr.
    auto tf = ctx.fontMgr()->legacyMakeTypeface(family.c_str(), style);

    SkFont font(std::move(tf), size);
    font.setHinting(SkFontHinting::kNone);
    font.setSubpixel(true);
    font.setLinearMetrics(true);
    font.setBaselineSnap(false);
    font.setEdging(SkFont::Edging::kAntiAlias);

    return font;
}

static sk_sp<SkSVGTextFragment> AsTextFragment(sk_sp<SkSVGNode> node) {
    switch (node->tag()) {
    case SkSVGTag::kText:
    case SkSVGTag::kTextLiteral:
    case SkSVGTag::kTSpan:
        return sk_sp<SkSVGTextFragment>(static_cast<SkSVGTextFragment*>(node.release()));
    default:
        return nullptr;
    }

    SkUNREACHABLE;
}

} // namespace

void SkSVGTextFragment::renderText(const SkSVGRenderContext& ctx, TextContext* tctx) const {
    SkSVGRenderContext localContext(ctx, this);

    if (this->onPrepareToRender(&localContext)) {
        this->onRenderText(localContext, tctx);
    }
}

void SkSVGTextLiteral::onRender(const SkSVGRenderContext&) const {
    // Text literals are only rendered via container calls.
}

void SkSVGTextLiteral::onRenderText(const SkSVGRenderContext& ctx, TextContext* tctx) const {
    const auto font = ResolveFont(ctx);

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
        SkTextUtils::DrawString(ctx.canvas(), fText.c_str(),
                                tctx->currentPos.x, tctx->currentPos.y,
                                font, *fillPaint, align);
    }

    if (const SkPaint* strokePaint = ctx.strokePaint()) {
        SkTextUtils::DrawString(ctx.canvas(), fText.c_str(),
                                tctx->currentPos.x, tctx->currentPos.y,
                                font, *strokePaint, align);
    }
}

SkPath SkSVGTextLiteral::onAsPath(const SkSVGRenderContext& ctx) const {
    // TODO
    return SkPath();
}

void SkSVGTextContainer::onRender(const SkSVGRenderContext& ctx) const {
    TextContext tctx {
        {
            ctx.lengthContext().resolve(fX, SkSVGLengthContext::LengthType::kHorizontal),
            ctx.lengthContext().resolve(fY, SkSVGLengthContext::LengthType::kVertical),
        },
        0
    };

    // Note: we're dispatching to onRenderText() (as opposed to renderText()) because styling has
    // already been applied by render() upstack.
    return this->onRenderText(ctx, &tctx);
}

void SkSVGTextContainer::onRenderText(const SkSVGRenderContext& ctx, TextContext* tctx) const {
    for (const auto& frag : fFragments) {
        frag->renderText(ctx, tctx);
    }
}

void SkSVGTextContainer::appendChild(sk_sp<SkSVGNode> child) {
    if (auto frag = AsTextFragment(std::move(child))) {
        fFragments.push_back(std::move(frag));
    }
}

SkPath SkSVGTextContainer::onAsPath(const SkSVGRenderContext& ctx) const {
    // TODO
    return SkPath();
}

bool SkSVGTextContainer::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setX(SkSVGAttributeParser::parse<SkSVGLength>("x", name, value)) ||
           this->setY(SkSVGAttributeParser::parse<SkSVGLength>("y", name, value));
}

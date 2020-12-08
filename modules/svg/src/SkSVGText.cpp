/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGText.h"

#include <vector>

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkString.h"
#include "modules/skshaper/include/SkShaper.h"
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

static float ComputeAlignmentFactor(const SkSVGRenderContext& ctx) {
    switch (ctx.presentationContext().fInherited.fTextAnchor->type()) {
    case SkSVGTextAnchor::Type::kStart : return  0.0f;
    case SkSVGTextAnchor::Type::kMiddle: return -0.5f;
    case SkSVGTextAnchor::Type::kEnd   : return -1.0f;
    case SkSVGTextAnchor::Type::kInherit:
        SkASSERT(false);
        return 0.0f;
    }
    SkUNREACHABLE;
}

} // namespace

// SkSVGTextContext is responsible for sequencing input text chars into "chunks".
// A single text chunk can span multiple structural elements (<text>, <tspan>, etc),
// and per [1] new chunks are emitted
//
//   a) for each top level text element (<text>, <textPath>)
//   b) whenever a character with an explicit absolute position is encountered
//
// The implementation queues shaped run data until a full text chunk is resolved, at which
// point we have enough information to perform final alignment and rendering.
//
// [1] https://www.w3.org/TR/SVG11/text.html#TextLayoutIntroduction
class SkSVGTextContext final : SkShaper::RunHandler {
public:
    SkSVGTextContext(const SkSVGTextContainer& tcontainer, const SkSVGRenderContext& ctx)
        : fShaper(SkShaper::Make(ctx.fontMgr()))
        , fChunkPos{ ctx.lengthContext().resolve(tcontainer.getX(),
                                                 SkSVGLengthContext::LengthType::kHorizontal),
                     ctx.lengthContext().resolve(tcontainer.getY(),
                                                 SkSVGLengthContext::LengthType::kVertical)}
        , fChunkAlignmentFactor(ComputeAlignmentFactor(ctx))
    {}

    // Queues codepoints for rendering.
    void appendFragment(const SkString& txt, const SkSVGRenderContext& ctx) {
        // TODO: xml::space filtering
        // TODO: absolute positioned chars => chunk breaks

        // Stash paints for access from SkShaper callbacks.
        fCurrentFill   = ctx.fillPaint();
        fCurrentStroke = ctx.strokePaint();

        // TODO: directionality hints?
        const auto LTR = true;

        // Initiate shaping: this will generate a series of runs via callbacks.
        fShaper->shape(txt.c_str(), txt.size(), ResolveFont(ctx), LTR, SK_ScalarMax, this);
    }

    // Perform actual rendering for queued codepoints.
    void flushChunk(const SkSVGRenderContext& ctx) {
        // The final rendering offset is determined by cumulative chunk advances and alignment.
        const auto pos = fChunkPos + fChunkAdvance * fChunkAlignmentFactor;

        SkTextBlobBuilder blobBuilder;

        for (const auto& run : fRuns) {
            const auto& buf = blobBuilder.allocRunPos(run.font, SkToInt(run.glyphCount));
            std::copy(run.glyphs  .get(), run.glyphs  .get() + run.glyphCount, buf.glyphs);
            std::copy(run.glyphPos.get(), run.glyphPos.get() + run.glyphCount, buf.points());

            // Technically, blobs with compatible paints could be merged --
            // but likely not worth the effort.
            const auto blob = blobBuilder.make();
            if (run.fillPaint) {
                ctx.canvas()->drawTextBlob(blob, pos.fX, pos.fY, *run.fillPaint);
            }
            if (run.strokePaint) {
                ctx.canvas()->drawTextBlob(blob, pos.fX, pos.fY, *run.strokePaint);
            }
        }

        fChunkPos += fChunkAdvance;
        fChunkAdvance = {0,0};
        fChunkAlignmentFactor = ComputeAlignmentFactor(ctx);

        fRuns.clear();
    }

private:
    struct RunRec {
        SkFont                       font;
        std::unique_ptr<SkPaint>     fillPaint,
                                     strokePaint;
        std::unique_ptr<SkGlyphID[]> glyphs;
        std::unique_ptr<SkPoint[]>   glyphPos;
        size_t                       glyphCount;
        SkVector                     advance;
    };

    // SkShaper callbacks
    void beginLine() override {}
    void runInfo(const RunInfo&) override {}
    void commitRunInfo() override {}
    Buffer runBuffer(const RunInfo& ri) override {
        SkASSERT(ri.glyphCount);

        fRuns.push_back({
            ri.fFont,
            fCurrentFill   ? std::make_unique<SkPaint>(*fCurrentFill)   : nullptr,
            fCurrentStroke ? std::make_unique<SkPaint>(*fCurrentStroke) : nullptr,
            std::make_unique<SkGlyphID[]>(ri.glyphCount),
            std::make_unique<SkPoint[]  >(ri.glyphCount),
            ri.glyphCount,
            ri.fAdvance,
        });

        return {
            fRuns.back().glyphs.get(),
            fRuns.back().glyphPos.get(),
            nullptr,
            nullptr,
            fChunkAdvance,
        };
    }
    void commitRunBuffer(const RunInfo& ri) override {
        fChunkAdvance += ri.fAdvance;
    }
    void commitLine() override {}

    // http://www.w3.org/TR/SVG11/text.html#TextLayout
    const std::unique_ptr<SkShaper> fShaper;
    std::vector<RunRec>             fRuns;

    SkPoint                         fChunkPos;             // current text chunk position
    SkVector                        fChunkAdvance = {0,0}; // cumulative advance
    float                           fChunkAlignmentFactor; // current chunk alignment

    // cached for access from SkShaper callbacks.
    const SkPaint*                  fCurrentFill;
    const SkPaint*                  fCurrentStroke;
};

void SkSVGTextContainer::appendChild(sk_sp<SkSVGNode> child) {
    // Only allow text nodes.
    switch (child->tag()) {
    case SkSVGTag::kText:
    case SkSVGTag::kTextLiteral:
    case SkSVGTag::kTSpan:
        this->INHERITED::appendChild(child);
        break;
    default:
        break;
    }
}

bool SkSVGTextContainer::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setX(SkSVGAttributeParser::parse<SkSVGLength>("x", name, value)) ||
           this->setY(SkSVGAttributeParser::parse<SkSVGLength>("y", name, value));
}

void SkSVGText::onRender(const SkSVGRenderContext& ctx) const {
    // <text> establishes a new text layout context.
    SkSVGTextContext tctx(*this, ctx);

    SkSVGRenderContext local_ctx(ctx, tctx);
    this->INHERITED::onRender(local_ctx);

    tctx.flushChunk(ctx);
}

void SkSVGTextLiteral::onRender(const SkSVGRenderContext& ctx) const {
    auto* tctx = ctx.textContext();
    if (!tctx) {
        return;
    }

    tctx->appendFragment(this->getText(), ctx);
}

SkPath SkSVGTextLiteral::onAsPath(const SkSVGRenderContext&) const {
    // TODO
    return SkPath();
}

SkSVGTextLiteral::~SkSVGTextLiteral() = default; // just to pin the vtable

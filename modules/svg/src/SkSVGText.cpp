/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGText.h"

#include <limits>

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkString.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"
#include "src/utils/SkUTF.h"

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

static float ComputeAlignmentFactor(const SkSVGPresentationContext& pctx) {
    switch (pctx.fInherited.fTextAnchor->type()) {
    case SkSVGTextAnchor::Type::kStart : return  0.0f;
    case SkSVGTextAnchor::Type::kMiddle: return -0.5f;
    case SkSVGTextAnchor::Type::kEnd   : return -1.0f;
    case SkSVGTextAnchor::Type::kInherit:
        SkASSERT(false);
        return 0.0f;
    }
    SkUNREACHABLE;
}

// Helper for encoding optional positional attributes.
class PosAttrs {
public:
    // TODO: dx, dy, rotate
    enum Attr : size_t {
        kX = 0,
        kY = 1,
    };

    float  operator[](Attr a) const { return fStorage[a]; }
    float& operator[](Attr a)       { return fStorage[a]; }

    bool has(Attr a) const { return fStorage[a] != kNone; }
    bool hasAny()    const { return this->has(kX) || this->has(kY); }

private:
    static constexpr auto kNone = std::numeric_limits<float>::infinity();

    float fStorage[2] = { kNone, kNone };
};

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

    // Helper for cascading position attribute resolution (x, y, dx, dy, rotate) [1]:
    //   - each text position element can specify an arbitrary-length attribute array
    //   - for each character, we look up a given attribute first in its local attribute array,
    //     then in the ancestor chain (cascading/fallback) - and return the first value encountered.
    //   - the lookup is based on character index relative to the text content subtree
    //     (i.e. the index crosses chunk boundaries)
    //
    // [1] https://www.w3.org/TR/SVG11/text.html#TSpanElementXAttribute
    class ScopedPosResolver {
    public:
        ScopedPosResolver(const SkSVGTextContainer& txt, const SkSVGLengthContext& lctx,
                          SkSVGTextContext* tctx, size_t charIndexOffset)
            : fTextContext(tctx)
            , fParent(tctx->fPosResolver)
            , fCharIndexOffset(charIndexOffset)
            , fX(ResolveLengths(lctx, txt.getX(), SkSVGLengthContext::LengthType::kHorizontal))
            , fY(ResolveLengths(lctx, txt.getY(), SkSVGLengthContext::LengthType::kVertical))
        {
            fTextContext->fPosResolver = this;
        }

        ScopedPosResolver(const SkSVGTextContainer& txt, const SkSVGLengthContext& lctx,
                          SkSVGTextContext* tctx)
            : ScopedPosResolver(txt, lctx, tctx, tctx->fCurrentCharIndex) {}

        ~ScopedPosResolver() {
            fTextContext->fPosResolver = fParent;
        }

        PosAttrs resolve(size_t charIndex) const {
            PosAttrs attrs;

            if (charIndex < fLastPosIndex) {
                SkASSERT(charIndex >= fCharIndexOffset);
                const auto localCharIndex = charIndex - fCharIndexOffset;

                const auto hasAllLocal = localCharIndex < fX.size() &&
                                         localCharIndex < fY.size();
                if (!hasAllLocal && fParent) {
                    attrs = fParent->resolve(charIndex);
                }

                if (localCharIndex < fX.size()) {
                    attrs[PosAttrs::kX] = fX[localCharIndex];
                }
                if (localCharIndex < fY.size()) {
                    attrs[PosAttrs::kY] = fY[localCharIndex];
                }

                if (!attrs.hasAny()) {
                    // Once we stop producing explicit position data, there is no reason to
                    // continue trying for higher indices.  We can suppress future lookups.
                    fLastPosIndex = charIndex;
                }
            }

            return attrs;
        }

    private:
        static std::vector<float> ResolveLengths(const SkSVGLengthContext& lctx,
                                                 const std::vector<SkSVGLength>& lengths,
                                                 SkSVGLengthContext::LengthType lt) {
            std::vector<float> resolved;
            resolved.reserve(lengths.size());

            for (const auto& l : lengths) {
                resolved.push_back(lctx.resolve(l, lt));
            }

            return resolved;
        }

        SkSVGTextContext*        fTextContext;
        const ScopedPosResolver* fParent;          // parent resolver (fallback)
        const size_t             fCharIndexOffset; // start index for the current resolver
        const std::vector<float> fX,
                                 fY;

        // cache for the last known index with explicit positioning
        mutable size_t           fLastPosIndex = std::numeric_limits<size_t>::max();

    };

    SkSVGTextContext(const SkSVGPresentationContext& pctx, sk_sp<SkFontMgr> fmgr)
        : fShaper(SkShaper::Make(std::move(fmgr)))
        , fChunkPos{ 0, 0 }
        , fChunkAlignmentFactor(ComputeAlignmentFactor(pctx))
    {}

    // Queues codepoints for rendering.
    void appendFragment(const SkString& txt, const SkSVGRenderContext& ctx, SkSVGXmlSpace xs) {
        // https://www.w3.org/TR/SVG11/text.html#WhiteSpace
        // https://www.w3.org/TR/2008/REC-xml-20081126/#NT-S
        auto filterWSDefault = [this](SkUnichar ch) -> SkUnichar {
            // Remove all newline chars.
            if (ch == '\n') {
                return -1;
            }

            // Convert tab chars to space.
            if (ch == '\t') {
                ch = ' ';
            }

            // Consolidate contiguous space chars and strip leading spaces (fPrevCharSpace
            // starts off as true).
            if (fPrevCharSpace && ch == ' ') {
                return -1;
            }

            // TODO: Strip trailing WS?  Doing this across chunks would require another buffering
            //   layer.  In general, trailing WS should have no rendering side effects. Skipping
            //   for now.
            return ch;
        };
        auto filterWSPreserve = [](SkUnichar ch) -> SkUnichar {
            // Convert newline and tab chars to space.
            if (ch == '\n' || ch == '\t') {
                ch = ' ';
            }
            return ch;
        };

        // Stash paints for access from SkShaper callbacks.
        fCurrentFill   = ctx.fillPaint();
        fCurrentStroke = ctx.strokePaint();

        const auto font = ResolveFont(ctx);

        SkSTArray<128, char, true> filtered;
        filtered.reserve_back(SkToInt(txt.size()));

        auto shapePending = [&filtered, &font, this]() {
            // TODO: directionality hints?
            const auto LTR  = true;
            // Initiate shaping: this will generate a series of runs via callbacks.
            fShaper->shape(filtered.data(), filtered.size(), font, LTR, SK_ScalarMax, this);
            filtered.reset();
        };

        const char* ch_ptr = txt.c_str();
        const char* ch_end = ch_ptr + txt.size();

        while (ch_ptr < ch_end) {
            auto ch = SkUTF::NextUTF8(&ch_ptr, ch_end);
            ch = (xs == SkSVGXmlSpace::kDefault)
                    ? filterWSDefault(ch)
                    : filterWSPreserve(ch);

            if (ch < 0) {
                // invalid utf or char filtered out
                continue;
            }

            SkASSERT(fPosResolver);
            const auto pos = fPosResolver->resolve(fCurrentCharIndex++);

            // Absolute position adjustments define a new chunk.
            // (https://www.w3.org/TR/SVG11/text.html#TextLayoutIntroduction)
            if (pos.has(PosAttrs::kX) || pos.has(PosAttrs::kY)) {
                shapePending();
                this->flushChunk(ctx);

                // New chunk position.
                if (pos.has(PosAttrs::kX)) {
                    fChunkPos.fX = pos[PosAttrs::kX];
                }
                if (pos.has(PosAttrs::kY)) {
                    fChunkPos.fY = pos[PosAttrs::kY];
                }
            }

            char utf8_buf[SkUTF::kMaxBytesInUTF8Sequence];
            filtered.push_back_n(SkToInt(SkUTF::ToUTF8(ch, utf8_buf)), utf8_buf);

            fPrevCharSpace = (ch == ' ');
        }

        // Note: at this point we have shaped and buffered the current fragment  The active
        // text chunk continues until an explicit or implicit flush.
        shapePending();
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
        fChunkAlignmentFactor = ComputeAlignmentFactor(ctx.presentationContext());

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
    const ScopedPosResolver*        fPosResolver = nullptr;

    SkPoint                         fChunkPos;             // current text chunk position
    SkVector                        fChunkAdvance = {0,0}; // cumulative advance
    float                           fChunkAlignmentFactor; // current chunk alignment

    // tracks the global text subtree char index (cross chunks).  Used for position resolution.
    size_t                          fCurrentCharIndex = 0;

    // cached for access from SkShaper callbacks.
    const SkPaint*                  fCurrentFill;
    const SkPaint*                  fCurrentStroke;

    bool                            fPrevCharSpace = true; // WS filter state
};

void SkSVGTextFragment::renderText(const SkSVGRenderContext& ctx, SkSVGTextContext* tctx,
                                   SkSVGXmlSpace xs) const {
    SkSVGRenderContext localContext(ctx, this);

    if (this->onPrepareToRender(&localContext)) {
        this->onRenderText(localContext, tctx, xs);
    }
}

SkPath SkSVGTextFragment::onAsPath(const SkSVGRenderContext&) const {
    // TODO
    return SkPath();
}

void SkSVGTextContainer::appendChild(sk_sp<SkSVGNode> child) {
    // Only allow text nodes.
    switch (child->tag()) {
    case SkSVGTag::kText:
    case SkSVGTag::kTextLiteral:
    case SkSVGTag::kTSpan:
        fChildren.push_back(
            sk_sp<SkSVGTextFragment>(static_cast<SkSVGTextFragment*>(child.release())));
        break;
    default:
        break;
    }
}

void SkSVGTextContainer::onRenderText(const SkSVGRenderContext& ctx, SkSVGTextContext* tctx,
                                      SkSVGXmlSpace) const {
    const SkSVGTextContext::ScopedPosResolver resolver(*this, ctx.lengthContext(), tctx);

    for (const auto& frag : fChildren) {
        // Containers always override xml:space with the local value.
        frag->renderText(ctx, tctx, this->getXmlSpace());
    }
}

// https://www.w3.org/TR/SVG11/text.html#WhiteSpace
template <>
bool SkSVGAttributeParser::parse(SkSVGXmlSpace* xs) {
    static constexpr std::tuple<const char*, SkSVGXmlSpace> gXmlSpaceMap[] = {
            {"default" , SkSVGXmlSpace::kDefault },
            {"preserve", SkSVGXmlSpace::kPreserve},
    };

    return this->parseEnumMap(gXmlSpaceMap, xs) && this->parseEOSToken();
}

bool SkSVGTextContainer::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setX(SkSVGAttributeParser::parse<std::vector<SkSVGLength>>("x", name, value)) ||
           this->setY(SkSVGAttributeParser::parse<std::vector<SkSVGLength>>("y", name, value)) ||
           this->setXmlSpace(SkSVGAttributeParser::parse<SkSVGXmlSpace>("xml:space", name, value));
}

void SkSVGTextContainer::onRender(const SkSVGRenderContext& ctx) const {
    // Root text nodes establish a new text layout context.
    SkSVGTextContext tctx(ctx.presentationContext(), ctx.fontMgr());

    this->onRenderText(ctx, &tctx, this->getXmlSpace());

    tctx.flushChunk(ctx);
}

void SkSVGTextLiteral::onRenderText(const SkSVGRenderContext& ctx, SkSVGTextContext* tctx,
                                    SkSVGXmlSpace xs) const {
    SkASSERT(tctx);

    tctx->appendFragment(this->getText(), ctx, xs);
}

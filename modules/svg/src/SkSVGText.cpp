/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGText.h"

#include <limits>

#include "include/core/SkCanvas.h"
#include "include/core/SkContourMeasure.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkString.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGValue.h"
#include "modules/svg/src/SkSVGTextPriv.h"
#include "src/core/SkTextBlobPriv.h"
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

} // namespace

SkSVGTextContext::ScopedPosResolver::ScopedPosResolver(const SkSVGTextContainer& txt,
                                                       const SkSVGLengthContext& lctx,
                                                       SkSVGTextContext* tctx,
                                                       size_t charIndexOffset)
    : fTextContext(tctx)
    , fParent(tctx->fPosResolver)
    , fCharIndexOffset(charIndexOffset)
    , fX(ResolveLengths(lctx, txt.getX(), SkSVGLengthContext::LengthType::kHorizontal))
    , fY(ResolveLengths(lctx, txt.getY(), SkSVGLengthContext::LengthType::kVertical))
    , fDx(ResolveLengths(lctx, txt.getDx(), SkSVGLengthContext::LengthType::kHorizontal))
    , fDy(ResolveLengths(lctx, txt.getDy(), SkSVGLengthContext::LengthType::kVertical))
    , fRotate(txt.getRotate())
{
    fTextContext->fPosResolver = this;
}

SkSVGTextContext::ScopedPosResolver::ScopedPosResolver(const SkSVGTextContainer& txt,
                                                       const SkSVGLengthContext& lctx,
                                                       SkSVGTextContext* tctx)
    : ScopedPosResolver(txt, lctx, tctx, tctx->fCurrentCharIndex) {}

SkSVGTextContext::ScopedPosResolver::~ScopedPosResolver() {
    fTextContext->fPosResolver = fParent;
}

SkSVGTextContext::PosAttrs SkSVGTextContext::ScopedPosResolver::resolve(size_t charIndex) const {
    PosAttrs attrs;

    if (charIndex < fLastPosIndex) {
        SkASSERT(charIndex >= fCharIndexOffset);
        const auto localCharIndex = charIndex - fCharIndexOffset;

        const auto hasAllLocal = localCharIndex < fX.size() &&
                                 localCharIndex < fY.size() &&
                                 localCharIndex < fDx.size() &&
                                 localCharIndex < fDy.size() &&
                                 localCharIndex < fRotate.size();
        if (!hasAllLocal && fParent) {
            attrs = fParent->resolve(charIndex);
        }

        if (localCharIndex < fX.size()) {
            attrs[PosAttrs::kX] = fX[localCharIndex];
        }
        if (localCharIndex < fY.size()) {
            attrs[PosAttrs::kY] = fY[localCharIndex];
        }
        if (localCharIndex < fDx.size()) {
            attrs[PosAttrs::kDx] = fDx[localCharIndex];
        }
        if (localCharIndex < fDy.size()) {
            attrs[PosAttrs::kDy] = fDy[localCharIndex];
        }

        // Rotation semantics are interestingly different [1]:
        //
        //   - values are not cumulative
        //   - if explicit values are present at any level in the ancestor chain, those take
        //     precedence (closest ancestor)
        //   - last specified value applies to all remaining chars (closest ancestor)
        //   - these rules apply at node scope (not chunk scope)
        //
        // This means we need to discriminate between explicit rotation (rotate value provided for
        // current char) and implicit rotation (ancestor has some values - but not for the requested
        // char - we use the last specified value).
        //
        // [1] https://www.w3.org/TR/SVG11/text.html#TSpanElementRotateAttribute
        if (!fRotate.empty()) {
            if (localCharIndex < fRotate.size()) {
                // Explicit rotation value overrides anything in the ancestor chain.
                attrs[PosAttrs::kRotate] = fRotate[localCharIndex];
                attrs.setImplicitRotate(false);
            } else if (!attrs.has(PosAttrs::kRotate) || attrs.isImplicitRotate()){
                // Local implicit rotation (last specified value) overrides ancestor implicit
                // rotation.
                attrs[PosAttrs::kRotate] = fRotate.back();
                attrs.setImplicitRotate(true);
            }
        }

        if (!attrs.hasAny()) {
            // Once we stop producing explicit position data, there is no reason to
            // continue trying for higher indices.  We can suppress future lookups.
            fLastPosIndex = charIndex;
        }
    }

    return attrs;
}

void SkSVGTextContext::ShapeBuffer::append(SkUnichar ch, PositionAdjustment pos) {
    // relative pos adjustments are cumulative
    if (!fUtf8PosAdjust.empty()) {
        pos.offset += fUtf8PosAdjust.back().offset;
    }

    char utf8_buf[SkUTF::kMaxBytesInUTF8Sequence];
    const auto utf8_len = SkToInt(SkUTF::ToUTF8(ch, utf8_buf));
    fUtf8         .push_back_n(utf8_len, utf8_buf);
    fUtf8PosAdjust.push_back_n(utf8_len, pos);
}

void SkSVGTextContext::shapePendingBuffer(const SkFont& font) {
    // TODO: directionality hints?
    const auto LTR  = true;

    // Initiate shaping: this will generate a series of runs via callbacks.
    fShaper->shape(fShapeBuffer.fUtf8.data(), fShapeBuffer.fUtf8.size(),
                   font, LTR, SK_ScalarMax, this);
    fShapeBuffer.reset();
}

SkSVGTextContext::SkSVGTextContext(const SkSVGRenderContext& ctx, const ShapedTextCallback& cb,
                                   const SkSVGTextPath* tpath)
    : fRenderContext(ctx)
    , fCallback(cb)
    , fShaper(SkShaper::Make(ctx.fontMgr()))
    , fChunkAlignmentFactor(ComputeAlignmentFactor(ctx.presentationContext()))
{
    if (tpath) {
        fPathData = std::make_unique<PathData>(ctx, *tpath);

        // https://www.w3.org/TR/SVG11/text.html#TextPathElementStartOffsetAttribute
        auto resolve_offset = [this](const SkSVGLength& offset) {
            if (offset.unit() != SkSVGLength::Unit::kPercentage) {
                // "If a <length> other than a percentage is given, then the ‘startOffset’
                // represents a distance along the path measured in the current user coordinate
                // system."
                return fRenderContext.lengthContext()
                                     .resolve(offset, SkSVGLengthContext::LengthType::kHorizontal);
            }

            // "If a percentage is given, then the ‘startOffset’ represents a percentage distance
            // along the entire path."
            return offset.value() * fPathData->length() / 100;
        };

        // startOffset acts as an initial absolute position
        fChunkPos.fX = resolve_offset(tpath->getStartOffset());
    }
}

SkSVGTextContext::~SkSVGTextContext() {
    this->flushChunk(fRenderContext);
}

void SkSVGTextContext::shapeFragment(const SkString& txt, const SkSVGRenderContext& ctx,
                                     SkSVGXmlSpace xs) {
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
    fShapeBuffer.reserve(txt.size());

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
            this->shapePendingBuffer(font);
            this->flushChunk(ctx);

            // New chunk position.
            if (pos.has(PosAttrs::kX)) {
                fChunkPos.fX = pos[PosAttrs::kX];
            }
            if (pos.has(PosAttrs::kY)) {
                fChunkPos.fY = pos[PosAttrs::kY];
            }
        }

        fShapeBuffer.append(ch, {
            {
                pos.has(PosAttrs::kDx) ? pos[PosAttrs::kDx] : 0,
                pos.has(PosAttrs::kDy) ? pos[PosAttrs::kDy] : 0,
            },
            pos.has(PosAttrs::kRotate) ? SkDegreesToRadians(pos[PosAttrs::kRotate]) : 0,
        });

        fPrevCharSpace = (ch == ' ');
    }

    this->shapePendingBuffer(font);

    // Note: at this point we have shaped and buffered RunRecs for the current fragment.
    // The active text chunk continues until an explicit or implicit flush.
}

SkSVGTextContext::PathData::PathData(const SkSVGRenderContext& ctx, const SkSVGTextPath& tpath)
{
    const auto ref = ctx.findNodeById(tpath.getHref());
    if (!ref) {
        return;
    }

    SkContourMeasureIter cmi(ref->asPath(ctx), false);
    while (sk_sp<SkContourMeasure> contour = cmi.next()) {
        fLength += contour->length();
        fContours.push_back(std::move(contour));
    }
}

SkMatrix SkSVGTextContext::PathData::getMatrixAt(float offset) const {
    if (offset >= 0) {
        for (const auto& contour : fContours) {
            const auto contour_len = contour->length();
            if (offset < contour_len) {
                SkMatrix m;
                return contour->getMatrix(offset, &m) ? m : SkMatrix::I();
            }
            offset -= contour_len;
        }
    }

    // Quick & dirty way to "skip" rendering of glyphs off path.
    return SkMatrix::Translate(std::numeric_limits<float>::infinity(),
                               std::numeric_limits<float>::infinity());
}

SkRSXform SkSVGTextContext::computeGlyphXform(SkGlyphID glyph, const SkFont& font,
                                              const SkPoint& glyph_pos,
                                              const PositionAdjustment& pos_adjust) const {
    SkPoint pos = fChunkPos + glyph_pos + pos_adjust.offset + fChunkAdvance * fChunkAlignmentFactor;
    if (!fPathData) {
        return SkRSXform::MakeFromRadians(/*scale=*/ 1, pos_adjust.rotation, pos.fX, pos.fY, 0, 0);
    }

    // We're in a textPath scope, reposition the glyph on path.
    // (https://www.w3.org/TR/SVG11/text.html#TextpathLayoutRules)

    // Path positioning is based on the glyph center (horizontal component).
    float glyph_width;
    font.getWidths(&glyph, 1, &glyph_width);
    auto path_offset = pos.fX + glyph_width * .5f;

    // In addition to the path matrix, the final glyph matrix also includes:
    //
    //   -- vertical position adjustment "dy" ("dx" is factored into path_offset)
    //   -- glyph origin adjustment (undoing the glyph center offset above)
    //   -- explicit rotation adjustment (composing with the path glyph rotation)
    const auto m = fPathData->getMatrixAt(path_offset) *
            SkMatrix::Translate(-glyph_width * .5f, pos_adjust.offset.fY) *
            SkMatrix::RotateRad(pos_adjust.rotation);

    return SkRSXform::Make(m.getScaleX(), m.getSkewY(), m.getTranslateX(), m.getTranslateY());
}

void SkSVGTextContext::flushChunk(const SkSVGRenderContext& ctx) {
    SkTextBlobBuilder blobBuilder;

    for (const auto& run : fRuns) {
        const auto& buf = blobBuilder.allocRunRSXform(run.font, SkToInt(run.glyphCount));
        std::copy(run.glyphs.get(), run.glyphs.get() + run.glyphCount, buf.glyphs);
        for (size_t i = 0; i < run.glyphCount; ++i) {
            buf.xforms()[i] = this->computeGlyphXform(run.glyphs[i],
                                                      run.font,
                                                      run.glyphPos[i],
                                                      run.glyhPosAdjust[i]);
        }

        fCallback(ctx, blobBuilder.make(), run.fillPaint.get(), run.strokePaint.get());
    }

    fChunkPos += fChunkAdvance;
    fChunkAdvance = {0,0};
    fChunkAlignmentFactor = ComputeAlignmentFactor(ctx.presentationContext());

    fRuns.clear();
}

SkShaper::RunHandler::Buffer SkSVGTextContext::runBuffer(const RunInfo& ri) {
    SkASSERT(ri.glyphCount);

    fRuns.push_back({
        ri.fFont,
        fCurrentFill.isValid()   ? std::make_unique<SkPaint>(*fCurrentFill)   : nullptr,
        fCurrentStroke.isValid() ? std::make_unique<SkPaint>(*fCurrentStroke) : nullptr,
        std::make_unique<SkGlyphID[]         >(ri.glyphCount),
        std::make_unique<SkPoint[]           >(ri.glyphCount),
        std::make_unique<PositionAdjustment[]>(ri.glyphCount),
        ri.glyphCount,
        ri.fAdvance,
    });

    // Ensure sufficient space to temporarily fetch cluster information.
    fShapeClusterBuffer.resize(std::max(fShapeClusterBuffer.size(), ri.glyphCount));

    return {
        fRuns.back().glyphs.get(),
        fRuns.back().glyphPos.get(),
        nullptr,
        fShapeClusterBuffer.data(),
        fChunkAdvance,
    };
}

void SkSVGTextContext::commitRunBuffer(const RunInfo& ri) {
    const auto& current_run = fRuns.back();

    // stash position adjustments
    for (size_t i = 0; i < ri.glyphCount; ++i) {
        const auto utf8_index = fShapeClusterBuffer[i];
        current_run.glyhPosAdjust[i] = fShapeBuffer.fUtf8PosAdjust[SkToInt(utf8_index)];
    }

    // Offset adjustments are cumulative - we only need to advance the current chunk
    // with the last value.
    fChunkAdvance += ri.fAdvance + fShapeBuffer.fUtf8PosAdjust.back().offset;
}

void SkSVGTextFragment::renderText(const SkSVGRenderContext& ctx, SkSVGTextContext* tctx,
                                   SkSVGXmlSpace xs) const {
    // N.B.: unlike regular elements, text fragments do not establish a new OBB scope -- they
    // always defer to the root <text> element for OBB resolution.
    SkSVGRenderContext localContext(ctx);

    if (this->onPrepareToRender(&localContext)) {
        this->onShapeText(localContext, tctx, xs);
    }
}

SkPath SkSVGTextFragment::onAsPath(const SkSVGRenderContext&) const {
    // TODO
    return SkPath();
}

void SkSVGTextContainer::appendChild(sk_sp<SkSVGNode> child) {
    // Only allow text content child nodes.
    switch (child->tag()) {
    case SkSVGTag::kTextLiteral:
    case SkSVGTag::kTextPath:
    case SkSVGTag::kTSpan:
        fChildren.push_back(
            sk_sp<SkSVGTextFragment>(static_cast<SkSVGTextFragment*>(child.release())));
        break;
    default:
        break;
    }
}

void SkSVGTextContainer::onShapeText(const SkSVGRenderContext& ctx, SkSVGTextContext* tctx,
                                     SkSVGXmlSpace) const {
    SkASSERT(tctx);

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
           this->setDx(SkSVGAttributeParser::parse<std::vector<SkSVGLength>>("dx", name, value)) ||
           this->setDy(SkSVGAttributeParser::parse<std::vector<SkSVGLength>>("dy", name, value)) ||
           this->setRotate(SkSVGAttributeParser::parse<std::vector<SkSVGNumberType>>("rotate",
                                                                                     name,
                                                                                     value)) ||
           this->setXmlSpace(SkSVGAttributeParser::parse<SkSVGXmlSpace>("xml:space", name, value));
}

void SkSVGTextLiteral::onShapeText(const SkSVGRenderContext& ctx, SkSVGTextContext* tctx,
                                   SkSVGXmlSpace xs) const {
    SkASSERT(tctx);

    tctx->shapeFragment(this->getText(), ctx, xs);
}

void SkSVGText::onRender(const SkSVGRenderContext& ctx) const {
    const SkSVGTextContext::ShapedTextCallback render_text = [](const SkSVGRenderContext& ctx,
                                                                const sk_sp<SkTextBlob>& blob,
                                                                const SkPaint* fill,
                                                                const SkPaint* stroke) {
        if (fill) {
            ctx.canvas()->drawTextBlob(blob, 0, 0, *fill);
        }
        if (stroke) {
            ctx.canvas()->drawTextBlob(blob, 0, 0, *stroke);
        }
    };

    // Root <text> nodes establish a text layout context.
    SkSVGTextContext tctx(ctx, render_text);

    this->onShapeText(ctx, &tctx, this->getXmlSpace());
}

SkRect SkSVGText::onObjectBoundingBox(const SkSVGRenderContext& ctx) const {
    SkRect bounds = SkRect::MakeEmpty();

    const SkSVGTextContext::ShapedTextCallback compute_bounds =
        [&bounds](const SkSVGRenderContext& ctx, const sk_sp<SkTextBlob>& blob, const SkPaint*,
                  const SkPaint*) {
            if (!blob) {
                return;
            }

            SkAutoSTArray<64, SkRect> glyphBounds;

            for (SkTextBlobRunIterator it(blob.get()); !it.done(); it.next()) {
                glyphBounds.reset(SkToInt(it.glyphCount()));
                it.font().getBounds(it.glyphs(), it.glyphCount(), glyphBounds.get(), nullptr);

                SkASSERT(it.positioning() == SkTextBlobRunIterator::kRSXform_Positioning);
                SkMatrix m;
                for (uint32_t i = 0; i < it.glyphCount(); ++i) {
                    m.setRSXform(it.xforms()[i]);
                    bounds.join(m.mapRect(glyphBounds[i]));
                }
            }
        };

    {
        SkSVGTextContext tctx(ctx, compute_bounds);
        this->onShapeText(ctx, &tctx, this->getXmlSpace());
    }

    return bounds;
}

SkPath SkSVGText::onAsPath(const SkSVGRenderContext& ctx) const {
    SkPathBuilder builder;

    const SkSVGTextContext::ShapedTextCallback as_path =
        [&builder](const SkSVGRenderContext& ctx, const sk_sp<SkTextBlob>& blob, const SkPaint*,
                   const SkPaint*) {
            if (!blob) {
                return;
            }

            for (SkTextBlobRunIterator it(blob.get()); !it.done(); it.next()) {
                struct GetPathsCtx {
                    SkPathBuilder&   builder;
                    const SkRSXform* xform;
                } get_paths_ctx {builder, it.xforms()};

                it.font().getPaths(it.glyphs(), it.glyphCount(), [](const SkPath* path,
                                                                    const SkMatrix& matrix,
                                                                    void* raw_ctx) {
                    auto* get_paths_ctx = static_cast<GetPathsCtx*>(raw_ctx);
                    const auto& glyph_rsx = *get_paths_ctx->xform++;

                    if (!path) {
                        return;
                    }

                    SkMatrix glyph_matrix;
                    glyph_matrix.setRSXform(glyph_rsx);
                    glyph_matrix.preConcat(matrix);

                    get_paths_ctx->builder.addPath(path->makeTransform(glyph_matrix));
                }, &get_paths_ctx);
            }
        };

    {
        SkSVGTextContext tctx(ctx, as_path);
        this->onShapeText(ctx, &tctx, this->getXmlSpace());
    }

    auto path = builder.detach();
    this->mapToParent(&path);

    return path;
}

void SkSVGTextPath::onShapeText(const SkSVGRenderContext& ctx, SkSVGTextContext* parent_tctx,
                                 SkSVGXmlSpace xs) const {
    SkASSERT(parent_tctx);

    // textPath nodes establish a new text layout context.
    SkSVGTextContext tctx(ctx, parent_tctx->getCallback(), this);

    this->INHERITED::onShapeText(ctx, &tctx, xs);
}

bool SkSVGTextPath::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
        this->setHref(SkSVGAttributeParser::parse<SkSVGIRI>("xlink:href", name, value)) ||
        this->setStartOffset(SkSVGAttributeParser::parse<SkSVGLength>("startOffset", name, value));
}

/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGRenderContext.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPath.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/private/SkTo.h"
#include "modules/svg/include/SkSVGAttribute.h"
#include "modules/svg/include/SkSVGClipPath.h"
#include "modules/svg/include/SkSVGFilter.h"
#include "modules/svg/include/SkSVGMask.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGTypes.h"

namespace {

SkScalar length_size_for_type(const SkSize& viewport, SkSVGLengthContext::LengthType t) {
    switch (t) {
    case SkSVGLengthContext::LengthType::kHorizontal:
        return viewport.width();
    case SkSVGLengthContext::LengthType::kVertical:
        return viewport.height();
    case SkSVGLengthContext::LengthType::kOther:
        return SkScalarSqrt(viewport.width() * viewport.height());
    }

    SkASSERT(false);  // Not reached.
    return 0;
}

// Multipliers for DPI-relative units.
constexpr SkScalar kINMultiplier = 1.00f;
constexpr SkScalar kPTMultiplier = kINMultiplier / 72.272f;
constexpr SkScalar kPCMultiplier = kPTMultiplier * 12;
constexpr SkScalar kMMMultiplier = kINMultiplier / 25.4f;
constexpr SkScalar kCMMultiplier = kMMMultiplier * 10;

}  // namespace

SkScalar SkSVGLengthContext::resolve(const SkSVGLength& l, LengthType t) const {
    switch (l.unit()) {
    case SkSVGLength::Unit::kNumber:
        // Fall through.
    case SkSVGLength::Unit::kPX:
        return l.value();
    case SkSVGLength::Unit::kPercentage:
        return l.value() * length_size_for_type(fViewport, t) / 100;
    case SkSVGLength::Unit::kCM:
        return l.value() * fDPI * kCMMultiplier;
    case SkSVGLength::Unit::kMM:
        return l.value() * fDPI * kMMMultiplier;
    case SkSVGLength::Unit::kIN:
        return l.value() * fDPI * kINMultiplier;
    case SkSVGLength::Unit::kPT:
        return l.value() * fDPI * kPTMultiplier;
    case SkSVGLength::Unit::kPC:
        return l.value() * fDPI * kPCMultiplier;
    default:
        SkDebugf("unsupported unit type: <%d>\n", l.unit());
        return 0;
    }
}

SkRect SkSVGLengthContext::resolveRect(const SkSVGLength& x, const SkSVGLength& y,
                                       const SkSVGLength& w, const SkSVGLength& h) const {
    return SkRect::MakeXYWH(
        this->resolve(x, SkSVGLengthContext::LengthType::kHorizontal),
        this->resolve(y, SkSVGLengthContext::LengthType::kVertical),
        this->resolve(w, SkSVGLengthContext::LengthType::kHorizontal),
        this->resolve(h, SkSVGLengthContext::LengthType::kVertical));
}

namespace {

SkPaint::Cap toSkCap(const SkSVGLineCap& cap) {
    switch (cap) {
    case SkSVGLineCap::kButt:
        return SkPaint::kButt_Cap;
    case SkSVGLineCap::kRound:
        return SkPaint::kRound_Cap;
    case SkSVGLineCap::kSquare:
        return SkPaint::kSquare_Cap;
    }
    SkUNREACHABLE;
}

SkPaint::Join toSkJoin(const SkSVGLineJoin& join) {
    switch (join.type()) {
    case SkSVGLineJoin::Type::kMiter:
        return SkPaint::kMiter_Join;
    case SkSVGLineJoin::Type::kRound:
        return SkPaint::kRound_Join;
    case SkSVGLineJoin::Type::kBevel:
        return SkPaint::kBevel_Join;
    default:
        SkASSERT(false);
        return SkPaint::kMiter_Join;
    }
}

void applySvgPaint(const SkSVGRenderContext& ctx, const SkSVGPaint& svgPaint, SkPaint* p) {
    switch (svgPaint.type()) {
    case SkSVGPaint::Type::kColor:
        p->setColor(SkColorSetA(ctx.resolveSvgColor(svgPaint.color()), p->getAlpha()));
        break;
    case SkSVGPaint::Type::kIRI: {
        // Out property inheritance is borked as it follows the render path and not the tree
        // hierarchy.  To avoid gross transgressions like leaf node presentation attributes leaking
        // into the paint server context, use a pristine presentation context when following hrefs.
        SkSVGPresentationContext pctx;
        SkSVGRenderContext local_context(ctx.canvas(),
                                         ctx.fontMgr(),
                                         ctx.idMapper(),
                                         ctx.lengthContext(),
                                         pctx, ctx.node());

        const auto node = ctx.findNodeById(svgPaint.iri());
        if (!node || !node->asPaint(local_context, p)) {
            p->setColor(SK_ColorTRANSPARENT);
        }
        break;
    }
    case SkSVGPaint::Type::kNone:
        // Do nothing
        break;
    }
}

inline uint8_t opacity_to_alpha(SkScalar o) {
    return SkTo<uint8_t>(SkScalarRoundToInt(SkTPin<SkScalar>(o, 0, 1) * 255));
}

// Commit the selected attribute to the paint cache.
template <SkSVGAttribute>
void commitToPaint(const SkSVGPresentationAttributes&,
                   const SkSVGRenderContext&,
                   SkSVGPresentationContext*) {
    // Default/no-op impl for properties which are not part of the SkPaint state.
}

template <>
void commitToPaint<SkSVGAttribute::kFill>(const SkSVGPresentationAttributes& attrs,
                                          const SkSVGRenderContext& ctx,
                                          SkSVGPresentationContext* pctx) {
    applySvgPaint(ctx, *attrs.fFill, &pctx->fFillPaint);
}

template <>
void commitToPaint<SkSVGAttribute::kStroke>(const SkSVGPresentationAttributes& attrs,
                                            const SkSVGRenderContext& ctx,
                                            SkSVGPresentationContext* pctx) {
    applySvgPaint(ctx, *attrs.fStroke, &pctx->fStrokePaint);
}

template <>
void commitToPaint<SkSVGAttribute::kFillOpacity>(const SkSVGPresentationAttributes& attrs,
                                                 const SkSVGRenderContext&,
                                                 SkSVGPresentationContext* pctx) {
    pctx->fFillPaint.setAlpha(opacity_to_alpha(*attrs.fFillOpacity));
}

template <>
void commitToPaint<SkSVGAttribute::kStrokeDashArray>(const SkSVGPresentationAttributes& attrs,
                                                     const SkSVGRenderContext& ctx,
                                                     SkSVGPresentationContext* pctx) {
    const auto& dashArray = attrs.fStrokeDashArray.getMaybeNull();
    SkASSERT(dashArray->type() != SkSVGDashArray::Type::kInherit);

    if (dashArray->type() != SkSVGDashArray::Type::kDashArray) {
        return;
    }

    const auto count = dashArray->dashArray().count();
    SkSTArray<128, SkScalar, true> intervals(count);
    for (const auto& dash : dashArray->dashArray()) {
        intervals.push_back(ctx.lengthContext().resolve(dash,
                                                        SkSVGLengthContext::LengthType::kOther));
    }

    if (count & 1) {
        // If an odd number of values is provided, then the list of values
        // is repeated to yield an even number of values.
        intervals.push_back_n(count);
        memcpy(intervals.begin() + count, intervals.begin(), count * sizeof(SkScalar));
    }

    SkASSERT((intervals.count() & 1) == 0);

    const SkScalar phase = ctx.lengthContext().resolve(*pctx->fInherited.fStrokeDashOffset,
                                                       SkSVGLengthContext::LengthType::kOther);
    pctx->fStrokePaint.setPathEffect(SkDashPathEffect::Make(intervals.begin(),
                                                            intervals.count(),
                                                            phase));
}

template <>
void commitToPaint<SkSVGAttribute::kStrokeLineCap>(const SkSVGPresentationAttributes& attrs,
                                                   const SkSVGRenderContext&,
                                                   SkSVGPresentationContext* pctx) {
    pctx->fStrokePaint.setStrokeCap(toSkCap(*attrs.fStrokeLineCap));
}

template <>
void commitToPaint<SkSVGAttribute::kStrokeLineJoin>(const SkSVGPresentationAttributes& attrs,
                                                    const SkSVGRenderContext&,
                                                    SkSVGPresentationContext* pctx) {
    const auto& join = *attrs.fStrokeLineJoin;
    SkASSERT(join.type() != SkSVGLineJoin::Type::kInherit);

    pctx->fStrokePaint.setStrokeJoin(toSkJoin(join));
}

template <>
void commitToPaint<SkSVGAttribute::kStrokeMiterLimit>(const SkSVGPresentationAttributes& attrs,
                                                      const SkSVGRenderContext&,
                                                      SkSVGPresentationContext* pctx) {
    pctx->fStrokePaint.setStrokeMiter(*attrs.fStrokeMiterLimit);
}

template <>
void commitToPaint<SkSVGAttribute::kStrokeOpacity>(const SkSVGPresentationAttributes& attrs,
                                                   const SkSVGRenderContext&,
                                                   SkSVGPresentationContext* pctx) {
    pctx->fStrokePaint.setAlpha(opacity_to_alpha(*attrs.fStrokeOpacity));
}

template <>
void commitToPaint<SkSVGAttribute::kStrokeWidth>(const SkSVGPresentationAttributes& attrs,
                                                 const SkSVGRenderContext& ctx,
                                                 SkSVGPresentationContext* pctx) {
    auto strokeWidth = ctx.lengthContext().resolve(*attrs.fStrokeWidth,
                                                   SkSVGLengthContext::LengthType::kOther);
    pctx->fStrokePaint.setStrokeWidth(strokeWidth);
}

}  // namespace

SkSVGPresentationContext::SkSVGPresentationContext()
    : fInherited(SkSVGPresentationAttributes::MakeInitial()) {

    fFillPaint.setStyle(SkPaint::kFill_Style);
    fStrokePaint.setStyle(SkPaint::kStroke_Style);

    // TODO: drive AA off presentation attrs also (shape-rendering?)
    fFillPaint.setAntiAlias(true);
    fStrokePaint.setAntiAlias(true);

    // Commit initial values to the paint cache.
    SkCanvas fakeCanvas(0, 0);
    SkSVGRenderContext fake(&fakeCanvas, nullptr, SkSVGIDMapper(),
                            SkSVGLengthContext(SkSize::Make(0, 0)),
                            *this, nullptr);

    commitToPaint<SkSVGAttribute::kFill>(fInherited, fake, this);
    commitToPaint<SkSVGAttribute::kFillOpacity>(fInherited, fake, this);
    commitToPaint<SkSVGAttribute::kStroke>(fInherited, fake, this);
    commitToPaint<SkSVGAttribute::kStrokeLineCap>(fInherited, fake, this);
    commitToPaint<SkSVGAttribute::kStrokeLineJoin>(fInherited, fake, this);
    commitToPaint<SkSVGAttribute::kStrokeMiterLimit>(fInherited, fake, this);
    commitToPaint<SkSVGAttribute::kStrokeOpacity>(fInherited, fake, this);
    commitToPaint<SkSVGAttribute::kStrokeWidth>(fInherited, fake, this);
}

SkSVGRenderContext::SkSVGRenderContext(SkCanvas* canvas,
                                       const sk_sp<SkFontMgr>& fmgr,
                                       const SkSVGIDMapper& mapper,
                                       const SkSVGLengthContext& lctx,
                                       const SkSVGPresentationContext& pctx,
                                       const SkSVGNode* node)
    : fFontMgr(fmgr)
    , fIDMapper(mapper)
    , fLengthContext(lctx)
    , fPresentationContext(pctx)
    , fCanvas(canvas)
    , fCanvasSaveCount(canvas->getSaveCount())
    , fNode(node) {}

SkSVGRenderContext::SkSVGRenderContext(const SkSVGRenderContext& other)
    : SkSVGRenderContext(other.fCanvas,
                         other.fFontMgr,
                         other.fIDMapper,
                         *other.fLengthContext,
                         *other.fPresentationContext,
                         other.fNode) {}

SkSVGRenderContext::SkSVGRenderContext(const SkSVGRenderContext& other, SkCanvas* canvas)
    : SkSVGRenderContext(canvas,
                         other.fFontMgr,
                         other.fIDMapper,
                         *other.fLengthContext,
                         *other.fPresentationContext,
                         other.fNode) {}

SkSVGRenderContext::SkSVGRenderContext(const SkSVGRenderContext& other, const SkSVGNode* node)
    : SkSVGRenderContext(other.fCanvas,
                         other.fFontMgr,
                         other.fIDMapper,
                         *other.fLengthContext,
                         *other.fPresentationContext,
                         node) {}

SkSVGRenderContext::~SkSVGRenderContext() {
    fCanvas->restoreToCount(fCanvasSaveCount);
}

SkSVGRenderContext::BorrowedNode SkSVGRenderContext::findNodeById(const SkString& id) const {
    return BorrowedNode(fIDMapper.find(id));
}

void SkSVGRenderContext::applyPresentationAttributes(const SkSVGPresentationAttributes& attrs,
                                                     uint32_t flags) {

#define ApplyLazyInheritedAttribute(ATTR)                                               \
    do {                                                                                \
        /* All attributes should be defined on the inherited context. */                \
        SkASSERT(fPresentationContext->fInherited.f ## ATTR.isValue());                 \
        const auto& attr = attrs.f ## ATTR;                                             \
        if (attr.isValue() && *attr != *fPresentationContext->fInherited.f ## ATTR) {   \
            /* Update the local attribute value */                                      \
            fPresentationContext.writable()->fInherited.f ## ATTR.set(*attr);           \
            /* Update the cached paints */                                              \
            commitToPaint<SkSVGAttribute::k ## ATTR>(attrs, *this,                      \
                                                     fPresentationContext.writable());  \
        }                                                                               \
    } while (false)

    ApplyLazyInheritedAttribute(Fill);
    ApplyLazyInheritedAttribute(FillOpacity);
    ApplyLazyInheritedAttribute(FillRule);
    ApplyLazyInheritedAttribute(FontFamily);
    ApplyLazyInheritedAttribute(FontSize);
    ApplyLazyInheritedAttribute(FontStyle);
    ApplyLazyInheritedAttribute(FontWeight);
    ApplyLazyInheritedAttribute(ClipRule);
    ApplyLazyInheritedAttribute(Stroke);
    ApplyLazyInheritedAttribute(StrokeDashOffset);
    ApplyLazyInheritedAttribute(StrokeDashArray);
    ApplyLazyInheritedAttribute(StrokeLineCap);
    ApplyLazyInheritedAttribute(StrokeLineJoin);
    ApplyLazyInheritedAttribute(StrokeMiterLimit);
    ApplyLazyInheritedAttribute(StrokeOpacity);
    ApplyLazyInheritedAttribute(StrokeWidth);
    ApplyLazyInheritedAttribute(TextAnchor);
    ApplyLazyInheritedAttribute(Visibility);
    ApplyLazyInheritedAttribute(Color);
    ApplyLazyInheritedAttribute(ColorInterpolation);
    ApplyLazyInheritedAttribute(ColorInterpolationFilters);

    // Local 'color' attribute: update paints for attributes that are set to 'currentColor'.
    if (attrs.fColor.isValue()) {
        updatePaintsWithCurrentColor(attrs);
    }

#undef ApplyLazyInheritedAttribute

    // Uninherited attributes.  Only apply to the current context.

    const bool hasFilter = attrs.fFilter.isValue();
    if (attrs.fOpacity.isValue()) {
        this->applyOpacity(*attrs.fOpacity, flags, hasFilter);
    }

    if (attrs.fClipPath.isValue()) {
        this->applyClip(*attrs.fClipPath);
    }

    if (attrs.fMask.isValue()) {
        this->applyMask(*attrs.fMask);
    }

    // TODO: when both a filter and opacity are present, we can apply both with a single layer
    if (hasFilter) {
        this->applyFilter(*attrs.fFilter);
    }

    // Remaining uninherited presentation attributes are accessed as SkSVGNode fields, not via
    // the render context.
    // TODO: resolve these in a pre-render styling pass and assert here that they are values.
    // - stop-color
    // - stop-opacity
    // - flood-color
    // - flood-opacity
}

void SkSVGRenderContext::applyOpacity(SkScalar opacity, uint32_t flags, bool hasFilter) {
    if (opacity >= 1) {
        return;
    }

    const bool hasFill   = SkToBool(this->fillPaint());
    const bool hasStroke = SkToBool(this->strokePaint());

    // We can apply the opacity as paint alpha if it only affects one atomic draw.
    // For now, this means all of the following must be true:
    //   - the target node doesn't have any descendants;
    //   - it only has a stroke or a fill (but not both);
    //   - it does not have a filter.
    // Going forward, we may needto refine this heuristic (e.g. to accommodate markers).
    if ((flags & kLeaf) && (hasFill ^ hasStroke) && !hasFilter) {
        auto* pctx = fPresentationContext.writable();
        if (hasFill) {
            pctx->fFillPaint.setAlpha(
                SkScalarRoundToInt(opacity * pctx->fFillPaint.getAlpha()));
        } else {
            pctx->fStrokePaint.setAlpha(
                SkScalarRoundToInt(opacity * pctx->fStrokePaint.getAlpha()));
        }
    } else {
        // Expensive, layer-based fall back.
        SkPaint opacityPaint;
        opacityPaint.setAlpha(opacity_to_alpha(opacity));
        // Balanced in the destructor, via restoreToCount().
        fCanvas->saveLayer(nullptr, &opacityPaint);
    }
}

void SkSVGRenderContext::applyFilter(const SkSVGFuncIRI& filter) {
    if (filter.type() != SkSVGFuncIRI::Type::kIRI) {
        return;
    }

    const auto node = this->findNodeById(filter.iri());
    if (!node || node->tag() != SkSVGTag::kFilter) {
        return;
    }

    const SkSVGFilter* filterNode = reinterpret_cast<const SkSVGFilter*>(node.get());
    sk_sp<SkImageFilter> imageFilter = filterNode->buildFilterDAG(*this);
    if (imageFilter) {
        SkPaint filterPaint;
        filterPaint.setImageFilter(imageFilter);
        // Balanced in the destructor, via restoreToCount().
        fCanvas->saveLayer(nullptr, &filterPaint);
    }
}

void SkSVGRenderContext::saveOnce() {
    // The canvas only needs to be saved once, per local SkSVGRenderContext.
    if (fCanvas->getSaveCount() == fCanvasSaveCount) {
        fCanvas->save();
    }

    SkASSERT(fCanvas->getSaveCount() > fCanvasSaveCount);
}

void SkSVGRenderContext::applyClip(const SkSVGFuncIRI& clip) {
    if (clip.type() != SkSVGFuncIRI::Type::kIRI) {
        return;
    }

    const auto clipNode = this->findNodeById(clip.iri());
    if (!clipNode || clipNode->tag() != SkSVGTag::kClipPath) {
        return;
    }

    const SkPath clipPath = static_cast<const SkSVGClipPath*>(clipNode.get())->resolveClip(*this);

    // We use the computed clip path in two ways:
    //
    //   - apply to the current canvas, for drawing
    //   - track in the presentation context, for asPath() composition
    //
    // TODO: the two uses are exclusive, avoid canvas churn when non needed.

    this->saveOnce();

    fCanvas->clipPath(clipPath, true);
    fClipPath.set(clipPath);
}

void SkSVGRenderContext::applyMask(const SkSVGFuncIRI& mask) {
    if (mask.type() != SkSVGFuncIRI::Type::kIRI) {
        return;
    }

    const auto node = this->findNodeById(mask.iri());
    if (!node || node->tag() != SkSVGTag::kMask) {
        return;
    }

    const auto* mask_node = static_cast<const SkSVGMask*>(node.get());
    const auto mask_bounds = mask_node->bounds(*this);

    // Isolation/mask layer.
    fCanvas->saveLayer(mask_bounds, nullptr);

    // Render and filter mask content.
    mask_node->renderMask(*this);

    // Content layer
    SkPaint masking_paint;
    masking_paint.setBlendMode(SkBlendMode::kSrcIn);
    fCanvas->saveLayer(mask_bounds, &masking_paint);

    // Content is also clipped to the specified mask bounds.
    fCanvas->clipRect(mask_bounds, true);

    // At this point we're set up for content rendering.
    // The pending layers are restored in the destructor (render context scope exit).
    // Restoring triggers srcIn-compositing the content against the mask.
}

void SkSVGRenderContext::updatePaintsWithCurrentColor(const SkSVGPresentationAttributes& attrs) {
    // Of the attributes that can use currentColor:
    //   https://www.w3.org/TR/SVG11/color.html#ColorProperty
    // Only fill and stroke require paint updates. The others are resolved at render time.

    const auto& fill = fPresentationContext->fInherited.fFill;
    if (fill->type() == SkSVGPaint::Type::kColor &&
        fill->color().type() == SkSVGColor::Type::kCurrentColor) {
        applySvgPaint(*this, *fill, &fPresentationContext.writable()->fFillPaint);
    }

    const auto& stroke = fPresentationContext->fInherited.fStroke;
    if (stroke->type() == SkSVGPaint::Type::kColor &&
        stroke->color().type() == SkSVGColor::Type::kCurrentColor) {
        applySvgPaint(*this, *stroke, &fPresentationContext.writable()->fStrokePaint);
    }
}

const SkPaint* SkSVGRenderContext::fillPaint() const {
    const SkSVGPaint::Type paintType = fPresentationContext->fInherited.fFill->type();
    return paintType != SkSVGPaint::Type::kNone ? &fPresentationContext->fFillPaint : nullptr;
}

const SkPaint* SkSVGRenderContext::strokePaint() const {
    const SkSVGPaint::Type paintType = fPresentationContext->fInherited.fStroke->type();
    return paintType != SkSVGPaint::Type::kNone ? &fPresentationContext->fStrokePaint : nullptr;
}

SkSVGColorType SkSVGRenderContext::resolveSvgColor(const SkSVGColor& color) const {
    switch (color.type()) {
        case SkSVGColor::Type::kColor:
            return color.color();
        case SkSVGColor::Type::kCurrentColor:
            return *fPresentationContext->fInherited.fColor;
        case SkSVGColor::Type::kICCColor:
            SkDebugf("ICC color unimplemented");
            return SK_ColorBLACK;
    }
    SkUNREACHABLE;
}

SkRect SkSVGRenderContext::resolveOBBRect(const SkSVGLength& x, const SkSVGLength& y,
                                          const SkSVGLength& w, const SkSVGLength& h,
                                          SkSVGObjectBoundingBoxUnits obbu) const {
    SkTCopyOnFirstWrite<SkSVGLengthContext> lctx(fLengthContext);

    if (obbu.type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
        *lctx.writable() = SkSVGLengthContext({1,1});
    }

    auto r = lctx->resolveRect(x, y, w, h);
    if (obbu.type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
        const auto obb = fNode->objectBoundingBox(*this);
        r = SkRect::MakeXYWH(obb.x() + r.x() * obb.width(),
                             obb.y() + r.y() * obb.height(),
                             r.width()  * obb.width(),
                             r.height() * obb.height());
    }

    return r;
}

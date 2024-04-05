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
#include "include/core/SkPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/private/base/SkTo.h"
#include "modules/svg/include/SkSVGAttribute.h"
#include "modules/svg/include/SkSVGClipPath.h"
#include "modules/svg/include/SkSVGFilter.h"
#include "modules/svg/include/SkSVGMask.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGTypes.h"

using namespace skia_private;

namespace {

SkScalar length_size_for_type(const SkSize& viewport, SkSVGLengthContext::LengthType t) {
    switch (t) {
    case SkSVGLengthContext::LengthType::kHorizontal:
        return viewport.width();
    case SkSVGLengthContext::LengthType::kVertical:
        return viewport.height();
    case SkSVGLengthContext::LengthType::kOther: {
        // https://www.w3.org/TR/SVG11/coords.html#Units_viewport_percentage
        constexpr SkScalar rsqrt2 = 1.0f / SK_ScalarSqrt2;
        const SkScalar w = viewport.width(), h = viewport.height();
        return rsqrt2 * SkScalarSqrt(w * w + h * h);
    }
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
        SkDebugf("unsupported unit type: <%d>\n", (int)l.unit());
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

static sk_sp<SkPathEffect> dash_effect(const SkSVGPresentationAttributes& props,
                                       const SkSVGLengthContext& lctx) {
    if (props.fStrokeDashArray->type() != SkSVGDashArray::Type::kDashArray) {
        return nullptr;
    }

    const auto& da = *props.fStrokeDashArray;
    const auto count = da.dashArray().size();
    STArray<128, SkScalar, true> intervals(count);
    for (const auto& dash : da.dashArray()) {
        intervals.push_back(lctx.resolve(dash, SkSVGLengthContext::LengthType::kOther));
    }

    if (count & 1) {
        // If an odd number of values is provided, then the list of values
        // is repeated to yield an even number of values.
        intervals.push_back_n(count);
        memcpy(intervals.begin() + count, intervals.begin(), count * sizeof(SkScalar));
    }

    SkASSERT((intervals.size() & 1) == 0);

    const auto phase = lctx.resolve(*props.fStrokeDashOffset,
                                    SkSVGLengthContext::LengthType::kOther);

    return SkDashPathEffect::Make(intervals.begin(), intervals.size(), phase);
}

}  // namespace

SkSVGPresentationContext::SkSVGPresentationContext()
    : fInherited(SkSVGPresentationAttributes::MakeInitial())
{}

SkSVGRenderContext::SkSVGRenderContext(SkCanvas* canvas,
                                       const sk_sp<SkFontMgr>& fmgr,
                                       const sk_sp<skresources::ResourceProvider>& rp,
                                       const SkSVGIDMapper& mapper,
                                       const SkSVGLengthContext& lctx,
                                       const SkSVGPresentationContext& pctx,
                                       const OBBScope& obbs,
                                       const sk_sp<SkShapers::Factory>& fact)
        : fFontMgr(fmgr)
        , fTextShapingFactory(fact)
        , fResourceProvider(rp)
        , fIDMapper(mapper)
        , fLengthContext(lctx)
        , fPresentationContext(pctx)
        , fCanvas(canvas)
        , fCanvasSaveCount(canvas->getSaveCount())
        , fOBBScope(obbs) {}

SkSVGRenderContext::SkSVGRenderContext(const SkSVGRenderContext& other)
        : SkSVGRenderContext(other.fCanvas,
                             other.fFontMgr,
                             other.fResourceProvider,
                             other.fIDMapper,
                             *other.fLengthContext,
                             *other.fPresentationContext,
                             other.fOBBScope,
                             other.fTextShapingFactory) {}

SkSVGRenderContext::SkSVGRenderContext(const SkSVGRenderContext& other, SkCanvas* canvas)
        : SkSVGRenderContext(canvas,
                             other.fFontMgr,
                             other.fResourceProvider,
                             other.fIDMapper,
                             *other.fLengthContext,
                             *other.fPresentationContext,
                             other.fOBBScope,
                             other.fTextShapingFactory) {}

SkSVGRenderContext::SkSVGRenderContext(const SkSVGRenderContext& other, const SkSVGNode* node)
        : SkSVGRenderContext(other.fCanvas,
                             other.fFontMgr,
                             other.fResourceProvider,
                             other.fIDMapper,
                             *other.fLengthContext,
                             *other.fPresentationContext,
                             OBBScope{node, this},
                             other.fTextShapingFactory) {}

SkSVGRenderContext::~SkSVGRenderContext() {
    fCanvas->restoreToCount(fCanvasSaveCount);
}

SkSVGRenderContext::BorrowedNode SkSVGRenderContext::findNodeById(const SkSVGIRI& iri) const {
    if (iri.type() != SkSVGIRI::Type::kLocal) {
        SkDebugf("non-local iri references not currently supported");
        return BorrowedNode(nullptr);
    }
    return BorrowedNode(fIDMapper.find(iri.iri()));
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
    // - lighting-color
}

void SkSVGRenderContext::applyOpacity(SkScalar opacity, uint32_t flags, bool hasFilter) {
    if (opacity >= 1) {
        return;
    }

    const auto& props = fPresentationContext->fInherited;
    const bool hasFill   = props.fFill  ->type() != SkSVGPaint::Type::kNone,
               hasStroke = props.fStroke->type() != SkSVGPaint::Type::kNone;

    // We can apply the opacity as paint alpha if it only affects one atomic draw.
    // For now, this means all of the following must be true:
    //   - the target node doesn't have any descendants;
    //   - it only has a stroke or a fill (but not both);
    //   - it does not have a filter.
    // Going forward, we may needto refine this heuristic (e.g. to accommodate markers).
    if ((flags & kLeaf) && (hasFill ^ hasStroke) && !hasFilter) {
        fDeferredPaintOpacity *= opacity;
    } else {
        // Expensive, layer-based fall back.
        SkPaint opacityPaint;
        opacityPaint.setAlphaf(SkTPin(opacity, 0.0f, 1.0f));
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

SkTLazy<SkPaint> SkSVGRenderContext::commonPaint(const SkSVGPaint& paint_selector,
                                                 float paint_opacity) const {
    if (paint_selector.type() == SkSVGPaint::Type::kNone) {
        return SkTLazy<SkPaint>();
    }

    SkTLazy<SkPaint> p;
    p.init();

    switch (paint_selector.type()) {
    case SkSVGPaint::Type::kColor:
        p->setColor(this->resolveSvgColor(paint_selector.color()));
        break;
    case SkSVGPaint::Type::kIRI: {
        // Our property inheritance is borked as it follows the render path and not the tree
        // hierarchy.  To avoid gross transgressions like leaf node presentation attributes
        // leaking into the paint server context, use a pristine presentation context when
        // following hrefs.
        //
        // Preserve the OBB scope because some paints use object bounding box coords
        // (e.g. gradient control points), which requires access to the render context
        // and node being rendered.
        SkSVGPresentationContext pctx;
        pctx.fNamedColors = fPresentationContext->fNamedColors;
        SkSVGRenderContext local_ctx(fCanvas,
                                     fFontMgr,
                                     fResourceProvider,
                                     fIDMapper,
                                     *fLengthContext,
                                     pctx,
                                     fOBBScope,
                                     fTextShapingFactory);

        const auto node = this->findNodeById(paint_selector.iri());
        if (!node || !node->asPaint(local_ctx, p.get())) {
            // Use the fallback color.
            p->setColor(this->resolveSvgColor(paint_selector.color()));
        }
    } break;
    default:
        SkUNREACHABLE;
    }

    p->setAntiAlias(true); // TODO: shape-rendering support

    // We observe 3 opacity components:
    //   - initial paint server opacity (e.g. color stop opacity)
    //   - paint-specific opacity (e.g. 'fill-opacity', 'stroke-opacity')
    //   - deferred opacity override (optimization for leaf nodes 'opacity')
    p->setAlphaf(SkTPin(p->getAlphaf() * paint_opacity * fDeferredPaintOpacity, 0.0f, 1.0f));

    return p;
}

SkTLazy<SkPaint> SkSVGRenderContext::fillPaint() const {
    const auto& props = fPresentationContext->fInherited;
    auto p = this->commonPaint(*props.fFill, *props.fFillOpacity);

    if (p.isValid()) {
        p->setStyle(SkPaint::kFill_Style);
    }

    return p;
}

SkTLazy<SkPaint> SkSVGRenderContext::strokePaint() const {
    const auto& props = fPresentationContext->fInherited;
    auto p = this->commonPaint(*props.fStroke, *props.fStrokeOpacity);

    if (p.isValid()) {
        p->setStyle(SkPaint::kStroke_Style);
        p->setStrokeWidth(fLengthContext->resolve(*props.fStrokeWidth,
                                                  SkSVGLengthContext::LengthType::kOther));
        p->setStrokeCap(toSkCap(*props.fStrokeLineCap));
        p->setStrokeJoin(toSkJoin(*props.fStrokeLineJoin));
        p->setStrokeMiter(*props.fStrokeMiterLimit);
        p->setPathEffect(dash_effect(props, *fLengthContext));
    }

    return p;
}

SkSVGColorType SkSVGRenderContext::resolveSvgColor(const SkSVGColor& color) const {
    if (fPresentationContext->fNamedColors) {
        for (auto&& ident : color.vars()) {
            SkSVGColorType* c = fPresentationContext->fNamedColors->find(ident);
            if (c) {
                return *c;
            }
        }
    }
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

SkSVGRenderContext::OBBTransform
SkSVGRenderContext::transformForCurrentOBB(SkSVGObjectBoundingBoxUnits u) const {
    if (!fOBBScope.fNode || u.type() == SkSVGObjectBoundingBoxUnits::Type::kUserSpaceOnUse) {
        return {{0,0},{1,1}};
    }
    SkASSERT(fOBBScope.fCtx);

    const auto obb = fOBBScope.fNode->objectBoundingBox(*fOBBScope.fCtx);
    return {{obb.x(), obb.y()}, {obb.width(), obb.height()}};
}

SkRect SkSVGRenderContext::resolveOBBRect(const SkSVGLength& x, const SkSVGLength& y,
                                          const SkSVGLength& w, const SkSVGLength& h,
                                          SkSVGObjectBoundingBoxUnits obbu) const {
    SkTCopyOnFirstWrite<SkSVGLengthContext> lctx(fLengthContext);

    if (obbu.type() == SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox) {
        *lctx.writable() = SkSVGLengthContext({1,1});
    }

    auto r = lctx->resolveRect(x, y, w, h);
    const auto obbt = this->transformForCurrentOBB(obbu);

    return SkRect::MakeXYWH(obbt.scale.x * r.x() + obbt.offset.x,
                            obbt.scale.y * r.y() + obbt.offset.y,
                            obbt.scale.x * r.width(),
                            obbt.scale.y * r.height());
}

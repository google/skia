/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPath.h"
#include "SkSVGAttribute.h"
#include "SkSVGNode.h"
#include "SkSVGRenderContext.h"
#include "SkSVGTypes.h"

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

} // anonymous ns

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
    switch (cap.type()) {
    case SkSVGLineCap::Type::kButt:
        return SkPaint::kButt_Cap;
    case SkSVGLineCap::Type::kRound:
        return SkPaint::kRound_Cap;
    case SkSVGLineCap::Type::kSquare:
        return SkPaint::kSquare_Cap;
    default:
        SkASSERT(false);
        return SkPaint::kButt_Cap;
    }
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
        p->setColor(SkColorSetA(svgPaint.color(), p->getAlpha()));
        break;
    case SkSVGPaint::Type::kIRI: {
        const auto* node = ctx.findNodeById(svgPaint.iri());
        if (!node || !node->asPaint(ctx, p)) {
            p->setColor(SK_ColorTRANSPARENT);
        }
        break;
    }
    case SkSVGPaint::Type::kCurrentColor:
        SkDebugf("unimplemented 'currentColor' paint type");
        // Fall through.
    case SkSVGPaint::Type::kNone:
        // Fall through.
    case SkSVGPaint::Type::kInherit:
        break;
    }
}

inline uint8_t opacity_to_alpha(SkScalar o) {
    return SkTo<uint8_t>(SkScalarRoundToInt(o * 255));
}

// Commit the selected attribute to the paint cache.
template <SkSVGAttribute>
void commitToPaint(const SkSVGPresentationAttributes&,
                   const SkSVGRenderContext&,
                   SkSVGPresentationContext*);

template <>
void commitToPaint<SkSVGAttribute::kFill>(const SkSVGPresentationAttributes& attrs,
                                          const SkSVGRenderContext& ctx,
                                          SkSVGPresentationContext* pctx) {
    applySvgPaint(ctx, *attrs.fFill.get(), &pctx->fFillPaint);
}

template <>
void commitToPaint<SkSVGAttribute::kStroke>(const SkSVGPresentationAttributes& attrs,
                                            const SkSVGRenderContext& ctx,
                                            SkSVGPresentationContext* pctx) {
    applySvgPaint(ctx, *attrs.fStroke.get(), &pctx->fStrokePaint);
}

template <>
void commitToPaint<SkSVGAttribute::kFillOpacity>(const SkSVGPresentationAttributes& attrs,
                                                 const SkSVGRenderContext&,
                                                 SkSVGPresentationContext* pctx) {
    pctx->fFillPaint.setAlpha(opacity_to_alpha(*attrs.fFillOpacity.get()));
}

template <>
void commitToPaint<SkSVGAttribute::kStrokeLineCap>(const SkSVGPresentationAttributes& attrs,
                                                   const SkSVGRenderContext&,
                                                   SkSVGPresentationContext* pctx) {
    const auto& cap = *attrs.fStrokeLineCap.get();
    if (cap.type() != SkSVGLineCap::Type::kInherit) {
        pctx->fStrokePaint.setStrokeCap(toSkCap(cap));
    }
}

template <>
void commitToPaint<SkSVGAttribute::kStrokeLineJoin>(const SkSVGPresentationAttributes& attrs,
                                                    const SkSVGRenderContext&,
                                                    SkSVGPresentationContext* pctx) {
    const auto& join = *attrs.fStrokeLineJoin.get();
    if (join.type() != SkSVGLineJoin::Type::kInherit) {
        pctx->fStrokePaint.setStrokeJoin(toSkJoin(join));
    }
}

template <>
void commitToPaint<SkSVGAttribute::kStrokeOpacity>(const SkSVGPresentationAttributes& attrs,
                                                   const SkSVGRenderContext&,
                                                   SkSVGPresentationContext* pctx) {
    pctx->fStrokePaint.setAlpha(opacity_to_alpha(*attrs.fStrokeOpacity.get()));
}

template <>
void commitToPaint<SkSVGAttribute::kStrokeWidth>(const SkSVGPresentationAttributes& attrs,
                                                 const SkSVGRenderContext& ctx,
                                                 SkSVGPresentationContext* pctx) {
    auto strokeWidth = ctx.lengthContext().resolve(*attrs.fStrokeWidth.get(),
                                                   SkSVGLengthContext::LengthType::kOther);
    pctx->fStrokePaint.setStrokeWidth(strokeWidth);
}

template <>
void commitToPaint<SkSVGAttribute::kFillRule>(const SkSVGPresentationAttributes&,
                                              const SkSVGRenderContext&,
                                              SkSVGPresentationContext*) {
    // Not part of the SkPaint state; applied to the path at render time.
}

} // anonymous ns

SkSVGPresentationContext::SkSVGPresentationContext()
    : fInherited(SkSVGPresentationAttributes::MakeInitial()) {

    fFillPaint.setStyle(SkPaint::kFill_Style);
    fStrokePaint.setStyle(SkPaint::kStroke_Style);

    // TODO: drive AA off presentation attrs also (shape-rendering?)
    fFillPaint.setAntiAlias(true);
    fStrokePaint.setAntiAlias(true);

    // Commit initial values to the paint cache.
    SkCanvas dummyCanvas(0, 0);
    SkSVGRenderContext dummy(&dummyCanvas, SkSVGIDMapper(), SkSVGLengthContext(SkSize::Make(0, 0)),
                             *this);

    commitToPaint<SkSVGAttribute::kFill>(fInherited, dummy, this);
    commitToPaint<SkSVGAttribute::kFillOpacity>(fInherited, dummy, this);
    commitToPaint<SkSVGAttribute::kStroke>(fInherited, dummy, this);
    commitToPaint<SkSVGAttribute::kStrokeLineCap>(fInherited, dummy, this);
    commitToPaint<SkSVGAttribute::kStrokeLineJoin>(fInherited, dummy, this);
    commitToPaint<SkSVGAttribute::kStrokeOpacity>(fInherited, dummy, this);
    commitToPaint<SkSVGAttribute::kStrokeWidth>(fInherited, dummy, this);
}

SkSVGRenderContext::SkSVGRenderContext(SkCanvas* canvas,
                                       const SkSVGIDMapper& mapper,
                                       const SkSVGLengthContext& lctx,
                                       const SkSVGPresentationContext& pctx)
    : fIDMapper(mapper)
    , fLengthContext(lctx)
    , fPresentationContext(pctx)
    , fCanvas(canvas)
    , fCanvasSaveCount(canvas->getSaveCount()) {}

SkSVGRenderContext::SkSVGRenderContext(const SkSVGRenderContext& other)
    : SkSVGRenderContext(other.fCanvas,
                         other.fIDMapper,
                         *other.fLengthContext,
                         *other.fPresentationContext) {}

SkSVGRenderContext::~SkSVGRenderContext() {
    fCanvas->restoreToCount(fCanvasSaveCount);
}

const SkSVGNode* SkSVGRenderContext::findNodeById(const SkString& id) const {
    const auto* v = fIDMapper.find(id);
    return v ? v->get() : nullptr;
}

void SkSVGRenderContext::applyPresentationAttributes(const SkSVGPresentationAttributes& attrs,
                                                     uint32_t flags) {

#define ApplyLazyInheritedAttribute(ATTR)                                               \
    do {                                                                                \
        /* All attributes should be defined on the inherited context. */                \
        SkASSERT(fPresentationContext->fInherited.f ## ATTR.isValid());                 \
        const auto* value = attrs.f ## ATTR.getMaybeNull();                             \
        if (value && *value != *fPresentationContext->fInherited.f ## ATTR.get()) {     \
            /* Update the local attribute value */                                      \
            fPresentationContext.writable()->fInherited.f ## ATTR.set(*value);          \
            /* Update the cached paints */                                              \
            commitToPaint<SkSVGAttribute::k ## ATTR>(attrs, *this,    \
                                                     fPresentationContext.writable());  \
        }                                                                               \
    } while (false)

    ApplyLazyInheritedAttribute(Fill);
    ApplyLazyInheritedAttribute(FillOpacity);
    ApplyLazyInheritedAttribute(FillRule);
    ApplyLazyInheritedAttribute(Stroke);
    ApplyLazyInheritedAttribute(StrokeLineCap);
    ApplyLazyInheritedAttribute(StrokeLineJoin);
    ApplyLazyInheritedAttribute(StrokeOpacity);
    ApplyLazyInheritedAttribute(StrokeWidth);

#undef ApplyLazyInheritedAttribute

    // Uninherited attributes.  Only apply to the current context.

    if (auto* opacity = attrs.fOpacity.getMaybeNull()) {
        this->applyOpacity(opacity->value(), flags);
    }

    if (auto* clip = attrs.fClipPath.getMaybeNull()) {
        this->applyClip(*clip);
    }
}

void SkSVGRenderContext::applyOpacity(SkScalar opacity, uint32_t flags) {
    if (opacity >= 1) {
        return;
    }

    const bool hasFill   = SkToBool(this->fillPaint());
    const bool hasStroke = SkToBool(this->strokePaint());

    // We can apply the opacity as paint alpha iif it only affects one atomic draw.
    // For now, this means a) the target node doesn't have any descendants, and
    // b) it only has a stroke or a fill (but not both).  Going forward, we may need
    // to refine this heuristic (e.g. to accommodate markers).
    if ((flags & kLeaf) && (hasFill ^ hasStroke)) {
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

void SkSVGRenderContext::applyClip(const SkSVGClip& clip) {
    if (clip.type() != SkSVGClip::Type::kIRI) {
        return;
    }

    const SkSVGNode* clipNode = this->findNodeById(clip.iri());
    if (!clipNode || clipNode->tag() != SkSVGTag::kClipPath) {
        return;
    }

    const SkPath clipPath = clipNode->asPath(*this);

    // We use the computed clip path in two ways:
    //
    //   - apply to the current canvas, for drawing
    //   - track in the presentation context, for asPath() composition
    //
    // TODO: the two uses are exclusive, avoid canvas churn when non needed.

    // Only save if needed
    if (fCanvas->getSaveCount() == fCanvasSaveCount) {
        fCanvas->save();
    }

    fCanvas->clipPath(clipPath, true);
    fClipPath.set(clipPath);
}

const SkPaint* SkSVGRenderContext::fillPaint() const {
    const SkSVGPaint::Type paintType = fPresentationContext->fInherited.fFill.get()->type();
    return paintType != SkSVGPaint::Type::kNone ? &fPresentationContext->fFillPaint : nullptr;
}

const SkPaint* SkSVGRenderContext::strokePaint() const {
    const SkSVGPaint::Type paintType = fPresentationContext->fInherited.fStroke.get()->type();
    return paintType != SkSVGPaint::Type::kNone ? &fPresentationContext->fStrokePaint : nullptr;
}

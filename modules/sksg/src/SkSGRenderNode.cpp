/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGRenderNode.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "modules/sksg/src/SkSGNodePriv.h"

namespace sksg {

namespace {

enum Flags : uint8_t {
    kInvisible_Flag = 1 << 0,
};

} // namespace

RenderNode::RenderNode(uint32_t inval_traits) : INHERITED(inval_traits) {}

bool RenderNode::isVisible() const {
    return !(fNodeFlags & kInvisible_Flag);
}

void RenderNode::setVisible(bool v) {
    if (v == this->isVisible()) {
        return;
    }

    this->invalidate();
    fNodeFlags = v ? (fNodeFlags & ~kInvisible_Flag)
                   : (fNodeFlags | kInvisible_Flag);
}

void RenderNode::render(SkCanvas* canvas, const RenderContext* ctx) const {
    SkASSERT(!this->hasInval());
    if (this->isVisible() && !this->bounds().isEmpty()) {
        this->onRender(canvas, ctx);
    }
    SkASSERT(!this->hasInval());
}

const RenderNode* RenderNode::nodeAt(const SkPoint& p) const {
    return this->bounds().contains(p.x(), p.y()) ? this->onNodeAt(p) : nullptr;
}

static SkAlpha ScaleAlpha(SkAlpha alpha, float opacity) {
   return SkToU8(sk_float_round2int(alpha * opacity));
}

static SkMatrix ComputeDiffInverse(const SkMatrix& base, const SkMatrix& ctm) {
    // Mask filters / shaders are declared to operate under a specific transform, but due to the
    // deferral mechanism, other transformations might have been pushed to the state.
    // We want to undo these transforms:
    //
    //   baseCTM x T = ctm
    //
    //   =>  T = Inv(baseCTM) x ctm
    //
    //   =>  Inv(T) = Inv(Inv(baseCTM) x ctm)
    //
    //   =>  Inv(T) = Inv(ctm) x baseCTM

    SkMatrix m;
    if (base != ctm && ctm.invert(&m)) {
        m.preConcat(base);
    } else {
        m = SkMatrix::I();
    }

    return m;
}

bool RenderNode::RenderContext::requiresIsolation() const {
    // Note: fShader is never applied on isolation layers.
    return ScaleAlpha(SK_AlphaOPAQUE, fOpacity) != SK_AlphaOPAQUE
        || fColorFilter
        || fMaskFilter
        || fBlendMode != SkBlendMode::kSrcOver;
}

void RenderNode::RenderContext::modulatePaint(const SkMatrix& ctm, SkPaint* paint) const {
    paint->setAlpha(ScaleAlpha(paint->getAlpha(), fOpacity));
    paint->setColorFilter(SkColorFilters::Compose(fColorFilter, paint->refColorFilter()));
    if (fShader) {
        paint->setShader(fShader->makeWithLocalMatrix(ComputeDiffInverse(fShaderCTM, ctm)));
    }
    if (fMaskFilter) {
        paint->setMaskFilter(fMaskFilter->makeWithMatrix(ComputeDiffInverse(fMaskCTM, ctm)));
    }
    paint->setBlendMode(fBlendMode);
}

RenderNode::ScopedRenderContext::ScopedRenderContext(SkCanvas* canvas, const RenderContext* ctx)
    : fCanvas(canvas)
    , fCtx(ctx ? *ctx : RenderContext())
    , fRestoreCount(canvas->getSaveCount()) {}

RenderNode::ScopedRenderContext::~ScopedRenderContext() {
    if (fRestoreCount >= 0) {
        fCanvas->restoreToCount(fRestoreCount);
    }
}

RenderNode::ScopedRenderContext&&
RenderNode::ScopedRenderContext::modulateOpacity(float opacity) {
    SkASSERT(opacity >= 0 && opacity <= 1);
    fCtx.fOpacity *= opacity;
    return std::move(*this);
}

RenderNode::ScopedRenderContext&&
RenderNode::ScopedRenderContext::modulateColorFilter(sk_sp<SkColorFilter> cf) {
    fCtx.fColorFilter = SkColorFilters::Compose(std::move(fCtx.fColorFilter), std::move(cf));
    return std::move(*this);
}

RenderNode::ScopedRenderContext&&
RenderNode::ScopedRenderContext::modulateShader(sk_sp<SkShader> sh, const SkMatrix& shader_ctm) {
    // Topmost shader takes precedence.
    if (!fCtx.fShader) {
        fCtx.fShader = std::move(sh);
        fCtx.fShaderCTM = shader_ctm;
    }

    return std::move(*this);
}

RenderNode::ScopedRenderContext&&
RenderNode::ScopedRenderContext::modulateMaskFilter(sk_sp<SkMaskFilter> mf, const SkMatrix& ctm) {
    if (fCtx.fMaskFilter) {
        // As we compose mask filters, use the relative transform T for the inner mask:
        //
        //   maskCTM x T = ctm
        //
        //   => T = Inv(maskCTM) x ctm
        //
        SkMatrix invMaskCTM;
        if (mf && fCtx.fMaskCTM.invert(&invMaskCTM)) {
            const auto relative_transform = SkMatrix::Concat(invMaskCTM, ctm);
            fCtx.fMaskFilter = SkMaskFilter::MakeCompose(std::move(fCtx.fMaskFilter),
                                                         mf->makeWithMatrix(relative_transform));
        }
    } else {
        fCtx.fMaskFilter = std::move(mf);
        fCtx.fMaskCTM    = ctm;
    }

    return std::move(*this);
}

RenderNode::ScopedRenderContext&&
RenderNode::ScopedRenderContext::modulateBlendMode(SkBlendMode mode) {
    fCtx.fBlendMode = mode;
    return std::move(*this);
}

RenderNode::ScopedRenderContext&&
RenderNode::ScopedRenderContext::setIsolation(const SkRect& bounds, const SkMatrix& ctm,
                                              bool isolation) {
    if (isolation && fCtx.requiresIsolation()) {
        SkPaint layer_paint;
        fCtx.modulatePaint(ctm, &layer_paint);
        fCanvas->saveLayer(bounds, &layer_paint);

        // Reset only the props applied via isolation layers.
        fCtx.fColorFilter = nullptr;
        fCtx.fMaskFilter  = nullptr;
        fCtx.fOpacity     = 1;
        fCtx.fBlendMode   = SkBlendMode::kSrcOver;
    }

    return std::move(*this);
}

RenderNode::ScopedRenderContext&&
RenderNode::ScopedRenderContext::setFilterIsolation(const SkRect& bounds, const SkMatrix& ctm,
                                                    sk_sp<SkImageFilter> filter) {
    SkPaint layer_paint;
    fCtx.modulatePaint(ctm, &layer_paint);

    SkASSERT(!layer_paint.getImageFilter());
    layer_paint.setImageFilter(std::move(filter));
    fCanvas->saveLayer(bounds, &layer_paint);
    fCtx = RenderContext();

    return std::move(*this);
}

CustomRenderNode::CustomRenderNode(std::vector<sk_sp<RenderNode>>&& children)
    : INHERITED(kOverrideDamage_Trait)  // We cannot make any assumptions - override conservatively.
    , fChildren(std::move(children)) {
    for (const auto& child : fChildren) {
        this->observeInval(child);
    }
}

CustomRenderNode::~CustomRenderNode() {
    for (const auto& child : fChildren) {
        this->unobserveInval(child);
    }
}

bool CustomRenderNode::hasChildrenInval() const {
    for (const auto& child : fChildren) {
        if (NodePriv::HasInval(child)) {
            return true;
        }
    }

    return false;
}

} // namespace sksg

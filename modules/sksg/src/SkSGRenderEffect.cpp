/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGRenderEffect.h"

#include "SkColorFilter.h"
#include "SkDropShadowImageFilter.h"
#include "SkImageFilter.h"
#include "SkMakeUnique.h"
#include "SkSGColor.h"

namespace sksg {

sk_sp<RenderNode> ColorFilterEffect::Make(sk_sp<RenderNode> child, sk_sp<ColorFilter> filter) {
    return (child && filter) ? sk_sp<RenderNode>(new ColorFilterEffect(std::move(child),
                                                                       std::move(filter)))
                             : child;
}

ColorFilterEffect::ColorFilterEffect(sk_sp<RenderNode> child, sk_sp<ColorFilter> filter)
    : INHERITED(std::move(child), std::move(filter)) {}

ColorFilterEffect::~ColorFilterEffect() = default;

SkRect ColorFilterEffect::onRevalidateFilterEffect(const SkRect& content_bounds) {
    // Color filter effects do not alter the descendants' bounds.
    return content_bounds;
}

const RenderNode* ColorFilterEffect::onNodeAt(const SkPoint& p) const {
    SkASSERT(this->bounds().contains(p.x(), p.y()));
    // Pass through to descendants.
    return this->INHERITED::onNodeAt(p);
}

void ColorFilterEffect::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    // TODO: hoist these checks to RenderNode?
    if (this->bounds().isEmpty())
        return;

    const auto local_ctx = ScopedRenderContext(canvas, ctx).modulateColorFilter(this->getFilter());

    this->INHERITED::onRender(canvas, local_ctx);
}

sk_sp<ColorModeFilter> ColorModeFilter::Make(sk_sp<Color> color, SkBlendMode mode) {
    return color ? sk_sp<ColorModeFilter>(new ColorModeFilter(std::move(color), mode))
                 : nullptr;
}

ColorModeFilter::ColorModeFilter(sk_sp<Color> color, SkBlendMode mode)
    : fColor(std::move(color))
    , fMode(mode) {
    this->observeInval(fColor);
}

ColorModeFilter::~ColorModeFilter() {
    this->unobserveInval(fColor);
}

sk_sp<SkColorFilter> ColorModeFilter::onRevalidateFilter() {
    fColor->revalidate(nullptr, SkMatrix::I());
    return SkColorFilter::MakeModeFilter(fColor->getColor(), fMode);
}

sk_sp<RenderNode> ImageFilterEffect::Make(sk_sp<RenderNode> child, sk_sp<ImageFilter> filter) {
    return (child && filter) ? sk_sp<RenderNode>(new ImageFilterEffect(std::move(child),
                                                                       std::move(filter)))
                             : child;
}

ImageFilterEffect::ImageFilterEffect(sk_sp<RenderNode> child, sk_sp<ImageFilter> filter)
    : INHERITED(std::move(child), std::move(filter)) {}

SkRect ImageFilterEffect::onRevalidateFilterEffect(const SkRect& content_bounds) {
    const auto& filter = this->getFilter();
    SkASSERT(filter->canComputeFastBounds());

    return filter->computeFastBounds(content_bounds);
}

const RenderNode* ImageFilterEffect::onNodeAt(const SkPoint& p) const {
    // TODO: map p through the filter DAG and dispatch to descendants?
    // For now, image filters occlude hit-testing.
    SkASSERT(this->bounds().contains(p.x(), p.y()));
    return this;
}

void ImageFilterEffect::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    // TODO: hoist these checks to RenderNode?
    if (this->bounds().isEmpty())
        return;

    // Note: we're using the source content bounds for saveLayer, not our local/filtered bounds.
    const auto filter_ctx =
        ScopedRenderContext(canvas, ctx).setFilterIsolation(this->getChild()->bounds(),
                                                            this->getFilter());
    this->INHERITED::onRender(canvas, filter_ctx);
}

sk_sp<DropShadowImageFilter> DropShadowImageFilter::Make(sk_sp<ImageFilter> input) {
    return sk_sp<DropShadowImageFilter>(new DropShadowImageFilter(std::move(input)));
}

DropShadowImageFilter::DropShadowImageFilter(sk_sp<ImageFilter> input)
    : INHERITED(std::move(input)) {}

DropShadowImageFilter::~DropShadowImageFilter() = default;

sk_sp<SkImageFilter> DropShadowImageFilter::onRevalidateFilter() {
    const auto mode = (fMode == Mode::kShadowOnly)
            ? SkDropShadowImageFilter::kDrawShadowOnly_ShadowMode
            : SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode;

    return SkDropShadowImageFilter::Make(fOffset.x(), fOffset.y(),
                                         fSigma.x(), fSigma.y(),
                                         fColor, mode, this->refInput(0));
}

} // namespace sksg

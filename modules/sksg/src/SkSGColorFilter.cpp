/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGColorFilter.h"

#include "SkColorFilter.h"
#include "SkSGColor.h"

namespace sksg {

ColorFilter::ColorFilter(sk_sp<RenderNode> child)
    : INHERITED(std::move(child)) {}

void ColorFilter::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    if (this->bounds().isEmpty())
        return;

    const auto local_ctx = ScopedRenderContext(canvas, ctx).modulateColorFilter(fColorFilter);

    this->INHERITED::onRender(canvas, local_ctx);
}

const RenderNode* ColorFilter::onNodeAt(const SkPoint& p) const {
    // TODO: we likely need to do something more sophisticated than delegate to descendants here.
    return this->INHERITED::onNodeAt(p);
}

SkRect ColorFilter::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    fColorFilter = this->onRevalidateFilter();

    return this->INHERITED::onRevalidate(ic, ctm);
}

sk_sp<ModeColorFilter> ModeColorFilter::Make(sk_sp<RenderNode> child, sk_sp<Color> color,
                                             SkBlendMode mode) {
    return (child && color) ? sk_sp<ModeColorFilter>(new ModeColorFilter(std::move(child),
                                                                         std::move(color), mode))
                            : nullptr;
}

ModeColorFilter::ModeColorFilter(sk_sp<RenderNode> child, sk_sp<Color> color, SkBlendMode mode)
    : INHERITED(std::move(child))
    , fColor(std::move(color))
    , fMode(mode) {
    this->observeInval(fColor);
}

ModeColorFilter::~ModeColorFilter() {
    this->unobserveInval(fColor);
}

sk_sp<SkColorFilter> ModeColorFilter::onRevalidateFilter() {
    fColor->revalidate(nullptr, SkMatrix::I());
    return SkColorFilter::MakeModeFilter(fColor->getColor(), fMode);
}

sk_sp<TintColorFilter> TintColorFilter::Make(sk_sp<RenderNode> child,
                                             sk_sp<Color> c0, sk_sp<Color> c1) {
    return (child && c0 && c1) ? sk_sp<TintColorFilter>(new TintColorFilter(std::move(child),
                                                                            std::move(c0),
                                                                            std::move(c1)))
                               : nullptr;
}

TintColorFilter::TintColorFilter(sk_sp<RenderNode> child, sk_sp<Color> c0, sk_sp<Color> c1)
    : INHERITED(std::move(child))
    , fColor0(std::move(c0))
    , fColor1(std::move(c1)) {
    this->observeInval(fColor0);
    this->observeInval(fColor1);
}

TintColorFilter::~TintColorFilter() {
    this->unobserveInval(fColor0);
    this->unobserveInval(fColor1);
}

sk_sp<SkColorFilter> TintColorFilter::onRevalidateFilter() {
    fColor0->revalidate(nullptr, SkMatrix::I());
    fColor1->revalidate(nullptr, SkMatrix::I());

    if (fWeight <= 0) {
        return nullptr;
    }

    const auto c0 = SkColor4f::FromColor(fColor0->getColor()),
               c1 = SkColor4f::FromColor(fColor1->getColor());

    // luminance coefficients
    static constexpr float kR = 0.2126f,
                           kG = 0.7152f,
                           kB = 0.0722f;

    const auto dR = c1.fR - c0.fR,
               dG = c1.fG - c0.fG,
               dB = c1.fB - c0.fB;

    // First, we need a luminance:
    //
    //   L = [r,g,b] . [kR,kG,kB]
    //
    // We can compute it using a color matrix (result stored in R):
    //
    //   | kR, kG, kB,  0,  0 |    r' = L
    //   |  0,  0,  0,  0,  0 |    g' = 0
    //   |  0,  0,  0,  0,  0 |    b' = 0
    //   |  0,  0,  0,  1,  0 |    a' = a
    //
    // Then we want to interpolate component-wise, based on L:
    //
    //   r' = c0.r + (c1.r - c0.r) * L = c0.r + dR*L
    //   g' = c0.g + (c1.g - c0.g) * L = c0.g + dG*L
    //   b' = c0.b + (c1.b - c0.b) * L = c0.b + dB*L
    //   a' = a
    //
    // This can be expressed as another color matrix (when L is stored in R):
    //
    //  | dR,  0,  0,  0, c0.r |
    //  | dG,  0,  0,  0, c0.g |
    //  | dB,  0,  0,  0, c0.b |
    //  |  0,  0,  0,  1,    0 |
    //
    // Composing these two, we get the total tint matrix:

    const SkScalar tint_matrix[] = {
        dR*kR, dR*kG, dR*kB, 0, c0.fR * 255,
        dG*kR, dG*kG, dG*kB, 0, c0.fG * 255,
        dB*kR, dB*kG, dB*kB, 0, c0.fB * 255,
            0,     0,     0, 1,           0,
    };

    return SkColorFilter::MakeMixer(nullptr,
                                    SkColorFilter::MakeMatrixFilterRowMajor255(tint_matrix),
                                    fWeight);
}

} // namespace sksg

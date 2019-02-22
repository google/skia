/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGColorFilter.h"

#include "SkColorData.h"
#include "SkColorFilter.h"
#include "SkSGColor.h"
#include "SkTableColorFilter.h"

#include <cmath>

namespace sksg {

ColorFilter::ColorFilter(sk_sp<RenderNode> child)
    : INHERITED(std::move(child)) {}

void ColorFilter::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
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

sk_sp<GradientColorFilter> GradientColorFilter::Make(sk_sp<RenderNode> child,
                                                     sk_sp<Color> c0, sk_sp<Color> c1) {
    return Make(std::move(child), { std::move(c0), std::move(c1) });
}

sk_sp<GradientColorFilter> GradientColorFilter::Make(sk_sp<RenderNode> child,
                                                     std::vector<sk_sp<Color>> colors) {
    return (child && colors.size() > 1)
        ? sk_sp<GradientColorFilter>(new GradientColorFilter(std::move(child), std::move(colors)))
        : nullptr;
}

GradientColorFilter::GradientColorFilter(sk_sp<RenderNode> child, std::vector<sk_sp<Color>> colors)
    : INHERITED(std::move(child))
    , fColors(std::move(colors)) {
    for (const auto& color : fColors) {
        this->observeInval(color);
    }
}

GradientColorFilter::~GradientColorFilter() {
    for (const auto& color : fColors) {
        this->unobserveInval(color);
    }
}

namespace  {

sk_sp<SkColorFilter> Make2ColorGradient(const sk_sp<Color>& color0, const sk_sp<Color>& color1) {
    const auto c0 = SkColor4f::FromColor(color0->getColor()),
               c1 = SkColor4f::FromColor(color1->getColor());

    const auto dR = c1.fR - c0.fR,
               dG = c1.fG - c0.fG,
               dB = c1.fB - c0.fB;

    // A 2-color gradient can be expressed as a color matrix (and combined with the luminance
    // calculation).  First, the luminance:
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
        dR*SK_LUM_COEFF_R, dR*SK_LUM_COEFF_G, dR*SK_LUM_COEFF_B, 0, c0.fR * 255,
        dG*SK_LUM_COEFF_R, dG*SK_LUM_COEFF_G, dG*SK_LUM_COEFF_B, 0, c0.fG * 255,
        dB*SK_LUM_COEFF_R, dB*SK_LUM_COEFF_G, dB*SK_LUM_COEFF_B, 0, c0.fB * 255,
                        0,                 0,                 0, 1,           0,
    };

    return SkColorFilter::MakeMatrixFilterRowMajor255(tint_matrix);
}

sk_sp<SkColorFilter> MakeNColorGradient(const std::vector<sk_sp<Color>>& colors) {
    // For N colors, we build a gradient color table.
    uint8_t rTable[256], gTable[256], bTable[256];

    SkASSERT(colors.size() > 2);
    const auto span_count = colors.size() - 1;

    size_t span_start = 0;
    for (size_t i = 0; i < span_count; ++i) {
        const auto span_stop = static_cast<size_t>(std::round((i + 1) * 255.0f / span_count)),
                   span_size = span_stop - span_start;
        if (span_start > span_stop) {
            // Degenerate case.
            continue;
        }
        SkASSERT(span_stop <= 255);

        // Fill the gradient in [span_start,span_stop] -> [c0,c1]
        const SkColor c0 = colors[i    ]->getColor(),
                      c1 = colors[i + 1]->getColor();
        float r = SkColorGetR(c0),
              g = SkColorGetG(c0),
              b = SkColorGetB(c0);
        const float dR = (SkColorGetR(c1) - r) / span_size,
                    dG = (SkColorGetG(c1) - g) / span_size,
                    dB = (SkColorGetB(c1) - b) / span_size;

        for (size_t j = span_start; j <= span_stop; ++j) {
            rTable[j] = static_cast<uint8_t>(std::round(r));
            gTable[j] = static_cast<uint8_t>(std::round(g));
            bTable[j] = static_cast<uint8_t>(std::round(b));
            r += dR;
            g += dG;
            b += dB;
        }

        // Ensure we always advance.
        span_start = span_stop + 1;
    }
    SkASSERT(span_start == 256);

    const SkScalar luminance_matrix[] = {
        SK_LUM_COEFF_R, SK_LUM_COEFF_G, SK_LUM_COEFF_B,  0,  0,  // r' = L
        SK_LUM_COEFF_R, SK_LUM_COEFF_G, SK_LUM_COEFF_B,  0,  0,  // g' = L
        SK_LUM_COEFF_R, SK_LUM_COEFF_G, SK_LUM_COEFF_B,  0,  0,  // b' = L
                     0,              0,              0,  1,  0,  // a' = a
    };

    return SkTableColorFilter::MakeARGB(nullptr, rTable, gTable, bTable)
            ->makeComposed(SkColorFilter::MakeMatrixFilterRowMajor255(luminance_matrix));
}

} // namespace

sk_sp<SkColorFilter> GradientColorFilter::onRevalidateFilter() {
    for (const auto& color : fColors) {
        color->revalidate(nullptr, SkMatrix::I());
    }

    if (fWeight <= 0) {
        return nullptr;
    }

    SkASSERT(fColors.size() > 1);
    auto gradientCF = (fColors.size() > 2) ? MakeNColorGradient(fColors)
                                           : Make2ColorGradient(fColors[0], fColors[1]);

    return SkColorFilter::MakeMixer(nullptr, std::move(gradientCF), fWeight);
}

} // namespace sksg

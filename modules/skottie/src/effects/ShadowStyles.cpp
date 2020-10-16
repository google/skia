/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/core/SkColorFilter.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/SkTPin.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "src/utils/SkJSON.h"

namespace skottie::internal {

namespace {

class ShadowAdapter final : public DiscardableAdapterBase<ShadowAdapter,
                                                          sksg::ExternalImageFilter> {
public:
    enum Type {
        kDropShadow,
        kInnerShadow,
    };

    ShadowAdapter(const skjson::ObjectValue& jstyle,
                  const AnimationBuilder& abuilder,
                  Type type)
        : fType(type) {
        this->bind(abuilder, jstyle["c"], fColor);
        this->bind(abuilder, jstyle["o"], fOpacity);
        this->bind(abuilder, jstyle["a"], fAngle);
        this->bind(abuilder, jstyle["s"], fSize);
        this->bind(abuilder, jstyle["d"], fDistance);
    }

private:
    void onSync() override {
        const auto    rad = SkDegreesToRadians(180 + fAngle), // 0deg -> left (style)
                    sigma = fSize * kBlurSizeToSigma,
                  opacity = SkTPin(fOpacity / 100, 0.0f, 1.0f);
        const auto  color = static_cast<SkColor4f>(fColor);
        const auto offset = SkV2{ fDistance * SkScalarCos(rad),
                                 -fDistance * SkScalarSin(rad)};

        // Shadow effects largely follow the feDropShadow spec [1]:
        //
        //   1) isolate source alpha
        //   2) apply a gaussian blur
        //   3) apply an offset
        //   4) modulate with a flood/color generator
        //   5) composite with the source
        //
        // Note: as an optimization, we can fold #1 and #4 into a single color matrix filter.
        //
        // Inner shadow differences:
        //
        //   a) operates on the inverse of source alpha
        //   b) the result is masked against the source
        //   c) composited on top of source
        //
        // [1] https://drafts.fxtf.org/filter-effects/#feDropShadowElement

        // Select and colorize the source alpha channel.
        SkColorMatrix cm{0, 0, 0,                  0, color.fR,
                         0, 0, 0,                  0, color.fG,
                         0, 0, 0,                  0, color.fB,
                         0, 0, 0, opacity * color.fA,        0};

        // Inner shadows use the alpha inverse.
        if (fType == Type::kInnerShadow) {
            cm.preConcat({1, 0, 0, 0, 0,
                          0, 1, 0, 0, 0,
                          0, 0, 1, 0, 0,
                          0, 0, 0,-1, 1});
        }
        auto f = SkImageFilters::ColorFilter(SkColorFilters::Matrix(cm), nullptr);

        if (sigma > 0) {
            f = SkImageFilters::Blur(sigma, sigma, std::move(f));
        }

        if (!SkScalarNearlyZero(offset.x) || !SkScalarNearlyZero(offset.y)) {
            f = SkImageFilters::Offset(offset.x, offset.y, std::move(f));
        }

        sk_sp<SkImageFilter> source;

        if (fType == Type::kInnerShadow) {
            // Inner shadows draw on top of, and are masked with, the source.
            f = SkImageFilters::Blend(SkBlendMode::kDstIn, std::move(f));

            std::swap(source, f);
        }

        this->node()->setImageFilter(SkImageFilters::Merge(std::move(f),
                                                           std::move(source)));
    }

    const Type fType;

    VectorValue fColor;
    ScalarValue fOpacity  = 100, // percentage
                fAngle    =   0, // degrees
                fSize     =   0,
                fDistance =   0;

    using INHERITED = DiscardableAdapterBase<ShadowAdapter, sksg::ExternalImageFilter>;
};

static sk_sp<sksg::RenderNode> make_shadow_effect(const skjson::ObjectValue& jstyle,
                                                  const AnimationBuilder& abuilder,
                                                  sk_sp<sksg::RenderNode> layer,
                                                  ShadowAdapter::Type type) {
    auto filter_node = abuilder.attachDiscardableAdapter<ShadowAdapter>(jstyle, abuilder, type);

    return sksg::ImageFilterEffect::Make(std::move(layer), std::move(filter_node));
}

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachDropShadowStyle(const skjson::ObjectValue& jstyle,
                                                             sk_sp<sksg::RenderNode> layer) const {
    return make_shadow_effect(jstyle, *fBuilder, std::move(layer),
                              ShadowAdapter::Type::kDropShadow);
}

sk_sp<sksg::RenderNode> EffectBuilder::attachInnerShadowStyle(const skjson::ObjectValue& jstyle,
                                                              sk_sp<sksg::RenderNode> layer) const {
    return make_shadow_effect(jstyle, *fBuilder, std::move(layer),
                              ShadowAdapter::Type::kInnerShadow);
}

}  // namespace skottie::internal

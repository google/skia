/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/core/SkColorFilter.h"
#include "include/effects/SkImageFilters.h"
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
        // TODO: kDropShadow,
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
                  opacity = SkTPin(fOpacity, 0.0f, 1.0f);
        const auto  color = static_cast<SkColor4f>(fColor);
        const auto offset = SkV2{ fDistance * SkScalarCos(rad),
                                 -fDistance * SkScalarSin(rad)};

        const auto alpha_scale = opacity * color.fA *
                                (fType == Type::kInnerShadow ? -1.0f : 1.0f),
                   alpha_bias  = fType == Type::kInnerShadow ?  1.0f : 0.0f;

        // Select and colorize the source alpha channel.
        const float cm[] = {
            0, 0, 0,           0,   color.fR,
            0, 0, 0,           0,   color.fG,
            0, 0, 0,           0,   color.fB,
            0, 0, 0, alpha_scale, alpha_bias,
        };
        auto f = SkImageFilters::ColorFilter(SkColorFilters::Matrix(cm), nullptr);

        // Apply blur.
        f = SkImageFilters::Blur(sigma, sigma, std::move(f));

        // Optional offset.
        // TODO: SkOffsetImageFilter should handle no-ops.
        if (!SkScalarNearlyZero(offset.x) || !SkScalarNearlyZero(offset.y)) {
            f = SkImageFilters::Offset(offset.x, offset.y, std::move(f));
        }

        sk_sp<SkImageFilter> source; // nullptr == filter chain input

        switch (fType) {
        case Type::kInnerShadow: {
            // Inner shadows draw on top of, and are masked with, the source.
            f = SkImageFilters::Xfermode(SkBlendMode::kDstIn, std::move(f));

            std::swap(source, f);
        } break;
        }

        // Merge with source.
        f = SkImageFilters::Merge(std::move(f), std::move(source));

        this->node()->setImageFilter(std::move(f));
    }

    const Type fType;

    VectorValue fColor;
    ScalarValue fOpacity  = 1,
                fAngle    = 0,
                fSize     = 0,
                fDistance = 0;

    using INHERITED = DiscardableAdapterBase<ShadowAdapter, sksg::ExternalImageFilter>;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachInnerShadowStyle(const skjson::ObjectValue& jstyle,
                                                              sk_sp<sksg::RenderNode> layer) const {
    auto filter_node =
            fBuilder->attachDiscardableAdapter<ShadowAdapter>(jstyle,
                                                              *fBuilder,
                                                              ShadowAdapter::Type::kInnerShadow);
    return sksg::ImageFilterEffect::Make(std::move(layer), std::move(filter_node));
}

} // namespece skottie::internal

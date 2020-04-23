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
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderEffect.h"

namespace skottie::internal {

namespace  {

class GlowAdapter final : public DiscardableAdapterBase<GlowAdapter, sksg::ExternalImageFilter> {
public:
    enum Type {
        kOuterGlow,
        kInnerGlow,
    };

    GlowAdapter(const skjson::ObjectValue& jstyle, const AnimationBuilder& abuilder, Type type)
        : fType(type) {
        this->bind(abuilder, jstyle["c" ], fColor);
        this->bind(abuilder, jstyle["o" ], fOpacity);
        this->bind(abuilder, jstyle["s" ], fSize);
        this->bind(abuilder, jstyle["sr"], fInnerSource);
    }

private:
    void onSync() override {
        const auto sigma = fSize * kBlurSizeToSigma,
                 opacity = SkTPin(fOpacity / 100, 0.0f, 1.0f);
        const auto color = static_cast<SkColor4f>(fColor);

        // Select and colorize the source alpha channel.
        SkColorMatrix cm{0, 0, 0,                  0, color.fR,
                         0, 0, 0,                  0, color.fG,
                         0, 0, 0,                  0, color.fB,
                         0, 0, 0, opacity * color.fA,        0};

        // Inner glows with an edge source use the alpha inverse.
        if (fType == Type::kInnerGlow && SkScalarRoundToInt(fInnerSource) == kEdge) {
            cm.preConcat({1, 0, 0, 0, 0,
                          0, 1, 0, 0, 0,
                          0, 0, 1, 0, 0,
                          0, 0, 0,-1, 1});
        }
        auto f = SkImageFilters::ColorFilter(SkColorFilters::Matrix(cm), nullptr);

        if (sigma > 0) {
            f = SkImageFilters::Blur(sigma, sigma, std::move(f));
        }

        sk_sp<SkImageFilter> source;

        if (fType == Type::kInnerGlow) {
            // Inner glows draw on top of, and are masked with, the source.
            f = SkImageFilters::Xfermode(SkBlendMode::kDstIn, std::move(f));

            std::swap(source, f);
        }

        this->node()->setImageFilter(SkImageFilters::Merge(std::move(f),
                                                           std::move(source)));
    }

    enum InnerSource {
        kEdge   = 1,
        kCenter = 2,
    };

    const Type fType;

    VectorValue fColor;
    ScalarValue fOpacity  = 100, // percentage
                fSize     =   0,
             fInnerSource = kEdge;

    using INHERITED = DiscardableAdapterBase<GlowAdapter, sksg::ExternalImageFilter>;
};

static sk_sp<sksg::RenderNode> make_glow_effect(const skjson::ObjectValue& jstyle,
                                                const AnimationBuilder& abuilder,
                                                sk_sp<sksg::RenderNode> layer,
                                                GlowAdapter::Type type) {
    auto filter_node = abuilder.attachDiscardableAdapter<GlowAdapter>(jstyle, abuilder, type);

    return sksg::ImageFilterEffect::Make(std::move(layer), std::move(filter_node));
}

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachOuterGlowStyle(const skjson::ObjectValue& jstyle,
                                                            sk_sp<sksg::RenderNode> layer) const {
    return make_glow_effect(jstyle, *fBuilder, std::move(layer), GlowAdapter::Type::kOuterGlow);
}

sk_sp<sksg::RenderNode> EffectBuilder::attachInnerGlowStyle(const skjson::ObjectValue& jstyle,
                                                            sk_sp<sksg::RenderNode> layer) const {
    return make_glow_effect(jstyle, *fBuilder, std::move(layer), GlowAdapter::Type::kInnerGlow);
}

} // namespace skottie::internal

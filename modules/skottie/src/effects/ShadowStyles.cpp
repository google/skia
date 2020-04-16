/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

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
                    sigma = fSize * kBlurSizeToSigma;
        const auto offset = SkV2{ fDistance * SkScalarCos(rad),
                                 -fDistance * SkScalarSin(rad)};
        this->node()->setImageFilter(SkImageFilters::DropShadowOnly(offset.x, offset.y, sigma, sigma, fColor, nullptr));
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

/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/layers/shapelayer/ShapeLayer.h"
#include "modules/sksg/include/SkSGRoundEffect.h"

namespace skottie {
namespace internal {

namespace  {

class RoundCornersAdapter final : public DiscardableAdapterBase<RoundCornersAdapter,
                                                                sksg::RoundEffect> {
public:
    RoundCornersAdapter(const skjson::ObjectValue& jround,
                        const AnimationBuilder& abuilder,
                        sk_sp<sksg::GeometryNode> child)
        : INHERITED(sksg::RoundEffect::Make(std::move(child))) {
        this->bind(abuilder, jround["r"], fRadius);
    }

private:
    void onSync() override {
        this->node()->setRadius(fRadius);
    }

    ScalarValue fRadius = 0;

    using INHERITED = DiscardableAdapterBase<RoundCornersAdapter, sksg::RoundEffect>;
};

} // namespace

std::vector<sk_sp<sksg::GeometryNode>> ShapeBuilder::AttachRoundGeometryEffect(
        const skjson::ObjectValue& jround, const AnimationBuilder* abuilder,
        std::vector<sk_sp<sksg::GeometryNode>>&& geos) {
    std::vector<sk_sp<sksg::GeometryNode>> rounded;
    rounded.reserve(geos.size());

    for (auto& g : geos) {
        rounded.push_back(
            abuilder->attachDiscardableAdapter<RoundCornersAdapter>
                        (jround, *abuilder, std::move(g)));
    }

    return rounded;
}

} // namespace internal
} // namespace skottie

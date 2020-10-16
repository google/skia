/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTPin.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/layers/shapelayer/ShapeLayer.h"
#include "modules/sksg/include/SkSGGeometryEffect.h"

namespace skottie::internal {

namespace  {

class OffsetPathsAdapter final : public DiscardableAdapterBase<OffsetPathsAdapter,
                                                               sksg::OffsetEffect> {
public:
    OffsetPathsAdapter(const skjson::ObjectValue& joffset,
                       const AnimationBuilder& abuilder,
                       sk_sp<sksg::GeometryNode> child)
        : INHERITED(sksg::OffsetEffect::Make(std::move(child))) {
        static constexpr SkPaint::Join gJoinMap[] = {
            SkPaint::kMiter_Join,  // 'lj': 1
            SkPaint::kRound_Join,  // 'lj': 2
            SkPaint::kBevel_Join,  // 'lj': 3
        };

        const auto join = ParseDefault<int>(joffset["lj"], 1) - 1;
        this->node()->setJoin(gJoinMap[SkTPin<int>(join, 0, SK_ARRAY_COUNT(gJoinMap) - 1)]);

        this->bind(abuilder, joffset["a" ], fAmount);
        this->bind(abuilder, joffset["ml"], fMiterLimit);
    }

private:
    void onSync() override {
        this->node()->setOffset(fAmount);
        this->node()->setMiterLimit(fMiterLimit);
    }

    ScalarValue fAmount     = 0,
                fMiterLimit = 0;

    using INHERITED = DiscardableAdapterBase<OffsetPathsAdapter, sksg::OffsetEffect>;
};

} // namespace

std::vector<sk_sp<sksg::GeometryNode>> ShapeBuilder::AttachOffsetGeometryEffect(
        const skjson::ObjectValue& jround, const AnimationBuilder* abuilder,
        std::vector<sk_sp<sksg::GeometryNode>>&& geos) {
    std::vector<sk_sp<sksg::GeometryNode>> offsetted;
    offsetted.reserve(geos.size());

    for (auto& g : geos) {
        offsetted.push_back(abuilder->attachDiscardableAdapter<OffsetPathsAdapter>
                                        (jround, *abuilder, std::move(g)));
    }

    return offsetted;
}

} // namespace skottie::internal

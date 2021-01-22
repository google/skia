/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRRect.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/layers/shapelayer/ShapeLayer.h"
#include "modules/sksg/include/SkSGRect.h"

namespace skottie {
namespace internal {

namespace  {

class RectangleGeometryAdapter final :
        public DiscardableAdapterBase<RectangleGeometryAdapter, sksg::RRect> {
public:
    RectangleGeometryAdapter(const skjson::ObjectValue& jrect,
                             const AnimationBuilder* abuilder) {
        this->node()->setDirection(ParseDefault(jrect["d"], -1) == 3 ? SkPathDirection::kCCW
                                                                     : SkPathDirection::kCW);
        this->node()->setInitialPointIndex(2); // starting point: (Right, Top - radius.y)

        this->bind(*abuilder, jrect["s"], fSize     );
        this->bind(*abuilder, jrect["p"], fPosition );
        this->bind(*abuilder, jrect["r"], fRoundness);
    }

private:
    void onSync() override {
        const auto bounds = SkRect::MakeXYWH(fPosition.x - fSize.x / 2,
                                             fPosition.y - fSize.y / 2,
                                             fSize.x, fSize.y);

        this->node()->setRRect(SkRRect::MakeRectXY(bounds, fRoundness, fRoundness));
    }

    Vec2Value   fSize      = {0,0},
                fPosition  = {0,0}; // center
    ScalarValue fRoundness = 0;
};

} // namespace

sk_sp<sksg::GeometryNode> ShapeBuilder::AttachRRectGeometry(const skjson::ObjectValue& jrect,
                                                            const AnimationBuilder* abuilder) {
    return abuilder->attachDiscardableAdapter<RectangleGeometryAdapter>(jrect, abuilder);
}

} // namespace internal
} // namespace skottie

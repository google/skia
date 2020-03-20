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

class EllipseGeometryAdapter final :
        public DiscardableAdapterBase<EllipseGeometryAdapter, sksg::RRect> {
public:
    EllipseGeometryAdapter(const skjson::ObjectValue& jellipse,
                           const AnimationBuilder* abuilder) {
        this->node()->setDirection(ParseDefault(jellipse["d"], -1) == 3 ? SkPathDirection::kCCW
                                                                        : SkPathDirection::kCW);
        this->node()->setInitialPointIndex(1); // starting point: (Center, Top)

        this->bind(*abuilder, jellipse["s"], fSize);
        this->bind(*abuilder, jellipse["p"], fPosition);
    }

private:
    void onSync() override {
        const auto bounds = SkRect::MakeXYWH(fPosition.x - fSize.x / 2,
                                             fPosition.y - fSize.y / 2,
                                             fSize.x, fSize.y);

        this->node()->setRRect(SkRRect::MakeOval(bounds));
    }

    Vec2Value fSize     = {0,0},
              fPosition = {0,0}; // center
};

} // namespace

sk_sp<sksg::GeometryNode> ShapeBuilder::AttachEllipseGeometry(const skjson::ObjectValue& jellipse,
                                                              const AnimationBuilder* abuilder) {
    return abuilder->attachDiscardableAdapter<EllipseGeometryAdapter>(jellipse, abuilder);
}

} // namespace internal
} // namespace skottie

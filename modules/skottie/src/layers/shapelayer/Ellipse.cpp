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

        this->bind(*abuilder, jellipse["s"], &fSize);
        this->bind(*abuilder, jellipse["p"], &fPosition);
    }

private:
    void onSync() override {
        const auto size   = ValueTraits<VectorValue>::As<SkSize >(fSize);
        const auto center = ValueTraits<VectorValue>::As<SkPoint>(fPosition);

        const auto bounds = SkRect::MakeXYWH(center.x() - size.width()  / 2,
                                             center.y() - size.height() / 2,
                                             size.width(), size.height());

        this->node()->setRRect(SkRRect::MakeOval(bounds));
    }

    VectorValue fSize,
                fPosition;
};

} // namespace

sk_sp<sksg::GeometryNode> ShapeBuilder::AttachEllipseGeometry(const skjson::ObjectValue& jellipse,
                                                              const AnimationBuilder* abuilder) {
    return abuilder->attachDiscardableAdapter<EllipseGeometryAdapter, sk_sp<sksg::GeometryNode>>
                (jellipse, abuilder);
}

} // namespace internal
} // namespace skottie

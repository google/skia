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

        this->bind(*abuilder, jrect["s"], &fSize);
        this->bind(*abuilder, jrect["p"], &fPosition);
        this->bind(*abuilder, jrect["r"], &fRoundness);
    }

private:
    void onSync() override {
        const auto size   = ValueTraits<VectorValue>::As<SkSize >(fSize);
        const auto center = ValueTraits<VectorValue>::As<SkPoint>(fPosition);

        const auto bounds = SkRect::MakeXYWH(center.x() - size.width()  / 2,
                                             center.y() - size.height() / 2,
                                             size.width(), size.height());

        this->node()->setRRect(SkRRect::MakeRectXY(bounds, fRoundness, fRoundness));
    }

    VectorValue fSize,
                fPosition;
    ScalarValue fRoundness = 0;
};

} // namespace

sk_sp<sksg::GeometryNode> ShapeBuilder::AttachRRectGeometry(const skjson::ObjectValue& jrect,
                                                            const AnimationBuilder* abuilder) {
    return abuilder->attachDiscardableAdapter<RectangleGeometryAdapter, sk_sp<sksg::GeometryNode>>
                (jrect, abuilder);
}

} // namespace internal
} // namespace skottie

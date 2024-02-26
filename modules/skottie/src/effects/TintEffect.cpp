/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <cstddef>
#include <utility>

namespace skjson {
class ArrayValue;
}

namespace skottie {
namespace internal {

namespace  {

class TintAdapter final : public AnimatablePropertyContainer {
public:
    static sk_sp<TintAdapter> Make(const skjson::ArrayValue& jprops,
                                   sk_sp<sksg::RenderNode> layer,
                                   const AnimationBuilder& abuilder) {
        return sk_sp<TintAdapter>(new TintAdapter(jprops, std::move(layer), abuilder));
    }

    const auto& node() const { return fFilterNode; }

private:
    TintAdapter(const skjson::ArrayValue& jprops,
                sk_sp<sksg::RenderNode> layer,
                const AnimationBuilder& abuilder)
        : fColorNode0(sksg::Color::Make(SK_ColorBLACK))
        , fColorNode1(sksg::Color::Make(SK_ColorBLACK))
        , fFilterNode(sksg::GradientColorFilter::Make(std::move(layer), fColorNode0, fColorNode1)) {

        enum : size_t {
            kMapBlackTo_Index = 0,
            kMapWhiteTo_Index = 1,
            kAmount_Index     = 2,
            // kOpacity_Index    = 3, // currently unused (not exported)

            kMax_Index        = kAmount_Index,
        };

        EffectBinder(jprops, abuilder, this)
            .bind(kMapBlackTo_Index, fMapBlackTo)
            .bind(kMapWhiteTo_Index, fMapWhiteTo)
            .bind(    kAmount_Index, fAmount    );
    }

    void onSync() override {
        fColorNode0->setColor(fMapBlackTo);
        fColorNode1->setColor(fMapWhiteTo);

        fFilterNode->setWeight(fAmount / 100); // 100-based
    }

    const sk_sp<sksg::Color>               fColorNode0,
                                           fColorNode1;
    const sk_sp<sksg::GradientColorFilter> fFilterNode;

    ColorValue  fMapBlackTo,
                fMapWhiteTo;
    ScalarValue fAmount = 0;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachTintEffect(const skjson::ArrayValue& jprops,
                                                        sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<TintAdapter>(jprops, std::move(layer), *fBuilder);
}

} // namespace internal
} // namespace skottie

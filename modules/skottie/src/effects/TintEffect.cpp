/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

sk_sp<sksg::RenderNode> EffectBuilder::attachTintEffect(const skjson::ArrayValue& jprops,
                                                        sk_sp<sksg::RenderNode> layer) const {
    enum : size_t {
        kMapBlackTo_Index = 0,
        kMapWhiteTo_Index = 1,
        kAmount_Index     = 2,
        // kOpacity_Index    = 3, // currently unused (not exported)

        kMax_Index        = kAmount_Index,
    };

    if (jprops.size() <= kMax_Index) {
        return nullptr;
    }

    const skjson::ObjectValue* color0_prop = jprops[kMapBlackTo_Index];
    const skjson::ObjectValue* color1_prop = jprops[kMapWhiteTo_Index];
    const skjson::ObjectValue* amount_prop = jprops[    kAmount_Index];

    if (!color0_prop || !color1_prop || !amount_prop) {
        return nullptr;
    }

    auto tint_node =
            sksg::GradientColorFilter::Make(std::move(layer),
                                            fBuilder->attachColor(*color0_prop, fScope, "v"),
                                            fBuilder->attachColor(*color1_prop, fScope, "v"));
    if (!tint_node) {
        return nullptr;
    }

    fBuilder->bindProperty<ScalarValue>((*amount_prop)["v"], fScope,
        [tint_node](const ScalarValue& w) {
            tint_node->setWeight(w / 100); // 100-based
        });

    return std::move(tint_node);
}

} // namespace internal
} // namespace skottie

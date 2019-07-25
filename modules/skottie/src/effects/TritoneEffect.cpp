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

sk_sp<sksg::RenderNode> EffectBuilder::attachTritoneEffect(const skjson::ArrayValue& jprops,
                                                           sk_sp<sksg::RenderNode> layer) const {
    enum : size_t {
        kHiColor_Index     = 0,
        kMiColor_Index     = 1,
        kLoColor_Index     = 2,
        kBlendAmount_Index = 3,

        kMax_Index         = kBlendAmount_Index,
    };

    if (jprops.size() <= kMax_Index) {
        return nullptr;
    }

    const skjson::ObjectValue* hicolor_prop = jprops[    kHiColor_Index];
    const skjson::ObjectValue* micolor_prop = jprops[    kMiColor_Index];
    const skjson::ObjectValue* locolor_prop = jprops[    kLoColor_Index];
    const skjson::ObjectValue*   blend_prop = jprops[kBlendAmount_Index];

    if (!hicolor_prop || !micolor_prop || !locolor_prop || !blend_prop) {
        return nullptr;
    }

    auto tritone_node =
            sksg::GradientColorFilter::Make(std::move(layer), {
                                            fBuilder->attachColor(*locolor_prop, "v"),
                                            fBuilder->attachColor(*micolor_prop, "v"),
                                            fBuilder->attachColor(*hicolor_prop, "v") });
    if (!tritone_node) {
        return nullptr;
    }

    fBuilder->bindProperty<ScalarValue>((*blend_prop)["v"],
        [tritone_node](const ScalarValue& w) {
            tritone_node->setWeight((100 - w) / 100); // 100-based, inverted (!?).
        });

    return std::move(tritone_node);
}

} // namespace internal
} // namespace skottie

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

sk_sp<sksg::RenderNode> EffectBuilder::attachFillEffect(const skjson::ArrayValue& jprops,
                                                        sk_sp<sksg::RenderNode> layer) const {
    enum : size_t {
        kFillMask_Index = 0,
        kAllMasks_Index = 1,
        kColor_Index    = 2,
        kInvert_Index   = 3,
        kHFeather_Index = 4,
        kVFeather_Index = 5,
        kOpacity_Index  = 6,

        kMax_Index      = kOpacity_Index,
    };

    if (jprops.size() <= kMax_Index) {
        return nullptr;
    }

    const skjson::ObjectValue*   color_prop = jprops[  kColor_Index];
    const skjson::ObjectValue* opacity_prop = jprops[kOpacity_Index];
    if (!color_prop || !opacity_prop) {
        return nullptr;
    }
    sk_sp<sksg::Color> color_node = fBuilder->attachColor(*color_prop, fScope, "v");
    if (!color_node) {
        return nullptr;
    }

    fBuilder->bindProperty<ScalarValue>((*opacity_prop)["v"], fScope,
        [color_node](const ScalarValue& o) {
            const auto c = color_node->getColor();
            const auto a = sk_float_round2int_no_saturate(SkTPin(o, 0.0f, 1.0f) * 255);
            color_node->setColor(SkColorSetA(c, a));
        });

    return sksg::ModeColorFilter::Make(std::move(layer),
                                       std::move(color_node),
                                       SkBlendMode::kSrcIn);
}

} // namespace internal
} // namespace skottie

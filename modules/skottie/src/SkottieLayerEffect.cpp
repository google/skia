/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottiePriv.h"

#include "SkJSON.h"
#include "SkottieJson.h"
#include "SkottieValue.h"
#include "SkSGColor.h"
#include "SkSGColorFilter.h"

namespace skottie {
namespace internal {

namespace {

sk_sp<sksg::RenderNode> AttachFillLayerEffect(const skjson::ArrayValue* jeffect_props,
                                              const AnimationBuilder* abuilder,
                                              AnimatorScope* ascope,
                                              sk_sp<sksg::RenderNode> layer) {
    if (!jeffect_props) return nullptr;

    // Effect properties are index-based.
    enum {
        kFillMask_Index = 0,
        kAllMasks_Index = 1,
        kColor_Index    = 2,
        kInvert_Index   = 3,
        kHFeather_Index = 4,
        kVFeather_Index = 5,
        kOpacity_Index  = 6,

        kMax_Index      = kOpacity_Index,
    };

    if (jeffect_props->size() <= kMax_Index) {
        return nullptr;
    }

    const skjson::ObjectValue*   color_prop = (*jeffect_props)[  kColor_Index];
    const skjson::ObjectValue* opacity_prop = (*jeffect_props)[kOpacity_Index];
    if (!color_prop || !opacity_prop) {
        return nullptr;
    }

    sk_sp<sksg::Color> color_node = abuilder->attachColor(*color_prop, ascope, "v");
    if (!color_node) {
        return nullptr;
    }

    abuilder->bindProperty<ScalarValue>((*opacity_prop)["v"], ascope,
        [color_node](const ScalarValue& o) {
            const auto c = color_node->getColor();
            const auto a = sk_float_round2int_no_saturate(SkTPin(o, 0.0f, 1.0f) * 255);
            color_node->setColor(SkColorSetA(c, a));
        });

    return sksg::ColorModeFilter::Make(std::move(layer),
                                       std::move(color_node),
                                       SkBlendMode::kSrcIn);
}

} // namespace

sk_sp<sksg::RenderNode> AnimationBuilder::attachLayerEffects(const skjson::ArrayValue& jeffects,
                                                             AnimatorScope* ascope,
                                                             sk_sp<sksg::RenderNode> layer) const {
    for (const skjson::ObjectValue* jeffect : jeffects) {
        if (!jeffect) continue;

        switch (const auto ty = ParseDefault<int>((*jeffect)["ty"], -1)) {
        case 21: // Fill
            layer = AttachFillLayerEffect((*jeffect)["ef"], this, ascope, std::move(layer));
            break;
        default:
            this->log(Logger::Level::kWarning, nullptr, "Unsupported layer effect type: %d.", ty);
            break;
        }

        if (!layer) {
            this->log(Logger::Level::kError, jeffect, "Invalid layer effect.");
            return nullptr;
        }
    }

    return layer;
}

} // namespace internal
} // namespace skottie

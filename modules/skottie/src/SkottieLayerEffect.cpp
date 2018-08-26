/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottiePriv.h"

#include "SkJSON.h"
#include "SkottieJson.h"
#include "SkSGColor.h"
#include "SkSGColorFilter.h"

namespace skottie {
namespace internal {

namespace {

sk_sp<sksg::RenderNode> AttachFillLayerEffect(const skjson::ArrayValue* jeffect_props,
                                              const AnimationBuilder* abuilder,
                                              AnimatorScope* ascope,
                                              sk_sp<sksg::RenderNode> layer) {
    if (!jeffect_props) return layer;

    sk_sp<sksg::Color> color_node;

    for (const skjson::ObjectValue* jprop : *jeffect_props) {
        if (!jprop) continue;

        switch (const auto ty = ParseDefault<int>((*jprop)["ty"], -1)) {
        case 2: // color
            color_node = abuilder->attachColor(*jprop, ascope, "v");
            break;
        default:
            LOG("?? Ignoring unsupported fill effect poperty type: %d\n", ty);
            break;
        }
    }

    return color_node
        ? sksg::ColorModeFilter::Make(std::move(layer), std::move(color_node), SkBlendMode::kSrcIn)
        : nullptr;
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
            LOG("?? Unsupported layer effect type: %d\n", ty);
            break;
        }
    }

    return layer;
}

} // namespace internal
} // namespace skottie

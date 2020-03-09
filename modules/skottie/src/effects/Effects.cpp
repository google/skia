/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "modules/skottie/src/SkottieJson.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

EffectBuilder::EffectBuilder(const AnimationBuilder* abuilder, const SkSize& layer_size)
    : fBuilder(abuilder)
    , fLayerSize(layer_size) {}

EffectBuilder::EffectBuilderT EffectBuilder::findBuilder(const skjson::ObjectValue& jeffect) const {
    // First, try assigned types.
    enum : int32_t {
        kTint_Effect         = 20,
        kFill_Effect         = 21,
        kTritone_Effect      = 23,
        kDropShadow_Effect   = 25,
        kRadialWipe_Effect   = 26,
        kGaussianBlur_Effect = 29,
    };

    const auto ty = ParseDefault<int>(jeffect["ty"], -1);

    switch (ty) {
    case kTint_Effect:
        return &EffectBuilder::attachTintEffect;
    case kFill_Effect:
        return &EffectBuilder::attachFillEffect;
    case kTritone_Effect:
        return &EffectBuilder::attachTritoneEffect;
    case kDropShadow_Effect:
        return &EffectBuilder::attachDropShadowEffect;
    case kRadialWipe_Effect:
        return &EffectBuilder::attachRadialWipeEffect;
    case kGaussianBlur_Effect:
        return &EffectBuilder::attachGaussianBlurEffect;
    default:
        break;
    }

    // Some effects don't have an assigned type, but the data is still present.
    // Try a name-based lookup.

    static constexpr char kGradientEffectMN[] = "ADBE Ramp",
                           kHueSaturationMN[] = "ADBE HUE SATURATION",
                            kLevelsEffectMN[] = "ADBE Easy Levels2",
                        kLinearWipeEffectMN[] = "ADBE Linear Wipe",
                        kMotionTileEffectMN[] = "ADBE Tile",
                         kTransformEffectMN[] = "ADBE Geometry2",
                    kVenetianBlindsEffectMN[] = "ADBE Venetian Blinds",
                     kShiftChannelsEffectMN[] = "ADBE Shift Channels";

    if (const skjson::StringValue* mn = jeffect["mn"]) {
        if (!strcmp(mn->begin(), kGradientEffectMN)) {
            return &EffectBuilder::attachGradientEffect;
        }
        if (!strcmp(mn->begin(), kHueSaturationMN)) {
            return &EffectBuilder::attachHueSaturationEffect;
        }
        if (!strcmp(mn->begin(), kLevelsEffectMN)) {
            return &EffectBuilder::attachLevelsEffect;
        }
        if (!strcmp(mn->begin(), kLinearWipeEffectMN)) {
            return &EffectBuilder::attachLinearWipeEffect;
        }
        if (!strcmp(mn->begin(), kMotionTileEffectMN)) {
            return &EffectBuilder::attachMotionTileEffect;
        }
        if (!strcmp(mn->begin(), kTransformEffectMN)) {
            return &EffectBuilder::attachTransformEffect;
        }
        if (!strcmp(mn->begin(), kVenetianBlindsEffectMN)) {
            return &EffectBuilder::attachVenetianBlindsEffect;
        }
        if (!strcmp(mn->begin(), kShiftChannelsEffectMN)) {
            return &EffectBuilder::attachShiftChannelsEffect;
        }
    }

    fBuilder->log(Logger::Level::kWarning, nullptr, "Unsupported layer effect type: %d.", ty);

    return nullptr;
}

sk_sp<sksg::RenderNode> EffectBuilder::attachEffects(const skjson::ArrayValue& jeffects,
                                                     sk_sp<sksg::RenderNode> layer) const {
    if (!layer) {
        return nullptr;
    }

    for (const skjson::ObjectValue* jeffect : jeffects) {
        if (!jeffect) {
            continue;
        }

        const auto builder = this->findBuilder(*jeffect);
        const skjson::ArrayValue* jprops = (*jeffect)["ef"];
        if (!builder || !jprops) {
            continue;
        }

        const AnimationBuilder::AutoPropertyTracker apt(fBuilder, *jeffect);
        layer = (this->*builder)(*jprops, std::move(layer));

        if (!layer) {
            fBuilder->log(Logger::Level::kError, jeffect, "Invalid layer effect.");
            return nullptr;
        }
    }

    return layer;
}

const skjson::Value& EffectBuilder::GetPropValue(const skjson::ArrayValue& jprops,
                                                 size_t prop_index) {
    static skjson::NullValue kNull;

    if (prop_index >= jprops.size()) {
        return kNull;
    }

    const skjson::ObjectValue* jprop = jprops[prop_index];

    return jprop ? (*jprop)["v"] : kNull;
}

MaskFilterEffectBase::MaskFilterEffectBase(sk_sp<sksg::RenderNode> child, const SkSize& ls)
    : fMaskNode(sksg::MaskFilter::Make(nullptr))
    , fMaskEffectNode(sksg::MaskFilterEffect::Make(std::move(child), fMaskNode))
    , fLayerSize(ls) {}

MaskFilterEffectBase::~MaskFilterEffectBase() = default;

void MaskFilterEffectBase::apply() const {
    const auto minfo = this->onMakeMask();

    fMaskEffectNode->setVisible(minfo.fVisible);
    fMaskNode->setMaskFilter(std::move(minfo.fMask));
}

} // namespace internal
} // namespace skottie

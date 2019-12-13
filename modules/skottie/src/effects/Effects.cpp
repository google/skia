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

#include <algorithm>
#include <iterator>

namespace skottie {
namespace internal {

EffectBuilder::EffectBuilder(const AnimationBuilder* abuilder, const SkSize& layer_size)
    : fBuilder(abuilder)
    , fLayerSize(layer_size) {}

EffectBuilder::EffectBuilderT EffectBuilder::findBuilder(const skjson::ObjectValue& jeffect) const {
    static constexpr struct BuilderInfo {
        const char*    fName;
        EffectBuilderT fBuilder;
    } gBuilderInfo[] = {
        { "ADBE Drop Shadow"    , &EffectBuilder::attachDropShadowEffect     },
        { "ADBE Easy Levels2"   , &EffectBuilder::attachLevelsEffect         },
        { "ADBE Fill"           , &EffectBuilder::attachFillEffect           },
        { "ADBE Gaussian Blur 2", &EffectBuilder::attachGaussianBlurEffect   },
        { "ADBE Geometry2"      , &EffectBuilder::attachTransformEffect      },
        { "ADBE HUE SATURATION" , &EffectBuilder::attachHueSaturationEffect  },
        { "ADBE Invert"         , &EffectBuilder::attachInvertEffect         },
        { "ADBE Linear Wipe"    , &EffectBuilder::attachLinearWipeEffect     },
        { "ADBE Radial Wipe"    , &EffectBuilder::attachRadialWipeEffect     },
        { "ADBE Ramp"           , &EffectBuilder::attachGradientEffect       },
        { "ADBE Shift Channels" , &EffectBuilder::attachShiftChannelsEffect  },
        { "ADBE Tile"           , &EffectBuilder::attachMotionTileEffect     },
        { "ADBE Tint"           , &EffectBuilder::attachTintEffect           },
        { "ADBE Tritone"        , &EffectBuilder::attachTritoneEffect        },
        { "ADBE Venetian Blinds", &EffectBuilder::attachVenetianBlindsEffect },
    };

    const skjson::StringValue* mn = jeffect["mn"];
    if (!mn) {
        return nullptr;
    }

    const BuilderInfo key { mn->begin(), nullptr };
    const auto* binfo = std::lower_bound(std::begin(gBuilderInfo),
                                         std::end  (gBuilderInfo),
                                         key,
                                         [](const BuilderInfo& a, const BuilderInfo& b) {
                                             return strcmp(a.fName, b.fName) < 0;
                                         });

    if (binfo == std::end(gBuilderInfo) || strcmp(binfo->fName, key.fName)) {
        fBuilder->log(Logger::Level::kWarning, nullptr,
                      "Unsupported layer effect: %s.", mn->begin());
        return nullptr;
    }

    return binfo->fBuilder;
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

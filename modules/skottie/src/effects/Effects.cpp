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
        { "ADBE Easy Levels2"   , &EffectBuilder::attachEasyLevelsEffect     },
        { "ADBE Fill"           , &EffectBuilder::attachFillEffect           },
        { "ADBE Gaussian Blur 2", &EffectBuilder::attachGaussianBlurEffect   },
        { "ADBE Geometry2"      , &EffectBuilder::attachTransformEffect      },
        { "ADBE HUE SATURATION" , &EffectBuilder::attachHueSaturationEffect  },
        { "ADBE Invert"         , &EffectBuilder::attachInvertEffect         },
        { "ADBE Linear Wipe"    , &EffectBuilder::attachLinearWipeEffect     },
        { "ADBE Pro Levels2"    , &EffectBuilder::attachProLevelsEffect      },
        { "ADBE Radial Wipe"    , &EffectBuilder::attachRadialWipeEffect     },
        { "ADBE Ramp"           , &EffectBuilder::attachGradientEffect       },
        { "ADBE Shift Channels" , &EffectBuilder::attachShiftChannelsEffect  },
        { "ADBE Tile"           , &EffectBuilder::attachMotionTileEffect     },
        { "ADBE Tint"           , &EffectBuilder::attachTintEffect           },
        { "ADBE Tritone"        , &EffectBuilder::attachTritoneEffect        },
        { "ADBE Venetian Blinds", &EffectBuilder::attachVenetianBlindsEffect },
    };

    const skjson::StringValue* mn = jeffect["mn"];
    if (mn) {
        const BuilderInfo key { mn->begin(), nullptr };
        const auto* binfo = std::lower_bound(std::begin(gBuilderInfo),
                                             std::end  (gBuilderInfo),
                                             key,
                                             [](const BuilderInfo& a, const BuilderInfo& b) {
                                                 return strcmp(a.fName, b.fName) < 0;
                                             });

        if (binfo != std::end(gBuilderInfo) && !strcmp(binfo->fName, key.fName)) {
            return binfo->fBuilder;
        }
    }

    // Some legacy clients rely solely on the 'ty' field and generate (non-BM) JSON
    // without a valid 'mn' string.  TODO: we should update them and remove this fallback.
    enum : int32_t {
        kTint_Effect         = 20,
        kFill_Effect         = 21,
        kTritone_Effect      = 23,
        kDropShadow_Effect   = 25,
        kRadialWipe_Effect   = 26,
        kGaussianBlur_Effect = 29,
    };

    switch (ParseDefault<int>(jeffect["ty"], -1)) {
        case         kTint_Effect: return &EffectBuilder::attachTintEffect;
        case         kFill_Effect: return &EffectBuilder::attachFillEffect;
        case      kTritone_Effect: return &EffectBuilder::attachTritoneEffect;
        case   kDropShadow_Effect: return &EffectBuilder::attachDropShadowEffect;
        case   kRadialWipe_Effect: return &EffectBuilder::attachRadialWipeEffect;
        case kGaussianBlur_Effect: return &EffectBuilder::attachGaussianBlurEffect;
        default: break;
    }

    fBuilder->log(Logger::Level::kWarning, &jeffect,
                  "Unsupported layer effect: %s", mn ? mn->begin() : "(unknown)");

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

sk_sp<sksg::RenderNode> EffectBuilder::attachStyles(const skjson::ArrayValue& jstyles,
                                                     sk_sp<sksg::RenderNode> layer) const {
#if !defined(SKOTTIE_DISABLE_STYLES)
    if (!layer) {
        return nullptr;
    }

    using StyleBuilder =
        sk_sp<sksg::RenderNode> (EffectBuilder::*)(const skjson::ObjectValue&,
                                                   sk_sp<sksg::RenderNode>) const;
    static constexpr StyleBuilder gStyleBuilders[] = {
        nullptr,                                 // 'ty': 0 -> stroke
        &EffectBuilder::attachDropShadowStyle,   // 'ty': 1 -> drop shadow
    };

    for (const skjson::ObjectValue* jstyle : jstyles) {
        if (!jstyle) {
            continue;
        }

        const auto style_type =
                ParseDefault<size_t>((*jstyle)["ty"], std::numeric_limits<size_t>::max());
        auto builder = style_type < SK_ARRAY_COUNT(gStyleBuilders) ? gStyleBuilders[style_type]
                                                                   : nullptr;

        if (!builder) {
            fBuilder->log(Logger::Level::kWarning, jstyle, "Unsupported layer style.");
            continue;
        }

        layer = (this->*builder)(*jstyle, std::move(layer));
    }
#endif // !defined(SKOTTIE_DISABLE_STYLES)

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

MaskShaderEffectBase::MaskShaderEffectBase(sk_sp<sksg::RenderNode> child, const SkSize& ls)
    : fMaskEffectNode(sksg::MaskShaderEffect::Make(std::move(child)))
    , fLayerSize(ls) {}

void MaskShaderEffectBase::onSync() {
    const auto minfo = this->onMakeMask();

    fMaskEffectNode->setVisible(minfo.fVisible);
    fMaskEffectNode->setShader(std::move(minfo.fMaskShader));
}

} // namespace internal
} // namespace skottie

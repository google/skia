/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

namespace {

sk_sp<sksg::RenderNode> AttachGradientLayerEffect(const skjson::ArrayValue& jprops,
                                                  const AnimationBuilder* abuilder,
                                                  AnimatorScope* ascope,
                                                  sk_sp<sksg::RenderNode> layer) {
    enum : size_t {
        kStartPoint_Index  = 0,
        kStartColor_Index  = 1,
        kEndPoint_Index    = 2,
        kEndColor_Index    = 3,
        kRampShape_Index   = 4,
        kRampScatter_Index = 5,
        kBlendRatio_Index  = 6,

        kMax_Index        = kBlendRatio_Index,
    };

    if (jprops.size() <= kMax_Index) {
        return nullptr;
    }

    const skjson::ObjectValue* p0 = jprops[ kStartPoint_Index];
    const skjson::ObjectValue* p1 = jprops[   kEndPoint_Index];
    const skjson::ObjectValue* c0 = jprops[ kStartColor_Index];
    const skjson::ObjectValue* c1 = jprops[   kEndColor_Index];
    const skjson::ObjectValue* sh = jprops[  kRampShape_Index];
    const skjson::ObjectValue* bl = jprops[ kBlendRatio_Index];
    const skjson::ObjectValue* sc = jprops[kRampScatter_Index];

    if (!p0 || !p1 || !c0 || !c1 || !sh || !bl || !sc) {
        return nullptr;
    }

    auto adapter = sk_make_sp<GradientRampEffectAdapter>(std::move(layer));

    abuilder->bindProperty<VectorValue>((*p0)["v"], ascope,
        [adapter](const VectorValue& p0) {
            adapter->setStartPoint(ValueTraits<VectorValue>::As<SkPoint>(p0));
        });
    abuilder->bindProperty<VectorValue>((*p1)["v"], ascope,
        [adapter](const VectorValue& p1) {
            adapter->setEndPoint(ValueTraits<VectorValue>::As<SkPoint>(p1));
        });
    abuilder->bindProperty<VectorValue>((*c0)["v"], ascope,
        [adapter](const VectorValue& c0) {
            adapter->setStartColor(ValueTraits<VectorValue>::As<SkColor>(c0));
        });
    abuilder->bindProperty<VectorValue>((*c1)["v"], ascope,
        [adapter](const VectorValue& c1) {
            adapter->setEndColor(ValueTraits<VectorValue>::As<SkColor>(c1));
        });
    abuilder->bindProperty<ScalarValue>((*sh)["v"], ascope,
        [adapter](const ScalarValue& shape) {
            adapter->setShape(shape);
        });
    abuilder->bindProperty<ScalarValue>((*sh)["v"], ascope,
        [adapter](const ScalarValue& blend) {
            adapter->setBlend(blend);
        });
    abuilder->bindProperty<ScalarValue>((*sc)["v"], ascope,
        [adapter](const ScalarValue& scatter) {
            adapter->setScatter(scatter);
        });

    return adapter->root();
}

sk_sp<sksg::RenderNode> AttachTintLayerEffect(const skjson::ArrayValue& jprops,
                                              const AnimationBuilder* abuilder,
                                              AnimatorScope* ascope,
                                              sk_sp<sksg::RenderNode> layer) {
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
                                            abuilder->attachColor(*color0_prop, ascope, "v"),
                                            abuilder->attachColor(*color1_prop, ascope, "v"));
    if (!tint_node) {
        return nullptr;
    }

    abuilder->bindProperty<ScalarValue>((*amount_prop)["v"], ascope,
        [tint_node](const ScalarValue& w) {
            tint_node->setWeight(w / 100); // 100-based
        });

    return std::move(tint_node);
}

sk_sp<sksg::RenderNode> AttachTritoneLayerEffect(const skjson::ArrayValue& jprops,
                                                 const AnimationBuilder* abuilder,
                                                 AnimatorScope* ascope,
                                                 sk_sp<sksg::RenderNode> layer) {
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
                                            abuilder->attachColor(*locolor_prop, ascope, "v"),
                                            abuilder->attachColor(*micolor_prop, ascope, "v"),
                                            abuilder->attachColor(*hicolor_prop, ascope, "v") });
    if (!tritone_node) {
        return nullptr;
    }

    abuilder->bindProperty<ScalarValue>((*blend_prop)["v"], ascope,
        [tritone_node](const ScalarValue& w) {
            tritone_node->setWeight((100 - w) / 100); // 100-based, inverted (!?).
        });

    return std::move(tritone_node);
}

sk_sp<sksg::RenderNode> AttachFillLayerEffect(const skjson::ArrayValue& jprops,
                                              const AnimationBuilder* abuilder,
                                              AnimatorScope* ascope,
                                              sk_sp<sksg::RenderNode> layer) {
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

    return sksg::ModeColorFilter::Make(std::move(layer),
                                       std::move(color_node),
                                       SkBlendMode::kSrcIn);
}

sk_sp<sksg::RenderNode> AttachDropShadowLayerEffect(const skjson::ArrayValue& jprops,
                                                    const AnimationBuilder* abuilder,
                                                    AnimatorScope* ascope,
                                                    sk_sp<sksg::RenderNode> layer) {
    enum : size_t {
        kShadowColor_Index = 0,
        kOpacity_Index     = 1,
        kDirection_Index   = 2,
        kDistance_Index    = 3,
        kSoftness_Index    = 4,
        kShadowOnly_Index  = 5,

        kMax_Index         = kShadowOnly_Index,
    };

    if (jprops.size() <= kMax_Index) {
        return nullptr;
    }

    const skjson::ObjectValue*       color_prop = jprops[kShadowColor_Index];
    const skjson::ObjectValue*     opacity_prop = jprops[    kOpacity_Index];
    const skjson::ObjectValue*   direction_prop = jprops[  kDirection_Index];
    const skjson::ObjectValue*    distance_prop = jprops[   kDistance_Index];
    const skjson::ObjectValue*    softness_prop = jprops[   kSoftness_Index];
    const skjson::ObjectValue* shadow_only_prop = jprops[ kShadowOnly_Index];

    if (!color_prop ||
        !opacity_prop ||
        !direction_prop ||
        !distance_prop ||
        !softness_prop ||
        !shadow_only_prop) {
        return nullptr;
    }

    auto shadow_effect  = sksg::DropShadowImageFilter::Make();
    auto shadow_adapter = sk_make_sp<DropShadowEffectAdapter>(shadow_effect);

    abuilder->bindProperty<VectorValue>((*color_prop)["v"], ascope,
        [shadow_adapter](const VectorValue& c) {
            shadow_adapter->setColor(ValueTraits<VectorValue>::As<SkColor>(c));
        });
    abuilder->bindProperty<ScalarValue>((*opacity_prop)["v"], ascope,
        [shadow_adapter](const ScalarValue& o) {
            shadow_adapter->setOpacity(o);
        });
    abuilder->bindProperty<ScalarValue>((*direction_prop)["v"], ascope,
        [shadow_adapter](const ScalarValue& d) {
            shadow_adapter->setDirection(d);
        });
    abuilder->bindProperty<ScalarValue>((*distance_prop)["v"], ascope,
        [shadow_adapter](const ScalarValue& d) {
            shadow_adapter->setDistance(d);
        });
    abuilder->bindProperty<ScalarValue>((*softness_prop)["v"], ascope,
        [shadow_adapter](const ScalarValue& s) {
            shadow_adapter->setSoftness(s);
        });
    abuilder->bindProperty<ScalarValue>((*shadow_only_prop)["v"], ascope,
        [shadow_adapter](const ScalarValue& s) {
            shadow_adapter->setShadowOnly(SkToBool(s));
        });

    return sksg::ImageFilterEffect::Make(std::move(layer), std::move(shadow_effect));
}

sk_sp<sksg::RenderNode> AttachGaussianBlurLayerEffect(const skjson::ArrayValue& jprops,
                                                      const AnimationBuilder* abuilder,
                                                      AnimatorScope* ascope,
                                                      sk_sp<sksg::RenderNode> layer) {
    enum : size_t {
        kBlurriness_Index = 0,
        kDimensions_Index = 1,
        kRepeatEdge_Index = 2,

        kMax_Index        = kRepeatEdge_Index,
    };

    if (jprops.size() <= kMax_Index) {
        return nullptr;
    }

    const skjson::ObjectValue* blurriness_prop = jprops[kBlurriness_Index];
    const skjson::ObjectValue* dimensions_prop = jprops[kDimensions_Index];
    const skjson::ObjectValue* repeatedge_prop = jprops[kRepeatEdge_Index];

    if (!blurriness_prop || !dimensions_prop || !repeatedge_prop) {
        return nullptr;
    }

    auto blur_effect   = sksg::BlurImageFilter::Make();
    auto blur_addapter = sk_make_sp<GaussianBlurEffectAdapter>(blur_effect);

    abuilder->bindProperty<ScalarValue>((*blurriness_prop)["v"], ascope,
        [blur_addapter](const ScalarValue& b) {
            blur_addapter->setBlurriness(b);
        });
    abuilder->bindProperty<ScalarValue>((*dimensions_prop)["v"], ascope,
        [blur_addapter](const ScalarValue& d) {
            blur_addapter->setDimensions(d);
        });
    abuilder->bindProperty<ScalarValue>((*repeatedge_prop)["v"], ascope,
        [blur_addapter](const ScalarValue& r) {
            blur_addapter->setRepeatEdge(r);
        });

    return sksg::ImageFilterEffect::Make(std::move(layer), std::move(blur_effect));
}

sk_sp<sksg::RenderNode> AttachLevelsLayerEffect(const skjson::ArrayValue& jprops,
                                                const AnimationBuilder* abuilder,
                                                AnimatorScope* ascope,
                                                sk_sp<sksg::RenderNode> layer) {
    enum : size_t {
        kChannel_Index        = 0,
        // ???                = 1,
        kInputBlack_Index     = 2,
        kInputWhite_Index     = 3,
        kGamma_Index          = 4,
        kOutputBlack_Index    = 5,
        kOutputWhite_Index    = 6,
        kClipToOutBlack_Index = 7,
        kClipToOutWhite_Index = 8,

        kMax_Index        = kClipToOutWhite_Index,
    };

    if (jprops.size() <= kMax_Index) {
        return nullptr;
    }

    const skjson::ObjectValue*    channel_prop = jprops[       kChannel_Index];
    const skjson::ObjectValue*     iblack_prop = jprops[    kInputBlack_Index];
    const skjson::ObjectValue*     iwhite_prop = jprops[    kInputWhite_Index];
    const skjson::ObjectValue*      gamma_prop = jprops[         kGamma_Index];
    const skjson::ObjectValue*     oblack_prop = jprops[   kOutputBlack_Index];
    const skjson::ObjectValue*     owhite_prop = jprops[   kOutputWhite_Index];
    const skjson::ObjectValue* clip_black_prop = jprops[kClipToOutBlack_Index];
    const skjson::ObjectValue* clip_white_prop = jprops[kClipToOutWhite_Index];

    if (!channel_prop || !iblack_prop || !iwhite_prop || !gamma_prop ||
        !oblack_prop || !owhite_prop || !clip_black_prop || !clip_white_prop) {
        return nullptr;
    }

    auto adapter = sk_make_sp<LevelsEffectAdapter>(std::move(layer));

    abuilder->bindProperty<ScalarValue>((*channel_prop)["v"], ascope,
        [adapter](const ScalarValue& channel) {
            adapter->setChannel(channel);
        });
    abuilder->bindProperty<ScalarValue>((*iblack_prop)["v"], ascope,
        [adapter](const ScalarValue& ib) {
            adapter->setInBlack(ib);
        });
    abuilder->bindProperty<ScalarValue>((*iwhite_prop)["v"], ascope,
        [adapter](const ScalarValue& iw) {
            adapter->setInWhite(iw);
        });
    abuilder->bindProperty<ScalarValue>((*oblack_prop)["v"], ascope,
        [adapter](const ScalarValue& ob) {
            adapter->setOutBlack(ob);
        });
    abuilder->bindProperty<ScalarValue>((*owhite_prop)["v"], ascope,
        [adapter](const ScalarValue& ow) {
            adapter->setOutWhite(ow);
        });
    abuilder->bindProperty<ScalarValue>((*gamma_prop)["v"], ascope,
        [adapter](const ScalarValue& g) {
            adapter->setGamma(g);
        });

    abuilder->bindProperty<ScalarValue>((*clip_black_prop)["v"], ascope,
        [adapter](const ScalarValue& cb) {
            adapter->setClipBlack(cb);
        });
    abuilder->bindProperty<ScalarValue>((*clip_white_prop)["v"], ascope,
        [adapter](const ScalarValue& cw) {
            adapter->setClipWhite(cw);
        });

    return adapter->root();
}

using EffectBuilderT = sk_sp<sksg::RenderNode> (*)(const skjson::ArrayValue&,
                                                   const AnimationBuilder*,
                                                   AnimatorScope*,
                                                   sk_sp<sksg::RenderNode>);

EffectBuilderT FindEffectBuilder(const AnimationBuilder* abuilder,
                                 const skjson::ObjectValue& jeffect) {
    // First, try assigned types.
    enum : int32_t {
        kTint_Effect         = 20,
        kFill_Effect         = 21,
        kTritone_Effect      = 23,
        kDropShadow_Effect   = 25,
        kGaussianBlur_Effect = 29,
    };

    const auto ty = ParseDefault<int>(jeffect["ty"], -1);

    switch (ty) {
    case kTint_Effect:
        return AttachTintLayerEffect;
    case kFill_Effect:
        return AttachFillLayerEffect;
    case kTritone_Effect:
        return AttachTritoneLayerEffect;
    case kDropShadow_Effect:
        return AttachDropShadowLayerEffect;
    case kGaussianBlur_Effect:
        return AttachGaussianBlurLayerEffect;
    default:
        break;
    }

    // Some effects don't have an assigned type, but the data is still present.
    // Try a name-based lookup.

    if (const skjson::StringValue* mn = jeffect["mn"]) {
        if (!strcmp(mn->begin(), "ADBE Ramp")) {
            return AttachGradientLayerEffect;
        }
        if (!strcmp(mn->begin(), "ADBE Easy Levels2")) {
            return AttachLevelsLayerEffect;
        }
        if (!strcmp(mn->begin(), "ADBE Geometry2")) {
            return AttachTransformEffect;
        }
    }

    abuilder->log(Logger::Level::kWarning, nullptr, "Unsupported layer effect type: %d.", ty);

    return nullptr;
}

} // namespace

sk_sp<sksg::RenderNode> AnimationBuilder::attachLayerEffects(const skjson::ArrayValue& jeffects,
                                                             AnimatorScope* ascope,
                                                             sk_sp<sksg::RenderNode> layer) const {
    if (!layer) {
        return nullptr;
    }

    for (const skjson::ObjectValue* jeffect : jeffects) {
        if (!jeffect) {
            continue;
        }

        const auto builder = FindEffectBuilder(this, *jeffect);
        const skjson::ArrayValue* jprops = (*jeffect)["ef"];
        if (!builder || !jprops) {
            continue;
        }

        layer = builder(*jprops, this, ascope, std::move(layer));

        if (!layer) {
            this->log(Logger::Level::kError, jeffect, "Invalid layer effect.");
            return nullptr;
        }
    }

    return layer;
}

} // namespace internal
} // namespace skottie

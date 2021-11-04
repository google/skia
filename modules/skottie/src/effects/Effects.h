/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieEffects_DEFINED
#define SkottieEffects_DEFINED

#include "modules/skottie/src/Composition.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/animator/Animator.h"

class SkMaskFilter;

namespace sksg {
class MaskShaderEffect;
} // namespace sksg

namespace skottie {
namespace internal {

class EffectBuilder final : public SkNoncopyable {
public:
    EffectBuilder(const AnimationBuilder*, const SkSize&, CompositionBuilder*);

    sk_sp<sksg::RenderNode> attachEffects(const skjson::ArrayValue&,
                                          sk_sp<sksg::RenderNode>) const;

    sk_sp<sksg::RenderNode> attachStyles(const skjson::ArrayValue&,
                                         sk_sp<sksg::RenderNode>) const;

    static const skjson::Value& GetPropValue(const skjson::ArrayValue& jprops, size_t prop_index);

    LayerBuilder* getLayerBuilder(int layer_index) const {
        return fCompBuilder->layerBuilder(layer_index);
    }

private:
    using EffectBuilderT = sk_sp<sksg::RenderNode>(EffectBuilder::*)(const skjson::ArrayValue&,
                                                                     sk_sp<sksg::RenderNode>) const;

    sk_sp<sksg::RenderNode> attachBlackAndWhiteEffect     (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachBrightnessContrastEffect(const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachBulgeEffect            (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachCornerPinEffect         (const skjson::ArrayValue&,
                                                            sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachCCTonerEffect           (const skjson::ArrayValue&,
                                                            sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachDirectionalBlurEffect   (const skjson::ArrayValue&,
                                                            sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachDisplacementMapEffect   (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachDropShadowEffect        (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachFillEffect              (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachFractalNoiseEffect      (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachGaussianBlurEffect      (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachGradientEffect          (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachHueSaturationEffect     (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachInvertEffect            (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachEasyLevelsEffect        (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachLinearWipeEffect        (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachMotionTileEffect        (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachProLevelsEffect         (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachRadialWipeEffect        (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachShiftChannelsEffect     (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachSkSLEffect              (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachSphereEffect            (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachThresholdEffect         (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachTintEffect              (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachTransformEffect         (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachTritoneEffect           (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachVenetianBlindsEffect    (const skjson::ArrayValue&,
                                                           sk_sp<sksg::RenderNode>) const;

    sk_sp<sksg::RenderNode> attachDropShadowStyle(const skjson::ObjectValue&,
                                                  sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachInnerShadowStyle(const skjson::ObjectValue&,
                                                   sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachInnerGlowStyle(const skjson::ObjectValue&,
                                                 sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachOuterGlowStyle(const skjson::ObjectValue&,
                                                 sk_sp<sksg::RenderNode>) const;

    EffectBuilderT findBuilder(const skjson::ObjectValue&) const;

    const AnimationBuilder* fBuilder;
    CompositionBuilder*     fCompBuilder;
    const SkSize            fLayerSize;
};

// Syntactic sugar/helper.
class EffectBinder {
public:
    EffectBinder(const skjson::ArrayValue& jprops,
                 const AnimationBuilder& abuilder,
                 AnimatablePropertyContainer* acontainer)
        : fProps(jprops)
        , fBuilder(abuilder)
        , fContainer(acontainer) {}

    template <typename T>
    const EffectBinder& bind(size_t prop_index, T& value) const {
        fContainer->bind(fBuilder, EffectBuilder::GetPropValue(fProps, prop_index), value);

        return *this;
    }

private:
    const skjson::ArrayValue&    fProps;
    const AnimationBuilder&      fBuilder;
    AnimatablePropertyContainer* fContainer;
};

/**
 * Base class for mask-shader-related effects.
 */
class MaskShaderEffectBase : public AnimatablePropertyContainer {
public:
    const sk_sp<sksg::MaskShaderEffect>& node() const { return fMaskEffectNode; }

protected:
    MaskShaderEffectBase(sk_sp<sksg::RenderNode>, const SkSize&);

    const SkSize& layerSize() const { return  fLayerSize; }

    struct MaskInfo {
        sk_sp<SkShader> fMaskShader;
        bool            fVisible;
    };
    virtual MaskInfo onMakeMask() const = 0;

private:
    void onSync() final;

    const sk_sp<sksg::MaskShaderEffect> fMaskEffectNode;
    const SkSize                        fLayerSize;
};

} // namespace internal
} // namespace skottie

#endif // SkottieEffects_DEFINED

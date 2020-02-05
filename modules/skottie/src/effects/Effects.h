/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieEffects_DEFINED
#define SkottieEffects_DEFINED

#include "modules/skottie/src/Animator.h"
#include "modules/skottie/src/SkottiePriv.h"

class SkMaskFilter;

namespace sksg {
class MaskFilter;
class MaskFilterEffect;
} // namespace sksg

namespace skottie {
namespace internal {

class EffectBuilder final : public SkNoncopyable {
public:
    EffectBuilder(const AnimationBuilder*, const SkSize&);

    sk_sp<sksg::RenderNode> attachEffects(const skjson::ArrayValue&,
                                          sk_sp<sksg::RenderNode>) const;

    static const skjson::Value& GetPropValue(const skjson::ArrayValue& jprops, size_t prop_index);

private:
    using EffectBuilderT = sk_sp<sksg::RenderNode>(EffectBuilder::*)(const skjson::ArrayValue&,
                                                                     sk_sp<sksg::RenderNode>) const;

    sk_sp<sksg::RenderNode> attachDropShadowEffect    (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachFillEffect          (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachGaussianBlurEffect  (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachGradientEffect      (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachHueSaturationEffect (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachInvertEffect        (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachEasyLevelsEffect    (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachLinearWipeEffect    (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachMotionTileEffect    (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachProLevelsEffect     (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachRadialWipeEffect    (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachTintEffect          (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachTransformEffect     (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachTritoneEffect       (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachVenetianBlindsEffect(const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::RenderNode> attachShiftChannelsEffect (const skjson::ArrayValue&,
                                                       sk_sp<sksg::RenderNode>) const;

    EffectBuilderT findBuilder(const skjson::ObjectValue&) const;

    const AnimationBuilder*   fBuilder;
    const SkSize              fLayerSize;
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
 * Base class for mask-filter-related effects.
 */
class MaskFilterEffectBase : public AnimatablePropertyContainer {
public:
    const sk_sp<sksg::MaskFilterEffect>& node() const { return fMaskEffectNode; }

protected:
    MaskFilterEffectBase(sk_sp<sksg::RenderNode>, const SkSize&);

    const SkSize& layerSize() const { return  fLayerSize; }

    struct MaskInfo {
        sk_sp<SkMaskFilter> fMask;
        bool                fVisible;
    };
    virtual MaskInfo onMakeMask() const = 0;

private:
    void onSync() final;

    const sk_sp<sksg::MaskFilter>       fMaskNode;
    const sk_sp<sksg::MaskFilterEffect> fMaskEffectNode;
    const SkSize                        fLayerSize;
};

} // namespace internal
} // namespace skottie

#endif // SkottieEffects_DEFINED

/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

namespace  {

class DropShadowAdapter final : public AnimatablePropertyContainer {
public:
    static sk_sp<DropShadowAdapter> MakeEffect(const skjson::ArrayValue& jprops,
                                               sk_sp<sksg::RenderNode> layer,
                                               const AnimationBuilder& abuilder) {
        enum : size_t {
            kShadowColor_Index = 0,
                kOpacity_Index = 1,
              kDirection_Index = 2,
               kDistance_Index = 3,
               kSoftness_Index = 4,
             kShadowOnly_Index = 5,
        };

        sk_sp<DropShadowAdapter> adapter(new DropShadowAdapter(std::move(layer), Type::fEffect));

        EffectBinder(jprops, abuilder, adapter.get())
                .bind(kShadowColor_Index, adapter->fColor    )
                .bind(    kOpacity_Index, adapter->fOpacity  )
                .bind(  kDirection_Index, adapter->fDirection)
                .bind(   kDistance_Index, adapter->fDistance )
                .bind(   kSoftness_Index, adapter->fSoftness )
                .bind( kShadowOnly_Index, adapter->fShdwOnly );

        return adapter;
    }

    static sk_sp<DropShadowAdapter> MakeStyle(const skjson::ObjectValue& jstyle,
                                              sk_sp<sksg::RenderNode> layer,
                                              const AnimationBuilder& abuilder) {
        sk_sp<DropShadowAdapter> adapter(new DropShadowAdapter(std::move(layer), Type::fStyle));

        adapter->bind(abuilder, jstyle["a"], adapter->fDirection);
        adapter->bind(abuilder, jstyle["c"], adapter->fColor    );
        adapter->bind(abuilder, jstyle["d"], adapter->fDistance );
        adapter->bind(abuilder, jstyle["o"], adapter->fOpacity  );
        adapter->bind(abuilder, jstyle["s"], adapter->fSoftness );

        return adapter;
    }

    const sk_sp<sksg::RenderNode>& node() const { return fImageFilterEffect; }

private:
    enum class Type { fEffect, fStyle };
    DropShadowAdapter(sk_sp<sksg::RenderNode> layer, Type ty)
        : fDropShadow(sksg::DropShadowImageFilter::Make())
        , fImageFilterEffect(sksg::ImageFilterEffect::Make(std::move(layer), fDropShadow))
        , fType(ty) {
        fOpacity = this->maxOpacity();
    }

    void onSync() override {
        // fColor -> RGB, fOpacity -> A
        const auto color = ValueTraits<VectorValue>::As<SkColor>(fColor);
        fDropShadow->setColor(SkColorSetA(color,
                                          SkScalarRoundToInt(SkTPin(fOpacity / this->maxOpacity(),
                                                                    0.0f, 1.0f) * 255)));

        // The offset is specified in terms of an angle + distance.
        const auto rad = SkDegreesToRadians(fType == Type::fEffect
                                                ?  90 - fDirection   // bearing      (effect)
                                                : 180 + fDirection); // 0deg -> left (style)
        fDropShadow->setOffset(SkVector::Make( fDistance * SkScalarCos(rad),
                                              -fDistance * SkScalarSin(rad)));

        const auto sigma = fSoftness * kBlurSizeToSigma;
        fDropShadow->setSigma(SkVector::Make(sigma, sigma));

        fDropShadow->setMode(SkToBool(fShdwOnly)
                                ? sksg::DropShadowImageFilter::Mode::kShadowOnly
                                : sksg::DropShadowImageFilter::Mode::kShadowAndForeground);
    }

    float maxOpacity() const {
        return fType == Type::fEffect
                ? 255.0f  // effect: 0 - 255
                : 100.0f; // style : 0 - 100
    }

    const sk_sp<sksg::DropShadowImageFilter> fDropShadow;
    const sk_sp<sksg::RenderNode>            fImageFilterEffect;
    const Type                               fType;

    VectorValue fColor     = { 0, 0, 0, 1 };
    ScalarValue fOpacity,       // initialized explicitly depending on type
                fDirection = 0,
                fDistance  = 0,
                fSoftness  = 0,
                fShdwOnly  = 0;
};

} // anonymous ns

sk_sp<sksg::RenderNode> EffectBuilder::attachDropShadowEffect(const skjson::ArrayValue& jprops,
                                                              sk_sp<sksg::RenderNode> layer) const {
    auto adapter     = DropShadowAdapter::MakeEffect(jprops, std::move(layer), *fBuilder);
    auto effect_node = adapter->node();

    fBuilder->attachDiscardableAdapter(std::move(adapter));

    return effect_node;
}

sk_sp<sksg::RenderNode> EffectBuilder::attachDropShadowStyle(const skjson::ObjectValue& jstyle,
                                                             sk_sp<sksg::RenderNode> layer) const {
    auto adapter     = DropShadowAdapter::MakeStyle(jstyle, std::move(layer), *fBuilder);
    auto effect_node = adapter->node();

    fBuilder->attachDiscardableAdapter(std::move(adapter));

    return effect_node;
}

} // namespace internal
} // namespace skottie

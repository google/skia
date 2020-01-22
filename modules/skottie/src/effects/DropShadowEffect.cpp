/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "modules/skottie/src/Animator.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

namespace  {

class DropShadowAdapter final : public AnimatablePropertyContainer {
public:
    static sk_sp<DropShadowAdapter> Make(const skjson::ArrayValue& jprops,
                                         sk_sp<sksg::RenderNode> layer,
                                         const AnimationBuilder* abuilder) {
        return sk_sp<DropShadowAdapter>(new DropShadowAdapter(jprops, std::move(layer), abuilder));
    }

    const sk_sp<sksg::RenderNode>& node() const { return fImageFilterEffect; }

private:
    DropShadowAdapter(const skjson::ArrayValue& jprops,
                      sk_sp<sksg::RenderNode> layer,
                      const AnimationBuilder* abuilder)
        : fDropShadow(sksg::DropShadowImageFilter::Make())
        , fImageFilterEffect(sksg::ImageFilterEffect::Make(std::move(layer), fDropShadow)) {
        enum : size_t {
            kShadowColor_Index = 0,
            kOpacity_Index     = 1,
            kDirection_Index   = 2,
            kDistance_Index    = 3,
            kSoftness_Index    = 4,
            kShadowOnly_Index  = 5,
        };

        this->bind(*abuilder, EffectBuilder::GetPropValue(jprops, kShadowColor_Index), &fColor    );
        this->bind(*abuilder, EffectBuilder::GetPropValue(jprops,     kOpacity_Index), &fOpacity  );
        this->bind(*abuilder, EffectBuilder::GetPropValue(jprops,   kDirection_Index), &fDirection);
        this->bind(*abuilder, EffectBuilder::GetPropValue(jprops,    kDistance_Index), &fDistance );
        this->bind(*abuilder, EffectBuilder::GetPropValue(jprops,    kSoftness_Index), &fSoftness );
        this->bind(*abuilder, EffectBuilder::GetPropValue(jprops,  kShadowOnly_Index), &fShdwOnly );
    }

    void onSync() override {
        // fColor -> RGB, fOpacity -> A
        const auto color = ValueTraits<VectorValue>::As<SkColor>(fColor);
        fDropShadow->setColor(SkColorSetA(color, SkTPin(SkScalarRoundToInt(fOpacity), 0, 255)));

        // The offset is specified in terms of a bearing angle + distance.
        SkScalar rad = SkDegreesToRadians(90 - fDirection);
        fDropShadow->setOffset(SkVector::Make( fDistance * SkScalarCos(rad),
                                              -fDistance * SkScalarSin(rad)));

        // Close enough to AE.
        static constexpr SkScalar kSoftnessToSigmaFactor = 0.3f;
        const auto sigma = fSoftness * kSoftnessToSigmaFactor;
        fDropShadow->setSigma(SkVector::Make(sigma, sigma));

        fDropShadow->setMode(SkToBool(fShdwOnly)
                                ? sksg::DropShadowImageFilter::Mode::kShadowOnly
                                : sksg::DropShadowImageFilter::Mode::kShadowAndForeground);
    }

    const sk_sp<sksg::DropShadowImageFilter> fDropShadow;
    const sk_sp<sksg::RenderNode>            fImageFilterEffect;

    VectorValue fColor     = { 0, 0, 0, 1 };
    ScalarValue fOpacity   = 255.f,
                fDirection = 0,
                fDistance  = 0,
                fSoftness  = 0,
                fShdwOnly  = 0;
};

} // anonymous ns

sk_sp<sksg::RenderNode> EffectBuilder::attachDropShadowEffect(const skjson::ArrayValue& jprops,
                                                              sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<DropShadowAdapter>(jprops,
                                                                 std::move(layer),
                                                                 fBuilder);
}

} // namespace internal
} // namespace skottie

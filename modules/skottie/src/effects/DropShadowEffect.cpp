/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

namespace  {

class DropShadowAdapter final : public SkRefCnt {
public:
    explicit DropShadowAdapter(sk_sp<sksg::DropShadowImageFilter> dropShadow)
        : fDropShadow(std::move(dropShadow)) {
        SkASSERT(fDropShadow);
    }

    ADAPTER_PROPERTY(Color     , SkColor , SK_ColorBLACK)
    ADAPTER_PROPERTY(Opacity   , SkScalar,           255)
    ADAPTER_PROPERTY(Direction , SkScalar,             0)
    ADAPTER_PROPERTY(Distance  , SkScalar,             0)
    ADAPTER_PROPERTY(Softness  , SkScalar,             0)
    ADAPTER_PROPERTY(ShadowOnly, bool    ,         false)

private:
    void apply() {
        // fColor -> RGB, fOpacity -> A
        fDropShadow->setColor(SkColorSetA(fColor, SkTPin(SkScalarRoundToInt(fOpacity), 0, 255)));

        // The offset is specified in terms of a bearing angle + distance.
        SkScalar rad = SkDegreesToRadians(90 - fDirection);
        fDropShadow->setOffset(SkVector::Make( fDistance * SkScalarCos(rad),
                                              -fDistance * SkScalarSin(rad)));

        // Close enough to AE.
        static constexpr SkScalar kSoftnessToSigmaFactor = 0.3f;
        const auto sigma = fSoftness * kSoftnessToSigmaFactor;
        fDropShadow->setSigma(SkVector::Make(sigma, sigma));

        fDropShadow->setMode(fShadowOnly ? sksg::DropShadowImageFilter::Mode::kShadowOnly
                                         : sksg::DropShadowImageFilter::Mode::kShadowAndForeground);
    }

    const sk_sp<sksg::DropShadowImageFilter> fDropShadow;
};

} // anonymous ns

sk_sp<sksg::RenderNode> EffectBuilder::attachDropShadowEffect(const skjson::ArrayValue& jprops,
                                                              sk_sp<sksg::RenderNode> layer) const {
    enum : size_t {
        kShadowColor_Index = 0,
        kOpacity_Index     = 1,
        kDirection_Index   = 2,
        kDistance_Index    = 3,
        kSoftness_Index    = 4,
        kShadowOnly_Index  = 5,
    };

    auto shadow_effect  = sksg::DropShadowImageFilter::Make();
    auto shadow_adapter = sk_make_sp<DropShadowAdapter>(shadow_effect);

    fBuilder->bindProp<VectorValue>(GetPropValue(jprops, kShadowColor_Index), fScope,
                                    shadow_adapter,
        [](const AnimationBuilder::Capture& cap, const VectorValue& c) {
            cap.as<decltype(shadow_adapter)::element_type>()
                    ->setColor(ValueTraits<VectorValue>::As<SkColor>(c));
        });
    fBuilder->bindProp<ScalarValue>(GetPropValue(jprops, kOpacity_Index), fScope, shadow_adapter,
        [](const AnimationBuilder::Capture& cap, const ScalarValue& o) {
            cap.as<decltype(shadow_adapter)::element_type>()->setOpacity(o);
        });
    fBuilder->bindProp<ScalarValue>(GetPropValue(jprops, kDirection_Index), fScope, shadow_adapter,
        [](const AnimationBuilder::Capture& cap, const ScalarValue& d) {
            cap.as<decltype(shadow_adapter)::element_type>()->setDirection(d);
        });
    fBuilder->bindProp<ScalarValue>(GetPropValue(jprops, kDistance_Index), fScope, shadow_adapter,
        [](const AnimationBuilder::Capture& cap, const ScalarValue& d) {
            cap.as<decltype(shadow_adapter)::element_type>()->setDistance(d);
        });
    fBuilder->bindProp<ScalarValue>(GetPropValue(jprops, kSoftness_Index), fScope, shadow_adapter,
        [](const AnimationBuilder::Capture& cap, const ScalarValue& s) {
            cap.as<decltype(shadow_adapter)::element_type>()->setSoftness(s);
        });
    fBuilder->bindProp<ScalarValue>(GetPropValue(jprops, kShadowOnly_Index), fScope, shadow_adapter,
        [](const AnimationBuilder::Capture& cap, const ScalarValue& s) {
            cap.as<decltype(shadow_adapter)::element_type>()->setShadowOnly(SkToBool(s));
        });

    return sksg::ImageFilterEffect::Make(std::move(layer), std::move(shadow_effect));
}

} // namespace internal
} // namespace skottie

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

class DropShadowAdapter final : public SkNVRefCnt<DropShadowAdapter> {
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
    auto shadow_adapter = sk_make_sp<DropShadowAdapter>(shadow_effect);

    fBuilder->bindProperty<VectorValue>((*color_prop)["v"], fScope,
        [shadow_adapter](const VectorValue& c) {
            shadow_adapter->setColor(ValueTraits<VectorValue>::As<SkColor>(c));
        });
    fBuilder->bindProperty<ScalarValue>((*opacity_prop)["v"], fScope,
        [shadow_adapter](const ScalarValue& o) {
            shadow_adapter->setOpacity(o);
        });
    fBuilder->bindProperty<ScalarValue>((*direction_prop)["v"], fScope,
        [shadow_adapter](const ScalarValue& d) {
            shadow_adapter->setDirection(d);
        });
    fBuilder->bindProperty<ScalarValue>((*distance_prop)["v"], fScope,
        [shadow_adapter](const ScalarValue& d) {
            shadow_adapter->setDistance(d);
        });
    fBuilder->bindProperty<ScalarValue>((*softness_prop)["v"], fScope,
        [shadow_adapter](const ScalarValue& s) {
            shadow_adapter->setSoftness(s);
        });
    fBuilder->bindProperty<ScalarValue>((*shadow_only_prop)["v"], fScope,
        [shadow_adapter](const ScalarValue& s) {
            shadow_adapter->setShadowOnly(SkToBool(s));
        });

    return sksg::ImageFilterEffect::Make(std::move(layer), std::move(shadow_effect));
}

} // namespace internal
} // namespace skottie

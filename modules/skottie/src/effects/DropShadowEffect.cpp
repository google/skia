/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/private/SkTPin.h"
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
                                         const AnimationBuilder& abuilder) {
        enum : size_t {
            kShadowColor_Index = 0,
                kOpacity_Index = 1,
              kDirection_Index = 2,
               kDistance_Index = 3,
               kSoftness_Index = 4,
             kShadowOnly_Index = 5,
        };

        sk_sp<DropShadowAdapter> adapter(new DropShadowAdapter(std::move(layer)));

        EffectBinder(jprops, abuilder, adapter.get())
                .bind(kShadowColor_Index, adapter->fColor    )
                .bind(    kOpacity_Index, adapter->fOpacity  )
                .bind(  kDirection_Index, adapter->fDirection)
                .bind(   kDistance_Index, adapter->fDistance )
                .bind(   kSoftness_Index, adapter->fSoftness )
                .bind( kShadowOnly_Index, adapter->fShdwOnly );

        return adapter;
    }

    const sk_sp<sksg::RenderNode>& node() const { return fImageFilterEffect; }

private:
    explicit DropShadowAdapter(sk_sp<sksg::RenderNode> layer)
        : fDropShadow(sksg::DropShadowImageFilter::Make())
        , fImageFilterEffect(sksg::ImageFilterEffect::Make(std::move(layer), fDropShadow)) {}

    void onSync() override {
        // fColor -> RGB, fOpacity -> A
        const SkColor color = fColor;
        fDropShadow->setColor(SkColorSetA(color, SkTPin(SkScalarRoundToInt(fOpacity), 0, 255)));

        // The offset is specified in terms of a bearing + distance.
        const auto rad = SkDegreesToRadians(90 - fDirection);
        fDropShadow->setOffset(SkVector::Make( fDistance * SkScalarCos(rad),
                                              -fDistance * SkScalarSin(rad)));

        const auto sigma = fSoftness * kBlurSizeToSigma;
        fDropShadow->setSigma(SkVector::Make(sigma, sigma));

        fDropShadow->setMode(SkToBool(fShdwOnly)
                                ? sksg::DropShadowImageFilter::Mode::kShadowOnly
                                : sksg::DropShadowImageFilter::Mode::kShadowAndForeground);
    }

    const sk_sp<sksg::DropShadowImageFilter> fDropShadow;
    const sk_sp<sksg::RenderNode>            fImageFilterEffect;

    VectorValue fColor     = { 0, 0, 0, 1 };
    ScalarValue fOpacity   = 255,
                fDirection = 0,
                fDistance  = 0,
                fSoftness  = 0,
                fShdwOnly  = 0;
};

}  // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachDropShadowEffect(const skjson::ArrayValue& jprops,
                                                              sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<DropShadowAdapter>(jprops,
                                                                 std::move(layer),
                                                                 *fBuilder);
}

} // namespace internal
} // namespace skottie

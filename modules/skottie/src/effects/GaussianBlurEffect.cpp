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

class GaussianBlurEffectAdapter final : public AnimatablePropertyContainer {
public:
    static sk_sp<GaussianBlurEffectAdapter> Make(const skjson::ArrayValue& jprops,
                                                 sk_sp<sksg::RenderNode> layer,
                                                 const AnimationBuilder* abuilder) {
        return sk_sp<GaussianBlurEffectAdapter>(new GaussianBlurEffectAdapter(jprops,
                                                                              std::move(layer),
                                                                              abuilder));
    }

    const sk_sp<sksg::RenderNode>& node() const { return fImageFilterEffect; }

private:
    GaussianBlurEffectAdapter(const skjson::ArrayValue& jprops,
                              sk_sp<sksg::RenderNode> layer,
                              const AnimationBuilder* abuilder)
        : fBlur(sksg::BlurImageFilter::Make())
        , fImageFilterEffect(sksg::ImageFilterEffect::Make(std::move(layer), fBlur)) {
        enum : size_t {
            kBlurriness_Index = 0,
            kDimensions_Index = 1,
            kRepeatEdge_Index = 2,
        };

        EffectBinder(jprops, *abuilder, this)
                .bind(kBlurriness_Index, fBlurriness)
                .bind(kDimensions_Index, fDimensions)
                .bind(kRepeatEdge_Index, fRepeatEdge);
    }

    void onSync() override {
        static constexpr SkVector kDimensionsMap[] = {
            { 1, 1 }, // 1 -> horizontal and vertical
            { 1, 0 }, // 2 -> horizontal
            { 0, 1 }, // 3 -> vertical
        };

        const auto dim_index = SkTPin<size_t>(static_cast<size_t>(fDimensions),
                                              1, SK_ARRAY_COUNT(kDimensionsMap)) - 1;

        const auto sigma = fBlurriness * kBlurSizeToSigma;

        fBlur->setSigma({ sigma * kDimensionsMap[dim_index].x(),
                          sigma * kDimensionsMap[dim_index].y() });

        static constexpr SkTileMode kRepeatEdgeMap[] = {
            SkTileMode::kDecal, // 0 -> repeat edge pixels: off
            SkTileMode::kClamp, // 1 -> repeat edge pixels: on
        };

        const auto repeat_index = SkTPin<size_t>(static_cast<size_t>(fRepeatEdge),
                                                 0, SK_ARRAY_COUNT(kRepeatEdgeMap) - 1);
        fBlur->setTileMode(kRepeatEdgeMap[repeat_index]);
    }

    const sk_sp<sksg::BlurImageFilter> fBlur;
    const sk_sp<sksg::RenderNode>      fImageFilterEffect;

    ScalarValue fBlurriness = 0, // Controls the blur sigma.
                fDimensions = 1, // 1 -> horizontal & vertical, 2 -> horizontal, 3 -> vertical
                fRepeatEdge = 0; // 0 -> clamp, 1 -> repeat
};

} // anonymous ns

sk_sp<sksg::RenderNode> EffectBuilder::attachGaussianBlurEffect(
        const skjson::ArrayValue& jprops,
        sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<GaussianBlurEffectAdapter>(jprops,
                                                                         std::move(layer),
                                                                         fBuilder);
}

} // namespace internal
} // namespace skottie

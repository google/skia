/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include "include/private/base/SkTPin.h"
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
                                              1, std::size(kDimensionsMap)) - 1;

        const auto sigma = fBlurriness * kBlurSizeToSigma;

        fBlur->setSigma({ sigma * kDimensionsMap[dim_index].x(),
                          sigma * kDimensionsMap[dim_index].y() });

        // 0 -> repeat edge pixels: off
        // 1 -> repeat edge pixels: on
        const auto repeat_edge = static_cast<bool>(fRepeatEdge);

        // Repeat edge pixels implies two things:
        //  - the blur uses kClamp tiling
        //  - the output is cropped to content size
        fBlur->setTileMode(repeat_edge
            ? SkTileMode::kClamp
            : SkTileMode::kDecal);
        static_cast<sksg::ImageFilterEffect*>(fImageFilterEffect.get())->setCropping(repeat_edge
            ? sksg::ImageFilterEffect::Cropping::kContent
            : sksg::ImageFilterEffect::Cropping::kNone);
    }

    const sk_sp<sksg::BlurImageFilter> fBlur;
    const sk_sp<sksg::RenderNode>      fImageFilterEffect;

    ScalarValue fBlurriness = 0, // Controls the blur sigma.
                fDimensions = 1, // 1 -> horizontal & vertical, 2 -> horizontal, 3 -> vertical
                fRepeatEdge = 0; // 0 -> clamp, 1 -> repeat
};

}  // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachGaussianBlurEffect(
        const skjson::ArrayValue& jprops,
        sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<GaussianBlurEffectAdapter>(jprops,
                                                                         std::move(layer),
                                                                         fBuilder);
}

} // namespace internal
} // namespace skottie

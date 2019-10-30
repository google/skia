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

class GaussianBlurEffectAdapter final : public SkNVRefCnt<GaussianBlurEffectAdapter> {
public:
    explicit GaussianBlurEffectAdapter(sk_sp<sksg::BlurImageFilter> blur)
        : fBlur(std::move(blur)) {
        SkASSERT(fBlur);
    }

    // AE/BM model properties.  These are all animatable/interpolatable.

    // Controls the blur sigma.
    ADAPTER_PROPERTY(Blurriness, SkScalar, 0)

    // Enum selecting the blur dimensionality:
    //
    //   1 -> horizontal & vertical
    //   2 -> horizontal
    //   3 -> vertical
    //
    ADAPTER_PROPERTY(Dimensions, SkScalar, 1)

    // Enum selecting edge behavior:
    //
    //   0 -> clamp
    //   1 -> repeat
    //
    ADAPTER_PROPERTY(RepeatEdge, SkScalar, 0)

private:
    void apply() {
        static constexpr SkVector kDimensionsMap[] = {
            { 1, 1 }, // 1 -> horizontal and vertical
            { 1, 0 }, // 2 -> horizontal
            { 0, 1 }, // 3 -> vertical
        };

        const auto dim_index = SkTPin<size_t>(static_cast<size_t>(fDimensions),
                                              1, SK_ARRAY_COUNT(kDimensionsMap)) - 1;

        // Close enough to AE.
        static constexpr SkScalar kBlurrinessToSigmaFactor = 0.3f;
        const auto sigma = fBlurriness * kBlurrinessToSigmaFactor;

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
};

} // anonymous ns

sk_sp<sksg::RenderNode> EffectBuilder::attachGaussianBlurEffect(
        const skjson::ArrayValue& jprops,
        sk_sp<sksg::RenderNode> layer) const {
    enum : size_t {
        kBlurriness_Index = 0,
        kDimensions_Index = 1,
        kRepeatEdge_Index = 2,
    };

    auto blur_effect   = sksg::BlurImageFilter::Make();
    auto blur_addapter = sk_make_sp<GaussianBlurEffectAdapter>(blur_effect);

    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kBlurriness_Index),
        [blur_addapter](const ScalarValue& b) {
            blur_addapter->setBlurriness(b);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kDimensions_Index),
        [blur_addapter](const ScalarValue& d) {
            blur_addapter->setDimensions(d);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kRepeatEdge_Index),
        [blur_addapter](const ScalarValue& r) {
            blur_addapter->setRepeatEdge(r);
        });

    return sksg::ImageFilterEffect::Make(std::move(layer), std::move(blur_effect));
}

} // namespace internal
} // namespace skottie

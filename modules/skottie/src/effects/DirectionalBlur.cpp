/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "src/utils/SkJSON.h"

namespace skottie::internal {

namespace {

class DirectionalBlurAdapter final : public DiscardableAdapterBase<DirectionalBlurAdapter,
                                                                   sksg::ExternalImageFilter> {
    public:
        DirectionalBlurAdapter(const skjson::ArrayValue& jprops,
                               const AnimationBuilder& abuilder)
            : INHERITED(sksg::ExternalImageFilter::Make())
        {
            enum : size_t {
                kDirection_Index  = 0,
                kBlurLength_Index = 1,
            };


            EffectBinder(jprops, abuilder, this)
                .bind(       kDirection_Index, fDirection)
                .bind(    kBlurLength_Index, fBlurLength);
        }
    private:
        void onSync() override {
            const auto rot = fDirection - 90;
            auto filter =
            SkImageFilters::MatrixTransform(SkMatrix::RotateDeg(rot),
            SkSamplingOptions(SkFilterMode::kLinear),
                SkImageFilters::Blur(fBlurLength * kBlurSizeToSigma, 0,
                    SkImageFilters::MatrixTransform(SkMatrix::RotateDeg(-rot),
                    SkSamplingOptions(SkFilterMode::kLinear), nullptr)));
            this->node()->setImageFilter(std::move(filter));
        }

        ScalarValue fDirection = 0;
        ScalarValue fBlurLength = 0;

        using INHERITED = DiscardableAdapterBase<DirectionalBlurAdapter, sksg::ExternalImageFilter>;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachDirectionalBlurEffect(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
    auto imageFilterNode = fBuilder->attachDiscardableAdapter<DirectionalBlurAdapter>(jprops,
                                                                      *fBuilder);
    return sksg::ImageFilterEffect::Make(std::move(layer), std::move(imageFilterNode));
}

} // namespace skottie::internal

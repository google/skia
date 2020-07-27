/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/effects/SkColorMatrix.h" 
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderEffect.h"

namespace skottie::internal {

namespace  {

class FractalNoiseAdapter final : public DiscardableAdapterBase<FractalNoiseAdapter,
                                                                sksg::ExternalImageFilter> {
public:
    FractalNoiseAdapter(const skjson::ArrayValue& jprops, const AnimationBuilder& abuilder) {
        enum : size_t {
            //      kFractalType_Index = 0,
            //        kNoiseType_Index = 1,
                       kInvert_Index = 2,
                     kContrast_Index = 3,
                   kBrightness_Index = 4,
            //         kOverflow_Index = 5,
                    kTransform_Index = 6,
                     kRotation_Index = 7,
               kUniformScaling_Index = 8,
                        kScale_Index = 9,
                   kScaleWidth_Index = 10,
                  kScaleHeight_Index = 11,
            // kOffsetTurbulence_Index = 12,
            //kPerspectiveOffset_Index = 13,
                   kComplexity_Index = 15,
            //      kSubSettings_Index = 16,
            //     kSubInfluence_Index = 17,
            //       kSubScaling_Index = 18,
            //      kSubRotation_Index = 19,
            //        kSubOffest_Index = 20,
            //   kCenterSubscale_Index = 21,
                    kEvolution_Index = 23,
            // kEvolutionOptions_Index = 24,
            //   kCycleEvolution_Index = 25,
            //            kCycle_Index = 26,
            //      kRandomSeed1_Index = 27,
            //      kRandomSeed2_Index = 28,
                      kOpacity_Index = 29,
                 kBlendingMode_Index = 30,
        };

        EffectBinder(jprops, abuilder, this)
                .bind(kInvert_Index, fInvert)
                .bind(kContrast_Index, fContrast)
                .bind(kBrightness_Index, fBrightness)
                .bind(kComplexity_Index, fComplexity)
                .bind(kOpacity_Index, fOpacity)
                .bind(kBlendingMode_Index, fBlendMode);
    }

private:
    void onSync() override {
        const auto octaves = SkScalarRoundToScalar(std::max(fComplexity, 1.0f));

        SkPaint shader_paint;
        shader_paint.setShader(SkPerlinNoiseShader::MakeImprovedNoise(0.03, 0.03, 3, fEvolution));

        auto f = SkImageFilters::Paint(shader_paint);
        
        SkColorMatrix cm{
            1,0,0,0,0,
            1,0,0,0,0,
            1,0,0,0,0,
            0,0,0,1,0,
        };

        f = SkImageFilters::ColorFilter(SkColorFilters::Matrix(cm), std::move(f));

        this->node()->setImageFilter(std::move(f));
    }

    ScalarValue fInvert     =   0,
                fContrast   = 100,
                fBrightness =   0,
                fComplexity =   1,
                fEvolution  =   0,
                fOpacity    = 100,
                fBlendMode  =   0;

    using INHERITED = DiscardableAdapterBase<FractalNoiseAdapter, sksg::ExternalImageFilter>;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachFractalNoiseEffect(const skjson::ArrayValue& jprops,
        sk_sp<sksg::RenderNode> layer) const {
    auto filter_node = fBuilder->attachDiscardableAdapter<FractalNoiseAdapter>(jprops, *fBuilder);

    return sksg::ImageFilterEffect::Make(std::move(layer), std::move(filter_node));
}

} // namespace skottie::internal


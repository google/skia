/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGGradient.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

namespace  {

class GradientRampEffectAdapter final : public AnimatablePropertyContainer {
public:
    static sk_sp<GradientRampEffectAdapter> Make(const skjson::ArrayValue& jprops,
                                                 sk_sp<sksg::RenderNode> layer,
                                                 const AnimationBuilder* abuilder) {
        return sk_sp<GradientRampEffectAdapter>(new GradientRampEffectAdapter(jprops,
                                                                              std::move(layer),
                                                                              abuilder));
    }

    sk_sp<sksg::RenderNode> node() const { return fShaderEffect; }

private:
    GradientRampEffectAdapter(const skjson::ArrayValue& jprops,
                              sk_sp<sksg::RenderNode> layer,
                              const AnimationBuilder* abuilder)
        : fShaderEffect(sksg::ShaderEffect::Make(std::move(layer))) {
        enum : size_t {
             kStartPoint_Index = 0,
             kStartColor_Index = 1,
               kEndPoint_Index = 2,
               kEndColor_Index = 3,
              kRampShape_Index = 4,
            kRampScatter_Index = 5,
             kBlendRatio_Index = 6,
        };

        EffectBinder(jprops, *abuilder, this)
                .bind( kStartPoint_Index, fStartPoint)
                .bind( kStartColor_Index, fStartColor)
                .bind(   kEndPoint_Index, fEndPoint  )
                .bind(   kEndColor_Index, fEndColor  )
                .bind(  kRampShape_Index, fShape     )
                .bind(kRampScatter_Index, fScatter   )
                .bind( kBlendRatio_Index, fBlend     );
    }

    enum class InstanceType {
        kNone,
        kLinear,
        kRadial,
    };

    void onSync() override {
        // This adapter manages a SG fragment with the following structure:
        //
        // - ShaderEffect [fRoot]
        //     \  GradientShader [fGradient]
        //     \  child/wrapped fragment
        //
        // The gradient shader is updated based on the (animatable) instance type (linear/radial).

        auto update_gradient = [this] (InstanceType new_type) {
            if (new_type != fInstanceType) {
                fGradient = new_type == InstanceType::kLinear
                        ? sk_sp<sksg::Gradient>(sksg::LinearGradient::Make())
                        : sk_sp<sksg::Gradient>(sksg::RadialGradient::Make());

                fShaderEffect->setShader(fGradient);
                fInstanceType = new_type;
            }

            fGradient->setColorStops({{0, ValueTraits<VectorValue>::As<SkColor4f>(fStartColor)},
                                      {1, ValueTraits<VectorValue>::As<SkColor4f>(  fEndColor)}});
        };

        static constexpr int kLinearShapeValue = 1;
        const auto instance_type = (SkScalarRoundToInt(fShape) == kLinearShapeValue)
                ? InstanceType::kLinear
                : InstanceType::kRadial;

        // Sync the gradient shader instance if needed.
        update_gradient(instance_type);

        // Sync instance-dependent gradient params.
        const auto start_point = SkPoint{fStartPoint.x, fStartPoint.y},
                     end_point = SkPoint{  fEndPoint.x,   fEndPoint.y};
        if (instance_type == InstanceType::kLinear) {
            auto* lg = static_cast<sksg::LinearGradient*>(fGradient.get());
            lg->setStartPoint(start_point);
            lg->setEndPoint(end_point);
        } else {
            SkASSERT(instance_type == InstanceType::kRadial);

            auto* rg = static_cast<sksg::RadialGradient*>(fGradient.get());
            rg->setStartCenter(start_point);
            rg->setEndCenter(start_point);
            rg->setEndRadius(SkPoint::Distance(start_point, end_point));
        }

        // TODO: blend, scatter
    }

    const sk_sp<sksg::ShaderEffect> fShaderEffect;
    sk_sp<sksg::Gradient>           fGradient;

    InstanceType              fInstanceType = InstanceType::kNone;

    VectorValue fStartColor,
                  fEndColor;
    Vec2Value   fStartPoint = {0,0},
                fEndPoint   = {0,0};
    ScalarValue fBlend   = 0,
                fScatter = 0,
                fShape   = 0; // 1 -> linear, 7 -> radial (?!)
};

} // anonymous ns

sk_sp<sksg::RenderNode> EffectBuilder::attachGradientEffect(const skjson::ArrayValue& jprops,
                                                            sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<GradientRampEffectAdapter>(jprops,
                                                                         std::move(layer),
                                                                         fBuilder);
}

} // namespace internal
} // namespace skottie

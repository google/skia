/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGGradient.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

namespace  {

class GradientRampEffectAdapter final : public SkNVRefCnt<GradientRampEffectAdapter> {
public:
    explicit GradientRampEffectAdapter(sk_sp<sksg::RenderNode> child)
        : fRoot(sksg::ShaderEffect::Make(std::move(child))) {}

    ADAPTER_PROPERTY(StartPoint, SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(EndPoint  , SkPoint , SkPoint::Make(0, 0))
    ADAPTER_PROPERTY(StartColor, SkColor ,       SK_ColorBLACK)
    ADAPTER_PROPERTY(EndColor  , SkColor ,       SK_ColorBLACK)
    ADAPTER_PROPERTY(Blend     , SkScalar,                   0)
    ADAPTER_PROPERTY(Scatter   , SkScalar,                   0)

    // Really an enum: 1 -> linear, 7 -> radial (?!)
    ADAPTER_PROPERTY(Shape     , SkScalar,                   0)

    const sk_sp<sksg::ShaderEffect>& root() const { return fRoot; }

private:
    enum class InstanceType {
        kNone,
        kLinear,
        kRadial,
    };

    void apply() {
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

                fRoot->setShader(fGradient);
                fInstanceType = new_type;
            }

            fGradient->setColorStops({ {0, fStartColor}, {1, fEndColor} });
        };

        static constexpr int kLinearShapeValue = 1;
        const auto instance_type = (SkScalarRoundToInt(fShape) == kLinearShapeValue)
                ? InstanceType::kLinear
                : InstanceType::kRadial;

        // Sync the gradient shader instance if needed.
        update_gradient(instance_type);

        // Sync instance-dependent gradient params.
        if (instance_type == InstanceType::kLinear) {
            auto* lg = static_cast<sksg::LinearGradient*>(fGradient.get());
            lg->setStartPoint(fStartPoint);
            lg->setEndPoint(fEndPoint);
        } else {
            SkASSERT(instance_type == InstanceType::kRadial);

            auto* rg = static_cast<sksg::RadialGradient*>(fGradient.get());
            rg->setStartCenter(fStartPoint);
            rg->setEndCenter(fStartPoint);
            rg->setEndRadius(SkPoint::Distance(fStartPoint, fEndPoint));
        }

        // TODO: blend, scatter
    }

    sk_sp<sksg::ShaderEffect> fRoot;
    sk_sp<sksg::Gradient>     fGradient;
    InstanceType              fInstanceType = InstanceType::kNone;
};

} // anonymous ns

sk_sp<sksg::RenderNode> EffectBuilder::attachGradientEffect(const skjson::ArrayValue& jprops,
                                                            sk_sp<sksg::RenderNode> layer) const {
    enum : size_t {
        kStartPoint_Index  = 0,
        kStartColor_Index  = 1,
        kEndPoint_Index    = 2,
        kEndColor_Index    = 3,
        kRampShape_Index   = 4,
        kRampScatter_Index = 5,
        kBlendRatio_Index  = 6,
    };

    auto adapter = sk_make_sp<GradientRampEffectAdapter>(std::move(layer));

    fBuilder->bindProperty<VectorValue>(GetPropValue(jprops, kStartPoint_Index),
        [adapter](const VectorValue& p0) {
            adapter->setStartPoint(ValueTraits<VectorValue>::As<SkPoint>(p0));
        });
    fBuilder->bindProperty<VectorValue>(GetPropValue(jprops, kEndPoint_Index),
        [adapter](const VectorValue& p1) {
            adapter->setEndPoint(ValueTraits<VectorValue>::As<SkPoint>(p1));
        });
    fBuilder->bindProperty<VectorValue>(GetPropValue(jprops, kStartColor_Index),
        [adapter](const VectorValue& c0) {
            adapter->setStartColor(ValueTraits<VectorValue>::As<SkColor>(c0));
        });
    fBuilder->bindProperty<VectorValue>(GetPropValue(jprops, kEndColor_Index),
        [adapter](const VectorValue& c1) {
            adapter->setEndColor(ValueTraits<VectorValue>::As<SkColor>(c1));
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kRampShape_Index),
        [adapter](const ScalarValue& shape) {
            adapter->setShape(shape);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kBlendRatio_Index),
        [adapter](const ScalarValue& blend) {
            adapter->setBlend(blend);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kRampScatter_Index),
        [adapter](const ScalarValue& scatter) {
            adapter->setScatter(scatter);
        });

    return adapter->root();
}

} // namespace internal
} // namespace skottie

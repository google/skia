/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/sksg/include/SkSGOpacityEffect.h"
#include "modules/sksg/include/SkSGTransform.h"

namespace skottie {
namespace internal {

namespace  {

// Transform effects can operate in either uniform or anisotropic mode, with each
// component (including mode) animated separately.
class ScaleAdapter final : public SkNVRefCnt<ScaleAdapter> {
public:
    explicit ScaleAdapter(sk_sp<TransformAdapter2D> tadapter)
        : fTransformAdapter(std::move(tadapter)) {}

    ADAPTER_PROPERTY(IsUniform, bool    , false)
    ADAPTER_PROPERTY(ScaleX   , SkScalar,   100)
    ADAPTER_PROPERTY(ScaleY   , SkScalar,   100)

private:
    void apply() {
        const auto scale = SkVector::Make(fScaleX, fIsUniform ? fScaleX : fScaleY);
        fTransformAdapter->setScale(scale);
    }

    const sk_sp<TransformAdapter2D> fTransformAdapter;
};

} // anonymous ns

sk_sp<sksg::RenderNode> AttachTransformEffect(const skjson::ArrayValue& jprops,
                                              const AnimationBuilder* abuilder,
                                              AnimatorScope* ascope,
                                              sk_sp<sksg::RenderNode> layer) {
    enum : size_t {
        kAnchorPoint_Index            =  0,
        kPosition_Index               =  1,
        kUniformScale_Index           =  2,
        kScaleX_Index                 =  3,
        kScaleY_Index                 =  4,
        kSkew_Index                   =  5,
        kSkewAxis_Index               =  6,
        kRotation_Index               =  7,
        kOpacity_Index                =  8,
        // kUseCompShutterAngle_Index =  9,
        // kShutterAngle_Index        = 10,
        // kSampling_Index            = 11,

        kMaxIndex                     = kOpacity_Index,
    };

    auto matrix = sksg::Matrix<SkMatrix>::Make(SkMatrix::I());
    auto t_adapter = sk_make_sp<TransformAdapter2D>(matrix);
    auto s_adapter = sk_make_sp<ScaleAdapter>(t_adapter);

    abuilder->bindProperty<VectorValue>(EffectBuilder::GetPropValue(jprops, kAnchorPoint_Index),
                                        ascope,
        [t_adapter](const VectorValue& ap) {
            t_adapter->setAnchorPoint(ValueTraits<VectorValue>::As<SkPoint>(ap));
        });
    abuilder->bindProperty<VectorValue>(EffectBuilder::GetPropValue(jprops, kPosition_Index),
                                        ascope,
        [t_adapter](const VectorValue& p) {
            t_adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops, kRotation_Index),
                                        ascope,
        [t_adapter](const ScalarValue& r) {
            t_adapter->setRotation(r);
        });
    abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops, kSkew_Index),
                                        ascope,
        [t_adapter](const ScalarValue& s) {
            t_adapter->setSkew(s);
        });
    abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops, kSkewAxis_Index),
                                        ascope,
        [t_adapter](const ScalarValue& sa) {
            t_adapter->setSkewAxis(sa);
        });

    abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops, kUniformScale_Index),
                                        ascope,
        [s_adapter](const ScalarValue& u) {
            s_adapter->setIsUniform(SkScalarRoundToInt(u));
        });
    abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops, kScaleX_Index),
                                        ascope,
        [s_adapter](const ScalarValue& sx) {
            s_adapter->setScaleX(sx);
        });
    abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops, kScaleY_Index),
                                        ascope,
        [s_adapter](const ScalarValue& sy) {
            s_adapter->setScaleY(sy);
        });

    auto opacity_node = sksg::OpacityEffect::Make(sksg::TransformEffect::Make(std::move(layer),
                                                                              std::move(matrix)));

    abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops, kOpacity_Index),
                                        ascope,
        [opacity_node](const ScalarValue& o) {
            opacity_node->setOpacity(o * 0.01f);
        });

    return std::move(opacity_node);
}

} // namespace internal
} // namespace skottie

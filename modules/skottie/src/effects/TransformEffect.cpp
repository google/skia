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
class ScaleAdapter final : public SkRefCnt {
public:
    explicit ScaleAdapter(sk_sp<TransformAdapter2D> tadapter)
        : fTransformAdapter(std::move(tadapter)) {}

    ADAPTER_PROPERTY(IsUniform  , bool    , false)
    ADAPTER_PROPERTY(ScaleWidth , SkScalar,   100)
    ADAPTER_PROPERTY(ScaleHeight, SkScalar,   100)

private:
    void apply() {
        // In uniform mode, the scale is based solely in ScaleHeight.
        const auto scale = SkVector::Make(fIsUniform ? fScaleHeight : fScaleWidth,
                                          fScaleHeight);
        fTransformAdapter->setScale(scale);
    }

    const sk_sp<TransformAdapter2D> fTransformAdapter;
};

} // anonymous ns

sk_sp<sksg::RenderNode> EffectBuilder::attachTransformEffect(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
    enum : size_t {
        kAnchorPoint_Index            =  0,
        kPosition_Index               =  1,
        kUniformScale_Index           =  2,
        kScaleHeight_Index            =  3,
        kScaleWidth_Index             =  4,
        kSkew_Index                   =  5,
        kSkewAxis_Index               =  6,
        kRotation_Index               =  7,
        kOpacity_Index                =  8,
        // kUseCompShutterAngle_Index =  9,
        // kShutterAngle_Index        = 10,
        // kSampling_Index            = 11,
    };

    auto matrix = sksg::Matrix<SkMatrix>::Make(SkMatrix::I());
    auto t_adapter = sk_make_sp<TransformAdapter2D>(matrix);
    auto s_adapter = sk_make_sp<ScaleAdapter>(t_adapter);

    fBuilder->bindProp<VectorValue>(GetPropValue(jprops, kAnchorPoint_Index), fScope, t_adapter,
        [](const AnimationBuilder::Capture& cap, const VectorValue& ap) {
            cap.as<decltype(t_adapter)::element_type>()
                    ->setAnchorPoint(ValueTraits<VectorValue>::As<SkPoint>(ap));
        });
    fBuilder->bindProp<VectorValue>(GetPropValue(jprops, kPosition_Index), fScope, t_adapter,
        [](const AnimationBuilder::Capture& cap, const VectorValue& p) {
            cap.as<decltype(t_adapter)::element_type>()
                    ->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    fBuilder->bindProp<ScalarValue>(GetPropValue(jprops, kRotation_Index), fScope, t_adapter,
        [](const AnimationBuilder::Capture& cap, const ScalarValue& r) {
            cap.as<decltype(t_adapter)::element_type>()->setRotation(r);
        });
    fBuilder->bindProp<ScalarValue>(GetPropValue(jprops, kSkew_Index), fScope, t_adapter,
        [](const AnimationBuilder::Capture& cap, const ScalarValue& s) {
            cap.as<decltype(t_adapter)::element_type>()->setSkew(s);
        });
    fBuilder->bindProp<ScalarValue>(GetPropValue(jprops, kSkewAxis_Index), fScope, t_adapter,
        [](const AnimationBuilder::Capture& cap, const ScalarValue& sa) {
            cap.as<decltype(t_adapter)::element_type>()->setSkewAxis(sa);
        });

    fBuilder->bindProp<ScalarValue>(GetPropValue(jprops, kUniformScale_Index), fScope, s_adapter,
        [](const AnimationBuilder::Capture& cap, const ScalarValue& u) {
             cap.as<decltype(s_adapter)::element_type>()->setIsUniform(SkScalarRoundToInt(u));
        });
    fBuilder->bindProp<ScalarValue>(GetPropValue(jprops, kScaleHeight_Index), fScope, s_adapter,
        [](const AnimationBuilder::Capture& cap, const ScalarValue& sh) {
            cap.as<decltype(s_adapter)::element_type>()->setScaleHeight(sh);
        });
    fBuilder->bindProp<ScalarValue>(GetPropValue(jprops, kScaleWidth_Index), fScope, s_adapter,
        [](const AnimationBuilder::Capture& cap, const ScalarValue& sw) {
            cap.as<decltype(s_adapter)::element_type>()->setScaleWidth(sw);
        });

    auto opacity_node = sksg::OpacityEffect::Make(sksg::TransformEffect::Make(std::move(layer),
                                                                              std::move(matrix)));

    fBuilder->bindProp<ScalarValue>(GetPropValue(jprops, kOpacity_Index), fScope, opacity_node,
        [](const AnimationBuilder::Capture& cap, const ScalarValue& o) {
            cap.as<decltype(opacity_node)::element_type>()->setOpacity(o * 0.01f);
        });

    return std::move(opacity_node);
}

} // namespace internal
} // namespace skottie

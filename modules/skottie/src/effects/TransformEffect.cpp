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

    fBuilder->bindProperty<VectorValue>(GetPropValue(jprops, kAnchorPoint_Index),
        [t_adapter](const VectorValue& ap) {
            t_adapter->setAnchorPoint(ValueTraits<VectorValue>::As<SkPoint>(ap));
        });
    fBuilder->bindProperty<VectorValue>(GetPropValue(jprops, kPosition_Index),
        [t_adapter](const VectorValue& p) {
            t_adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kRotation_Index),
        [t_adapter](const ScalarValue& r) {
            t_adapter->setRotation(r);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kSkew_Index),
        [t_adapter](const ScalarValue& s) {
            t_adapter->setSkew(s);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kSkewAxis_Index),
        [t_adapter](const ScalarValue& sa) {
            t_adapter->setSkewAxis(sa);
        });

    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kUniformScale_Index),
        [s_adapter](const ScalarValue& u) {
            s_adapter->setIsUniform(SkScalarRoundToInt(u));
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kScaleHeight_Index),
        [s_adapter](const ScalarValue& sh) {
            s_adapter->setScaleHeight(sh);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kScaleWidth_Index),
        [s_adapter](const ScalarValue& sw) {
            s_adapter->setScaleWidth(sw);
        });

    auto opacity_node = sksg::OpacityEffect::Make(sksg::TransformEffect::Make(std::move(layer),
                                                                              std::move(matrix)));

    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kOpacity_Index),
        [opacity_node](const ScalarValue& o) {
            opacity_node->setOpacity(o * 0.01f);
        });

    return opacity_node;
}

} // namespace internal
} // namespace skottie

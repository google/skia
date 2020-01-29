/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/Transform.h"
#include "modules/sksg/include/SkSGOpacityEffect.h"
#include "modules/sksg/include/SkSGTransform.h"

namespace skottie {
namespace internal {

namespace  {

class TransformEffectAdapter final : public DiscardableAdapterBase<TransformEffectAdapter,
                                                                   sksg::OpacityEffect> {
public:
    TransformEffectAdapter(const AnimationBuilder& abuilder,
                           const skjson::ObjectValue* jopacity,
                           const skjson::ObjectValue* jscale_uniform,
                           const skjson::ObjectValue* jscale_width,
                           const skjson::ObjectValue* jscale_height,
                           sk_sp<TransformAdapter2D> tadapter,
                           sk_sp<sksg::RenderNode> child)
        : INHERITED(sksg::OpacityEffect::Make(std::move(child)))
        , fTransformAdapter(std::move(tadapter)) {
        this->bind(abuilder, jopacity      , fOpacity     );
        this->bind(abuilder, jscale_uniform, fUniformScale);
        this->bind(abuilder, jscale_width  , fScaleWidth  );
        this->bind(abuilder, jscale_height , fScaleHeight );

        this->attachDiscardableAdapter(fTransformAdapter);
    }

private:
    void onSync() override {
        this->node()->setOpacity(fOpacity * 0.01f);

        // In uniform mode, the scale is based solely in ScaleHeight.
        const auto scale = SkVector::Make(SkScalarRoundToInt(fUniformScale) ? fScaleHeight
                                                                            : fScaleWidth,
                                          fScaleHeight);

        // NB: this triggers an transform adapter -> SG sync.
        fTransformAdapter->setScale(scale);
    }

    const sk_sp<TransformAdapter2D> fTransformAdapter;

    ScalarValue fOpacity      = 100,
                fUniformScale =   0, // bool
                fScaleWidth   = 100,
                fScaleHeight  = 100;

    using INHERITED = DiscardableAdapterBase<TransformEffectAdapter, sksg::OpacityEffect>;
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

    auto transform_adapter = TransformAdapter2D::Make(*fBuilder,
                                                      GetPropValue(jprops, kAnchorPoint_Index),
                                                      GetPropValue(jprops, kPosition_Index),
                                                      nullptr, // scale is handled externally
                                                      GetPropValue(jprops, kRotation_Index),
                                                      GetPropValue(jprops, kSkew_Index),
                                                      GetPropValue(jprops, kSkewAxis_Index));
    if (!transform_adapter) {
        return nullptr;
    }

    auto transform_effect_node = sksg::TransformEffect::Make(std::move(layer),
                                                             transform_adapter->node());
    return fBuilder->attachDiscardableAdapter<TransformEffectAdapter>
            (*fBuilder,
             GetPropValue(jprops, kOpacity_Index),
             GetPropValue(jprops, kUniformScale_Index),
             GetPropValue(jprops, kScaleWidth_Index),
             GetPropValue(jprops, kScaleHeight_Index),
             std::move(transform_adapter),
             std::move(transform_effect_node)
             );
}

} // namespace internal
} // namespace skottie

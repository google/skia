/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/effects/SkGradientShader.h"
#include "include/effects/SkShaderMaskFilter.h"
#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "src/utils/SkJSON.h"

#include <cmath>
#include <utility>

namespace skottie {
namespace internal {

namespace  {

class LinearWipeAdapter final : public SkNVRefCnt<LinearWipeAdapter> {
public:
    LinearWipeAdapter(sk_sp<sksg::RenderNode> layer, const SkSize& ls)
        : fMaskNode(sksg::MaskFilter::Make(nullptr))
        , fMaskEffectNode(sksg::MaskFilterEffect::Make(std::move(layer), fMaskNode))
        , fLayerSize(ls) {}

    ADAPTER_PROPERTY(Completion, float, 0)
    ADAPTER_PROPERTY(Angle     , float, 0)
    ADAPTER_PROPERTY(Feather   , float, 0)

    const sk_sp<sksg::MaskFilterEffect>& root() const { return fMaskEffectNode; }

private:
    void apply() const {
        if (fCompletion >= 100) {
            // The layer is fully disabled.
            fMaskEffectNode->setVisible(false);
            return;
        }
        fMaskEffectNode->setVisible(true);

        if (fCompletion <= 0) {
            // The layer is fully visible (no mask).
            fMaskNode->setMaskFilter(nullptr);
            return;
        }

        const auto t = SkTPin(fCompletion * 0.01f, 0.0f, 1.0f),
             feather = std::max(fFeather, 0.0f),
               angle = SkDegreesToRadians(90 - fAngle),
                cos_ = std::cos(angle),
                sin_ = std::sin(angle);

        // Select the correct diagonal vector depending on quadrant.
        const SkVector angle_v = {cos_, sin_},
                        diag_v = {std::copysign(fLayerSize.width() , cos_),
                                  std::copysign(fLayerSize.height(), sin_)};

        // The transition length is the projection of the diagonal onto the angle vector.
        const auto len = SkVector::DotProduct(diag_v, angle_v);

        // Pad the gradient segment to accommodate optional feather ramps at both extremities.
        const auto grad_len   = len + feather * 2;
        const SkVector grad_v = angle_v * grad_len,
              adjusted_grad_v = { grad_v.fX, -grad_v.fY }, // Y flipped for drawing space.
                     center_v = {fLayerSize.width() * 0.5f, fLayerSize.height() * 0.5f};

        // Gradient start/end points:
        const SkPoint pts[] = {
            center_v - adjusted_grad_v * 0.5f,
            center_v + adjusted_grad_v * 0.5f,
        };

        static constexpr SkColor colors[] = { 0x00000000,
                                              0xffffffff };

        // To emulate the feather effect, we distance the color stops to generate
        // a linear transition/ramp.  For t == 0 the ramp should be completely outside/before
        // the transition domain, and for t == 1 it should be completely outside/after.
        //
        //                      [0 ................... |len|]
        //
        //   [0  <feather_ramp> [                           ] <feather_ramp> |grad_len|]
        const auto adjusted_t = t * (len + feather) / grad_len;
        const SkScalar  pos[] = { adjusted_t,
                                  adjusted_t + feather / grad_len };

        fMaskNode->setMaskFilter(SkShaderMaskFilter::Make(
            SkGradientShader::MakeLinear(pts, colors, pos, 2, SkTileMode::kClamp)));
    }

    const sk_sp<sksg::MaskFilter>       fMaskNode;
    const sk_sp<sksg::MaskFilterEffect> fMaskEffectNode;
    const SkSize                        fLayerSize;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachLinearWipeEffect(const skjson::ArrayValue& jprops,
                                                              sk_sp<sksg::RenderNode> layer) const {
    enum : size_t {
        kCompletion_Index = 0,
        kAngle_Index      = 1,
        kFeather_Index    = 2,
    };

    auto adapter = sk_make_sp<LinearWipeAdapter>(std::move(layer), fLayerSize);

    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kCompletion_Index), fScope,
        [adapter](const ScalarValue& c) {
            adapter->setCompletion(c);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kAngle_Index), fScope,
        [adapter](const ScalarValue& a) {
            adapter->setAngle(a);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kFeather_Index), fScope,
        [adapter](const ScalarValue& f) {
            adapter->setFeather(f);
        });

    return adapter->root();
}

} // namespace internal
} // namespace skottie


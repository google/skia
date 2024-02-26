/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkTPin.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>

namespace skjson {
class ArrayValue;
}

namespace skottie {
namespace internal {

namespace  {

class LinearWipeAdapter final : public MaskShaderEffectBase {
public:
    static sk_sp<LinearWipeAdapter> Make(const skjson::ArrayValue& jprops,
                                         sk_sp<sksg::RenderNode> layer,
                                         const SkSize& layer_size,
                                         const AnimationBuilder* abuilder) {
        return sk_sp<LinearWipeAdapter>(new LinearWipeAdapter(jprops,
                                                              std::move(layer),
                                                              layer_size,
                                                              abuilder));
    }

private:
    LinearWipeAdapter(const skjson::ArrayValue& jprops,
                      sk_sp<sksg::RenderNode> layer,
                      const SkSize& layer_size,
                      const AnimationBuilder* abuilder)
        : INHERITED(std::move(layer), layer_size) {
        enum : size_t {
            kCompletion_Index = 0,
                 kAngle_Index = 1,
               kFeather_Index = 2,
        };

        EffectBinder(jprops, *abuilder, this)
                .bind(kCompletion_Index, fCompletion)
                .bind(     kAngle_Index, fAngle     )
                .bind(   kFeather_Index, fFeather   );
    }

    MaskInfo onMakeMask() const override {
        if (fCompletion >= 100) {
            // The layer is fully disabled.
            // TODO: fix layer controller visibility clash and pass a null shader instead.
            return { SkShaders::Color(SK_ColorTRANSPARENT), false };
        }

        if (fCompletion <= 0) {
            // The layer is fully visible (no mask).
            return { nullptr, true };
        }

        const auto t = SkTPin(fCompletion * 0.01f, 0.0f, 1.0f),
             feather = std::max(fFeather, 0.0f),
               angle = SkDegreesToRadians(90 - fAngle),
                cos_ = std::cos(angle),
                sin_ = std::sin(angle);

        // Select the correct diagonal vector depending on quadrant.
        const SkVector angle_v = {cos_, sin_},
                        diag_v = {std::copysign(this->layerSize().width() , cos_),
                                  std::copysign(this->layerSize().height(), sin_)};

        // The transition length is the projection of the diagonal onto the angle vector.
        const auto len = SkVector::DotProduct(diag_v, angle_v);

        // Pad the gradient segment to accommodate optional feather ramps at both extremities.
        const auto grad_len   = len + feather * 2;
        const SkVector grad_v = angle_v * grad_len,
              adjusted_grad_v = { grad_v.fX, -grad_v.fY }, // Y flipped for drawing space.
                     center_v = {0.5f * this->layerSize().width(),
                                 0.5f * this->layerSize().height()};

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

        return { SkGradientShader::MakeLinear(pts, colors, pos, 2, SkTileMode::kClamp), true };
    }

    ScalarValue fCompletion = 0,
                fAngle      = 0,
                fFeather    = 0;

    using INHERITED = MaskShaderEffectBase;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachLinearWipeEffect(const skjson::ArrayValue& jprops,
                                                              sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<LinearWipeAdapter>(jprops,
                                                                 std::move(layer),
                                                                 fLayerSize,
                                                                 fBuilder);
}

} // namespace internal
} // namespace skottie


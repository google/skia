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
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <utility>

namespace skjson {
class ArrayValue;
}

namespace skottie {
namespace internal {

namespace  {

class VenetianBlindsAdapter final : public MaskShaderEffectBase {
public:
    static sk_sp<VenetianBlindsAdapter> Make(const skjson::ArrayValue& jprops,
                                             sk_sp<sksg::RenderNode> layer,
                                             const SkSize& layer_size,
                                             const AnimationBuilder* abuilder) {
        return sk_sp<VenetianBlindsAdapter>(
                    new VenetianBlindsAdapter(jprops, std::move(layer), layer_size, abuilder));
    }

private:
    VenetianBlindsAdapter(const skjson::ArrayValue& jprops,
                          sk_sp<sksg::RenderNode> layer, const SkSize& ls,
                          const AnimationBuilder* abuilder)
        : INHERITED(std::move(layer), ls) {
        enum : size_t {
            kCompletion_Index = 0,
             kDirection_Index = 1,
                 kWidth_Index = 2,
               kFeather_Index = 3,
        };

        EffectBinder(jprops, *abuilder, this)
                .bind(kCompletion_Index, fCompletion)
                .bind( kDirection_Index, fDirection )
                .bind(     kWidth_Index, fWidth     )
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

        static constexpr float kFeatherSigmaFactor = 3.0f,
                                       kMinFeather = 0.5f; // for soft gradient edges

        const auto t = fCompletion * 0.01f,
                size = std::max(1.0f, fWidth),
               angle = SkDegreesToRadians(-fDirection),
             feather = std::max(fFeather * kFeatherSigmaFactor, kMinFeather),
                  df = feather / size, // feather distance in normalized stop space
                 df0 = 0.5f * std::min(df,     t),
                 df1 = 0.5f * std::min(df, 1 - t);

        // In its simplest form, the Venetian Blinds effect is a single-step gradient
        // repeating along the direction vector.
        //
        // To avoid an expensive blur pass, we emulate the feather property by softening
        // the gradient edges:
        //
        //  1.0 [                                 |       -------       ]
        //      [                                 |      /       \      ]
        //      [                                 |     /         \     ]
        //      [                                 |    /           \    ]
        //      [                                 |   /             \   ]
        //      [                                 |  /               \  ]
        //      [                                 | /                 \ ]
        //      [                                 |/                   \]
        //  0.5 [                                 |                     ]
        //      [\                               /|                     ]
        //      [ \                             / |                     ]
        //      [  \                           /  |                     ]
        //      [   \                         /   |                     ]
        //      [    \                       /    |                     ]
        //      [     \                     /     |                     ]
        //      [      \                   /      |                     ]
        //  0.0 [       -------------------       |                     ]
        //
        //      ^       ^                 ^       ^       ^     ^       ^
        //      0      fp0               fp1      T      fp2   fp3      1
        //
        //      |       |                 |       |       |     |       |
        //      |< df0 >|                 |< df0 >|< df1 >|     |< df1 >|
        //
        //  ... df     >|                 |<      df     >|     |<      df ...
        //
        // Note 1: fp0-fp1 and/or fp2-fp3 can collapse when df is large enough.
        //
        // Note 2: G(fp0) == G(fp1) and G(fp2) == G(fp3), whether collapsed or not.
        //
        // Note 3: to minimize the number of gradient stops, we can shift the gradient by -df0
        //         (such that fp0 aligns with 0/pts[0]).

        // Gradient value at fp0/fp1, fp2/fp3.
        // Note: g01 > 0 iff fp0-fp1 is collapsed and g23 < 1 iff fp2-fp3 is collapsed
        const auto g01 = std::max(0.0f, 0.5f * (1 + sk_ieee_float_divide(0 - t, df))),
                   g23 = std::min(1.0f, 0.5f * (1 + sk_ieee_float_divide(1 - t, df)));
        SkASSERT(0 <= g01 && g01 <= g23 && g23 <= 1);

        const SkColor c01 = SkColorSetA(SK_ColorWHITE, SkScalarRoundToInt(g01 * 0xff)),
                      c23 = SkColorSetA(SK_ColorWHITE, SkScalarRoundToInt(g23 * 0xff)),
                 colors[] = { c01, c23, c23, c01 };

        const SkScalar pos[] = {
         // 0,              // fp0
            t - df0 - df0,  // fp1
            t + df1 - df0,  // fp2
            1 - df1 - df0,  // fp3
            1,
        };
        static_assert(std::size(colors) == std::size(pos), "");

        const auto center = SkPoint::Make(0.5f * this->layerSize().width(),
                                          0.5f * this->layerSize().height()),
                 grad_vec = SkVector::Make( size * std::cos(angle),
                                           -size * std::sin(angle));

        const SkPoint pts[] = {
            center + grad_vec * (df0 + 0),
            center + grad_vec * (df0 + 1),
        };

        return {
            SkGradientShader::MakeLinear(pts, colors, pos, std::size(colors),
                                         SkTileMode::kRepeat),
            true
        };
    }

    ScalarValue fCompletion = 0,
                fDirection  = 0,
                fWidth      = 0,
                fFeather    = 0;

    using INHERITED = MaskShaderEffectBase;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachVenetianBlindsEffect(
        const skjson::ArrayValue& jprops, sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<VenetianBlindsAdapter>(jprops,
                                                                     std::move(layer),
                                                                     fLayerSize,
                                                                     fBuilder);
}

} // namespace internal
} // namespace skottie

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

namespace skottie {
namespace internal {

namespace  {

class VenetialBlindsAdapter final : public MaskFilterEffectBase {
public:
    VenetialBlindsAdapter(sk_sp<sksg::RenderNode> layer, const SkSize& ls)
        : INHERITED(std::move(layer), ls) {}

    ADAPTER_PROPERTY(Completion, float, 0)
    ADAPTER_PROPERTY(Direction , float, 0)
    ADAPTER_PROPERTY(Width     , float, 0)
    ADAPTER_PROPERTY(Feather   , float, 0)

private:
    MaskInfo onMakeMask() const override {
        if (fCompletion >= 100) {
            // The layer is fully disabled.
            return { nullptr, false };
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
        const auto g01 = std::max(0.0f, 0.5f * (1 + (0 - t) / df)),
                   g23 = std::min(1.0f, 0.5f * (1 + (1 - t) / df));

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
        static_assert(SK_ARRAY_COUNT(colors) == SK_ARRAY_COUNT(pos), "");

        const auto center = SkPoint::Make(0.5f * this->layerSize().width(),
                                          0.5f * this->layerSize().height()),
                 grad_vec = SkVector::Make( size * std::cos(angle),
                                           -size * std::sin(angle));

        const SkPoint pts[] = {
            center + grad_vec * (df0 + 0),
            center + grad_vec * (df0 + 1),
        };

        return {
            SkShaderMaskFilter::Make(SkGradientShader::MakeLinear(pts, colors, pos,
                                                                  SK_ARRAY_COUNT(colors),
                                                                  SkTileMode::kRepeat,
                                                                  0, nullptr)),
            true
        };
    }

    using INHERITED = MaskFilterEffectBase;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachVenetianBlindsEffect(
        const skjson::ArrayValue& jprops, sk_sp<sksg::RenderNode> layer) const {
    enum : size_t {
        kCompletion_Index = 0,
        kDirection_Index  = 1,
        kWidth_Index      = 2,
        kFeather_Index    = 3,
    };

    auto adapter = sk_make_sp<VenetialBlindsAdapter>(std::move(layer), fLayerSize);

    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kCompletion_Index),
        [adapter](const ScalarValue& c) {
            adapter->setCompletion(c);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kDirection_Index),
        [adapter](const ScalarValue& d) {
            adapter->setDirection(d);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kWidth_Index),
        [adapter](const ScalarValue& w) {
            adapter->setWidth(w);
        });
    fBuilder->bindProperty<ScalarValue>(GetPropValue(jprops, kFeather_Index),
        [adapter](const ScalarValue& f) {
            adapter->setFeather(f);
        });

    return adapter->root();
}

} // namespace internal
} // namespace skottie

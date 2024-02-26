/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkAssert.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <cstddef>
#include <utility>

namespace skjson {
class ArrayValue;
}

namespace skottie::internal {
namespace {

// The B&W effect allows controlling individual luminance contribution of
// primary and secondary colors.
//
// The implementation relies on computing primary/secondary relative weights
// for the input color on the hue hexagon, and modulating based on weight
// coefficients.
//
// Note:
//   - at least one of (dr,dg,db) is 0
//   - at least two of (wr,wg,wb) and two of (wy,wc,wm) are 0
//  => we are effectively selecting the color hue sextant without explicit branching
//
// (inspired by https://github.com/RoyiAvital/StackExchangeCodes/blob/master/SignalProcessing/Q688/ApplyBlackWhiteFilter.m)

static sk_sp<SkRuntimeEffect> make_effect() {
    static constexpr char BLACK_AND_WHITE_EFFECT[] =
        "uniform half kR, kY, kG, kC, kB, kM;"

        "half4 main(half4 c) {"
            "half m = min(min(c.r, c.g), c.b),"

                "dr = c.r - m,"
                "dg = c.g - m,"
                "db = c.b - m,"

                // secondaries weights
                "wy = min(dr,dg),"
                "wc = min(dg,db),"
                "wm = min(db,dr),"

                // primaries weights
                "wr = dr - wy - wm,"
                "wg = dg - wy - wc,"
                "wb = db - wc - wm,"

                // final luminance
                "l = m + kR*wr + kY*wy + kG*wg + kC*wc + kB*wb + kM*wm;"

            "return half4(l, l, l, c.a);"
        "}"
    ;

    static const SkRuntimeEffect* effect =
            SkRuntimeEffect::MakeForColorFilter(SkString(BLACK_AND_WHITE_EFFECT)).effect.release();
    SkASSERT(effect);

    return sk_ref_sp(effect);
}

class BlackAndWhiteAdapter final : public DiscardableAdapterBase<BlackAndWhiteAdapter,
                                                                 sksg::ExternalColorFilter> {
public:
    BlackAndWhiteAdapter(const skjson::ArrayValue& jprops,
                         const AnimationBuilder& abuilder,
                         sk_sp<sksg::RenderNode> layer)
        : INHERITED(sksg::ExternalColorFilter::Make(std::move(layer)))
        , fEffect(make_effect())
    {
        SkASSERT(fEffect);

        enum : size_t {
                kReds_Index = 0,
             kYellows_Index = 1,
              kGreens_Index = 2,
               kCyans_Index = 3,
               kBlues_Index = 4,
            kMagentas_Index = 5,
            // TODO
            //    kTint_Index = 6,
            // kTintColorIndex = 7,
        };

        EffectBinder(jprops, abuilder, this)
            .bind(    kReds_Index, fCoeffs[0])
            .bind( kYellows_Index, fCoeffs[1])
            .bind(  kGreens_Index, fCoeffs[2])
            .bind(   kCyans_Index, fCoeffs[3])
            .bind(   kBlues_Index, fCoeffs[4])
            .bind(kMagentas_Index, fCoeffs[5]);
    }

private:
    void onSync() override {
        struct {
            float normalized_coeffs[6];
        } coeffs = {
            (fCoeffs[0] ) / 100,
            (fCoeffs[1] ) / 100,
            (fCoeffs[2] ) / 100,
            (fCoeffs[3] ) / 100,
            (fCoeffs[4] ) / 100,
            (fCoeffs[5] ) / 100,
        };

        this->node()->setColorFilter(
                fEffect->makeColorFilter(SkData::MakeWithCopy(&coeffs, sizeof(coeffs))));
    }

    const sk_sp<SkRuntimeEffect> fEffect;

    ScalarValue                  fCoeffs[6];

    using INHERITED = DiscardableAdapterBase<BlackAndWhiteAdapter, sksg::ExternalColorFilter>;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachBlackAndWhiteEffect(
        const skjson::ArrayValue& jprops, sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<BlackAndWhiteAdapter>(jprops,
                                                                    *fBuilder,
                                                                    std::move(layer));
}

} // namespace skottie::internal

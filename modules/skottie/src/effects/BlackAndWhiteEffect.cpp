/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkTPin.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"

namespace skottie::internal {

namespace  {

static sk_sp<SkRuntimeEffect> make_effect() {
    static constexpr char BLACK_AND_WHITE_EFFECT[] = R"(
        in shader input;
        uniform half kR, kY, kG, kC, kB, kM;

        half4 main() {
            half4 c = sample(input);

            half m = min(min(c.r, c.g), c.b);

            half dR = c.r - m;
            half dG = c.g - m;
            half dB = c.b - m;

            half dRG = min(dR, dG);
            half dGB = min(dG, dB);
            half dBR = min(dB, dR);

            m += kR*(dR - dRG - dBR) +
                 kG*(dG - dRG - dGB) +
                 kB*(dB - dGB - dBR) +
                 kY*dRG +
                 kC*dGB +
                 kM*dBR;

            return half4(m, m, m, c.a);
        }
    )";

    static const SkRuntimeEffect* effect =
            std::get<0>(SkRuntimeEffect::Make(SkString(BLACK_AND_WHITE_EFFECT))).release();
    if (1 && !effect) {
        auto err = std::get<1>(SkRuntimeEffect::Make(SkString(BLACK_AND_WHITE_EFFECT)));
        printf("!!! %s\n", err.c_str());
    }
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
    sk_sp<SkColorFilter> makeCF() const {
        sk_sp<SkColorFilter> input;
        struct {
            float r,y,g,c,b,m;
        } coeffs = {
            (fCoeffs[0] ) / 100,
            (fCoeffs[1] ) / 100,
            (fCoeffs[2] ) / 100,
            (fCoeffs[3] ) / 100,
            (fCoeffs[4] ) / 100,
            (fCoeffs[5] ) / 100,
        };
        return fEffect->makeColorFilter(SkData::MakeWithCopy(&coeffs, sizeof(coeffs)), &input, 1);
    }

    void onSync() override {
        this->node()->setColorFilter(this->makeCF());
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

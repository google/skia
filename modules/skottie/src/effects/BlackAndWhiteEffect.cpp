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
        uniform half coeff_r,
                     coeff_y,
                     coeff_g,
                     coeff_c,
                     coeff_b,
                     coeff_m;

        half4 main() {
            half4 c = sample(input);

            half4 p = (c.g < c.b) ? half4(c.bg, -1,  2/3.0)
                                  : half4(c.gb,  0, -1/3.0);
            half4 q = (c.r < p.x) ? half4(p.x, c.r, p.yw)
                                  : half4(c.r, p.x, p.yz);

            // q.x  -> max channel value
            // q.yz -> 2nd/3rd channel values (unsorted)
            // q.w  -> bias value dependent on max channel selection


            half eps = 0.0001;
            half pmV = q.x;
            half pmC = pmV - min(q.y, q.z);
            half pmL = pmV - pmC * 0.5;
            half   H = abs(q.w + (q.y - q.z) / (pmC * 6 + eps));
            half   S = pmC / (c.a + eps - abs(pmL * 2 - c.a));
            half   L = pmL / (c.a + eps);

            half3 kk = (H < 1.0/6) ? half3(H - 0.0/6, coeff_r, coeff_y) :
                       (H < 2.0/6) ? half3(H - 1.0/6, coeff_y, coeff_g) :
                       (H < 3.0/6) ? half3(H - 2.0/6, coeff_g, coeff_c) :
                       (H < 4.0/6) ? half3(H - 3.0/6, coeff_c, coeff_b) :
                       (H < 5.0/6) ? half3(H - 4.0/6, coeff_b, coeff_m) :
                                     half3(H - 5.0/6, coeff_m, coeff_r);
            
            half t = kk.x * 6;
            half Lc = kk.y + (kk.z - kk.y) * t;
//            half Lc = (1 - step(1.0/6, abs(H - 0.0/6))) * S * coeff_r +
//                      (1 - step(1.0/6, abs(H - 1.0/6))) * S * coeff_y +
//                      (1 - step(1.0/6, abs(H - 2.0/6))) * S * coeff_g +
//                      (1 - step(1.0/6, abs(H - 3.0/6))) * S * coeff_c +
//                      (1 - step(1.0/6, abs(H - 4.0/6))) * S * coeff_b +
//                      (1 - step(1.0/6, abs(H - 5.0/6))) * S * coeff_m;

            L *= 1 + Lc * 5 * S;
            return half4(L, L, L, c.a);
        }
    )";

    static constexpr char BLACK_AND_WHITE_EFFECT2[] = R"(
        in shader input;
        uniform half coeff_r,
                     coeff_y,
                     coeff_g,
                     coeff_c,
                     coeff_b,
                     coeff_m;

        half4 main() {
            half4 c = sample(input);

            half grayPx = min(min(c.r,c.g),c.b);

            half rPx = c.r - grayPx;
            half gPx = c.g - grayPx;
            half bPx = c.b - grayPx;

            if (abs(rPx) < 0.0001) {
                half cyanPx = min(gPx, bPx);
                gPx         = gPx - cyanPx;
                bPx         = bPx - cyanPx;
                grayPx += coeff_g * gPx + coeff_c * cyanPx + coeff_b * bPx;
            } else if (abs(gPx) < 0.0001) {
                half magentaPx = min(rPx, bPx);
                rPx            = rPx - magentaPx;
                bPx            = bPx - magentaPx;

                grayPx += coeff_r * rPx + coeff_b * bPx + coeff_m * magentaPx;
            } else {
                half yellowPx = min(rPx, gPx);
                rPx           = rPx - yellowPx;
                gPx           = gPx - yellowPx;

                grayPx += coeff_r * rPx + coeff_y * yellowPx + coeff_g * gPx;
            }

            return half4(grayPx, grayPx, grayPx, c.a);
        }
    )";

    static const SkRuntimeEffect* effect =
            std::get<0>(SkRuntimeEffect::Make(SkString(BLACK_AND_WHITE_EFFECT2))).release();
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
#if (0)
            (fCoeffs[0] - 50) / 250,
            (fCoeffs[1] - 50) / 250,
            (fCoeffs[2] - 50) / 250,
            (fCoeffs[3] - 50) / 250,
            (fCoeffs[4] - 50) / 250,
            (fCoeffs[5] - 50) / 250,
#else
            (fCoeffs[0] ) / 100,
            (fCoeffs[1] ) / 100,
            (fCoeffs[2] ) / 100,
            (fCoeffs[3] ) / 100,
            (fCoeffs[4] ) / 100,
            (fCoeffs[5] ) / 100,
#endif
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

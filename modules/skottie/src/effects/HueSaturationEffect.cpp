/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTPin.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace skjson {
class ArrayValue;
}

namespace skottie::internal {
namespace {

// AE Saturation semantics:
//
//   - saturation is applied as a component-wise scale (interpolation/extrapolation)
//     relative to chroma mid point
//   - the scale factor is clamped such that none of the components over/under saturates
//     (e.g. below G/R and B are constrained to low_range and high_range, respectively)
//   - the scale is also clammped to a maximum value of 126, empirically
//   - the control is mapped linearly when desaturating, and non-linearly (1/1-S) when saturating
//
// 0               G    R                  B                                   1
// |---------------+----+------------------+-----------------------------------|
//                 |           |           |
//                min         mid         max
//                  <------- chroma ------>
//  <------- low_range -------> <---------------- high_range ----------------->
//
// With some care, we can stay in premul for these calculations.
static constexpr char gSaturateSkSL[] =
"uniform half u_scale;"

"half4 main(half4 c) {"
    // component min/max
    "half2 rg_srt = (c.r < c.g) ? c.rg : c.gr;"
    "half c_min = min(rg_srt.x, c.b),"
         "c_max = max(rg_srt.y, c.b),"

    // chroma and mid-chroma (epsilon to avoid blowing up in the division below)
    "ch     = max(c_max - c_min, 0.0001),"
    "ch_mid = (c_min + c_max)*0.5,"

    // clamp scale to the maximum value which doesn't over/under saturate individual components
    "scale_max = min(ch_mid, c.a - ch_mid)/ch*2,"
    "scale = min(u_scale, scale_max);"

    // lerp
    "c.rgb = ch_mid + (c.rgb - ch_mid)*scale;"

    "return c;"
"}";

static sk_sp<SkColorFilter> make_saturate(float chroma_scale) {
    static const auto* effect =
            SkRuntimeEffect::MakeForColorFilter(SkString(gSaturateSkSL), {}).effect.release();
    SkASSERT(effect);

    return effect->makeColorFilter(SkData::MakeWithCopy(&chroma_scale, sizeof(chroma_scale)));
}

class HueSaturationEffectAdapter final : public AnimatablePropertyContainer {
public:
    static sk_sp<HueSaturationEffectAdapter> Make(const skjson::ArrayValue& jprops,
                                                  sk_sp<sksg::RenderNode> layer,
                                                  const AnimationBuilder* abuilder) {

        return sk_sp<HueSaturationEffectAdapter>(
                    new HueSaturationEffectAdapter(jprops, std::move(layer), abuilder));
    }

    const sk_sp<sksg::ExternalColorFilter>& node() const { return fColorFilter; }

private:
    HueSaturationEffectAdapter(const skjson::ArrayValue& jprops,
                               sk_sp<sksg::RenderNode> layer,
                               const AnimationBuilder* abuilder)
        : fColorFilter(sksg::ExternalColorFilter::Make(std::move(layer))) {
        enum : size_t {
               kChannelControl_Index = 0,
                 kChannelRange_Index = 1,
                    kMasterHue_Index = 2,
                    kMasterSat_Index = 3,
              kMasterLightness_Index = 4,
                     kColorize_Index = 5,
                  kColorizeHue_Index = 6,
                  kColorizeSat_Index = 7,
            kColorizeLightness_Index = 8,
        };

        EffectBinder(jprops, *abuilder, this)
                .bind( kChannelControl_Index, fChanCtrl   )
                .bind(      kMasterHue_Index, fMasterHue  )
                .bind(      kMasterSat_Index, fMasterSat  )
                .bind(kMasterLightness_Index, fMasterLight);

        // TODO: colorize support?
    }

    void onSync() override {
        fColorFilter->setColorFilter(this->makeColorFilter());
    }

    sk_sp<SkColorFilter> makeColorFilter() const {
        enum : uint8_t {
            kMaster_Chan   = 0x01,
            kReds_Chan     = 0x02,
            kYellows_Chan  = 0x03,
            kGreens_Chan   = 0x04,
            kCyans_Chan    = 0x05,
            kBlues_Chan    = 0x06,
            kMagentas_Chan = 0x07,
        };

        // We only support master channel controls at this point.
        if (static_cast<int>(fChanCtrl) != kMaster_Chan) {
            return nullptr;
        }

        sk_sp<SkColorFilter> cf;

        if (!SkScalarNearlyZero(fMasterHue)) {
            // Linear control mapping hue(degrees) -> hue offset]
            const auto h = fMasterHue/360;

            const float cm[20] = {
                1, 0, 0, 0, h,
                0, 1, 0, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 0, 1, 0,
            };

            cf = SkColorFilters::HSLAMatrix(cm);
        }

        if (!SkScalarNearlyZero(fMasterSat)) {
            // AE clamps the max chroma scale to this value.
            static constexpr auto kMaxScale = 126.0f;

            // Control mapping:
            //   * sat [-100 .. 0) -> scale [0 .. 1)   , linear
            //   * sat  [0 .. 100] -> scale [1 .. max] , nonlinear: 100/(100 - sat)
            const auto s            = SkTPin(fMasterSat/100, -1.0f, 1.0f),
                       chroma_scale = s < 0 ? s + 1 : std::min(1/(1 - s), kMaxScale);

            cf = SkColorFilters::Compose(std::move(cf), make_saturate(chroma_scale));
        }

        if (!SkScalarNearlyZero(fMasterLight)) {
            // AE implements Lightness as a component-wise interpolation to 0 (for L < 0),
            // or 1 (for L > 0).
            //
            // Control mapping:
            //   * lightness [-100 .. 0) -> lerp[0 .. 1) from 0, linear
            //   * lightness  [0 .. 100] -> lerp[1 .. 0] from 1, linear
            const auto l  = SkTPin(fMasterLight/100, -1.0f, 1.0f),
                       ls = 1 - std::abs(l),    // scale
                       lo = l < 0 ? 0 : 1 - ls; // offset

            const float cm[20] = {
                ls,  0,  0, 0, lo,
                 0, ls,  0, 0, lo,
                 0,  0, ls, 0, lo,
                 0,  0,  0, 1,  0,
            };

            cf = SkColorFilters::Compose(std::move(cf), SkColorFilters::Matrix(cm));
        }

        return cf;
    }

    const sk_sp<sksg::ExternalColorFilter> fColorFilter;

    float fChanCtrl    = 0.0f,
          fMasterHue   = 0.0f,
          fMasterSat   = 0.0f,
          fMasterLight = 0.0f;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachHueSaturationEffect(
        const skjson::ArrayValue& jprops, sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<HueSaturationEffectAdapter>(jprops,
                                                                          std::move(layer),
                                                                          fBuilder);
}

} // namespace skottie::internal

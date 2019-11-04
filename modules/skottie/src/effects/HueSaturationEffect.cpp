/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/effects/SkColorMatrix.h"
#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

namespace  {

class HueSaturationEffectAdapter final : public DiscardableAdaptorBase {
public:
    static sk_sp<HueSaturationEffectAdapter> Make(const skjson::ArrayValue& jprops,
                                                  sk_sp<sksg::RenderNode> layer,
                                                  const AnimationBuilder* abuilder) {
        enum : size_t {
            kChannelControl_Index     = 0,
            kChannelRange_Index       = 1,
            kMasterHue_Index          = 2,
            kMasterSat_Index          = 3,
            kMasterLightness_Index    = 4,
            kColorize_Index           = 5,
            kColorizeHue_Index        = 6,
            kColorizeSat_Index        = 7,
            kColorizeLightness_Index  = 8,

            kMax_Index = kColorizeLightness_Index
        };

        auto adapter = sk_sp<HueSaturationEffectAdapter>(
                            new HueSaturationEffectAdapter(std::move(layer)));

        auto* raw_adapter = adapter.get();

        abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops,
                                                                        kChannelControl_Index),
            [raw_adapter](const ScalarValue& c) {
                raw_adapter->fChanCtrl = c;
            });
        abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops,
                                                                        kMasterHue_Index),
            [raw_adapter](const ScalarValue& h) {
                raw_adapter->fMasterHue = h;
            });
        abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops,
                                                                        kMasterSat_Index),
            [raw_adapter](const ScalarValue& s) {
                raw_adapter->fMasterSat = s;
            });
        abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops,
                                                                        kMasterLightness_Index),
            [raw_adapter](const ScalarValue& l) {
                raw_adapter->fMasterLight = l;
            });

        // TODO: colorize support?

        return adapter;
    }

    const sk_sp<sksg::ExternalColorFilter>& renderNode() const { return fColorFilter; }

protected:
    void onSync() override {
        fColorFilter->setColorFilter(this->makeColorFilter());
    }

private:
    explicit HueSaturationEffectAdapter(sk_sp<sksg::RenderNode> layer)
        : fColorFilter(sksg::ExternalColorFilter::Make(std::move(layer))) {}

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

        // AE semantics:
        //
        // master       hue [degrees]   => color.H offset
        // master       sat [-100..100] => [-100..0) -> [0 .. color.S)
        //                                 ( 0..100] -> (color.S .. 1]
        // master lightness [-100..100] => [-100..0) -> [0 .. color.L]
        //                                 ( 0..100] -> (color.L .. 1]
        const auto h = fMasterHue / 360,
                   s = SkTPin(fMasterSat   / 100, -1.0f, 1.0f),
                   l = SkTPin(fMasterLight / 100, -1.0f, 1.0f),
                   h_bias  = h,
                   s_bias  = std::max(s, 0.0f),
                   s_scale = 1 - std::abs(s),
                   l_bias  = std::max(l, 0.0f),
                   l_scale = 1 - std::abs(l);

        const float hsl_cm[20] = {
            1,       0,       0, 0, h_bias,
            0, s_scale,       0, 0, s_bias,
            0,       0, l_scale, 0, l_bias,
            0,       0,       0, 1,      0,
        };

        return SkColorFilters::HSLAMatrix(hsl_cm);
    }

    const sk_sp<sksg::ExternalColorFilter> fColorFilter;

    float fChanCtrl    = 0.0f,
          fMasterHue   = 0.0f,
          fMasterSat   = 0.0f,
          fMasterLight = 0.0f;

    using INHERITED = DiscardableAdaptorBase;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachHueSaturationEffect(
        const skjson::ArrayValue& jprops, sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<HueSaturationEffectAdapter>(jprops,
                                                                          std::move(layer),
                                                                          fBuilder);
}

} // namespace internal
} // namespace skottie

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

        // TODO: colorize support

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

        // Saturation is specified in [-100..100]
        const auto s = SkTPin(fMasterSat / 100, -1.0f, 1.0f) + 1,
                   h = SkDegreesToRadians(fMasterHue),
                hcos = std::cos(h),
                hsin = std::sin(h);

        const float hue[20] = {
            0.213f + hcos * 0.787f - hsin * 0.213f,
            0.715f - hcos * 0.715f - hsin * 0.715f,
            0.072f - hcos * 0.072f + hsin * 0.928f,
            0, 0,
            0.213f - hcos * 0.213f + hsin * 0.143f,
            0.715f + hcos * 0.285f + hsin * 0.140f,
            0.072f - hcos * 0.072f - hsin * 0.283f,
            0, 0,
            0.213f - hcos * 0.213f - hsin * 0.787f,
            0.715f - hcos * 0.715f + hsin * 0.715f,
            0.072f + hcos * 0.928f + hsin * 0.072f,
            0, 0,

            0, 0, 0, 1, 0
        };

        const float sat[20] = {
            0.213f + 0.787f * s, 0.715f - 0.715f * s, 0.072f - 0.072f * s, 0, 0,
            0.213f - 0.213f * s, 0.715f + 0.285f * s, 0.072f - 0.072f * s, 0, 0,
            0.213f - 0.213f * s, 0.715f - 0.715f * s, 0.072f + 0.928f * s, 0, 0,
                              0,                   0,                   0, 1, 0
        };

        SkColorMatrix hue_m, sat_m;
        hue_m.set20(hue);
        sat_m.set20(sat);
        hue_m.preConcat(sat_m);

        return SkColorFilters::Matrix(hue_m);
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

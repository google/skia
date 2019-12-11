/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

namespace {

class InvertEffectAdapter final : public DiscardableAdaptorBase {
public:
    static sk_sp<InvertEffectAdapter> Make(const skjson::ArrayValue& jprops,
                                           sk_sp<sksg::RenderNode> layer,
                                           const AnimationBuilder* abuilder) {
        enum : size_t {
            kChannel_Index     = 0,

            kMax_Index = kChannel_Index
        };

        auto adapter = sk_sp<InvertEffectAdapter>(new InvertEffectAdapter(std::move(layer)));

        auto* raw_adapter = adapter.get();
        
        abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops, kChannel_Index),
            [raw_adapter](const ScalarValue& c) {
                raw_adapter->fChannel = c;
            });

        return adapter;
    }

    const sk_sp<sksg::ExternalColorFilter>& renderNode() const { return fColorFilter; }

private:
    explicit InvertEffectAdapter(sk_sp<sksg::RenderNode> layer)
        : fColorFilter(sksg::ExternalColorFilter::Make(std::move(layer))) {}
    
    void onSync() override {
        // https://helpx.adobe.com/after-effects/using/channel-effects.html#invert_effect
        enum : uint8_t {
            kRGB_Channel =  1,
              kR_Channel =  2,
              kG_Channel =  3,
              kB_Channel =  4,
            kHLS_Channel =  6,
              kH_Channel =  7,
              kL_Channel =  8,
              kS_Channel =  9,
              // kYIQ_Channel =  9,
              // kLum_Channel = 10,
              // kIPC_Channel = 11,
              // kQAC_Channel = 12,
              kA_Channel = 16,
        };

        auto       chan = static_cast<uint8_t>(fChannel);
        const auto hsla = chan >= kHLS_Channel && chan <= kS_Channel;
        if (hsla) {
            printf("** chan: %d -> ", chan);
            chan -= kHLS_Channel - 1;
            printf("%d\n", chan);
        }

        const auto ir = chan == kR_Channel || chan == kRGB_Channel,
                   ig = chan == kG_Channel || chan == kRGB_Channel,
                   ib = chan == kB_Channel || chan == kRGB_Channel,
                   ia = chan == kA_Channel;

        auto s = [](bool invert) { return invert ? -1.0f : 1.0f; };
        auto t = [](bool invert) { return invert ?  1.0f : 0.0f; };

        const float m[] = {
            s(ir),     0,     0,     0, t(ir) * (hsla ? 360.f : 1.f),
                0, s(ig),     0,     0, t(ig),
                0,     0, s(ib),     0, t(ib),
                0,     0,     0, s(ia), t(ia),
        };

        fColorFilter->setColorFilter(hsla ? SkColorFilters::HSLAMatrix(m)
                                          : SkColorFilters::Matrix(m));
    }

    const sk_sp<sksg::ExternalColorFilter> fColorFilter;

    float fChannel = 0;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachInvertEffect(const skjson::ArrayValue& jprops,
                                                          sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<InvertEffectAdapter>(jprops,
                                                                   std::move(layer),
                                                                   fBuilder);
}

} // namespace internal
} // namespace skottie

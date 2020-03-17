/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

namespace {

class InvertEffectAdapter final : public AnimatablePropertyContainer {
public:
    static sk_sp<InvertEffectAdapter> Make(const skjson::ArrayValue& jprops,
                                           sk_sp<sksg::RenderNode> layer,
                                           const AnimationBuilder* abuilder) {
        return sk_sp<InvertEffectAdapter>(
                    new InvertEffectAdapter(jprops, std::move(layer), abuilder));
    }

    const sk_sp<sksg::ExternalColorFilter>& node() const { return fColorFilter; }

private:
    InvertEffectAdapter(const skjson::ArrayValue& jprops,
                        sk_sp<sksg::RenderNode> layer,
                        const AnimationBuilder* abuilder)
        : fColorFilter(sksg::ExternalColorFilter::Make(std::move(layer))) {
        enum : size_t {
            kChannel_Index = 0,
        };

        EffectBinder(jprops, *abuilder, this).bind(kChannel_Index, fChannel);
    }

    void onSync() override {
        struct STColorMatrix {
            std::array<float,4> scale,
                                trans;
            bool                hsla;
        };

        const auto stcm = [this]() -> STColorMatrix {
            // https://helpx.adobe.com/after-effects/using/channel-effects.html#invert_effect
            enum : uint8_t {
                kRGB_Channel =  1,
                  kR_Channel =  2,
                  kG_Channel =  3,
                  kB_Channel =  4,

                // NB: HLS vs. HSL
                kHLS_Channel =  6,
                  kH_Channel =  7,
                  kL_Channel =  8,
                  kS_Channel =  9,

                  // kYIQ_Channel = ?,
                  // kLum_Channel = ?,
                  // kIPC_Channel = ?,
                  // kQAC_Channel = ?,

                  kA_Channel = 16,
            };

            switch (static_cast<uint8_t>(fChannel)) {
            case   kR_Channel: return { {-1, 1, 1, 1}, {  1,0,0,0}, false}; // r' = 1 - r
            case   kG_Channel: return { { 1,-1, 1, 1}, {  0,1,0,0}, false}; // g' = 1 - g
            case   kB_Channel: return { { 1, 1,-1, 1}, {  0,0,1,0}, false}; // b' = 1 - b
            case   kA_Channel: return { { 1, 1, 1,-1}, {  0,0,0,1}, false}; // a' = 1 - a
            case kRGB_Channel: return { {-1,-1,-1, 1}, {  1,1,1,0}, false};

            case   kH_Channel: return { {-1, 1, 1, 1}, {.5f,0,0,0},  true}; // h' = .5 - h
            case   kS_Channel: return { { 1,-1, 1, 1}, {  0,1,0,0},  true}; // s' = 1 - s
            case   kL_Channel: return { { 1, 1,-1, 1}, {  0,0,1,0},  true}; // l' = 1 - l
            case kHLS_Channel: return { {-1,-1,-1, 1}, {.5f,1,1,0},  true};

                      default: return { { 1, 1, 1, 1}, {  0,0,0,0}, false};
            }

            SkUNREACHABLE;
        }();

        const float m[] = {
            stcm.scale[0],             0,             0,             0, stcm.trans[0],
                        0, stcm.scale[1],             0,             0, stcm.trans[1],
                        0,             0, stcm.scale[2],             0, stcm.trans[2],
                        0,             0,             0, stcm.scale[3], stcm.trans[3],

        };

        fColorFilter->setColorFilter(stcm.hsla ? SkColorFilters::HSLAMatrix(m)
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

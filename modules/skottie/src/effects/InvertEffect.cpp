/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkRefCnt.h"
#include "include/effects/SkColorMatrix.h"
#include "include/private/base/SkAssert.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace skjson {
class ArrayValue;
}

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
        enum class CS { kRGB, kHSL, kYIQ };

        struct STColorMatrix {
            std::array<float,4> scale,
                                trans;
            CS                  cs;
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

                kYIQ_Channel = 11,
                  kY_Channel = 12,
                  kI_Channel = 13,
                  kQ_Channel = 14,

                  kA_Channel = 16,
            };

            switch (static_cast<uint8_t>(fChannel)) {
            case   kR_Channel: return { {-1, 1, 1, 1}, {  1,0,0,0}, CS::kRGB }; // r' = 1 - r
            case   kG_Channel: return { { 1,-1, 1, 1}, {  0,1,0,0}, CS::kRGB }; // g' = 1 - g
            case   kB_Channel: return { { 1, 1,-1, 1}, {  0,0,1,0}, CS::kRGB }; // b' = 1 - b
            case   kA_Channel: return { { 1, 1, 1,-1}, {  0,0,0,1}, CS::kRGB }; // a' = 1 - a
            case kRGB_Channel: return { {-1,-1,-1, 1}, {  1,1,1,0}, CS::kRGB };

            case   kH_Channel: return { {-1, 1, 1, 1}, {.5f,0,0,0}, CS::kHSL }; // h' = .5 - h
            case   kS_Channel: return { { 1,-1, 1, 1}, {  0,1,0,0}, CS::kHSL }; // s' = 1 - s
            case   kL_Channel: return { { 1, 1,-1, 1}, {  0,0,1,0}, CS::kHSL }; // l' = 1 - l
            case kHLS_Channel: return { {-1,-1,-1, 1}, {.5f,1,1,0}, CS::kHSL };

            case   kY_Channel: return { {-1, 1, 1, 1}, {  1,0,0,0}, CS::kYIQ }; // y' = 1 - y
            case   kI_Channel: return { { 1,-1, 1, 1}, {  0,0,0,0}, CS::kYIQ }; // i' = -i
            case   kQ_Channel: return { { 1, 1,-1, 1}, {  0,0,0,0}, CS::kYIQ }; // q' = -q
            case kYIQ_Channel: return { {-1,-1,-1, 1}, {  1,0,0,0}, CS::kYIQ };

                      default: return { { 1, 1, 1, 1}, {  0,0,0,0}, CS::kRGB };
            }

            SkUNREACHABLE;
        }();

        SkColorMatrix m(
            stcm.scale[0],             0,             0,             0, stcm.trans[0],
                        0, stcm.scale[1],             0,             0, stcm.trans[1],
                        0,             0, stcm.scale[2],             0, stcm.trans[2],
                        0,             0,             0, stcm.scale[3], stcm.trans[3]

        );

        if (stcm.cs == CS::kYIQ) {
            // https://en.wikipedia.org/wiki/YIQ
            static constexpr SkColorMatrix RGB2YIQ(
                0.2990f,  0.5870f,  0.1140f, 0, 0,
                0.5959f, -0.2746f, -0.3213f, 0, 0,
                0.2115f, -0.5227f,  0.3112f, 0, 0,
                      0,        0,        0, 1, 0
            );
            static constexpr SkColorMatrix YIQ2RGB(
                      1,  0.9560f,  0.6190f, 0, 0,
                      1, -0.2720f, -0.6470f, 0, 0,
                      1, -1.1060f,  1.7030f, 0, 0,
                      0,        0,        0, 1, 0
            );

            m.preConcat (RGB2YIQ);
            m.postConcat(YIQ2RGB);
        }

        fColorFilter->setColorFilter(stcm.cs == CS::kHSL ? SkColorFilters::HSLAMatrix(m)
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

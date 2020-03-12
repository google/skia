/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/effects/SkTableColorFilter.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "src/utils/SkJSON.h"

#include <array>
#include <cmath>

namespace skottie {
namespace internal {

namespace  {

struct ClipInfo {
    ScalarValue fClipBlack = 1, // 1: clip, 2/3: don't clip
                fClipWhite = 1; // ^
};

struct ChannelMapper {
    ScalarValue fInBlack  = 0,
                fInWhite  = 1,
                fOutBlack = 0,
                fOutWhite = 1,
                fGamma    = 1;

    const uint8_t* build_lut(std::array<uint8_t, 256>& lut_storage,
                             const ClipInfo& clip_info) const {
        auto in_0 = fInBlack,
             in_1 = fInWhite,
            out_0 = fOutBlack,
            out_1 = fOutWhite,
                g = sk_ieee_float_divide(1, std::max(fGamma, 0.0f));

        float clip[] = {0, 1};
        const auto kLottieDoClip = 1;
        if (SkScalarTruncToInt(clip_info.fClipBlack) == kLottieDoClip) {
            const auto idx = fOutBlack <= fOutWhite ? 0 : 1;
            clip[idx] = SkTPin(out_0, 0.0f, 1.0f);
        }
        if (SkScalarTruncToInt(clip_info.fClipWhite) == kLottieDoClip) {
            const auto idx = fOutBlack <= fOutWhite ? 1 : 0;
            clip[idx] = SkTPin(out_1, 0.0f, 1.0f);
        }
        SkASSERT(clip[0] <= clip[1]);

        if (SkScalarNearlyEqual(in_0, out_0) &&
            SkScalarNearlyEqual(in_1, out_1) &&
            SkScalarNearlyEqual(g, 1)) {
            // no-op
            return nullptr;
        }

        auto dIn  =  in_1 -  in_0,
             dOut = out_1 - out_0;

        if (SkScalarNearlyZero(dIn)) {
            // Degenerate dIn == 0 makes the arithmetic below explode.
            //
            // We could specialize the builder to deal with that case, or we could just
            // nudge by epsilon to make it all work.  The latter approach is simpler
            // and doesn't have any noticeable downsides.
            //
            // Also nudge in_0 towards 0.5, in case it was sqashed against an extremity.
            // This allows for some abrupt transition when the output interval is not
            // collapsed, and produces results closer to AE.
            static constexpr auto kEpsilon = 2 * SK_ScalarNearlyZero;
            dIn  += std::copysign(kEpsilon, dIn);
            in_0 += std::copysign(kEpsilon, .5f - in_0);
            SkASSERT(!SkScalarNearlyZero(dIn));
        }

        auto t =      -in_0 / dIn,
            dT = 1 / 255.0f / dIn;

        for (size_t i = 0; i < 256; ++i) {
            const auto out = out_0 + dOut * std::pow(std::max(t, 0.0f), g);
            SkASSERT(!SkScalarIsNaN(out));

            lut_storage[i] = static_cast<uint8_t>(std::round(SkTPin(out, clip[0], clip[1]) * 255));

            t += dT;
        }

        return lut_storage.data();
    }
};

// ADBE Easy Levels2 color correction effect.
//
// Maps the selected channel(s) from [inBlack...inWhite] to [outBlack, outWhite],
// based on a gamma exponent.
//
// For [i0..i1] -> [o0..o1]:
//
//   c' = o0 + (o1 - o0) * ((c - i0) / (i1 - i0)) ^ G
//
// The output is optionally clipped to the output range.
//
// In/out intervals are clampped to [0..1].  Inversion is allowed.

class EasyLevelsEffectAdapter final : public DiscardableAdapterBase<EasyLevelsEffectAdapter,
                                                                    sksg::ExternalColorFilter> {
public:
    EasyLevelsEffectAdapter(const skjson::ArrayValue& jprops,
                            sk_sp<sksg::RenderNode> layer,
                            const AnimationBuilder* abuilder)
        : INHERITED(sksg::ExternalColorFilter::Make(std::move(layer))) {
        enum : size_t {
                   kChannel_Index = 0,
                   // kHist_Index = 1,
                   kInBlack_Index = 2,
                   kInWhite_Index = 3,
                     kGamma_Index = 4,
                  kOutBlack_Index = 5,
                  kOutWhite_Index = 6,
            kClipToOutBlack_Index = 7,
            kClipToOutWhite_Index = 8,
        };

        EffectBinder(jprops, *abuilder, this)
            .bind(       kChannel_Index, fChannel         )
            .bind(       kInBlack_Index, fMapper.fInBlack )
            .bind(       kInWhite_Index, fMapper.fInWhite )
            .bind(         kGamma_Index, fMapper.fGamma   )
            .bind(      kOutBlack_Index, fMapper.fOutBlack)
            .bind(      kOutWhite_Index, fMapper.fOutWhite)
            .bind(kClipToOutBlack_Index, fClip.fClipBlack )
            .bind(kClipToOutWhite_Index, fClip.fClipWhite );
    }

private:
    void onSync() override {
        enum LottieChannel {
            kRGB_Channel = 1,
              kR_Channel = 2,
              kG_Channel = 3,
              kB_Channel = 4,
              kA_Channel = 5,
        };

        const auto channel = SkScalarTruncToInt(fChannel);
        std::array<uint8_t, 256> lut;
        if (channel < kRGB_Channel || channel > kA_Channel || !fMapper.build_lut(lut, fClip)) {
            this->node()->setColorFilter(nullptr);
            return;
        }

        this->node()->setColorFilter(SkTableColorFilter::MakeARGB(
            channel == kA_Channel                            ? lut.data() : nullptr,
            channel == kR_Channel || channel == kRGB_Channel ? lut.data() : nullptr,
            channel == kG_Channel || channel == kRGB_Channel ? lut.data() : nullptr,
            channel == kB_Channel || channel == kRGB_Channel ? lut.data() : nullptr
        ));
    }

    ChannelMapper fMapper;
    ClipInfo      fClip;
    ScalarValue   fChannel   = 1; // 1: RGB, 2: R, 3: G, 4: B, 5: A

    using INHERITED = DiscardableAdapterBase<EasyLevelsEffectAdapter, sksg::ExternalColorFilter>;
};

// ADBE Pro Levels2 color correction effect.
//
// Similar to ADBE Easy Levels2, but offers separate controls for each channel.

class ProLevelsEffectAdapter final : public DiscardableAdapterBase<ProLevelsEffectAdapter,
                                                                   sksg::ExternalColorFilter> {
public:
    ProLevelsEffectAdapter(const skjson::ArrayValue& jprops,
                           sk_sp<sksg::RenderNode> layer,
                           const AnimationBuilder* abuilder)
        : INHERITED(sksg::ExternalColorFilter::Make(std::move(layer))) {
        enum : size_t {
            //    kHistChan_Index =  0,
            //        kHist_Index =  1,
            //    kRGBBegin_Index =  2,
                kRGBInBlack_Index =  3,
                kRGBInWhite_Index =  4,
                  kRGBGamma_Index =  5,
               kRGBOutBlack_Index =  6,
               kRGBOutWhite_Index =  7,
            //      kRGBEnd_Index =  8,
            //      kRBegin_Index =  9,
                  kRInBlack_Index = 10,
                  kRInWhite_Index = 11,
                    kRGamma_Index = 12,
                 kROutBlack_Index = 13,
                 kROutWhite_Index = 14,
            //        kREnd_Index = 15,
            //      kGBegin_Index = 16,
                  kGInBlack_Index = 17,
                  kGInWhite_Index = 18,
                    kGGamma_Index = 19,
                 kGOutBlack_Index = 20,
                 kGOutWhite_Index = 21,
            //        kGEnd_Index = 22,
            //      kBBegin_Index = 23,
                  kBInBlack_Index = 24,
                  kBInWhite_Index = 25,
                    kBGamma_Index = 26,
                 kBOutBlack_Index = 27,
                 kBOutWhite_Index = 28,
            //        kBEnd_Index = 29,
            //      kABegin_Index = 30,
                  kAInBlack_Index = 31,
                  kAInWhite_Index = 32,
                    kAGamma_Index = 33,
                 kAOutBlack_Index = 34,
                 kAOutWhite_Index = 35,
            //        kAEnd_Index = 36,
            kClipToOutBlack_Index = 37,
            kClipToOutWhite_Index = 38,
        };

        EffectBinder(jprops, *abuilder, this)
            .bind( kRGBInBlack_Index, fRGBMapper.fInBlack )
            .bind( kRGBInWhite_Index, fRGBMapper.fInWhite )
            .bind(   kRGBGamma_Index, fRGBMapper.fGamma   )
            .bind(kRGBOutBlack_Index, fRGBMapper.fOutBlack)
            .bind(kRGBOutWhite_Index, fRGBMapper.fOutWhite)

            .bind( kRInBlack_Index, fRMapper.fInBlack )
            .bind( kRInWhite_Index, fRMapper.fInWhite )
            .bind(   kRGamma_Index, fRMapper.fGamma   )
            .bind(kROutBlack_Index, fRMapper.fOutBlack)
            .bind(kROutWhite_Index, fRMapper.fOutWhite)

            .bind( kGInBlack_Index, fGMapper.fInBlack )
            .bind( kGInWhite_Index, fGMapper.fInWhite )
            .bind(   kGGamma_Index, fGMapper.fGamma   )
            .bind(kGOutBlack_Index, fGMapper.fOutBlack)
            .bind(kGOutWhite_Index, fGMapper.fOutWhite)

            .bind( kBInBlack_Index, fBMapper.fInBlack )
            .bind( kBInWhite_Index, fBMapper.fInWhite )
            .bind(   kBGamma_Index, fBMapper.fGamma   )
            .bind(kBOutBlack_Index, fBMapper.fOutBlack)
            .bind(kBOutWhite_Index, fBMapper.fOutWhite)

            .bind( kAInBlack_Index, fAMapper.fInBlack )
            .bind( kAInWhite_Index, fAMapper.fInWhite )
            .bind(   kAGamma_Index, fAMapper.fGamma   )
            .bind(kAOutBlack_Index, fAMapper.fOutBlack)
            .bind(kAOutWhite_Index, fAMapper.fOutWhite);
    }

private:
    void onSync() override {
        std::array<uint8_t, 256> a_lut_storage,
                                 r_lut_storage,
                                 g_lut_storage,
                                 b_lut_storage;

        auto cf = SkTableColorFilter::MakeARGB(fAMapper.build_lut(a_lut_storage, fClip),
                                               fRMapper.build_lut(r_lut_storage, fClip),
                                               fGMapper.build_lut(g_lut_storage, fClip),
                                               fBMapper.build_lut(b_lut_storage, fClip));

        // The RGB mapper composes outside individual channel mappers.
        if (const auto* rgb_lut = fRGBMapper.build_lut(a_lut_storage, fClip)) {
            cf = SkColorFilters::Compose(SkTableColorFilter::MakeARGB(nullptr,
                                                                      rgb_lut,
                                                                      rgb_lut,
                                                                      rgb_lut),
                                         std::move(cf));
        }

        this->node()->setColorFilter(std::move(cf));
    }

    ChannelMapper fRGBMapper,
                  fRMapper,
                  fGMapper,
                  fBMapper,
                  fAMapper;

    ClipInfo      fClip;

    using INHERITED = DiscardableAdapterBase<ProLevelsEffectAdapter, sksg::ExternalColorFilter>;
};

} // anonymous ns

sk_sp<sksg::RenderNode> EffectBuilder::attachEasyLevelsEffect(const skjson::ArrayValue& jprops,
                                                              sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<EasyLevelsEffectAdapter>(jprops,
                                                                       std::move(layer),
                                                                       fBuilder);
}

sk_sp<sksg::RenderNode> EffectBuilder::attachProLevelsEffect(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<ProLevelsEffectAdapter>(jprops,
                                                                      std::move(layer),
                                                                      fBuilder);
}

} // namespace internal
} // namespace skottie

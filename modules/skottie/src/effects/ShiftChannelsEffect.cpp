/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/private/SkColorData.h"
#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/sksg/include/SkSGColorFilter.h"

namespace skottie {
namespace internal {

namespace {

/*
 * AE's Shift Channels effect overrides individual channels based on a selectable function:
 *
 *     C' = {fR(C), fG(C), fB(C), fA(C)}
 *
 * where fR, fG, fB, fA can be one of
 *
 *     C.r, C.g, C.b, C.a, Luminance(C), Hue(C), Saturation(C), Lightness(C), 1 or 0.
 */
class ShiftChannelsEffectAdapter final : public DiscardableAdaptorBase {
public:
    static sk_sp<ShiftChannelsEffectAdapter> Make(const skjson::ArrayValue& jprops,
                                                  sk_sp<sksg::RenderNode> layer,
                                                  const AnimationBuilder* abuilder) {
        enum : size_t {
            kTakeAlphaFrom_Index = 0,
            kTakeRedFrom_Index   = 1,
            kTakeGreenFrom_Index = 2,
            kTakeBlueFrom_Index  = 3,

            kMax_Index = kTakeBlueFrom_Index
        };

        auto adapter = sk_sp<ShiftChannelsEffectAdapter>(
                    new ShiftChannelsEffectAdapter(std::move(layer)));

        // Use raw captures, pending TheBigRefactoringToComeReallySoonNow.
        auto* raw_adapter = adapter.get();

        abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops,
                                                                        kTakeRedFrom_Index),
            [raw_adapter](const ScalarValue& r) { raw_adapter->fR = r; });
        abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops,
                                                                        kTakeGreenFrom_Index),
            [raw_adapter](const ScalarValue& g) { raw_adapter->fG = g; });
        abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops,
                                                                        kTakeBlueFrom_Index),
            [raw_adapter](const ScalarValue& b) { raw_adapter->fB = b; });
        abuilder->bindProperty<ScalarValue>(EffectBuilder::GetPropValue(jprops,
                                                                        kTakeAlphaFrom_Index),
            [raw_adapter](const ScalarValue& a) { raw_adapter->fA = a; });

        return adapter;
    }

    const sk_sp<sksg::ExternalColorFilter>& renderNode() const { return fColorFilter; }

private:
    explicit ShiftChannelsEffectAdapter(sk_sp<sksg::RenderNode> layer)
        : fColorFilter(sksg::ExternalColorFilter::Make(std::move(layer))) {}

    enum class Source : uint8_t {
        kAlpha      = 1,
        kRed        = 2,
        kGreen      = 3,
        kBlue       = 4,
        kLuminance  = 5,
        kHue        = 6,
        kLightness  = 7,
        kSaturation = 8,
        kFullOn     = 9,
        kFullOff    = 10,

        kMax        = kFullOff
    };

    void onSync() override {
        // TODO: support for HSL sources will require a custom color filter.

        static constexpr float gSourceCoeffs[][5] = {
            {             0,              0,              0, 1, 0}, // kAlpha
            {             1,              0,              0, 0, 0}, // kRed
            {             0,              1,              0, 0, 0}, // kGreen
            {             0,              0,              1, 0, 0}, // kBlue
            {SK_LUM_COEFF_R, SK_LUM_COEFF_G, SK_LUM_COEFF_B, 0, 0}, // kLuminance
            {             0,              0,              0, 0, 0}, // TODO: kHue
            {             0,              0,              0, 0, 0}, // TODO: kLightness
            {             0,              0,              0, 0, 0}, // TODO: kSaturation
            {             0,              0,              0, 0, 1}, // kFullOn
            {             0,              0,              0, 0, 0}, // kFullOff
        };
        static_assert(SK_ARRAY_COUNT(gSourceCoeffs) == static_cast<size_t>(Source::kMax), "");

        auto coeffs = [](float src) {
            // Channel sources are encoded as Source enum values.
            // We map these onto our coeffs table.
            src = SkTPin(src, 1.0f, static_cast<float>(Source::kMax));
            return gSourceCoeffs[static_cast<size_t>(src) - 1];
        };

        const float* rc = coeffs(fR);
        const float* gc = coeffs(fG);
        const float* bc = coeffs(fB);
        const float* ac = coeffs(fA);

        const float cm[] = {
            rc[0], rc[1], rc[2], rc[3], rc[4],
            gc[0], gc[1], gc[2], gc[3], gc[4],
            bc[0], bc[1], bc[2], bc[3], bc[4],
            ac[0], ac[1], ac[2], ac[3], ac[4],
        };

        fColorFilter->setColorFilter(SkColorFilters::Matrix(cm));
    }

    const sk_sp<sksg::ExternalColorFilter> fColorFilter;

    float fR = static_cast<float>(Source::kRed),
          fG = static_cast<float>(Source::kGreen),
          fB = static_cast<float>(Source::kBlue),
          fA = static_cast<float>(Source::kAlpha);

    using INHERITED = DiscardableAdaptorBase;
};

} // namespace


sk_sp<sksg::RenderNode> EffectBuilder::attachShiftChannelsEffect(
        const skjson::ArrayValue& jprops, sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<ShiftChannelsEffectAdapter>(jprops,
                                                                          std::move(layer),
                                                                          fBuilder);
}

} // namespace internal
} // namespace skottie

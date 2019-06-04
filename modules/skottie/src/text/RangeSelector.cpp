/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/RangeSelector.h"

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"

#include <algorithm>
#include <cmath>

namespace skottie {
namespace internal {

namespace  {

// Maps a 1-based JSON enum to one of the values in the array.
template <typename T, typename TArray>
T ParseEnum(const TArray& arr, const skjson::Value& jenum,
            const AnimationBuilder* abuilder, const char* warn_name) {

    const auto idx = ParseDefault<int>(jenum, 1);

    if (idx > 0 && SkToSizeT(idx) <= SK_ARRAY_COUNT(arr)) {
        return arr[idx - 1];
    }

    abuilder->log(Logger::Level::kWarning, nullptr,
                  "Ignoring unknown range selector %s '%d'", warn_name, idx);

    static_assert(SK_ARRAY_COUNT(arr) > 0, "");
    return arr[0];
}

template <RangeSelector::Units>
struct UnitTraits;

template <>
struct UnitTraits<RangeSelector::Units::kPercentage> {
    static constexpr auto Defaults() {
        return std::make_tuple<float, float, float>(0, 100, 0);
    }

    static auto Resolve(float s, float e, float o, size_t domain_size) {
        return std::make_tuple(domain_size * (s + o) / 100,
                               domain_size * (e + o) / 100);
    }
};

template <>
struct UnitTraits<RangeSelector::Units::kIndex> {
    static constexpr auto Defaults() {
        // It's OK to default fEnd to FLOAT_MAX, as it gets clamped when resolved.
        return std::make_tuple<float, float, float>(0, std::numeric_limits<float>::max(), 0);
    }

    static auto Resolve(float s, float e, float o, size_t domain_size) {
        return std::make_tuple(s + o, e + o);
    }
};

template <RangeSelector::Mode>
struct ModeTraits;

template <>
struct ModeTraits<RangeSelector::Mode::kAdd> {
    static void modulate(TextAnimator::AnimatedPropsModulator* c_begin,
                         const TextAnimator::AnimatedPropsModulator* c_end, float amount) {
        if (!amount) return; // 0 -> noop

        for (auto c = c_begin; c < c_end; ++c) {
            c->coverage = SkTPin<float>(c->coverage + amount, -1, 1);
        }
    }
};

} // namespace

sk_sp<RangeSelector> RangeSelector::Make(const skjson::ObjectValue* jrange,
                                         const AnimationBuilder* abuilder,
                                         AnimatorScope *ascope) {
    if (!jrange) {
        return nullptr;
    }

    static constexpr Units gUnitMap[] = {
        Units::kPercentage,  // 'r': 1
        Units::kIndex,       // 'r': 2
    };

    static constexpr Domain gDomainMap[] = {
        Domain::kChars,      // 'b': 1
    };

    static constexpr Mode gModeMap[] = {
        Mode::kAdd,          // 'm': 1
    };

    auto selector = sk_sp<RangeSelector>(
            new RangeSelector(ParseEnum<Units> (gUnitMap  , (*jrange)["r"], abuilder, "units" ),
                              ParseEnum<Domain>(gDomainMap, (*jrange)["b"], abuilder, "domain"),
                              ParseEnum<Mode>  (gModeMap  , (*jrange)["m"], abuilder, "mode"  )));

    abuilder->bindProperty<ScalarValue>((*jrange)["s"], ascope,
        [selector](const ScalarValue& s) {
            selector->fStart = s;
        });
    abuilder->bindProperty<ScalarValue>((*jrange)["e"], ascope,
        [selector](const ScalarValue& e) {
            selector->fEnd = e;
        });
    abuilder->bindProperty<ScalarValue>((*jrange)["o"], ascope,
        [selector](const ScalarValue& o) {
            selector->fOffset = o;
        });
    abuilder->bindProperty<ScalarValue>((*jrange)["a"], ascope,
        [selector](const ScalarValue& a) {
            selector->fAmount = a;
        });

    return selector;
}

RangeSelector::RangeSelector(Units u, Domain d, Mode m)
    : fUnits(u)
    , fDomain(d)
    , fMode(m) {

    // Range defaults are unit-specific.
    switch (fUnits) {
    case Units::kPercentage:
        std::tie(fStart, fEnd, fOffset) = UnitTraits<Units::kPercentage>::Defaults();
        break;
    case Units::kIndex:
        std::tie(fStart, fEnd, fOffset) = UnitTraits<Units::kIndex     >::Defaults();
        break;
    }
}

std::tuple<float, float> RangeSelector::resolve(size_t len) const {
    float f_i0, f_i1;

    SkASSERT(fUnits == Units::kPercentage || fUnits == Units::kIndex);
    const auto resolver = (fUnits == Units::kPercentage)
            ? UnitTraits<Units::kPercentage>::Resolve
            : UnitTraits<Units::kIndex     >::Resolve;

    std::tie(f_i0, f_i1) = resolver(fStart, fEnd, fOffset, len);
    if (f_i0 > f_i1) {
        std::swap(f_i0, f_i1);
    }

    return std::make_tuple(SkTPin<float>(f_i0, 0, len),
                           SkTPin<float>(f_i1, 0, len));
}

void RangeSelector::modulateCoverage(TextAnimator::ModulatorBuffer& buf) const {
    SkASSERT(!buf.empty());

    // Amount is percentage based [-100% .. 100%].
    const auto amount  = SkTPin<float>(fAmount / 100, -1, 1);

    // First, resolve to a float range in the given domain.
    SkAssertResult(fDomain == Domain::kChars);
    const auto f_range = this->resolve(buf.size());

    // Figure out the integral/index coverage range.
    const auto i0 = std::min<size_t>(std::get<0>(f_range), buf.size() - 1),
               i1 = std::min<size_t>(std::get<1>(f_range), buf.size() - 1);

    SkAssertResult(fMode == Mode::kAdd);
    const auto modulate = ModeTraits<Mode::kAdd>::modulate;

    // Apply coverage modulation across different domain segments.
    modulate(buf.data()         , buf.data() + i0        ,      0); // [0 ..i0) -> zero coverage.
    modulate(buf.data() + i0 + 1, buf.data() + i1        , amount); // (i0..i1) -> full coverage.
    modulate(buf.data() + i1 + 1, buf.data() + buf.size(),      0); // (i1.. N] -> zero coverage.

    // For i0 and i1 we have fractional coverage.
    const auto fract_0 = 1 - (std::get<0>(f_range) - i0),
               fract_1 =      std::get<1>(f_range) - i1;
    SkASSERT(fract_0 >= 0 && fract_0 <= 1);
    SkASSERT(fract_1 >= 0 && fract_1 <= 1);

    if (i0 == i1) {
        // The range falls within a single index.
        SkASSERT(fract_0 + fract_1 >= 1);
        modulate(buf.data() + i0, buf.data() + i0 + 1, (fract_0 + fract_1 - 1) * amount);
    } else {
        // Separate indices for i0, i1.
        modulate(buf.data() + i0, buf.data() + i0 + 1, fract_0 * amount);
        modulate(buf.data() + i1, buf.data() + i1 + 1, fract_1 * amount);
    }
}

} // namespace internal
} // namespace skottie

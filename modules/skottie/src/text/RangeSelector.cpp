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

using CoverageProcT = void(*)(float amount,
                              TextAnimator::AnimatedPropsModulator* dst,
                              size_t count);

static const CoverageProcT gCoverageProcs[] = {
    // Mode::kAdd
    [](float amount, TextAnimator::AnimatedPropsModulator* dst, size_t count) {
        if (!amount || !count) return;

        for (size_t i = 0; i < count; ++i) {
            dst[i].coverage = SkTPin<float>(dst[i].coverage + amount, -1, 1);
        }
    },
};

// Each shape generator is defined in a normalized domain, over three |t| intervals:
//
//   (-inf..0) -> lo (constant value)
//   [0..1]    -> func(t)
//   (1..+inf) -> hi (constant value)
//
struct ShapeGenerator {
    float lo,             // constant value for t < 0
          hi;             // constant value for t > 1
    float (*func)(float); // shape generator for t in [0..1]

    // Apply the shape generator, scale, and coverage proc to a single fragment.
    // Should not be called outside the shape domain [0..1].
    void operator()(const CoverageProcT& coverage_proc,
                    float t, float scale,
                    TextAnimator::AnimatedPropsModulator* dst) const {
        SkASSERT(0 <= t && t <= 1);
        const auto coverage = this->func(t);
        coverage_proc(coverage * scale, dst, 1);
    }
};

static const ShapeGenerator gShapeGenerators[] = {
    // Shape::kSquare
    { 0, 0, [](float  )->float { return 1.0f; }},
    // Shape::kRampUp
    { 0, 1, [](float t)->float { return t; }},
    // Shape::kRampDown
    { 1, 0, [](float t)->float { return 1 - t; }},
    // Shape::kTriangle
    { 0, 0, [](float t)->float { return 1 - std::abs(0.5f - t) / 0.5f; }},
    // Shape::kRound
    { 0, 0, [](float t)->float {
                                 static constexpr auto cx  = 0.5f,
                                                       cx2 = cx * cx;
                                 return std::sqrt(cx2 - (t - cx) * (t - cx));
    }},
    // Shape::kSmooth
    { 0, 0, [](float t)->float { return (std::cos(SK_FloatPI * (1 + 2 * t)) + 1) * 0.5f; }},
};

float Lerp(float a, float b, float t) { return a + (b - a) * t; }

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

    static constexpr Shape gShapeMap[] = {
        Shape::kSquare,      // 'sh': 1
        Shape::kRampUp,      // 'sh': 2
        Shape::kRampDown,    // 'sh': 3
        Shape::kTriangle,    // 'sh': 4
        Shape::kRound,       // 'sh': 5
        Shape::kSmooth,      // 'sh': 6
    };

    auto selector = sk_sp<RangeSelector>(
            new RangeSelector(ParseEnum<Units> (gUnitMap  , (*jrange)["r" ], abuilder, "units" ),
                              ParseEnum<Domain>(gDomainMap, (*jrange)["b" ], abuilder, "domain"),
                              ParseEnum<Mode>  (gModeMap  , (*jrange)["m" ], abuilder, "mode"  ),
                              ParseEnum<Shape> (gShapeMap , (*jrange)["sh"], abuilder, "shape" )));

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

RangeSelector::RangeSelector(Units u, Domain d, Mode m, Shape sh)
    : fUnits(u)
    , fDomain(d)
    , fMode(m)
    , fShape(sh) {

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

    return std::make_tuple(f_i0, f_i1);
}

/*
 * General RangeSelector operation:
 *
 *   1) The range is resolved to a target domain (characters, words, etc) interval, based on
 *      |start|, |end|, |offset|, |units|.
 *
 *   2) A shape generator is mapped to this interval and applied across the whole domain, yielding
 *      coverage values in [0..1].
 *
 *   2') When the interval extremes don't coincide with fragment boundaries, the corresponding
 *      fragment coverage is further modulated for partial interval overlap.
 *
 *   3) The coverage is then scaled by the |amount| parameter.
 *
 *   4) Finally, the resulting coverage is accumulated to existing fragment coverage based on
 *      the specified Mode (add, difference, etc).
 */
void RangeSelector::modulateCoverage(TextAnimator::ModulatorBuffer& buf) const {
    SkASSERT(!buf.empty());

    // Amount is percentage based [-100% .. 100%].
    const auto amount = SkTPin<float>(fAmount / 100, -1, 1);

    // First, resolve to a float range in the given domain.
    SkAssertResult(fDomain == Domain::kChars);
    const auto f_range = this->resolve(buf.size());

    // f_range pinned to [0..size].
    const auto f_buf_size = static_cast<float>(buf.size()),
                       f0 = SkTPin(std::get<0>(f_range), 0.0f, f_buf_size),
                       f1 = SkTPin(std::get<1>(f_range), 0.0f, f_buf_size);

    SkASSERT(static_cast<size_t>(fMode) < SK_ARRAY_COUNT(gCoverageProcs));
    const auto& coverage_proc = gCoverageProcs[static_cast<size_t>(fMode)];

    SkASSERT(static_cast<size_t>(fShape) < SK_ARRAY_COUNT(gShapeGenerators));
    const auto& generator = gShapeGenerators[static_cast<size_t>(fShape)];

    // Blit constant coverage outside the shape.
    {
        // Constant coverage count before the shape left edge, and after the right edge.
        const auto count_lo = static_cast<size_t>(std::floor(f0)),
                   count_hi = static_cast<size_t>(f_buf_size - std::ceil (f1));
        SkASSERT(count_lo <= buf.size());
        SkASSERT(count_hi <= buf.size());

        coverage_proc(amount * generator.lo, buf.data()                        , count_lo);
        coverage_proc(amount * generator.hi, buf.data() + buf.size() - count_hi, count_hi);

        if (count_lo == buf.size() || count_hi == buf.size()) {
            // The shape is completely outside the domain - we're done.
            return;
        }
    }

    // Integral/index range.
    const auto i0 = std::min<size_t>(f0, buf.size() - 1),
               i1 = std::min<size_t>(f1, buf.size() - 1);
    SkASSERT(i0 <= i1);

    const auto range_span = std::get<1>(f_range) - std::get<0>(f_range);
    if (SkScalarNearlyZero(range_span)) {
        // Empty range - the shape is collapsed. Modulate with lo/hi weighted average.
        SkASSERT(i0 == i1);
        const auto ratio = f0 - i0,
                coverage = Lerp(generator.lo, generator.hi, ratio);
        coverage_proc(amount * coverage, buf.data() + i0, 1);

        return;
    }

    // Modulate [i0..i1] with shape-generated coverage.
    const auto dt = 1 / range_span;
          // note: we sample mid-fragment
          auto  t = (i0 + 0.5f - std::get<0>(f_range)) / range_span;

    if (i0 == i1) {
        // The whole interval falls within a single fragment.
        const auto fract_coverage = f1 - f0;
        SkASSERT(fract_coverage <= 1);
        generator(coverage_proc, SkTPin(t, 0.0f, 1.0f), amount * fract_coverage, buf.data() + i0);
    } else {
        // The range spans multiple fragments.

        // [i0] -> partial interval coverage modulates the specified amount vs generator.lo
        const auto fract_coverage_0 = 1 - (f0 - i0);
        generator(coverage_proc, std::max(t, 0.0f),
                  amount * Lerp(generator.lo, 1, fract_coverage_0), buf.data() + i0);
        t += dt;

        // (i0..i1) -> full interval coverage => using full amount
        for (auto i = i0 + 1; i < i1; ++i) {
            generator(coverage_proc, t, amount, buf.data() + i);
            t += dt;
        }

        // [i1] -> partial interval coverage modulates the specified amount vs generator.hi
        const auto fract_coverage_1 = (f1 - i1);
        generator(coverage_proc, std::min(t, 1.0f),
                  amount * Lerp(generator.hi, 1, fract_coverage_1), buf.data() + i1);
    }
}

} // namespace internal
} // namespace skottie

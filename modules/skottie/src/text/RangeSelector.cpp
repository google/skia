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

    // For animators without selectors, BM emits dummy selector entries with 0 (inval) props.
    // Supress warnings for these as they are "normal".
    if (idx != 0) {
        abuilder->log(Logger::Level::kWarning, nullptr,
                      "Ignoring unknown range selector %s '%d'", warn_name, idx);
    }

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

class CoverageProcessor {
public:
    CoverageProcessor(const TextAnimator::DomainMaps& maps,
                      RangeSelector::Domain domain,
                      RangeSelector::Mode mode,
                      TextAnimator::ModulatorBuffer& dst)
        : fDst(dst)
        , fDomainSize(dst.size()) {

        SkASSERT(mode == RangeSelector::Mode::kAdd);
        fProc = &CoverageProcessor::add_proc;

        switch (domain) {
        case RangeSelector::Domain::kChars:
            // Direct (1-to-1) index mapping.
            break;
        case RangeSelector::Domain::kCharsExcludingSpaces:
            fMap = &maps.fNonWhitespaceMap;
            break;
        case RangeSelector::Domain::kWords:
            fMap = &maps.fWordsMap;
            break;
        case RangeSelector::Domain::kLines:
            fMap = &maps.fLinesMap;
            break;
        }

        // When no domain map is active, fProc points directly to the mode proc.
        // Otherwise, we punt through a domain mapper proxy.
        if (fMap) {
            fMappedProc = fProc;
            fProc = &CoverageProcessor::domain_map_proc;
            fDomainSize = fMap->size();
        }
    }

    size_t size() const { return fDomainSize; }

    void operator()(float amount, size_t offset, size_t count) const {
        (this->*fProc)(amount, offset, count);
    }

private:
    // mode: kAdd
    void add_proc(float amount, size_t offset, size_t count) const {
        if (!amount || !count) return;

        for (auto* dst = fDst.data() + offset; dst < fDst.data() + offset + count; ++dst) {
            dst->coverage = SkTPin<float>(dst->coverage + amount, -1, 1);
        }
    }

    // A proxy for mapping domain indices to the target buffer.
    void domain_map_proc(float amount, size_t offset, size_t count) const {
        SkASSERT(fMap);
        SkASSERT(fMappedProc);

        for (auto i = offset; i < offset + count; ++i) {
            const auto& span = (*fMap)[i];
            (this->*fMappedProc)(amount, span.fOffset, span.fCount);
        }
    }

    using ProcT = void(CoverageProcessor::*)(float amount, size_t offset, size_t count) const;

    TextAnimator::ModulatorBuffer& fDst;
    ProcT                          fProc,
                                   fMappedProc = nullptr;
    const TextAnimator::DomainMap* fMap = nullptr;
    size_t                         fDomainSize;
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

    float operator()(float t) const { return this->func(t); }
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
                                         const AnimationBuilder* abuilder) {
    if (!jrange) {
        return nullptr;
    }

    static constexpr Units gUnitMap[] = {
        Units::kPercentage,  // 'r': 1
        Units::kIndex,       // 'r': 2
    };

    static constexpr Domain gDomainMap[] = {
        Domain::kChars,                 // 'b': 1
        Domain::kCharsExcludingSpaces,  // 'b': 2
        Domain::kWords,                 // 'b': 3
        Domain::kLines,                 // 'b': 4
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

    abuilder->bindProperty<ScalarValue>((*jrange)["s"],
        [selector](const ScalarValue& s) {
            selector->fStart = s;
        });
    abuilder->bindProperty<ScalarValue>((*jrange)["e"],
        [selector](const ScalarValue& e) {
            selector->fEnd = e;
        });
    abuilder->bindProperty<ScalarValue>((*jrange)["o"],
        [selector](const ScalarValue& o) {
            selector->fOffset = o;
        });
    abuilder->bindProperty<ScalarValue>((*jrange)["a"],
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
void RangeSelector::modulateCoverage(const TextAnimator::DomainMaps& maps,
                                     TextAnimator::ModulatorBuffer& mbuf) const {
    const CoverageProcessor coverage_proc(maps, fDomain, fMode, mbuf);
    if (coverage_proc.size() == 0) {
        return;
    }

    // Amount is percentage based [-100% .. 100%].
    const auto amount = SkTPin<float>(fAmount / 100, -1, 1);

    // First, resolve to a float range in the given domain.
    const auto f_range = this->resolve(coverage_proc.size());

    // f_range pinned to [0..domain_size].
    const auto f_dom_size = static_cast<float>(coverage_proc.size()),
                       f0 = SkTPin(std::get<0>(f_range), 0.0f, f_dom_size),
                       f1 = SkTPin(std::get<1>(f_range), 0.0f, f_dom_size);

    SkASSERT(static_cast<size_t>(fShape) < SK_ARRAY_COUNT(gShapeGenerators));
    const auto& generator = gShapeGenerators[static_cast<size_t>(fShape)];

    // Blit constant coverage outside the shape.
    {
        // Constant coverage count before the shape left edge, and after the right edge.
        const auto count_lo = static_cast<size_t>(std::floor(f0)),
                   count_hi = static_cast<size_t>(f_dom_size - std::ceil (f1));
        SkASSERT(count_lo <= coverage_proc.size());
        SkASSERT(count_hi <= coverage_proc.size());

        coverage_proc(amount * generator.lo, 0                              , count_lo);
        coverage_proc(amount * generator.hi, coverage_proc.size() - count_hi, count_hi);

        if (count_lo == coverage_proc.size() || count_hi == coverage_proc.size()) {
            // The shape is completely outside the domain - we're done.
            return;
        }
    }

    // Integral/index range.
    const auto i0 = std::min<size_t>(f0, coverage_proc.size() - 1),
               i1 = std::min<size_t>(f1, coverage_proc.size() - 1);
    SkASSERT(i0 <= i1);

    const auto range_span = std::get<1>(f_range) - std::get<0>(f_range);
    if (SkScalarNearlyZero(range_span)) {
        // Empty range - the shape is collapsed. Modulate with lo/hi weighted average.
        SkASSERT(i0 == i1);
        const auto ratio = f0 - i0,
                coverage = Lerp(generator.lo, generator.hi, ratio);
        coverage_proc(amount * coverage, i0, 1);

        return;
    }

    // At this point the clamped range maps to the index interval [i0..i1],
    // with the left/right edges falling within i0/i1, respectively:
    //
    //    -----------  ------------------  ------------------  -----------
    //   |  0 |    | .. |    | i0 |    | .. |    | i1 |    | .. |    |  N |
    //    -----------  ------------------  ------------------  -----------
    //                          ^                   ^
    //                          [___________________]
    //
    //                         f0                   f1
    //
    // Note: i0 and i1 can have partial coverage, and also i0 may be the same as i1.

    // Computes partial coverage when one or both range edges fall within the same index [i].
    const auto partial_coverage = [&](float shape_val, float i) {
        // At least one of the range edges falls within the current fragment.
        SkASSERT(SkScalarNearlyEqual(i, std::round(i)));
        SkASSERT((i <= f0 && f0 <= i + 1) || (i <= f1 && f1 <= i + 1));

        // The resulting coverage is a three-way weighted average
        // of the three range segments (lo, shape_val, hi).
        const auto lo_weight = std::max(f0 - i, 0.0f),
                   mi_weight = std::min(f1 - i, 1.0f) - lo_weight,
                   hi_weight = std::max(i + 1 - f1, 0.0f);

        SkASSERT(0 <= lo_weight && lo_weight <= 1);
        SkASSERT(0 <= mi_weight && mi_weight <= 1);
        SkASSERT(0 <= hi_weight && hi_weight <= 1);
        SkASSERT(SkScalarNearlyEqual(lo_weight + mi_weight + hi_weight, 1));

        return lo_weight * generator.lo +
               mi_weight * shape_val +
               hi_weight * generator.hi;
    };

    // The shape domain [0..1] is mapped to the range.
    const auto dt = 1 / range_span;
          // note: we sample mid-fragment
          auto  t = (i0 + 0.5f - std::get<0>(f_range)) / range_span;

    // [i0] may have partial coverage.
    coverage_proc(amount * partial_coverage(generator(std::max(t, 0.0f)), i0), i0, 1);

    // If the whole range falls within a single fragment, we're done.
    if (i0 == i1) {
        return;
    }

    t += dt;

    // [i0+1..i1-1] has full coverage.
    for (auto i = i0 + 1; i < i1; ++i) {
        SkASSERT(0 <= t && t <= 1);
        coverage_proc(amount * generator(t), i, 1);
        t += dt;
    }

    // [i1] may have partial coverage.
    coverage_proc(amount * partial_coverage(generator(std::min(t, 1.0f)), i1), i1, 1);
}

} // namespace internal
} // namespace skottie

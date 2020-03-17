/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/RangeSelector.h"

#include "include/core/SkCubicMap.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"

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


/*
  Selector shapes can be generalized as a signal generator with the following
  parameters/properties:


  1  +               -------------------------
     |              /.           .           .\
     |             / .           .           . \
     |            /  .           .           .  \
     |           /   .           .           .   \
     |          /    .           .           .    \
     |         /     .           .           .     \
     |        /      .           .           .      \
     |       /       .           .           .       \
  0  +----------------------------------------------------------
            ^ <----->            ^            <-----> ^
           e0   crs             sp              crs    e1


    * e0, e1: left/right edges
    * sp    : symmetry/reflection point (sp == (e0+e1)/2)
    * crs   : cubic ramp size (transitional portion mapped using a Bezier easing function)

  Based on these,

            |  0                  , t <= e0
            |
            |  Bez((t-e0)/crs)    , e0 < t < e0+crs
     F(t) = |
            |  1                  , e0 + crs <= t <= sp
            |
            |  F(reflect(t,sp))   , t > sp


   Tweaking this function's parameters, we can achieve all range selectors shapes:

     - square    -> e0:    0, e1:    1, crs: 0
     - ramp up   -> e0:    0, e1: +inf, crs: 1
     - ramp down -> e0: -inf, e1:    1, crs: 1
     - triangle  -> e0:    0, e1:    1, crs: 0.5
     - round     -> e0:    0, e1:    1, crs: 0.5   (nonlinear cubic mapper)
     - smooth    -> e0:    0, e1:    1, crs: 0.5   (nonlinear cubic mapper)

*/

struct ShapeInfo {
   SkVector ctrl0,
            ctrl1;
   float    e0, e1, crs;
};

SkVector EaseVec(float ease) {
    return (ease < 0) ? SkVector{0, -ease} : SkVector{ease, 0};
}

struct ShapeGenerator {
    SkCubicMap shape_mapper,
                ease_mapper;
    float      e0, e1, crs;

    ShapeGenerator(const ShapeInfo& sinfo, float ease_lo, float ease_hi)
        : shape_mapper(sinfo.ctrl0, sinfo.ctrl1)
        , ease_mapper(EaseVec(ease_lo), SkVector{1,1} - EaseVec(ease_hi))
        , e0(sinfo.e0)
        , e1(sinfo.e1)
        , crs(sinfo.crs) {}

    float operator()(float t) const {
        // SkCubicMap clamps its input, so we can let it all hang out.
        t = std::min(t - e0, e1 - t);
        t = sk_ieee_float_divide(t, crs);

        return ease_mapper.computeYFromX(shape_mapper.computeYFromX(t));
    }
};

static constexpr ShapeInfo gShapeInfo[] = {
    { {0  ,0  }, {1  ,1}, 0                       , 1               , 0.0f }, // Shape::kSquare
    { {0  ,0  }, {1  ,1}, 0                       , SK_FloatInfinity, 1.0f }, // Shape::kRampUp
    { {0  ,0  }, {1  ,1}, SK_FloatNegativeInfinity, 1               , 1.0f }, // Shape::kRampDown
    { {0  ,0  }, {1  ,1}, 0                       , 1               , 0.5f }, // Shape::kTriangle
    { {0  ,.5f}, {.5f,1}, 0                       , 1               , 0.5f }, // Shape::kRound
    { {.5f,0  }, {.5f,1}, 0                       , 1               , 0.5f }, // Shape::kSmooth
};

} // namespace

sk_sp<RangeSelector> RangeSelector::Make(const skjson::ObjectValue* jrange,
                                         const AnimationBuilder* abuilder,
                                         AnimatablePropertyContainer* acontainer) {
    if (!jrange) {
        return nullptr;
    }

    enum : int32_t {
             kRange_SelectorType = 0,
        kExpression_SelectorType = 1,

        // kWiggly_SelectorType = ? (not exported)
    };

    {
        const auto type = ParseDefault<int>((*jrange)["t"], kRange_SelectorType);
        if (type != kRange_SelectorType) {
            abuilder->log(Logger::Level::kWarning, nullptr,
                          "Ignoring unsupported selector type '%d'", type);
            return nullptr;
        }
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

    acontainer->bind(*abuilder, (*jrange)["s" ], &selector->fStart );
    acontainer->bind(*abuilder, (*jrange)["e" ], &selector->fEnd   );
    acontainer->bind(*abuilder, (*jrange)["o" ], &selector->fOffset);
    acontainer->bind(*abuilder, (*jrange)["a" ], &selector->fAmount);
    acontainer->bind(*abuilder, (*jrange)["ne"], &selector->fEaseLo);
    acontainer->bind(*abuilder, (*jrange)["xe"], &selector->fEaseHi);

    // Optional square "smoothness" prop.
    if (selector->fShape == Shape::kSquare) {
        acontainer->bind(*abuilder, (*jrange)["sm" ], &selector->fSmoothness);
    }

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

    // Amount, ease-low and ease-high are percentage-based [-100% .. 100%].
    const auto amount = SkTPin<float>(fAmount / 100, -1, 1),
              ease_lo = SkTPin<float>(fEaseLo / 100, -1, 1),
              ease_hi = SkTPin<float>(fEaseHi / 100, -1, 1);

    // Resolve to a float range in the given domain.
    const auto range = this->resolve(coverage_proc.size());
    auto          r0 = std::get<0>(range),
                 len = std::max(std::get<1>(range) - r0, std::numeric_limits<float>::epsilon());

    SkASSERT(static_cast<size_t>(fShape) < SK_ARRAY_COUNT(gShapeInfo));
    ShapeGenerator gen(gShapeInfo[static_cast<size_t>(fShape)], ease_lo, ease_hi);

    if (fShape == Shape::kSquare) {
        // Canonical square generators have collapsed ramps, but AE square selectors have
        // an additional "smoothness" property (0..1) which introduces a non-zero transition.
        // We achieve this by moving the range edges outward by |smoothness|/2, and adjusting
        // the generator cubic ramp size.

        // smoothness is percentage-based [0..100]
        const auto smoothness = SkTPin<float>(fSmoothness / 100, 0, 1);

        r0  -= smoothness / 2;
        len += smoothness;

        gen.crs += smoothness / len;
    }

    SkASSERT(len > 0);
    const auto dt = 1 / len;
          auto  t = (0.5f - r0) / len; // sampling bias: mid-unit

    for (size_t i = 0; i < coverage_proc.size(); ++i, t += dt) {
        coverage_proc(amount * gen(t), i, 1);
    }
}

} // namespace internal
} // namespace skottie

/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieRangeSelector_DEFINED
#define SkottieRangeSelector_DEFINED

#include "include/core/SkRefCnt.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/text/TextAnimator.h"

#include <tuple>
#include <vector>

namespace skottie {
namespace internal {

class RangeSelector final : public SkNVRefCnt<RangeSelector> {
public:
    static sk_sp<RangeSelector> Make(const skjson::ObjectValue*,
                                     const AnimationBuilder*);

    enum class Units : uint8_t {
        kPercentage,  // values are percentages of domain size
        kIndex,       // values are direct domain indices
    };

    enum class Domain : uint8_t {
        kChars,                   // domain indices map to glyph indices
        kCharsExcludingSpaces,    // domain indices map to glyph indices (ignoring spaces)
        kWords,                   // domain indices map to word indices
        kLines,                   // domain indices map to line indices
    };

    enum class Mode : uint8_t {
        kAdd,
        // kSubtract,
        // kIntersect,
        // kMin,
        // kMax,
        // kDifference,
    };

    enum class Shape : uint8_t {
        kSquare,
        kRampUp,
        kRampDown,
        kTriangle,
        kRound,
        kSmooth,
    };

    void modulateCoverage(const TextAnimator::DomainMaps&, TextAnimator::ModulatorBuffer&) const;

private:
    RangeSelector(Units, Domain, Mode, Shape);

    // Resolves this selector to a range in the coverage buffer index domain.
    std::tuple<float, float> resolve(size_t domain_size) const;

    const Units  fUnits;
    const Domain fDomain;
    const Mode   fMode;
    const Shape  fShape;

    float        fStart,
                 fEnd,
                 fOffset,
                 fAmount     = 100,
                 fEaseLo     =   0,
                 fEaseHi     =   0,
                 fSmoothness = 100;
};

} // namespace internal
} // namespace skottie

#endif // SkottieRangeSelector_DEFINED

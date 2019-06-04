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

#include <vector>

namespace skottie {
namespace internal {

class RangeSelector final : public SkNVRefCnt<RangeSelector> {
public:
    static sk_sp<RangeSelector> Make(const skjson::ObjectValue*,
                                     const AnimationBuilder*,
                                     AnimatorScope* ascope);

    using CoverageBuffer = std::vector<float>;
    void modulateCoverage(CoverageBuffer&) const;

    enum class Units : uint8_t {
        kPercentage,
    };

    enum class Base : uint8_t {
        kCharacters,
    };

    enum class Mode : uint8_t {
        kAdd,
    };

private:
    RangeSelector(Units, Base, Mode);

    const Units fUnits;
    const Base  fBase;
    const Mode  fMode;

    float       fStart  = 0,
                fEnd    = 100,
                fOffset = 0,
                fAmount = 100;
};

} // namespace internal
} // namespace skottie

#endif // SkottieRangeSelector_DEFINED

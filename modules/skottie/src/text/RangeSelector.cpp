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
#include <tuple>

namespace skottie {
namespace internal {

sk_sp<RangeSelector> RangeSelector::Make(const skjson::ObjectValue* jrange,
                                         const AnimationBuilder* abuilder,
                                         AnimatorScope *ascope) {
    if (!jrange) {
        return nullptr;
    }


    static const auto get_units = [](const AnimationBuilder* abuilder, int u) {
        switch (u) {
        case 1:  return Units::kPercentage;
        default: break;
        }

        abuilder->log(Logger::Level::kWarning, nullptr,
                      "Ignoring unknown range selector units '%d'", u);
        return Units::kPercentage;
    };

    static const auto get_base = [](const AnimationBuilder* abuilder, int b) {
        switch (b) {
        case 1:  return Base::kCharacters;
        default: break;
        }

        abuilder->log(Logger::Level::kWarning, nullptr,
                      "Ignoring unknown range selector base '%d'", b);
        return Base::kCharacters;
    };

    static const auto get_mode = [](const AnimationBuilder* abuilder, int m) {
        switch (m) {
        case 1:  return Mode::kAdd;
        default: break;
        }

        abuilder->log(Logger::Level::kWarning, nullptr,
                      "Ignoring unknown range selector mode '%d'", m);
        return Mode::kAdd;
    };

    auto selector = sk_sp<RangeSelector>(
                new RangeSelector(get_units(abuilder, ParseDefault<int>((*jrange)["r"], 1)),
                                  get_base (abuilder, ParseDefault<int>((*jrange)["b"], 1)),
                                  get_mode (abuilder, ParseDefault<int>((*jrange)["m"], 1))));

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

template <RangeSelector::Mode>
struct ModeTraits;

template <>
struct ModeTraits<RangeSelector::Mode::kAdd> {
    static void modulate(float* c_begin, const float* c_end, float amount) {
        if (!amount) return;

        for (auto c = c_begin; c < c_end; ++c) {
            *c = SkTPin<float>(*c + amount, -1, 1);
        }
    }
};

RangeSelector::RangeSelector(Units u, Base b, Mode m) : fUnits(u), fBase(b), fMode(m) {}

void RangeSelector::modulateCoverage(CoverageBuffer& buf) const {
    SkASSERT(!buf.empty());

    SkASSERT(fUnits == Units::kPercentage);
    SkASSERT(fBase  == Base::kCharacters);
    SkASSERT(fMode  == Mode::kAdd);

    const auto get_index_range = [this](size_t len) {
        float f_i0, f_i1;

        switch (fUnits) {
        case Units::kPercentage:
            f_i0 = len * (fOffset + fStart) / 100;
            f_i1 = len * (fOffset + fEnd  ) / 100;
            break;
        }

        if (f_i0 > f_i1) {
            std::swap(f_i0, f_i1);
        }
        return std::make_tuple(SkTPin<float>(f_i0, 0, len),
                               SkTPin<float>(f_i1, 0, len));
    };

    const auto f_range = get_index_range(buf.size());
    const auto i_range = std::make_tuple(std::min<size_t>(std::get<0>(f_range), buf.size() - 1),
                                         std::min<size_t>(std::get<1>(f_range), buf.size() - 1));
    const auto amount  = SkTPin<float>(fAmount / 100, -1, 1);

    const auto modulate_proc = [this]() {
        switch (fMode) {
        case Mode::kAdd: return ModeTraits<Mode::kAdd>::modulate;
        }
    }();

    modulate_proc(buf.data()                           , buf.data() + std::get<0>(i_range),      0);
    modulate_proc(buf.data() + std::get<0>(i_range) + 1, buf.data() + std::get<1>(i_range), amount);
    modulate_proc(buf.data() + std::get<1>(i_range) + 1, buf.data() + buf.size(),                0);

    if (std::get<0>(i_range) == std::get<1>(i_range)) {
        const auto fract = std::get<1>(f_range) - std::get<0>(f_range);
        SkASSERT(fract >= 0 && fract <= 1);
        modulate_proc(buf.data() + std::get<0>(i_range),
                      buf.data() + std::get<0>(i_range) + 1,
                      fract * amount);
    } else {
        const auto fract_0 = 1 - (std::get<0>(f_range) - std::get<0>(i_range)),
                   fract_1 =      std::get<1>(f_range) - std::get<1>(i_range);
        SkASSERT(fract_0 >= 0 && fract_0 <= 1);
        SkASSERT(fract_1 >= 0 && fract_1 <= 1);

        modulate_proc(buf.data() + std::get<0>(i_range),
                      buf.data() + std::get<0>(i_range) + 1,
                      fract_0 * amount);
        modulate_proc(buf.data() + std::get<1>(i_range),
                      buf.data() + std::get<1>(i_range) + 1,
                      fract_1 * amount);
    }
}

} // namespace internal
} // namespace skottie

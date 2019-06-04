/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/RangeSelector.h"

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"

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
        case 1:  return BasedOn::kCharacters;
        default: break;
        }

        abuilder->log(Logger::Level::kWarning, nullptr,
                      "Ignoring unknown range selector base '%d'", b);
        return BasedOn::kCharacters;
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

RangeSelector::RangeSelector(Units u, BasedOn b, Mode m) : fUnits(u), fBasedOn(b), fMode(m) {}

void RangeSelector::modulateCoverage(CoverageBuffer& buf) const {
    SkASSERT(fUnits   == Units::kPercentage);
    SkASSERT(fBasedOn == BasedOn::kCharacters);
    SkASSERT(fMode    == Mode::kAdd);

    const auto start = 0.01f * fStart,
                 end = 0.01f * fEnd,
              offset = 0.01f * fOffset;

    auto   start_idx = SkTPin<float>((offset + start) * buf.size(),
                                     0, std::nextafterf(buf.size(), 0)),
             end_idx = SkTPin<float>((offset + end  ) * buf.size(),
                                     0, std::nextafterf(buf.size(), 0));
    if (start_idx > end_idx) {
        std::swap(start_idx, end_idx);
    }

    const auto amount = SkTPin<float>(0.01f * fAmount, -1, 1);

    const auto modulate = [](float src, float dst) {
        return SkTPin<float>(src + dst, -1, 1);
    };

    const auto si = static_cast<size_t>(start_idx),
               ei = static_cast<size_t>(end_idx);

    for (size_t i = 0; i < si; ++i) {
        buf[i] = modulate(0, buf[i]);
    }
    for (size_t i = si + 1; i < ei; ++i) {
        buf[i] = modulate(amount, buf[i]);
    }
    for (size_t i = ei + 1; i < buf.size(); ++i) {
        buf[i] = modulate(0, buf[i]);
    }

    if (si == ei) {
        buf[si] = modulate(end_idx - start_idx, buf[si]);
    } else {
        const auto s_fract = 1 - (start_idx - std::floor(start_idx)),
                   e_fract = end_idx - std::floor(end_idx);
        buf[si] = modulate(s_fract, buf[si]);
        buf[ei] = modulate(e_fract, buf[ei]);
    }

    if (0) {
        printf("** start: %f, end: %f, offset: %f, start_idx: %f, end_idx: %f, si: %lu, ei: %lu, amount: %f\n",
               start, end, offset, start_idx, end_idx, si, ei, amount);
    }
}

} // namespace internal
} // namespace skottie
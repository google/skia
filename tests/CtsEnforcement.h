/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef CtsEnforcement_DEFINED
#define CtsEnforcement_DEFINED

#include "include/core/SkTypes.h"

#include <climits>
#include <cstdint>

/**
 * Determines how unit tests are enforced by CTS. Depending on the ApiLevel, a test will be run
 * in one of 3 states: run without workarounds, run with workarounds or skipped.
 */
class CtsEnforcement {
public:
    enum ApiLevel : int32_t {
        /* When used as fStrictVersion, always skip this test. It is not relevant to CTS.
         * When used as fWorkaroundsVersion, there are no api levels that should run the
         * test with workarounds.
         */
        kNever = INT32_MAX,
        /* The kApiLevel_* values are directly correlated with Android API levels.  Every new
         * CTS/SkQP release has a corresponding Android API level that will be captured by these
         * enum values.
         */
        kApiLevel_T = 33,
        kApiLevel_U = 34,
        kApiLevel_V = 35,
        /* kNextRelease is a placeholder value that all new unit tests should use.  It implies that
         * this test will be enforced in the next Android release.  At the time of the release a
         * new kApiLevel_* value will be added and all current kNextRelease values will be replaced
         * with that new value.
         */
        kNextRelease = kApiLevel_V
    };

    /**
     * Tests will run in strict (no workarounds) mode if the device API level is >= strictVersion
     */
    constexpr CtsEnforcement(ApiLevel strictVersion)
            : fStrictVersion(strictVersion), fWorkaroundsVersion(kNever) {}

    /**
     * Test will run with workarounds if the device API level is >= workaroundVersion
     * and < strictVersion
     */
    constexpr CtsEnforcement& withWorkarounds(ApiLevel workaroundVersion) {
        SkASSERT(workaroundVersion <= fStrictVersion);
        fWorkaroundsVersion = workaroundVersion;
        return *this;
    }

    enum class RunMode { kSkip = 0, kRunWithWorkarounds = 1, kRunStrict = 2 };
    RunMode eval(int apiLevel) const;

private:
    ApiLevel fStrictVersion;
    ApiLevel fWorkaroundsVersion;
};

#endif

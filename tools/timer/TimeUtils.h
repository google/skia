// Copyright 2019 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TimeUtils_DEFINED
#define TimeUtils_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkFloatingPoint.h"

#include <cmath>

namespace TimeUtils {
    // Returns 0 if the timer is stopped. Behavior is undefined if the timer
    // has been running longer than SK_MSecMax.
    static inline SkMSec NanosToMSec(double nanos) {
        const double msec = nanos * 1e-6;
        SkASSERT(SK_MSecMax >= msec);
        return static_cast<SkMSec>(msec);
    }

    static inline double NanosToSeconds(double nanos) {
        return nanos * 1e-9;
    }

    // Return the time scaled by "speed" and (if not zero) mod by period.
    static inline float Scaled(float time, float speed, float period = 0) {
        double value = time * speed;
        if (period) {
            value = ::fmod(value, (double)(period));
        }
        return (float)value;
    }

    // Transitions from ends->mid->ends linearly over period time. The phase
    // specifies a phase shift in time units.
    static inline float PingPong(double time,
                                 float period,
                                 float phase,
                                 float ends,
                                 float mid) {
        double value = ::fmod(time + phase, period);
        double half  = period / 2.0;
        double diff  = ::fabs(value - half);
        return (float)(ends + (1.0 - diff / half) * (mid - ends));
    }

    static inline float SineWave(double time,
                                 float periodInSecs,
                                 float phaseInSecs,
                                 float min,
                                 float max) {
        if (periodInSecs < 0.f) {
            return (min + max) / 2.f;
        }
        double t = NanosToSeconds(time) + phaseInSecs;
        t *= 2 * SK_FloatPI / periodInSecs;
        float halfAmplitude = (max - min) / 2.f;
        return halfAmplitude * std::sin(t) + halfAmplitude + min;
    }
}  // namespace TimeUtils
#endif

/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTPin_DEFINED
#define SkTPin_DEFINED

/** @return x pinned (clamped) between lo and hi, inclusively.

    Unlike std::clamp(), SkTPin() always returns a value between lo and hi.
    If x is NaN, SkTPin() returns hi but std::clamp() returns NaN.
*/
template <typename T>
static constexpr const T& SkTPin(const T& x, const T& lo, const T& hi) {
    // TODO: return std::max(std::min(hi, x), lo)
    // TODO: return std::max(lo, std::min(x, hi)) ?   (clamps NaN to lo)
    return x < lo ? lo
        :  x < hi ? x
        : /*else*/  hi;
}

#endif

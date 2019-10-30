/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkUtils.h"

namespace {  // NOLINT(google-build-namespaces)

inline Sk4px::Wide Sk4px::widen() const {
    return Sk16h((*this)[ 0], (*this)[ 1], (*this)[ 2], (*this)[ 3],
                 (*this)[ 4], (*this)[ 5], (*this)[ 6], (*this)[ 7],
                 (*this)[ 8], (*this)[ 9], (*this)[10], (*this)[11],
                 (*this)[12], (*this)[13], (*this)[14], (*this)[15]);
}

inline Sk4px::Wide Sk4px::mulWiden(const Sk16b& other) const {
    return this->widen() * Sk4px(other).widen();
}

inline Sk4px Sk4px::Wide::addNarrowHi(const Sk16h& other) const {
    Sk4px::Wide r = (*this + other) >> 8;
    return Sk16b(r[ 0], r[ 1], r[ 2], r[ 3],
                 r[ 4], r[ 5], r[ 6], r[ 7],
                 r[ 8], r[ 9], r[10], r[11],
                 r[12], r[13], r[14], r[15]);
}

inline Sk4px Sk4px::Wide::div255() const {
    // Calculated as ((x+128) + ((x+128)>>8)) >> 8.
    auto v = *this + Sk16h(128);
    return v.addNarrowHi(v>>8);
}

inline Sk4px Sk4px::alphas() const {
    static_assert(SK_A32_SHIFT == 24, "This method assumes little-endian.");
    return Sk16b((*this)[ 3], (*this)[ 3], (*this)[ 3], (*this)[ 3],
                 (*this)[ 7], (*this)[ 7], (*this)[ 7], (*this)[ 7],
                 (*this)[11], (*this)[11], (*this)[11], (*this)[11],
                 (*this)[15], (*this)[15], (*this)[15], (*this)[15]);
}

inline Sk4px Sk4px::Load4Alphas(const SkAlpha a[4]) {
    return Sk16b(a[0], a[0], a[0], a[0],
                 a[1], a[1], a[1], a[1],
                 a[2], a[2], a[2], a[2],
                 a[3], a[3], a[3], a[3]);
}

inline Sk4px Sk4px::Load2Alphas(const SkAlpha a[2]) {
    return Sk16b(a[0], a[0], a[0], a[0],
                 a[1], a[1], a[1], a[1],
                 0,0,0,0,
                 0,0,0,0);
}

}  // namespace

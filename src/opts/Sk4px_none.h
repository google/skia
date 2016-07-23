/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkUtils.h"

namespace { // See Sk4px.h

static_assert(sizeof(Sk4px) == 16, "This file uses memcpy / sk_memset32, so exact size matters.");

inline Sk4px Sk4px::DupPMColor(SkPMColor px) {
    Sk4px px4 = Sk16b();
    sk_memset32((uint32_t*)&px4, px, 4);
    return px4;
}

inline Sk4px Sk4px::Load4(const SkPMColor px[4]) {
    Sk4px px4 = Sk16b();
    memcpy(&px4, px, 16);
    return px4;
}

inline Sk4px Sk4px::Load2(const SkPMColor px[2]) {
    Sk4px px2 = Sk16b();
    memcpy(&px2, px, 8);
    return px2;
}

inline Sk4px Sk4px::Load1(const SkPMColor px[1]) {
    Sk4px px1 = Sk16b();
    memcpy(&px1, px, 4);
    return px1;
}

inline void Sk4px::store4(SkPMColor px[4]) const { memcpy(px, this, 16); }
inline void Sk4px::store2(SkPMColor px[2]) const { memcpy(px, this,  8); }
inline void Sk4px::store1(SkPMColor px[1]) const { memcpy(px, this,  4); }

inline Sk4px::Wide Sk4px::widenLo() const {
    return Sk16h((*this)[ 0], (*this)[ 1], (*this)[ 2], (*this)[ 3],
                 (*this)[ 4], (*this)[ 5], (*this)[ 6], (*this)[ 7],
                 (*this)[ 8], (*this)[ 9], (*this)[10], (*this)[11],
                 (*this)[12], (*this)[13], (*this)[14], (*this)[15]);
}

inline Sk4px::Wide Sk4px::widenHi() const { return this->widenLo() << 8; }

inline Sk4px::Wide Sk4px::widenLoHi() const { return this->widenLo() + this->widenHi(); }

inline Sk4px::Wide Sk4px::mulWiden(const Sk16b& other) const {
    return this->widenLo() * Sk4px(other).widenLo();
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

inline Sk4px Sk4px::zeroAlphas() const {
    static_assert(SK_A32_SHIFT == 24, "This method assumes little-endian.");
    return Sk16b((*this)[ 0], (*this)[ 1], (*this)[ 2], 0,
                 (*this)[ 4], (*this)[ 5], (*this)[ 6], 0,
                 (*this)[ 8], (*this)[ 9], (*this)[10], 0,
                 (*this)[12], (*this)[13], (*this)[14], 0);
}

inline Sk4px Sk4px::zeroColors() const {
    static_assert(SK_A32_SHIFT == 24, "This method assumes little-endian.");
    return Sk16b(0,0,0, (*this)[ 3],
                 0,0,0, (*this)[ 7],
                 0,0,0, (*this)[11],
                 0,0,0, (*this)[15]);
}

}  // namespace

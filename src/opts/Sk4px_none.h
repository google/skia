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
    return Sk16h(this->kth< 0>(), this->kth< 1>(), this->kth< 2>(), this->kth< 3>(),
                 this->kth< 4>(), this->kth< 5>(), this->kth< 6>(), this->kth< 7>(),
                 this->kth< 8>(), this->kth< 9>(), this->kth<10>(), this->kth<11>(),
                 this->kth<12>(), this->kth<13>(), this->kth<14>(), this->kth<15>());
}

inline Sk4px::Wide Sk4px::widenHi() const { return this->widenLo() << 8; }

inline Sk4px::Wide Sk4px::widenLoHi() const { return this->widenLo() + this->widenHi(); }

inline Sk4px::Wide Sk4px::mulWiden(const Sk16b& other) const {
    return this->widenLo() * Sk4px(other).widenLo();
}

inline Sk4px Sk4px::Wide::addNarrowHi(const Sk16h& other) const {
    Sk4px::Wide r = (*this + other) >> 8;
    return Sk16b(r.kth< 0>(), r.kth< 1>(), r.kth< 2>(), r.kth< 3>(),
                 r.kth< 4>(), r.kth< 5>(), r.kth< 6>(), r.kth< 7>(),
                 r.kth< 8>(), r.kth< 9>(), r.kth<10>(), r.kth<11>(),
                 r.kth<12>(), r.kth<13>(), r.kth<14>(), r.kth<15>());
}

inline Sk4px Sk4px::alphas() const {
    static_assert(SK_A32_SHIFT == 24, "This method assumes little-endian.");
    return Sk16b(this->kth< 3>(), this->kth< 3>(), this->kth< 3>(), this->kth< 3>(),
                 this->kth< 7>(), this->kth< 7>(), this->kth< 7>(), this->kth< 7>(),
                 this->kth<11>(), this->kth<11>(), this->kth<11>(), this->kth<11>(),
                 this->kth<15>(), this->kth<15>(), this->kth<15>(), this->kth<15>());
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
    return Sk16b(this->kth< 0>(), this->kth< 1>(), this->kth< 2>(), 0,
                 this->kth< 4>(), this->kth< 5>(), this->kth< 6>(), 0,
                 this->kth< 8>(), this->kth< 9>(), this->kth<10>(), 0,
                 this->kth<12>(), this->kth<13>(), this->kth<14>(), 0);
}

inline Sk4px Sk4px::zeroColors() const {
    static_assert(SK_A32_SHIFT == 24, "This method assumes little-endian.");
    return Sk16b(0,0,0, this->kth< 3>(),
                 0,0,0, this->kth< 7>(),
                 0,0,0, this->kth<11>(),
                 0,0,0, this->kth<15>());
}

inline Sk4px Sk4px::Load4(const SkPMColor16 src[4]) {
    SkPMColor src32[4];
    for (int i = 0; i < 4; i++) { src32[i] = SkPixel16ToPixel32(src[i]); }
    return Load4(src32);
}
inline Sk4px Sk4px::Load2(const SkPMColor16 src[2]) {
    SkPMColor src32[2];
    for (int i = 0; i < 2; i++) { src32[i] = SkPixel16ToPixel32(src[i]); }
    return Load2(src32);
}
inline Sk4px Sk4px::Load1(const SkPMColor16 src[1]) {
    SkPMColor src32 = SkPixel16ToPixel32(src[0]);
    return Load1(&src32);
}

inline void Sk4px::store4(SkPMColor16 dst[4]) const {
    SkPMColor dst32[4];
    this->store4(dst32);
    for (int i = 0; i < 4; i++) { dst[i] = SkPixel32ToPixel16(dst32[i]); }
}
inline void Sk4px::store2(SkPMColor16 dst[2]) const {
    SkPMColor dst32[2];
    this->store2(dst32);
    for (int i = 0; i < 2; i++) { dst[i] = SkPixel32ToPixel16(dst32[i]); }
}
inline void Sk4px::store1(SkPMColor16 dst[1]) const {
    SkPMColor dst32;
    this->store1(&dst32);
    dst[0] = SkPixel32ToPixel16(dst32);
}

}  // namespace

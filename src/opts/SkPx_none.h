/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPx_none_DEFINED
#define SkPx_none_DEFINED

// Nothing fancy here.  We're the backup _none case after all.
// Our declared sweet spot is simply a single pixel at a time.

namespace none {

struct SkPx {
    static const int N = 1;
    uint8_t f8[4];

    SkPx(uint32_t px) { memcpy(f8, &px, 4); }
    SkPx(uint8_t x, uint8_t y, uint8_t z, uint8_t a) {
        f8[0] = x; f8[1] = y; f8[2] = z; f8[3] = a;
    }

    static SkPx Dup(uint32_t px) { return px; }
    static SkPx Load(const uint32_t* px) { return *px; }
    static SkPx Load(const uint32_t* px, int n) {
        SkASSERT(false);  // There are no 0<n<1.
        return 0;
    }

    void store(uint32_t* px) const { memcpy(px, f8, 4); }
    void store(uint32_t* px, int n) const {
        SkASSERT(false);  // There are no 0<n<1.
    }

    struct Alpha {
        uint8_t fA;
        Alpha(uint8_t a) : fA(a) {}

        static Alpha Dup(uint8_t a) { return a; }
        static Alpha Load(const uint8_t* a) { return *a; }
        static Alpha Load(const uint8_t* a, int n) {
            SkASSERT(false);  // There are no 0<n<1.
            return 0;
        }
        Alpha inv() const { return 255 - fA; }
    };

    struct Wide {
        uint16_t f16[4];

        Wide(uint16_t x, uint16_t y, uint16_t z, uint16_t a) {
            f16[0] = x; f16[1] = y; f16[2] = z; f16[3] = a;
        }

        Wide operator+(const Wide& o) const {
            return Wide(f16[0]+o.f16[0], f16[1]+o.f16[1], f16[2]+o.f16[2], f16[3]+o.f16[3]);
        }
        Wide operator-(const Wide& o) const {
            return Wide(f16[0]-o.f16[0], f16[1]-o.f16[1], f16[2]-o.f16[2], f16[3]-o.f16[3]);
        }
        template <int bits> Wide shl() const {
            return Wide(f16[0]<<bits, f16[1]<<bits, f16[2]<<bits, f16[3]<<bits);
        }
        template <int bits> Wide shr() const {
            return Wide(f16[0]>>bits, f16[1]>>bits, f16[2]>>bits, f16[3]>>bits);
        }

        SkPx addNarrowHi(const SkPx& o) const {
            Wide sum = (*this + o.widenLo()).shr<8>();
            return SkPx(sum.f16[0], sum.f16[1], sum.f16[2], sum.f16[3]);
        }
    };

    Alpha alpha() const { return f8[3]; }

    Wide widenLo() const { return Wide(f8[0], f8[1], f8[2], f8[3]); }
    Wide widenHi() const { return this->widenLo().shl<8>(); }
    Wide widenLoHi() const { return this->widenLo() + this->widenHi(); }

    SkPx operator+(const SkPx& o) const {
        return SkPx(f8[0]+o.f8[0], f8[1]+o.f8[1], f8[2]+o.f8[2], f8[3]+o.f8[3]);
    }
    SkPx operator-(const SkPx& o) const {
        return SkPx(f8[0]-o.f8[0], f8[1]-o.f8[1], f8[2]-o.f8[2], f8[3]-o.f8[3]);
    }
    SkPx saturatedAdd(const SkPx& o) const {
        return SkPx(SkTMax(0, SkTMin(255, f8[0]+o.f8[0])),
                         SkTMax(0, SkTMin(255, f8[1]+o.f8[1])),
                         SkTMax(0, SkTMin(255, f8[2]+o.f8[2])),
                         SkTMax(0, SkTMin(255, f8[3]+o.f8[3])));
    }

    Wide operator*(const Alpha& a) const {
        return Wide(f8[0]*a.fA, f8[1]*a.fA, f8[2]*a.fA, f8[3]*a.fA);
    }
    SkPx approxMulDiv255(const Alpha& a) const {
        return (*this * a).addNarrowHi(*this);
    }

    SkPx addAlpha(const Alpha& a) const {
        return SkPx(f8[0], f8[1], f8[2], f8[3]+a.fA);
    }
};

}  // namespace none

typedef none::SkPx SkPx;

#endif//SkPx_none_DEFINED

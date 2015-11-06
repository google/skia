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

struct SkPx_none {
    static const int N = 1;
    uint8_t f8[4];

    SkPx_none(uint32_t px) { memcpy(f8, &px, 4); }
    SkPx_none(uint8_t x, uint8_t y, uint8_t z, uint8_t a) {
        f8[0] = x; f8[1] = y; f8[2] = z; f8[3] = a;
    }

    static SkPx_none Dup(uint32_t px) { return px; }
    static SkPx_none Load(const uint32_t* px) { return *px; }
    static SkPx_none Load(const uint32_t* px, int n) {
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
        Wide operator<<(int bits) const {
            return Wide(f16[0]<<bits, f16[1]<<bits, f16[2]<<bits, f16[3]<<bits);
        }
        Wide operator>>(int bits) const {
            return Wide(f16[0]>>bits, f16[1]>>bits, f16[2]>>bits, f16[3]>>bits);
        }

        SkPx_none addNarrowHi(const SkPx_none& o) const {
            Wide sum = (*this + o.widenLo()) >> 8;
            return SkPx_none(sum.f16[0], sum.f16[1], sum.f16[2], sum.f16[3]);
        }
    };

    Alpha alpha() const { return f8[3]; }

    Wide widenLo() const { return Wide(f8[0], f8[1], f8[2], f8[3]); }
    Wide widenHi() const { return this->widenLo() << 8; }
    Wide widenLoHi() const { return this->widenLo() + this->widenHi(); }

    SkPx_none operator+(const SkPx_none& o) const {
        return SkPx_none(f8[0]+o.f8[0], f8[1]+o.f8[1], f8[2]+o.f8[2], f8[3]+o.f8[3]);
    }
    SkPx_none operator-(const SkPx_none& o) const {
        return SkPx_none(f8[0]-o.f8[0], f8[1]-o.f8[1], f8[2]-o.f8[2], f8[3]-o.f8[3]);
    }
    SkPx_none saturatedAdd(const SkPx_none& o) const {
        return SkPx_none(SkTMax(0, SkTMin(255, f8[0]+o.f8[0])),
                         SkTMax(0, SkTMin(255, f8[1]+o.f8[1])),
                         SkTMax(0, SkTMin(255, f8[2]+o.f8[2])),
                         SkTMax(0, SkTMin(255, f8[3]+o.f8[3])));
    }

    Wide operator*(const Alpha& a) const {
        return Wide(f8[0]*a.fA, f8[1]*a.fA, f8[2]*a.fA, f8[3]*a.fA);
    }
    SkPx_none approxMulDiv255(const Alpha& a) const {
        return (*this * a).addNarrowHi(*this);
    }

    SkPx_none addAlpha(const Alpha& a) const {
        return SkPx_none(f8[0], f8[1], f8[2], f8[3]+a.fA);
    }
};
typedef SkPx_none SkPx;

#endif//SkPx_none_DEFINED

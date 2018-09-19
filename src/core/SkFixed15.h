/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFixed15_DEFINED
#define SkFixed15_DEFINED

#include "include/core/SkTypes.h"

// SkFixed15 is a fixed point value that represents values in [0,1] as [0x0000, 0x8000].
// This mapping allows us to implement most operations in tightly packed 16-bit SIMD,
// most notably multiplying using Q15 multiplication instructions (and a little fixup).

class SkFixed15 {
public:
    SkFixed15() = default;

    SkFixed15(float val) : fVal(val * 32768) { SkASSERT(0.0f <= val && val <= 1.0f); }
    explicit operator float() const { return fVal * (1/32768.0f); }

    static SkFixed15 Load(uint16_t val) {
        SkASSERT(val <= 32768);
        return val;
    }
    uint16_t store() const { return fVal; }

    static SkFixed15 FromU8(uint8_t val) {
        return val*128 + (val>>1)  // 32768/255 == 128.50196..., which is very close to 128 + 0.5.
             + ((val+1)>>8);       // All val but 255 are correct.  +1 if val == 255 to get 32768.
    }

    uint8_t to_u8() const {
        // FromU8() and to_u8() roundtrip all bytes.
        // There is still much room to tweak this towards the ideal, a rounding scale by 255/32768.
        return (fVal - (fVal>>8))>>7;
    }

    SkFixed15 operator +(SkFixed15 o) const { return fVal + o.fVal; }
    SkFixed15 operator -(SkFixed15 o) const { return fVal - o.fVal; }
    SkFixed15 operator *(SkFixed15 o) const { return (fVal * o.fVal + (1<<14)) >> 15; }
    SkFixed15 operator<<(int bits) const { return fVal << bits; }
    SkFixed15 operator>>(int bits) const { return fVal >> bits; }

    SkFixed15& operator +=(SkFixed15 o) { return (*this = *this + o); }
    SkFixed15& operator -=(SkFixed15 o) { return (*this = *this - o); }
    SkFixed15& operator *=(SkFixed15 o) { return (*this = *this * o); }
    SkFixed15& operator<<=(int bits) { return (*this = *this << bits); }
    SkFixed15& operator>>=(int bits) { return (*this = *this >> bits); }

    bool operator==(SkFixed15 o) const { return fVal == o.fVal; }
    bool operator!=(SkFixed15 o) const { return fVal != o.fVal; }
    bool operator<=(SkFixed15 o) const { return fVal <= o.fVal; }
    bool operator>=(SkFixed15 o) const { return fVal >= o.fVal; }
    bool operator< (SkFixed15 o) const { return fVal <  o.fVal; }
    bool operator> (SkFixed15 o) const { return fVal >  o.fVal; }

private:
    SkFixed15(int val) : fVal(val) {}

    uint16_t fVal;
};

// Notes
//  - SSSE3+ multiply is _mm_abs_epi16(_mm_mulhrs_epi16(x, y));
//  - NEON multipy is vsraq_n_u16(vabsq_s16(vqrdmulhq_s16(x,y)),
//                                vandq_s16(x,y), 15);
//  - Conversion to and from float can be done manually with bit masks and float add/subtract,
//    rather than the naive version here involving int<->float conversion and float multiply.
//  - On x86, conversion to float is _mm_sub_ps(_mm_unpacklo_epi16(x, _mm_set1_epi16(0x4380)),
//                                              _mm_set1_ps(256.0f)).  // 0x43800000
//  - On ARM, we can use the vcvtq_n_f32_u32(vmovl_u16(x), 15) to convert to float,
//    and vcvtq_n_u32_f32(..., 15) for the other way around.

#endif//SkFixed15_DEFINED

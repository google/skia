/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFix_DEFINED
#define SkFix_DEFINED

#include "SkTypes.h"

// SkFix is a relatively high-precision (~15 bit) fixed point value that can
// represent values between 0 and 1 inclusively.  It makes a good temporary
// linear color channel.  It is _not_ a unorm16.

class SkFix {
public:
    SkFix() = default;

    SkFix(float val) : fVal(val * 32768) {}
    explicit operator float() const { return fVal * (1/32768.0f); }

    static SkFix Load(uint16_t val) { return SkFix(val); }
    uint16_t store() const { return fVal; }

    SkFix operator +(SkFix  o) const { return fVal + o.fVal; }
    SkFix operator -(SkFix  o) const { return fVal - o.fVal; }
    SkFix operator *(SkFix  o) const { return (fVal * o.fVal + (1<<14)) >> 15; }
    SkFix operator<<(int bits) const { return fVal << bits; }
    SkFix operator>>(int bits) const { return fVal >> bits; }

    SkFix& operator +=(SkFix  o) { return (*this = *this + o); }
    SkFix& operator -=(SkFix  o) { return (*this = *this - o); }
    SkFix& operator *=(SkFix  o) { return (*this = *this * o); }
    SkFix& operator<<=(int bits) { return (*this = *this << bits); }
    SkFix& operator>>=(int bits) { return (*this = *this >> bits); }

    bool operator==(SkFix o) const { return fVal == o.fVal; }
    bool operator!=(SkFix o) const { return fVal != o.fVal; }
    bool operator<=(SkFix o) const { return fVal <= o.fVal; }
    bool operator>=(SkFix o) const { return fVal >= o.fVal; }
    bool operator< (SkFix o) const { return fVal <  o.fVal; }
    bool operator> (SkFix o) const { return fVal >  o.fVal; }

private:
    SkFix(int val) : fVal(val) {}

    uint16_t fVal;
};

#endif//SkFix_DEFINED

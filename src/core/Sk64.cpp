/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk64.h"
#include "SkMathPriv.h"

#define shift_left(hi, lo)          \
    hi = (hi << 1) | (lo >> 31);    \
    lo <<= 1

#define shift_left_bits(hi, lo, bits)           \
    SkASSERT((unsigned)(bits) < 31);                \
    hi = (hi << (bits)) | (lo >> (32 - (bits)));    \
    lo <<= (bits)

//////////////////////////////////////////////////////////////////////

int Sk64::getClzAbs() const
{
    int32_t     hi = fHi;
    uint32_t    lo = fLo;

    // get abs
    if (hi < 0)
    {
        hi = -hi - Sk32ToBool(lo);
        lo = 0 - lo;
    }
    return hi ? SkCLZ(hi) : SkCLZ(lo) + 32;
}

void Sk64::shiftLeft(unsigned bits)
{
    SkASSERT(bits <= 63);
    if (bits == 0)
        return;

    if (bits >= 32)
    {
        fHi = fLo << (bits - 32);
        fLo = 0;
    }
    else
    {
        fHi = (fHi << bits) | (fLo >> (32 - bits));
        fLo <<= bits;
    }
}

int32_t Sk64::getShiftRight(unsigned bits) const
{
    SkASSERT(bits <= 63);

    if (bits == 0)
        return fLo;

    if (bits >= 32)
        return fHi >> (bits - 32);
    else
    {
#ifdef SK_DEBUG
        int32_t tmp = fHi >> bits;
        SkASSERT(tmp == 0 || tmp == -1);
#endif
        return (fHi << (32 - bits)) | (fLo >> bits);
    }
}

void Sk64::shiftRight(unsigned bits)
{
    SkASSERT(bits <= 63);
    if (bits == 0)
        return;

    if (bits >= 32)
    {
        fLo = fHi >> (bits - 32);
        fHi >>= 31;
    }
    else
    {
        fLo = (fHi << (32 - bits)) | (fLo >> bits);
        fHi >>= bits;
    }
}

void Sk64::roundRight(unsigned bits)
{
    SkASSERT(bits <= 63);
    if (bits)
    {
        Sk64 one;
        one.set(1);
        one.shiftLeft(bits - 1);
        this->add(one);
        this->shiftRight(bits);
    }
}

int Sk64::shiftToMake32() const
{
    int32_t     hi = fHi;
    uint32_t    lo = fLo;

    if (hi < 0) // make it positive
    {
        hi = -hi - Sk32ToBool(lo);
        lo = 0 - lo;
    }

    if (hi == 0)
        return lo >> 31;
    else
        return 33 - SkCLZ(hi);
}

void Sk64::negate()
{
    fHi = -fHi - Sk32ToBool(fLo);
    fLo = 0 - fLo;
}

void Sk64::abs()
{
    if (fHi < 0)
    {
        fHi = -fHi - Sk32ToBool(fLo);
        fLo = 0 - fLo;
    }
}

#if 0
SkBool Sk64::isFixed() const
{
    Sk64 tmp = *this;
    tmp.roundRight(16);
    return tmp.is32();
}
#endif

void Sk64::sub(const Sk64& a)
{
    fHi = fHi - a.fHi - (fLo < a.fLo);
    fLo = fLo - a.fLo;
}

void Sk64::rsub(const Sk64& a)
{
    fHi = a.fHi - fHi - (a.fLo < fLo);
    fLo = a.fLo - fLo;
}

void Sk64::setMul(int32_t a, int32_t b)
{
    int sa = a >> 31;
    int sb = b >> 31;
    // now make them positive
    a = (a ^ sa) - sa;
    b = (b ^ sb) - sb;

    uint32_t    ah = a >> 16;
    uint32_t    al = a & 0xFFFF;
    uint32_t bh = b >> 16;
    uint32_t bl = b & 0xFFFF;

    uint32_t A = ah * bh;
    uint32_t B = ah * bl + al * bh;
    uint32_t C = al * bl;

    /*  [  A  ]
           [  B  ]
              [  C  ]
    */
    fLo = C + (B << 16);
    fHi = A + (B >>16) + (fLo < C);

    if (sa != sb)
        this->negate();
}

void Sk64::div(int32_t denom, DivOptions option)
{
    SkASSERT(denom);

    int32_t     hi = fHi;
    uint32_t    lo = fLo;
    int         sign = denom ^ hi;

    denom = SkAbs32(denom);
    if (hi < 0)
    {
        hi = -hi - Sk32ToBool(lo);
        lo = 0 - lo;
    }

    if (option == kRound_DivOption) // add denom/2
    {
        uint32_t newLo = lo + (denom >> 1);
        hi += (newLo < lo);
        lo = newLo;
    }

    if (hi == 0)    // fast-case
    {
        if (lo < (uint32_t)denom)
            this->set(0, 0);
        else
        {
            this->set(0, lo / denom);
            if (sign < 0)
                this->negate();
        }
        return;
    }

    int bits;

    {
        int dbits = SkCLZ(denom);
        int nbits = SkCLZ(hi);

        bits = 32 + dbits - nbits;
        SkASSERT(bits <= 63);
        if (bits <= 0)
        {
            this->set(0, 0);
            return;
        }
        denom <<= (dbits - 1);
        shift_left_bits(hi, lo, nbits - 1);
    }

    int32_t     rhi = 0;
    uint32_t    rlo = 0;

    do {
        shift_left(rhi, rlo);
        if ((uint32_t)denom <= (uint32_t)hi)
        {
            hi -= denom;
            rlo |= 1;
        }
        shift_left(hi, lo);
    } while (--bits >= 0);
    SkASSERT(rhi >= 0);

    fHi = rhi;
    fLo = rlo;
    if (sign < 0)
        this->negate();
}

#define shift_left_2(a, b, c)   \
    a = (a << 2) | (b >> 30);   \
    b = (b << 2) | (c >> 30);   \
    c <<= 2

int32_t Sk64::getSqrt() const
{
    SkASSERT(!this->isNeg());

    uint32_t    hi = fHi;
    uint32_t lo = fLo;
    uint32_t    sqr = 0;
    uint32_t root = 0;
    int count = 31;

    do {
        root <<= 1;
        shift_left_2(sqr, hi, lo);

        uint32_t testDiv = (root << 1) + 1;
        if (sqr >= testDiv)
        {
            sqr -= testDiv;
            root++;
        }
    } while (--count >= 0);
    SkASSERT((int32_t)root >= 0);

    return root;
}

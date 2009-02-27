/* libs/corecg/Sk64.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "Sk64.h"

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

////////////////////////////////////////////////////////////////

static inline int32_t round_right_16(int32_t hi, uint32_t lo)
{
    uint32_t sum = lo + (1 << 15);
    hi += (sum < lo);
    return (hi << 16) | (sum >> 16);
}

SkBool Sk64::isFixed() const
{
    Sk64 tmp = *this;
    tmp.roundRight(16);
    return tmp.is32();
}

SkFract Sk64::getFract() const
{
    Sk64 tmp = *this;
    tmp.roundRight(30);
    return tmp.get32();
}

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
#ifdef SK_CPU_HAS_CONDITIONAL_INSTR
        if ((uint32_t)denom <= (uint32_t)hi)
        {
            hi -= denom;
            rlo |= 1;
        }
#else
        int32_t diff = (denom - hi - 1) >> 31;
        hi -= denom & diff;
        rlo -= diff;
#endif
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

#ifdef SkLONGLONG
    SkLONGLONG Sk64::getLongLong() const
    {
        SkLONGLONG value = fHi;
        value <<= 32;
        return value | fLo;
    }
#endif

SkFixed Sk64::getFixedDiv(const Sk64& denom) const
{
    Sk64    N = *this;
    Sk64    D = denom;
    int32_t sign = SkExtractSign(N.fHi ^ D.fHi);
    SkFixed result;

    N.abs();
    D.abs();

    // need to knock D down to just 31 bits
    // either by rounding it to the right, or shifting N to the left
    // then we can just call 64/32 div

    int nclz = N.fHi ? SkCLZ(N.fHi) : 32;
    int dclz = D.fHi ? SkCLZ(D.fHi) : (33 - (D.fLo >> 31));

    int shiftN = nclz - 1;
    SkASSERT(shiftN >= 0);
    int shiftD = 33 - dclz;
    SkASSERT(shiftD >= 0);

    if (shiftD + shiftN < 16)
        shiftD = 16 - shiftN;
    else
        shiftN = 16 - shiftD;

    D.roundRight(shiftD);
    if (D.isZero())
        result = SK_MaxS32;
    else
    {
        if (shiftN >= 0)
            N.shiftLeft(shiftN);
        else
            N.roundRight(-shiftN);
        N.div(D.get32(), Sk64::kTrunc_DivOption);
        if (N.is32())
            result = N.get32();
        else
            result = SK_MaxS32;
    }
    return SkApplySign(result, sign);
}


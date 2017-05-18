/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4px_DEFINED
#define Sk4px_DEFINED

#include "SkNx.h"
#include "SkColor.h"
#include "SkColorPriv.h"

// This file may be included multiple times by .cpp files with different flags, leading
// to different definitions.  Usually that doesn't matter because it's all inlined, but
// in Debug modes the compilers may not inline everything.  So wrap everything in an
// anonymous namespace to give each includer their own silo of this code (or the linker
// will probably pick one randomly for us, which is rarely correct).
namespace {

// 1, 2 or 4 SkPMColors, generally vectorized.
class Sk4px : public Sk16b {
public:
    static Sk4px DupAlpha(SkAlpha a) { return Sk16b(a); }  //    a -> aaaa aaaa aaaa aaaa
    static Sk4px DupPMColor(SkPMColor c);                  // argb -> argb argb argb argb

    Sk4px(const Sk16b& v) : INHERITED(v) {}

    Sk4px alphas() const;  // ARGB argb XYZW xyzw -> AAAA aaaa XXXX xxxx

    // Mask away color or alpha lanes.
    Sk4px zeroColors() const;  // ARGB argb XYZW xyzw -> A000 a000 X000 x000
    Sk4px zeroAlphas() const;  // ARGB argb XYZW xyzw -> 0RGB 0rgb 0YZW 0yzw

    Sk4px inv() const { return Sk16b(255) - *this; }

    // When loading or storing fewer than 4 SkPMColors, we use the low lanes.
    static Sk4px Load4(const SkPMColor[4]);  // PMColor[4] -> ARGB argb XYZW xyzw
    static Sk4px Load2(const SkPMColor[2]);  // PMColor[2] -> ARGB argb ???? ????
    static Sk4px Load1(const SkPMColor[1]);  // PMColor[1] -> ARGB ???? ???? ????

    // Ditto for Alphas... Load2Alphas fills the low two lanes of Sk4px.
    static Sk4px Load4Alphas(const SkAlpha[4]);  // AaXx -> AAAA aaaa XXXX xxxx
    static Sk4px Load2Alphas(const SkAlpha[2]);  // Aa   -> AAAA aaaa ???? ????

    void store4(SkPMColor[4]) const;
    void store2(SkPMColor[2]) const;
    void store1(SkPMColor[1]) const;

    // 1, 2, or 4 SkPMColors with 16-bit components.
    // This is most useful as the result of a multiply, e.g. from mulWiden().
    class Wide : public Sk16h {
    public:
        Wide(const Sk16h& v) : Sk16h(v) {}

        // Pack the top byte of each component back down into 4 SkPMColors.
        Sk4px addNarrowHi(const Sk16h&) const;

        // Rounds, i.e. (x+127) / 255.
        Sk4px div255() const;

        // These just keep the types as Wide so the user doesn't have to keep casting.
        Wide operator * (const Wide& o) const { return INHERITED::operator*(o); }
        Wide operator + (const Wide& o) const { return INHERITED::operator+(o); }
        Wide operator - (const Wide& o) const { return INHERITED::operator-(o); }
        Wide operator >> (int bits) const { return INHERITED::operator>>(bits); }
        Wide operator << (int bits) const { return INHERITED::operator<<(bits); }
        static Wide Min(const Wide& a, const Wide& b) { return INHERITED::Min(a,b); }
        Wide thenElse(const Wide& t, const Wide& e) const { return INHERITED::thenElse(t,e); }

    private:
        typedef Sk16h INHERITED;
    };

    Wide widenLo() const;               // ARGB -> 0A 0R 0G 0B
    Wide widenHi() const;               // ARGB -> A0 R0 G0 B0
    Wide widenLoHi() const;             // ARGB -> AA RR GG BB
    Wide mulWiden(const Sk16b&) const;  // 8-bit x 8-bit -> 16-bit components.

    // The only 8-bit multiply we use is 8-bit x 8-bit -> 16-bit.  Might as well make it pithy.
    Wide operator * (const Sk4px& o) const { return this->mulWiden(o); }

    // These just keep the types as Sk4px so the user doesn't have to keep casting.
    Sk4px operator + (const Sk4px& o) const { return INHERITED::operator+(o); }
    Sk4px operator - (const Sk4px& o) const { return INHERITED::operator-(o); }
    Sk4px operator < (const Sk4px& o) const { return INHERITED::operator<(o); }
    Sk4px thenElse(const Sk4px& t, const Sk4px& e) const { return INHERITED::thenElse(t,e); }

    // Generally faster than (*this * o).div255().
    // May be incorrect by +-1, but is always exactly correct when *this or o is 0 or 255.
    Sk4px approxMulDiv255(const Sk16b& o) const {
        // (x*y + x) / 256 meets these criteria.  (As of course does (x*y + y) / 256 by symmetry.)
        // FYI: (x*y + 255) / 256 also meets these criteria.  In my brief testing, it was slower.
        return this->widenLo().addNarrowHi(*this * o);
    }

    // A generic driver that maps fn over a src array into a dst array.
    // fn should take an Sk4px (4 src pixels) and return an Sk4px (4 dst pixels).
    template <typename Fn>
    static void MapSrc(int n, SkPMColor* dst, const SkPMColor* src, const Fn& fn) {
        SkASSERT(dst);
        SkASSERT(src);
        // This looks a bit odd, but it helps loop-invariant hoisting across different calls to fn.
        // Basically, we need to make sure we keep things inside a single loop.
        while (n > 0) {
            if (n >= 8) {
                Sk4px dst0 = fn(Load4(src+0)),
                      dst4 = fn(Load4(src+4));
                dst0.store4(dst+0);
                dst4.store4(dst+4);
                dst += 8; src += 8; n -= 8;
                continue;  // Keep our stride at 8 pixels as long as possible.
            }
            SkASSERT(n <= 7);
            if (n >= 4) {
                fn(Load4(src)).store4(dst);
                dst += 4; src += 4; n -= 4;
            }
            if (n >= 2) {
                fn(Load2(src)).store2(dst);
                dst += 2; src += 2; n -= 2;
            }
            if (n >= 1) {
                fn(Load1(src)).store1(dst);
            }
            break;
        }
    }

    // As above, but with dst4' = fn(dst4, src4).
    template <typename Fn>
    static void MapDstSrc(int n, SkPMColor* dst, const SkPMColor* src, const Fn& fn) {
        SkASSERT(dst);
        SkASSERT(src);
        while (n > 0) {
            if (n >= 8) {
                Sk4px dst0 = fn(Load4(dst+0), Load4(src+0)),
                      dst4 = fn(Load4(dst+4), Load4(src+4));
                dst0.store4(dst+0);
                dst4.store4(dst+4);
                dst += 8; src += 8; n -= 8;
                continue;  // Keep our stride at 8 pixels as long as possible.
            }
            SkASSERT(n <= 7);
            if (n >= 4) {
                fn(Load4(dst), Load4(src)).store4(dst);
                dst += 4; src += 4; n -= 4;
            }
            if (n >= 2) {
                fn(Load2(dst), Load2(src)).store2(dst);
                dst += 2; src += 2; n -= 2;
            }
            if (n >= 1) {
                fn(Load1(dst), Load1(src)).store1(dst);
            }
            break;
        }
    }

    // As above, but with dst4' = fn(dst4, alpha4).
    template <typename Fn>
    static void MapDstAlpha(int n, SkPMColor* dst, const SkAlpha* a, const Fn& fn) {
        SkASSERT(dst);
        SkASSERT(a);
        while (n > 0) {
            if (n >= 8) {
                Sk4px dst0 = fn(Load4(dst+0), Load4Alphas(a+0)),
                      dst4 = fn(Load4(dst+4), Load4Alphas(a+4));
                dst0.store4(dst+0);
                dst4.store4(dst+4);
                dst += 8; a += 8; n -= 8;
                continue;  // Keep our stride at 8 pixels as long as possible.
            }
            SkASSERT(n <= 7);
            if (n >= 4) {
                fn(Load4(dst), Load4Alphas(a)).store4(dst);
                dst += 4; a += 4; n -= 4;
            }
            if (n >= 2) {
                fn(Load2(dst), Load2Alphas(a)).store2(dst);
                dst += 2; a += 2; n -= 2;
            }
            if (n >= 1) {
                fn(Load1(dst), DupAlpha(*a)).store1(dst);
            }
            break;
        }
    }

    // As above, but with dst4' = fn(dst4, src4, alpha4).
    template <typename Fn>
    static void MapDstSrcAlpha(int n, SkPMColor* dst, const SkPMColor* src, const SkAlpha* a,
                               const Fn& fn) {
        SkASSERT(dst);
        SkASSERT(src);
        SkASSERT(a);
        while (n > 0) {
            if (n >= 8) {
                Sk4px dst0 = fn(Load4(dst+0), Load4(src+0), Load4Alphas(a+0)),
                      dst4 = fn(Load4(dst+4), Load4(src+4), Load4Alphas(a+4));
                dst0.store4(dst+0);
                dst4.store4(dst+4);
                dst += 8; src += 8; a += 8; n -= 8;
                continue;  // Keep our stride at 8 pixels as long as possible.
            }
            SkASSERT(n <= 7);
            if (n >= 4) {
                fn(Load4(dst), Load4(src), Load4Alphas(a)).store4(dst);
                dst += 4; src += 4; a += 4; n -= 4;
            }
            if (n >= 2) {
                fn(Load2(dst), Load2(src), Load2Alphas(a)).store2(dst);
                dst += 2; src += 2; a += 2; n -= 2;
            }
            if (n >= 1) {
                fn(Load1(dst), Load1(src), DupAlpha(*a)).store1(dst);
            }
            break;
        }
    }

private:
    typedef Sk16b INHERITED;
};

}  // namespace

#ifdef SKNX_NO_SIMD
    #include "../opts/Sk4px_none.h"
#else
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
        #include "../opts/Sk4px_SSE2.h"
    #elif defined(SK_ARM_HAS_NEON)
        #include "../opts/Sk4px_NEON.h"
    #else
        #include "../opts/Sk4px_none.h"
    #endif
#endif

#endif//Sk4px_DEFINED

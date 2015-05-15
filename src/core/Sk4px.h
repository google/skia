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

// 1, 2 or 4 SkPMColors, generally vectorized.
class Sk4px : public Sk16b {
public:
    Sk4px(SkAlpha a) : INHERITED(a) {} // Duplicate 16x: a    -> aaaa aaaa aaaa aaaa
    Sk4px(SkPMColor);                  // Duplicate 4x:  argb -> argb argb argb argb
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

        Sk4px div255TruncNarrow() const { return this->addNarrowHi(*this >> 8); }
        Sk4px div255RoundNarrow() const {
            return Sk4px::Wide(*this + Sk16h(128)).div255TruncNarrow();
        }

    private:
        typedef Sk16h INHERITED;
    };

    Wide widenLo() const;               // ARGB -> 0A 0R 0G 0B
    Wide widenHi() const;               // ARGB -> A0 R0 G0 B0
    Wide mulWiden(const Sk16b&) const;  // 8-bit x 8-bit -> 16-bit components.
    Wide mul255Widen() const {
        // TODO: x*255 = x*256-x, so something like this->widenHi() - this->widenLo()?
        return this->mulWiden(Sk16b(255));
    }

    // A generic driver that maps fn over a src array into a dst array.
    // fn should take an Sk4px (4 src pixels) and return an Sk4px (4 dst pixels).
    template <typename Fn>
    static void MapSrc(int count, SkPMColor* dst, const SkPMColor* src, Fn fn) {
        // This looks a bit odd, but it helps loop-invariant hoisting across different calls to fn.
        // Basically, we need to make sure we keep things inside a single loop.
        while (count > 0) {
            if (count >= 8) {
                Sk4px dst0 = fn(Load4(src+0)),
                      dst4 = fn(Load4(src+4));
                dst0.store4(dst+0);
                dst4.store4(dst+4);
                dst += 8; src += 8; count -= 8;
                continue;  // Keep our stride at 8 pixels as long as possible.
            }
            SkASSERT(count <= 7);
            if (count >= 4) {
                fn(Load4(src)).store4(dst);
                dst += 4; src += 4; count -= 4;
            }
            if (count >= 2) {
                fn(Load2(src)).store2(dst);
                dst += 2; src += 2; count -= 2;
            }
            if (count >= 1) {
                fn(Load1(src)).store1(dst);
            }
            break;
        }
    }

    // As above, but with dst4' = fn(dst4, src4).
    template <typename Fn>
    static void MapDstSrc(int count, SkPMColor* dst, const SkPMColor* src, Fn fn) {
        while (count > 0) {
            if (count >= 8) {
                Sk4px dst0 = fn(Load4(dst+0), Load4(src+0)),
                      dst4 = fn(Load4(dst+4), Load4(src+4));
                dst0.store4(dst+0);
                dst4.store4(dst+4);
                dst += 8; src += 8; count -= 8;
                continue;  // Keep our stride at 8 pixels as long as possible.
            }
            SkASSERT(count <= 7);
            if (count >= 4) {
                fn(Load4(dst), Load4(src)).store4(dst);
                dst += 4; src += 4; count -= 4;
            }
            if (count >= 2) {
                fn(Load2(dst), Load2(src)).store2(dst);
                dst += 2; src += 2; count -= 2;
            }
            if (count >= 1) {
                fn(Load1(dst), Load1(src)).store1(dst);
            }
            break;
        }
    }

    // As above, but with dst4' = fn(dst4, src4, alpha4).
    template <typename Fn>
    static void MapDstSrcAlpha(
            int count, SkPMColor* dst, const SkPMColor* src, const SkAlpha* a, Fn fn) {
        while (count > 0) {
            if (count >= 8) {
                Sk4px alpha0 = Load4Alphas(a+0),
                      alpha4 = Load4Alphas(a+4);
                Sk4px dst0 = fn(Load4(dst+0), Load4(src+0), alpha0),
                      dst4 = fn(Load4(dst+4), Load4(src+4), alpha4);
                dst0.store4(dst+0);
                dst4.store4(dst+4);
                dst += 8; src += 8; a += 8; count -= 8;
                continue;  // Keep our stride at 8 pixels as long as possible.
            }
            SkASSERT(count <= 7);
            if (count >= 4) {
                Sk4px alpha = Load4Alphas(a);
                fn(Load4(dst), Load4(src), alpha).store4(dst);
                dst += 4; src += 4; a += 4; count -= 4;
            }
            if (count >= 2) {
                Sk4px alpha = Load2Alphas(a);
                fn(Load2(dst), Load2(src), alpha).store2(dst);
                dst += 2; src += 2; a += 2; count -= 2;
            }
            if (count >= 1) {
                Sk4px alpha(*a);
                fn(Load1(dst), Load1(src), alpha).store1(dst);
            }
            break;
        }
    }

private:
    typedef Sk16b INHERITED;
};

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

/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4px_DEFINED
#define Sk4px_DEFINED

#include "include/core/SkColor.h"
#include "src/base/SkVx.h"
#include "src/core/SkColorData.h"

// 1, 2 or 4 SkPMColors, generally vectorized.
class Sk4px {
public:
    Sk4px(const skvx::byte16& v) : fV(v) {}

    static Sk4px DupPMColor(SkPMColor c) {
        skvx::uint4 splat(c);

        Sk4px v;
        memcpy((void*)&v, &splat, 16);
        return v;
    }

    // RGBA rgba XYZW xyzw -> AAAA aaaa WWWW wwww
    Sk4px alphas() const {
        static_assert(SK_A32_SHIFT == 24, "This method assumes little-endian.");
        return Sk4px(skvx::shuffle<3,3,3,3, 7,7,7,7, 11,11,11,11, 15,15,15,15>(fV));
    }
    Sk4px inv() const { return Sk4px(skvx::byte16(255) - fV); }

    // When loading or storing fewer than 4 SkPMColors, we use the low lanes.
    static Sk4px Load4(const SkPMColor px[4]) {
        Sk4px v;
        memcpy((void*)&v, px, 16);
        return v;
    }
    static Sk4px Load2(const SkPMColor px[2]) {
        Sk4px v;
        memcpy((void*)&v, px, 8);
        return v;
    }
    static Sk4px Load1(const SkPMColor px[1]) {
        Sk4px v;
        memcpy((void*)&v, px, 4);
        return v;
    }

    // Ditto for Alphas... Load2Alphas fills the low two lanes of Sk4px.
    // AaXx -> AAAA aaaa XXXX xxxx
    static Sk4px Load4Alphas(const SkAlpha alphas[4]) {
        skvx::byte4 a = skvx::byte4::Load(alphas);
        return Sk4px(skvx::shuffle<0,0,0,0, 1,1,1,1, 2,2,2,2, 3,3,3,3>(a));
    }
    // Aa   -> AAAA aaaa ???? ????
    static Sk4px Load2Alphas(const SkAlpha alphas[2]) {
        skvx::byte2 a = skvx::byte2::Load(alphas);
        return Sk4px(join(skvx::shuffle<0,0,0,0, 1,1,1,1>(a), skvx::byte8()));
    }

    void store4(SkPMColor px[4]) const { memcpy(px, this, 16); }
    void store2(SkPMColor px[2]) const { memcpy(px, this,  8); }
    void store1(SkPMColor px[1]) const { memcpy(px, this,  4); }

    // 1, 2, or 4 SkPMColors with 16-bit components.
    // This is most useful as the result of a multiply, e.g. from mulWiden().
    class Wide {
    public:
        Wide(const skvx::Vec<16, uint16_t>& v) : fV(v) {}

        // Rounds, i.e. (x+127) / 255.
        Sk4px div255() const { return Sk4px(skvx::div255(fV)); }

        Wide operator * (const Wide& o) const { return Wide(fV * o.fV); }
        Wide operator + (const Wide& o) const { return Wide(fV + o.fV); }
        Wide operator - (const Wide& o) const { return Wide(fV - o.fV); }
        Wide operator >> (int bits) const { return Wide(fV >> bits); }
        Wide operator << (int bits) const { return Wide(fV << bits); }

    private:
        skvx::Vec<16, uint16_t> fV;
    };

    // Widen 8-bit values to low 8-bits of 16-bit lanes.
    Wide widen() const { return Wide(skvx::cast<uint16_t>(fV)); }
    // 8-bit x 8-bit -> 16-bit components.
    Wide mulWiden(const skvx::byte16& o) const { return Wide(mull(fV, o)); }

    // The only 8-bit multiply we use is 8-bit x 8-bit -> 16-bit.  Might as well make it pithy.
    Wide operator * (const Sk4px& o) const { return this->mulWiden(o.fV); }

    Sk4px operator + (const Sk4px& o) const { return Sk4px(fV + o.fV); }
    Sk4px operator - (const Sk4px& o) const { return Sk4px(fV - o.fV); }
    Sk4px operator < (const Sk4px& o) const { return Sk4px(fV < o.fV); }
    Sk4px operator & (const Sk4px& o) const { return Sk4px(fV & o.fV); }
    Sk4px thenElse(const Sk4px& t, const Sk4px& e) const {
        return Sk4px(if_then_else(fV, t.fV, e.fV));
    }

    // Generally faster than (*this * o).div255().
    // May be incorrect by +-1, but is always exactly correct when *this or o is 0 or 255.
    Sk4px approxMulDiv255(const Sk4px& o) const {
        return Sk4px(approx_scale(fV, o.fV));
    }

    Sk4px saturatedAdd(const Sk4px& o) const {
        return Sk4px(saturated_add(fV, o.fV));
    }

    // A generic driver that maps fn over a src array into a dst array.
    // fn should take an Sk4px (4 src pixels) and return an Sk4px (4 dst pixels).
    template <typename Fn>
    [[maybe_unused]] static void MapSrc(int n, SkPMColor* dst, const SkPMColor* src, const Fn& fn) {
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
    [[maybe_unused]] static void MapDstSrc(int n, SkPMColor* dst, const SkPMColor* src,
                                           const Fn& fn) {
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
    [[maybe_unused]] static void MapDstAlpha(int n, SkPMColor* dst, const SkAlpha* a,
                                             const Fn& fn) {
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
                fn(Load1(dst), skvx::byte16(*a)).store1(dst);
            }
            break;
        }
    }

    // As above, but with dst4' = fn(dst4, src4, alpha4).
    template <typename Fn>
    [[maybe_unused]] static void MapDstSrcAlpha(int n, SkPMColor* dst, const SkPMColor* src,
                                                const SkAlpha* a, const Fn& fn) {
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
                fn(Load1(dst), Load1(src), skvx::byte16(*a)).store1(dst);
            }
            break;
        }
    }

private:
    Sk4px() = default;

    skvx::byte16 fV;
};

static_assert(sizeof(Sk4px) == sizeof(skvx::byte16));
static_assert(alignof(Sk4px) == alignof(skvx::byte16));

#endif // Sk4px_DEFINED

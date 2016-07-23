/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4pxXfermode_DEFINED
#define Sk4pxXfermode_DEFINED

#include "Sk4px.h"
#include "SkMSAN.h"
#include "SkNx.h"
#include "SkXfermode_proccoeff.h"

namespace {

// Most xfermodes can be done most efficiently 4 pixels at a time in 8 or 16-bit fixed point.
#define XFERMODE(Xfermode) \
    struct Xfermode { Sk4px operator()(const Sk4px&, const Sk4px&) const; }; \
    inline Sk4px Xfermode::operator()(const Sk4px& d, const Sk4px& s) const

XFERMODE(Clear) { return Sk4px::DupPMColor(0); }
XFERMODE(Src)   { return s; }
XFERMODE(Dst)   { return d; }
XFERMODE(SrcIn)   { return     s.approxMulDiv255(d.alphas()      ); }
XFERMODE(SrcOut)  { return     s.approxMulDiv255(d.alphas().inv()); }
XFERMODE(SrcOver) { return s + d.approxMulDiv255(s.alphas().inv()); }
XFERMODE(DstIn)   { return SrcIn  ()(s,d); }
XFERMODE(DstOut)  { return SrcOut ()(s,d); }
XFERMODE(DstOver) { return SrcOver()(s,d); }

// [ S * Da + (1 - Sa) * D]
XFERMODE(SrcATop) { return (s * d.alphas() + d * s.alphas().inv()).div255(); }
XFERMODE(DstATop) { return SrcATop()(s,d); }
//[ S * (1 - Da) + (1 - Sa) * D ]
XFERMODE(Xor) { return (s * d.alphas().inv() + d * s.alphas().inv()).div255(); }
// [S + D ]
XFERMODE(Plus) { return s.saturatedAdd(d); }
// [S * D ]
XFERMODE(Modulate) { return s.approxMulDiv255(d); }
// [S + D - S * D]
XFERMODE(Screen) {
    // Doing the math as S + (1-S)*D or S + (D - S*D) means the add and subtract can be done
    // in 8-bit space without overflow.  S + (1-S)*D is a touch faster because inv() is cheap.
    return s + d.approxMulDiv255(s.inv());
}
XFERMODE(Multiply) { return (s * d.alphas().inv() + d * s.alphas().inv() + s*d).div255(); }
// [ Sa + Da - Sa*Da, Sc + Dc - 2*min(Sc*Da, Dc*Sa) ]  (And notice Sa*Da == min(Sa*Da, Da*Sa).)
XFERMODE(Difference) {
    auto m = Sk4px::Wide::Min(s * d.alphas(), d * s.alphas()).div255();
    // There's no chance of underflow, and if we subtract m before adding s+d, no overflow.
    return (s - m) + (d - m.zeroAlphas());
}
// [ Sa + Da - Sa*Da, Sc + Dc - 2*Sc*Dc ]
XFERMODE(Exclusion) {
    auto p = s.approxMulDiv255(d);
    // There's no chance of underflow, and if we subtract p before adding src+dst, no overflow.
    return (s - p) + (d - p.zeroAlphas());
}

// We take care to use exact math for these next few modes where alphas
// and colors are calculated using significantly different math.  We need
// to preserve premul invariants, and exact math makes this easier.
//
// TODO: Some of these implementations might be able to be sped up a bit
// while maintaining exact math, but let's follow up with that.

XFERMODE(HardLight) {
    auto sa = s.alphas(),
         da = d.alphas();

    auto srcover = s + (d * sa.inv()).div255();

    auto isLite = ((sa-s) < s).widenLoHi();

    auto lite = sa*da - ((da-d)*(sa-s) << 1),
         dark = s*d << 1,
         both = s*da.inv() + d*sa.inv();

    auto alphas = srcover;
    auto colors = (both + isLite.thenElse(lite, dark)).div255();
    return alphas.zeroColors() + colors.zeroAlphas();
}
XFERMODE(Overlay) { return HardLight()(s,d); }

XFERMODE(Darken) {
    auto sa = s.alphas(),
         da = d.alphas();

    auto sda = (s*da).div255(),
         dsa = (d*sa).div255();

    auto srcover = s + (d * sa.inv()).div255(),
         dstover = d + (s * da.inv()).div255();
    auto alphas = srcover,
         colors = (sda < dsa).thenElse(srcover, dstover);
    return alphas.zeroColors() + colors.zeroAlphas();
}
XFERMODE(Lighten) {
    auto sa = s.alphas(),
         da = d.alphas();

    auto sda = (s*da).div255(),
         dsa = (d*sa).div255();

    auto srcover = s + (d * sa.inv()).div255(),
         dstover = d + (s * da.inv()).div255();
    auto alphas = srcover,
         colors = (dsa < sda).thenElse(srcover, dstover);
    return alphas.zeroColors() + colors.zeroAlphas();
}
#undef XFERMODE

// Some xfermodes use math like divide or sqrt that's best done in floats 1 pixel at a time.
#define XFERMODE(Xfermode) \
    struct Xfermode { Sk4f operator()(const Sk4f&, const Sk4f&) const; }; \
    inline Sk4f Xfermode::operator()(const Sk4f& d, const Sk4f& s) const

static inline Sk4f a_rgb(const Sk4f& a, const Sk4f& rgb) {
    static_assert(SK_A32_SHIFT == 24, "");
    return a * Sk4f(0,0,0,1) + rgb * Sk4f(1,1,1,0);
}
static inline Sk4f alphas(const Sk4f& f) {
    return f[SK_A32_SHIFT/8];
}

XFERMODE(ColorDodge) {
    auto sa = alphas(s),
         da = alphas(d),
         isa = Sk4f(1)-sa,
         ida = Sk4f(1)-da;

    auto srcover = s + d*isa,
         dstover = d + s*ida,
         otherwise = sa * Sk4f::Min(da, (d*sa)*(sa-s).invert()) + s*ida + d*isa;

    // Order matters here, preferring d==0 over s==sa.
    auto colors = (d == Sk4f(0)).thenElse(dstover,
                  (s ==      sa).thenElse(srcover,
                                          otherwise));
    return a_rgb(srcover, colors);
}
XFERMODE(ColorBurn) {
    auto sa = alphas(s),
         da = alphas(d),
         isa = Sk4f(1)-sa,
         ida = Sk4f(1)-da;

    auto srcover = s + d*isa,
         dstover = d + s*ida,
         otherwise = sa*(da-Sk4f::Min(da, (da-d)*sa*s.invert())) + s*ida + d*isa;

    // Order matters here, preferring d==da over s==0.
    auto colors = (d ==      da).thenElse(dstover,
                  (s == Sk4f(0)).thenElse(srcover,
                                          otherwise));
    return a_rgb(srcover, colors);
}
XFERMODE(SoftLight) {
    auto sa = alphas(s),
         da = alphas(d),
         isa = Sk4f(1)-sa,
         ida = Sk4f(1)-da;

    // Some common terms.
    auto m  = (da > Sk4f(0)).thenElse(d / da, Sk4f(0)),
         s2 = Sk4f(2)*s,
         m4 = Sk4f(4)*m;

    // The logic forks three ways:
    //    1. dark src?
    //    2. light src, dark dst?
    //    3. light src, light dst?
    auto darkSrc = d*(sa + (s2 - sa)*(Sk4f(1) - m)),        // Used in case 1.
         darkDst = (m4*m4 + m4)*(m - Sk4f(1)) + Sk4f(7)*m,  // Used in case 2.
         liteDst = m.sqrt() - m,                            // Used in case 3.
         liteSrc = d*sa + da*(s2-sa)*(Sk4f(4)*d <= da).thenElse(darkDst, liteDst); // Case 2 or 3?

    auto alpha  = s + d*isa;
    auto colors = s*ida + d*isa + (s2 <= sa).thenElse(darkSrc, liteSrc);           // Case 1 or 2/3?

    return a_rgb(alpha, colors);
}
#undef XFERMODE

// A reasonable fallback mode for doing AA is to simply apply the transfermode first,
// then linearly interpolate the AA.
template <typename Xfermode>
static Sk4px xfer_aa(const Sk4px& d, const Sk4px& s, const Sk4px& aa) {
    Sk4px bw = Xfermode()(d, s);
    return (bw * aa + d * aa.inv()).div255();
}

// For some transfermodes we specialize AA, either for correctness or performance.
#define XFERMODE_AA(Xfermode) \
    template <> Sk4px xfer_aa<Xfermode>(const Sk4px& d, const Sk4px& s, const Sk4px& aa)

// Plus' clamp needs to happen after AA.  skia:3852
XFERMODE_AA(Plus) {  // [ clamp( (1-AA)D + (AA)(S+D) ) == clamp(D + AA*S) ]
    return d.saturatedAdd(s.approxMulDiv255(aa));
}

#undef XFERMODE_AA

// Src and Clear modes are safe to use with unitialized dst buffers,
// even if the implementation branches based on bytes from dst (e.g. asserts in Debug mode).
// For those modes, just lie to MSAN that dst is always intialized.
template <typename Xfermode> static void mark_dst_initialized_if_safe(void*, void*) {}
template <> void mark_dst_initialized_if_safe<Src>(void* dst, void* end) {
    sk_msan_mark_initialized(dst, end, "Src doesn't read dst.");
}
template <> void mark_dst_initialized_if_safe<Clear>(void* dst, void* end) {
    sk_msan_mark_initialized(dst, end, "Clear doesn't read dst.");
}

template <typename Xfermode>
class Sk4pxXfermode : public SkProcCoeffXfermode {
public:
    Sk4pxXfermode(const ProcCoeff& rec, SkXfermode::Mode mode)
        : INHERITED(rec, mode) {}

    void xfer32(SkPMColor dst[], const SkPMColor src[], int n, const SkAlpha aa[]) const override {
        mark_dst_initialized_if_safe<Xfermode>(dst, dst+n);
        if (nullptr == aa) {
            Sk4px::MapDstSrc(n, dst, src, Xfermode());
        } else {
            Sk4px::MapDstSrcAlpha(n, dst, src, aa, xfer_aa<Xfermode>);
        }
    }

    void xfer16(uint16_t dst[], const SkPMColor src[], int n, const SkAlpha aa[]) const override {
        mark_dst_initialized_if_safe<Xfermode>(dst, dst+n);
        SkPMColor dst32[4];
        while (n >= 4) {
            dst32[0] = SkPixel16ToPixel32(dst[0]);
            dst32[1] = SkPixel16ToPixel32(dst[1]);
            dst32[2] = SkPixel16ToPixel32(dst[2]);
            dst32[3] = SkPixel16ToPixel32(dst[3]);

            this->xfer32(dst32, src, 4, aa);

            dst[0] = SkPixel32ToPixel16(dst32[0]);
            dst[1] = SkPixel32ToPixel16(dst32[1]);
            dst[2] = SkPixel32ToPixel16(dst32[2]);
            dst[3] = SkPixel32ToPixel16(dst32[3]);

            dst += 4;
            src += 4;
            aa  += aa ? 4 : 0;
            n -= 4;
        }
        while (n) {
            SkPMColor dst32 = SkPixel16ToPixel32(*dst);
            this->xfer32(&dst32, src, 1, aa);
            *dst = SkPixel32ToPixel16(dst32);

            dst += 1;
            src += 1;
            aa  += aa ? 1 : 0;
            n   -= 1;
        }
    }

private:
    typedef SkProcCoeffXfermode INHERITED;
};

template <typename Xfermode>
class Sk4fXfermode : public SkProcCoeffXfermode {
public:
    Sk4fXfermode(const ProcCoeff& rec, SkXfermode::Mode mode)
        : INHERITED(rec, mode) {}

    void xfer32(SkPMColor dst[], const SkPMColor src[], int n, const SkAlpha aa[]) const override {
        for (int i = 0; i < n; i++) {
            dst[i] = Xfer32_1(dst[i], src[i], aa ? aa+i : nullptr);
        }
    }

    void xfer16(uint16_t dst[], const SkPMColor src[], int n, const SkAlpha aa[]) const override {
        for (int i = 0; i < n; i++) {
            SkPMColor dst32 = SkPixel16ToPixel32(dst[i]);
            dst32 = Xfer32_1(dst32, src[i], aa ? aa+i : nullptr);
            dst[i] = SkPixel32ToPixel16(dst32);
        }
    }

private:
    static SkPMColor Xfer32_1(SkPMColor dst, const SkPMColor src, const SkAlpha* aa) {
        Sk4f d = Load(dst),
             s = Load(src),
             b = Xfermode()(d, s);
        if (aa) {
            Sk4f a = Sk4f(*aa) * Sk4f(1.0f/255);
            b = b*a + d*(Sk4f(1)-a);
        }
        return Round(b);
    }

    static Sk4f Load(SkPMColor c) {
        return SkNx_cast<float>(Sk4b::Load(&c)) * Sk4f(1.0f/255);
    }

    static SkPMColor Round(const Sk4f& f) {
        SkPMColor c;
        SkNx_cast<uint8_t>(f * Sk4f(255) + Sk4f(0.5f)).store(&c);
        return c;
    }

    typedef SkProcCoeffXfermode INHERITED;
};

} // namespace

namespace SK_OPTS_NS {

static SkXfermode* create_xfermode(const ProcCoeff& rec, SkXfermode::Mode mode) {
    switch (mode) {
#define CASE(Xfermode) \
    case SkXfermode::k##Xfermode##_Mode: return new Sk4pxXfermode<Xfermode>(rec, mode)
        CASE(Clear);
        CASE(Src);
        CASE(Dst);
        CASE(SrcOver);
        CASE(DstOver);
        CASE(SrcIn);
        CASE(DstIn);
        CASE(SrcOut);
        CASE(DstOut);
        CASE(SrcATop);
        CASE(DstATop);
        CASE(Xor);
        CASE(Plus);
        CASE(Modulate);
        CASE(Screen);
        CASE(Multiply);
        CASE(Difference);
        CASE(Exclusion);
        CASE(HardLight);
        CASE(Overlay);
        CASE(Darken);
        CASE(Lighten);
    #undef CASE

#define CASE(Xfermode) \
    case SkXfermode::k##Xfermode##_Mode: return new Sk4fXfermode<Xfermode>(rec, mode)
        CASE(ColorDodge);
        CASE(ColorBurn);
        CASE(SoftLight);
    #undef CASE

        default: break;
    }
    return nullptr;
}

} // namespace SK_OPTS_NS

#endif//Sk4pxXfermode_DEFINED

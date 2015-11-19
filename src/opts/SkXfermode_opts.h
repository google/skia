/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4pxXfermode_DEFINED
#define Sk4pxXfermode_DEFINED

#include "Sk4px.h"
#include "SkNx.h"
#include "SkXfermode_proccoeff.h"

namespace {

// Most xfermodes can be done most efficiently 4 pixels at a time in 8 or 16-bit fixed point.
#define XFERMODE(Name) static Sk4px SK_VECTORCALL Name(Sk4px s, Sk4px d)

XFERMODE(Clear) { return Sk4px::DupPMColor(0); }
XFERMODE(Src)   { return s; }
XFERMODE(Dst)   { return d; }
XFERMODE(SrcIn)   { return     s.approxMulDiv255(d.alphas()      ); }
XFERMODE(SrcOut)  { return     s.approxMulDiv255(d.alphas().inv()); }
XFERMODE(SrcOver) { return s + d.approxMulDiv255(s.alphas().inv()); }
XFERMODE(DstIn)   { return SrcIn  (d,s); }
XFERMODE(DstOut)  { return SrcOut (d,s); }
XFERMODE(DstOver) { return SrcOver(d,s); }

// [ S * Da + (1 - Sa) * D]
XFERMODE(SrcATop) { return (s * d.alphas() + d * s.alphas().inv()).div255(); }
XFERMODE(DstATop) { return SrcATop(d,s); }
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
XFERMODE(Overlay) { return HardLight(d,s); }

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
#define XFERMODE(Name) static Sk4f SK_VECTORCALL Name(Sk4f d, Sk4f s)

static inline Sk4f a_rgb(const Sk4f& a, const Sk4f& rgb) {
    static_assert(SK_A32_SHIFT == 24, "");
    return a * Sk4f(0,0,0,1) + rgb * Sk4f(1,1,1,0);
}
static inline Sk4f alphas(const Sk4f& f) {
    return SkNx_dup<SK_A32_SHIFT/8>(f);
}

XFERMODE(ColorDodge) {
    auto sa = alphas(s),
         da = alphas(d),
         isa = Sk4f(1)-sa,
         ida = Sk4f(1)-da;

    auto srcover = s + d*isa,
         dstover = d + s*ida,
         otherwise = sa * Sk4f::Min(da, (d*sa)*(sa-s).approxInvert()) + s*ida + d*isa;

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
         otherwise = sa*(da-Sk4f::Min(da, (da-d)*sa*s.approxInvert())) + s*ida + d*isa;

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
template <Sk4px (SK_VECTORCALL *Mode)(Sk4px, Sk4px)>
static Sk4px SK_VECTORCALL xfer_aa(Sk4px s, Sk4px d, Sk4px aa) {
    Sk4px bw = Mode(s, d);
    return (bw * aa + d * aa.inv()).div255();
}

// For some transfermodes we specialize AA, either for correctness or performance.
#define XFERMODE_AA(Name) \
    template <> Sk4px SK_VECTORCALL xfer_aa<Name>(Sk4px s, Sk4px d, Sk4px aa)

// Plus' clamp needs to happen after AA.  skia:3852
XFERMODE_AA(Plus) {  // [ clamp( (1-AA)D + (AA)(S+D) ) == clamp(D + AA*S) ]
    return d.saturatedAdd(s.approxMulDiv255(aa));
}

#undef XFERMODE_AA

class Sk4pxXfermode : public SkProcCoeffXfermode {
public:
    typedef Sk4px (SK_VECTORCALL *Proc4)(Sk4px, Sk4px);
    typedef Sk4px (SK_VECTORCALL *AAProc4)(Sk4px, Sk4px, Sk4px);

    Sk4pxXfermode(const ProcCoeff& rec, SkXfermode::Mode mode, Proc4 proc4, AAProc4 aaproc4)
        : INHERITED(rec, mode)
        , fProc4(proc4)
        , fAAProc4(aaproc4) {}

    void xfer32(SkPMColor dst[], const SkPMColor src[], int n, const SkAlpha aa[]) const override {
        if (nullptr == aa) {
            Sk4px::MapDstSrc(n, dst, src, [&](const Sk4px& dst4, const Sk4px& src4) {
                return fProc4(src4, dst4);
            });
        } else {
            Sk4px::MapDstSrcAlpha(n, dst, src, aa,
                    [&](const Sk4px& dst4, const Sk4px& src4, const Sk4px& alpha) {
                return fAAProc4(src4, dst4, alpha);
            });
        }
    }

    void xfer16(uint16_t dst[], const SkPMColor src[], int n, const SkAlpha aa[]) const override {
        if (nullptr == aa) {
            Sk4px::MapDstSrc(n, dst, src, [&](const Sk4px& dst4, const Sk4px& src4) {
                return fProc4(src4, dst4);
            });
        } else {
            Sk4px::MapDstSrcAlpha(n, dst, src, aa,
                    [&](const Sk4px& dst4, const Sk4px& src4, const Sk4px& alpha) {
                return fAAProc4(src4, dst4, alpha);
            });
        }
    }

private:
    Proc4 fProc4;
    AAProc4 fAAProc4;
    typedef SkProcCoeffXfermode INHERITED;
};

class Sk4fXfermode : public SkProcCoeffXfermode {
public:
    typedef Sk4f (SK_VECTORCALL *ProcF)(Sk4f, Sk4f);
    Sk4fXfermode(const ProcCoeff& rec, SkXfermode::Mode mode, ProcF procf)
        : INHERITED(rec, mode)
        , fProcF(procf) {}

    void xfer32(SkPMColor dst[], const SkPMColor src[], int n, const SkAlpha aa[]) const override {
        for (int i = 0; i < n; i++) {
            dst[i] = aa ? this->xfer32(dst[i], src[i], aa[i])
                        : this->xfer32(dst[i], src[i]);
        }
    }

    void xfer16(uint16_t dst[], const SkPMColor src[], int n, const SkAlpha aa[]) const override {
        for (int i = 0; i < n; i++) {
            SkPMColor dst32 = SkPixel16ToPixel32(dst[i]);
            dst32 = aa ? this->xfer32(dst32, src[i], aa[i])
                       : this->xfer32(dst32, src[i]);
            dst[i] = SkPixel32ToPixel16(dst32);
        }
    }

private:
    static Sk4f Load(SkPMColor c) {
        return Sk4f::FromBytes((uint8_t*)&c) * Sk4f(1.0f/255);
    }
    static SkPMColor Round(const Sk4f& f) {
        SkPMColor c;
        (f * Sk4f(255) + Sk4f(0.5f)).toBytes((uint8_t*)&c);
        return c;
    }
    inline SkPMColor xfer32(SkPMColor dst, SkPMColor src) const {
        return Round(fProcF(Load(dst), Load(src)));
    }

    inline SkPMColor xfer32(SkPMColor dst, SkPMColor src, SkAlpha aa) const {
        Sk4f s(Load(src)),
             d(Load(dst)),
             b(fProcF(d,s));
        // We do aa in full float precision before going back down to bytes, because we can!
        Sk4f a = Sk4f(aa) * Sk4f(1.0f/255);
        b = b*a + d*(Sk4f(1)-a);
        return Round(b);
    }

    ProcF fProcF;
    typedef SkProcCoeffXfermode INHERITED;
};

} // namespace

namespace SK_OPTS_NS {

static SkXfermode* create_xfermode(const ProcCoeff& rec, SkXfermode::Mode mode) {
    switch (mode) {
#define CASE(Mode) \
    case SkXfermode::k##Mode##_Mode: return new Sk4pxXfermode(rec, mode, &Mode, &xfer_aa<Mode>)
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

#define CASE(Mode) \
    case SkXfermode::k##Mode##_Mode: return new Sk4fXfermode(rec, mode, &Mode)
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

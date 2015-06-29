/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4pxXfermode_DEFINED
#define Sk4pxXfermode_DEFINED

#include "Sk4px.h"
#include "SkPMFloat.h"

// This file is possibly included into multiple .cpp files.
// Each gets its own independent instantiation by wrapping in an anonymous namespace.
namespace {

// Most xfermodes can be done most efficiently 4 pixels at a time in 8 or 16-bit fixed point.
#define XFERMODE(Name)                                                    \
    struct Name {                                                         \
        static Sk4px Xfer(const Sk4px&, const Sk4px&);                    \
        static const SkXfermode::Mode kMode = SkXfermode::k##Name##_Mode; \
    };                                                                    \
    inline Sk4px Name::Xfer(const Sk4px& s, const Sk4px& d)

XFERMODE(Clear) { return Sk4px::DupPMColor(0); }
XFERMODE(Src)   { return s; }
XFERMODE(Dst)   { return d; }
XFERMODE(SrcIn)   { return     s.approxMulDiv255(d.alphas()      ); }
XFERMODE(SrcOut)  { return     s.approxMulDiv255(d.alphas().inv()); }
XFERMODE(SrcOver) { return s + d.approxMulDiv255(s.alphas().inv()); }
XFERMODE(DstIn)   { return SrcIn  ::Xfer(d,s); }
XFERMODE(DstOut)  { return SrcOut ::Xfer(d,s); }
XFERMODE(DstOver) { return SrcOver::Xfer(d,s); }

// [ S * Da + (1 - Sa) * D]
XFERMODE(SrcATop) { return (s * d.alphas() + d * s.alphas().inv()).div255(); }
XFERMODE(DstATop) { return SrcATop::Xfer(d,s); }
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

XFERMODE(HardLight) {
    auto alphas = SrcOver::Xfer(s,d);

    auto sa = s.alphas(),
         da = d.alphas();

    auto isDark = s < (sa-s);

    auto dark = s*d << 1,
         lite = sa*da - ((da-d)*(sa-s) << 1),
         both = s*da.inv() + d*sa.inv();

    // TODO: do isDark in 16-bit so we only have to div255() once.
    auto colors = isDark.thenElse((dark + both).div255(),
                                  (lite + both).div255());
    return alphas.zeroColors() + colors.zeroAlphas();
}
XFERMODE(Overlay) { return HardLight::Xfer(d,s); }

XFERMODE(Darken) {
    auto sda = s.approxMulDiv255(d.alphas()),
         dsa = d.approxMulDiv255(s.alphas());
    auto srcover = s + (d - dsa),
         dstover = d + (s - sda);
    auto alphas = srcover,
         colors = (sda < dsa).thenElse(srcover, dstover);
    return alphas.zeroColors() + colors.zeroAlphas();
}
XFERMODE(Lighten) {
    auto sda = s.approxMulDiv255(d.alphas()),
         dsa = d.approxMulDiv255(s.alphas());
    auto srcover = s + (d - dsa),
         dstover = d + (s - sda);
    auto alphas = srcover,
         colors = (sda < dsa).thenElse(dstover, srcover);
    return alphas.zeroColors() + colors.zeroAlphas();
}
#undef XFERMODE

// Some xfermodes use math like divide or sqrt that's best done in floats 1 pixel at a time.
#define XFERMODE(Name) \
    struct Name {                                                         \
        static SkPMFloat Xfer(const SkPMFloat&, const SkPMFloat&);        \
        static const SkXfermode::Mode kMode = SkXfermode::k##Name##_Mode; \
    };                                                                    \
    inline SkPMFloat Name::Xfer(const SkPMFloat& s, const SkPMFloat& d)

XFERMODE(ColorDodge) {
    auto sa = s.alphas(),
         da = d.alphas(),
         isa = Sk4f(1)-sa,
         ida = Sk4f(1)-da;

    auto srcover = s + d*isa,
         dstover = d + s*ida,
         otherwise = sa * Sk4f::Min(da, (d*sa)*(sa-s).approxInvert()) + s*ida + d*isa;

    // Order matters here, preferring d==0 over s==sa.
    auto colors = (d == Sk4f(0)).thenElse(dstover,
                  (s ==      sa).thenElse(srcover,
                                          otherwise));
    return srcover * SkPMFloat(1,0,0,0) + colors * SkPMFloat(0,1,1,1);
}
XFERMODE(ColorBurn) {
    auto sa = s.alphas(),
         da = d.alphas(),
         isa = Sk4f(1)-sa,
         ida = Sk4f(1)-da;

    auto srcover = s + d*isa,
         dstover = d + s*ida,
         otherwise = sa*(da-Sk4f::Min(da, (da-d)*sa*s.approxInvert())) + s*ida + d*isa;

    // Order matters here, preferring d==da over s==0.
    auto colors = (d ==      da).thenElse(dstover,
                  (s == Sk4f(0)).thenElse(srcover,
                                          otherwise));
    return srcover * SkPMFloat(1,0,0,0) + colors * SkPMFloat(0,1,1,1);
}
#undef XFERMODE

// A reasonable fallback mode for doing AA is to simply apply the transfermode first,
// then linearly interpolate the AA.
template <typename Mode>
static Sk4px xfer_aa(const Sk4px& s, const Sk4px& d, const Sk4px& aa) {
    Sk4px bw = Mode::Xfer(s, d);
    return (bw * aa + d * aa.inv()).div255();
}

// For some transfermodes we specialize AA, either for correctness or performance.
#define XFERMODE_AA(Name) \
    template <> Sk4px xfer_aa<Name>(const Sk4px& s, const Sk4px& d, const Sk4px& aa)

// Plus' clamp needs to happen after AA.  skia:3852
XFERMODE_AA(Plus) {  // [ clamp( (1-AA)D + (AA)(S+D) ) == clamp(D + AA*S) ]
    return d.saturatedAdd(s.approxMulDiv255(aa));
}

#undef XFERMODE_AA

template <typename ProcType>
class SkT4pxXfermode : public SkProcCoeffXfermode {
public:
    static SkProcCoeffXfermode* Create(const ProcCoeff& rec) {
        return SkNEW_ARGS(SkT4pxXfermode, (rec));
    }

    void xfer32(SkPMColor dst[], const SkPMColor src[], int n, const SkAlpha aa[]) const override {
        if (NULL == aa) {
            Sk4px::MapDstSrc(n, dst, src, [&](const Sk4px& dst4, const Sk4px& src4) {
                return ProcType::Xfer(src4, dst4);
            });
        } else {
            Sk4px::MapDstSrcAlpha(n, dst, src, aa,
                    [&](const Sk4px& dst4, const Sk4px& src4, const Sk4px& alpha) {
                return xfer_aa<ProcType>(src4, dst4, alpha);
            });
        }
    }

private:
    SkT4pxXfermode(const ProcCoeff& rec) : INHERITED(rec, ProcType::kMode) {}

    typedef SkProcCoeffXfermode INHERITED;
};

template <typename ProcType>
class SkTPMFloatXfermode : public SkProcCoeffXfermode {
public:
    static SkProcCoeffXfermode* Create(const ProcCoeff& rec) {
        return SkNEW_ARGS(SkTPMFloatXfermode, (rec));
    }

    void xfer32(SkPMColor dst[], const SkPMColor src[], int n, const SkAlpha aa[]) const override {
        for (int i = 0; i < n; i++) {
            SkPMFloat s(src[i]),
                      d(dst[i]),
                      b(ProcType::Xfer(s,d));
            if (aa) {
                // We do aa in full float precision before going back down to bytes, because we can!
                SkPMFloat a = Sk4f(aa[i]) * Sk4f(1.0f/255);
                b = b*a + d*(Sk4f(1)-a);
            }
            dst[i] = b.round();
        }
    }

private:
    SkTPMFloatXfermode(const ProcCoeff& rec) : INHERITED(rec, ProcType::kMode) {}

    typedef SkProcCoeffXfermode INHERITED;
};

static SkProcCoeffXfermode* SkCreate4pxXfermode(const ProcCoeff& rec, SkXfermode::Mode mode) {
#if !defined(SK_CPU_ARM32) || defined(SK_ARM_HAS_NEON)
    switch (mode) {
        case SkXfermode::kClear_Mode:      return SkT4pxXfermode<Clear>::Create(rec);
        case SkXfermode::kSrc_Mode:        return SkT4pxXfermode<Src>::Create(rec);
        case SkXfermode::kDst_Mode:        return SkT4pxXfermode<Dst>::Create(rec);
        case SkXfermode::kSrcOver_Mode:    return SkT4pxXfermode<SrcOver>::Create(rec);
        case SkXfermode::kDstOver_Mode:    return SkT4pxXfermode<DstOver>::Create(rec);
        case SkXfermode::kSrcIn_Mode:      return SkT4pxXfermode<SrcIn>::Create(rec);
        case SkXfermode::kDstIn_Mode:      return SkT4pxXfermode<DstIn>::Create(rec);
        case SkXfermode::kSrcOut_Mode:     return SkT4pxXfermode<SrcOut>::Create(rec);
        case SkXfermode::kDstOut_Mode:     return SkT4pxXfermode<DstOut>::Create(rec);
        case SkXfermode::kSrcATop_Mode:    return SkT4pxXfermode<SrcATop>::Create(rec);
        case SkXfermode::kDstATop_Mode:    return SkT4pxXfermode<DstATop>::Create(rec);
        case SkXfermode::kXor_Mode:        return SkT4pxXfermode<Xor>::Create(rec);
        case SkXfermode::kPlus_Mode:       return SkT4pxXfermode<Plus>::Create(rec);
        case SkXfermode::kModulate_Mode:   return SkT4pxXfermode<Modulate>::Create(rec);
        case SkXfermode::kScreen_Mode:     return SkT4pxXfermode<Screen>::Create(rec);
        case SkXfermode::kMultiply_Mode:   return SkT4pxXfermode<Multiply>::Create(rec);
        case SkXfermode::kDifference_Mode: return SkT4pxXfermode<Difference>::Create(rec);
        case SkXfermode::kExclusion_Mode:  return SkT4pxXfermode<Exclusion>::Create(rec);
#if !defined(SK_SUPPORT_LEGACY_XFERMODES)  // For staging in Chrome (layout tests).
        case SkXfermode::kHardLight_Mode:  return SkT4pxXfermode<HardLight>::Create(rec);
        case SkXfermode::kOverlay_Mode:    return SkT4pxXfermode<Overlay>::Create(rec);
        case SkXfermode::kDarken_Mode:     return SkT4pxXfermode<Darken>::Create(rec);
        case SkXfermode::kLighten_Mode:    return SkT4pxXfermode<Lighten>::Create(rec);

        case SkXfermode::kColorDodge_Mode: return SkTPMFloatXfermode<ColorDodge>::Create(rec);
        case SkXfermode::kColorBurn_Mode:  return SkTPMFloatXfermode<ColorBurn>::Create(rec);
#endif
        default: break;
    }
#endif
    return nullptr;
}

} // namespace

#endif//Sk4pxXfermode_DEFINED

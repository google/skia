/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4pxXfermode_DEFINED
#define Sk4pxXfermode_DEFINED

#include "Sk4px.h"

// This file is possibly included into multiple .cpp files.
// Each gets its own independent instantiation by wrapping in an anonymous namespace.
namespace {

#define XFERMODE(Name)                                                    \
    struct Name {                                                         \
        static Sk4px Xfer(const Sk4px&, const Sk4px&);                    \
        static const SkXfermode::Mode kMode = SkXfermode::k##Name##_Mode; \
    };                                                                    \
    inline Sk4px Name::Xfer(const Sk4px& s, const Sk4px& d)

XFERMODE(Clear) { return Sk4px((SkPMColor)0); }
XFERMODE(Src)   { return s; }
XFERMODE(Dst)   { return d; }
XFERMODE(SrcIn)   { return     s.fastMulDiv255Round(d.alphas()      ); }
XFERMODE(SrcOut)  { return     s.fastMulDiv255Round(d.alphas().inv()); }
XFERMODE(SrcOver) { return s + d.fastMulDiv255Round(s.alphas().inv()); }
XFERMODE(DstIn)   { return SrcIn  ::Xfer(d,s); }
XFERMODE(DstOut)  { return SrcOut ::Xfer(d,s); }
XFERMODE(DstOver) { return SrcOver::Xfer(d,s); }

// [ S * Da + (1 - Sa) * D]
XFERMODE(SrcATop) {
    return Sk4px::Wide(s.mulWiden(d.alphas()) + d.mulWiden(s.alphas().inv()))
        .div255RoundNarrow();
}
XFERMODE(DstATop) { return SrcATop::Xfer(d,s); }
//[ S * (1 - Da) + (1 - Sa) * D ]
XFERMODE(Xor) {
    return Sk4px::Wide(s.mulWiden(d.alphas().inv()) + d.mulWiden(s.alphas().inv()))
        .div255RoundNarrow();
}
// [S + D ]
XFERMODE(Plus) { return s.saturatedAdd(d); }
// [S * D ]
XFERMODE(Modulate) { return s.fastMulDiv255Round(d); }
// [S + D - S * D]
XFERMODE(Screen) {
    // Doing the math as S + (1-S)*D or S + (D - S*D) means the add and subtract can be done
    // in 8-bit space without overflow.  S + (1-S)*D is a touch faster because inv() is cheap.
    return s + d.fastMulDiv255Round(s.inv());
}
XFERMODE(Multiply) {
    return Sk4px::Wide(s.mulWiden(d.alphas().inv()) +
            d.mulWiden(s.alphas().inv()) +
            s.mulWiden(d))
        .div255RoundNarrow();
}
// [ Sa + Da - Sa*Da, Sc + Dc - 2*min(Sc*Da, Dc*Sa) ]  (And notice Sa*Da == min(Sa*Da, Da*Sa).)
XFERMODE(Difference) {
    auto m = Sk4px::Wide(Sk16h::Min(s.mulWiden(d.alphas()), d.mulWiden(s.alphas())))
        .div255RoundNarrow();
    // There's no chance of underflow, and if we subtract m before adding s+d, no overflow.
    return (s - m) + (d - m.zeroAlphas());
}
// [ Sa + Da - Sa*Da, Sc + Dc - 2*Sc*Dc ]
XFERMODE(Exclusion) {
    auto p = s.fastMulDiv255Round(d);
    // There's no chance of underflow, and if we subtract p before adding src+dst, no overflow.
    return (s - p) + (d - p.zeroAlphas());
}

#undef XFERMODE

// A reasonable fallback mode for doing AA is to simply apply the transfermode first,
// then linearly interpolate the AA.
template <typename Mode>
static Sk4px xfer_aa(const Sk4px& s, const Sk4px& d, const Sk16b& aa) {
    Sk4px noAA = Mode::Xfer(s, d);
    return Sk4px::Wide(noAA.mulWiden(aa) + d.mulWiden(Sk4px(aa).inv()))
        .div255RoundNarrow();
}

// For some transfermodes we specialize AA, either for correctness or performance.
#ifndef SK_NO_SPECIALIZED_AA_XFERMODES
    #define XFERMODE_AA(Name) \
        template <> Sk4px xfer_aa<Name>(const Sk4px& s, const Sk4px& d, const Sk16b& aa)

    // Plus' clamp needs to happen after AA.  skia:3852
    XFERMODE_AA(Plus) {  // [ clamp( (1-AA)D + (AA)(S+D) ) == clamp(D + AA*S) ]
        return d.saturatedAdd(s.fastMulDiv255Round(aa));
    }

    #undef XFERMODE_AA
#endif

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
                    [&](const Sk4px& dst4, const Sk4px& src4, const Sk16b& alpha) {
                return xfer_aa<ProcType>(src4, dst4, alpha);
            });
        }
    }

private:
    SkT4pxXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, ProcType::kMode) {}

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
        default: break;
    }
#endif
    return nullptr;
}

} // namespace

#endif//Sk4pxXfermode_DEFINED

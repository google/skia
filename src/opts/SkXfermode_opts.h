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
#include "SkXfermodePriv.h"

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
class Sk4pxXfermode : public SkXfermode {
public:
    Sk4pxXfermode() {}

    void xfer32(SkPMColor dst[], const SkPMColor src[], int n, const SkAlpha aa[]) const override {
        mark_dst_initialized_if_safe<Xfermode>(dst, dst+n);
        if (nullptr == aa) {
            Sk4px::MapDstSrc(n, dst, src, Xfermode());
        } else {
            Sk4px::MapDstSrcAlpha(n, dst, src, aa, xfer_aa<Xfermode>);
        }
    }
};

} // namespace

namespace SK_OPTS_NS {

/*not static*/ inline SkXfermode* create_xfermode(SkBlendMode mode) {
    switch (mode) {
#define CASE(Xfermode) \
    case SkBlendMode::k##Xfermode: return new Sk4pxXfermode<Xfermode>()
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

        default: break;
    }
    return nullptr;
}

} // namespace SK_OPTS_NS

#endif//Sk4pxXfermode_DEFINED

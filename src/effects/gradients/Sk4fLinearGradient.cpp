/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk4fLinearGradient.h"
#include "SkUtils.h"
#include "SkXfermode.h"

namespace {

template<typename DstType, SkColorProfileType, ApplyPremul>
void fill(const Sk4f& c, DstType* dst, int n);

template<>
void fill<SkPM4f, kLinear_SkColorProfileType, ApplyPremul::False>
         (const Sk4f& c, SkPM4f* dst, int n) {
    while (n > 0) {
        c.store(dst++);
        n--;
    }
}

template<>
void fill<SkPM4f, kLinear_SkColorProfileType, ApplyPremul::True>
         (const Sk4f& c, SkPM4f* dst, int n) {
    fill<SkPM4f, kLinear_SkColorProfileType, ApplyPremul::False>(premul_4f(c), dst, n);
}

template<>
void fill<SkPMColor, kLinear_SkColorProfileType, ApplyPremul::False>
         (const Sk4f& c, SkPMColor* dst, int n) {
    sk_memset32(dst, trunc_from_4f_255<ApplyPremul::False>(c), n);
}

template<>
void fill<SkPMColor, kLinear_SkColorProfileType, ApplyPremul::True>
         (const Sk4f& c, SkPMColor* dst, int n) {
    sk_memset32(dst, trunc_from_4f_255<ApplyPremul::True>(c), n);
}

template<>
void fill<SkPMColor, kSRGB_SkColorProfileType, ApplyPremul::False>
         (const Sk4f& c, SkPMColor* dst, int n) {
    // FIXME: this assumes opaque colors.  Handle unpremultiplication.
    sk_memset32(dst, Sk4f_toS32(c), n);
}

template<>
void fill<SkPMColor, kSRGB_SkColorProfileType, ApplyPremul::True>
         (const Sk4f& c, SkPMColor* dst, int n) {
    sk_memset32(dst, Sk4f_toS32(premul_4f(c)), n);
}

template<>
void fill<uint64_t, kLinear_SkColorProfileType, ApplyPremul::False>
         (const Sk4f& c, uint64_t* dst, int n) {
    sk_memset64(dst, SkFloatToHalf_01(c), n);
}

template<>
void fill<uint64_t, kLinear_SkColorProfileType, ApplyPremul::True>
         (const Sk4f& c, uint64_t* dst, int n) {
    sk_memset64(dst, SkFloatToHalf_01(premul_4f(c)), n);
}

template<typename DstType, SkColorProfileType profile, ApplyPremul premul>
void ramp(const Sk4f& c, const Sk4f& dc, DstType* dst, int n) {
    SkASSERT(n > 0);

    const Sk4f dc2 = dc + dc;
    const Sk4f dc4 = dc2 + dc2;

    Sk4f c0 = c ;
    Sk4f c1 = c + dc;
    Sk4f c2 = c0 + dc2;
    Sk4f c3 = c1 + dc2;

    while (n >= 4) {
        store4x<DstType, profile, premul>(c0, c1, c2, c3, dst);
        dst += 4;

        c0 = c0 + dc4;
        c1 = c1 + dc4;
        c2 = c2 + dc4;
        c3 = c3 + dc4;
        n -= 4;
    }
    if (n & 2) {
        store<DstType, profile, premul>(c0, dst++);
        store<DstType, profile, premul>(c1, dst++);
        c0 = c0 + dc2;
    }
    if (n & 1) {
        store<DstType, profile, premul>(c0, dst);
    }
}

template<SkShader::TileMode>
SkScalar pinFx(SkScalar);

template<>
SkScalar pinFx<SkShader::kClamp_TileMode>(SkScalar fx) {
    return fx;
}

template<>
SkScalar pinFx<SkShader::kRepeat_TileMode>(SkScalar fx) {
    const SkScalar f = SkScalarFraction(fx);
    return f < 0 ? f + 1 : f;
}

template<>
SkScalar pinFx<SkShader::kMirror_TileMode>(SkScalar fx) {
    const SkScalar f = SkScalarMod(fx, 2.0f);
    return f < 0 ? f + 2 : f;
}

// true when x is in [k1,k2)
bool in_range(SkScalar x, SkScalar k1, SkScalar k2) {
    SkASSERT(k1 != k2);
    return (k1 < k2)
        ? (x >= k1 && x < k2)
        : (x >= k2 && x < k1);
}

} // anonymous namespace

SkLinearGradient::
LinearGradient4fContext::LinearGradient4fContext(const SkLinearGradient& shader,
                                                 const ContextRec& rec)
    : INHERITED(shader, rec) {

    // Our fast path expects interval points to be monotonically increasing in x.
    const bool reverseIntervals = this->isFast() && fDstToPos.getScaleX() < 0;
    this->buildIntervals(shader, rec, reverseIntervals);

    SkASSERT(fIntervals.count() > 0);
    fCachedInterval = fIntervals.begin();
}

const SkGradientShaderBase::GradientShaderBase4fContext::Interval*
SkLinearGradient::LinearGradient4fContext::findInterval(SkScalar fx) const {
    SkASSERT(in_range(fx, fIntervals.front().fP0, fIntervals.back().fP1));

    if (1) {
        // Linear search, using the last scanline interval as a starting point.
        SkASSERT(fCachedInterval >= fIntervals.begin());
        SkASSERT(fCachedInterval < fIntervals.end());
        const int search_dir = fDstToPos.getScaleX() >= 0 ? 1 : -1;
        while (!in_range(fx, fCachedInterval->fP0, fCachedInterval->fP1)) {
            fCachedInterval += search_dir;
            if (fCachedInterval >= fIntervals.end()) {
                fCachedInterval = fIntervals.begin();
            } else if (fCachedInterval < fIntervals.begin()) {
                fCachedInterval = fIntervals.end() - 1;
            }
        }
        return fCachedInterval;
    } else {
        // Binary search.  Seems less effective than linear + caching.
        const Interval* i0 = fIntervals.begin();
        const Interval* i1 = fIntervals.end() - 1;

        while (i0 != i1) {
            SkASSERT(i0 < i1);
            SkASSERT(in_range(fx, i0->fP0, i1->fP1));

            const Interval* i = i0 + ((i1 - i0) >> 1);

            if (in_range(fx, i0->fP0, i->fP1)) {
                i1 = i;
            } else {
                SkASSERT(in_range(fx, i->fP1, i1->fP1));
                i0 = i + 1;
            }
        }

        SkASSERT(in_range(fx, i0->fP0, i0->fP1));
        return i0;
    }
}

void SkLinearGradient::
LinearGradient4fContext::shadeSpan(int x, int y, SkPMColor dst[], int count) {
    if (!this->isFast()) {
        this->INHERITED::shadeSpan(x, y, dst, count);
        return;
    }

    // TODO: plumb dithering
    SkASSERT(count > 0);
    if (fColorsArePremul) {
        this->shadePremulSpan<SkPMColor,
                              kLinear_SkColorProfileType,
                              ApplyPremul::False>(x, y, dst, count);
    } else {
        this->shadePremulSpan<SkPMColor,
                              kLinear_SkColorProfileType,
                              ApplyPremul::True>(x, y, dst, count);
    }
}

void SkLinearGradient::
LinearGradient4fContext::shadeSpan4f(int x, int y, SkPM4f dst[], int count) {
    if (!this->isFast()) {
        this->INHERITED::shadeSpan4f(x, y, dst, count);
        return;
    }

    // TONOTDO: plumb dithering
    SkASSERT(count > 0);
    if (fColorsArePremul) {
        this->shadePremulSpan<SkPM4f,
                              kLinear_SkColorProfileType,
                              ApplyPremul::False>(x, y, dst, count);
    } else {
        this->shadePremulSpan<SkPM4f,
                              kLinear_SkColorProfileType,
                              ApplyPremul::True>(x, y, dst, count);
    }
}

template<typename DstType, SkColorProfileType profile, ApplyPremul premul>
void SkLinearGradient::
LinearGradient4fContext::shadePremulSpan(int x, int y,
                                         DstType dst[],
                                         int count) const {
    const SkLinearGradient& shader =
        static_cast<const SkLinearGradient&>(fShader);
    switch (shader.fTileMode) {
    case kClamp_TileMode:
        this->shadeSpanInternal<DstType,
                                profile,
                                premul,
                                kClamp_TileMode>(x, y, dst, count);
        break;
    case kRepeat_TileMode:
        this->shadeSpanInternal<DstType,
                                profile,
                                premul,
                                kRepeat_TileMode>(x, y, dst, count);
        break;
    case kMirror_TileMode:
        this->shadeSpanInternal<DstType,
                                profile,
                                premul,
                                kMirror_TileMode>(x, y, dst, count);
        break;
    }
}

template<typename DstType, SkColorProfileType profile, ApplyPremul premul,
         SkShader::TileMode tileMode>
void SkLinearGradient::
LinearGradient4fContext::shadeSpanInternal(int x, int y,
                                           DstType dst[],
                                           int count) const {
    SkPoint pt;
    fDstToPosProc(fDstToPos,
                  x + SK_ScalarHalf,
                  y + SK_ScalarHalf,
                  &pt);
    const SkScalar fx = pinFx<tileMode>(pt.x());
    const SkScalar dx = fDstToPos.getScaleX();
    LinearIntervalProcessor<DstType, profile, tileMode> proc(fIntervals.begin(),
                                                             fIntervals.end() - 1,
                                                             this->findInterval(fx),
                                                             fx,
                                                             dx,
                                                             SkScalarNearlyZero(dx * count));
    while (count > 0) {
        // What we really want here is SkTPin(advance, 1, count)
        // but that's a significant perf hit for >> stops; investigate.
        const int n = SkScalarTruncToInt(
            SkTMin<SkScalar>(proc.currentAdvance() + 1, SkIntToScalar(count)));

        // The current interval advance can be +inf (e.g. when reaching
        // the clamp mode end intervals) - when that happens, we expect to
        //   a) consume all remaining count in one swoop
        //   b) return a zero color gradient
        SkASSERT(SkScalarIsFinite(proc.currentAdvance())
            || (n == count && proc.currentRampIsZero()));

        if (proc.currentRampIsZero()) {
            fill<DstType, profile, premul>(proc.currentColor(),
                                           dst, n);
        } else {
            ramp<DstType, profile, premul>(proc.currentColor(),
                                           proc.currentColorGrad(),
                                           dst, n);
        }

        proc.advance(SkIntToScalar(n));
        count -= n;
        dst   += n;
    }
}

template<typename DstType, SkColorProfileType profile, SkShader::TileMode tileMode>
class SkLinearGradient::
LinearGradient4fContext::LinearIntervalProcessor {
public:
    LinearIntervalProcessor(const Interval* firstInterval,
                            const Interval* lastInterval,
                            const Interval* i,
                            SkScalar fx,
                            SkScalar dx,
                            bool is_vertical)
        : fAdvX((i->fP1 - fx) / dx)
        , fFirstInterval(firstInterval)
        , fLastInterval(lastInterval)
        , fInterval(i)
        , fDx(dx)
        , fIsVertical(is_vertical)
    {
        SkASSERT(firstInterval <= lastInterval);
        SkASSERT(in_range(fx, i->fP0, i->fP1));
        this->compute_interval_props(fx - i->fP0);
    }

    SkScalar currentAdvance() const {
        SkASSERT(fAdvX >= 0);
        SkASSERT(fAdvX <= (fInterval->fP1 - fInterval->fP0) / fDx);
        return fAdvX;
    }

    bool currentRampIsZero() const { return fZeroRamp; }
    const Sk4f& currentColor() const { return fCc; }
    const Sk4f& currentColorGrad() const { return fDcDx; }

    void advance(SkScalar advX) {
        SkASSERT(advX > 0);
        SkASSERT(fAdvX >= 0);

        if (advX >= fAdvX) {
            advX = this->advance_interval(advX);
        }
        SkASSERT(advX < fAdvX);

        fCc = fCc + fDcDx * Sk4f(advX);
        fAdvX -= advX;
    }

private:
    void compute_interval_props(SkScalar t) {
        fDc   = dst_swizzle<DstType>(fInterval->fDc);
        fCc   = dst_swizzle<DstType>(fInterval->fC0);
        fCc   = fCc + fDc * Sk4f(t);
        fCc   = scale_for_dest<DstType, profile>(fCc);
        fDcDx = scale_for_dest<DstType, profile>(fDc * Sk4f(fDx));
        fZeroRamp = fIsVertical || fInterval->isZeroRamp();
    }

    const Interval* next_interval(const Interval* i) const {
        SkASSERT(i >= fFirstInterval);
        SkASSERT(i <= fLastInterval);
        i++;

        if (tileMode == kClamp_TileMode) {
            SkASSERT(i <= fLastInterval);
            return i;
        }

        return (i <= fLastInterval) ? i : fFirstInterval;
    }

    SkScalar advance_interval(SkScalar advX) {
        SkASSERT(advX >= fAdvX);

        do {
            advX -= fAdvX;
            fInterval = this->next_interval(fInterval);
            fAdvX = (fInterval->fP1 - fInterval->fP0) / fDx;
            SkASSERT(fAdvX > 0);
        } while (advX >= fAdvX);

        compute_interval_props(0);

        SkASSERT(advX >= 0);
        return advX;
    }

    // Current interval properties.
    Sk4f            fDc;        // local color gradient (dc/dt)
    Sk4f            fDcDx;      // dst color gradient (dc/dx)
    Sk4f            fCc;        // current color, interpolated in dst
    SkScalar        fAdvX;      // remaining interval advance in dst
    bool            fZeroRamp;  // current interval color grad is 0

    const Interval* fFirstInterval;
    const Interval* fLastInterval;
    const Interval* fInterval;  // current interval
    const SkScalar  fDx;        // 'dx' for consistency with other impls; actually dt/dx
    const bool      fIsVertical;
};

void SkLinearGradient::
LinearGradient4fContext::mapTs(int x, int y, SkScalar ts[], int count) const {
    SkASSERT(count > 0);
    SkASSERT(fDstToPosClass != kLinear_MatrixClass);

    SkScalar sx = x + SK_ScalarHalf;
    const SkScalar sy = y + SK_ScalarHalf;
    SkPoint pt;

    if (fDstToPosClass != kPerspective_MatrixClass) {
        // kLinear_MatrixClass, kFixedStepInX_MatrixClass => fixed dt per scanline
        const SkScalar dtdx = fDstToPos.fixedStepInX(sy).x();
        fDstToPosProc(fDstToPos, sx, sy, &pt);

        const Sk4f dtdx4 = Sk4f(4 * dtdx);
        Sk4f t4 = Sk4f(pt.x() + 0 * dtdx,
                       pt.x() + 1 * dtdx,
                       pt.x() + 2 * dtdx,
                       pt.x() + 3 * dtdx);

        while (count >= 4) {
            t4.store(ts);
            t4 = t4 + dtdx4;
            ts += 4;
            count -= 4;
        }

        if (count & 2) {
            *ts++ = t4[0];
            *ts++ = t4[1];
            t4 = SkNx_shuffle<2, 0, 1, 3>(t4);
        }

        if (count & 1) {
            *ts++ = t4[0];
        }
    } else {
        for (int i = 0; i < count; ++i) {
            fDstToPosProc(fDstToPos, sx, sy, &pt);
            ts[i] = pt.x();
            sx += SK_Scalar1;
        }
    }
}

SkShader::Context::BlitProc SkLinearGradient::
LinearGradient4fContext::onChooseBlitProc(const SkImageInfo& info, BlitState* state) {
    SkXfermode::Mode mode;
    if (!SkXfermode::AsMode(state->fXfer, &mode)) {
        return nullptr;
    }

    const SkGradientShaderBase& shader = static_cast<const SkGradientShaderBase&>(fShader);
    if (mode != SkXfermode::kSrc_Mode &&
        !(mode == SkXfermode::kSrcOver_Mode && shader.colorsAreOpaque())) {
        return nullptr;
    }

    switch (info.colorType()) {
        case kN32_SkColorType:
            return D32_BlitProc;
        case kRGBA_F16_SkColorType:
            return D64_BlitProc;
        default:
            return nullptr;
    }
}

void SkLinearGradient::
LinearGradient4fContext::D32_BlitProc(BlitState* state, int x, int y, const SkPixmap& dst,
                                      int count, const SkAlpha aa[]) {
    // FIXME: ignoring coverage for now
    const LinearGradient4fContext* ctx =
        static_cast<const LinearGradient4fContext*>(state->fCtx);

    if (dst.info().isLinear()) {
        if (ctx->fColorsArePremul) {
            ctx->shadePremulSpan<SkPMColor, kLinear_SkColorProfileType, ApplyPremul::False>(
                x, y, dst.writable_addr32(x, y), count);
        } else {
            ctx->shadePremulSpan<SkPMColor, kLinear_SkColorProfileType, ApplyPremul::True>(
                x, y, dst.writable_addr32(x, y), count);
        }
    } else {
        if (ctx->fColorsArePremul) {
            ctx->shadePremulSpan<SkPMColor, kSRGB_SkColorProfileType, ApplyPremul::False>(
                x, y, dst.writable_addr32(x, y), count);
        } else {
            ctx->shadePremulSpan<SkPMColor, kSRGB_SkColorProfileType, ApplyPremul::True>(
                x, y, dst.writable_addr32(x, y), count);
        }
    }
}

void SkLinearGradient::
LinearGradient4fContext::D64_BlitProc(BlitState* state, int x, int y, const SkPixmap& dst,
                                      int count, const SkAlpha aa[]) {
    // FIXME: ignoring coverage for now
    const LinearGradient4fContext* ctx =
        static_cast<const LinearGradient4fContext*>(state->fCtx);

    if (ctx->fColorsArePremul) {
        ctx->shadePremulSpan<uint64_t, kLinear_SkColorProfileType, ApplyPremul::False>(
            x, y, dst.writable_addr64(x, y), count);
    } else {
        ctx->shadePremulSpan<uint64_t, kLinear_SkColorProfileType, ApplyPremul::True>(
            x, y, dst.writable_addr64(x, y), count);
    }
}

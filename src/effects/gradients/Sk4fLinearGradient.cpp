/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk4fLinearGradient.h"
#include "Sk4x4f.h"

#include <cmath>

namespace {

template<DstType dstType, ApplyPremul premul>
void ramp(const Sk4f& c, const Sk4f& dc, typename DstTraits<dstType, premul>::Type dst[], int n) {
    SkASSERT(n > 0);

    const Sk4f dc2 = dc + dc;
    const Sk4f dc4 = dc2 + dc2;

    Sk4f c0 = c ;
    Sk4f c1 = c + dc;
    Sk4f c2 = c0 + dc2;
    Sk4f c3 = c1 + dc2;

    while (n >= 4) {
        DstTraits<dstType, premul>::store4x(c0, c1, c2, c3, dst);
        dst += 4;

        c0 = c0 + dc4;
        c1 = c1 + dc4;
        c2 = c2 + dc4;
        c3 = c3 + dc4;
        n -= 4;
    }
    if (n & 2) {
        DstTraits<dstType, premul>::store(c0, dst++);
        DstTraits<dstType, premul>::store(c1, dst++);
        c0 = c0 + dc2;
    }
    if (n & 1) {
        DstTraits<dstType, premul>::store(c0, dst);
    }
}

// Planar version of ramp (S32 no-premul only).
template<>
void ramp<DstType::S32, ApplyPremul::False>(const Sk4f& c, const Sk4f& dc, SkPMColor dst[], int n) {
    SkASSERT(n > 0);

    const Sk4f    dc4 = dc * 4;
    const Sk4x4f dc4x = { Sk4f(dc4[0]), Sk4f(dc4[1]), Sk4f(dc4[2]), Sk4f(dc4[3]) };
    Sk4x4f        c4x = Sk4x4f::Transpose(c, c + dc, c + dc * 2, c + dc * 3);

    while (n >= 4) {
        ( sk_linear_to_srgb(c4x.r) <<  0
        | sk_linear_to_srgb(c4x.g) <<  8
        | sk_linear_to_srgb(c4x.b) << 16
        | Sk4f_round(255.0f*c4x.a) << 24).store(dst);

        c4x.r += dc4x.r;
        c4x.g += dc4x.g;
        c4x.b += dc4x.b;
        c4x.a += dc4x.a;

        dst += 4;
        n   -= 4;
    }

    if (n & 2) {
        DstTraits<DstType::S32, ApplyPremul::False>
            ::store(Sk4f(c4x.r[0], c4x.g[0], c4x.b[0], c4x.a[0]), dst++);
        DstTraits<DstType::S32, ApplyPremul::False>
            ::store(Sk4f(c4x.r[1], c4x.g[1], c4x.b[1], c4x.a[1]), dst++);
    }

    if (n & 1) {
        DstTraits<DstType::S32, ApplyPremul::False>
            ::store(Sk4f(c4x.r[n & 2], c4x.g[n & 2], c4x.b[n & 2], c4x.a[n & 2]), dst);
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
    SkScalar f = SkScalarFraction(fx);
    if (f < 0) {
        f = SkTMin(f + 1, nextafterf(1, 0));
    }
    SkASSERT(f >= 0);
    SkASSERT(f < 1.0f);
    return f;
}

template<>
SkScalar pinFx<SkShader::kMirror_TileMode>(SkScalar fx) {
    SkScalar f = SkScalarMod(fx, 2.0f);
    if (f < 0) {
        f = SkTMin(f + 2, nextafterf(2, 0));
    }
    SkASSERT(f >= 0);
    SkASSERT(f < 2.0f);
    return f;
}

// true when x is in [k1,k2], or [k2, k1] when the interval is reversed.
// TODO(fmalita): hoist the reversed interval check out of this helper.
bool in_range(SkScalar x, SkScalar k1, SkScalar k2) {
    SkASSERT(k1 != k2);
    return (k1 < k2)
        ? (x >= k1 && x <= k2)
        : (x >= k2 && x <= k1);
}

} // anonymous namespace

SkLinearGradient::
LinearGradient4fContext::LinearGradient4fContext(const SkLinearGradient& shader,
                                                 const ContextRec& rec)
    : INHERITED(shader, rec) {

    // Our fast path expects interval points to be monotonically increasing in x.
    const bool reverseIntervals = this->isFast() && std::signbit(fDstToPos.getScaleX());
    fIntervals.init(shader.fOrigColors, shader.fOrigPos, shader.fColorCount, shader.fTileMode,
                    fColorsArePremul, rec.fPaint->getAlpha() * (1.0f / 255), reverseIntervals);

    SkASSERT(fIntervals->count() > 0);
    fCachedInterval = fIntervals->begin();
}

const Sk4fGradientInterval*
SkLinearGradient::LinearGradient4fContext::findInterval(SkScalar fx) const {
    SkASSERT(in_range(fx, fIntervals->front().fT0, fIntervals->back().fT1));

    if (1) {
        // Linear search, using the last scanline interval as a starting point.
        SkASSERT(fCachedInterval >= fIntervals->begin());
        SkASSERT(fCachedInterval < fIntervals->end());
        const int search_dir = fDstToPos.getScaleX() >= 0 ? 1 : -1;
        while (!in_range(fx, fCachedInterval->fT0, fCachedInterval->fT1)) {
            fCachedInterval += search_dir;
            if (fCachedInterval >= fIntervals->end()) {
                fCachedInterval = fIntervals->begin();
            } else if (fCachedInterval < fIntervals->begin()) {
                fCachedInterval = fIntervals->end() - 1;
            }
        }
        return fCachedInterval;
    } else {
        // Binary search.  Seems less effective than linear + caching.
        const auto* i0 = fIntervals->begin();
        const auto* i1 = fIntervals->end() - 1;

        while (i0 != i1) {
            SkASSERT(i0 < i1);
            SkASSERT(in_range(fx, i0->fT0, i1->fT1));

            const auto* i = i0 + ((i1 - i0) >> 1);

            if (in_range(fx, i0->fT0, i->fT1)) {
                i1 = i;
            } else {
                SkASSERT(in_range(fx, i->fT1, i1->fT1));
                i0 = i + 1;
            }
        }

        SkASSERT(in_range(fx, i0->fT0, i0->fT1));
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
        this->shadePremulSpan<DstType::L32,
                              ApplyPremul::False>(x, y, dst, count);
    } else {
        this->shadePremulSpan<DstType::L32,
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
        this->shadePremulSpan<DstType::F32,
                              ApplyPremul::False>(x, y, dst, count);
    } else {
        this->shadePremulSpan<DstType::F32,
                              ApplyPremul::True>(x, y, dst, count);
    }
}

template<DstType dstType, ApplyPremul premul>
void SkLinearGradient::
LinearGradient4fContext::shadePremulSpan(int x, int y,
                                         typename DstTraits<dstType, premul>::Type dst[],
                                         int count) const {
    const SkLinearGradient& shader =
        static_cast<const SkLinearGradient&>(fShader);
    switch (shader.fTileMode) {
    case kClamp_TileMode:
        this->shadeSpanInternal<dstType,
                                premul,
                                kClamp_TileMode>(x, y, dst, count);
        break;
    case kRepeat_TileMode:
        this->shadeSpanInternal<dstType,
                                premul,
                                kRepeat_TileMode>(x, y, dst, count);
        break;
    case kMirror_TileMode:
        this->shadeSpanInternal<dstType,
                                premul,
                                kMirror_TileMode>(x, y, dst, count);
        break;
    }
}

template<DstType dstType, ApplyPremul premul, SkShader::TileMode tileMode>
void SkLinearGradient::
LinearGradient4fContext::shadeSpanInternal(int x, int y,
                                           typename DstTraits<dstType, premul>::Type dst[],
                                           int count) const {
    SkPoint pt;
    fDstToPosProc(fDstToPos,
                  x + SK_ScalarHalf,
                  y + SK_ScalarHalf,
                  &pt);
    const SkScalar fx = pinFx<tileMode>(pt.x());
    const SkScalar dx = fDstToPos.getScaleX();
    LinearIntervalProcessor<dstType, premul, tileMode> proc(fIntervals->begin(),
                                                            fIntervals->end() - 1,
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
            DstTraits<dstType, premul>::store(proc.currentColor(),
                                              dst, n);
        } else {
            ramp<dstType, premul>(proc.currentColor(),
                                  proc.currentColorGrad(),
                                  dst, n);
        }

        proc.advance(SkIntToScalar(n));
        count -= n;
        dst   += n;
    }
}

template<DstType dstType, ApplyPremul premul, SkShader::TileMode tileMode>
class SkLinearGradient::
LinearGradient4fContext::LinearIntervalProcessor {
public:
    LinearIntervalProcessor(const Sk4fGradientInterval* firstInterval,
                            const Sk4fGradientInterval* lastInterval,
                            const Sk4fGradientInterval* i,
                            SkScalar fx,
                            SkScalar dx,
                            bool is_vertical)
        : fAdvX(is_vertical ? SK_ScalarInfinity : (i->fT1 - fx) / dx)
        , fFirstInterval(firstInterval)
        , fLastInterval(lastInterval)
        , fInterval(i)
        , fDx(dx)
        , fIsVertical(is_vertical)
    {
        SkASSERT(fAdvX >= 0);
        SkASSERT(firstInterval <= lastInterval);

        if (tileMode != kClamp_TileMode && !is_vertical) {
            const auto spanX = (lastInterval->fT1 - firstInterval->fT0) / dx;
            SkASSERT(spanX >= 0);

            // If we're in a repeating tile mode and the whole gradient is compressed into a
            // fraction of a pixel, we just use the average color in zero-ramp mode.
            // This also avoids cases where we make no progress due to interval advances being
            // close to zero.
            static constexpr SkScalar kMinSpanX = .25f;
            if (spanX < kMinSpanX) {
                this->init_average_props();
                return;
            }
        }

        this->compute_interval_props(fx);
    }

    SkScalar currentAdvance() const {
        SkASSERT(fAdvX >= 0);
        SkASSERT(fAdvX <= (fInterval->fT1 - fInterval->fT0) / fDx || !std::isfinite(fAdvX));
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
        SkASSERT(in_range(t, fInterval->fT0, fInterval->fT1));

        fZeroRamp     = fIsVertical || fInterval->fZeroRamp;
        fCc           = DstTraits<dstType, premul>::load(fInterval->fCb);

        if (fInterval->fZeroRamp) {
            fDcDx = 0;
        } else {
            const Sk4f dC = DstTraits<dstType, premul>::load(fInterval->fCg);
            fCc           = fCc + dC * Sk4f(t);
            fDcDx         = dC * fDx;
        }
    }

    void init_average_props() {
        fAdvX     = SK_ScalarInfinity;
        fZeroRamp = true;
        fDcDx     = 0;
        fCc       = Sk4f(0);

        // TODO: precompute the average at interval setup time?
        for (const auto* i = fFirstInterval; i <= fLastInterval; ++i) {
            // Each interval contributes its average color to the total/weighted average:
            //
            //   C = (c0 + c1) / 2 = (Cb + Cg * t0 + Cb + Cg * t1) / 2 = Cb + Cg *(t0 + t1) / 2
            //
            //   Avg += C * (t1 - t0)
            //
            auto c = DstTraits<dstType, premul>::load(i->fCb);
            if (!i->fZeroRamp) {
                c = c + DstTraits<dstType, premul>::load(i->fCg) * (i->fT0 + i->fT1) * 0.5f;
            }
            fCc = fCc + c * (i->fT1 - i->fT0);
        }
    }

    const Sk4fGradientInterval* next_interval(const Sk4fGradientInterval* i) const {
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
            fAdvX = (fInterval->fT1 - fInterval->fT0) / fDx;
            SkASSERT(fAdvX > 0);
        } while (advX >= fAdvX);

        compute_interval_props(fInterval->fT0);

        SkASSERT(advX >= 0);
        return advX;
    }

    // Current interval properties.
    Sk4f            fDcDx;      // dst color gradient (dc/dx)
    Sk4f            fCc;        // current color, interpolated in dst
    SkScalar        fAdvX;      // remaining interval advance in dst
    bool            fZeroRamp;  // current interval color grad is 0

    const Sk4fGradientInterval* fFirstInterval;
    const Sk4fGradientInterval* fLastInterval;
    const Sk4fGradientInterval* fInterval;  // current interval
    const SkScalar              fDx;        // 'dx' for consistency with other impls; actually dt/dx
    const bool                  fIsVertical;
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
            // Perspective may yield NaN values.
            // Short of a better idea, drop to 0.
            ts[i] = SkScalarIsNaN(pt.x()) ? 0 : pt.x();
            sx += SK_Scalar1;
        }
    }
}

bool SkLinearGradient::LinearGradient4fContext::onChooseBlitProcs(const SkImageInfo& info,
                                                                  BlitState* state) {
    if (state->fMode != SkBlendMode::kSrc &&
        !(state->fMode == SkBlendMode::kSrcOver && (fFlags & kOpaqueAlpha_Flag))) {
        return false;
    }

    switch (info.colorType()) {
        case kN32_SkColorType:
            state->fBlitBW = D32_BlitBW;
            return true;
        case kRGBA_F16_SkColorType:
            state->fBlitBW = D64_BlitBW;
            return true;
        default:
            return false;
    }
}

void SkLinearGradient::
LinearGradient4fContext::D32_BlitBW(BlitState* state, int x, int y, const SkPixmap& dst,
                                    int count) {
    // FIXME: ignoring coverage for now
    const LinearGradient4fContext* ctx =
        static_cast<const LinearGradient4fContext*>(state->fCtx);

    if (!dst.info().gammaCloseToSRGB()) {
        if (ctx->fColorsArePremul) {
            ctx->shadePremulSpan<DstType::L32, ApplyPremul::False>(
                x, y, dst.writable_addr32(x, y), count);
        } else {
            ctx->shadePremulSpan<DstType::L32, ApplyPremul::True>(
                x, y, dst.writable_addr32(x, y), count);
        }
    } else {
        if (ctx->fColorsArePremul) {
            ctx->shadePremulSpan<DstType::S32, ApplyPremul::False>(
                x, y, dst.writable_addr32(x, y), count);
        } else {
            ctx->shadePremulSpan<DstType::S32, ApplyPremul::True>(
                x, y, dst.writable_addr32(x, y), count);
        }
    }
}

void SkLinearGradient::
LinearGradient4fContext::D64_BlitBW(BlitState* state, int x, int y, const SkPixmap& dst,
                                    int count) {
    // FIXME: ignoring coverage for now
    const LinearGradient4fContext* ctx =
        static_cast<const LinearGradient4fContext*>(state->fCtx);

    if (ctx->fColorsArePremul) {
        ctx->shadePremulSpan<DstType::F16, ApplyPremul::False>(
            x, y, dst.writable_addr64(x, y), count);
    } else {
        ctx->shadePremulSpan<DstType::F16, ApplyPremul::True>(
            x, y, dst.writable_addr64(x, y), count);
    }
}

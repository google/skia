/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk4fLinearGradient.h"
#include "Sk4x4f.h"
#include "SkPaint.h"

#include <cmath>

namespace {

template<typename dstType, ApplyPremul premul>
void ramp(const Sk4f& c, const Sk4f& dc, dstType dst[], int n,
          const Sk4f& bias0, const Sk4f& bias1) {
    SkASSERT(n > 0);

    const Sk4f dc2 = dc + dc,
               dc4 = dc2 + dc2;

    Sk4f c0 =  c +      DstTraits<dstType, premul>::pre_lerp_bias(bias0),
         c1 =  c + dc + DstTraits<dstType, premul>::pre_lerp_bias(bias1),
         c2 = c0 + dc2,
         c3 = c1 + dc2;

    while (n >= 4) {
        DstTraits<dstType, premul>::store4x(c0, c1, c2, c3, dst, bias0, bias1);
        dst += 4;

        c0 = c0 + dc4;
        c1 = c1 + dc4;
        c2 = c2 + dc4;
        c3 = c3 + dc4;
        n -= 4;
    }
    if (n & 2) {
        DstTraits<dstType, premul>::store(c0, dst++, bias0);
        DstTraits<dstType, premul>::store(c1, dst++, bias1);
        c0 = c0 + dc2;
    }
    if (n & 1) {
        DstTraits<dstType, premul>::store(c0, dst, bias0);
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
    const bool reverseIntervals = std::signbit(fDstToPos.getScaleX());
    fIntervals.init(shader, rec.fDstColorSpace, shader.fTileMode,
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
    SkASSERT(count > 0);

    float bias0 = 0,
          bias1 = 0;

    if (fDither) {
        static constexpr float dither_cell[] = {
            -3/8.0f,  1/8.0f,
             3/8.0f, -1/8.0f,
        };

        const int rowIndex = (y & 1) << 1;
        bias0 = dither_cell[rowIndex + 0];
        bias1 = dither_cell[rowIndex + 1];

        if (x & 1) {
            SkTSwap(bias0, bias1);
        }
    }

    if (fColorsArePremul) {
        // In premul interpolation mode, components are pre-scaled by 255 and the store
        // op is truncating. We pre-bias here to achieve rounding.
        bias0 += 0.5f;
        bias1 += 0.5f;

        this->shadePremulSpan<SkPMColor, ApplyPremul::False>(x, y, dst, count, bias0, bias1);
    } else {
        // In unpremul interpolation mode, Components are not pre-scaled.
        bias0 *= 1/255.0f;
        bias1 *= 1/255.0f;

        this->shadePremulSpan<SkPMColor, ApplyPremul::True >(x, y, dst, count, bias0, bias1);
    }
}

void SkLinearGradient::
LinearGradient4fContext::shadeSpan4f(int x, int y, SkPM4f dst[], int count) {
    SkASSERT(count > 0);

    // 4f dests are dithered at a later stage, if needed.
    static constexpr float bias0 = 0,
                           bias1 = 0;
    if (fColorsArePremul) {
        this->shadePremulSpan<SkPM4f, ApplyPremul::False>(x, y, dst, count, bias0, bias1);
    } else {
        this->shadePremulSpan<SkPM4f, ApplyPremul::True >(x, y, dst, count, bias0, bias1);
    }
}

template<typename dstType, ApplyPremul premul>
void SkLinearGradient::
LinearGradient4fContext::shadePremulSpan(int x, int y, dstType dst[], int count,
                                         float bias0, float bias1) const {
    const SkLinearGradient& shader = static_cast<const SkLinearGradient&>(fShader);
    switch (shader.fTileMode) {
    case kClamp_TileMode:
        this->shadeSpanInternal<dstType, premul, kClamp_TileMode >(x, y, dst, count, bias0, bias1);
        break;
    case kRepeat_TileMode:
        this->shadeSpanInternal<dstType, premul, kRepeat_TileMode>(x, y, dst, count, bias0, bias1);
        break;
    case kMirror_TileMode:
        this->shadeSpanInternal<dstType, premul, kMirror_TileMode>(x, y, dst, count, bias0, bias1);
        break;
    }
}

template<typename dstType, ApplyPremul premul, SkShader::TileMode tileMode>
void SkLinearGradient::
LinearGradient4fContext::shadeSpanInternal(int x, int y, dstType dst[], int count,
                                           float bias0, float bias1) const {
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
    Sk4f bias4f0(bias0),
         bias4f1(bias1);

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
            DstTraits<dstType, premul>::store(proc.currentColor(), dst, n);
        } else {
            ramp<dstType, premul>(proc.currentColor(), proc.currentColorGrad(), dst, n,
                                  bias4f0, bias4f1);
        }

        proc.advance(SkIntToScalar(n));
        count -= n;
        dst   += n;

        if (n & 1) {
            SkTSwap(bias4f0, bias4f1);
        }
    }
}

template<typename dstType, ApplyPremul premul, SkShader::TileMode tileMode>
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

        const Sk4f dc = DstTraits<dstType, premul>::load(fInterval->fCg);
                  fCc = DstTraits<dstType, premul>::load(fInterval->fCb) + dc * Sk4f(t);
                fDcDx = dc * fDx;
            fZeroRamp = fIsVertical || (dc == 0).allTrue();
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
            const auto c = DstTraits<dstType, premul>::load(i->fCb)
                         + DstTraits<dstType, premul>::load(i->fCg) * (i->fT0 + i->fT1) * 0.5f;
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

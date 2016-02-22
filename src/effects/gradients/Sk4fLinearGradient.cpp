/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk4fLinearGradient.h"

namespace {

Sk4f premul_4f(const Sk4f& c) {
    const float alpha = c[SkPM4f::A];
    // FIXME: portable swizzle?
    return c * Sk4f(alpha, alpha, alpha, 1);
}

template <bool do_premul>
SkPMColor trunc_from_255(const Sk4f& c) {
    SkPMColor pmc;
    SkNx_cast<uint8_t>(c).store(&pmc);
    if (do_premul) {
        pmc = SkPreMultiplyARGB(SkGetPackedA32(pmc), SkGetPackedR32(pmc),
                                SkGetPackedG32(pmc), SkGetPackedB32(pmc));
    }
    return pmc;
}

template<typename DstType, bool do_premul>
void fill(const Sk4f& c, DstType* dst, int n);

template<>
void fill<SkPM4f, false>(const Sk4f& c, SkPM4f* dst, int n) {
    while (n > 0) {
        c.store(dst++);
        n--;
    }
}

template<>
void fill<SkPM4f, true>(const Sk4f& c, SkPM4f* dst, int n) {
    fill<SkPM4f, false>(premul_4f(c), dst, n);
}

template<>
void fill<SkPMColor, false>(const Sk4f& c, SkPMColor* dst, int n) {
    sk_memset32(dst, trunc_from_255<false>(c), n);
}

template<>
void fill<SkPMColor, true>(const Sk4f& c, SkPMColor* dst, int n) {
    sk_memset32(dst, trunc_from_255<true>(c), n);
}

template<typename DstType, bool do_premul>
void store(const Sk4f& color, DstType* dst);

template<>
void store<SkPM4f, false>(const Sk4f& c, SkPM4f* dst) {
    c.store(dst);
}

template<>
void store<SkPM4f, true>(const Sk4f& c, SkPM4f* dst) {
    store<SkPM4f, false>(premul_4f(c), dst);
}

template<>
void store<SkPMColor, false>(const Sk4f& c, SkPMColor* dst) {
    *dst = trunc_from_255<false>(c);
}

template<>
void store<SkPMColor, true>(const Sk4f& c, SkPMColor* dst) {
    *dst = trunc_from_255<true>(c);
}

template<typename DstType, bool do_premul>
void store4x(const Sk4f& c0,
             const Sk4f& c1,
             const Sk4f& c2,
             const Sk4f& c3,
             DstType* dst) {
    store<DstType, do_premul>(c0, dst++);
    store<DstType, do_premul>(c1, dst++);
    store<DstType, do_premul>(c2, dst++);
    store<DstType, do_premul>(c3, dst++);
}

template<>
void store4x<SkPMColor, false>(const Sk4f& c0,
                               const Sk4f& c1,
                               const Sk4f& c2,
                               const Sk4f& c3,
                               SkPMColor* dst) {
    Sk4f_ToBytes((uint8_t*)dst, c0, c1, c2, c3);
}

template<typename DstType, bool do_premul>
void ramp(const Sk4f& c, const Sk4f& dc, DstType* dst, int n) {
    SkASSERT(n > 0);

    const Sk4f dc2 = dc + dc;
    const Sk4f dc4 = dc2 + dc2;

    Sk4f c0 = c ;
    Sk4f c1 = c + dc;
    Sk4f c2 = c0 + dc2;
    Sk4f c3 = c1 + dc2;

    while (n >= 4) {
        store4x<DstType, do_premul>(c0, c1, c2, c3, dst);
        dst += 4;

        c0 = c0 + dc4;
        c1 = c1 + dc4;
        c2 = c2 + dc4;
        c3 = c3 + dc4;
        n -= 4;
    }
    if (n & 2) {
        store<DstType, do_premul>(c0, dst++);
        store<DstType, do_premul>(c1, dst++);
        c0 = c0 + dc2;
    }
    if (n & 1) {
        store<DstType, do_premul>(c0, dst);
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

template<typename DstType>
float dst_component_scale();

template<>
float dst_component_scale<SkPM4f>() {
    return 1;
}

template<>
float dst_component_scale<SkPMColor>() {
    return 255;
}

} // anonymous namespace

SkLinearGradient::
LinearGradient4fContext::LinearGradient4fContext(const SkLinearGradient& shader,
                                                 const ContextRec& rec)
    : INHERITED(shader, rec) {}

void SkLinearGradient::
LinearGradient4fContext::shadeSpan(int x, int y, SkPMColor dst[], int count) {
    // TODO: plumb dithering
    SkASSERT(count > 0);
    if (fColorsArePremul) {
        this->shadePremulSpan<SkPMColor, false>(x, y, dst, count);
    } else {
        this->shadePremulSpan<SkPMColor, true>(x, y, dst, count);
    }
}

void SkLinearGradient::
LinearGradient4fContext::shadeSpan4f(int x, int y, SkPM4f dst[], int count) {
    // TONOTDO: plumb dithering
    SkASSERT(count > 0);
    if (fColorsArePremul) {
        this->shadePremulSpan<SkPM4f, false>(x, y, dst, count);
    } else {
        this->shadePremulSpan<SkPM4f, true>(x, y, dst, count);
    }
}

template<typename DstType, bool do_premul>
void SkLinearGradient::
LinearGradient4fContext::shadePremulSpan(int x, int y,
                                         DstType dst[],
                                         int count) const {
    const SkLinearGradient& shader =
        static_cast<const SkLinearGradient&>(fShader);
    switch (shader.fTileMode) {
    case kClamp_TileMode:
        this->shadeSpanInternal<DstType,
                                do_premul,
                                kClamp_TileMode>(x, y, dst, count);
        break;
    case kRepeat_TileMode:
        this->shadeSpanInternal<DstType,
                                do_premul,
                                kRepeat_TileMode>(x, y, dst, count);
        break;
    case kMirror_TileMode:
        this->shadeSpanInternal<DstType,
                                do_premul,
                                kMirror_TileMode>(x, y, dst, count);
        break;
    }
}

template<typename DstType, bool do_premul, SkShader::TileMode tileMode>
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
    LinearIntervalProcessor<DstType, tileMode> proc(fIntervals.begin(),
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
            fill<DstType, do_premul>(proc.currentColor(),
                                     dst, n);
        } else {
            ramp<DstType, do_premul>(proc.currentColor(),
                                     proc.currentColorGrad(),
                                     dst, n);
        }

        proc.advance(SkIntToScalar(n));
        count -= n;
        dst   += n;
    }
}

template<typename DstType, SkShader::TileMode tileMode>
class SkLinearGradient::
LinearGradient4fContext::LinearIntervalProcessor {
public:
    LinearIntervalProcessor(const Interval* firstInterval,
                            const Interval* lastInterval,
                            const Interval* i,
                            SkScalar fx,
                            SkScalar dx,
                            bool is_vertical)
        : fDstComponentScale(dst_component_scale<DstType>())
        , fAdvX((i->fP1 - fx) / dx)
        , fFirstInterval(firstInterval)
        , fLastInterval(lastInterval)
        , fInterval(i)
        , fDx(dx)
        , fIsVertical(is_vertical)
    {
        SkASSERT(firstInterval <= lastInterval);
        SkASSERT(i->contains(fx));
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
        fDc   = Sk4f::Load(fInterval->fDc.fVec);
        fCc   = Sk4f::Load(fInterval->fC0.fVec);
        fCc   = fCc + fDc * Sk4f(t);
        fCc   = fCc * fDstComponentScale;
        fDcDx = fDc * fDstComponentScale * Sk4f(fDx);
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

    const Sk4f      fDstComponentScale; // cached dst scale (PMC: 255, PM4f: 1)

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

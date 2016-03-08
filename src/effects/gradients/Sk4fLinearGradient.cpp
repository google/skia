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

template<typename DstType>
Sk4f dst_swizzle(const SkPM4f&);

template<>
Sk4f dst_swizzle<SkPM4f>(const SkPM4f& c) {
    return c.to4f();
}

template<>
Sk4f dst_swizzle<SkPMColor>(const SkPM4f& c) {
    return c.to4f_pmorder();
}

SkPMColor pack_color(SkColor c, bool premul) {
    return premul
        ? SkPreMultiplyColor(c)
        : SkPackARGB32NoCheck(SkColorGetA(c), SkColorGetR(c), SkColorGetG(c), SkColorGetB(c));
}

// true when x is in [k1,k2)
bool in_range(SkScalar x, SkScalar k1, SkScalar k2) {
    SkASSERT(k1 != k2);
    return (k1 < k2)
        ? (x >= k1 && x < k2)
        : (x >= k2 && x < k1);
}

class IntervalBuilder {
public:
    IntervalBuilder(const SkColor* colors, const SkScalar* pos, int count, bool reverse)
        : fColors(colors)
        , fPos(pos)
        , fCount(count)
        , fFirstPos(reverse ? SK_Scalar1 : 0)
        , fBegin(reverse ? count - 1 : 0)
        , fAdvance(reverse ? -1 : 1) {
        SkASSERT(colors);
        SkASSERT(count > 1);
    }

    template<typename F>
    void build(F func) const {
        if (!fPos) {
            this->buildImplicitPos(func);
            return;
        }

        const int end = fBegin + fAdvance * (fCount - 1);
        const SkScalar lastPos = 1 - fFirstPos;
        int prev = fBegin;
        SkScalar prevPos = fFirstPos;

        do {
            const int curr = prev + fAdvance;
            SkASSERT(curr >= 0 && curr < fCount);

            // TODO: this sanitization should be done in SkGradientShaderBase
            const SkScalar currPos = (fAdvance > 0)
                ? SkTPin(fPos[curr], prevPos, lastPos)
                : SkTPin(fPos[curr], lastPos, prevPos);

            if (currPos != prevPos) {
                SkASSERT((currPos - prevPos > 0) == (fAdvance > 0));
                func(fColors[prev], fColors[curr], prevPos, currPos);
            }

            prev = curr;
            prevPos = currPos;
        } while (prev != end);
    }

private:
    template<typename F>
    void buildImplicitPos(F func) const {
        // When clients don't provide explicit color stop positions (fPos == nullptr),
        // the color stops are distributed evenly across the unit interval
        // (implicit positioning).
        const SkScalar dt = fAdvance * SK_Scalar1 / (fCount - 1);
        const int end = fBegin + fAdvance * (fCount - 2);
        int prev = fBegin;
        SkScalar prevPos = fFirstPos;

        while (prev != end) {
            const int curr = prev + fAdvance;
            SkASSERT(curr >= 0 && curr < fCount);

            const SkScalar currPos = prevPos + dt;
            func(fColors[prev], fColors[curr], prevPos, currPos);
            prev = curr;
            prevPos = currPos;
        }

        // emit the last interval with a pinned end position, to avoid precision issues
        func(fColors[prev], fColors[prev + fAdvance], prevPos, 1 - fFirstPos);
    }

    const SkColor*  fColors;
    const SkScalar* fPos;
    const int       fCount;
    const SkScalar  fFirstPos;
    const int       fBegin;
    const int       fAdvance;
};

} // anonymous namespace

SkLinearGradient::
LinearGradient4fContext::LinearGradient4fContext(const SkLinearGradient& shader,
                                                 const ContextRec& rec)
    : INHERITED(shader, rec) {
    // The main job here is to build a specialized interval list: a different
    // representation of the color stops data, optimized for efficient scan line
    // access during shading.
    //
    //   [{P0,C0} , {P1,C1}) [{P1,C2} , {P2,c3}) ... [{Pn,C2n} , {Pn+1,C2n+1})
    //
    // The list is sorted in increasing dst order, i.e. X(Pk) < X(Pk+1).  This
    // allows us to always traverse left->right when iterating over a scan line.
    // It also means that the interval order matches the color stops when dx >= 0,
    // and is the inverse (pos, colors, order are flipped) when dx < 0.
    //
    // Note: the current representation duplicates pos data; we could refactor to
    //       avoid this if interval storage size becomes a concern.
    //
    // Aside from reordering, we also perform two more pre-processing steps at
    // this stage:
    //
    //   1) scale the color components depending on paint alpha and the requested
    //      interpolation space (note: the interval color storage is SkPM4f, but
    //      that doesn't necessarily mean the colors are premultiplied; that
    //      property is tracked in fColorsArePremul)
    //
    //   2) inject synthetic intervals to support tiling.
    //
    //      * for kRepeat, no extra intervals are needed - the iterator just
    //        wraps around at the end:
    //
    //          ->[P0,P1)->..[Pn-1,Pn)->
    //
    //      * for kClamp, we add two "infinite" intervals before/after:
    //
    //          [-/+inf , P0)->[P0 , P1)->..[Pn-1 , Pn)->[Pn , +/-inf)
    //
    //        (the iterator should never run off the end in this mode)
    //
    //      * for kMirror, we extend the range to [0..2] and add a flipped
    //        interval series - then the iterator operates just as in the
    //        kRepeat case:
    //
    //          ->[P0,P1)->..[Pn-1,Pn)->[2 - Pn,2 - Pn-1)->..[2 - P1,2 - P0)->
    //
    // TODO: investigate collapsing intervals << 1px.

    SkASSERT(shader.fColorCount > 1);
    SkASSERT(shader.fOrigColors);

    const float paintAlpha = rec.fPaint->getAlpha() * (1.0f / 255);
    const Sk4f componentScale = fColorsArePremul
        ? Sk4f(paintAlpha)
        : Sk4f(1.0f, 1.0f, 1.0f, paintAlpha);
    const bool dx_is_pos = fDstToPos.getScaleX() >= 0;
    const int first_index = dx_is_pos ? 0 : shader.fColorCount - 1;
    const int last_index = shader.fColorCount - 1 - first_index;
    const SkScalar first_pos = dx_is_pos ? 0 : SK_Scalar1;
    const SkScalar last_pos = 1 - first_pos;

    if (shader.fTileMode == SkShader::kClamp_TileMode) {
        // synthetic edge interval: -/+inf .. P0
        const SkPMColor clamp_color = pack_color(shader.fOrigColors[first_index],
                                                 fColorsArePremul);
        const SkScalar clamp_pos = dx_is_pos ? SK_ScalarMin : SK_ScalarMax;
        fIntervals.emplace_back(clamp_color, clamp_pos,
                                clamp_color, first_pos,
                                componentScale);
    } else if (shader.fTileMode == SkShader::kMirror_TileMode && !dx_is_pos) {
        // synthetic mirror intervals injected before main intervals: (2 .. 1]
        addMirrorIntervals(shader, componentScale, dx_is_pos);
    }

    const IntervalBuilder builder(shader.fOrigColors,
                                  shader.fOrigPos,
                                  shader.fColorCount,
                                  !dx_is_pos);
    builder.build([this, &componentScale] (SkColor c0, SkColor c1, SkScalar p0, SkScalar p1) {
        SkASSERT(fIntervals.empty() || fIntervals.back().fP1 == p0);

        fIntervals.emplace_back(pack_color(c0, fColorsArePremul),
                                p0,
                                pack_color(c1, fColorsArePremul),
                                p1,
                                componentScale);
    });

    if (shader.fTileMode == SkShader::kClamp_TileMode) {
        // synthetic edge interval: Pn .. +/-inf
        const SkPMColor clamp_color =
            pack_color(shader.fOrigColors[last_index], fColorsArePremul);
        const SkScalar clamp_pos = dx_is_pos ? SK_ScalarMax : SK_ScalarMin;
        fIntervals.emplace_back(clamp_color, last_pos,
                                clamp_color, clamp_pos,
                                componentScale);
    } else if (shader.fTileMode == SkShader::kMirror_TileMode && dx_is_pos) {
        // synthetic mirror intervals injected after main intervals: [1 .. 2)
        addMirrorIntervals(shader, componentScale, dx_is_pos);
    }

    SkASSERT(fIntervals.count() > 0);
    fCachedInterval = fIntervals.begin();
}

void SkLinearGradient::
LinearGradient4fContext::addMirrorIntervals(const SkLinearGradient& shader,
                                            const Sk4f& componentScale, bool dx_is_pos) {
    // Iterates in reverse order (vs main interval builder) and adds intervals reflected in 2.
    const IntervalBuilder builder(shader.fOrigColors,
                                  shader.fOrigPos,
                                  shader.fColorCount,
                                  dx_is_pos);
    builder.build([this, &componentScale] (SkColor c0, SkColor c1, SkScalar p0, SkScalar p1) {
        SkASSERT(fIntervals.empty() || fIntervals.back().fP1 == 2 - p0);

        fIntervals.emplace_back(pack_color(c0, fColorsArePremul),
                                2 - p0,
                                pack_color(c1, fColorsArePremul),
                                2 - p1,
                                componentScale);
    });
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
        fDc   = dst_swizzle<DstType>(fInterval->fDc);
        fCc   = dst_swizzle<DstType>(fInterval->fC0);
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

/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk4fGradientBase.h"

#include <functional>

namespace {

Sk4f pack_color(SkColor c, bool premul, const Sk4f& component_scale) {
    const SkColor4f c4f = SkColor4f::FromColor(c);
    const Sk4f pm4f = premul
        ? c4f.premul().to4f()
        : Sk4f{c4f.fR, c4f.fG, c4f.fB, c4f.fA};

    return pm4f * component_scale;
}

class IntervalIterator {
public:
    IntervalIterator(const SkColor* colors, const SkScalar* pos, int count, bool reverse)
        : fColors(colors)
        , fPos(pos)
        , fCount(count)
        , fFirstPos(reverse ? SK_Scalar1 : 0)
        , fBegin(reverse ? count - 1 : 0)
        , fAdvance(reverse ? -1 : 1) {
        SkASSERT(colors);
        SkASSERT(count > 0);
    }

    void iterate(std::function<void(SkColor, SkColor, SkScalar, SkScalar)> func) const {
        if (!fPos) {
            this->iterateImplicitPos(func);
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
    void iterateImplicitPos(std::function<void(SkColor, SkColor, SkScalar, SkScalar)> func) const {
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

void addMirrorIntervals(const SkColor colors[],
                        const SkScalar pos[], int count,
                        const Sk4f& componentScale,
                        bool premulColors, bool reverse,
                        Sk4fGradientIntervalBuffer::BufferType* buffer) {
    const IntervalIterator iter(colors, pos, count, reverse);
    iter.iterate([&] (SkColor c0, SkColor c1, SkScalar t0, SkScalar t1) {
        SkASSERT(buffer->empty() || buffer->back().fT1 == 2 - t0);

        const auto mirror_t0 = 2 - t0;
        const auto mirror_t1 = 2 - t1;
        // mirror_p1 & mirror_p1 may collapse for very small values - recheck to avoid
        // triggering Interval asserts.
        if (mirror_t0 != mirror_t1) {
            buffer->emplace_back(pack_color(c0, premulColors, componentScale), mirror_t0,
                                 pack_color(c1, premulColors, componentScale), mirror_t1);
        }
    });
}

} // anonymous namespace

Sk4fGradientInterval::Sk4fGradientInterval(const Sk4f& c0, SkScalar t0,
                                           const Sk4f& c1, SkScalar t1)
    : fT0(t0)
    , fT1(t1)
    , fZeroRamp((c0 == c1).allTrue()) {
    SkASSERT(t0 != t1);
    // Either p0 or p1 can be (-)inf for synthetic clamp edge intervals.
    SkASSERT(SkScalarIsFinite(t0) || SkScalarIsFinite(t1));

    const auto dt = t1 - t0;

    // Clamp edge intervals are always zero-ramp.
    SkASSERT(SkScalarIsFinite(dt) || fZeroRamp);
    SkASSERT(SkScalarIsFinite(t0) || fZeroRamp);
    const Sk4f   dc = SkScalarIsFinite(dt) ? (c1 - c0) / dt : 0;
    const Sk4f bias = c0 - (SkScalarIsFinite(t0) ? t0 * dc : 0);

    bias.store(&fCb.fVec);
    dc.store(&fCg.fVec);
}

void Sk4fGradientIntervalBuffer::init(const SkColor colors[], const SkScalar pos[], int count,
                                      SkShader::TileMode tileMode, bool premulColors,
                                      SkScalar alpha, bool reverse) {
    // The main job here is to build a specialized interval list: a different
    // representation of the color stops data, optimized for efficient scan line
    // access during shading.
    //
    //   [{P0,C0} , {P1,C1}) [{P1,C2} , {P2,c3}) ... [{Pn,C2n} , {Pn+1,C2n+1})
    //
    // The list may be inverted when requested (such that e.g. points are sorted
    // in increasing x order when dx < 0).
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

    SkASSERT(count > 0);
    SkASSERT(colors);

    fIntervals.reset();

    const Sk4f componentScale = premulColors
        ? Sk4f(alpha)
        : Sk4f(1.0f, 1.0f, 1.0f, alpha);
    const int first_index = reverse ? count - 1 : 0;
    const int last_index = count - 1 - first_index;
    const SkScalar first_pos = reverse ? SK_Scalar1 : 0;
    const SkScalar last_pos = SK_Scalar1 - first_pos;

    if (tileMode == SkShader::kClamp_TileMode) {
        // synthetic edge interval: -/+inf .. P0
        const Sk4f clamp_color = pack_color(colors[first_index],
                                            premulColors, componentScale);
        const SkScalar clamp_pos = reverse ? SK_ScalarInfinity : SK_ScalarNegativeInfinity;
        fIntervals.emplace_back(clamp_color, clamp_pos,
                                clamp_color, first_pos);
    } else if (tileMode == SkShader::kMirror_TileMode && reverse) {
        // synthetic mirror intervals injected before main intervals: (2 .. 1]
        addMirrorIntervals(colors, pos, count, componentScale, premulColors, false, &fIntervals);
    }

    const IntervalIterator iter(colors, pos, count, reverse);
    iter.iterate([&] (SkColor c0, SkColor c1, SkScalar t0, SkScalar t1) {
        SkASSERT(fIntervals.empty() || fIntervals.back().fT1 == t0);

        fIntervals.emplace_back(pack_color(c0, premulColors, componentScale), t0,
                                pack_color(c1, premulColors, componentScale), t1);
    });

    if (tileMode == SkShader::kClamp_TileMode) {
        // synthetic edge interval: Pn .. +/-inf
        const Sk4f clamp_color = pack_color(colors[last_index], premulColors, componentScale);
        const SkScalar clamp_pos = reverse ? SK_ScalarNegativeInfinity : SK_ScalarInfinity;
        fIntervals.emplace_back(clamp_color, last_pos,
                                clamp_color, clamp_pos);
    } else if (tileMode == SkShader::kMirror_TileMode && !reverse) {
        // synthetic mirror intervals injected after main intervals: [1 .. 2)
        addMirrorIntervals(colors, pos, count, componentScale, premulColors, true, &fIntervals);
    }
}

const Sk4fGradientInterval* Sk4fGradientIntervalBuffer::find(SkScalar t) const {
    // Binary search.
    const auto* i0 = fIntervals.begin();
    const auto* i1 = fIntervals.end() - 1;

    while (i0 != i1) {
        SkASSERT(i0 < i1);
        SkASSERT(t >= i0->fT0 && t <= i1->fT1);

        const auto* i = i0 + ((i1 - i0) >> 1);

        if (t > i->fT1) {
            i0 = i + 1;
        } else {
            i1 = i;
        }
    }

    SkASSERT(i0->contains(t));
    return i0;
}

const Sk4fGradientInterval* Sk4fGradientIntervalBuffer::findNext(
    SkScalar t, const Sk4fGradientInterval* prev, bool increasing) const {

    SkASSERT(!prev->contains(t));
    SkASSERT(prev >= fIntervals.begin() && prev < fIntervals.end());
    SkASSERT(t >= fIntervals.front().fT0 && t <= fIntervals.back().fT1);

    const auto* i = prev;

    // Use the |increasing| signal to figure which direction we should search for
    // the next interval, then perform a linear search.
    if (increasing) {
        do {
            i += 1;
            if (i >= fIntervals.end()) {
                i = fIntervals.begin();
            }
        } while (!i->contains(t));
    } else {
        do {
            i -= 1;
            if (i < fIntervals.begin()) {
                i = fIntervals.end() - 1;
            }
        } while (!i->contains(t));
    }

    return i;
}

SkGradientShaderBase::
GradientShaderBase4fContext::GradientShaderBase4fContext(const SkGradientShaderBase& shader,
                                                         const ContextRec& rec)
    : INHERITED(shader, rec)
    , fFlags(this->INHERITED::getFlags())
#ifdef SK_SUPPORT_LEGACY_GRADIENT_DITHERING
    , fDither(true)
#else
    , fDither(rec.fPaint->isDither())
#endif
{
    const SkMatrix& inverse = this->getTotalInverse();
    fDstToPos.setConcat(shader.fPtsToUnit, inverse);
    fDstToPosProc = fDstToPos.getMapXYProc();
    fDstToPosClass = static_cast<uint8_t>(INHERITED::ComputeMatrixClass(fDstToPos));

    if (shader.fColorsAreOpaque && this->getPaintAlpha() == SK_AlphaOPAQUE) {
        fFlags |= kOpaqueAlpha_Flag;
    }

    fColorsArePremul =
        (shader.fGradFlags & SkGradientShader::kInterpolateColorsInPremul_Flag)
        || shader.fColorsAreOpaque;
}

bool SkGradientShaderBase::
GradientShaderBase4fContext::isValid() const {
    return fDstToPos.isFinite();
}

void SkGradientShaderBase::
GradientShaderBase4fContext::shadeSpan(int x, int y, SkPMColor dst[], int count) {
    if (fColorsArePremul) {
        this->shadePremulSpan<DstType::L32, ApplyPremul::False>(x, y, dst, count);
    } else {
        this->shadePremulSpan<DstType::L32, ApplyPremul::True>(x, y, dst, count);
    }
}

void SkGradientShaderBase::
GradientShaderBase4fContext::shadeSpan4f(int x, int y, SkPM4f dst[], int count) {
    if (fColorsArePremul) {
        this->shadePremulSpan<DstType::F32, ApplyPremul::False>(x, y, dst, count);
    } else {
        this->shadePremulSpan<DstType::F32, ApplyPremul::True>(x, y, dst, count);
    }
}

template<DstType dstType, ApplyPremul premul>
void SkGradientShaderBase::
GradientShaderBase4fContext::shadePremulSpan(int x, int y,
                                             typename DstTraits<dstType, premul>::Type dst[],
                                             int count) const {
    const SkGradientShaderBase& shader =
        static_cast<const SkGradientShaderBase&>(fShader);

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
void SkGradientShaderBase::
GradientShaderBase4fContext::shadeSpanInternal(int x, int y,
                                               typename DstTraits<dstType, premul>::Type dst[],
                                               int count) const {
    static const int kBufSize = 128;
    SkScalar ts[kBufSize];
    TSampler<dstType, premul, tileMode> sampler(*this);

    SkASSERT(count > 0);
    do {
        const int n = SkTMin(kBufSize, count);
        this->mapTs(x, y, ts, n);
        for (int i = 0; i < n; ++i) {
            const Sk4f c = sampler.sample(ts[i]);
            DstTraits<dstType, premul>::store(c, dst++);
        }
        x += n;
        count -= n;
    } while (count > 0);
}

template<DstType dstType, ApplyPremul premul, SkShader::TileMode tileMode>
class SkGradientShaderBase::GradientShaderBase4fContext::TSampler {
public:
    TSampler(const GradientShaderBase4fContext& ctx)
        : fCtx(ctx)
        , fInterval(nullptr) {
        switch (tileMode) {
        case kClamp_TileMode:
            fLargestIntervalValue = SK_ScalarInfinity;
            break;
        case kRepeat_TileMode:
            fLargestIntervalValue = nextafterf(1, 0);
            break;
        case kMirror_TileMode:
            fLargestIntervalValue = nextafterf(2.0f, 0);
            break;
        }
    }

    Sk4f sample(SkScalar t) {
        const auto tiled_t = tileProc(t);

        if (!fInterval) {
            // Very first sample => locate the initial interval.
            // TODO: maybe do this in ctor to remove a branch?
            fInterval = fCtx.fIntervals.find(tiled_t);
            this->loadIntervalData(fInterval);
        } else if (!fInterval->contains(tiled_t)) {
            fInterval = fCtx.fIntervals.findNext(tiled_t, fInterval, t >= fPrevT);
            this->loadIntervalData(fInterval);
        }

        fPrevT = t;
        return lerp(tiled_t);
    }

private:
    SkScalar tileProc(SkScalar t) const {
        switch (tileMode) {
        case kClamp_TileMode:
            // synthetic clamp-mode edge intervals allow for a free-floating t:
            //   [-inf..0)[0..1)[1..+inf)
            return t;
        case kRepeat_TileMode:
            // t % 1  (intervals range: [0..1))
            // Due to the extra arithmetic, we must clamp to ensure the value remains less than 1.
            return SkTMin(t - SkScalarFloorToScalar(t), fLargestIntervalValue);
        case kMirror_TileMode:
            // t % 2  (synthetic mirror intervals expand the range to [0..2)
            // Due to the extra arithmetic, we must clamp to ensure the value remains less than 2.
            return SkTMin(t - SkScalarFloorToScalar(t / 2) * 2, fLargestIntervalValue);
        }

        SK_ABORT("Unhandled tile mode.");
        return 0;
    }

    Sk4f lerp(SkScalar t) {
        SkASSERT(fInterval->contains(t));
        return fCb + fCg * t;
    }

    void loadIntervalData(const Sk4fGradientInterval* i) {
        fCb = DstTraits<dstType, premul>::load(i->fCb);
        fCg = DstTraits<dstType, premul>::load(i->fCg);
    }

    const GradientShaderBase4fContext& fCtx;
    const Sk4fGradientInterval*        fInterval;
    SkScalar                           fPrevT;
    SkScalar                           fLargestIntervalValue;
    Sk4f                               fCb;
    Sk4f                               fCg;
};
